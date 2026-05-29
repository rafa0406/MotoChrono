#include "ImuManager.h" // Inclusion cruciale du header corrigé !
#include "Config.h"     // Inclusion de ta config si nécessaire

// Définition des broches I2C internes de ta carte Waveshare
#define I2C_SDA_INTERNAL 6
#define I2C_SCL_INTERNAL 7
#define QMI8658_ADDRESS  0x6B // Adresse I2C standard du QMI8658

// --- Allocation mémoire des variables statiques de la classe ---
Madgwick IMUManager::filter;
Preferences IMUManager::preferences;

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

    // Initialisation du bus I2C interne de la carte Waveshare
    Wire.begin(I2C_SDA_INTERNAL, I2C_SCL_INTERNAL);
    
    // Initialisation du filtre de Madgwick (ex: à 100 Hz)
    filter.begin(100);

    // [!] Ici, il faudra envoyer les commandes I2C pour réveiller ton capteur QMI8658
    // Wire.beginTransmission(QMI8658_ADDRESS);
    // ...
}

// ==========================================
// == BOUCLE DE MISE À JOUR (CORE 0)       ==
// ==========================================
void IMUManager::update() {
    // --- 1. Lecture I2C des données brutes (Ax, Ay, Az, Gx, Gy, Gz) ---
    // Remplace ces variables par tes vraies lectures I2C du capteur QMI8658
    float ax = 0.0f, ay = 0.0f, az = 1.0f; // Variables factices pour compiler
    float gx = 0.0f, gy = 0.0f, gz = 0.0f; 

    // --- 2. Mise à jour du filtre de Madgwick ---
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // --- 3. Récupération et application de la calibration ---
    currentRoll = filter.getRoll() - rollOffset;
    currentPitch = filter.getPitch() - pitchOffset;

    // --- 4. Calcul des forces G ---
    currentGTotal = sqrt((ax * ax) + (ay * ay) + (az * az));
    currentGLong = ay; // À adapter selon le sens physique de montage de ta carte !
}

// ==========================================
// == CALIBRATION ET SAUVEGARDE (NVS)      ==
// ==========================================
void IMUManager::calibrateZero() {
    Serial.println(F("[IMU] Calibration du Zéro en cours... Maintenez la moto droite !"));
    
    // On attend un peu que le filtre se stabilise
    delay(1000); 
    
    // On enregistre les angles actuels comme étant le nouveau point zéro
    rollOffset = filter.getRoll();
    pitchOffset = filter.getPitch();
    
    // --- SAUVEGARDE NVS IMMÉDIATE ---
    preferences.begin("imu_data", false);
    preferences.putFloat("roll_off", rollOffset);
    preferences.putFloat("pitch_off", pitchOffset);
    preferences.end();
    
    Serial.println(F("[IMU] Moto calibrée et sauvegardée définitivement (NVS)."));
}

// ==========================================
// == GETTERS (Accesseurs)                 ==
// ==========================================
float IMUManager::getRoll() {
    return currentRoll;
}

float IMUManager::getPitch() {
    return currentPitch;
}

float IMUManager::getGForceTotal() {
    return currentGTotal;
}

float IMUManager::getGForceLong() {
    return currentGLong;
}