//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"

#include <Arduino.h>

// Statistics object: Communication to it will be hardcoded, so no patterln like Pub/Sub or Visitor yet. For now only BleHearRate and FLClassic are needed. It's simpler to make them actively store their data in Statistics object
#include <Statistics.h>
Statistics stats;

//#include <BtClassicForumsLader.h>
//BtClassicForumsLader forumslader(stats);

#include <BleHeartRate.h>
BleHeartRate BLEhrm(stats);

#include <TRGBSuppport.h>
TRGBSuppport trgb;

//SD
#include <SDLogger.h>

//#include "mbedtls/md5.h"

//#include "time.h"
#include <DateTime.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <version.h>
#include <pindef.h>

const char* ssid = "IA216mobil";
const char* password = "WeNiHueIOf!";

//const char* ssid = "IA216oT";
//const char* password = "SwieSecurity";

AsyncWebServer server(80);

// Inputs / Outputs
// D34 Q Taster (LED)
// D35 I Taster
// D4 I_Ser LED-Strip 8xWS2812

//#include <Automaton.h>
//Atm_button pushb;
////Atm_led pushled;
//Atm_bit dimmstate;

//#include <esp32_touch.hpp>
//ESP32Touch touchpins;
//bool touch[4] = {false, false, false, false};

//#include <Singletons.h>
//
//const struct {
//	uint8_t max;
//	CRGB leds[5];
//} hr_leds[] = { { 70,  {CRGB::Turquoise,	CRGB::Black,        CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
//		        { 80,  {CRGB::Turquoise,    CRGB::Turquoise,    CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
//				{ 90,  {CRGB::Turquoise,    CRGB::Green,	    CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
//				{ 100, {CRGB::Green,	    CRGB::Green,	    CRGB::Black,   		CRGB::Black,		CRGB::Black  }},
//				{ 110, {CRGB::Green,	    CRGB::Green,	    CRGB::Green,		CRGB::Black,		CRGB::Black  }},
//				{ 120, {CRGB::Green,	    CRGB::Green,	    CRGB::Green,		CRGB::Green,		CRGB::Black  }},
//				{ 125, {CRGB::Green,	    CRGB::Green,	    CRGB::Green,		CRGB::GreenYellow,	CRGB::Black  }},
//				{ 130, {CRGB::Green,	    CRGB::Green,	    CRGB::GreenYellow,	CRGB::GreenYellow,	CRGB::Black  }},
//				{ 135, {CRGB::Green,	    CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,	CRGB::Black  }},
//				{ 140, {CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,	CRGB::Black  }},
//				{ 145, {CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,	CRGB::GreenYellow}},
//				{ 150, {CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,	CRGB::Yellow }},
//				{ 155, {CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::GreenYellow,  CRGB::Yellow,   	CRGB::Yellow }},
//				{ 160, {CRGB::GreenYellow,  CRGB::Yellow,       CRGB::Yellow,       CRGB::Orange,		CRGB::Orange }},
//				{ 165, {CRGB::Yellow, 	    CRGB::Yellow,       CRGB::Yellow,       CRGB::Orange,		CRGB::Orange }},
//				{ 170, {CRGB::Yellow,       CRGB::Yellow,       CRGB::Orange,       CRGB::Orange,		CRGB::Orange }},
//				{ 255, {CRGB::Yellow,	    CRGB::Orange,		CRGB::Orange,		CRGB::Orange,		CRGB::Red    }}
//};


void task_writeLog(void * p) {
	sdl.log(SDLogger::Log_Info, SDLogger::TAG_OP, "🏃 - Task LogWriter started");
	while(true) {
		sdl.logf(SDLogger::Log_Debug, SDLogger::TAG_OP, "Task writeLog: stack usage %d bytes", uxTaskGetStackHighWaterMark(NULL));
//		sdl.appendLog(forumslader.getSpeed(), 0, forumslader.getGradient(), 0, forumslader.getHeight(), BLEhrm.getHR());
		sdl.appendLog(0, 0, 0, 0, 0, BLEhrm.getHR());

		vTaskDelay(5000 / portTICK_PERIOD_MS);			// Sleep 5sec
	}
}

