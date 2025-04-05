#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <SdFat.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Define the SD card chip select pin
#define SD_CS_PIN 10
#define GREEN_LED 7
#define RED_LED 8

// Initialize INA219 sensors with their respective addresses
Adafruit_INA219 ina2axis(0x40);
Adafruit_INA219 ina1axis(0x41);
Adafruit_INA219 inaTilted(0x44);
Adafruit_INA219 inaFlat(0x45);

// Create an SD card object
SdFat sd;
SdFile logFile;
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD I2C address 0x27, 20x4 display

unsigned long lastSDWrite = 0; // Track last SD card write time
const unsigned long SDWriteInterval = 900000; // 15 minutes in milliseconds
char logFileName[13]; // Filename format: solarYYMMDD.csv

void setup() {
    Serial.begin(115200);
    Serial.println("INA219_4sensor_SD_RTC_LED_20x4_15min_chat.ino");
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();  //CLM added
    lcd.setCursor(0, 0);  //CLM added
    lcd.print("   2ax  1ax  tlt flt"); //CLM added
    delay(1000);  //CLM added

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);

    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("RTC not found!");
        digitalWrite(RED_LED, HIGH);
        while (1);
    }

    // Ensure RTC has the correct time
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize all INA219 sensors
    ina2axis.begin();
    ina1axis.begin();
    inaTilted.begin();
    inaFlat.begin();

    // Set calibration for higher precision
    ina2axis.setCalibration_16V_400mA();
    ina1axis.setCalibration_16V_400mA();
    inaTilted.setCalibration_16V_400mA();
    inaFlat.setCalibration_16V_400mA();

    // Initialize SD card
    if (!sd.begin(SD_CS_PIN, SPI_HALF_SPEED)) {
        Serial.println("SD card initialization failed!");
        digitalWrite(RED_LED, HIGH);
        return;
    }

    // Generate logfile name using RTC date
    DateTime now = rtc.now();
    snprintf(logFileName, sizeof(logFileName), "solar%02d%02d%02d.csv", now.year() % 100, now.month(), now.day());

    // Open file to write headers
    if (!logFile.open(logFileName, O_WRITE | O_CREAT | O_AT_END)) {
        Serial.println("File open failed");
        digitalWrite(RED_LED, HIGH);
        return;
    }

    // Write headers if file is new
    if (logFile.fileSize() == 0) {
        logFile.println("yyyy,mo,day,hr,min,power_mW_2Ax,power_mW_1Ax,power_mW_Tilt,power_mW_Flat,busVoltage2Ax,busVoltage1Ax,busVoltageTilt,busVoltageFlat,current2Ax,current1Ax,currentTilt,currentFlat");
    }
    logFile.close();
    digitalWrite(GREEN_LED, HIGH); // Indicate successful setup
}

void loop() {
    DateTime now = rtc.now();

    // Read sensor values
    float power_mW_2Ax = ina2axis.getPower_mW();
    float power_mW_1Ax = ina1axis.getPower_mW();
    float power_mW_Tilt = inaTilted.getPower_mW();
    float power_mW_Flat = inaFlat.getPower_mW();

    float busVoltage2Ax = ina2axis.getBusVoltage_V();
    float busVoltage1Ax = ina1axis.getBusVoltage_V();
    float busVoltageTilt = inaTilted.getBusVoltage_V();
    float busVoltageFlat = inaFlat.getBusVoltage_V();

    float current2Ax = ina2axis.getCurrent_mA();
    float current1Ax = ina1axis.getCurrent_mA();
    float currentTilt = inaTilted.getCurrent_mA();
    float currentFlat = inaFlat.getCurrent_mA();

    // Print to Serial Monitor every 3 seconds
    Serial.print(now.year()); Serial.print(",");
    Serial.print(now.month()); Serial.print(",");
    Serial.print(now.day()); Serial.print(",");
    Serial.print(now.hour()); Serial.print(",");
    Serial.print(now.minute()); Serial.print(",");
    Serial.print(power_mW_2Ax); Serial.print(",");
    Serial.print(power_mW_1Ax); Serial.print(",");
    Serial.print(power_mW_Tilt); Serial.print(",");
    Serial.print(power_mW_Flat); Serial.print(",");
    Serial.print(busVoltage2Ax); Serial.print(",");
    Serial.print(busVoltage1Ax); Serial.print(",");
    Serial.print(busVoltageTilt); Serial.print(",");
    Serial.print(busVoltageFlat); Serial.print(",");
    Serial.print(current2Ax); Serial.print(",");
    Serial.print(current1Ax); Serial.print(",");
    Serial.print(currentTilt); Serial.print(",");
    Serial.println(currentFlat);

    // Update LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   2ax  1ax  tlt flt");
    lcd.setCursor(0, 1);
    lcd.print("mW "); 
    lcd.print(power_mW_2Ax,1); lcd.print(" ");
    lcd.print(power_mW_1Ax,1); lcd.print(" ");
    lcd.print(power_mW_Tilt,1); lcd.print(" ");
    lcd.print(power_mW_Flat,1);
    lcd.setCursor(0, 2);
    lcd.print("V  ");
    lcd.print(busVoltage2Ax,1); lcd.print(" ");
    lcd.print(busVoltage1Ax,1); lcd.print(" ");
    lcd.print(busVoltageTilt,1); lcd.print(" ");
    lcd.print(busVoltageFlat,1);
    lcd.setCursor(0, 3);
    lcd.print("mA ");
    lcd.print(current2Ax,1); lcd.print(" ");
    lcd.print(current1Ax,1); lcd.print(" ");
    lcd.print(currentTilt,1); lcd.print(" ");
    lcd.print(currentFlat,1);

    delay(3000); // Print to Serial every 3 seconds
}

