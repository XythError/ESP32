/***************************************************************************
   ESP32-S3 Projekt: Display-Menü, Sensoren, SD-Logging, Webserver
   Basierend auf deiner Pinbelegung und Anforderungen.
***************************************************************************/

// ------------------------- Includes ------------------------------------
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <WebServer.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <RTClib.h> // Für DS1307
#include <DHT.h>    // Für DHT Sensor


// I2C-Bus #1 (TwoWire 1) für RTC:
TwoWire I2CRTC = TwoWire(1);

// ------------------------- Pin-Definitionen ----------------------------
#if DEBUG_SERIAL
#define SERIAL_PRINT(x) Serial.print(x)
#define SERIAL_PRINTLN(x) Serial.println(x)
#define SERIAL_PRINTF(x, ...) Serial.printf((x), ##__VA_ARGS__)
#else
#define SERIAL_PRINT(x)
#define SERIAL_PRINTLN(x)
#define SERIAL_PRINTF(x, ...)
#endif

// Display per SW-I2C auf GPIO 20/21
#define DISPLAY_SDA_PIN 20
#define DISPLAY_SCL_PIN 21

// RTC (I2C)
#define RTC_SDA_PIN 15

#define RTC_SCL_PIN 16

// Buttons
#define BTN_K0_PIN 36 // UP
#define BTN_K1_PIN 35 // DOWN
#define BTN_K2_PIN 48 // OK/ENTER
#define BTN_K3_PIN 47 // BACK/HOME

// SD-Kartenleser (SPI)
#define SD_CS_PIN 10
#define SD_MOSI_PIN 13 // ESP32 MOSI an SD MOSI
#define SD_MISO_PIN 11 // ESP32 MISO an SD MISO
#define SD_SCK_PIN 12  // ESP32 SCK an SD SCK
SPIClass spiSD(HSPI); // HSPI (oft als VSPI bezeichnet) oder FSPI. Für ESP32-S3 ist HSPI oft die zweite verfügbare.

// RGB-LED
#define RGB_R_PIN 1
#define RGB_G_PIN 2
#define RGB_B_PIN 37

// DHT Sensor
#define DHT_PIN 39
#define DHT_TYPE DHT11 // oder DHT11, DHT21, je nach deinem Sensor
// WICHTIG: Schließe VCC des DHT-Sensors an 3.3V an, um den ESP32-Datenpin zu schützen!
// ------------------------- Serial Debug Configuration ------------------
#define DEBUG_SERIAL 1 // 1 to enable Serial output, 0 to disable


// ------------------------- Globale Konstanten --------------------------
const uint32_t SERIAL_BAUD_RATE = 115200;
const uint32_t DEBOUNCE_DELAY_MS = 100; // Für Taster-Entprellung
const uint32_t SENSOR_LOG_INTERVAL_MINUTES = 60; // Wie oft auf SD loggen
const uint32_t SENSOR_READ_INTERVAL_MS = 2000;   // Wie oft Sensoren für Display lesen

// ------------------------- Objekte -------------------------------------
// Display: U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Standard I2C
// Da wir benutzerdefinierte I2C-Pins verwenden:
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, DISPLAY_SCL_PIN, DISPLAY_SDA_PIN, U8X8_PIN_NONE); // Software I2C

RTC_DS1307 rtc;
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

// ------------------------- Globale Variablen ---------------------------
// UI State
enum Page
{
    PAGE_HOME,
    PAGE_MENU,
    PAGE_WIFI_SCAN,
    PAGE_SENSORS
};
Page currentPage = PAGE_HOME;
uint8_t currentMenuIndex = 0;
const char *mainMenuItems[] = {"WiFi Scan", "File Server", "Sensors", "Reboot", "Home"};
const uint8_t MAIN_MENU_ITEM_COUNT = sizeof(mainMenuItems) / sizeof(char *);

