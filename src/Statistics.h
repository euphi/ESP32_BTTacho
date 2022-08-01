/*
 * Statistics.h
 *
 *  Created on: 02.04.2022
 *      Author: ian
 */

#pragma once
#include <Arduino.h>

class Statistics {
public:
	Statistics();

	enum ESummaryType{
		FL_TOUR = 0,
		FL_TRIP,
		ESP_TRIP,
		ESP_TOUR,
		ESP_START,

		ESummaryTypeMax
	};

	enum EDrivingState {
		NO_CONN,
		BREAK,
		STOP,
		DRIVE_COASTING,
		DRIVE_POWER
	};

	float getAvgSpeed(ESummaryType idx) const {return avg_speed[idx];}
	float getMaxSpeed(ESummaryType idx) const {return max_speed[idx];}
	float getDistance(ESummaryType idx) const {return distance - start_distance[idx];}
	uint16_t getTimeDriving(ESummaryType idx) const {return time_driving[idx];}
	uint16_t getTimeTotal(ESummaryType idx) const {return time_total[idx];}

	void getStateTime(String& rc) const;
	EDrivingState getState() const { return drivingState;}

	EDrivingState drivingState;

	void updateDistance(uint32_t _distance);
	void updateFLStoredDistance(ESummaryType type, uint32_t distance);
	void updateSpeed(float speed) { curSpeed = speed;}

	void AddHR(uint8_t hr);
	uint8_t getAvgHR(ESummaryType type) const;
	uint8_t getMinHR(ESummaryType type) const;
	uint8_t getMaxHR(ESummaryType type) const;
	const unsigned char* getDriveStateIcon() const;

	void AddCadence(uint8_t cadence);
	uint8_t getAvgCadence(ESummaryType type) const;
	uint8_t getMaxCadence(ESummaryType type) const;

	void setIPStr(const String& _ipStr) {ipStr = _ipStr;}
	const String& getIPStr() const {return ipStr;}

	void cycle();



private:
	float curSpeed;

	float avg_speed[ESummaryTypeMax];
	float max_speed[ESummaryTypeMax];
	uint32_t start_distance[ESummaryTypeMax];  // start_distance: For locally stored distances, this is the distance the counter was reset. For FL-side stored distance this is the actual distance
	uint32_t distance;							// current distance counter
	uint16_t time_total[ESummaryTypeMax];
	uint16_t time_driving[ESummaryTypeMax];
	uint16_t time_pause[ESummaryTypeMax];

	uint32_t hr_avg_total[ESummaryTypeMax];
	uint16_t hr_avg_count[ESummaryTypeMax];

	uint8_t hr_min[ESummaryTypeMax];
	uint8_t hr_max[ESummaryTypeMax];
	uint16_t timeDriving_aboveAnaerobe;
	uint16_t timeTotal_aboveAnaerobe;

	uint16_t hr_histogram_bucket[9];      // <75 ; 75-100 ; 100-120 ; 120-130; 130-140; 140-150; 150-160; 160-170; > 170
	const uint8_t hr_histogram_boundary[9] = { 75, 100, 120, 130, 140, 150, 160, 170, 255};

	uint32_t cadence_avg_total[ESummaryTypeMax];
	uint16_t cadence_avg_count[ESummaryTypeMax];
	uint8_t cadence_max[ESummaryTypeMax];

	uint16_t cadence_histogram_bucket[9];      // <5;5-40;40-60;60-70;70-80;80-90;100-110;>110
	const uint8_t cadence_histogram_boundary[9] = { 5, 40, 60, 70, 80, 90, 100, 110, 255};


	uint32_t timestamp_state = 0;	// Timestamp Last State Change
	uint32_t timestamp_cycle = 0;	// Timestame Last Cycle

	String ipStr = "n/a";

};
