# 🏍️ Roadmap d'Améliorations : Projet MotoChrono
**Cible :** Télémétrie ESP32-S3 pour Honda CBR 929 RR (SC44)

---

## 🛠️ 1. Matériel & Électronique ("Zéro Risque")
*Objectif : Fiabiliser l'électronique face à l'environnement piste (vibrations, surtensions, intempéries).*

- [ ] **Protection Alimentation (Load Dumps) :** Ajouter une diode TVS (Transient Voltage Suppressor) bidirectionnelle en parallèle de l'entrée du convertisseur Step-Down 12V/5V.
- [ ] **Sécurisation du Signal Injecteur :**
  - Remplacer la diode 1N4007 par une diode de commutation rapide (ex: 1N4148).
  - Ajouter une résistance de pull-down côté ESP32 pour garantir un signal propre et éviter les rebonds.
  Valeur de résistance recommandée : 1 kΩ (ou 2.2 kΩ maximum)
  Tolérance : 1% ou 5% (classique)
  Puissance : 1/4 W (standard)
  [Côté sortie Optocoupleur - Broches 3 et 4]

````Broche 4 (Collecteur) ----------------------> +3.3V (Alimentation propre de l'ESP32)

Broche 3 (Émetteur)   -----+----------------> GPIO 1 de l'ESP32 (Broche d'interruption)
                           |
                           Z
                           Z  Résistance de Pull-Down (1 kΩ)
                           Z
                           |
                           +----------------> GND (Masse de l'ESP32)
````
- [ ] **Fiabilisation Mécanique (Track-Ready) :**
  - Appliquer du vernis tropicalisant (Conformal Coating) sur le PCB (attention à masquer le baromètre/IMU et les connecteurs).
  - Monter le boîtier complet sur silent blocs (caoutchouc ou mousse haute densité) pour absorber les vibrations moteur (11 000 tr/min).


---

## 💻 2. Architecture Firmware C++ (Modulaire & FreeRTOS)
*Objectif : Éviter les pertes de données et optimiser la précision du chronométrage.*

- [ ] **Ségrégation des Cœurs (Dual-Core) :**
  - **Core 0 (Temps Réel) :** Affecter les ISR (Injecteur), la lecture UART GPS (10 Hz via DMA) et la lecture I2C de l'IMU QMI8658 (Filtre de Madgwick).
  - **Core 1 (Tâches Lourdes) :** Affecter l'affichage SPI (ST7789) et l'écriture sur la carte MicroSD.
- [ ] **Communication Inter-Tâches :**
  - Mettre en place des *Ring Buffers* ou des *FreeRTOS Queues* pour passer les paquets de données du Core 0 vers le Core 1.
  - Implémenter l'écriture SD par blocs (Chunking) pour éviter les blocages du processeur.
- [ ] **Précision Microseconde :**
  - Modifier l'interruption (ISR) de lecture du temps d'ouverture injecteur pour utiliser `micros()` au lieu de `millis()`.

---

## 📊 3. Télémétrie & Outils Logiciels
*Objectif : Faciliter l'exploitation des données dans le paddock et affiner l'analyse.*

- [ ] **Mode "Paddock" (Wi-Fi Local) :**
  - Configurer un appui long sur le bouton IP67 pour basculer l'ESP32 en mode Point d'Accès (AP) Wi-Fi.
  - Créer un mini-serveur web asynchrone sur l'ESP32 permettant de lister et télécharger les fichiers CSV directement sur un smartphone (sans sortir la SD).
- [ ] **Outil d'Analyse (MotoChrono.html / JS) :**
  - Implémenter la formule de *Haversine* pour calculer les distances GPS.
  - Ajouter une fonctionnalité "Auto-Split" : définir une ligne d'arrivée virtuelle (coordonnées GPS) pour découper automatiquement le CSV en tours et générer un tableau des chronos.