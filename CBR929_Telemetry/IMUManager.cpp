#include "IMUManager.h"
#include "Config.h"

// Broches I2C internes de ta carte Waveshare
#define I2C_SDA_INTERNAL 6
#define I2C_SCL_INTERNAL 7

uint8_t qmi8658_address = 0x6B; // On testera 0x6B, puis 0x6A
bool imuFound = false;

Preferences IMUManager::preferences;
Madgwick IMUManager::filter;

unsigned long IMUManager::lastUpdateMicros = 0;
float IMUManager::currentRoll = 0.0f;
float IMUManager::currentPitch = 0.0f;
float IMUManager::currentGTotal = 1.0f;
float IMUManager::currentGLong = 0.0f;

float IMUManager::rollOffset = 0.0f;
float IMUManager::pitchOffset = 0.0f;

void IMUManager::init() {
    Serial.println(F("[IMU] Initialisation I2C du QMI8658..."));

    preferences.begin("imu_data", false);
    rollOffset = preferences.getFloat("roll_off", 0.0f);
    pitchOffset = preferences.getFloat("pitch_off", 0.0f);
    preferences.end();

    Wire.begin(I2C_SDA_INTERNAL, I2C_SCL_INTERNAL);
    Wire.setClock(400000); 

    // --- DÉTECTION AUTOMATIQUE DE L'ADRESSE ---
    Wire.beginTransmission(0x6B);
    if (Wire.endTransmission() == 0) {
        qmi8658_address = 0x6B;
        imuFound = true;
    } else {
        Wire.beginTransmission(0x6A);
        if (Wire.endTransmission() == 0) {
            qmi8658_address = 0x6A;
            imuFound = true;
        }
    }

    if (!imuFound) {
        Serial.println(F("[IMU] ❌ ERREUR : Capteur QMI8658 introuvable !"));
        return; // On stoppe l'init ici pour ne pas faire planter la boucle
    }

    Serial.print(F("[IMU] ✅ Capteur trouvé à l'adresse 0x"));
    Serial.println(qmi8658_address, HEX);

    // Allumage et configuration du capteur
    Wire.beginTransmission(qmi8658_address);
    Wire.write(0x02); Wire.write(0x60); 
    Wire.endTransmission();

    Wire.beginTransmission(qmi8658_address);
    Wire.write(0x03); Wire.write(0x24); 
    Wire.endTransmission();

    Wire.beginTransmission(qmi8658_address);
    Wire.write(0x04); Wire.write(0x54); 
    Wire.endTransmission();

    Wire.beginTransmission(qmi8658_address);
    Wire.write(0x08); Wire.write(0x03); 
    Wire.endTransmission();

    filter.begin(100);
}

void IMUManager::update() {
    if (!imuFound) return; // Sécurité

    Wire.beginTransmission(qmi8658_address);
    Wire.write(0x35); 
    if (Wire.endTransmission(false) != 0) return; 

    Wire.requestFrom(qmi8658_address, (uint8_t)12);
    
    if(Wire.available() == 12) {
        int16_t ax_raw = Wire.read() | (Wire.read() << 8);
        int16_t ay_raw = Wire.read() | (Wire.read() << 8);
        int16_t az_raw = Wire.read() | (Wire.read() << 8);
        
        int16_t gx_raw = Wire.read() | (Wire.read() << 8);
        int16_t gy_raw = Wire.read() | (Wire.read() << 8);
        int16_t gz_raw = Wire.read() | (Wire.read() << 8);

        float ax = ax_raw / 4096.0f; 
        float ay = ay_raw / 4096.0f;
        float az = az_raw / 4096.0f;
        
        float gx = gx_raw / 64.0f;   
        float gy = gy_raw / 64.0f;
        float gz = gz_raw / 64.0f;

        filter.updateIMU(gx, gy, gz, ax, ay, az);

        currentRoll = filter.getRoll() - rollOffset;
        currentPitch = filter.getPitch() - pitchOffset;

        currentGTotal = sqrt((ax * ax) + (ay * ay) + (az * az));
        currentGLong = ay; 
    }
}

void IMUManager::calibrateZero() {
    if (!imuFound) return;
    rollOffset = filter.getRoll();
    pitchOffset = filter.getPitch();
    preferences.begin("imu_data", false);
    preferences.putFloat("roll_off", rollOffset);
    preferences.putFloat("pitch_off", pitchOffset);
    preferences.end();
}

float IMUManager::getRoll() { return currentRoll; }
float IMUManager::getPitch() { return currentPitch; }
float IMUManager::getGForceTotal() { return currentGTotal; }
float IMUManager::getGForceLong() { return currentGLong; }