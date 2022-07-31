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

	enum BLEDEV_CONN_STATE {
		IDLE,
		UNKOWN,
		KNOWN_NOTCONN,
		CONNECTING,
		CONNECTED,
		LOST
	};

	struct BLEDev {
		BLEDEV_CONN_STATE state = IDLE;
		BLEAddress* addr = 0;
		BLEClient* client = 0;
//		const BLEUUID charUUID;		//TODO: Fow now, to complicate to initialize
//		const BLEUUID serviceUUID;
	};

	unsigned long lastUpdate = 0;

	// Interface BLEAdvertisedDeviceCallbacks
	void onResult(BLEAdvertisedDevice advertisedDevice);

	// Interface ??
	void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

	uint8_t getHR() const {return hr;}

	bool isConnectedHRM() const {return hrm.client ? hrm.client->isConnected() : false ;}
	bool isConnectedCadence() const {return cadence.client ? cadence.client->isConnected() : false ;}

	// BLE
	// The remote HRM service we wish to connect to.
	static const BLEUUID serviceUUIDhrm;
	// The HRM characteristic of the remote service we are interested in.
	static const BLEUUID charUUIDhrm;
	static const BLEUUID serviceUUIDcadence;
	static const BLEUUID charUUIDcadence;

	static const BLEUUID serviceUUIbatterie;
	static const BLEUUID charUUIDbatterie;




private:
	Statistics& stats;
	uint8_t hr;

	bool connectToServer(BLEDev& device, const BLEUUID& serviceUUID, const BLEUUID& charUUID);

	static const int scanTime = 15; //In seconds
	BLEScan* pBLEScan;


	BLEAddress *pHrmAddress;
	BLEAddress *pCadenceAddress;
//	BLEClient  *pClient;
	bool doConnect = false;
	bool connected = false;
	bool notification = false;
	BLERemoteCharacteristic* pRemoteCharacteristic;
	bool simulation;

	bool doConnectCadence = false;
	bool cadenceConnected = false;

//	BLEDEV_CONN_STATE bleHrmState = IDLE;
//	BLEDEV_CONN_STATE bleCadenceState = IDLE;
//	bleHrmServiceID =
//	bleCadenceServiceID =

	BLEDev hrm;
	BLEDev cadence;


	void careForBLEDev(BLEDev & dev, const BLEUUID& serviceUUID, const BLEUUID& charUUID);


};
