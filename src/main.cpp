
#include <BluetoothSerial.h>
//#include <BTAdvertisedDevice.h>
#include <SSD1306Wire.h>

#include <BtClassicForumsLader.h>

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"

#include <BleHeartRate.h>

BtClassicForumsLader forumslader;
BleHeartRate BLEhrm;

// SSD1306 OLED 128x64 on I2C
SSD1306Wire display(0x3c, SDA, SCL);

//8x WS2812 LED-Strip (for Heart Rate, ...) -> FastLED
#include <FastLED.h>
CRGB leds[8];


bool confirmRequestPending = true;



static const unsigned char icon_heart[] PROGMEM = {
    0b00000000, 0b00000000, //
    0b01111000, 0b00111100, //   ####   ####
    0b11111100, 0b01111110, //  ###### ######
    0b11111110, 0b11111111, // ###############
    0b11111110, 0b11111111, // ###############
    0b11111110, 0b11111111, // ###############
    0b11111110, 0b11111111, // ###############
    0b11111100, 0b01111111, //  #############
    0b11111100, 0b01111111, //  #############
    0b11111000, 0b00111111, //   ###########
    0b11110000, 0b00011111, //    #########
    0b11100000, 0b00001111, //     #######
    0b11000000, 0b00000111, //      #####
    0b10000000, 0b00000011, //       ###
    0b00000000, 0b00000001, //    	  #
	0b00000000, 0b00000000, //
};


#ifdef BT_SCAN
void btAdvertisedDeviceFound(BTAdvertisedDevice* pDevice) {
	Serial.printf("ℹ️ Found a classic BT device asynchronously: %s\n", pDevice->toString().c_str());
}
#endif



void setup()
{
  Serial.begin(115200);

  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, 5>(leds, 8);
  FastLED.setBrightness(64);

  leds[0] = CRGB::Red;
  FastLED.show();

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Startup. Connecting...");
  display.display();

  forumslader.connect();

  BLEhrm.setup();

}

void loop() {
	static uint32_t lastDisplayUpdate = 0;
	static uint16_t ani_counter = 0;
	forumslader.loop();
	BLEhrm.loop();
	yield();
	if (lastDisplayUpdate + 100 < millis()) {
		lastDisplayUpdate = millis();
		ani_counter++;
		display.clear();

		if (BLEhrm.isConnected()) display.drawXbm(16, 0, 16, 16, icon_heart);

		// Speed - TODO: Extract method
		display.setFont(ArialMT_Plain_24);
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		String speedStr;
		speedStr = String(forumslader.getSpeed(), 1);
		display.drawString(100, 40, speedStr);
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(100, 40, "km");
		display.drawHorizontalLine(100, 51, 12);
		display.drawString(105, 52, "h");

		// Batterie - TODO: Extract method
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(60, 0, String(forumslader.getVoltageTotal(), 1)+"V");
		display.drawProgressBar(99, 0, 28, 8, static_cast<uint8_t>((forumslader.getVoltageTotal() - 11.5) / (12.5-11.5) * 100));

		// Level - TODO: Extract method
		int8_t stage = forumslader.getStage();
		//Simulation: stage = (millis() / 1000) % 5;
		for (int_fast8_t c = 0; c<4; c++) {
			if (stage > c) {
				display.fillCircle(123, 59-(8*c), 4);
			} else {
				display.drawCircle(123, 59-(8*c), 4);
			}
		}
		// Gradient - TODO: Extract method
		display.setFont(ArialMT_Plain_16);
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		display.drawString(60, 16, String(forumslader.getGradient(), 1)+"%");


		// HRM
		display.setFont(ArialMT_Plain_16);
		display.drawString(60, 32, String(BLEhrm.getHrm().HRM)+"bpm");
		// write the buffer to the display
		display.display();

		uint16_t hr = BLEhrm.getHrm().HRM;
		int8_t hrm_step = (hr-45) / 9;  //FIXME: test for quick reaction at home
		Serial.printf("HRM-Step: %d\n", hrm_step);
		if (hrm_step < 0) hrm_step=0; //tot?
		if (hrm_step > 5) hrm_step=5;

		for (uint_fast8_t i = 0; i < 5 ; i++) {
			leds[7-i] = (hrm_step >= i) ? CRGB::BlueViolet : CRGB::Black;
		}
		FastLED.show();
	}
	delay(10);
}