// Taster-Status
bool btnK0_pressed = false;
bool btnK1_pressed = false;
bool btnK2_pressed = false;
bool btnK3_pressed = false;
uint32_t lastDebounceTime = 0;

// Sensorwerte
float temperature = 0.0;
float humidity = 0.0;
DateTime now;
bool rtcOk = false;
bool sdOk = false;
String sdCardType = "None";
uint64_t sdCardSizeMB = 0;
uint64_t sdUsedBytesMB = 0;
uint64_t sdTotalBytesMB = 0;


// WLAN & Webserver
bool fileServerActive = false;
String currentSsid = "Not Connected";
String currentIp = "0.0.0.0";
char *ap_ssid = "ESP32-S3-AP"; // Name des Access Points, wenn kein WLAN verbunden
char *ap_password = "password123"; // Passwort für AP (mind. 8 Zeichen, oder NULL für offen)


// Timer
uint32_t lastSensorReadMs = 0;
uint32_t lastSdLogMs = 0;

// ------------------------- Funktionsprototypen -------------------------
void setupPins();
void setupSerial();
void setupDisplay();
void setupRTC();
void setupDHT();
void setupSDCard();
void setupWiFi(); // Grundlegendes Setup, verbindet nicht automatisch
void connectToKnownWiFi(); // Versucht, sich mit gespeicherten Credentials zu verbinden (hier nicht implementiert, AP-Modus als Fallback)
void startFileServer();
void stopFileServer();
void handleHttpRoot();
void handleHttpData();
void handleHttpLogList();
void handleNotFound();
String getContentType(String filename);
bool handleFileRead(String path);


void readButtons();
void updateDisplay();
void drawHomePage();
void drawMenuPage();
void drawWifiScanPage();
void drawSensorsPage();

void performAction();
void doWiFiScan();
void toggleFileServer();
void rebootESP();
void showSensors();
void goHome();

void readSensors();
void logDataToSD();
void setRGB(int r, int g, int b);
String getFormattedTimestamp(DateTime dt);

// ------------------------- Setup ---------------------------------------
void setup()
{
    setupSerial();
    SERIAL_PRINTLN(F("\n[SETUP] ESP32-S3 Project Initializing..."));

    // ① Display läuft per SW-I2C über U8g2, kein Hardware-Wire mehr nötig
    // ② RTC auf separatem Hardware-Bus initialisieren:
    I2CRTC.begin(RTC_SDA_PIN, RTC_SCL_PIN);
    rtc.begin(&I2CRTC);

    setupPins();
    setupDisplay();
    setRGB(255, 0, 255); // Lila beim Booten
    delay(100);

    setupRTC();
    setupDHT();
    setupSDCard();
    setupWiFi(); // Initialisiert WiFi-Modus

    // Testweise Verbindung oder AP-Start (hier nicht implementiert, Server startet AP bei Bedarf)
    // connectToKnownWiFi(); // Implementiere dies, wenn du dich mit einem existierenden WLAN verbinden willst

    SERIAL_PRINTLN(F("[SETUP] Initialization complete."));
    setRGB(0, 0, 0); // LED aus
    lastSdLogMs = millis(); // Für den ersten Log-Eintrag
}

// ------------------------- Hauptschleife -------------------------------
void loop()
{
    uint32_t currentTimeMs = millis();

    readButtons();
    performAction(); // Aktionen basierend auf Tastendrücken ausführen

    if (currentTimeMs - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorReadMs = currentTimeMs;
        readSensors();
    }

    if (sdOk && (currentTimeMs - lastSdLogMs >= (SENSOR_LOG_INTERVAL_MINUTES * 60 * 1000UL)))
    {
        lastSdLogMs = currentTimeMs;
        logDataToSD();
    }

    updateDisplay(); // Display regelmäßig aktualisieren

    if (fileServerActive)
    {
        server.handleClient();
        setRGB(0,255,0); // Grün wenn Server aktiv
    } else {
        // Andere LED-Status hier, z.B. wenn WiFi verbunden aber Server aus
        if (WiFi.status() == WL_CONNECTED) setRGB(0,0,255); // Blau wenn WiFi verbunden
        else setRGB(0,0,0); // Aus
    }
     if (!sdOk) setRGB(255,255,0); // Gelb bei SD-Fehler
}

