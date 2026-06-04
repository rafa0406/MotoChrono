#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <SD.h>
#include "SettingsManager.h" // Inclusion du gestionnaire de paramètres

class WebServerManager {
public:
    static void init();
    static void handleClient();
    static void toggleWiFi();
    static bool isWiFiActive();

private:
    static WebServer server;
    static DNSServer dnsServer;
    static bool wifiActive;
    static File fsUploadFile;

    static void handleRoot();
    static void handleDownload();
    
    // Gestion de la page de configuration
    static void handleConfig();
    static void handleSaveConfig();

    // HANDLERS FILE MANAGER
    static void handleDelete();
    static void handleUploadComplete();
    static void handleFileUpload();
};

#endif // WEBSERVER_MANAGER_H