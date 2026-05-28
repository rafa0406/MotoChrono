// ... existing code ...
#define I2C_SDA_INTERNAL 6
#define I2C_SCL_INTERNAL 7
#define QMI8658_ADDRESS  0x6B // Adresse I2C standard du QMI

Preferences IMUManager::preferences; // Initialisation de l'objet NVS

// Initialisation des variables statiques
Madgwick IMUManager::filter;
// ... existing code ...
float IMUManager::currentGTotal = 1.0f;
float IMUManager::currentGLong = 0.0f;

void IMUManager::init() {
    Serial.println(F("[IMU] Initialisation du QMI8658..."));

    // --- CHARGEMENT NVS ---
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
// ... existing code ...
void IMUManager::calibrateZero() {
    Serial.println(F("[IMU] Calibration du Zéro en cours... Maintenez la moto droite !"));
    
    // On attend un peu que le filtre se stabilise
    delay(1000); 
    
    // On enregistre les angles actuels comme étant le point zéro
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
// ... existing code ...