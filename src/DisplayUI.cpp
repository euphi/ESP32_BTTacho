/*r
 * DisplayUI.cpp
 *
 *  Created on: 27.03.2022
 *      Author: ian
 */

#include <DisplayUI.h>
#include <DisplayUIFonts.h>
#include <WiFi.h>

#include <version.h>
#include <pindef.h>

// Icons TODO: Use own class/modules for graphics
//static const unsigned char icon_heart[] PROGMEM = {
//    0b00000000, 0b00000000, //
//    0b01111000, 0b00111100, //   ####   ####
//    0b11111100, 0b01111110, //  ###### ######
//    0b11111110, 0b11111111, // ###############
//    0b11111110, 0b11111111, // ###############
//    0b11111110, 0b11111111, // ###############
//    0b11111110, 0b11111111, // ###############
//    0b11111100, 0b01111111, //  #############
//    0b11111100, 0b01111111, //  #############
//    0b11111000, 0b00111111, //   ###########
//    0b11110000, 0b00011111, //    #########
//    0b11100000, 0b00001111, //     #######
//    0b11000000, 0b00000111, //      #####
//    0b10000000, 0b00000011, //       ###
//    0b00000000, 0b00000001, //    	  #
//	0b00000000, 0b00000000, //
//};

//static const unsigned char icon_heart[] = { 0x00,0x36,0x5d,0x49,0x41,0x22,0x14,0x08}; // 8x8
static unsigned char icon_heart_filled[] = {0x36,0x7f,0x7f,0x7f,0x3e,0x1c,0x08,0x00}; // 8x8
static const unsigned char icon_bt[] = {0x18, 0x2A, 0x6C, 0x38, 0x38, 0x6C, 0x2A, 0x18 }; // 8x8
static const unsigned char icon_wifi[] = { 0x80, 0xA0, 0xA8, 0xAB, 0xAB, 0xA8, 0xA0, 0x80 }; // 8x8

//static const DisplayUI::EPage DisplayUI::pages[COUNT_PAGE] = {
//};

//struct page_info {
//	EIconTypes icon_info[COUNT_ICON];
//	ETextTypes text_info[COUNT_TEXT];
//	String title;			// Debug only
//};

#define ICON_DISABLED {0, 0, 0, 0, NULL, false}
#define TEXT_DISABLED {0, 0, ArialMT_Plain_24, TEXT_ALIGN_RIGHT, false}

//const DisplayUI::SIconInfo DisplayUI::icons[COUNT_ICON] = {
//		{0, 0, 16, 16, icon_heart, true},		// ICON_CONNECTED_FL = 0,
//		{0, 0, 16, 16, icon_heart, true},		// ICON_CONNECTED_HR,
//		ICON_DISABLED,      		// ICON_SD_CARD,
//		ICON_DISABLED,         		// ICON_TOUCH,
//		ICON_DISABLED,         		// ICON_BATTERIE,
//		ICON_DISABLED,         		// ICON_TRACTION,
//		ICON_DISABLED         		// ICON_STANDSTILL,
//};
//};

//const DisplayUI::page_info DisplayUI::pages[COUNT_PAGE] = {
//		{ICON_DISABLED, TEXT_DISABLED, "Speed Page"},
//		{ICON_DISABLED, TEXT_DISABLED, "Batterie Page"},
//		{ICON_DISABLED, TEXT_DISABLED, "Trip Page"},
//		{ICON_DISABLED, TEXT_DISABLED, "Tour Page"},
//		{ICON_DISABLED, TEXT_DISABLED, "Detail Page"}
//};

DisplayUI::DisplayUI(const BtClassicForumsLader& _fl, const BleHeartRate& _blehrm, const Statistics& _stats): display(0x3c, SDA, SCL), fl(_fl), blehrm(_blehrm), stats(_stats), anicounter(0), simulation(false) {

}

