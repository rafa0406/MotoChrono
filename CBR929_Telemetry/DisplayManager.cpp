#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h" 
#include "SDLogger.h" 
#include "SettingsManager.h"
#include "WebServerManager.h"

// --- Inclusion de l'interface EEZ LVGL ---
#include "src/ui/ui.h"
#include "src/ui/vars.h"

// Variables statiques globales
TFT_eSPI tft = TFT_eSPI();
lv_disp_draw_buf_t DisplayManager::draw_buf;
lv_color_t *DisplayManager::buf1 = nullptr;

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
DisplayPage DisplayManager::previousPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;
unsigned long DisplayManager::lastMotionTime = 0;

// Variables de records
float DisplayManager::maxSpeed = 0.0f; 
float DisplayManager::maxLeanLeft = 0.0f;
float DisplayManager::maxLeanRight = 0.0f;
float DisplayManager::gForceAtMaxLeanLeft = 0.0f;
float DisplayManager::gForceAtMaxLeanRight = 0.0f;
float DisplayManager::maxPitchUp = 0.0f;
float DisplayManager::maxPitchDown = 0.0f;
float DisplayManager::gForceAtMaxPitchUp = 0.0f;
float DisplayManager::gForceAtMaxPitchDown = 0.0f;
float DisplayManager::maxGForce = 0.0f;
float DisplayManager::maxBrakingG = 0.0f;
float DisplayManager::maxAccelG = 0.0f;
float DisplayManager::best0to100 = 99.99f; 

unsigned long DisplayManager::accelStartTime = 0;
bool DisplayManager::isTracking0to100 = false;
bool DisplayManager::tropheesLoaded = false;
bool DisplayManager::tropheesChanged = false;

// ======================================================================
// == VARIABLES TAMPONS POUR EEZ STUDIO (Valeurs par défaut)           ==
// ======================================================================
static char eez_speed[8] = "0";
static char eez_max_speed[8] = "0";
static char eez_fuel_value[8] = "0.0";
static char eez_fuel_avg[8] = "0.0";
static char eez_g_force[16] = "0.00 G";
static char eez_roll_str[8] = "0";
static char eez_pitch_str[8] = "0";

static int32_t eez_fuel_int = 0;
static int32_t eez_roll_int = 0;
static int32_t eez_pitch_int = 0;

// Variables Booléennes (Attention : logiques inversées pour "Hidden" = caché)
static bool eez_gps_fix_ok_hidden = true;  // Caché par défaut
static bool eez_gps_fix_ko_hidden = false; // Visible par défaut
static bool eez_rec_hidden = true;         // Caché par défaut
static bool eez_fuel_res_hidden = true;    // Caché par défaut


// ==========================================
// 1. LE "BRIDGE" SPI (Écriture ultra-rapide)
// ==========================================
void DisplayManager::my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}