// ------------------------- Initialisierungsfunktionen ------------------
void setupSerial()
{
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 2000); // Warte kurz auf Serial
}

void setupPins()
{
    SERIAL_PRINTLN(F("[SETUP] Configuring GPIO pins..."));
    pinMode(BTN_K0_PIN, INPUT_PULLUP);
    pinMode(BTN_K1_PIN, INPUT_PULLUP);
    pinMode(BTN_K2_PIN, INPUT_PULLUP);
    pinMode(BTN_K3_PIN, INPUT_PULLUP);

    pinMode(RGB_R_PIN, OUTPUT);
    pinMode(RGB_G_PIN, OUTPUT);
    pinMode(RGB_B_PIN, OUTPUT);
    setRGB(0, 0, 0); // LEDs aus
}

void setupDisplay()
{
    SERIAL_PRINTLN(F("[SETUP] Initializing Display..."));
    display.begin();
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 10, "Booting...");
    display.sendBuffer();
}

void setupRTC()
{
    SERIAL_PRINTLN(F("[SETUP] Initializing RTC DS1307..."));
    if (!rtc.begin())
    {
        SERIAL_PRINTLN(F("[ERROR] Couldn't find RTC!"));
        display.setCursor(0, 22); display.print("RTC Error!"); display.sendBuffer();
        rtcOk = false;
        delay(1000);
    } else {
        rtcOk = true;
        SERIAL_PRINTLN(F("[SETUP] RTC OK."));
        if (!rtc.isrunning()) { // Prüfe, ob die Uhr läuft
            SERIAL_PRINTLN(F("[RTC] RTC is NOT running, setting time to compile time..."));
            // Wenn die Zeit verloren gegangen ist (z.B. Batterie leer/nicht vorhanden),
            // oder die Uhr nie gestellt wurde, stelle sie auf die Kompilierzeit ein.
            // HINWEIS: Dies funktioniert nur, wenn der Sketch neu kompiliert und hochgeladen wird.
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            // Alternativ: rtc.adjust(DateTime(2024, 1, 1, 0, 0, 0)); // Manuell setzen
        }
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Für Testzwecke immer neu setzen
        now = rtc.now();
        SERIAL_PRINT(F("[RTC] Current time: "));
        SERIAL_PRINTLN(getFormattedTimestamp(now));
    }
}

void setupDHT()
{
    SERIAL_PRINTLN(F("[SETUP] Initializing DHT Sensor..."));
    dht.begin();
    // Erster Lesevorgang zum Test
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity) || isnan(temperature)) {
        SERIAL_PRINTLN(F("[ERROR] Failed to read from DHT sensor!"));
        display.setCursor(0, 34); display.print("DHT Error!"); display.sendBuffer();
        delay(1000);
    } else {
        SERIAL_PRINTLN(F("[SETUP] DHT OK."));
    }
}