void DisplayUI::setup() {
	  display.init();
	  display.flipScreenVertically();

	  display.clear();
	  display.setFont(ArialMT_Plain_10);
	  display.drawString(0, 0, "Startup - Connecting...");
	  display.setFont(ArialMT_Plain_16);
	  display.drawString(0, 20, VERSION_SHORT);
	  display.display();

	  //TOuch
	  //touchSetCycles(0x2000, 0x2000);
	  touch[0].begin(TOUCH_UP).onPress([this](int idx, int v, int up){page++;}, 0);
	  touch[1].begin(TOUCH_DN).onPress([this](int idx, int v, int up){page--;}, 0);
	  touch[2].begin(TOUCH_R).onPress([this](int idx, int v, int up){frame++;}, 0);
	  touch[3].begin(TOUCH_L).onPress([this](int idx, int v, int up){frame--;}, 0);
	  AtmESP32TouchButton::setup();
}

void DisplayUI::cycle() {
	pageBoundary(page, 0, COUNT_PAGE-1);
	anicounter++;
	uint16_t seccounter = anicounter / 10;
	display.clear();
	displayIcons();
	switch (page) {
		case PAGE_SPEED:
			pageBoundary(frame, 0, 2);

			//display.drawVerticalLine(48, 16, 20);
			switch (frame) {
			case 0:
				displayDistance(112, 16, SIZE_24, Statistics::ESP_START);
				displayGradient(24,42, SIZE_16);
				break;
			case 1:
				displayHR(106, 16, SIZE_24);
				displayGradient(24,42, SIZE_16);
				break;
			case 2:
				displayHR(106, 16, SIZE_24);
				displayAvgMaxSpeed(0,12, SIZE_16, Statistics::ESP_START, true);
				displayAvgMaxSpeed(0,36, SIZE_16, Statistics::ESP_START);
				break;
			}
			displaySpeed(112, 34, SIZE_36);
			break;
		case PAGE_TOTALS:
			frame = seccounter % 4;
			pageBoundary(frame, 0, 3);
			displaySpeed(112, 30, SIZE_36);
			displayHR(106, 16, SIZE_24);
			switch (frame) {
			case 0:
				break;
			case 1:
				break;
			case 2:
				break;
			case 3:
				break;
			}
			break;
		case PAGE_BATT:
			pageBoundary(frame, 0, 2);
			displayBatterie(0,16, ArialMT_Plain_16);
//			displayLevel();
//			displayConsumerCurrent();
			break;
		case PAGE_DETAIL:
			pageBoundary(frame, 0, 4);
			display.setTextAlignment(TEXT_ALIGN_LEFT);
			display.setFont(ArialMT_Plain_10);
			if (!stats.getIPStr().isEmpty()) {
				display.drawString(0, 16, String("IP: ") + stats.getIPStr());
			}
			display.drawString(0, 28, VERSION);

			break;
	}
	display.display();
}

