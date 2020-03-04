#include "AQMonitor.h"

char buffer[4096];

WebServer::WebServer(Logger* logger, NetworkSettings* networkSettings)
    :WebServerBase(networkSettings, logger, NULL) {
}

void WebServer::registerHandlers() {
    server->on("/", std::bind(&WebServer::handle_root, this));
    server->on("/get", std::bind(&WebServer::handle_get, this));
    server->on("/settings", std::bind(&WebServer::handle_settings, this));
    server->on("/reset", std::bind(&WebServer::handle_reset, this));
    server->on("/blink", std::bind(&WebServer::handle_blink, this));
    server->on("/stats", std::bind(&WebServer::handle_stats, this));
}

void WebServer::handle_root() {
    // systemCheck->registerWebCall();
    server->sendHeader("Location","/settings");
    server->send(303);
}

void WebServer::handle_get() {
    // systemCheck->registerWebCall();

    sprintf_P(buffer,
              GET_JSON,
              tempSensor.getTemp(),
              tempSensor.getHumidity(),
              tempSensor.getPressure(),
              tempSensor.getGasResistance(),
              tempSensor.getAccuracy(),
              tempSensor.getIAQ(),
              tempSensor.getStaticIAQ(),
              ESP.getFreeHeap());
    server->send(200, "application/json", buffer);
}

void WebServer::handle_settings() {
    // systemCheck->registerWebCall();

    bool save = false;

    wifi.parse_config_params(this, save);
    dataCollector.parse_config_params(this, save);
    tempSensor.parse_config_params(this, save);
    led.parse_config_params(this, save);

    if (save) {
        settings.save();
    }

    char network_settings[strlen_P(NETWORK_CONFIG_PAGE) + 32];
    wifi.get_config_page(network_settings);

    char data_collector_settings[strlen_P(INFLUXDB_CONFIG_PAGE) + 96];
    dataCollector.get_config_page(data_collector_settings);

    char temp_sensor_settings[strlen_P(BME680_CONFIG_PAGE) + 16];
    tempSensor.get_config_page(temp_sensor_settings);

    char led_settings[strlen_P(LED_CONFIG_PAGE) + 16];
    led.get_config_page(led_settings);

    sprintf_P(
        buffer,
        CONFIG_PAGE,
        network_settings,
        data_collector_settings,
        temp_sensor_settings,
        led_settings);
    server->send(200, "text/html", buffer);
}

void WebServer::handle_reset() {
    // systemCheck->registerWebCall();
    if (server->hasArg("full")) {
        // Erase absolutely all settings, even WiFi credentials.
        settings.erase();
    } else {
        // Erase only the sensor calibration data.
        memset(
            settings.getSettings()->bme680.sensorCalibration,
            0,
            sizeof(settings.getSettings()->bme680.sensorCalibration));
    }
    server->send(200, "text/plain", "Calibration data erased.");
}

void WebServer::handle_blink() {
//    systemCheck->registerWebCall();

    if (server->hasArg("iaq")) {
        float iaq = server->arg("iaq").toFloat();
        led.blink(iaq);
        server->send(200, "text/plain", "Just blinked!");
    } else {
        server->send(400, "text/plain", "Missing iaq argument!");
    }
}

void WebServer::handle_stats() {
    sprintf_P(buffer,
              PSTR("Uptime: %lus. Free heap: %u"),
              millis()/1000,
              ESP.getFreeHeap());
    server->send(200, "text/plain", buffer);
}