void setup()
{
  delay(100);   // Rumors say it helps avoid sporadical crashes after wakeup from deep-sleep
  Serial.begin(115200);
  Serial.setTxTimeoutMs(1);  // workaround for blocking output if no host is connected to native USB CDC
  trgb.init();
  // Initialize I2C (no longer necessary  - is in TRGBSupport)
  //Wire.begin();

  //Start Wifi
  configTime(3600, 3600, "pool.ntp.org");
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
//  Serial.print("LED Array size: ");
//  Serial.println(sizeof(hr_leds) / sizeof(hr_leds[0]));


//  // Initialise FastLED
//  FastLED.addLeds<NEOPIXEL, LEDSTRIP_0>(leds, 8);
//  FastLED.setBrightness(200);
//  FastLED.setCorrection(TypicalSMD5050);
//
//  fill_gradient_RGB(powerledsteps, 0, CRGB::BlueViolet, 7, CRGB::Green);
//  fill_gradient_RGB(battledsteps, 0, CRGB::Red, 3, CRGB::Orange);
//  fill_gradient_RGB(battledsteps, 4, CRGB::YellowGreen, 7, CRGB::Green);
//  for (uint_fast8_t i=0; i<8; i++) leds[i] = powerledsteps[i];
//  FastLED.show();
//  display.setup();

//  forumslader.connect();
  BLEhrm.setup();


//  dimmstate.trace(Serial);
//  //pushled.trace(Serial);
//
//  // Pushbutton for Dimmer + dimmer status LED. Both are connected via Automaton EVENTs
//  pushb.begin(PB_0).onPress(dimmstate, Atm_bit::EVT_TOGGLE);
//  //pushled.begin(LED_0, true);
//  //dimmstate.begin(Atm_bit::OFF).onChange(pushled, Atm_led::EVT_TOGGLE).onChange([](int idx, int v, int up){Serial.printf("DIMM Toggle (idx: %d, v: %d, up: %d\n", idx, v, up);display.setDimmed(v);FastLED.setBrightness(v?15:200);}, 0);
//  dimmstate.begin(Atm_bit::OFF).led(LED_0).onChange([](int idx, int v, int up){
//	  display.setDimmed(v);
//	  FastLED.setBrightness(v?15:200);
//	  //pushled.trigger(v?Atm_led::EVT_ON : Atm_led::EVT_OFF);
//  }, 1);

  sdl.setup();

  xTaskCreate(task_writeLog, "LogWriter SD",
    2048,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    NULL             // Task handle
  );
}

void setupWebserver() {
	static String htmlresponse(""); // Reserve buffer for HTML response
	// Enable file deletion
	//				server.on("^\\/del\\/[0-9]+)$", HTTP_GET, [](AsyncWebServerRequest *request) {   // using DELETE method on the same URI as for "serveStatic" would be more elegant, but is not possible to create links that result in making the browser use DELETE method. So use special "del" uri
	server.on("^\\/del\\/([A-Z,0-9,_,.,/]+)$", HTTP_GET, [](AsyncWebServerRequest *request) {
		// using DELETE method on the same URI as for "serveStatic" would be more elegant, but is not possible to create links that result in making the browser use DELETE method. So use special "del" uri
		sdl.logf(SDLogger::Log_Info, SDLogger::TAG_WIFI, "💻 Request on /del/: %s\n\tPath-Arg: %s", request->url().c_str(), request->pathArg(0).c_str());
		String dUri = String("/BTTacho/") + request->pathArg(0);
		uint16_t http_code = 500;
		String html_resp("<html><body>");
		if (sdl.deleteFile(dUri)) {
			http_code = 200;
			html_resp += "OK - file deleted.";
		} else {
			http_code = 403;
			html_resp += "Forbidden - file can't be deleted (probably because it does not exist";
		}
		html_resp += "</body></html>";
		request->send(http_code, "text/html", html_resp.c_str());
	});

	// -- download Binary Logfile
	server.serveStatic("/log/", SD, "/BTTacho/");

	// -- generate Logfile Index
	server.on("/logfiles/", HTTP_GET, [](AsyncWebServerRequest *request) {
		htmlresponse.clear();
		sdl.getAllFileLinks(htmlresponse);
		Serial.println(htmlresponse);
		request->send(200, "text/html", htmlresponse.c_str()); // TODO: Check of the webserver handles the String. It's on stack, so there may be a use-after-free issue here.
	});

	// -- offer cleanup
	server.on("/cleanup", HTTP_GET, [](AsyncWebServerRequest *request) {
		sdl.autoCleanUp("/BTTacho/");
		request->send(200, "text/plain", "Cleanup done.");
	});

	// -- Allow OTA via Web ("ElegantOTA" library)
	AsyncElegantOTA.begin(&server); // Start ElegantOTA
	server.onNotFound([](AsyncWebServerRequest *request) {
		sdl.logf(SDLogger::Log_Info, SDLogger::TAG_WIFI, "🛈 💻 Can't handle request on : %s\n", request->url().c_str());
		String responsetext = request->url() + " not found!\n";
		request->send(404, "text/plain", responsetext.c_str());
	});
	server.begin();
	sdl.logf(SDLogger::Log_Info, SDLogger::TAG_WIFI, "🛈 💻 Connected to WiFi - HTTP server started at %s.\n", WiFi.localIP().toString().c_str());
}