void setupSDCard() {
    SERIAL_PRINTLN(F("[SETUP] Initializing SD Card..."));
    display.setCursor(0, 46); display.print("SD Init..."); display.sendBuffer();

    spiSD.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, -1); // CS wird von SD.begin() gesteuert

    if (!SD.begin(SD_CS_PIN, spiSD)) {
        SERIAL_PRINTLN(F("[ERROR] SD Card Mount Failed! Check wiring, card."));
        display.setCursor(0, 46); display.print("SD Error!"); display.sendBuffer();
        sdOk = false;
        delay(1000);
        return;
    }
    sdOk = true;
    SERIAL_PRINTLN(F("[SETUP] SD Card initialized."));
    display.setCursor(0, 46); display.print("SD OK!"); display.sendBuffer();

    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE){
        sdCardType = "None";
    } else if(cardType == CARD_MMC){
        sdCardType = "MMC";
    } else if(cardType == CARD_SD){
        sdCardType = "SDSC";
    } else if(cardType == CARD_SDHC){
        sdCardType = "SDHC";
    } else {
        sdCardType = "UNKNOWN";
    }
    SERIAL_PRINT(F("[SD] Card Type: ")); SERIAL_PRINTLN(sdCardType);

    sdCardSizeMB = SD.cardSize() / (1024 * 1024);
    sdTotalBytesMB = SD.totalBytes() / (1024 * 1024);
    sdUsedBytesMB = SD.usedBytes() / (1024 * 1024);

    SERIAL_PRINTF("[SD] Size: %lluMB, Total: %lluMB, Used: %lluMB\n", sdCardSizeMB, sdTotalBytesMB, sdUsedBytesMB);
    delay(500);
}

void setupWiFi() {
    SERIAL_PRINTLN(F("[SETUP] Initializing Wi-Fi..."));
    WiFi.mode(WIFI_STA); // Start im Station-Modus
    WiFi.disconnect(true, true); // Alte Verbindungen trennen
    SERIAL_PRINTLN(F("[SETUP] Wi-Fi STA mode set."));
    // Kein automatischer Verbindungsaufbau hier, das passiert bei Bedarf
}


// ------------------------- Button & UI Logic -------------------------
void readButtons() {
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        // Liest den invertierten Zustand, da INPUT_PULLUP (LOW = gedrückt)
        btnK0_pressed = !digitalRead(BTN_K0_PIN); // UP
        btnK1_pressed = !digitalRead(BTN_K1_PIN); // DOWN
        btnK2_pressed = !digitalRead(BTN_K2_PIN); // OK
        btnK3_pressed = !digitalRead(BTN_K3_PIN); // BACK/HOME

        if (btnK0_pressed || btnK1_pressed || btnK2_pressed || btnK3_pressed) {
            lastDebounceTime = millis(); // Debounce Timer zurücksetzen
        }
    }
}

void performAction() {
    if (currentPage == PAGE_HOME) {
        if (btnK2_pressed) { // OK -> Menü öffnen
            currentPage = PAGE_MENU;
            currentMenuIndex = 0;
            btnK2_pressed = false; // Reset button state
            SERIAL_PRINTLN(F("[UI] Home: OK -> Menu"));
        }
    } else if (currentPage == PAGE_MENU) {
        if (btnK0_pressed) { // UP
            if (currentMenuIndex == 0) currentMenuIndex = MAIN_MENU_ITEM_COUNT - 1;
            else currentMenuIndex--;
            btnK0_pressed = false;
            SERIAL_PRINTF("[UI] Menu: UP -> Index %d\n", currentMenuIndex);
        } else if (btnK1_pressed) { // DOWN
            currentMenuIndex = (currentMenuIndex + 1) % MAIN_MENU_ITEM_COUNT;
            btnK1_pressed = false;
            SERIAL_PRINTF("[UI] Menu: DOWN -> Index %d\n", currentMenuIndex);
        } else if (btnK3_pressed) { // BACK -> Home
            goHome();
            btnK3_pressed = false;
            SERIAL_PRINTLN(F("[UI] Menu: BACK -> Home"));
        } else if (btnK2_pressed) { // OK -> Aktion ausführen
            SERIAL_PRINTF("[UI] Menu: OK on '%s'\n", mainMenuItems[currentMenuIndex]);
            switch (currentMenuIndex) {
                case 0: doWiFiScan(); break;
                case 1: toggleFileServer(); break;
                case 2: showSensors(); break;
                case 3: rebootESP(); break;
                case 4: goHome(); break;
            }
            btnK2_pressed = false;
        }
    } else if (currentPage == PAGE_WIFI_SCAN) {
        if (btnK3_pressed || btnK2_pressed) { // BACK oder OK -> Zurück zum Menü
            currentPage = PAGE_MENU;
            btnK3_pressed = false; btnK2_pressed = false;
            SERIAL_PRINTLN(F("[UI] WiFi Scan: BACK/OK -> Menu"));
        }
    } else if (currentPage == PAGE_SENSORS) {
         if (btnK3_pressed || btnK2_pressed) { // BACK oder OK -> Zurück zum Menü
            currentPage = PAGE_MENU;
            btnK3_pressed = false; btnK2_pressed = false;
            SERIAL_PRINTLN(F("[UI] Sensors: BACK/OK -> Menu"));
        }
    }
    // Reset button flags if not used or after use in a specific page
    btnK0_pressed = btnK1_pressed = btnK2_pressed = btnK3_pressed = false;
}


