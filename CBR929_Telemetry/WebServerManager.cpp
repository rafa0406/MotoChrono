#include "WebServerManager.h"
#include <vector>
#include <algorithm> // Nécessaire pour std::sort

// Petite structure légère pour stocker les infos des fichiers avant le tri
struct CsvFile {
    String name;
    size_t size;
};

// Instanciation du serveur sur le port HTTP standard (80)
WebServer WebServerManager::server(80);

void WebServerManager::init() {
    Serial.println(F("[WIFI] Démarrage du point d'accès MotoChrono..."));
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MotoChrono_CBR929", "piste929"); 

    Serial.print(F("[WIFI] Point d'accès créé. IP : "));
    Serial.println(WiFi.softAPIP()); 

    // Définition des routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/download", HTTP_GET, handleDownload);
    
    // NOUVELLES ROUTES : Configuration
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/save_config", HTTP_POST, handleSaveConfig);

    server.begin();
    Serial.println(F("[WIFI] Serveur Web HTTP démarré !"));
}

void WebServerManager::handleClient() {
    server.handleClient(); 
}

void WebServerManager::handleRoot() {
    // Génération de la page HTML
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif; background:#0f172a; color:white; padding:20px;} ";
    html += "a{display:block; padding:15px; margin:10px 0; text-align:center; font-weight:bold; color:white; text-decoration:none; border-radius:8px;} ";
    html += ".btn-app{background:#0284c7; box-shadow:0 4px 6px rgba(2,132,199,0.3);} "; 
    html += ".btn-apk{background:#16a34a; box-shadow:0 4px 6px rgba(22,163,74,0.3);} ";
    html += ".btn-csv{background:#dc2626; box-shadow:0 4px 6px rgba(220,38,38,0.3);} "; 
    html += ".btn-config{background:#f59e0b; box-shadow:0 4px 6px rgba(245,158,11,0.3);} "; // Bouton Orange
    html += "</style></head><body>";
    
    html += "<h2 style='text-align:center;'>🏍️ MotoChrono AP</h2>";

    // --- 0. BOUTON DE CONFIGURATION ---
    html += "<div style='margin-bottom:25px;'>";
    html += "<a href='/config' class='btn-config'>⚙️ Configuration MOTO</a>";
    html += "</div>";

    File root = SD.open("/");
    if (!root) {
        html += "<p>Erreur de lecture de la carte SD.</p></body></html>";
        server.send(200, "text/html", html);
        return;
    }

    // --- 1. BOUTONS D'APPLICATIONS (APK et HTML) ---
    bool hasApk = SD.exists("/MotoChrono.apk");
    bool hasHtml = SD.exists("/MotoChrono.html");

    if (hasApk || hasHtml) {
        html += "<div style='background:#1e293b; padding:15px; border-radius:8px; margin-bottom:25px; border:1px solid #334155;'>";
        html += "<h3 style='margin-top:0; color:#38bdf8;'>📱 Application d'Analyse</h3>";
        html += "<p style='font-size:12px; color:#94a3b8; margin-bottom:15px;'>Téléchargez l'interface pour visualiser vos tracés hors-ligne.</p>";
        
        if (hasApk) html += "<a href='/download?file=/MotoChrono.apk' class='btn-apk'>🤖 Installer l'App (Android APK)</a>";
        if (hasHtml) html += "<a href='/download?file=/MotoChrono.html' class='btn-app'>💻 Télécharger l'App (HTML)</a>";
        
        html += "</div>";
    }

    // --- 2. LISTE DES FICHIERS CSV (AVEC TRI DESCENDANT) ---
    html += "<h3>📊 Fichiers Télémétrie</h3>";
    
    std::vector<CsvFile> csvList;
    File file = root.openNextFile();
    
    while (file) {
        if (!file.isDirectory()) {
            String fileName = String(file.name());
            if (fileName.endsWith(".csv")) {
                csvList.push_back({fileName, file.size()});
            }
        }
        file = root.openNextFile();
    }

    std::sort(csvList.begin(), csvList.end(), [](const CsvFile& a, const CsvFile& b) {
        return a.name > b.name; 
    });

    if (csvList.empty()) {
        html += "<p style='color:#94a3b8;'>Aucun fichier CSV trouvé.</p>";
    } else {
        for (const auto& csv : csvList) {
            html += "<a href='/download?file=/" + csv.name + "' class='btn-csv'>📄 " + csv.name + " (" + String(csv.size / 1024) + " Ko)</a>";
        }
    }

    html += "</body></html>";
    server.send(200, "text/html; charset=utf-8", html);
}

