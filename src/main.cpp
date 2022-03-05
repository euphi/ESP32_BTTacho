//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"

#include <Arduino.h>

#include <BtClassicForumsLader.h>
BtClassicForumsLader forumslader;


#include <BleHeartRate.h>
BleHeartRate BLEhrm;

// SSD1306 OLED 128x64 on I2C
#include <SSD1306Wire.h>
SSD1306Wire display(0x3c, SDA, SCL);

//8x WS2812 LED-Strip (for Heart Rate, ...) -> FastLED
#include <FastLED.h>
CRGB leds[8];
CRGB powerledsteps[8];
CRGB battledsteps[8];

// Touch input
#include <esp32_touch.hpp>
ESP32Touch touchpins;
bool touch[4] = {false, false, false, false};

#include <FS.h>
#include <SPIFFS.h>

// Icons TODO: Use own class/modules for graphics
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

const struct {
	uint8_t max;
	CRGB leds[5];
} hr_leds[] = { { 70, {CRGB::Turquoise, CRGB::Black,     CRGB::Black,   CRGB::Black,   CRGB::Black  }},
		        { 80, {CRGB::Turquoise, CRGB::Turquoise, CRGB::Black,   CRGB::Black,   CRGB::Black  }},
				{ 80, {CRGB::Turquoise, CRGB::Turquoise, CRGB::Black,   CRGB::Black,   CRGB::Black  }},
				{ 90, {CRGB::Green, CRGB::Green, CRGB::Black,   CRGB::Black,   CRGB::Black  }},
				{ 100, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Black,   CRGB::Black  }},
				{ 110, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::GreenYellow,   CRGB::Black  }},
				{ 120, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Yellow,   CRGB::Black  }},
				{ 130, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Yellow,   CRGB::Black  }},
				{ 150, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Yellow,   CRGB::GreenYellow  }},
				{ 160, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Yellow,   CRGB::Yellow  }},
				{ 170, {CRGB::Green, CRGB::Green, CRGB::GreenYellow,   CRGB::Orange,   CRGB::Orange  }},
				{255, {CRGB::GreenYellow, CRGB::GreenYellow, CRGB::Orange,  CRGB::Orange,  CRGB::Red    }}
};

void setup()
{
  Serial.begin(115200);

  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, 5>(leds, 8);
  FastLED.setBrightness(64);
  FastLED.setCorrection(TypicalSMD5050);

  fill_gradient_RGB(powerledsteps, 0, CRGB::Blue, 7, CRGB::Green);
  fill_gradient_RGB(battledsteps, 0, CRGB::Red, 7, CRGB::Green);
  for (uint_fast8_t i=0; i<8; i++) leds[i] = powerledsteps[i];
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

//  LittleFS.begin(true);
//  Serial.printf("LittleFS: %d of %d\n", LittleFS.usedBytes(), LittleFS.totalBytes());
  SPIFFS.begin(true);
  Serial.printf("LittleFS: %d of %d\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());


  //TOuch
  //touchSetCycles(0x2000, 0x2000);

  // Rechts
  touchpins.configure_input(4, 95,  []() {
      Serial.println("Rechts - T4");
      touch[0] = true;
  });

  // Oben
  touchpins.configure_input(5, 95, []() {
      Serial.println("Oben - T5");
      touch[1] = true;
  });

  // Links
  touchpins.configure_input(6, 95, []() {
      Serial.println("Links - T6");
      touch[2] = true;
  });

  // Unten
  touchpins.configure_input(7, 95, []() {
      Serial.println("Unten - T7");
      touch[3] = true;
  });

  touchpins.begin();
}

void loop() {
	static uint32_t lastDisplayUpdate = 0;
	static uint16_t ani_counter = 0;
	static uint16_t touch_ani[4] = {0, 0, 0, 0};
	forumslader.loop();
	BLEhrm.loop();
	yield();
	if (lastDisplayUpdate + 100 < millis()) {
		lastDisplayUpdate = millis();
		ani_counter++;
		display.clear();
		// Icons
		if (BLEhrm.isConnected()) display.drawXbm(0, 0, 16, 16, icon_heart);

		// Touch
		for (uint_fast8_t i = 0; i<4; i++) {
			if (touch[i]) {
				display.fillCircle(21 + i * 8, 4, 4);
				if (touch_ani[i] == ani_counter ) touch[i] = false;
				if (ani_counter > touch_ani[i]) touch_ani[i] = ani_counter + 5;
			} else {
				display.drawCircle(21 + i * 8, 4, 4);
			}
		}

//		display.setFont(ArialMT_Plain_10);
//		display.setTextAlignment(TEXT_ALIGN_LEFT);
//		char buffer[32];
//		display.drawStringf(64, 16, buffer, "%d, %d, %d, %d", touch[0], touch[1], touch[2], touch[3]);


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

		// Power
		float power = forumslader.getDynPower();
		int8_t powerStep = floor(power * 8 / 10.0); // 8 LEDs-Steps, 10 W peak
		if (powerStep < 0) powerStep = 0;
		if (powerStep > 7) powerStep = 7;
		leds[0] = powerledsteps[powerStep];

		float cur = forumslader.getBatCurrent();
		int8_t batStep = floor((cur + 2.0) * 8 / 4.0); // 8 LEDs-Steps, -2A - +2A (4A Diff)
		if (batStep < 0) batStep = 0;
		if (batStep > 7) batStep = 7;
		leds[1] = battledsteps[batStep];

		leds[2] = CRGB::Black;

		if (ani_counter % 10 == 0) Serial.printf("Steps: Power: %d (%.1fW), BAT: %d (%.2fA)\n", powerStep, power, batStep, cur);

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

		uint16_t hr = BLEhrm.getHrm().HRM;
//		int8_t hrm_step = (hr-45) / 9;  //FIXME: test for quick reaction at home
//		if (hrm_step < 0) hrm_step=0; //tot?
//		if (hrm_step > 5) hrm_step=5;
//		for (uint_fast8_t i = 0; i < 5 ; i++) {
//			leds[7-i] = (hrm_step >= i) ? CRGB::BlueViolet : CRGB::Black;
//		}

		for (uint_fast8_t i = 0; i < sizeof(hr_leds[0].leds); i++) {
			if (hr < hr_leds[i].max) {
				for (uint_fast8_t l = 0; l < 5 ; l++) {
							leds[7-l] = hr_leds[i].leds[l];
			    }
				break;
			}
		}
		leds[2] = forumslader.getStateLED();


		if (hr == 0) leds[7] = CRGB::Orange;
		if (!BLEhrm.isConnected()) leds[7] = CRGB::Red;

		// write the buffer to the display and update LEDs
		display.display();
		FastLED.show();
		if (ani_counter % 10 == 0) touchpins.diagnostics();

	}
	delay(10);
}
