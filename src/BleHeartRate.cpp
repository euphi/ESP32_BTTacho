/*
 * BleHeartRate.cpp
 *
 *  Created on: 04.02.2022
 *      Author: ian
 */

#include <BleHeartRate.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

// Init static members
// HRM service
const BLEUUID BleHeartRate::serviceUUIDhrm = BLEUUID((uint16_t)0x180D);
// The HRM characteristic of the remote service we are interested in.
const BLEUUID BleHeartRate::charUUIDhrm = BLEUUID((uint16_t)0x2A37);

const BLEUUID BleHeartRate::serviceUUIDcadence = BLEUUID((uint16_t)0x1816);
const BLEUUID BleHeartRate::charUUIDcadence = BLEUUID((uint16_t)0x2A5B);


const BLEUUID BleHeartRate::serviceUUIbatterie = BLEUUID((uint16_t)0x180F);
const BLEUUID BleHeartRate::charUUIDbatterie =  BLEUUID((uint16_t)0x2902);

// --------------------------------------------------------------------------------------------------------------------
// FIXME: Should be part of class
bool scanning = false;
void scanCompleteCB(BLEScanResults result) {
	Serial.print("BLE scan completed: ");
	Serial.println(result.getCount());
	scanning = false;
}
// --------------------------------------------------------------------------------------------------------------------


/**
 * Basic Flow
 * 						State variables:     | rel. Objects      | scanning | doConnect    |
 * 																			 doConnectCadence
 *                                           |                   |          |              |
 * ctor:                                     |                   |          |              |
 *      0.                                   |                   |    f     |  f           |
 * setup():                                  |                   |          |              |
 * 		1. Start background scan for devices | pBLEScan          |  f -> t  |              |
 *                                           |                   |          |              |
 * onResult()                                | advertisedDevice  |          |              |
 *      2. Stop scannning (FIXME!)           |                   | t -> f   |              |   FIXME: Wait for all searched devices or timeout.
 *      3. Create server adress (cad or hrm) | pCadenceAddress
 *            (no start of connection yet)     pServerAddress    |          | f -> t       |
 *                                           |                   |          |              |
 * loop():                                   |                   |          |              |
 *      4a) connect to HR                    |                   |          |              |
 *      4b) connect to HR                    |                   |          |              |
 *                                           |                   |          |              |
 *                                           |                   |          |              |
 *                                           |                   |          |              |
 */

BleHeartRate::BleHeartRate(Statistics& _stats): stats(_stats), simulation(false) {
	// empty ctor
}

// Interface BLEAdvertisedDeviceCallbacks
void BleHeartRate::onResult(BLEAdvertisedDevice advertisedDevice) {
	Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
	// We have found a device, let us now see if it contains the service we are looking for.
	if (advertisedDevice.haveServiceUUID()) {
		if (advertisedDevice.getServiceUUID().equals(serviceUUIDcadence)) {
			Serial.print(F("Found cadence device!  address: "));
			cadence.addr = new BLEAddress(advertisedDevice.getAddress()); //FIXME: find correct BLEDev
		    Serial.println(cadence.addr ->toString().c_str());
			cadence.state = KNOWN_NOTCONN;
		} else if (advertisedDevice.getServiceUUID().equals(serviceUUIDhrm)) {
			Serial.print(F("Found hrm device!  address: "));
			hrm.addr = new BLEAddress(advertisedDevice.getAddress());
			Serial.println(hrm.addr ->toString().c_str());
			hrm.state = KNOWN_NOTCONN;
		} else {
			Serial.printf("Unknown Service %s.\n", advertisedDevice.getServiceUUID().toString().c_str());
		}
	}
	if (scanning && hrm.addr != 0 && cadence.addr != 0) {
		scanning = false;
		Serial.println("Stopped scanning because both devices found!");
		pBLEScan->stop();
		pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
	}
//	bool stopScan = false; // FIXME
//	if (stopScan) {
//		advertisedDevice.getScan()->stop();
//
//		//Also stop pBLEScan--- // TODO: Check if correct. Stopped twice?
//		pBLEScan->stop();
//		pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
//		scanning = false;
//
//	}
}



