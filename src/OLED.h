#ifndef _OLED_H_
#define _OLED_H_

#include "AQMonitor.h"

#include <SNTPtime.h>
#include <Wire.h>          // Include Wire if you're using I2C
#include <SFE_MicroOLED.h> // Include the SFE_MicroOLED library

#define PIN_RESET 255      //
#define DC_JUMPER 0        // I2C Addres: 0 - 0x3C, 1 - 0x3D
#define WIFI_TIMEOUT 15000 // while trying to connect to WiFi

class OLED
{
  public:
    wl_status_t WIFI_conn; 
    OLED(BME680* sensor);
    void begin();
    void loop();

  private:
    // Use these variables to set the initial time
    int hours = 12;
    int minutes = 00;
    int seconds = 00;

    // How fast do you want the clock to spin? Set this to 1 for fun.
    // Set this to 1000 to get _about_ 1 second timing.
    const int CLOCK_SPEED = 1000;

    MicroOLED* _oled;

    // Global variables to help draw the clock face:
    int MIDDLE_Y; // = oled.getLCDHeight() / 2;
    int MIDDLE_X; // = oled.getLCDWidth() / 2;
    int CLOCK_RADIUS;
    int POS_12_X, POS_12_Y;
    int POS_3_X, POS_3_Y;
    int POS_6_X, POS_6_Y;
    int POS_9_X, POS_9_Y;
    int S_LENGTH;
    int M_LENGTH;
    int H_LENGTH;

    unsigned long lastDraw = 0;
    strDateTime dateTime;

    BME680* _sensor;

    void drawFace();
    void drawArms(int hours, int minutes, int seconds);
    void drawTimeWarning();
    void drawVcc(uint16_t);
    void drawIAQstatus(uint16_t);
    void drawTemp(float);
    void drawHum(float humidity);
    void drawIAQ(float IAQ);
    void drawClock(uint16_t vcc = NULL, int cTemp = NULL, int humidity = NULL, int pressure = NULL, int iaq = NULL);
    void initClockVariables();
    void updateTimeNet();
    void updateTime();
};

#endif
