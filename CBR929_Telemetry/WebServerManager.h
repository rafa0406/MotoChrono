#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>

class WebServerManager {
public:
    // Configure le point d'accès WiFi et démarre le serveur
    static void init();

    // À appeler dans la boucle du Core 1 pour traiter les requêtes entrantes
    static void handleClient();

private:
    static WebServer server;

    // Routes (Endpoints) du serveur web
    static void handleRoot();
    static void handleDownload();
};

#endif // WEBSERVER_MANAGER_H