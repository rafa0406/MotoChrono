#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SD.h>
#include "SettingsManager.h" // NOUVEAU : Inclusion du gestionnaire de paramètres

class WebServerManager {
public:
    static void init();
    static void handleClient();
    static void toggleWiFi();
    static bool isWiFiActive();

private:
    static WebServer server;
    static bool wifiActive;
    
    static void handleRoot();
    static void handleDownload();
    
    // NOUVELLES METHODES : Gestion de la page de configuration
    static void handleConfig();
    static void handleSaveConfig();
};

#endif // WEBSERVER_MANAGER_H