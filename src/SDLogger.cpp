/*
 * SDLogger.cpp
 *
 *  Created on: 05.03.2022
 *      Author: ian
 */

#include <SDLogger.h>

#include "SPI.h"
#include <DateTime.h>

#include "esp_task_wdt.h"

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
    file_data = buffer;
    snprintf(buffer, sizeof(buffer), "/BTTacho/N_%04u.TXT", max_number);
	file_nmealog = buffer;

}


void SDLogger::appendLog(float speed, float temp, float gradient, uint32_t distance, float height, uint8_t hr) {
	LogData b;

	time_t now;
	time(&now);

	b.timestamp = now;
	b.speed = speed;
	b.temp = temp;
	b.grad = gradient;
	b.dist_m = distance;
	b.height = height;
	b.hr = hr;

	File f = SD.open(file_data, FILE_APPEND, true);
	if (!f) {
		Serial.println("Failed to open file for appending");
		return;
	}
	f.write((byte*) &b, sizeof(b));
	f.flush();
	f.close();
}

bool SDLogger::deleteFile(const String& path){
  if (SD.remove(path.c_str())) {
	  sdl.logf(Log_Info, TAG_SD, "🛈 💾 File %s deleted.", path.c_str());
	  return true;
  } else {
	  sdl.logf(Log_Warn, TAG_SD, "❌ 💾 Deletion of file %s failed.", path.c_str());
	  return false;
  }
}



void SDLogger::setup(){

  if(!SD.begin(5)){
	  sdl.log(Log_Warn, TAG_SD, "❌ 💾 Cannot initialize SD card reader!");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
	sdl.log(Log_Warn, TAG_SD, "❌ 💾 No SD card attached!");
    return;
  }

  sdl.logf(Log_Info, TAG_SD, "💾 SD Card Type: %s", (cardType == CARD_MMC) ? "MMC" : (cardType == CARD_SD) ? "SDSC" : (cardType == CARD_SDHC) ? "SDHC" :"UNKNOWN");

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  logf(SDLogger::Log_Info, SDLogger::TAG_SD, "SD Card Size: %lluMB\n)", cardSize);
  logf(SDLogger::Log_Info, SDLogger::TAG_SD, "Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  logf(SDLogger::Log_Info, SDLogger::TAG_SD, "Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  createDir("/BTTacho");
  autoCleanUp("/BTTacho");

  time_t now;
  time(&now);
  if (DateTime.isTimeValid()) { // Time is available
	  String time_str = DateFormatter::format("/BTTacho/%Y%m%d/L_%H%M%S.bin",now);
	  Serial.println(time_str);
	  file_data = time_str;
	  time_str = DateFormatter::format("/BTTacho/%Y%m%d/N_%H%M%S.bin",now);
//	  snprintf(buffer, sizeof(buffer)-1, "/BTTacho/%04d%02d%02d/N_%02d%02d%02d.txt", year(t), month(t), day(t), hour(t), minute(t), second(t));
	  file_nmealog = time_str;
	  time_str = DateFormatter::format("/BTTacho/%Y%m%d/D_%H%M%S.bin",now);
	  file_debuglog = time_str;
  } else {
	  listDir("/BTTacho", 0);
  }
  logf(SDLogger::Log_Info, SDLogger::TAG_SD, "🗎 - New file name: %s\n", file_data.c_str());
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
		//esp_task_wdt_reset();
		log(SDLogger::Log_Info, SDLogger::TAG_SD, file.name());
		if (file.name()[0] == 'N' || file.name()[0] == 'D') {
			file = root.openNextFile();   // next file in root-DIR
			continue;
		}
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
			rc = rc + " (" + file.size() + ")";
			rc = rc + " <a href=\"/del/"+uri+"\">DEL</a><br />\n";
		}
		file = root.openNextFile();   // next file in root-DIR
	}
}

void SDLogger::autoCleanUp(const char* root_name) {
	  File root = SD.open(root_name);
	  if(!root){
	    log(SDLogger::Log_Warn, SDLogger::TAG_SD, F("🧹 autoCleanUp - Failed to open directory"));
	    return;
	  }
	  if(!root.isDirectory()){
		  log(SDLogger::Log_Warn, SDLogger::TAG_SD, F("🧹 autoCleanUp - Not a directory"));
	    return;
	  }
	  cleanUp(root, 250);
}

bool SDLogger::cleanUp(File& root, uint32_t minsize) {
    File file = root.openNextFile();  // First file in root-DIR
    bool allFileDeleted = true;
	while (file) {
		if (file.isDirectory()) {
			bool dirClean = cleanUp(file, minsize);
			allFileDeleted &= dirClean;
			bool delOk = SD.rmdir(file.path());
			sdl.logf(delOk ? SDLogger::Log_Info : SDLogger::Log_Warn, SDLogger::TAG_SD, "🧹%s Directory %s empty. Deleting - %s", delOk ? "✅":"❌", file.name(), delOk ? "OK":"failed!");
		} else if (file.size() < minsize) {
			bool delOk = SD.remove(file.path());
			sdl.logf(delOk ? SDLogger::Log_Info : SDLogger::Log_Warn, SDLogger::TAG_SD, "🧹%s File %s to small (%d). Deleting - %s", delOk ? "✅":"❌", file.name(), file.size(), delOk ? "OK":"failed!");
		} else {
			allFileDeleted = false;
		}
		file = root.openNextFile();   // next file in root-DIR
	}
	return allFileDeleted;
}

void SDLogger::logf(LogType type, LogTag tag, const char* format, ...) const {
	if ( !(checkLogLevel(type, tag, true) || checkLogLevel(type, tag, false)) ) return;
	va_list arg;
	va_start(arg, format);
	char temp[256];
	size_t len = vsnprintf(temp, sizeof(temp), format, arg);
	if (len>=sizeof(temp)) Serial.println("❌ 📜 Logger: Log-String concatenated (too long)");
	va_end(arg);
	log(type, tag, String(temp));
}

void SDLogger::log(LogType type, LogTag tag, const String str) const {
	bool write_file = checkLogLevel(type, tag, true);
    bool write_serial = checkLogLevel(type, tag, false);
    if (!(write_file || write_serial)) return;

	File f;
	if (write_file) {
		String filename = (tag == TAG_RAW_NMEA) ? file_nmealog : file_debuglog;
		f = SD.open(filename, FILE_APPEND, true);
		if (!f) {
			Serial.printf("❌ 💾 Failed to open file %s for appending.\n", filename.c_str());
			write_file= false;
			write_serial = true;
		}
	}

    if (!(write_file || write_serial)) return;

	char buffer[18];
	time_t now;
	time(&now);

    String time_str = DateTime.format("%Y%m%d-%H%M%S: ");
	if (write_file) {
		f.print(time_str);
		f.println(str);
		f.flush();
		f.close();
	}
	if (write_serial) {
		Serial.print(buffer);
		Serial.println(str);
	}
}

bool SDLogger::createDir(const char *dirname) {
	bool rc = SD.mkdir(dirname);
	if (!rc) Serial.printf("❌ 💾 Failed to create dir %s.\n", dirname);
	return rc;
}

SDLogger sdl;