void WebServerManager::handleDownload() {
    if (server.hasArg("file")) {
        String fileName = server.arg("file"); 
        
        if (SD.exists(fileName)) {
            File downloadFile = SD.open(fileName, FILE_READ);
            if (downloadFile) {
                String contentType = "text/plain";
                if (fileName.endsWith(".csv")) contentType = "text/csv";
                else if (fileName.endsWith(".html")) contentType = "text/html";
                else if (fileName.endsWith(".apk")) contentType = "application/vnd.android.package-archive"; 

                String cleanName = fileName;
                if (cleanName.startsWith("/")) {
                    cleanName = cleanName.substring(1); 
                }

                server.sendHeader("Content-Disposition", "attachment; filename=\"" + cleanName + "\"");
                server.streamFile(downloadFile, contentType);
                downloadFile.close();
                return;
            }
        }
    }
    server.send(404, "text/plain; charset=utf-8", "Fichier introuvable sur la carte SD.");
}

// ==========================================
// == PAGES DE CONFIGURATION               ==
// ==========================================

void WebServerManager::handleConfig() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif; background:#0f172a; color:white; padding:20px;} ";
    html += "input{width:90%; padding:10px; margin:5px 0 15px 0; border-radius:5px; border:1px solid #334155; background:#1e293b; color:white; display:block;} ";
    html += "label{font-size:13px; color:#94a3b8; font-weight:bold;} ";
    html += ".btn{display:block; padding:15px; text-align:center; font-weight:bold; color:white; text-decoration:none; border-radius:8px; border:none; width:100%; cursor:pointer; font-size:16px;} ";
    html += ".btn-save{background:#16a34a; margin-top:20px;} .btn-back{background:#475569; margin-top:10px;}";
    html += "</style></head><body>";
    
    html += "<h2 style='text-align:center; color:#f59e0b;'>⚙️ Configuration MOTO</h2>";
    
    html += "<form action='/save_config' method='POST'>";
    
    html += "<label>Capacité Réservoir (Litres)</label>";
    html += "<input type='number' step='0.1' name='tankCap' value='" + String(SettingsManager::tankCapacity) + "'>";

    html += "<label>Capacité Réserve (Litres)</label>";
    html += "<input type='number' step='0.1' name='resCap' value='" + String(SettingsManager::reserveCapacity) + "'>";

    html += "<label>Fréquence Télémétrie SD (Hz)</label>";
    html += "<input type='number' step='1' name='logFreq' value='" + String(SettingsManager::logFrequencyHz) + "'>";

    html += "<label>Vitesse Min. Enregistrement (km/h)</label>";
    html += "<input type='number' step='0.1' name='minSpd' value='" + String(SettingsManager::minSpeedLogging) + "'>";

    html += "<label>Lat. défaut (Garage/Atelier)</label>";
    html += "<input type='number' step='0.000001' name='gpsLat' value='" + String(SettingsManager::gpsDefaultLat, 6) + "'>";

    html += "<label>Lng. défaut (Garage/Atelier)</label>";
    html += "<input type='number' step='0.000001' name='gpsLng' value='" + String(SettingsManager::gpsDefaultLng, 6) + "'>";

    html += "<label>Vitesse factice GPS (km/h)</label>";
    html += "<input type='number' step='0.1' name='gpsSpd' value='" + String(SettingsManager::gpsFakeSpeed) + "'>";

    html += "<button type='submit' class='btn btn-save'>💾 Sauvegarder</button>";
    html += "</form>";
    
    html += "<a href='/' class='btn btn-back'>⬅️ Retour</a>";
    
    html += "</body></html>";
    server.send(200, "text/html; charset=utf-8", html);
}

void WebServerManager::handleSaveConfig() {
    // Mise à jour des variables en RAM
    if (server.hasArg("tankCap")) SettingsManager::tankCapacity = server.arg("tankCap").toFloat();
    if (server.hasArg("resCap")) SettingsManager::reserveCapacity = server.arg("resCap").toFloat();
    if (server.hasArg("logFreq")) SettingsManager::logFrequencyHz = server.arg("logFreq").toInt();
    if (server.hasArg("minSpd")) SettingsManager::minSpeedLogging = server.arg("minSpd").toFloat();
    if (server.hasArg("gpsLat")) SettingsManager::gpsDefaultLat = server.arg("gpsLat").toDouble();
    if (server.hasArg("gpsLng")) SettingsManager::gpsDefaultLng = server.arg("gpsLng").toDouble();
    if (server.hasArg("gpsSpd")) SettingsManager::gpsFakeSpeed = server.arg("gpsSpd").toFloat();

    // Ordre d'écriture dans la mémoire flash (NVS)
    SettingsManager::save(); 

    // Redirection HTTP "See Other" vers l'accueil
    server.sendHeader("Location", "/");
    server.send(303);
}