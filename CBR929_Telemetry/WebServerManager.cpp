#include "WebServerManager.h"
#include <vector>
#include <algorithm> 

struct CsvFile {
    String name;
    size_t size;
};

WebServer WebServerManager::server(80);

void WebServerManager::init() {
    Serial.println(F("[WIFI] Démarrage du point d'accès MotoChrono..."));
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MotoChrono_CBR929", "piste929"); 

    Serial.print(F("[WIFI] Point d'accès créé. IP : "));
    Serial.println(WiFi.softAPIP()); 

    server.on("/", HTTP_GET, handleRoot);
    server.on("/download", HTTP_GET, handleDownload);

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
    html += ".btn-apk{background:#16a34a; box-shadow:0 4px 6px rgba(22,163,74,0.3);} "; // Vert pour Android
    html += ".btn-csv{background:#dc2626; box-shadow:0 4px 6px rgba(220,38,38,0.3);} "; 
    html += "</style></head><body>";
    html += "<h2 style='text-align:center;'>🏍️ MotoChrono AP</h2>";

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
        
        // NOUVEAU : Bouton pour l'application Android native
        if (hasApk) {
            html += "<a href='/download?file=/MotoChrono.apk' class='btn-apk'>🤖 Installer l'App (Android APK)</a>";
        }
        
        // Bouton pour la version HTML classique (PC / iOS)
        if (hasHtml) {
            html += "<a href='/download?file=/MotoChrono.html' class='btn-app'>💻 Télécharger l'App (HTML)</a>";
        }
        html += "</div>";
    }

    // --- 2. LISTE DES FICHIERS CSV (TRI DESCENDANT) ---
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
                
                // --- ROUTAGE DYNAMIQUE DES TYPES MIME ---
                String contentType = "text/plain";
                if (fileName.endsWith(".csv")) {
                    contentType = "text/csv";
                } else if (fileName.endsWith(".html")) {
                    contentType = "text/html";
                } else if (fileName.endsWith(".apk")) {
                    // C'est LE paramètre critique pour qu'Android lance l'installation de l'APK
                    contentType = "application/vnd.android.package-archive"; 
                }

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