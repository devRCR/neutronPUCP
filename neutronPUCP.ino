#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

RTC_DS3231 rtc;
const int chipSelect = 5;
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;

// Configuración de pines de interrupción
const int interruptPin1 = 34;
const int interruptPin2 = 35;
unsigned long lastHourTimestamp = 0;

void IRAM_ATTR handleInterrupt1() {
  pulseCount1++;
}

void IRAM_ATTR handleInterrupt2() {
  pulseCount2++;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Inicializar RTC
  if (!rtc.begin()) {
    Serial.println("No se pudo encontrar RTC");
    while (1);
  }

  // Comprobar si el RTC está funcionando
  if (rtc.lostPower()) {
    Serial.println("RTC perdió energía, configurando tiempo!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Inicializar tarjeta SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Fallo al inicializar la tarjeta SD");
    return;
  }

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
  if (currentTimestamp - lastHourTimestamp >= 3600) {
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
  snprintf(filename, sizeof(filename), "%04d%02d%02d.txt", now.year(), now.month(), now.day());

  // Abrir el archivo
  File dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    // Escribir el timestamp y las cuentas de pulsos
    dataFile.print("Timestamp: ");
    dataFile.print(now.timestamp());
    dataFile.print(", Pin 34 count: ");
    dataFile.print(pulseCount1);
    dataFile.print(", Pin 35 count: ");
    dataFile.println(pulseCount2);
    dataFile.close();
    Serial.println("Datos registrados");
  } else {
    Serial.println("Error al abrir el archivo");
  }
}
