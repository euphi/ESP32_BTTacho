/*
 * BtClassicForumsLader.h
 *
 *  Created on: 04.02.2022
 *      Author: ian
 */

#pragma once
#include <Arduino.h>
#include <BluetoothSerial.h>

//#include <pixeltypes.h>
#include <FastLED.h>

class BtClassicForumsLader {
public:
	BtClassicForumsLader();
	void loop();
	void connect();

	CRGB getStateLED();

	static constexpr float dist_per_pulse = 2.155 / 14; // Umfang / Polpaare
	static constexpr float hmh_per_pulse = 2.155 / 14 * 3600 * 0.001 * 10; // Umfang / Polpaare * 3600 sec/hour * 0.001 m/km * 10 (100m/km)

	enum {
		FLAG_BALANCE1       =  23 , // (1 << 7) << 16,
		FLAG_BALANCE2       =  22  , // (1 << 6) << 16,
		FLAG_BALANCE3       =  21  , // (1 << 5) << 16,
		FLAG_SHORTCIRCUIT   =  20  , // (1 << 4) << 16,
		FLAG_DISCHARGE_OVR  =  19  , // (1 << 3) << 16,
		FLAG_CHARGE_OVR     =  18  , // (1 << 2) << 16,
		FLAG_DISCHARGE_HGH  =  17  , // (1 << 1) << 16,
		FLAG_CHARGE_HGH     =  16  , // (1 << 0) << 16,

		FLAG_OVPWRRED       =  15  , // (1 << 7) << 8,
		FLAG_OVERLOAD       =  14  , // (1 << 6) << 8, --> 2
		FLAG_IN_DUVR        =  13  , // (1 << 5) << 8,
		FLAG_CHARGE_INH     =  12  , // (1 << 4) << 8,
		FLAG_DISCHARGE_INH  =  11  , // (1 << 3) << 8,
		FLAG_FULL_DISCHARGE =  10  , // (1 << 2) << 8,
		FLAG_CAPACITY_ACC   =  9   , // (1 << 1) << 8,
		FLAG_DISCHARGE      =  8   , // (1 << 0) << 8,

		ERR_CRITICAL        =  7  , // (1 << 7),
		ERR_CELL_TEMP_LOW   =  6  , // (1 << 6),
		ERR_CELL_TEMP_HIGH  =  5  , // (1 << 5),
		ERR_VOLTAGE_LOW     =  4  , // (1 << 4),
		ERR_VOLTAGE_HIGH    =  3  , // (1 << 3),
		ERR_CHARGE_PROT     =  2  , // (1 << 2),
		ERR_CHECKSUM        =  1  , // (1 << 1),
		ERR_SYSTEM_IRQ      =  0  , // (1 << 0)
	} Flags;

private:

	BluetoothSerial SerialBT;

	enum CONN_STATE {STATE_INIT, STATE_CONNECTING, STATE_CONNECTED, STATE_DISCONNECTED} ;
	CONN_STATE cstate = STATE_INIT;
	unsigned long lastUpdate = 0;

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
	uint8_t  batt_perc;		// in %
	int16_t int_temp;		// in 1/10°C (or K?)
	int16_t timeout;

    int16_t temperature;	// in 1/10°C
	uint32_t pressure;		// in Pa (1.013E05 -> normal pressure
	uint16_t height;		// in 1/10m
	int16_t  gradient;		// in ‰


	//uint16_t unknown[8];	// yet unkown values

	String bufferSerial;

	void updateDataFromString();
	void readFromSerial();

	bool connected = false;

	static uint8_t address[6];


public:
	float getVoltageTotal() const { return ( (batterie[0]+ batterie[1] + batterie[2]) / 1000.0);}
	float getSpeed() const { return (speed / 10.0);}
	float getGradient() const { return (gradient / 10.0);}
	float getHeight() const { return (height / 10.0);}

	//Batterie
	float getDynCurrent() const { return ((batt_current + cons_current)/ 1000.0) ; }
	float getDynPower() const { return (getDynCurrent() * getVoltageTotal()); }
	float getBatCurrent() const { return (batt_current / 1000.0 ); }
	float getConsCurrent() const { return ( cons_current / 1000.0); }


	int8_t getStage() const {return stufe;}
};

