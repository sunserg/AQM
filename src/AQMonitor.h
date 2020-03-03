#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#include "user_interface.h"

#include "esp8266-base.h"


#include "Settings.h"

extern Logger logger;
extern Settings settings;
extern WiFiManager wifi;

#include "BME680.h"
#include "DataCollector.h"
#include "LED.h"
#include "OLED.h"

extern BME680 bme680;
//extern SensairS8 co2;
//extern PlantowerPMSx003 pm;
extern DataCollector dataCollector;

#define HTTP_PORT 80
#define HOSTNAME "aq-monitor"

#define I2C_SCL D1
#define I2C_SDA D2
#define RED D5
#define GREEN D7
#define BLUE D6

#define MIN_TO_MILLIS(mins) (mins * 60L * 1000L)
#define BME680_SAVE_STATE_PERIOD MIN_TO_MILLIS(4L*60L)  // Save the BSEC state each 4 hours.

// struct SettingsData {
//     NetworkSettings network;
//     struct AQSensor {
//         uint8_t sensorCalibration[BSEC_MAX_STATE_BLOB_SIZE];
//         // Actual correction = value / 10. I.e. value of -23 is -2.3C. Same apply for humidity.
//         int16_t temperatureOffset;
//         int16_t humidityOffset;
//         uint16_t calibrationPeriod;
//     } aqSensor;

//     struct LED {
//         uint16_t blinkInterval;
//     } led;

//     struct InfluxDBCollectorSettings influxDB;
// };

// extern Logger logger;
// extern Settings settings;
// extern SettingsData settingsData;
// extern WiFiManager wifi;
// extern SystemCheck systemCheck;
// extern InfluxDBCollector telemetryCollector;

// #endif
