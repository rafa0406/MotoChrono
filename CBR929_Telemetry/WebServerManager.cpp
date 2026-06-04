#include "WebServerManager.h"
#include <vector>
#include <algorithm> 
#include "Config.h"

// Instanciation du serveur et de la variable d'état
WebServer WebServerManager::server(80);
DNSServer WebServerManager::dnsServer;
bool WebServerManager::wifiActive = false; // <-- Radio OFF par défaut

struct CsvFile {
    String name;
    size_t size;
};

void WebServerManager::init() {
    Serial.println(F("[WIFI] Préparation du serveur (Radio OFF par défaut - Eco Energie)"));
    
    // On coupe la radio au boot
    WiFi.mode(WIFI_OFF);

    // Définition des routes (Fait une seule fois)
    server.on("/", HTTP_GET, handleRoot);
    server.on("/download", HTTP_GET, handleDownload);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/save_config", HTTP_POST, handleSaveConfig);
    server.on("/delete", HTTP_GET, handleDelete);
    server.on("/upload", HTTP_POST, handleUploadComplete, handleFileUpload);
}

void WebServerManager::toggleWiFi() {
    wifiActive = !wifiActive;

    if (wifiActive) {
        Serial.println(F("[WIFI] Démarrage du point d'accès MotoChrono..."));
        
        // Forcer une IP personnalisée (ex: 10.0.0.1) ---
        IPAddress local_IP(WIFI_IP_1, WIFI_IP_2, WIFI_IP_3, WIFI_IP_4);     // L'adresse IP de ton ESP32
        IPAddress gateway(WIFI_IP_1, WIFI_IP_2, WIFI_IP_3, WIFI_IP_4);      // La passerelle (souvent identique à l'IP en mode AP)
        IPAddress subnet(WIFI_IP_MASK_1, WIFI_IP_MASK_2, WIFI_IP_MASK_3, WIFI_IP_MASK_4);  // Le masque de sous-réseau standard
        
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(local_IP, gateway, subnet); // <-- Application de la config IP
        WiFi.softAP(String(WIFI_SSID), String(WIFI_MDP)); 
        
        // Redirige la requête Alias vers notre IP local
        dnsServer.start(53, String(WIFI_ALIAS_IP), local_IP);

        server.begin();
        Serial.print(F("[WIFI] Point d'accès créé. IP : "));
        Serial.println(WiFi.softAPIP()); 
    } else {
        Serial.println(F("[WIFI] Arrêt du point d'accès..."));
        dnsServer.stop();
        server.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
    }
}

bool WebServerManager::isWiFiActive() {
    return wifiActive;
}

void WebServerManager::handleClient() {
    if (wifiActive) {
        dnsServer.processNextRequest(); // <-- NOUVEAU : Écoute DNS
        server.handleClient(); 
    }
}

