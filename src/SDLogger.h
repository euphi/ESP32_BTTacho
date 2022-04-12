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


	void appendLog(float speed, float temp, float gradient, uint32_t distance, float height, uint8_t hr);

private:
	void listDir(const char *dirname, uint8_t levels);
	void deleteFile(const char *path);
	bool createDir(const char* dirname);

	String filename;
	unsigned int max_number = 0;

};

#endif /* SRC_SDLOGGER_H_ */

