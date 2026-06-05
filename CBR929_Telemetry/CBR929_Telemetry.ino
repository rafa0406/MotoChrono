#include "Config.h"

// Inclusions des modules
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h"
#include "DisplayManager.h"
#include "SDLogger.h"
#include "WebServerManager.h"

// --- NOUVEAU : Inclusion de l'interface LVGL générée ---
#include "src/ui/ui.h"

// --- Prototypes des tâches ---
void core0Task(void * pvParameters);
void core1Task(void * pvParameters);

TaskHandle_t TaskCore0;
TaskHandle_t TaskCore1;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Démarrage Système CBR 929 (V2 LVGL)..."));

  pinMode(PIN_IGNITION, INPUT_PULLUP);
  pinMode(PIN_BUTTON1, INPUT_PULLUP);
  pinMode(PIN_BUTTON2, INPUT_PULLUP);

  FuelManager::init();
  
  // 1. Allumage de l'écran et création du pont SPI <-> LVGL
  DisplayManager::init(); 
  
  // 2. Lancement de l'interface dessinée dans EEZ Studio !
  ui_init(); 

  GPSManager::init();
  IMUManager::init();
  SDLogger::init();
  WebServerManager::init();

  // Lancement des tâches FreeRTOS
  xTaskCreatePinnedToCore(core0Task, "Task_Acq", 10000, NULL, 2, &TaskCore0, 0);
  xTaskCreatePinnedToCore(core1Task, "Task_UI",  10000, NULL, 1, &TaskCore1, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}

// ==========================================
// == TÂCHE CORE 0 : ACQUISITION CAPTEURS ===
// ==========================================
void core0Task(void * pvParameters) {
  for(;;) {
    GPSManager::update();
    IMUManager::update();
    vTaskDelay(pdMS_TO_TICKS(10)); // Boucle très rapide (~100Hz)
  }
}

// ==========================================
// == TÂCHE CORE 1 : UI & CARTE SD        ===
// ==========================================
void core1Task(void * pvParameters) {
  bool wasBtn1Pressed = (digitalRead(PIN_BUTTON1) == LOW);
  bool wasBtn2Pressed = (digitalRead(PIN_BUTTON2) == LOW);

  for(;;) {
    // --- GESTION DU BOUTON 1 (Changement de page) ---
    bool isBtn1Pressed = (digitalRead(PIN_BUTTON1) == LOW);
    bool triggerPageChange = false;
    
    // Front descendant (On ne déclenche qu'au moment précis de l'appui)
    if (isBtn1Pressed && !wasBtn1Pressed) {
        triggerPageChange = true;
    }
    wasBtn1Pressed = isBtn1Pressed;

    // --- GESTION DU BOUTON 2 (Start/Stop REC SD) ---
    bool isBtn2Pressed = (digitalRead(PIN_BUTTON2) == LOW);
    if (isBtn2Pressed && !wasBtn2Pressed) {
        SDLogger::toggleRecording();
    }
    wasBtn2Pressed = isBtn2Pressed;

    // --- LE MOTEUR GRAPHIQUE LVGL ---
    // On passe l'ordre de changement de page au DisplayManager
    DisplayManager::update(triggerPageChange); 
    
    // --- GESTION SD & WEBSERVER ---
    SDLogger::logData();
    WebServerManager::handleClient();
    
    // Pause OS pour laisser LVGL respirer (20 fps = très fluide)
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}