void DisplayUI::displayIcons() {
   // LEFT ----> RIGHT
   // Connections
   // BLE HR
   if (blehrm.isConnectedHRM()) display.drawXbm(0, 0, 8, 8, icon_heart_filled);
   if (blehrm.isConnectedCadence()) display.drawXbm(0, 8, 8, 8, icon_heart_filled);
   //BT
   if (fl.isConnected() == BtClassicForumsLader::STATE_CONNECTED ||
	   (fl.isConnected() == BtClassicForumsLader::STATE_CONNECTING && ((anicounter/5)%2 == 0)) ) {
	   	   display.drawXbm(8, 8, 8, 8, icon_bt);
   }
   if (WiFi.isConnected()) {
	   display.drawXbm(8, 0, 8, 8, icon_wifi);
   }

   const unsigned char* state_icon = stats.getDriveStateIcon();
   if (state_icon) display.drawXbm(16, 0, 16, 16, state_icon);
   display.setFont(ArialMT_Plain_10);
   display.setTextAlignment(TEXT_ALIGN_LEFT);
   //DbZ display.drawString(32, 0, page_title[page] + String(frame));



   //  LEFT <------- RIGHT
   // Batterie
   display.drawProgressBar(99, 0, 28, 8, fl.getBattPerc());
   //display.drawProgressBar(99, 0, 28, 8, static_cast<uint8_t>((fl.getVoltageTotal() - 11.5) / (12.5 - 11.5) * 100));
   display.setFont(Nimbus_Sans_L_Regular_Condensed_8);
   display.setTextAlignment(TEXT_ALIGN_RIGHT);
   display.drawString(127, 8, String(fl.getVoltageTotal(), 1) + " V");

   // Stufe
   uint8_t powerPerc = fl.getDynPower() * 10;
   int8_t stage = fl.getStage();
	//Simulation:
   if (simulation) {
	   stage = (anicounter/12) % 5;
	   powerPerc = anicounter%100;
   }
   if (powerPerc > 100) powerPerc = 100;

   display.drawProgressBar(70, 0, 28, 6, powerPerc);
	for (int_fast8_t c = 0; c < 4; c++) {
		if (stage > c) {
			display.fillCircle(73 + (7*c), 11, 3);
		} else {
			display.drawCircle(73 + (7*c), 11, 3);
		}
	}

   // Touch
   uint8_t midx = 61;
   uint8_t midy = 6;
   if (touch[0].state() == Atm_button::PRESSED) display.fillTriangle(midx, midy-1, midx-6, midy-6, midx+6, midy-6);  // TOP
   if (touch[1].state() == Atm_button::PRESSED) display.fillTriangle(midx, midy-1, midx-6, midy+6, midx+6, midy+6);  // BOTT
   if (touch[2].state() == Atm_button::PRESSED) display.fillTriangle(midx+1, midy, midx+6, midy-6, midx+6, midy+6);  // RIGHT
   if (touch[3].state() == Atm_button::PRESSED) display.fillTriangle(midx-1, midy, midx-6, midy-6, midx-6, midy+6);  // LEFT
}

void DisplayUI::displaySpeed(const uint8_t x, const uint8_t y, const uint8_t size) {
	//		digitalWrite(32, digitalRead(33));
	//		if (ani_counter % 10 == 0) Serial.println(digitalRead(33) ? "Taster pressed" : "Taster released");
	//digitalWrite(32, ((ani_counter / 10) % 2) == 0);
	//		display.setFont(ArialMT_Plain_10);
	//		display.setTextAlignment(TEXT_ALIGN_LEFT);
	//		char buffer[32];
	//		display.drawStringf(64, 16, buffer, "%d, %d, %d, %d", touch[0], touch[1], touch[2], touch[3]);
	// Speed - TODO: Extract method
	//display.setFont(ArialMT_Plain_24);
	const uint8_t* font=0;
	switch (size) {
	case SIZE_10:
	case SIZE_16:
		return; //TODO not supported yet
	case SIZE_24:
		//font = Orbitron_Medium_24;
		font = DSEG7_Classic_Bold_24;
		//font = Roboto_Mono_24;
		break;
	case SIZE_36:
		font = Orbitron_Medium_36;
		//font = DSEG7_Classic_Bold_36;

//		font = Roboto_Mono_36;
		break;
	}
	display.setFont(font);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	String speedStr;
	speedStr = String(fl.getSpeed(), 1);
	display.drawString(x, y, speedStr);
	if (size >= SIZE_24) {
		uint8_t off = (size == SIZE_24) ? 0 : 4;
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(x, y+off, "km");
		display.drawHorizontalLine(x, y+off+11, 12);
		display.drawString(x+5, y+off+12, "h");
	}
}

void DisplayUI::displayHR(const uint8_t x, const uint8_t y, const uint8_t size) {
	const uint8_t* font=0;
	switch (size) {
	case SIZE_10:
	case SIZE_16:
		return; //TODO not supported yet
	case SIZE_24:
		font = Orbitron_Medium_24;
		//font = Roboto_Mono_24;
		break;
	case SIZE_36:
		font = Orbitron_Medium_36;
//		font = Roboto_Mono_36;
		break;
	}
	display.setFont(font);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(x,y, String(blehrm.getHR()));
	if (size >= SIZE_24) {
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(x, y + (size == SIZE_24) ? 12 : 16, "bpm");
	}

}

