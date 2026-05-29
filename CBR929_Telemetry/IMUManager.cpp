#include "IMUManager.h"
#include "Config.h"

// Définition des broches I2C internes de ta carte Waveshare
#define I2C_SDA_INTERNAL 6
#define I2C_SCL_INTERNAL 7
#define QMI8658_ADDRESS  0x6B // Adresse I2C standard du QMI8658

Preferences IMUManager::preferences;

Madgwick IMUManager::filter;

unsigned long IMUManager::lastUpdateMicros = 0;

float IMUManager::currentRoll = 0.0f;
float IMUManager::currentPitch = 0.0f;
float IMUManager::currentGTotal = 1.0f;
float IMUManager::currentGLong = 0.0f;

float IMUManager::rollOffset = 0.0f;
float IMUManager::pitchOffset = 0.0f;

// ==========================================
// == INITIALISATION                       ==
// ==========================================
void IMUManager::init() {
    Serial.println(F("[IMU] Initialisation du QMI8658..."));

    // --- CHARGEMENT NVS (Offsets de calibration) ---
    preferences.begin("imu_data", false);
    rollOffset = preferences.getFloat("roll_off", 0.0f);
    pitchOffset = preferences.getFloat("pitch_off", 0.0f);
    preferences.end();
    
    Serial.print(F("[IMU] Offsets chargés - Roulis: "));
    Serial.print(rollOffset);
    Serial.print(F(" | Tangage: "));
    Serial.println(pitchOffset);

    // --- INITIALISATION DU CAPTEUR (Bus I2C) ---
    Wire.begin(I2C_SDA_INTERNAL, I2C_SCL_INTERNAL);
    Wire.setClock(400000); // I2C Fast Mode (400 kHz) pour lecture très rapide

    // 1. Configuration CTRL1 (Adresse auto-increment)
    Wire.beginTransmission(QMI8658_ADDRESS);
    Wire.write(0x02); 
    Wire.write(0x60); 
    Wire.endTransmission();

    // 2. Configuration CTRL2 : Accéléromètre (±8g, 1000Hz)
    Wire.beginTransmission(QMI8658_ADDRESS);
    Wire.write(0x03); 
    Wire.write(0x24); 
    Wire.endTransmission();

    // 3. Configuration CTRL3 : Gyroscope (±512 °/s, 1000Hz)
    Wire.beginTransmission(QMI8658_ADDRESS);
    Wire.write(0x04); 
    Wire.write(0x54); 
    Wire.endTransmission();

    // 4. Configuration CTRL7 : Activation des capteurs
    Wire.beginTransmission(QMI8658_ADDRESS);
    Wire.write(0x08); 
    Wire.write(0x03); // Activer Gyro et Accel
    Wire.endTransmission();

    // Initialisation du filtre de Madgwick (~100 Hz correspondant à notre Tâche Core 0)
    filter.begin(100);
}

// ==========================================
// == BOUCLE DE MISE À JOUR (CORE 0)       ==
// ==========================================
void IMUManager::update() {
    // Pointer sur le premier registre de données (0x35 : Accel X Low)
    Wire.beginTransmission(QMI8658_ADDRESS);
    Wire.write(0x35); 
    if (Wire.endTransmission(false) != 0) return; // Sécurité si capteur non trouvé

    // Demander la lecture des 12 octets consécutifs (6 axes * 2 octets)
    Wire.requestFrom(QMI8658_ADDRESS, 12);
    
    if(Wire.available() == 12) {
        // Lecture brute de l'accéléromètre
        int16_t ax_raw = Wire.read() | (Wire.read() << 8);
        int16_t ay_raw = Wire.read() | (Wire.read() << 8);
        int16_t az_raw = Wire.read() | (Wire.read() << 8);
        
        // Lecture brute du gyroscope
        int16_t gx_raw = Wire.read() | (Wire.read() << 8);
        int16_t gy_raw = Wire.read() | (Wire.read() << 8);
        int16_t gz_raw = Wire.read() | (Wire.read() << 8);

        // --- CONVERSIONS ---
        // Accéléromètre paramétré à ±8g -> 4096 LSB/g
        float ax = ax_raw / 4096.0f; 
        float ay = ay_raw / 4096.0f;
        float az = az_raw / 4096.0f;
        
        // Gyroscope paramétré à ±512 dps -> 64 LSB/dps
        float gx = gx_raw / 64.0f;   
        float gy = gy_raw / 64.0f;
        float gz = gz_raw / 64.0f;

        // --- MISE À JOUR DU FILTRE ---
        filter.updateIMU(gx, gy, gz, ax, ay, az);

        // --- APPLICATION DE LA CALIBRATION ---
        currentRoll = filter.getRoll() - rollOffset;
        currentPitch = filter.getPitch() - pitchOffset;

        // --- CALCUL DES FORCES G ---
        currentGTotal = sqrt((ax * ax) + (ay * ay) + (az * az));
        
        // La Force G longitudinale (freinage/accélération) dépend du sens de la carte !
        // Si ta carte est à plat, le freinage se lira souvent sur l'axe Y ou X. 
        // À ajuster selon le montage final (ex: currentGLong = ay;)
        currentGLong = ay; 
    }
}

// ==========================================
// == CALIBRATION ET SAUVEGARDE (NVS)      ==
// ==========================================
void IMUManager::calibrateZero() {
    Serial.println(F("[IMU] Calibration du Zéro en cours... Maintenez la moto droite !"));
    
    delay(1000); 
    
    rollOffset = filter.getRoll();
    pitchOffset = filter.getPitch();
    
    preferences.begin("imu_data", false);
    preferences.putFloat("roll_off", rollOffset);
    preferences.putFloat("pitch_off", pitchOffset);
    preferences.end();
    
    Serial.println(F("[IMU] Moto calibrée et sauvegardée définitivement (NVS)."));
}

// ==========================================
// == GETTERS (Accesseurs)                 ==
// ==========================================
float IMUManager::getRoll() { return currentRoll; }
float IMUManager::getPitch() { return currentPitch; }
float IMUManager::getGForceTotal() { return currentGTotal; }
float IMUManager::getGForceLong() { return currentGLong; }