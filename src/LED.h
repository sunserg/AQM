#pragma once

//#include "AQMonitor.h"

struct LEDSettings
{
    int8_t blinkInterval;     // In s  
};

const char LED_CONFIG_PAGE[] PROGMEM = R"=====(
<fieldset style='display: inline-block; width: 300px'>
    <legend>LED settings</legend>
    Blink interval:<br>
    <input type="text" name="blink_interval" value="%d"><br>
    <small><em>in seconds, 0 to disable blinking, max value - 3600</em></small><br>
</fieldset>
)=====";

class LED {
    public:
        void begin();
        void loop();
        void set(uint8_t red, uint8_t green, uint8_t blue);
        void blink(float iaq);

        void get_config_page(char* buffer);
        void parse_config_params(WebServerBase* webServer, bool& save);

    private:
        unsigned long _lastBlink = 0;
        unsigned long _lastUpdate = 0;
        uint8_t _red = 0, _targetRed = 0;
        uint8_t _green = 0, _targetGreen = 0;
        uint8_t _blue = 0, _targetBlue = 0;
};

extern LED led;
