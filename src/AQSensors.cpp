#include "AQMonitor.h"

AQSensors::AQSensors() {
    _lastRefresh = 0;
    _lastStateUpdate = 0;
}

void AQSensors::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);

    // Init the sensor
    _iaqSensor.begin(BME680_I2C_ADDR_SECONDARY, Wire);

    _output = "BSEC library version " +
        String(_iaqSensor.version.major) +
        "." +
        String(_iaqSensor.version.minor) +
        "." + String(_iaqSensor.version.major_bugfix) +
        "." + String(_iaqSensor.version.minor_bugfix);
    Serial.println(_output);

    // As per the documentation - 'The default configuration (after calling bsec_init), to which
    // BSEC will be configured, is "generic_18v_300s_4d"'. However this firmware is for PCB that
    // works on 3.3V, so - the default config is overridden with the generic_33v_300s_4d one.
    const uint8_t generic_33v_300s_4d[BSEC_MAX_PROPERTY_BLOB_SIZE] = {
        0,6,4,1,61,0,0,0,0,0,0,0,24,1,0,0,40,0,1,0,137,65,0,63,0,0,64,63,205,204,76,62,0,0,225,68,
        0,192,168,71,0,0,0,0,0,80,10,90,0,0,0,0,0,0,0,0,21,0,2,0,0,244,1,225,0,25,10,144,1,0,0,112,
        65,0,0,0,63,16,0,3,0,10,215,163,60,10,215,35,59,10,215,35,59,9,0,5,0,0,0,0,0,1,51,0,9,0,
        10,215,163,59,205,204,204,61,225,122,148,62,41,92,15,61,0,0,0,63,0,0,0,63,154,153,89,63,
        154,153,25,62,1,1,0,0,128,63,6,236,81,184,61,51,51,131,64,12,0,10,0,0,0,0,0,0,0,0,0,131,0,
        254,0,2,1,5,48,117,100,0,44,1,151,7,132,3,197,0,144,1,64,1,64,1,48,117,48,117,48,117,48,
        117,100,0,100,0,100,0,48,117,48,117,48,117,100,0,100,0,48,117,100,0,100,0,100,0,100,0,48,
        117,48,117,48,117,100,0,100,0,100,0,48,117,48,117,100,0,44,1,44,1,44,1,44,1,44,1,44,1,44,1,
        44,1,44,1,44,1,44,1,44,1,44,1,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,44,1,0,0,0,0,98,149,0,0
    };
    _iaqSensor.setConfig(generic_33v_300s_4d);

    checkIaqSensorStatus();
    loadState();

    bsec_virtual_sensor_t sensorList[7] = {
        BSEC_OUTPUT_RAW_TEMPERATURE,
        BSEC_OUTPUT_RAW_PRESSURE,
        BSEC_OUTPUT_RAW_HUMIDITY,
        BSEC_OUTPUT_RAW_GAS,
        BSEC_OUTPUT_IAQ_ESTIMATE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
        BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY
    };

    _iaqSensor.updateSubscription(sensorList, 7, BSEC_SAMPLE_RATE_LP);
    checkIaqSensorStatus();
}