void updateDisplay() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tf); // Standardfont

    switch (currentPage) {
        case PAGE_HOME:
            drawHomePage();
            break;
        case PAGE_MENU:
            drawMenuPage();
            break;
        case PAGE_WIFI_SCAN:
            drawWifiScanPage();
            break;
        case PAGE_SENSORS:
            drawSensorsPage();
            break;
    }
    display.sendBuffer();
}

void drawHomePage() {
    char buf[40];
    display.setFont(u8g2_font_helvR08_tr); // Kleinerer Font für mehr Infos

    // WLAN Status
    if (WiFi.status() == WL_CONNECTED) {
        currentSsid = WiFi.SSID();
        currentIp = WiFi.localIP().toString();
    } else if (fileServerActive && WiFi.getMode() == WIFI_AP){ // Wenn AP Modus für Server
        currentSsid = ap_ssid;
        currentIp = WiFi.softAPIP().toString();
    }
     else {
        currentSsid = "N/A";
        currentIp = "0.0.0.0";
    }
    snprintf(buf, sizeof(buf), "WiFi: %s", currentSsid.substring(0,15).c_str());
    display.drawStr(0, 10, buf);
    snprintf(buf, sizeof(buf), "IP: %s", currentIp.c_str());
    display.drawStr(0, 20, buf);

    // Zeit und Datum
    if (rtcOk) {
        now = rtc.now();
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d  %02d/%02d/%02d",
                 now.hour(), now.minute(), now.second(),
                 now.day(), now.month(), now.year() % 100);
    } else {
        strcpy(buf, "RTC Error");
    }
    display.drawStr(0, 30, buf);

    // Temp & Feuchte
    snprintf(buf, sizeof(buf), "Temp: %.1fC  Hum: %.0f%%", temperature, humidity);
    display.drawStr(0, 40, buf);

    // SD Info
    if (sdOk) {
        sdUsedBytesMB = SD.usedBytes() / (1024 * 1024); // Aktualisiere bei Bedarf
        sdTotalBytesMB = SD.totalBytes() / (1024*1024);
        snprintf(buf, sizeof(buf), "SD: %llu/%lluMB %s", sdUsedBytesMB, sdTotalBytesMB, sdCardType.c_str());
    } else {
        strcpy(buf, "SD: Error");
    }
    display.drawStr(0, 50, buf);

    display.setFont(u8g2_font_5x8_tf);
    display.drawStr(0, 62, "K2 fuer Menue");
}

void drawMenuPage() {
    display.setFont(u8g2_font_6x12_tf);
    for (uint8_t i = 0; i < MAIN_MENU_ITEM_COUNT; ++i) {
        if (i == currentMenuIndex) {
            display.setDrawColor(1);
            display.drawBox(0, i * 12, display.getDisplayWidth(), 12);
            display.setDrawColor(0); // Text auf Box
        } else {
            display.setDrawColor(1);
        }
        display.drawStr(2, i * 12 + 10, mainMenuItems[i]);
    }
    display.setDrawColor(1); // Farbe zurücksetzen
}