void WebServerManager::handleRoot() {
    // Génération de la page HTML
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif; background:#0f172a; color:white; padding:20px;} ";
    html += "a{display:block; padding:15px; margin:10px 0; text-align:center; font-weight:bold; color:white; text-decoration:none; border-radius:8px;} ";
    html += ".btn-app{background:#0284c7; box-shadow:0 4px 6px rgba(2,132,199,0.3);} "; 
    html += ".btn-apk{background:#16a34a; box-shadow:0 4px 6px rgba(22,163,74,0.3);} ";
    html += ".btn-config{background:#f59e0b; box-shadow:0 4px 6px rgba(245,158,11,0.3);} "; 
    
    // --- NOUVEAUX STYLES FILE MANAGER ---
    html += ".upload-box{background:#1e293b; padding:15px; border-radius:8px; border:1px dashed #475569; margin-bottom:25px;}";
    html += ".file-row{display:flex; align-items:center; width:100%; margin:10px 0;} ";
    html += ".btn-csv{background:#dc2626; box-shadow:0 4px 6px rgba(220,38,38,0.3); flex-grow:1; margin:0;} "; 
    html += ".btn-del{background:#991b1b; width:60px; padding:15px 0; margin:0 0 0 10px; display:inline-block;} ";
    
    html += "</style></head><body>";
    
    html += "<h2 style='text-align:center;'>🏍️ MotoChrono OS</h2>";

    // --- 0. BOUTON DE CONFIGURATION ---
    html += "<div style='margin-bottom:25px;'>";
    html += "<a href='/config' class='btn-config'>⚙️ Configuration MOTO</a>";
    html += "</div>";

    // --- 1. UPLOAD DE FICHIERS (NOUVEAU) ---
    html += "<div class='upload-box'>";
    html += "<h3 style='margin-top:0; color:#38bdf8;'>📤 Mettre à jour l'App</h3>";
    html += "<p style='font-size:12px; color:#94a3b8; margin-bottom:15px;'>Envoyez une nouvelle version de MotoChrono.html ou .apk sur la SD.</p>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='f' style='margin-bottom:10px; color:#94a3b8; width:100%;'><br>";
    html += "<input type='submit' value='Envoyer le fichier' class='btn-app' style='width:100%; padding:15px; border:none; color:white; font-weight:bold; border-radius:8px;'>";
    html += "</form></div>";

    File root = SD.open("/");
    if (!root) {
        html += "<p>Erreur de lecture de la carte SD.</p></body></html>";
        server.send(200, "text/html", html);
        return;
    }

    // --- 2. BOUTONS D'APPLICATIONS (APK et HTML) ---
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

    // --- 3. LISTE DES FICHIERS CSV (AVEC BOUTON POUBELLE) ---
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
            html += "<div class='file-row'>";
            // Bouton de téléchargement rouge original
            html += "<a href='/download?file=/" + csv.name + "' class='btn-csv'>📄 " + csv.name + " (" + String(csv.size / 1024) + " Ko)</a>";
            // Nouveau bouton de suppression à côté
            html += "<a href='/delete?file=/" + csv.name + "' class='btn-del' onclick=\"return confirm('Supprimer " + csv.name + " definitivement ?');\">🗑️</a>";
            html += "</div>";
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

    html += "<label>Coeff. Injecteur (L/µs)</label>";
    html += "<input type='number' step='any' name='injCoeff' value='" + String(SettingsManager::injectorCoeff, 9) + "'>";

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
    if (server.hasArg("injCoeff")) SettingsManager::injectorCoeff = server.arg("injCoeff").toFloat();

    // Ordre d'écriture dans la mémoire flash (NVS)
    SettingsManager::save(); 

    // Redirection HTTP "See Other" vers l'accueil
    server.sendHeader("Location", "/");
    server.send(303);
}

// ==========================================
// == GESTION DES FICHIERS (UPLOAD / DEL)  ==
// ==========================================

void WebServerManager::handleDelete() {
    if (server.hasArg("file")) {
        String filename = server.arg("file");
        if (SD.exists(filename)) {
            SD.remove(filename);
            Serial.println("[WIFI] Fichier supprimé : " + filename);
        }
    }
    // Recharge la page d'accueil
    server.sendHeader("Location", "/");
    server.send(303);
}

void WebServerManager::handleUploadComplete() {
    // Une fois l'upload terminé, on redirige le navigateur vers l'accueil
    server.sendHeader("Location", "/");
    server.send(303);
}

void WebServerManager::handleFileUpload() {
    // Traitement du flux de données "Multipart" reçu de la page web
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;
        
        Serial.print(F("[WIFI] Début de l'Upload : ")); 
        Serial.println(filename);
        
        // On écrase le fichier s'il existe déjà
        if (SD.exists(filename)) SD.remove(filename);
        fsUploadFile = SD.open(filename, FILE_WRITE);
        
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Écriture du fichier par blocs (Evite la saturation de la RAM)
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
        
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
            Serial.print(F("[WIFI] Upload terminé ! Taille : ")); 
            Serial.print(upload.totalSize / 1024);
            Serial.println(F(" Ko"));
        } else {
            Serial.println(F("[WIFI] ERREUR d'écriture SD pendant l'upload."));
        }
    }
}