void AQSensors::loop() {
    // Sensor shows the temperature of the chip itself and this is usually above ambient. An offset
    // can be configured to compensate for this. The offset depends on the PCB layout.
    _iaqSensor.setTemperatureOffset(settings.get()->temperatureOffset * -0.1f);

    bool newDataAvailable = _iaqSensor.run();
    if (millis() - _lastRefresh > 1000 * 10) {
        if (!newDataAvailable) {
            // Sensor is running. Get its status and try next time.
            checkIaqSensorStatus();
        } else {
            if (_lastRefresh == 0) {
                // Print the header
                _output = "sec,\tr_temp,\tpressure,\tr_hum,\tgas (Ohm),\tIAQ,\t\tacc,\ttemp,\thumidity";
                Serial.println(_output);
            }

            _lastRefresh = millis();

            _temp = _iaqSensor.temperature;
            _humidity = _iaqSensor.humidity + (settings.get()->humidityOffset * 0.1f);
            _pressure = _iaqSensor.pressure;
            _iaq = _iaqSensor.iaqEstimate;
            _iaq_accuracy = _iaqSensor.iaqAccuracy;
            _gas_resistance = _iaqSensor.gasResistance;

            // Some calculation with fixed values for good and bad air quality sensor resistance.
            // Bad sensor resistance should be the value for AQ=250, good for AQ=25. Calculations
            // are using linear interpolation to get the AQ value.
            float dRper1AQ = 1000 * (settings.get()->goodAQResistance - settings.get()->badAQResistance) / 225.0f;
            if (_gas_resistance - 1000 * settings.get()->badAQResistance == 0) {
                _calculated_iaq = 250.0f;
            } else {
                _calculated_iaq = 250.0f - (_gas_resistance - 1000 * settings.get()->badAQResistance) / dRper1AQ;
            }
            _calculated_iaq = min(_calculated_iaq, 500.0f);
            _calculated_iaq = max(_calculated_iaq, 0.0f);

            _output = String(millis()/1000);
            _output += ",\t" + String(_iaqSensor.rawTemperature);
            _output += ",\t" + String(_iaqSensor.pressure);
            _output += ",\t" + String(_iaqSensor.rawHumidity);
            _output += ",\t" + String(_iaqSensor.gasResistance);
            _output += ",\t" + String(_iaqSensor.iaqEstimate);
            _output += ",\t\t" + String(_iaqSensor.iaqAccuracy);
            _output += ",\t" + String(_iaqSensor.temperature);
            _output += ",\t" + String(_iaqSensor.humidity);

            Serial.println(_output);

            updateState();
        }
    }
}

float AQSensors::getIAQAccuracy() {
    return _iaq_accuracy;
}

float AQSensors::getIAQ() {
    return _iaq;
}

float AQSensors::getHumidity() {
    return _humidity;
}

float AQSensors::getTemp() {
    return _temp;
}

float AQSensors::getCalculatedIAQ() {
    return _calculated_iaq;
}

float AQSensors::getPressure() {
    return _pressure;
}

float AQSensors::getGasResistance() {
    return _gas_resistance;
}

// Helper function definitions
void AQSensors::checkIaqSensorStatus(void)
{
    if (_iaqSensor.status != BSEC_OK) {
        if (_iaqSensor.status < BSEC_OK) {
            _output = "BSEC error code : " + String(_iaqSensor.status);
            Serial.println(_output);
            for (;;) {
                Serial.println(_output);
                delay(1000);
            }
        } else {
            _output = "BSEC warning code : " + String(_iaqSensor.status);
            Serial.println(_output);
        }
    }

    if (_iaqSensor.bme680Status != BME680_OK) {
        if (_iaqSensor.bme680Status < BME680_OK) {
            _output = "BME680 error code : " + String(_iaqSensor.bme680Status);
            Serial.println(_output);
            for (;;) {
                Serial.println(_output);
                delay(1000);
            }
        } else {
            _output = "BME680 warning code : " + String(_iaqSensor.bme680Status);
            Serial.println(_output);
        }
    }
}

void AQSensors::loadState(void)
{
    uint8_t *state = settings.get()->sensorCalibration;
    for (int i = 0; i < BSEC_MAX_STATE_BLOB_SIZE; i++) {
        // Check if we have non-zero calibration byte. If we do - the calibraiton
        // data is valid and can be used.
        if (state[i] != 0) {
            _iaqSensor.setState(settings.get()->sensorCalibration);
            checkIaqSensorStatus();
            Serial.println("Settings applied.");
            break;
        }
    }
}

void AQSensors::updateState(void)
{
    bool update = false;
    if (_lastStateUpdate == 0) {
        if (_iaqSensor.iaqAccuracy >= 3) {
            update = true;
        }
    } else {
        unsigned long timeSinceLastStateUpdate = millis() - _lastStateUpdate;
        if (timeSinceLastStateUpdate > BME680_SAVE_STATE_PERIOD) {
            update = true;
        }
    }

    if (update) {
        _lastStateUpdate = millis();
        _iaqSensor.getState(settings.get()->sensorCalibration);
        checkIaqSensorStatus();
        settings.save();
    }
}

AQSensors aqSensors = AQSensors();