void drawWifiScanPage() {
    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(0, 10, "WiFi Scan Results:");
    int16_t n = WiFi.scanComplete(); // Get status of scan
    if (n == WIFI_SCAN_FAILED) {
        display.drawStr(0, 22, "Scan Failed!");
    } else if (n == WIFI_SCAN_RUNNING) {
        display.drawStr(0, 22, "Scanning...");
    } else if (n > 0) {
        int yPos = 22;
        for (int i = 0; i < n && i < 4; ++i) { // Zeige max 4 Ergebnisse
            char buf[50];
            snprintf(buf, sizeof(buf), "%s (%d)", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            display.drawStr(0, yPos, buf);
            yPos += 10;
        }
        if (n > 4) display.drawStr(0, yPos, "More...");
    } else {
        display.drawStr(0, 22, "No networks found.");
    }
     display.setFont(u8g2_font_5x8_tf);
    display.drawStr(0, 62, "K2/K3 zurueck");
}

void drawSensorsPage() {
    display.setFont(u8g2_font_7x13B_tr); // Größerer Font
    char buf[30];

    snprintf(buf, sizeof(buf), "Temp: %.1f C", temperature);
    display.drawStr(5, 20, buf);

    snprintf(buf, sizeof(buf), "Hum:  %.0f %%", humidity);
    display.drawStr(5, 40, buf);

    display.setFont(u8g2_font_5x8_tf);
    display.drawStr(0, 62, "K2/K3 zurueck");
}

// ------------------------- Menu Actions --------------------------------
void doWiFiScan() {
    SERIAL_PRINTLN(F("[ACTION] Starting WiFi Scan..."));
    currentPage = PAGE_WIFI_SCAN;
    WiFi.disconnect(); // Sicherstellen, dass nicht verbunden für Scan
    WiFi.mode(WIFI_STA); // In STA Modus wechseln falls AP aktiv war
    WiFi.scanNetworks(true, false, false, 300); // Async scan, show hidden, passive, max 300ms per channel
    // Ergebnis wird in drawWifiScanPage() angezeigt
}

void toggleFileServer() {
    if (fileServerActive) {
        SERIAL_PRINTLN(F("[ACTION] Stopping File Server..."));
        stopFileServer();
        fileServerActive = false;
        currentSsid = "N/A"; // Zurücksetzen
        currentIp = "0.0.0.0";
        setRGB(0,0,0);
    } else {
        SERIAL_PRINTLN(F("[ACTION] Starting File Server..."));
        startFileServer(); // Startet AP Modus
        fileServerActive = true;
        currentSsid = ap_ssid; // Im AP Modus
        currentIp = WiFi.softAPIP().toString();
        setRGB(0,255,0);
    }
    currentPage = PAGE_HOME; // Zurück zum Home Screen, um IP etc. zu zeigen
}

void rebootESP() {
    SERIAL_PRINTLN(F("[ACTION] Rebooting ESP..."));
    display.clearBuffer();
    display.setFont(u8g2_font_6x12_tf);
    display.drawStr(10,30,"Rebooting...");
    display.sendBuffer();
    delay(1000);
    ESP.restart();
}

void showSensors() {
    currentPage = PAGE_SENSORS;
}

void goHome() {
    currentPage = PAGE_HOME;
}

// ------------------------- Sensor & SD Functions -----------------------
void readSensors() {
    if (rtcOk) {
        now = rtc.now(); // Zeit regelmäßig lesen
    }

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
        humidity = h;
        temperature = t;
        SERIAL_PRINTF("[SENSOR] Temp: %.1fC, Hum: %.0f%%\n", temperature, humidity);
    } else {
        SERIAL_PRINTLN(F("[ERROR] Failed to read from DHT sensor in loop."));
    }
}

