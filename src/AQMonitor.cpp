#include "AQMonitor.h"

Logger logger = Logger(true);
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network);
//SystemCheck systemCheck = SystemCheck(&logger);
// InfluxDBCollector telemetryCollector = InfluxDBCollector(
//     &logger, &wifi, &settingsData.influxDB, &settingsData.network, collectData, shouldPush, onPush);

WebServer webServer = WebServer(&logger, &settings.getSettings()->network);

DataCollector dataCollector = DataCollector();
BME680 tempSensor = BME680();
OLED oled = OLED(&tempSensor);

ADC_MODE(ADC_VCC);

void setup()
{   
    settings.begin();
    
    wifi.begin();
    wifi.connect();
    
    webServer.begin();

    led.begin();
    tempSensor.begin();
    //systemCheck.begin();
    dataCollector.begin();
    oled.begin();
}

void loop() {
    wifi.loop();
    webServer.loop();
    tempSensor.loop();
    settings.loop();
    led.loop();
    // systemCheck.loop();
    dataCollector.loop();

    // if (settingsData.influxDB.enable) {
    //     systemCheck.stop();
    //     telemetryCollector.start();
    // } else {
    //     telemetryCollector.stop();
    //     systemCheck.start();
    // }

    // // Disconnect after the first 30 minutes even in AP mode.
    // if (millis() > 30 * 60 * 1000) {
    //     wifi.disconnect();
    // }

    oled.loop();

    delay(100);
}
