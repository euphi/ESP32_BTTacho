//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"

#include <Arduino.h>

// Statistics object: Communication to it will be hardcoded, so no patterln like Pub/Sub or Visitor yet. For now only BleHearRate and FLClassic are needed. It's simpler to make them actively store their data in Statistics object
#include <Statistics.h>
Statistics stats;

#include <BtClassicForumsLader.h>
BtClassicForumsLader forumslader(stats);

#include <BleHeartRate.h>
BleHeartRate BLEhrm(stats);

//8x WS2812 LED-Strip (for Heart Rate, ...) -> FastLED
#include <FastLED.h>
CRGB leds[8];
CRGB powerledsteps[8];
CRGB battledsteps[8];

#include <DisplayUI.h>
DisplayUI display(forumslader, BLEhrm, stats);


//SD
#include <SDLogger.h>
SDLogger sdl;

#include "mbedtls/md5.h"

#include "time.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "IA216mobil";
const char* password = "WeNiHueIOf!";


AsyncWebServer server(80);

// Inputs / Outputs
// D34 Q Taster (LED)
// D35 I Taster
// D4 I_Ser LED-Strip 8xWS2812

#include <Automaton.h>
Atm_button pushb;
Atm_led pushled;

//#include <esp32_touch.hpp>
//ESP32Touch touchpins;
//bool touch[4] = {false, false, false, false};

#include <Singletons.h>

