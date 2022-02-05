/*
 * BtClassicForumsLader.h
 *
 *  Created on: 04.02.2022
 *      Author: ian
 */

#pragma once
#include <Arduino.h>
#include <BluetoothSerial.h>

class BtClassicForumsLader {
public:
	BtClassicForumsLader();
	void loop();

	static constexpr float dist_per_pulse = 2.155 / 14; // Umfang / Polpaare
	static constexpr float hmh_per_pulse = 2.155 / 14 * 3600 * 0.001 * 10; // Umfang / Polpaare * 3600 sec/hour * 0.001 m/km * 10 (100m/km)


private:

	BluetoothSerial SerialBT;

	int8_t	stufe;			// -1: Dauer-Aus, 0: aus, 1-4: nach Geschwindigkeit
	// Data fields from Forumslader
	uint16_t err_flags;		// Bitset
	uint16_t speed;			// in 100m/h
	uint16_t speedAvgDay;	// in 100m/h
	uint16_t speedAvgTour;	// in 100m/h
	uint16_t totalDay;		// in 100m
	uint16_t totalTour;		// in 100m
	uint16_t batterie[3];	// in mV
	int16_t  batt_current;	// in mA
	int16_t  cons_current;  // in mA

    int16_t temperature;	// in 1/10°C
	uint32_t pressure;		// in Pa (1.013E05 -> normal pressure
	uint16_t height;		// in 1/10m
	int16_t  gradient;		// in ‰


	uint16_t unknown[8];	// yet unkown values

	String bufferSerial;

	void updateDataFromString();
	void readFromSerial();

public:
	float getVoltageTotal() { return ( (batterie[0]+ batterie[1] + batterie[2]) / 1000.0);}
	float getSpeed() { return (speed / 10.0);}
	float getGradient() { return (gradient / 10.0);}
	float getHeight() { return (height / 10.0);}

	int8_t getStage() const {return stufe;}
};

