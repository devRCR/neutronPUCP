#include <Arduino.h>
#line 1 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
#include <Wire.h>
#include <RTClib.h>
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Define SD card connection
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCLK 18
#define SD_CS 5

File dataFile;

RTC_DS3231 rtc;
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;

// Configuración de pines de interrupción
const int interruptPin1 = 34;
const int interruptPin2 = 35;
unsigned long lastHourTimestamp = 0;

#line 34 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void setup();
#line 94 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void loop();
#line 112 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void logData(DateTime now);
#line 129 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void writeFile(fs::FS &fs, const char *path, const char *message);
#line 146 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void appendFile(fs::FS &fs, const char *path, const char *message);
#line 24 "c:\\Users\\RCHAN\\OneDrive - UNIVERSIDAD NACIONAL DE INGENIERIA\\IPEN\\Arduino\\neutronPUCP\\neutronPUCP.ino"
void IRAM_ATTR handleInterrupt1() {
  pulseCount1++;
}

void IRAM_ATTR handleInterrupt2() {
  pulseCount2++;
}

void printDirectory(File dir, int numTabs);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inicializar RTC
  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar RTC");
    while (1)
      ;
  }

  // Comprobar si el RTC está funcionando
  if (rtc.lostPower()) {
    Serial.println("RTC perdió energía, configurando tiempo!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Inicializar tarjeta SD
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;  // init failed
  }

  DateTime now = rtc.now();
  unsigned long currentTimestamp = now.unixtime();
  char filename[20];
  sprintf(filename, "/%04d%02d%02d.txt", now.year(), now.month(), now.day());
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open(filename);
  if (!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, filename, "Timestamp,Detector1 counts,Detector2 counts \r\n");
  } else {
    Serial.println("File already exists");
  }
  file.close();

  // Configurar pines de interrupción
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin1), handleInterrupt1, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), handleInterrupt2, RISING);

  // Obtener la hora actual
  lastHourTimestamp = rtc.now().unixtime();
}

void loop() {
  DateTime now = rtc.now();
  unsigned long currentTimestamp = now.unixtime();

  // Comprobar si ha pasado una hora
  if (currentTimestamp - lastHourTimestamp >= 10) {
    // Guardar los datos en la tarjeta SD
    logData(now);

    // Reiniciar los contadores
    pulseCount1 = 0;
    pulseCount2 = 0;

    // Actualizar el timestamp de la última hora
    lastHourTimestamp = currentTimestamp;
  }
}

void logData(DateTime now) {
  // Crear el nombre del archivo basado en la fecha
  char filename[20];
  sprintf(filename, "%04d%02d%02d.txt", now.year(), now.month(), now.day());

  
    dataFile.print(now.timestamp());
    dataFile.print(",");
    dataFile.print(pulseCount1);
    dataFile.print(",");
    dataFile.println(pulseCount2);
    dataFile.close();
    Serial.println("Datos registrados");

}

// Write to the SD card (DON'T MODIFY THIS FUNCTION)
void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