void DisplayManager::init() {
    Serial.println(F("[DISPLAY] 1. Init Hardware..."));
    pinMode(PIN_BACKLIGHT, OUTPUT); 
    digitalWrite(PIN_BACKLIGHT, HIGH); 
    
    tft.init(); 
    tft.setRotation(3); 
    tft.fillScreen(TFT_BLACK);

    Serial.println(F("[DISPLAY] 2. Init LVGL Core..."));
    lv_init();

    size_t buffer_size = (240 * 240) / 10;
    buf1 = (lv_color_t *)heap_caps_malloc(buffer_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    lv_disp_draw_buf_init(&draw_buf, buf1, NULL, buffer_size);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush; 
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lastMotionTime = millis();
}

void DisplayManager::update(bool isButtonPressed) {
    static uint32_t lastTick = 0;
    uint32_t currentMillis = millis();
    lv_tick_inc(currentMillis - lastTick);
    lastTick = currentMillis;

    // ==========================================
    // ANIMATION DE DÉMARRAGE (BOOT LOGO)
    // ==========================================
    static bool isBooting = true;
    static unsigned long bootStartTime = 0;

    if (isBooting) {
        if (bootStartTime == 0) bootStartTime = currentMillis; 
        unsigned long elapsed = currentMillis - bootStartTime;

        if (elapsed < 3200) { 
            if (lv_scr_act() != objects.logo) loadScreen(SCREEN_ID_LOGO);
            float phase = (float)elapsed / 3000.0f * 3.0f * PI;
            int32_t rpm_val = 1 + pow(sin(phase), 6) * 11; 
            if (objects.compte_tour_logo && screen_logo_state.tr_min) {
                lv_meter_set_indicator_value(objects.compte_tour_logo, screen_logo_state.tr_min, rpm_val);
            }
        } else {
            isBooting = false;
            currentPage = PAGE_ROUTE;
            loadScreen(SCREEN_ID_SCREEN_ROUTE); 
        }
        ui_tick(); 
        lv_timer_handler(); 
        return; 
    }

    // ==========================================
    // LOGIQUE TÉLÉMÉTRIE & RECORDS
    // ==========================================
    if (!tropheesLoaded && currentMillis > 4000) {
        if (SDLogger::getIsInitialized()) {
            SDLogger::loadTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
            tropheesLoaded = true;
        }
    }

    float currentSpeed = GPSManager::getSpeedKmh();
    float currentRoll = IMUManager::getRoll(); 
    float currentPitch = IMUManager::getPitch();
    float currentGForce = IMUManager::getGForceTotal(); 

    // Enregistrement des records
    bool recordBroken = false; 
    if (currentRoll < maxLeanLeft) { maxLeanLeft = currentRoll; gForceAtMaxLeanLeft = currentGForce; recordBroken = true; }
    if (currentRoll > maxLeanRight) { maxLeanRight = currentRoll; gForceAtMaxLeanRight = currentGForce; recordBroken = true; }
    if (currentPitch < maxPitchDown) { maxPitchDown = currentPitch; gForceAtMaxPitchDown = currentGForce; }
    if (currentPitch > maxPitchUp) { maxPitchUp = currentPitch; gForceAtMaxPitchUp = currentGForce; }
    if (currentGForce > maxGForce) { maxGForce = currentGForce; recordBroken = true; }
    if (currentSpeed > maxSpeed) { maxSpeed = currentSpeed; recordBroken = true; }
    if (currentPitch < 0.0f && currentGForce > maxBrakingG) { maxBrakingG = currentGForce; recordBroken = true; }
    if (currentPitch > 0.0f && currentGForce > maxAccelG) { maxAccelG = currentGForce; recordBroken = true; }
    if (recordBroken) tropheesChanged = true;

    // Réveil et Veille
    if (currentSpeed >= STANDBY_SPEED_THRESH) {
        lastMotionTime = currentMillis; 
        if (currentPage == PAGE_VEILLE) {
            currentPage = previousPage; 
            digitalWrite(PIN_BACKLIGHT, HIGH); 
        }
    }

    if (currentPage != PAGE_VEILLE && (currentMillis - lastMotionTime > STANDBY_TIMEOUT_MS)) {
        previousPage = currentPage; 
        currentPage = PAGE_VEILLE;
        digitalWrite(PIN_BACKLIGHT, LOW); 
        if (tropheesChanged) {
            SDLogger::saveTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
            tropheesChanged = false; 
        }
    }

    // Changement de page avec EEZ
    if (isButtonPressed && (currentMillis - lastButtonPressMs > 300)) { 
        lastButtonPressMs = currentMillis;
        lastMotionTime = currentMillis; 
        if (currentPage == PAGE_VEILLE) {
            currentPage = previousPage;
            digitalWrite(PIN_BACKLIGHT, HIGH);
        } else {
            if (currentPage == PAGE_ROUTE) { currentPage = PAGE_PISTE; loadScreen(SCREEN_ID_SCREEN_PISTE); }
            else if (currentPage == PAGE_PISTE) { currentPage = PAGE_TROPHEES; loadScreen(SCREEN_ID_SCREEN_TROPHEES); }
            else if (currentPage == PAGE_TROPHEES) { currentPage = PAGE_CALIBRATION; loadScreen(SCREEN_ID_SCREEN_CALIBRATION); }
            else if (currentPage == PAGE_CALIBRATION) { currentPage = PAGE_GPS; loadScreen(SCREEN_ID_SCREEN_GPS); }
            else if (currentPage == PAGE_GPS) { currentPage = PAGE_WIFI; loadScreen(SCREEN_ID_SCREEN_WI_FI); }
            else { currentPage = PAGE_ROUTE; loadScreen(SCREEN_ID_SCREEN_ROUTE); }
        }
    }

    // ==========================================================
    // INJECTION DES DONNÉES DANS LES TAMPONS EEZ
    // ==========================================================
    snprintf(eez_speed, sizeof(eez_speed), "%d", (int)currentSpeed);
    snprintf(eez_max_speed, sizeof(eez_max_speed), "%d", (int)maxSpeed);
    snprintf(eez_g_force, sizeof(eez_g_force), "%.2f G", currentGForce);
    snprintf(eez_fuel_value, sizeof(eez_fuel_value), "%.1f", FuelManager::getRemainingLiters());
    
    // Nouveaux tampons String pour le Roll et Pitch
    snprintf(eez_roll_str, sizeof(eez_roll_str), "%d", (int)currentRoll);
    snprintf(eez_pitch_str, sizeof(eez_pitch_str), "%d", (int)currentPitch);

    eez_fuel_int = (int32_t)FuelManager::getRemainingLiters();
    eez_roll_int = (int32_t)currentRoll;
    eez_pitch_int = (int32_t)currentPitch;

    // -----------------------------------------------------
    // LOGIQUE UI : LEDs GPS (Caché = true)
    // -----------------------------------------------------
    bool hasFix = GPSManager::isDataValid();
    if (hasFix) {
        eez_gps_fix_ok_hidden = false; // GPS OK -> Vert VISIBLE
        eez_gps_fix_ko_hidden = true;  // Rouge CACHÉ
    } else {
        eez_gps_fix_ok_hidden = true;  // Vert CACHÉ
        // Le "(currentMillis / 500) % 2 != 0" génère une alternance vrai/faux
        eez_gps_fix_ko_hidden = ((currentMillis / 500) % 2 != 0); 
    }

    // -----------------------------------------------------
    // LOGIQUE UI : LED REC
    // -----------------------------------------------------
    if (SDLogger::isRecordingStatus()) {
        eez_rec_hidden = ((currentMillis / 500) % 2 != 0); 
    } else {
        eez_rec_hidden = true; 
    }
    
    // -----------------------------------------------------
    // LOGIQUE UI : LED RÉSERVE ESSENCE
    // -----------------------------------------------------
    // isReserve() renvoie true quand on EST en réserve.
    // Donc on la cache (!isReserve) quand on N'EST PAS en réserve.
    eez_fuel_res_hidden = !FuelManager::isReserve(); 

    // Ordonne à EEZ de traiter les variables, puis à LVGL de dessiner
    ui_tick(); 
    lv_timer_handler(); 
}

void DisplayManager::showByeBye() {
    if (tropheesChanged) {
        SDLogger::saveTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
        tropheesChanged = false;
    }
    loadScreen(SCREEN_ID_LOGO);
    ui_tick();
    lv_timer_handler();
}

// ======================================================================
// == PONT C/C++ : VARIABLES GLOBALES NATIVES GÉNÉRÉES PAR EEZ STUDIO ===
// ======================================================================

extern "C" {
    const char* get_var_speed() { return eez_speed; }
    void set_var_speed(const char* value) { }

    const char* get_var_max_speed() { return eez_max_speed; }
    void set_var_max_speed(const char* value) { }

    bool get_var_gps_fix_ok() { return eez_gps_fix_ok_hidden; }
    void set_var_gps_fix_ok(bool value) { }

    bool get_var_gps_fix_ko() { return eez_gps_fix_ko_hidden; }
    void set_var_gps_fix_ko(bool value) { }

    bool get_var_rec_start() { return eez_rec_hidden; }
    void set_var_rec_start(bool value) { }

    float get_var_fuel_max() { return (SettingsManager::tankCapacity > 0) ? SettingsManager::tankCapacity : 18.0f; }
    void set_var_fuel_max(float value) { }

    float get_var_fuel_reserve() { return SettingsManager::reserveCapacity; }
    void set_var_fuel_reserve(float value) { }

    const char* get_var_fuel_value() { return eez_fuel_value; }
    void set_var_fuel_value(const char* value) { }

    int32_t get_var_fuel_value_int() { return eez_fuel_int; }
    void set_var_fuel_value_int(int32_t value) { }

    const char* get_var_fuel_average_value() { return eez_fuel_avg; }
    void set_var_fuel_average_value(const char* value) { }

    bool get_var_fuel_is_reseve() { return eez_fuel_res_hidden; }
    void set_var_fuel_is_reseve(bool value) { }

    const char* get_var_g_force() { return eez_g_force; }
    void set_var_g_force(const char* value) { }

    // Les nouveaux accesseurs pour l'inclinaison et l'assiette (String + Int)
    const char* get_var_roll_value() { return eez_roll_str; }
    void set_var_roll_value(const char* value) { }

    const char* get_var_pitch_value() { return eez_pitch_str; }
    void set_var_pitch_value(const char* value) { }

    int32_t get_var_roll_value_int() { return eez_roll_int; }
    void set_var_roll_value_int(int32_t value) { }

    int32_t get_var_pitch_value_int() { return eez_pitch_int; }
    void set_var_pitch_value_int(int32_t value) { }
}