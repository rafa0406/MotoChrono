# Cahier des Charges - Système de Télémétrie & Tableau de Bord pour Honda CBR 929 RR (SC44)

## 1. Objectif du Projet
Développer un système embarqué autonome basé sur un microcontrôleur ESP32-S3, destiné à compenser l'absence de jauge à carburant d'origine et à ajouter des fonctionnalités de télémétrie pour un usage sur piste (trackdays). Le système s'articule autour d'une carte très intégrée (Waveshare ESP32-S3-LCD-1.3) pour limiter le câblage, maximiser la fiabilité face aux vibrations et enregistrer les données pour analyse.

## 2. Fonctionnalités Attendues

### 2.1. Gestion du Carburant (Usage Route/Piste)
* **Acquisition :** Lecture du temps d'ouverture d'un injecteur (Pulse Width) via interruption matérielle pour en déduire le volume consommé.
* **Calculs :** 
  * Consommation instantanée (L/100km).
  * Consommation moyenne (L/100km).
  * Autonomie restante estimée (km) basée sur une capacité théorique de 18L.
* **Persistance :** Sauvegarde des données de consommation en mémoire non volatile (NVS) lors de la coupure du contact (gestion du Deep Sleep).
* **Interface :** Bouton poussoir physique pour réinitialiser le volume à 18L lors du passage à la pompe.

### 2.2. Télémétrie & Dynamique (Usage Piste)
* **Vitesse & Position :** Acquisition GPS à haute fréquence (10 Hz) pour une vitesse réelle (indépendante du compteur) et le tracé des trajectoires.
* **Chronométrage :** Détection du passage d'une ligne d'arrivée virtuelle (coordonnées GPS pré-enregistrées) pour le calcul des temps au tour.
* **Dynamique :** Mesure des angles d'inclinaison (roulis) et des forces G via l'IMU QMI8658 intégré. La fusion de capteurs (filtre de Madgwick) sera calculée par le logiciel.
* **Datalogging :** Enregistrement continu (à haute fréquence) des métriques (Timestamp, Vitesse, G, Angle, GPS_X, GPS_Y, Conso) dans un fichier `.csv` sur la carte MicroSD intégrée.

### 2.3. Affichage
* Écran LCD IPS ST7789 Couleur 1.3" intégré affichant en temps réel :
  * Page 1 (Route) : Jauge % ou Litres, Conso Inst., Autonomie, alertes couleurs.
  * Page 2 (Piste) : Vitesse réelle, Temps du dernier tour, Meilleur tour.

## 3. Architecture Matérielle (Nomenclature)

| Composant | Fonction Principale | Remarques | 
| ----- | ----- | ----- | 
| **Waveshare ESP32-S3-LCD-1.3** | Carte Mère Tout-en-un | Intègre : MCU ESP32-S3, Écran IPS ST7789, IMU QMI8658 (6 axes), Slot MicroSD. | 
| **Step-Down (12V->5V)** | Alimentation isolée | Alimentation permanente depuis la batterie. | 
| **PC817 (x2)** | Optocoupleurs | Isolation galvanique Injecteur et signal "Après Contact". | 
| **1N4007** | Diode de protection | Bloque la tension de flyback de la bobine d'injecteur. | 
| **Beitian BN-220 / ATGM336H** | Module GPS | Alternative économique, éprouvée (drones FPV). Compatible 10 Hz. Connexion UART. | 
| **Bouton Poussoir** | Interface Utilisateur | Type étanche (IP67). | 
| **Divers** | Résistances (1kΩ), Condensateurs | Filtrage et pull-ups matériels. | 

## 4. Estimation des Coûts Matériels (Budget)

L'utilisation d'une carte intégrée réduit drastiquement les coûts et la complexité d'assemblage.

| Composant / Module | Prix estimé (€) | 
| ----- | ----- | 
| **Carte Waveshare ESP32-S3-LCD-1.3** | 20 à 25 € | 
| **Alimentation (Step-Down 12V/5V étanche)** | 4 à 6 € | 
| **Composants d'isolation (PC817, Diodes, Résistances)** | \~ 2 € | 
| **Module GPS (BN-220 ou ATGM336H)** | 8 à 15 € | 
| **Bouton poussoir IP67 (étanche)** | 3 à 5 € | 
| **Connectique, PCB perforé, Fils & Boîtier/Résine** | 10 à 15 € | 
| **TOTAL ESTIMÉ** | **\~ 47 € à 68 €** | 

## 5. Sécurité et Intégrité du Faisceau (Principe du "Zéro Risque")

Le montage ne causera **aucun dysfonctionnement** sur la moto :

1. **Isolation Galvanique (Électronique) :** L'optocoupleur (PC817) garantit une séparation physique. La surconsommation est dérisoire (\~12 mA) et invisible pour l'ECU Honda.
2. **Intégrité Mécanique du Câblage :**
   * **Interdiction de couper** le fil d'origine de l'injecteur.
   * **Méthode :** Dénuder proprement 5 mm de gaine, enrouler le fil de dérivation, souder à l'étain.
   * **Isolation :** Ruban adhésif d'électricien pro (ex: 3M) ou gaine thermorétractable liquide, puis fixer au faisceau d'origine (colliers Rilsan).
   * *Alternative :* Connecteurs type **Posi-Tap** exclusivement.

## 6. Schéma Fonctionnel de Câblage (Simplifié pour Waveshare)

Grâce à la carte Waveshare, tous les bus complexes (I2C, SPI) sont internes. Seules 5 broches (IO1 à IO5) sont utilisées en externe.

```text
======================= BLOC ALIMENTATION =======================
Batterie Moto (+12V) ----[ Fusible 2A ]----> IN+ (Step-Down)
Batterie Moto (GND) -----------------------> IN- (Step-Down)
                                              |
[Step-Down Sortie 5V] ---------------------> 5V (Waveshare)
[Step-Down Sortie GND] --------------------> GND (Waveshare)

======================= BLOC ACQUISITION MOTO =======================
1. Signal Injecteur (Acquisition Volume)
   +12V Injecteur ----[ R 1kΩ ]----> (Pin 1) PC817_A
                                           | (Pin 2)
   Fil Commande ECU <----[ Diode ]---------+ (Cathode vers injecteur)
   
   PC817_A (Pin 3) ----> GND
   PC817_A (Pin 4) ----> IO1 (Waveshare - Interruption)

2. Signal Contact (Gestion Deep Sleep / Sauvegarde)
   +12V Après Contact ----[ R 1kΩ ]----> (Pin 1) PC817_B
   Masse Moto -------------------------> (Pin 2) PC817_B
   
   PC817_B (Pin 3) ----> GND
   PC817_B (Pin 4) ----> IO2 (Waveshare - Wake/Sleep)

======================= CAPTEURS & INTERFACE EXTERNES =======================
1. UART (GPS)
   IO4 (Waveshare) ----------------> RX (BN-220 / ATGM336H)
   IO5 (Waveshare) ----------------> TX (BN-220 / ATGM336H)

2. Interface
   IO3 (Waveshare) <----[ Bouton ]----> GND

======================= MODULES INTERNES (Pré-câblés) =======================
* Écran LCD ST7789V2 (SPI interne)
* IMU QMI8658 6-Axes (I2C interne)
* Lecteur MicroSD (SDIO/SPI interne)