void loop() {
	static uint32_t lastDisplayUpdate = 0;
	static uint16_t ani_counter = 0;
	static bool wifiInitialized = false;

	//static uint16_t touch_ani[4] = {0, 0, 0, 0};
//	forumslader.loop();
	BLEhrm.loop();
//	automaton.run();
	yield();
	if (lastDisplayUpdate + 100 < millis()) {			// 100 ms Cycle
		lastDisplayUpdate = millis();
		ani_counter++;

		stats.cycle();
//		display.cycle();

		//LED-Strip

//		// Power
//		float power = forumslader.getDynPower();
//		int8_t powerStep = floor(power); // * 8 / 8.0); // 8 LEDs-Steps, 8 W peak
//		if (powerStep < 0) powerStep = 0;
//		if (powerStep > 7) powerStep = 7;
//		leds[0] = powerledsteps[powerStep];
//
//		float cur = forumslader.getBatCurrent();
//		int8_t batStep = floor((cur + 0.8) * 8 / 1.6); // 8 LEDs-Steps, -0.8A - +0.8A (1.6A Diff)
//		if (batStep < 0) batStep = 0;
//		if (batStep > 7) batStep = 7;
//		leds[1] = battledsteps[batStep];

		//if (ani_counter % 10 == 0) Serial.printf("Steps: Power: %d (%.1fW), BAT: %d (%.2fA)\n", powerStep, power, batStep, cur); // 10 * 190ms = 1sec cycle

		//Heart rate
		uint8_t hr = BLEhrm.getHR();
//		for (uint_fast8_t i = 0; i < 17; i++) {//TODO: Use sizeof for array size (sizeof(hr_leds) / sizeof(hr_leds[0]) ?
//			if (hr < hr_leds[i].max) {
//				for (uint_fast8_t l = 0; l < 5 ; l++) {
//							leds[7-l] = hr_leds[i].leds[l];
//			    }
//				break;
//			}
//		}

//		// State LED
//		leds[2] = forumslader.getStateLED();  // FL
//		if (hr == 0) leds[7] = CRGB::Orange;  // HR
//		if (!BLEhrm.isConnected()) leds[7] = CRGB::Red;
//
//		FastLED.show();

		if (ani_counter % 50 == 0 && ( sdl.checkLogLevel(SDLogger::Log_Debug, SDLogger::TAG_OP, false) || sdl.checkLogLevel(SDLogger::Log_Debug, SDLogger::TAG_OP, true) ) ) {			// 50 * 190ms = 5sec cycle
//			AtmESP32TouchButton::diagnostics();
//			Serial.println();
			String time_str_rtc = DateTime.format(DateFormatter::HTTP);
			sdl.logf(SDLogger::Log_Debug, SDLogger::TAG_OP, "🕑 %s", time_str_rtc.c_str());
		}

		// Check WiFi
		if (!wifiInitialized) {
			if (WiFi.status() == WL_CONNECTED) {
				wifiInitialized = true;
				stats.setIPStr(WiFi.localIP().toString());
				// Setup Web Server
				setupWebserver();
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