String getFormattedTimestamp(DateTime dt) {
    char buf[25];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
             dt.year(), dt.month(), dt.day(),
             dt.hour(), dt.minute(), dt.second());
    return String(buf);
}

void logDataToSD() {
    if (!sdOk) {
        SERIAL_PRINTLN(F("[SD_LOG] SD Card not ready, skipping log."));
        return;
    }

    now = rtc.now(); // Aktuelle Zeit für den Logeintrag holen
    String timestamp = getFormattedTimestamp(now);
    String dataLine = timestamp + "," + String(temperature, 1) + "," + String(humidity, 0);

    char filename[20];
    snprintf(filename, sizeof(filename), "/log_%04d%02d%02d.csv", now.year(), now.month(), now.day());

    SERIAL_PRINTF("[SD_LOG] Logging to %s: %s\n", filename, dataLine.c_str());

    File dataFile = SD.open(filename, FILE_APPEND);
    if (dataFile) {
        if (dataFile.size() == 0) { // Neue Datei, Header schreiben
            SERIAL_PRINTLN(F("[SD_LOG] Writing CSV Header."));
            dataFile.println("Timestamp,Temperature,Humidity");
        }
        dataFile.println(dataLine);
        dataFile.close();
        SERIAL_PRINTLN(F("[SD_LOG] Log successful."));
    } else {
        SERIAL_PRINTF("[ERROR] Failed to open %s for appending.\n", filename);
        sdOk = false; // Evtl. SD-Problem
        setRGB(255,165,0); // Orange bei Log Fehler
    }
}


// ------------------------- RGB LED Control -----------------------------
void setRGB(int r, int g, int b) {
    // Annahme: Common Anode RGB LED, d.h. LOW = an, HIGH = aus
    // Wenn Common Cathode: Logik umkehren (analogWrite(pin, val))
    // Hier für common Cathode implementiert, da P1 an GND
    analogWrite(RGB_R_PIN, r);
    analogWrite(RGB_G_PIN, g);
    analogWrite(RGB_B_PIN, b);
}

// ------------------------- Webserver Functions -------------------------
void startFileServer() {
    SERIAL_PRINTLN(F("[WEB] Starting File Server in AP Mode..."));
    WiFi.softAP(ap_ssid, ap_password);
    IPAddress myIP = WiFi.softAPIP();
    SERIAL_PRINT(F("[WEB] AP IP address: "));
    SERIAL_PRINTLN(myIP);

    server.on("/", HTTP_GET, handleHttpLogList); // Liste der Log-Dateien
    server.on("/data.json", HTTP_GET, handleHttpData); // Aktuelle Sensorwerte als JSON
    // Route, um Dateien von SD zu servieren
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            handleNotFound();
        }
    });

    server.begin();
    SERIAL_PRINTLN(F("[WEB] HTTP server started."));
}

void stopFileServer() {
    SERIAL_PRINTLN(F("[WEB] Stopping File Server..."));
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA); // Zurück in STA Modus
    SERIAL_PRINTLN(F("[WEB] Server stopped, AP mode turned off."));
}

