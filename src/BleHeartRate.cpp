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
const BLEUUID BleHeartRate::serviceUUID = BLEUUID((uint16_t)0x180D);
// The HRM characteristic of the remote service we are interested in.
const BLEUUID BleHeartRate::charUUID = BLEUUID((uint16_t)0x2A37);


// --------------------------------------------------------------------------------------------------------------------
// FIXME: Should be part of class
bool scanning = false;
void scanCompleteCB(BLEScanResults result) {
	Serial.print("BLE scan completed: ");
	Serial.println(result.getCount());
	scanning = false;
}
// --------------------------------------------------------------------------------------------------------------------


BleHeartRate::BleHeartRate(Statistics& _stats): stats(_stats), simulation(false) {
	// empty ctor
}

// Interface BLEAdvertisedDeviceCallbacks
void BleHeartRate::onResult(BLEAdvertisedDevice advertisedDevice) {
	Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
	// We have found a device, let us now see if it contains the service we are looking for.
	if (advertisedDevice.haveServiceUUID()  && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
		Serial.print(F("Found our device!  address: "));
		advertisedDevice.getScan()->stop();

		//Also stop pBLEScan--- // TODO: Check if correct. Stopped twice?
		pBLEScan->stop();
		pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
		scanning = false;

		pServerAddress = new BLEAddress(advertisedDevice.getAddress());
		doConnect = true;
	} // Found our server
}



//--------------------------------------------------------------------------------------------
// BLE notifyCallback
//--------------------------------------------------------------------------------------------
void BleHeartRate::notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
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
	if (doConnect) {
	    if (connectToServer(*pServerAddress)) {
	      Serial.println(F("We are now connected to the BLE HRM"));
	      connected = true;
	    } else {
	      Serial.println(F("We have failed to connect to the HRM; there is nothing more we will do."));
	    }
	    doConnect = false;
	} else if (connected) {
		if (!pClient->isConnected()) {
			Serial.println("No longer connected - restart scanning");
			connected = false;
			hr = 0;
		}
		yield();
	} else if (!scanning) {
		pBLEScan->stop();
		pBLEScan->clearResults();
		Serial.println("BLE: Restart scan");
		pBLEScan->start(scanTime, scanCompleteCB); // Non-Blocking
		scanning = true;
	}
	if (simulation) {
		hr = 50 + (millis()/1000) % 150;
	}
}


//--------------------------------------------------------------------------------------------
//  Connect to BLE HRM
//--------------------------------------------------------------------------------------------
bool BleHeartRate::connectToServer(BLEAddress pAddress) {
    Serial.print(F("Forming a connection to "));
    Serial.println(pAddress.toString().c_str());

    if (!pClient) {
    	pClient  = BLEDevice::createClient();
    	Serial.println(F(" - Created client"));
    } else {
    	Serial.println(F(" - reusing existing client"));
    }

    // Connect to the HRM BLE Server.
    pClient->connect(pAddress);
    Serial.println(F(" - Connected to server"));

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      //Serial.println(serviceUUID.toString().c_str());
      return false;
    }
    Serial.println(F(" - Found our service"));

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



