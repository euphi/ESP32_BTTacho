/*
 * SDLogger.cpp
 *
 *  Created on: 05.03.2022
 *      Author: ian
 */

#include <SDLogger.h>

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-microsd-card-arduino/

  This sketch can be found at: Examples > SD(esp32) > SD_Test
*/


#include "SPI.h"
#include <Singletons.h>
#include <TimeLib.h>


void SDLogger::listDir(const char * dirname, uint8_t levels){
  Serial.printf("📁 Listing directory: %s\n", dirname);

  File root = SD.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }


  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
      unsigned int filenumber;
      if (sscanf(file.name(),"LOG_%4u.BIN", &filenumber) == 1) {
    	  Serial.printf("File number %d.\n", filenumber);
    	  if (filenumber > max_number) max_number = filenumber;
      }
    }
    file = root.openNextFile();
  }
    Serial.printf("!!! - MAX File number %d.\n", max_number++);
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "/BTTacho/LOG_%04u.BIN", max_number);
    filename = buffer;
}


void SDLogger::appendLog(float speed, float temp, float gradient, uint32_t distance, float height, uint8_t hr) {
	LogData b;
	char buffer[16];
	time_t t = now();
	snprintf(buffer, sizeof(buffer)-1, "%04d%02d%02d%02d%02d%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
	Serial.println(buffer);

	b.timestamp = t;
	b.speed = speed;
	b.temp = temp;
	b.grad = gradient;
	b.dist_m = distance;
	b.height = height;
	b.hr = hr;

	File f = SD.open(filename, FILE_APPEND, true);
	if (!f) {
		Serial.println("Failed to open file for appending");
		return;
	}
	f.write((byte*) &b, sizeof(b));
	f.flush();
	f.close();
}

void SDLogger::deleteFile(const char * path){
  Serial.printf("Deleting file: %s\n", path);
  if(SD.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}



void SDLogger::setup(){

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  createDir("/BTTacho");

  time_t t = now();
  if (year(t)>=2000) { // Time is available
	  char buffer[40];
	  snprintf(buffer, sizeof(buffer)-1, "/BTTacho/%04d%02d%02d/L_%02d%02d%02d.bin", year(t), month(t), day(t), hour(t), minute(t), second(t));
	  Serial.println(buffer);
	  filename = buffer;
  } else {
	  listDir("/BTTacho", 0);
  }
  Serial.printf("🗎 - New file name: %s\n", filename.c_str()); ;
}

uint16_t SDLogger::getAllFileLinks(String &rc) const {
	rc += "<html><body><h1>Logfiles</h1>\n<p>\n";
	File root = SD.open("/BTTacho");

	  if(!root){
	    rc += "Failed to open directory";
	    Serial.println("500 - Can't open file/dir");
	    return 500;
	  }
	  if(!root.isDirectory()){
	    rc += "Not a directory";
	    Serial.println("500 - Not a directory");
	    return 500;
	  }
	  getFileHTML(rc, root, 8);  // 8 chars for "/BTTacho"

	rc += "</p></body></html>";
	return 200;
}

void SDLogger::getFileHTML(String &rc, File &root, uint8_t strip_front) const {
    File file = root.openNextFile();  // First file in root-DIR

	while (file) {
		Serial.println(file.name());
		Serial.flush();
		String filehtml;
		if (file.isDirectory()) {
			rc += "<h2>";
			rc += file.name();
			rc += "</h2>\n";
			String fh;
			getFileHTML(fh, file, strip_front);
			rc += fh;
			rc += "\n";
		} else {
			String uri= (file.path()+ strip_front+1);
			rc = rc + "<a href=\"/log/"+uri+"\">" + uri + "</a>";
			rc = rc + " (" + file.size() + ") <br />\n";
		}
		file = root.openNextFile();   // next file in root-DIR
	}
}


bool SDLogger::createDir(const char *dirname) {
	bool rc = SD.mkdir(dirname);
	if (!rc) Serial.printf("Failed to create dir %s.\n", dirname);
	return rc;
}
