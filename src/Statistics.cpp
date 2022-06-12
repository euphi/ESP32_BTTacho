/*
 * Statistics.cpp
 *
 *  Created on: 02.04.2022
 *      Author: ian
 */

#include <Statistics.h>

// Icons

static unsigned char icon_power[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x04, 0x20, 0x04, 0x3c, 0x04,
   0x50, 0x0a, 0x52, 0x4a, 0x89, 0x81, 0xf9, 0x92, 0x41, 0x82, 0x01, 0x80,
   0x12, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static unsigned char icon_break[] = {
   0x00, 0x00, 0x40, 0x00, 0x80, 0x00, 0x90, 0x00, 0x90, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0x1f, 0x00, 0x10, 0x00, 0x70, 0x00, 0x10, 0x02, 0x10,
   0x00, 0x28, 0x04, 0x04, 0x18, 0x02, 0xfe, 0x1f };
static unsigned char icon_pause[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x0e, 0x70, 0x0e, 0x70, 0x0e,
   0x70, 0x0e, 0x70, 0x0e, 0x70, 0x0e, 0x70, 0x0e, 0x70, 0x0e, 0x70, 0x0e,
   0x70, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static unsigned char icon_stop[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f,
   0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f,
   0xf8, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


Statistics::Statistics(): curSpeed(NAN) {

}

void Statistics::AddHR(uint8_t hr) {
	uint8_t histBucket = 0;
	for (histBucket = 0; histBucket<sizeof(histBucket); histBucket++) {
		if (hr < hr_histogram_boundary[histBucket]) break;
	}
	Serial.printf("Adding HR of %d to bucket %d\n", hr, histBucket);
	hr_histogram_bucket[histBucket] += 1;

	for (uint8_t i = ESP_TRIP; i <= ESP_START; i++) {
		if (hr > hr_max[i]) hr_max[i] = hr;
		if (hr < hr_min[i]) hr_min[i] = hr;
		// This calculates the average of all received HR rates. It is a valid time-avarge only if HR rate is sent frequently and continuously.
		hr_avg_total[i] += hr;
		hr_avg_count[i]++;
	}
}


uint8_t Statistics::getAvgHR(ESummaryType type) const {
	return hr_avg_total[type] / hr_avg_count[type];
}

uint8_t Statistics::getMinHR(ESummaryType type) const {
	return hr_min[type];
}

uint8_t Statistics::getMaxHR(ESummaryType type) const {
	return hr_max[type];
}

void Statistics::updateDistance(uint32_t _distance) {
	distance = _distance;
	if (start_distance[ESP_START] == 0) start_distance[ESP_START] = _distance;

}

void Statistics::cycle() {
	if (isnan(curSpeed)) {
		drivingState = NO_CONN;
		return;			// NaN as speed means not connected.
	}
	if (curSpeed < 0.1) {
		if (drivingState > STOP) {
			timestamp_standstill = millis();
			drivingState = STOP;
		}
		if (drivingState == STOP && (millis() > (timestamp_standstill + 120000))) {
			drivingState = BREAK;
		}
	} else {
		if (drivingState <= STOP) {
			drivingState = DRIVE_POWER;
			//TODO: Cadence sensor needed to distinguish between POWER and COASTING
		}
	}
	uint32_t time_diff = millis() - timestamp_cycle;

	for (uint8_t i = ESP_TRIP; i <= ESP_START; i++) {
		time_total[i] += time_diff;
		if (drivingState > STOP) time_driving[i] += time_diff;
		if (curSpeed > max_speed[i]) max_speed[i] = curSpeed;
	}
}

const unsigned char* Statistics::getDriveStateIcon() const {
	switch (drivingState) {
	case NO_CONN:
		return icon_stop;
	case BREAK:
		return icon_break;
	case STOP:
		return icon_pause;
	case DRIVE_COASTING:
	case DRIVE_POWER:
		//TODO: add animation by using some cycle-counter
		return icon_power;
	}
	return NULL;
}

void Statistics::updateFLStoredDistance(ESummaryType type, uint32_t distance) {
	start_distance[type] = distance;
}