const struct {
	uint8_t max;
	CRGB leds[5];
} hr_leds[] = { { 70,  {CRGB::Turquoise,CRGB::Black,    CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
		        { 80,  {CRGB::Turquoise,CRGB::Turquoise,CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
				{ 90,  {CRGB::Turquoise,CRGB::Green,	CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
				{ 100, {CRGB::Green,	CRGB::Green,	CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
				{ 110, {CRGB::Green,	CRGB::Green,	CRGB::Green,		CRGB::Black,		CRGB::Black  }},
				{ 120, {CRGB::Green,	CRGB::Green,	CRGB::Green,		CRGB::Green,		CRGB::Black  }},
				{ 130, {CRGB::Green,	CRGB::Green,	CRGB::GreenYellow,	CRGB::GreenYellow,	CRGB::Black  }},
				{ 140, {CRGB::Green,	CRGB::GreenYellow,CRGB::GreenYellow,CRGB::GreenYellow,	CRGB::Black  }},
				{ 150, {CRGB::Green,	CRGB::GreenYellow,CRGB::GreenYellow,CRGB::GreenYellow,	CRGB::GreenYellow  }},
				{ 160, {CRGB::Green,	CRGB::GreenYellow,CRGB::GreenYellow,CRGB::Yellow,		CRGB::Yellow  }},
				{ 170, {CRGB::Green,	CRGB::GreenYellow,CRGB::GreenYellow,CRGB::Orange,		CRGB::Orange  }},
				{ 255, {CRGB::Yellow,CRGB::Yellow, CRGB::Orange,		CRGB::Orange,		CRGB::Red    }}
};


void task_writeLog(void * p) {
	Serial.println("🏃 - Task LogWriter started");
	while(true) {
		Serial.print("Write Log - Stack: ");
		Serial.println(uxTaskGetStackHighWaterMark(NULL));  // DEBUG

		sdl.appendLog(forumslader.getSpeed(), 0, forumslader.getGradient(), 0, forumslader.getHeight(), BLEhrm.getHR());
		vTaskDelay(5000 / portTICK_PERIOD_MS);			// Sleep 5sec
	}
}

void setup()
{
  Serial.begin(115200);

  // Initialize I2C
  //Wire.begin();
  Wire.begin();
  setSyncProvider(RTC.get);   // the function to sync the time from the RTC


  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  Serial.print("LED Array size: ");
  Serial.println(sizeof(hr_leds) / sizeof(hr_leds[0]));


  //rtc.set(0, 07, 12, 6, 9, 4, 22);


  // Initialise FastLED
  FastLED.addLeds<NEOPIXEL, 27>(leds, 8);
  FastLED.setBrightness(64);
  FastLED.setCorrection(TypicalSMD5050);

  fill_gradient_RGB(powerledsteps, 0, CRGB::BlueViolet, 7, CRGB::Green);
  fill_gradient_RGB(battledsteps, 0, CRGB::Red, 3, CRGB::Orange);
  fill_gradient_RGB(battledsteps, 4, CRGB::YellowGreen, 7, CRGB::Green);
  for (uint_fast8_t i=0; i<8; i++) leds[i] = powerledsteps[i];
  FastLED.show();
  display.setup();

  forumslader.connect();
  BLEhrm.setup();


  pushb.begin(25).onPress(pushled, Atm_led::EVT_TOGGLE);
  pushled.begin(32, true);

  sdl.setup();
  xTaskCreate(task_writeLog, "LogWriter SD",
    2048,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}


void loop() {
	static uint32_t lastDisplayUpdate = 0;
	static uint16_t ani_counter = 0;
	static bool wifiInitialized = false;
	//static uint16_t touch_ani[4] = {0, 0, 0, 0};
	forumslader.loop();
	BLEhrm.loop();
	automaton.run();
	yield();
	if (lastDisplayUpdate + 100 < millis()) {			// 100 ms Cycle
		lastDisplayUpdate = millis();
		ani_counter++;

		stats.cycle();
		display.cycle();


//		// Touch
//		for (uint_fast8_t i = 0; i<4; i++) {
//			if (touch[i].state() == Atm_button::PRESSED) {
//				display.fillCircle(21 + i * 8, 4, 4);
//				//if (touch_ani[i] == ani_counter ) touch[i] = false;
//				if (ani_counter > touch_ani[i]) touch_ani[i] = ani_counter + 5;
//			} else {
//				display.drawCircle(21 + i * 8, 4, 4);
//			}
//		}

		//LED-Strip

		// Power
		float power = forumslader.getDynPower();
		int8_t powerStep = floor(power); // * 8 / 8.0); // 8 LEDs-Steps, 8 W peak
		if (powerStep < 0) powerStep = 0;
		if (powerStep > 7) powerStep = 7;
		leds[0] = powerledsteps[powerStep];

		float cur = forumslader.getBatCurrent();
		int8_t batStep = floor((cur + 0.8) * 8 / 1.6); // 8 LEDs-Steps, -0.8A - +0.8A (1.6A Diff)
		if (batStep < 0) batStep = 0;
		if (batStep > 7) batStep = 7;
		leds[1] = battledsteps[batStep];

		if (ani_counter % 10 == 0) Serial.printf("Steps: Power: %d (%.1fW), BAT: %d (%.2fA)\n", powerStep, power, batStep, cur); // 10 * 190ms = 1sec cycle


		//Heart rate
		uint8_t hr = BLEhrm.getHR();
		for (uint_fast8_t i = 0; i < 12; i++) {//TODO: Use sizeof for array size (sizeof(hr_leds) / sizeof(hr_leds[0]) ?
			if (hr < hr_leds[i].max) {
				for (uint_fast8_t l = 0; l < 5 ; l++) {
							leds[7-l] = hr_leds[i].leds[l];
			    }
				break;
			}
		}

		// State LED
		leds[2] = forumslader.getStateLED();  // FL
		if (hr == 0) leds[7] = CRGB::Orange;  // HR
		if (!BLEhrm.isConnected()) leds[7] = CRGB::Red;

		FastLED.show();
		if (ani_counter % 50 == 0) {
			AtmESP32TouchButton::diagnostics();		// 50 * 190ms = 5sec cycle
			Serial.println();
			time_t timeNow = now();
			Serial.printf("Now: %ld\n",timeNow);
		}

		// Check WiFi
		if (!wifiInitialized) {
			if (WiFi.status() == WL_CONNECTED) {
				wifiInitialized = true;
//				server.on("/logfile.bin", HTTP_GET, [](AsyncWebServerRequest *request) {
//					//request->send(200, "text/plain", "Hi! /Logfile placeholder.");
//					request->send(SD, "/BTTacho/LOG_0002.BIN", "application/octet-stream", true);
//				});
////				server.on("/logfile.bin", HTTP_GET, [](AsyncWebServerRequest *request) {
////							//request->send(200, "text/plain", "Hi! Logfile (no /) placeholder.");
////							request->send(SD, "/BTTacho/LOG_0002.BIN", "application/octet-stream", true);
////						});
//				server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//					request->send(200, "text/plain", "Hi! I am ESP32.");
//				});
//				server.onNotFound([](AsyncWebServerRequest *request){
//				  request->send(404);
//				});
				//server.serveStatic("/log", SD, "/BTTacho/");
				//server.serveStatic("/", SD, "/");
				AsyncElegantOTA.begin(&server);    // Start ElegantOTA
				server.begin();
				Serial.printf("Connected to WiFi - HTTP server started at %s.\n", WiFi.localIP().toString().c_str());
				stats.setIPStr(WiFi.localIP().toString());
				//init and get the time
				configTime(3600, 3600, "pool.ntp.org");
				  struct tm timeinfo;
				  if(!getLocalTime(&timeinfo)){
				    Serial.println("Failed to obtain time");
				    return;
				  }
				  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
				  RTC.set(time(0));
			} else if (ani_counter > 150) {
				wifiInitialized = true;
				Serial.println("Not connected ot Wifi - disabling it");
				WiFi.mode(WIFI_MODE_NULL);
				WiFi.setSleep(WIFI_PS_MAX_MODEM);
				WiFi.setSleep(true);
			}
		}
	}
	delay(10);		// minimum 10ms pause between cycles (should save some power?)
}

