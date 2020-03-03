#include "OLED.h"

SNTPtime NTPch("nl.pool.ntp.org");

OLED::OLED(BME680 *sensor)
{
	_sensor = sensor;
}

void OLED::begin()
{
	// create the OLED
	_oled = new MicroOLED(PIN_RESET, DC_JUMPER); // I2C Example

	MIDDLE_Y = _oled->getLCDHeight() / 2;
	MIDDLE_X = _oled->getLCDWidth() / 2;
	Serial.println(MIDDLE_X);
	Serial.println(MIDDLE_Y);
	_oled->begin(); // Initialize the OLED
	initClockVariables();
	drawClock();
	//	drawClock(3300, _sensor->getTemp(), _sensor->getHumidity(), _sensor->getPressure(), _sensor->getIAQ());
}

void OLED::loop()
{
	// Check if we need to update seconds, minutes, hours:
	if (lastDraw + CLOCK_SPEED < millis())
	{
		lastDraw = millis();
		// Add a second, update minutes/hours if necessary:
		if (WiFi.status() == WL_CONNECTED)
		{
			updateTimeNet();
		}
		else
		{
			updateTime();
		}

		drawClock(_sensor->getAccuracy(), _sensor->getTemp(), _sensor->getHumidity(), _sensor->getPressure(), _sensor->getStaticIAQ());
	}
}

// Simple function to increment seconds and then increment minutes and hours if necessary.
void OLED::updateTime()
{
	seconds++;		   // Increment seconds
	if (seconds >= 60) // If seconds overflows (>=60)
	{
		seconds = 0;	   // Set seconds back to 0
		minutes++;		   // Increment minutes
		if (minutes >= 60) // If minutes overflows (>=60)
		{
			minutes = 0;	 // Set minutes back to 0
			hours++;		 // Increment hours
			if (hours >= 12) // If hours overflows (>=12)
			{
				hours = 0; // Set hours back to 0
			}
		}
	}
}

void OLED::updateTimeNet()
{
	if (WIFI_conn != WiFi.status())
	{
		if (WiFi.status() == WL_CONNECTED)
		{
			Serial.println("WiFi connected");
			while (!NTPch.setSNTPtime())
				Serial.print("!"); // set internal clock
			Serial.println();
			Serial.println("Time set");
		}
		else
		{
			Serial.println("WiFi NOT connected. Time NOT set");
		}
		WIFI_conn = WiFi.status();
	}
	dateTime = NTPch.getTime(1.0, 1); // get time from internal clock
	hours = dateTime.hour;
	minutes = dateTime.minute;
	seconds = dateTime.second;
}

void OLED::drawClock(uint16_t vcc, int cTemp, int humidity, int pressure, int iaq)
{
	// Draw the clock:
	_oled->clear(PAGE);				   // Clear the buffer
	drawFace();						   // Draw the face to the buffer
	drawArms(hours, minutes, seconds); // Draw arms to the buffer
	if (vcc != NULL)
		drawIAQstatus(vcc);
	if (cTemp != NULL)
		drawTemp(cTemp);
	if (humidity != NULL)
		drawHum(humidity);
	if (iaq != NULL)
		drawIAQ(iaq);
	// if (WiFi.status() != WL_CONNECTED)
	// {
	// 	drawTimeWarning();
	// }
	_oled->display(); // Draw the memory buffer
}

void OLED::initClockVariables()
{
	// Calculate constants for clock face component positions:
	_oled->setFontType(0);
	if (MIDDLE_X > MIDDLE_Y)
		CLOCK_RADIUS = MIDDLE_Y;
	else
		CLOCK_RADIUS = MIDDLE_X;
	CLOCK_RADIUS = CLOCK_RADIUS - 1;
	POS_12_X = MIDDLE_X - _oled->getFontWidth();
	POS_12_Y = MIDDLE_Y - CLOCK_RADIUS + 2;
	POS_3_X = MIDDLE_X + CLOCK_RADIUS - _oled->getFontWidth() - 1;
	POS_3_Y = MIDDLE_Y - _oled->getFontHeight() / 2;
	POS_6_X = MIDDLE_X - _oled->getFontWidth() / 2;
	POS_6_Y = MIDDLE_Y + CLOCK_RADIUS - _oled->getFontHeight() - 1;
	POS_9_X = MIDDLE_X - CLOCK_RADIUS + _oled->getFontWidth() - 2;
	POS_9_Y = MIDDLE_Y - _oled->getFontHeight() / 2;

	// Calculate clock arm lengths
	S_LENGTH = CLOCK_RADIUS - 2;
	M_LENGTH = S_LENGTH * 0.7;
	H_LENGTH = S_LENGTH * 0.5;
}

