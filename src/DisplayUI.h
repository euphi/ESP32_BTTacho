/*
 * DisplayUI.h
 *
 *  Created on: 27.03.2022
 *      Author: ian
 */

#pragma once

// SSD1306 OLED 128x64 on I2C
#include <SSD1306Wire.h>

#include <BleHeartRate.h>
#include <BtClassicForumsLader.h>
#include <Statistics.h>

// Touch input
#include <AtmESP32TouchButton.h>

class DisplayUI {
public:
	DisplayUI(const BtClassicForumsLader& _fl, const BleHeartRate& _blehrm, const Statistics& _stats);
	void setup();
	void cycle();
private:
	void displayIcons();
	void displaySpeed(const uint8_t x, const uint8_t y, const uint8_t size);
	void displayHR(const uint8_t x, const uint8_t y, const uint8_t size);
	void displayBatterie(const uint8_t x, const uint8_t y, const uint8_t* font);
	void displayLevel(const uint8_t x, const uint8_t y, const uint8_t* font);
	void displayGradient(const uint8_t x, const uint8_t y, const uint8_t size);
	void displayConsumerCurrent(const uint8_t x, const uint8_t y, const uint8_t* font);
	void displayDistance(const uint8_t x, const uint8_t y, const uint8_t size, const Statistics::ESummaryType type);
	void displayAvgMaxSpeed(const uint8_t x, const uint8_t y, const uint8_t size, const Statistics::ESummaryType type, bool max=false);

	void inline pageBoundary(int8_t &p, uint8_t min, uint8_t max) {
		if (p < 0) p = max;
		if (p > max) p = 0;
	};

	SSD1306Wire display;
	const BtClassicForumsLader& fl;
	const BleHeartRate& blehrm;
	const Statistics& stats;
	AtmESP32TouchButton touch[4];
	uint32_t anicounter;
	bool simulation;


	enum EIconTypes {
		ICON_CONNECTED_FL = 0,
		ICON_CONNECTED_HR,
		ICON_SD_CARD,
		ICON_TOUCH,
		ICON_BATTERIE,
		ICON_TRACTION,			// Cadence matches speed
		ICON_STANDSTILL,

		COUNT_ICON
	} ;

	enum ESize {
		SIZE_10 = 0,
		SIZE_16,
		SIZE_24,
		SIZE_36
	};


	enum ETextTypes {
		TEXT_SPEED = 0,
		TEXT_GRADIENT,
		TEXT_CONSCURR,
		TEXT_POWER,
		TEXT_TEMP,
		TEXT_TIME_1,
		TEXT_TIME_2,
		TEXT_DISTANCE_1,
		TEXT_DISTANCE_2,

		COUNT_TEXT
	};

	enum EPage {
		PAGE_SPEED = 0,
		PAGE_BATT,
		PAGE_TOTALS,
		PAGE_DETAIL,

		COUNT_PAGE
	};

	int8_t page;
	int8_t frame;
	const char* page_title[COUNT_PAGE] = {"Speed", "Bat", "Dist", "Det"};

//	struct SIconInfo {
//		uint8_t sx;
//		uint8_t sy;
//		const unsigned char* pIcon;
//		bool enabled[COUNT_PAGE];
//		bool leftright;		// false: left - true:right
//	};
//	struct page_info {
//		SIconInfo icon_info[COUNT_ICON];
//		ETextTypes text_info[COUNT_TEXT];
//		String title;			// Debug only
//	};
//
//
//	static const SIconInfo icons[COUNT_ICON];
};
