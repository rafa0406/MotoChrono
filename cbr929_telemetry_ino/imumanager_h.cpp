// ... existing code ...
#include <Arduino.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Preferences.h> // Ajout de la librairie NVS

class IMUManager {
public:
// ... existing code ...
    static void calibrateZero();

private:
    static Madgwick filter;
    static Preferences preferences; // Ajout de l'objet Preferences
    static unsigned long lastUpdateMicros;

    // Décalages pour la calibration
// ... existing code ...