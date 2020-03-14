#include "AQMonitor.h"
#include "DataCollector.h"

DataCollector::DataCollector():
    InfluxDBCollector(&logger,
                      &wifi,
                      &settings.getSettings()->influxDB,
                      &settings.getSettings()->network) {
}

bool DataCollector::shouldCollect() {
    return tempSensor.getPressure() > 100 ||
           tempSensor.getHumidity() > 0 ||
           tempSensor.getIAQ() > 0;
}

void DataCollector::collectData() {

    if (tempSensor.getAccuracy() > 0) {
        append("iaqAccuracy", tempSensor.getAccuracy());

        lastIAQ = tempSensor.getAccuracy();
        if (lastPushedIAQaccuracy < 0) {
            lastPushedIAQaccuracy = tempSensor.getAccuracy(); 
        }
    }

    if (tempSensor.getPressure() > 100) {
        append("temperature", tempSensor.getTemp(), 2);
        append("humidity", tempSensor.getHumidity(), 1);
        append("pressure", tempSensor.getPressure(), 1);
        append("IAQ", tempSensor.getIAQ());
        append("staticIAQ", tempSensor.getStaticIAQ());
        append("gas_resistance", tempSensor.getGasResistance());
        append("free_heap", ESP.getFreeHeap());

        if (lastPushedHumidity < 0) {
            lastPushedTemp = tempSensor.getTemp();
            lastPushedHumidity = tempSensor.getHumidity();
        }
        lastTemp = tempSensor.getTemp();
        lastHumidity = tempSensor.getHumidity();
    }
}

bool DataCollector::shouldPush() {
    float IAQratio = 1.0 * lastPushedIAQaccuracy / lastIAQaccuracy;
    if (lastPushedIAQaccuracy > 0 && (IAQratio <= 0.8f || IAQratio >= 1.2f)) {
        return true;
    }

    if (lastPushedHumidity > 0) {
        if (fabsf(lastPushedTemp - lastTemp) > 1.0) {
            return true;
        }

        if (fabsf(lastPushedHumidity - lastHumidity) > 5.0) {
            return true;
        }
    }

    return false;
}   

void DataCollector::onPush() {
    lastPushedIAQaccuracy = lastIAQaccuracy;
    lastPushedIAQ = lastIAQ;
    lastPushedTemp = lastTemp;
    lastPushedHumidity = lastHumidity;
}