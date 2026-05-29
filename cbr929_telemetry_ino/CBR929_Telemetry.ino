/*
  Projet : Télémétrie & Tableau de Bord pour Honda CBR 929 RR
  Architecture : ESP32-S3 (Dual Core) avec QMI8658 & ST7789
  Description : Fichier principal orchestrant les différents modules.
====== Bibliothèques externes ======
- TFT_eSPI (de Bodmer) : C'est la bibliothèque ultra-rapide et optimisée qui gère l'affichage sur ton écran IPS ST7789.
- TinyGPSPlus (de Mikal Hart) : Elle permet de décoder (parser) les trames NMEA brutes envoyées par ton module GPS (ATGM336H/BN-220) pour en extraire la vitesse, la latitude et la longitude.
- Madgwick (de Arduino)(souvent nommée MadgwickAHRS) : C'est l'algorithme mathématique de fusion de capteurs qui va transformer les données brutes du QMI8658 (accéléromètre + gyroscope) en angles d'inclinaison (roulis et tangage) exploitables.
*/

#include "Config.h"
// Inclusions futures des autres modules
// #include "FuelManager.h"
// #include "GPSManager.h"
// #include "IMUManager.h"
// #include "DisplayManager.h"
// #include "SDLogger.h"

// --- Déclaration des prototypes des fonctions ---
void core0Task(void * pvParameters);
void core1Task(void * pvParameters);

// --- Tâches FreeRTOS pour le Dual Core ---
TaskHandle_t TaskCore0; // Gèrera l'Acquisition (Interruptions, GPS, IMU)
TaskHandle_t TaskCore1; // Gèrera l'Affichage et le Datalogging SD

void setup() {
  Serial.begin(115200);
  Serial.println(F("Démarrage Système CBR 929 (ESP32-S3)..."));

  // 1. Initialisation des Pins de base
  pinMode(PIN_IGNITION, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  // (L'interruption de l'injecteur sera initialisée dans le FuelManager)

  // 2. Initialisation des modules (à décommenter au fur et à mesure)
  // FuelManager::init();
  // DisplayManager::init();
  // GPSManager::init();
  // IMUManager::init();
  // SDLogger::init();

  // 3. Lancement des tâches sur les deux cœurs
  // Core 0 (Priorité haute) : Acquisition des données critiques
  xTaskCreatePinnedToCore(
    core0Task,          /* Fonction de la tâche */
    "Task_Acquisition", /* Nom de la tâche */
    10000,              /* Taille de la pile (Stack size) */
    NULL,               /* Paramètre de la tâche */
    2,                  /* Priorité (0 = basse, configMAX_PRIORITIES-1 = haute) */
    &TaskCore0,         /* Handle de la tâche */
    0);                 /* Numéro du cœur (Core 0) */

  // Core 1 (Priorité basse) : Affichage et Écriture SD (Opérations lentes)
  xTaskCreatePinnedToCore(
    core1Task,
    "Task_UI_SD",
    10000,
    NULL,
    1,
    &TaskCore1,
    1);                 /* Numéro du cœur (Core 1) */
}

void loop() {
  // La fonction loop() par défaut tourne sur le Core 1, mais nous 
  // gérons tout via les tâches FreeRTOS créées ci-dessus.
  // On la laisse vide ou on y met un petit délai.
  vTaskDelay(portMAX_DELAY);
}

// ==========================================
// == TÂCHES DES COEURS (BOUCLES INFINIES) ==
// ==========================================

void core0Task(void * pvParameters) {
  for(;;) {
    // --- LECTURE DES CAPTEURS (Ultra-rapide) ---
    // GPSManager::update();
    // IMUManager::update();
    
    // NB: La lecture de l'injecteur se fait via Interruption Matérielle
    // Elle interviendra asynchronement et interrompra cette boucle 
    // l'espace de quelques microsecondes.

    // Petite pause pour laisser l'OS respirer (Watchdog)
    vTaskDelay(pdMS_TO_TICKS(10)); // Boucle à ~100Hz max
  }
}

void core1Task(void * pvParameters) {
  for(;;) {
    // --- INTERFACE ET STOCKAGE (Plus lent) ---
    // bool buttonPressed = digitalRead(PIN_BUTTON) == LOW;
    // DisplayManager::update(buttonPressed);
    // SDLogger::logData(FuelManager::getData(), GPSManager::getData(), IMUManager::getData());

    // --- GESTION DE LA COUPURE MOTEUR ---
    if(digitalRead(PIN_IGNITION) == HIGH) { // Si la clé est coupée
      Serial.println(F("Contact coupé ! Sauvegarde en cours..."));
      // FuelManager::saveToNVS();
      // SDLogger::closeFile();
      // DisplayManager::showByeBye();
      
      // Passage en Deep Sleep
      esp_deep_sleep_start();
    }

    // L'écran et la SD n'ont pas besoin d'être mis à jour trop vite
    vTaskDelay(pdMS_TO_TICKS(50)); // Boucle à ~20Hz
  }
}