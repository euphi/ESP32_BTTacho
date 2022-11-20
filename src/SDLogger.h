/*
 * SDLogger.h
 *
 *  Created on: 05.03.2022
 *      Author: ian
 */

#ifndef SRC_SDLOGGER_H_
#define SRC_SDLOGGER_H_

#include "FS.h"
#include "SD.h"

class SDLogger {
public:
	SDLogger() {}	;
	void setup();

	struct LogData {
		time_t timestamp;					//        4
		float speed;						// + 4 =  8
		float temp;  						// + 4 = 12
		float grad;							// + 4 = 16
		float height;						// + 4 = 20
		uint32_t dist_m;					// + 4 = 24
		uint8_t hr : 8;						// + 1 = 25
		uint8_t pad_count: 8;				// + 1 = 26
	};

	enum LogType {
		Log_Debug = 1,
		Log_Info,
		Log_Warn
	};

	enum LogTag {
		TAG_RAW_NMEA = 0,
		TAG_FL,
		TAG_HR,
		TAG_STAT,
		TAG_WIFI,
		TAG_SD,
		TAG_OP,
		TAG_LAST
	};


	void appendLog(float speed, float temp, float gradient, uint32_t distance, float height, uint8_t hr);

	uint16_t getAllFileLinks(String& rc) const;
	void     getFileHTML(String& rc, File& root, uint8_t strip_front) const;
	bool deleteFile(const String& path);
	void autoCleanUp(const char* root_name);
	bool cleanUp(File& root, uint32_t minsize);
	void log(LogType type, LogTag tag, const String str) const;
	void logf(LogType type, LogTag tag, const char* format, ...) const;


	inline bool checkLogLevel(LogType type, LogTag tag, bool write_file = false) const {return loglevel[write_file?1:0][tag] <= type;}
private:
	void listDir(const char *dirname, uint8_t levels);
	bool createDir(const char* dirname);

	//                             TAG_RAW_NMEA TAG_FL    TAG_HR    TAG_STAT  TAG_WIFI 	TAG_SD    TAG_OP,
	LogType loglevel[2][TAG_LAST] = {{Log_Info, Log_Info, Log_Info, Log_Info, Log_Info, Log_Info, Log_Debug},    // Terminal
			                         {Log_Info, Log_Info, Log_Info, Log_Info, Log_Info, Log_Info, Log_Info}};   // File


	String file_data;
	String file_debuglog;
	String file_nmealog;

	unsigned int max_number = 0;

};

extern SDLogger sdl;


#endif /* SRC_SDLOGGER_H_ */

