
#include <BluetoothSerial.h>
//#include <BTAdvertisedDevice.h>
#include <SSD1306Wire.h>

#ifdef USE_BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#endif

#include <BtClassicForumsLader.h>

//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
//#include "esp_log.h"


BluetoothSerial SerialBT;
BtClassicForumsLader forumslader;

//#include "esp_bt_main.h"
//#include "esp_bt_device.h"
//#include"esp_gap_bt_api.h"
//#include "esp_err.h"
//
//#define REMOVE_BONDED_DEVICES 1   // <- Set to 0 to view all bonded devices addresses, set to 1 to remove
//
//#define PAIR_MAX_DEVICES 20
//uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
//char bda_str[18];
//
//bool initBluetooth()
//{
//  if(!btStart()) {
//    Serial.println("Failed to initialize controller");
//    return false;
//  }
//
//  if(esp_bluedroid_init() != ESP_OK) {
//    Serial.println("Failed to initialize bluedroid");
//    return false;
//  }
//
//  if(esp_bluedroid_enable() != ESP_OK) {
//    Serial.println("Failed to enable bluedroid");
//    return false;
//  }
//  return true;
//}
//
//char *bda2str(const uint8_t* bda, char *str, size_t size)
//{
//  if (bda == NULL || str == NULL || size < 18) {
//    return NULL;
//  }
//  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
//          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
//  return str;
//}
//
//void remove_devs()
//{
//
//  initBluetooth();
//  Serial.print("ESP32 bluetooth address: "); Serial.println(bda2str(esp_bt_dev_get_address(), bda_str, 18));
//  // Get the numbers of bonded/paired devices in the BT module
//  int count = esp_bt_gap_get_bond_device_num();
//  if(!count) {
//    Serial.println("No bonded device found.");
//  } else {
//    Serial.print("Bonded device count: "); Serial.println(count);
//    if(PAIR_MAX_DEVICES < count) {
//      count = PAIR_MAX_DEVICES;
//      Serial.print("Reset bonded device count: "); Serial.println(count);
//    }
//    esp_err_t tError =  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
//    if(ESP_OK == tError) {
//      for(int i = 0; i < count; i++) {
//        Serial.print("Found bonded device # "); Serial.print(i); Serial.print(" -> ");
//        Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
//        if(REMOVE_BONDED_DEVICES) {
//          esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
//          if(ESP_OK == tError) {
//            Serial.print("Removed bonded device # ");
//          } else {
//            Serial.print("Failed to remove bonded device # ");
//          }
//          Serial.println(i);
//        }
//      }
//    }
//  }
//}

// ------------------------------------------------------------------------------------------------------------


bool confirmRequestPending = true;

#ifdef USE_BLE
int scanTime = 5; //In seconds
BLEScan* pBLEScan;
#endif


SSD1306Wire display(0x3c, SDA, SCL);  

// TypeDef
typedef struct {
  char ID[20];
  uint16_t HRM;
}HRM;
HRM hrm;

// BT CLassic

//uint8_t address[6] = { 0xd0, 0x5a, 0xfd, 0x7f, 0xfc, 0xc0};	// Ianis REALME
uint8_t address[6] = { 0x20, 0x13, 0x01, 0x18, 0x02, 0x26 }; // Forumslader

//uint8_t address[6] = { 0x00, 0x04, 0x61, 0x81, 0xDD, 0xF3};
bool ClassicConnected = false;

#ifdef BT_SCAN
void btAdvertisedDeviceFound(BTAdvertisedDevice* pDevice) {
	Serial.printf("ℹ️ Found a classic BT device asynchronously: %s\n", pDevice->toString().c_str());
}
#endif


// BLE
#ifdef USE_BLE

// BLE
// The remote HRM service we wish to connect to.
static  const BLEUUID serviceUUID(BLEUUID((uint16_t)0x180D));
// The HRM characteristic of the remote service we are interested in.
static  const BLEUUID    charUUID(BLEUUID((uint16_t)0x2A37));

static BLEAddress *pServerAddress;
static bool doConnect = false;
static bool connected = false;
static bool notification = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      // We have found a device, let us now see if it contains the service we are looking for.
       if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {

         //
         Serial.print(F("Found our device!  address: "));
         advertisedDevice.getScan()->stop();

         pServerAddress = new BLEAddress(advertisedDevice.getAddress());
         doConnect = true;

       } // Found our server
    }
};