// Draw the clock's three arms: seconds, minutes, hours.
void OLED::drawArms(int h, int m, int s)
{
	double midHours; // this will be used to slightly adjust the hour hand
	static int hx, hy, mx, my, sx, sy;

	// Adjust time to shift display 90 degrees ccw
	// this will turn the clock the same direction as text:
	h -= 3;
	m -= 15;
	s -= 15;
	if (h <= 0)
		h += 12;
	if (m < 0)
		m += 60;
	if (s < 0)
		s += 60;

	// Calculate and draw new lines:
	s = map(s, 0, 60, 0, 360);					// map the 0-60, to "360 degrees"
	sx = S_LENGTH * cos(PI * ((float)s) / 180); // woo trig!
	sy = S_LENGTH * sin(PI * ((float)s) / 180); // woo trig!
	// draw the second hand:
	_oled->line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + sx, MIDDLE_Y + sy);

	m = map(m, 0, 60, 0, 360);					// map the 0-60, to "360 degrees"
	mx = M_LENGTH * cos(PI * ((float)m) / 180); // woo trig!
	my = M_LENGTH * sin(PI * ((float)m) / 180); // woo trig!
	// draw the minute hand
	_oled->line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + mx, MIDDLE_Y + my);

	midHours = minutes / 12;					// midHours is used to set the hours hand to middling levels between whole hours
	h *= 5;										// Get hours and midhours to the same scale
	h += midHours;								// add hours and midhours
	h = map(h, 0, 60, 0, 360);					// map the 0-60, to "360 degrees"
	hx = H_LENGTH * cos(PI * ((float)h) / 180); // woo trig!
	hy = H_LENGTH * sin(PI * ((float)h) / 180); // woo trig!
	// draw the hour hand:
	_oled->line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + hx, MIDDLE_Y + hy);
}

// Draw an analog clock face
void OLED::drawFace()
{
	// Draw the clock border
	_oled->circle(MIDDLE_X, MIDDLE_Y, CLOCK_RADIUS);

	// Draw the clock numbers
	_oled->setFontType(0);				  // set font type 0, please see declaration in SFE_MicroOLED.cpp
	_oled->setCursor(POS_12_X, POS_12_Y); // points cursor to x=27 y=0
	_oled->print(12);
	_oled->setCursor(POS_6_X, POS_6_Y);
	_oled->print(6);
	_oled->setCursor(POS_9_X, POS_9_Y);
	_oled->print(9);
	_oled->setCursor(POS_3_X, POS_3_Y);
	_oled->print(3);
}

// Draw a warning on the clock face
void OLED::drawTimeWarning()
{
	_oled->setFontType(0);								   // set font type 0, please see declaration in SFE_MicroOLED.cpp
	_oled->setCursor(0, POS_6_Y - _oled->getFontHeight()); // points cursor to x=27 y=0
	_oled->print("wrong time");
}

void OLED::drawVcc(uint16_t vcc)
{
	_oled->setCursor(0, _oled->getLCDHeight() - _oled->getFontHeight() + 1);
	_oled->print(vcc / 1000.);
}

void OLED::drawIAQstatus(uint16_t status)
{
	_oled->setCursor(0, _oled->getLCDHeight() - _oled->getFontHeight() + 1);
	_oled->print("IAQ"); _oled->print(status);
}

void OLED::drawTemp(float c_temp)
{
	int t = c_temp;
	_oled->setCursor(0, 0);
	_oled->print(t);
	_oled->print("C");
}

void OLED::drawHum(float humidity)
{
	int h = humidity;
	_oled->setCursor(_oled->getLCDWidth() - 4 * _oled->getFontWidth(), 0);
	_oled->print(h);
	_oled->print("%");
}

void OLED::drawIAQ(float IAQ)
{
	int iaq = IAQ;
	_oled->setCursor(_oled->getLCDWidth() - 4 * _oled->getFontWidth(), _oled->getLCDHeight() - _oled->getFontHeight() + 1);
	_oled->print(iaq);
}
