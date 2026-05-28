# ⚡ Schéma Électrique Complet : MotoChrono

**Cible :** Télémétrie ESP32-S3 pour Honda CBR 929 RR (SC44)

````
====================================================================================
                        SCHEMA D'ARCHITECTURE LOGIQUE : MOTOCHRONO
====================================================================================

[ ZONE MOTO 12V (BRUITÉE) ]       [ BARRIÈRE D'ISOLATION ]         [ ZONE ESP32-S3 (3.3V PROPRE) ]

(+12V Après Contact) ---------+-----> [ REGULATEUR 12V->5V ] --(+5V)-----> [ VIN / 5V ESP32 ]
                              |                     |
                           +--+--+                  |
                           |  |  | TVS Bi-dir.      |
                           | -|- | (Symétrique:     |
                           |  |  |  pas de sens)    |
                           +--+--+                  |
                              |                     |
(Masse Moto / Cadre) ---------+---------------------+-----(Masse Commune)----> [ GND ESP32 ]


                                  [ CIRCUIT OPTOCOUPLEUR ]

                               Anode (A)   Cathode (K)
                                  |          | (Trait gris sur le composant)
                                  V          V
(Signal Masse ECU) ---------------|>|---------------- [R 1.2kΩ] ---> (Broche 1)_______(Broche 4) <---(+3.3V)- [ 3.3V ESP32 ]
                              [Diode UF4007]                         |        [ PC817 ]        |
(Masse Moto / Cadre) ----------------------------------------------> (Broche 2)_______(Broche 3) ---+-------> [ GPIO 1 (ISR) ]
                                                                                                    |
                                                                                             [R 1kΩ Pull-Down]
                                                                                                    |
                                                                                                  (GND)

====================================================================================
                               PÉRIPHÉRIQUES CONNECTÉS
====================================================================================

[ IMU QMI8658 (I2C) ]                   [ GPS BN-220 10Hz (UART) ]
  SDA (Data)  <-------> GPIO 8            TX (Data Out) <--------> GPIO 43 (RX ESP)
  SCL (Clock) <-------> GPIO 9            RX (Data In)  <--------> GPIO 44 (TX ESP)

[ ECRAN ST7789 (SPI) ]                  [ LECTEUR MICRO-SD (SPI) ]
  SCL (Clock) <-------> GPIO 12           SCK (Clock)   <-------> GPIO 12
  SDA (MOSI)  <-------> GPIO 11           MOSI (Data In)<-------> GPIO 11
  RES (Reset) <-------> GPIO 10           MISO (Data Out)<------> GPIO 14
  DC (Data/C) <-------> GPIO 13           CS (Chip Sel.)<-------> GPIO 15
  BLK (Rétro) <-------> 3.3V

[ BOUTON IP67 (PADDOCK/CHRONO) ]
  Contact 1   <-------> GPIO 0 (INPUT_PULLUP interne)
  Contact 2   <-------> GND ESP32

====================================================================================
````

## 1. Alimentation Principale (Réseau 12V Moto -> 5V/3.3V)

*Objectif : Isoler l'électronique des variations de l'alternateur et des pics de tension (Load Dumps).*

* **Entrée Moto :** +12V (Après contact) et Masse (GND) du faisceau.

* **Protection Surtension :** Diode TVS (bidirectionnelle) placée en parallèle entre le +12V et le GND avant le régulateur.

* **Régulateur :** Module Step-Down (Buck Converter) acceptant au moins 24V en entrée.

  * *IN+* : +12V Moto (Après diode TVS)

  * *IN-* : Masse Moto (GND)

  * *OUT+* : +5V vers ESP32 (Broche 5V/VIN)

  * *OUT-* : Masse commune vers ESP32 (GND)

## 2. Acquisition Signal Injecteur (Isolation Galvanique)

*Objectif : Lecture du temps d'ouverture (Pulse Width) de l'injecteur sans aucun contact électrique direct avec l'ESP32.*

### A. Côté Moto (Haute Tension / Bruit)

* **Connexion :** Piquage sur le fil de commande de masse d'un injecteur (le fil commuté par l'ECU).

* **Composants en série :**

  1. **Diode de protection (Flyback) :** `UF4007` (Anode côté faisceau moto, Cathode vers résistance). Bloque les pics de tension inverse ultra-rapides.

  2. **Résistance de limitation :** `1.2 kΩ` (1/2 Watt).

* **Vers Optocoupleur (PC817) :**

  * Sortie résistance -> `Broche 1` (Anode de la LED interne).

  * Masse Moto -> `Broche 2` (Cathode de la LED interne).

### B. Côté ESP32 (Basse Tension 3.3V)

* **Depuis Optocoupleur (PC817) :**

  * `Broche 4` (Collecteur) -> Connectée au `+3.3V` de l'ESP32.

  * `Broche 3` (Émetteur) -> Connectée au `GPIO 1` de l'ESP32 (Broche d'interruption).

* **Traitement du Signal :**

  * **Résistance Pull-Down :** `1 kΩ` (1/4 Watt) placée entre le `GPIO 1` et le `GND` de l'ESP32. Force le signal à 0V instantanément à la fermeture de l'injecteur.

## 3. Périphériques I2C (Centrale Inertielle - IMU)

*Composant : QMI8658 (Calcul d'inclinaison via Filtre de Madgwick).*

* **VCC :** +3.3V de l'ESP32

* **GND :** Masse de l'ESP32

* **SDA (Data) :** `GPIO 8`

* **SCL (Clock) :** `GPIO 9`

## 4. Périphériques UART (GPS)

*Composant : Module GPS 10Hz (ex: BN-220).*

* **VCC :** +3.3V ou +5V (Selon tolérance du modèle GPS)

* **GND :** Masse de l'ESP32

* **TX (GPS) :** -> `GPIO 43` (RX UART ESP32 - Lecture temps réel)

* **RX (GPS) :** -> `GPIO 44` (TX UART ESP32 - Configuration module)

## 5. Périphériques SPI (Écran & Lecteur SD)

*Composants : Écran LCD IPS ST7789 et Lecteur Carte MicroSD (Partage du bus SPI).*

### A. Écran LCD IPS ST7789

* **VCC :** +3.3V de l'ESP32

* **GND :** Masse de l'ESP32

* **SCL (Clock) :** `GPIO 12` (SCK SPI - Partagé)

* **SDA (MOSI) :** `GPIO 11` (MOSI SPI - Partagé)

* **RES (Reset) :** `GPIO 10`

* **DC (Data/Command) :** `GPIO 13`

* **BLK (Rétroéclairage) :** `+3.3V` (ou GPIO libre pour contrôle PWM)

### B. Lecteur Carte MicroSD

* **VCC :** +3.3V ou +5V (Selon régulateur intégré au module)

* **GND :** Masse de l'ESP32

* **SCK (Clock) :** `GPIO 12` (Partagé avec l'écran)

* **MOSI (Data IN) :** `GPIO 11` (Partagé avec l'écran)

* **MISO (Data OUT) :** `GPIO 14` (MISO SPI - Dédié à la SD car l'écran n'envoie pas de données)

* **CS (Chip Select) :** `GPIO 15` (Dédié à l'activation de la carte SD)

## 6. Bouton Utilisateur (Contrôle Paddock / Chrono)

*Composant : Bouton poussoir étanche IP67 au guidon.*

* **Connexion 1 :** `GPIO 0` (Nécessite l'activation de `INPUT_PULLUP` dans le code).

* **Connexion 2 :** Masse (GND) de l'ESP32.

* *Comportement :* État HAUT (3.3V) au repos, état BAS (0V) lors de l'appui.