void handleHttpRoot() { // Einfache Info-Seite
    String html = "<html><head><title>ESP32-S3 Server</title>";
    html += "<style>body{font-family: Arial, sans-serif; margin: 20px;} table{border-collapse: collapse;} th, td{padding: 8px; border: 1px solid #ddd; text-align: left;}</style>";
    html += "</head><body><h1>ESP32-S3 Sensor Station</h1>";
    html += "<h2>Current Status</h2>";
    html += "<table><tr><th>Parameter</th><th>Value</th></tr>";
    html += "<tr><td>Time</td><td>" + getFormattedTimestamp(rtc.now()) + "</td></tr>";
    html += "<tr><td>Temperature</td><td>" + String(temperature, 1) + " °C</td></tr>";
    html += "<tr><td>Humidity</td><td>" + String(humidity, 0) + " %</td></tr>";
    if (sdOk) {
        html += "<tr><td>SD Card</td><td>OK (" + sdCardType + ", " + String(SD.usedBytes()/(1024*1024)) + "/" + String(SD.totalBytes()/(1024*1024)) + " MB)</td></tr>";
    } else {
        html += "<tr><td>SD Card</td><td>Error</td></tr>";
    }
    html += "</table>";
    html += "<h2><a href=\"/logs\">View Log Files</a></h2>";
    html += "<h2><a href=\"/data.json\">Sensor Data (JSON)</a></h2>";
    // Hier könnte ein Link zu einer Seite mit Graphen sein, wenn implementiert
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleHttpLogList() {
    if (!sdOk) {
        server.send(500, "text/plain", "SD Card Error");
        return;
    }
    String html = "<html><head><title>Log Files</title>";
    html += "<style>body{font-family: Arial, sans-serif; margin: 20px;} li{margin-bottom: 5px;}</style></head>";
    html += "<body><h1>Log Files on SD Card</h1><ul>";

    File root = SD.open("/");
    if (!root) {
        server.send(500, "text/plain", "Could not open SD root.");
        return;
    }
    if(!root.isDirectory()){
        server.send(500, "text/plain", "SD root is not a directory.");
        root.close();
        return;
    }

    File file = root.openNextFile();
    bool filesFound = false;
    while(file){
        if(!file.isDirectory() && String(file.name()).endsWith(".csv")){
            filesFound = true;
            html += "<li><a href=\"";
            html += file.name(); // Dateiname mit führendem Slash, falls von SD so geliefert
            html += "\">";
            html += String(file.name()).substring(1); // Ohne Slash für Anzeige
            html += "</a> (";
            html += String(file.size());
            html += " bytes)</li>";
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();

    if (!filesFound) {
        html += "<p>No .csv log files found.</p>";
    }
    html += "</ul><p><a href=\"/\">Back to Home</a></p></body></html>";
    server.send(200, "text/html", html);
}


void handleHttpData() { // Liefert aktuelle Sensordaten als JSON
    String json = "{";
    json += "\"timestamp\":\"" + getFormattedTimestamp(rtc.now()) + "\",";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 0);
    // Hier könnten auch die letzten X Werte von der SD-Karte gelesen und als Array angehängt werden.
    json += "}";
    server.send(200, "application/json", json);
}


String getContentType(String filename) {
    if (server.hasArg("download")) return "application/octet-stream";
    else if (filename.endsWith(".htm")) return "text/html";
    else if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".png")) return "image/png";
    else if (filename.endsWith(".gif")) return "image/gif";
    else if (filename.endsWith(".jpg")) return "image/jpeg";
    else if (filename.endsWith(".ico")) return "image/x-icon";
    else if (filename.endsWith(".xml")) return "text/xml";
    else if (filename.endsWith(".pdf")) return "application/pdf";
    else if (filename.endsWith(".zip")) return "application/zip";
    else if (filename.endsWith(".csv")) return "text/csv";
    return "text/plain";
}

bool handleFileRead(String path) { // Liest Datei von SD und sendet sie
    SERIAL_PRINTLN("handleFileRead: " + path);
    if (path.endsWith("/")) path += "index.htm"; // Standardseite (nicht verwendet hier)
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SD.exists(pathWithGz) || SD.exists(path)) {
        if (SD.exists(pathWithGz)) path += ".gz"; // Bevorzuge .gz wenn vorhanden (nicht implementiert)

        File file = SD.open(path, "r");
        if (!file) {
             SERIAL_PRINTLN("Failed to open file for reading: " + path);
            return false;
        }
        server.streamFile(file, contentType);
        file.close();
        SERIAL_PRINTLN(String("Sent file: ") + path);
        return true;
    }
    SERIAL_PRINTLN(String("File Not Found: ") + path);
    return false;
}


void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}