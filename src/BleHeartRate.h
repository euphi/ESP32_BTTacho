/*
 * BleHeartRate.h
 *
 *  Created on: 04.02.2022
 *      Author: ian
 */

#pragma once

#include <vector> // Arduino BLE somehow misses this include. So add it here.
#include <BLEScan.h>
#include <BLEUtils.h>

#include <Statistics.h>

class BleHeartRate: public BLEAdvertisedDeviceCallbacks {
public:
	BleHeartRate(Statistics& _stats);
	void setup();
	void loop();

	// TypeDef
//	typedef struct {
//	  char ID[20];
//	  uint16_t HRM;
//	}HRM;

//	enum {
//		IDLE,
//		SCAN_RUN,
//		SCAN_STOP,
//		DEV_KNOWN,
//		CONNECTED
//	} CONN_STATE;

	unsigned long lastUpdate = 0;

	// Interface BLEAdvertisedDeviceCallbacks
	void onResult(BLEAdvertisedDevice advertisedDevice);

	// Interface ??
	void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

	uint8_t getHR() const {return hr;}

	bool isConnected() const {return pClient ? pClient->isConnected() : false ;}

	// BLE
	// The remote HRM service we wish to connect to.
	static const BLEUUID serviceUUID;
	// The HRM characteristic of the remote service we are interested in.
	static const BLEUUID charUUID;


private:
	Statistics& stats;
	uint8_t hr;

	bool connectToServer(BLEAddress pAddress);

	static const int scanTime = 5; //In seconds
	BLEScan* pBLEScan;


	BLEAddress *pServerAddress;
	BLEClient  *pClient;
	bool doConnect = false;
	bool connected = false;
	bool notification = false;
	BLERemoteCharacteristic* pRemoteCharacteristic;
	bool simulation;

};
