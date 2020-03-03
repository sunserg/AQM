#pragma once

//#include "AQMonitor.h"
//#include <Wire.h>
//#include <EEPROM.h>
#include "bsec.h"

struct BME680Settings {
    int8_t temperatureOffset;  // In 0.1C
    int8_t humidityOffset;     // In mbar
    int16_t calibrationPeriod;    // 4 or 28 days
    int8_t blinkInterval;     // In s TODO: move to LED class
    uint8_t sensorCalibration[BSEC_MAX_STATE_BLOB_SIZE];
};

const char BME680_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
    <legend>BME680 sensor settings</legend>
    Temperature offset:<br>
    <input type="text" name="temp_offset" value="%d"><br>
    <small><em>in 0.1 degrees, from -125 to 125</em></small><br>
    <br>
    Humidity offset:<br>
    <input type="text" name="humidity_offset" value="%d"><br>
    <small><em>in 0.1 percents, from -125 to 125</em></small><br>
    <br>
    Calibration period:<br>
    <select name="calibration_period">
     <option value="4" %s>4 days</option>
     <option value="28" %s>28 days</option>
    </select><br>
    <small><em>Period over which the sensor will recalibrate its IAQ values (requires restart)</em></small><br>
</fieldset>
)=====";

class BME680 {
    public:
        BME680();
        void begin();
        void loop();

        float getTemp();
        float getHumidity();
        float getPressure();
        float getGasResistance();
        uint8_t getAccuracy();
        float getIAQ();
        float getStaticIAQ();

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer, bool& save);

    private:
        float _temp = 0.0f;
        float _humidity = 0.0f;
        float _pressure = 0.0f;
        float _gas_resistance = 0.0f;

        uint8_t _accuracy = 0;
        float _iaq = 0.0f;
        float _static_iaq = 0.0f;

        unsigned long _lastStateUpdate;

        Bsec _iaqSensor = Bsec();

        // Helper function definitions
        void checkIaqSensorStatus(void);
        void loadState(void);
        void updateState(void);
};

//extern AQSensors aqSensors;