//--------------------------------------------------------------------------------------------
// BLE notifyCallback
//--------------------------------------------------------------------------------------------
void BleHeartRate::notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.printf("BLE Callback with Characteristic %s.\n", pBLERemoteCharacteristic->readValue().c_str());
	lastUpdate = millis();
  if (bitRead(pData[0], 0) == 1) {
    Serial.println(F("16bit HeartRate Detected"));
  } else {
    Serial.println(F("8bit HeartRate Detected"));
  }

  uint32_t* pData32 = (uint32_t*)pData;
  hr = pData[1];
  Serial.printf("Data with length %d bytes: %d [%x]\n", length, hr, *pData32);
  if (length == 2) {
    Serial.print("Heart Rate ");
    Serial.print(hr, DEC);
    Serial.println("bpm");
  }
  //
  bool data8Bit = (pData[0] & 1) == 0; // lowest Bit 1 --> 16 Bit
  uint8_t contact = (pData[0] >> 1) & 3; // Bit 1&2 --> Contact
  switch (contact) {
  case 2:
	  Serial.println("No contact");
	  break;
  case 3:
	  Serial.println("Contact");
	  break;
  default:
	  Serial.printf("Contact not supported [%x]\n", contact);
  }
  bool ee_status =  ((pData[0] >> 3) & 1) == 1;  // Bit 3 --> EE
  bool rr_interval = ((pData[0] >> 4) & 1) == 1;  // Bit 4 --> RR

  uint16_t hr;
  if (data8Bit) {
      hr =  pData[1];
  } else {
      hr = (pData[2] << 8) | pData[1];
  }
  Serial.printf("%sbit Data HR: %d bpm - EE: %s - RR: %s\n", data8Bit?"8":"16", hr, ee_status?"true":"false", rr_interval?"true":"false");


//  if res["ee_status"]:
//      res["ee"] = (data[i + 1] << 8) | data[i]
//      i += 2
//
//  if res["rr_interval"]:
//      res["rr"] = []
//      while i < len(data):
//          # Note: Need to divide the value by 1024 to get in seconds
//          res["rr"].append((data[i + 1] << 8) | data[i])
//          i += 2
//
//  return res

}


void BleHeartRate::careForBLEDev(BLEDev & dev, const BLEUUID& serviceUUID, const BLEUUID& charUUID) {
	if (dev.state == KNOWN_NOTCONN) {
		dev.state = CONNECTING;
		if (connectToServer(dev, serviceUUID, charUUID)) {  // BLOCKING!
	      Serial.println(F("We are now connected to the BLE HRM"));
	      dev.state = CONNECTED;
	    } else {
	      dev.state = LOST;
	      Serial.println(F("We have failed to connect to the BLE Device; there is nothing more we will do."));
	    }

	}
}


void BleHeartRate::setup() {
  BLEDevice::init("ESP32_BTTacho BLE");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(this);
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  pBLEScan->start(scanTime, scanCompleteCB); // Non-Blocking
  scanning = true;
}

void BleHeartRate::loop() {
	if (lastUpdate + 10000 < millis()) {	// Timeout
		hr = 0;
	}
	if (!scanning) {
		careForBLEDev(hrm, serviceUUIDhrm, charUUIDhrm);
		careForBLEDev(cadence, serviceUUIDcadence, charUUIDcadence);
	}

//	if (doConnect) {
//	    if (connectToServer(*pServerAddress)) {
//	      Serial.println(F("We are now connected to the BLE HRM"));
//	      connected = true;
//	    } else {
//	      Serial.println(F("We have failed to connect to the HRM; there is nothing more we will do."));
//	    }
//	    doConnect = false;
//	} else if (connected ....



//	if (connected) {
//		if (!pClient->isConnected()) {
//			Serial.println("No longer connected - restart scanning");
//			connected = false;
//			hr = 0;
//		}
//		yield();
//	} else if (!scanning) {
//		pBLEScan->stop();
//		pBLEScan->clearResults();
//		Serial.println("BLE: Restart scan");
//		pBLEScan->start(scanTime, scanCompleteCB); // Non-Blocking
//		scanning = true;
//	}
	if (simulation) {
		hr = 50 + (millis()/1000) % 150;
	}
//	if (doConnectCadence) {
//		if (connectToServer(*pCadenceAddress)) {
//		      Serial.println(F("We are now connected to the BLE Cadence"));
//		      cadenceConnected = true;
//		} else {
//		      Serial.println(F("Failed to connect to the Cadence."));
//		}
//		doConnectCadence = false;
//	}
}


//--------------------------------------------------------------------------------------------
//  Connect to BLE HRM
//--------------------------------------------------------------------------------------------
bool BleHeartRate::connectToServer(BLEDev& device, const BLEUUID& serviceUUID, const BLEUUID& charUUID) {
    Serial.print(F("Forming a connection to "));
    Serial.println(device.addr->toString().c_str());

    if (!device.client) {
    	device.client  = BLEDevice::createClient();
    	Serial.println(F(" - Created client"));
    } else {
    	Serial.println(F(" - reusing existing client"));
    }

    // Connect to the HRM BLE Server.
    device.client ->connect(*device.addr);
    Serial.println(F(" - Connected to server"));

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = device.client->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      //Serial.println(serviceUUID.toString().c_str());
      return false;
    }

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      //Serial.println(charUUID.toString().c_str());
      return false;
    }
    Serial.println(F(" - Found our characteristic"));

    // Register for Notify
    pRemoteCharacteristic->registerForNotify([&](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {notifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);});
    return true;
}