void DisplayUI::displayBatterie(const uint8_t x, const uint8_t y, const uint8_t* font) {
  display.setFont(font);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.drawString(0, 16, String(fl.getVoltageTotal(), 1) + "V");
	display.drawString(48, 16, String(fl.getBattPerc())+"%");
	display.drawString(0, 28, String("B: ") + String(fl.getBatCurrent(), 2)+"A " + String("C: ") + String(fl.getConsCurrent(), 2)+"A");
	display.drawString(0, 52, String("D: ") + String(fl.getDynCurrent(), 2)+"A");
	// Batterie - TODO: Extract method

	//display.drawString(x+64, y+48, "Pow:" + String(fl.getDynPower(), 1) + "W");
	//display.drawString(x, y+16, "SoC:" + String(fl.getBattPerc()) + "%");
}

void DisplayUI::displayLevel(const uint8_t x, const uint8_t y, const uint8_t* font) {
	// Level - TODO: Extract method
	int8_t stage = fl.getStage();
	//Simulation: stage = (wikis() / 1000) % 5;
	for (int_fast8_t c = 0; c < 4; c++) {
		if (stage > c) {
			//display.fillCircle(123, 59 - (8 * c), 4);
		} else {
			//display.drawCircle(123, 59 - (8 * c), 4);
		}
	}
}

void DisplayUI::displayGradient(const uint8_t x, const uint8_t y, const uint8_t size) {
	// Gradient - TODO: Extract method
	if (size != SIZE_16) return; //TODO: Add other sizes
	display.setFont(ArialMT_Plain_16);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(x, y, String(fl.getGradient(),1) + "%");
}

//void DisplayUI::displayConsumerCurrent(const uint8_t x, const uint8_t y, const uint8_t* font) {
//	// ConsCurrent - TODO: Extract method
//	display.setFont(ArialMT_Plain_16);
//	display.setTextAlignment(TEXT_ALIGN_LEFT);
//	//display.drawString(64, 16, String(fl.getConsCurrent(), 1) + "mA");
//}

void DisplayUI::displayDistance(const uint8_t x, const uint8_t y, const uint8_t size, const Statistics::ESummaryType type) {
	const uint8_t* font=0;
	switch (size) {
	case SIZE_10:
	case SIZE_16:
		return; //TODO not supported yet
	case SIZE_24:
		font = Orbitron_Medium_24;
		//font = Roboto_Mono_24;
		break;
	case SIZE_36:
		font = Orbitron_Medium_36;
//		font = Roboto_Mono_36;
		break;
	}
	display.setFont(font);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);
	display.drawString(x, y, String(stats.getDistance(type),1));
	if (size >= SIZE_16) {
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(x, y + ((size == SIZE_24) ? 6 : 10), "km");
	}

}

void DisplayUI::displayConsumerCurrent(const uint8_t x, const uint8_t y,
		const uint8_t *font) {
}

void DisplayUI::displayAvgMaxSpeed(const uint8_t x, const uint8_t y, const uint8_t size, const Statistics::ESummaryType type, bool max) {
	const uint8_t* font=0;
	switch (size) {
	case SIZE_10:
		font = ArialMT_Plain_10;
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		break;
	case SIZE_16:
		font = ArialMT_Plain_16;
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		break;
	case SIZE_24:
	case SIZE_36:
		return; //TODO not supported yet
	}
	display.setFont(font);
	float avg;
	if (max) {
		avg = stats.getMaxSpeed(type);
	} else {
		avg = stats.getAvgSpeed(type);
	}
	display.drawString(x, y+11, String(avg, 1));
	display.setFont(ArialMT_Plain_10);
	display.drawString(x+1, y, max?"max":"avg");
	//display.drawString(x+1, y+10, "km/h");
}

void DisplayUI::displayHistogram(const uint8_t hist[], const uint8_t hist_size,
		const uint8_t x, const uint8_t y) {
}
