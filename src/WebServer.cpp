#include "AQMonitor.h"

WebServer::WebServer(int port) {

    _server = new ESP8266WebServer(port);
    _server->on("/", std::bind(&WebServer::handle_root, this));
    _server->on("/get", std::bind(&WebServer::handle_get, this));
    _server->on("/settings", std::bind(&WebServer::handle_settings, this));
    _server->on("/reset", std::bind(&WebServer::handle_reset, this));
    _server->on("/hard-reset", std::bind(&WebServer::handle_hard_reset, this));
    _server->on("/blink", std::bind(&WebServer::handle_blink, this));

    _httpUpdater = new ESP8266HTTPUpdateServer(true);
    _httpUpdater->setup(_server);

    MDNS.begin(HOSTNAME);
    MDNS.addService("http", "tcp", 80);

}

void WebServer::begin() {
    _server->begin();
}

void WebServer::loop() {
    _server->handleClient();
}

void WebServer::handle_root() {
    systemCheck.registerWebCall();
    _server->sendHeader("Location","/settings");
    _server->send(303);
}

void WebServer::handle_get() {
    systemCheck.registerWebCall();

    char resp[strlen_P(GET_JSON) + 32];
    sprintf_P(resp,
              GET_JSON,
              aqSensors.getTemp(),
              aqSensors.getHumidity(),
              aqSensors.getPressure(),
              aqSensors.getIAQ(),
              aqSensors.getIAQAccuracy(),
              aqSensors.getGasResistance());
    _server->send(200, "application/json", resp);
}

void WebServer::handle_settings() {
    systemCheck.registerWebCall();

    bool save = false;

    if (_server->hasArg("hostname")) {
        String new_hostname = _server->arg("plain");
        unsigned int max_hostname_length = sizeof(settings.get()->hostname) - 1;

        if (new_hostname.length() > 2 && new_hostname.length() < max_hostname_length) {
            bool valid = true;
            for (unsigned int i = 0; i < new_hostname.length(); i++) {
                char ch = new_hostname.charAt(i);
                if (!isalnum(ch) && ch != '-') {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                strcpy(settings.get()->hostname, new_hostname.c_str());
                save = true;
            }
        }
    }

    if (_server->hasArg("temp_offset")) {
        int val = _server->arg("temp_offset").toInt();
        if (-125 <= val && val <= 125) {
            settings.get()->temperatureOffset = val;
            save = true;
        }
    }

    if (_server->hasArg("humidity_offset")) {
        int val = _server->arg("humidity_offset").toInt();
        if (-125 <= val && val <= 125) {
            settings.get()->humidityOffset = val;
            save = true;
        }
    }

    if (save) {
        settings.save();        
    }

    char resp[strlen_P(CONFIG_PAGE) + 128];
    sprintf_P(
        resp,
        CONFIG_PAGE,
        settings.get()->hostname, 
        settings.get()->temperatureOffset, 
        settings.get()->humidityOffset);
    _server->send(200, "text/html", resp);
}

void WebServer::handle_reset() {
    systemCheck.registerWebCall();
    _server->send(200);
    delay(1000);
    ESP.reset();
}

void WebServer::handle_hard_reset() {
    systemCheck.registerWebCall();
    settings.erase();
    _server->send(200, "text/plain", "Calibration data erased.");
}

void WebServer::handle_blink() {
    systemCheck.registerWebCall();
    led.set(255, 0, 0);
    delay(500);
    led.set(0, 255, 0);
    delay(500);
    led.set(0, 0, 255);
    delay(500);
    led.set(0, 0, 0);

    _server->send(200, "text/plain", "Just blinked!");
}

WebServer webServer = WebServer(HTTP_PORT);