//--------------------------------------------------------------------------------------------
// BLE notifyCallback
//--------------------------------------------------------------------------------------------
static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {

  if (bitRead(pData[0], 0) == 1) {
    Serial.println(F("16bit HeartRate Detected"));
  } else {
    Serial.println(F("8bit HeartRate Detected"));
  }

  uint32_t* pData32 = (uint32_t*)pData;
  hrm.HRM = pData[1];
  Serial.printf("Data with length %d bytes: %d [%x]\n", length, hrm.HRM, *pData32);
  if (length == 2) {
    Serial.print("Heart Rate ");
    Serial.print(hrm.HRM, DEC);
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

//--------------------------------------------------------------------------------------------
//  Connect to BLE HRM
//--------------------------------------------------------------------------------------------
bool connectToServer(BLEAddress pAddress) {
    Serial.print(F("Forming a connection to "));
    Serial.println(pAddress.toString().c_str());

    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(F(" - Created client"));

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
    pRemoteCharacteristic->registerForNotify(notifyCallback);
    return true;
}

#endif

void setup()
{
  Serial.begin(115200);
  SerialBT.begin("ESP32test", true); //Bluetooth device name
  SerialBT.enableSSP();
  SerialBT.setPin("1234");
  //SerialBT.setPin("0000");
  Serial.println("The device started in master mode, make sure remote BT device is on!");

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Hello world");
  display.display();

#ifdef BT_SCAN
  Serial.print("Starting discoverAsync...");
  if (SerialBT.discoverAsync(btAdvertisedDeviceFound)) {
      Serial.println("Findings will be reported in \"btAdvertisedDeviceFound\"");
      delay(10000);
      Serial.print("Stopping discoverAsync... ");
      SerialBT.discoverAsyncStop();
      Serial.println("stopped");
  } else {
      Serial.println("Error on discoverAsync f.e. not workin after a \"connect\"");
  }
#endif

  SerialBT.setPin("1234");
  ClassicConnected = SerialBT.connect(address);
  if (ClassicConnected) {
	  Serial.println("Connect success");
  } else {
    while(!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
#ifdef USE_BLE
  BLEDevice::init("ESP32test BLE");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
#endif
}

void loop() {
	static uint32_t lastDisplayUpdate = 0;
	forumslader.loop();
	yield();
	if (lastDisplayUpdate + 100 < millis()) {
		lastDisplayUpdate = millis();
		display.clear();
		display.setFont(ArialMT_Plain_24);
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		// Speed - TODO: Extract method
		String speedStr;
		speedStr = String(forumslader.getSpeed(), 1);
		display.drawString(100, 40, speedStr);
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(100, 40, "km");
		display.drawHorizontalLine(100, 51, 12);
		display.drawString(105, 52, "h");

		// Batterie - TODO: Extract method
		display.setFont(ArialMT_Plain_10);
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawString(60, 0, String(forumslader.getVoltageTotal(), 1)+"V");
		display.drawProgressBar(99, 0, 28, 8, static_cast<uint8_t>((forumslader.getVoltageTotal() - 11.5) / (12.5-11.5) * 100));
		Serial.println(static_cast<uint8_t>((forumslader.getVoltageTotal() - 11.5) / 12.5 * 100));
		Serial.println(forumslader.getVoltageTotal());

		// Level - TODO: Extract method
		int8_t stage = forumslader.getStage();
		//Simulation: stage = (millis() / 1000) % 5;
		for (int_fast8_t c = 0; c<4; c++) {
			if (stage >c) {
				display.fillCircle(123, 59-(8*c), 4);
			} else {
				display.drawCircle(123, 59-(8*c), 4);
			}
		}

		// Gradient - TODO: Extract method
		display.setFont(ArialMT_Plain_16);
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		display.drawString(60, 0, String(forumslader.getGradient(), 1)+"%");


		//display.drawString(40, 32, String(hrm.HRM));
		// write the buffer to the display
		display.display();
	}
	delay(10);
#ifdef USE_BLE
	if (doConnect) {
	    if (connectToServer(*pServerAddress)) {
	      Serial.println(F("We are now connected to the BLE HRM"));
	      connected = true;
	    } else {
	      Serial.println(F("We have failed to connect to the HRM; there is nothin more we will do."));
	    }
	    doConnect = false;
	} else if (connected) {
		//
		yield();
	} else {
		BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
		Serial.print("Devices found: ");
		Serial.println(foundDevices.getCount());
		Serial.println("Scan done!");
		pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
	}
#endif

	//delay(100);

}
