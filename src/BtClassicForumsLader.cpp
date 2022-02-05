/*
 * BtClassicForumsLader.cpp
 *
 *  Created on: 04.02.2022
 *      Author: ian
 */

#include <BtClassicForumsLader.h>

#include <inttypes.h>

BtClassicForumsLader::BtClassicForumsLader() {
	// TODO Auto-generated constructor stub

}

void BtClassicForumsLader::updateDataFromString() {
	Serial.print("ℹ️ Rvcd string from BT: ");
	Serial.println(bufferSerial);
	int8_t scanCt = 0;
	float speed_f = NAN;
	if (bufferSerial.startsWith("$FL")) {
		switch(bufferSerial.charAt(3)) { //                                                                                                                 Strom in mA? // Verbraucherstrom?                     // int. Temp?
		case '5':  // $FL5,08c800,0,0,4158,4161,4162,-18,0,294,1,233,3679,13231,25897;   $FL5,08c800, 0,           0,          4158,        4161,        4162,       -18,         0,          294,        1,          233,        3679,     13231,         25897;
			uint16_t pulses;
			int32_t  timecounter;
			int32_t  pulsecounter;
			int32_t  micropulsecounter;
			scanCt = sscanf(bufferSerial.c_str(), "$FL5,%hx,%hhd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%d,%d,%d\n", &err_flags, &stufe, &pulses, &batterie[0],&batterie[1],&batterie[2],&unknown[2],&unknown[3],&unknown[3],&unknown[4],&unknown[5],&micropulsecounter,&pulsecounter,&timecounter);
			if (scanCt != 14) Serial.println("❌ Not all fields scanned");
			Serial.printf("[%x] Batteries: %dmV %dmV %dmV\n", scanCt, batterie[0], batterie[1], batterie[2]);
			speed_f = pulses * hmh_per_pulse;
			speed = static_cast<uint16_t>(speed_f);
			Serial.printf("Speed: %.1f - %d ", speed_f, speed);
			break;
		case 'B': // $FLB,850,98591,2731,0;
			scanCt = sscanf(bufferSerial.c_str(), "$FLB,%hd,%d,%hd,%hd\n", &temperature, &pressure,&height,&gradient);
			if (scanCt != 4) Serial.println("❌ Not all fields scanned");
			Serial.printf("[%x] Temp: %.02f\tPressure: %.01f\tHeight: %.01f\tGradient: %d\n", scanCt, temperature/10.0, pressure/100.0, height/10.0, gradient);
			break;
		case 'C':   // $FLC,0,0,0,200,3798,26;
					// $FLC,1,0,0,200,3798,26;
				    // $FLC,2,478561,0,200,0,200;
					// $FLC,3,8126,0,0,1799,0;
			      	// $FLC,4,48,48,0,0,0;
				  	// $FLC,5,1870,100,1667,36,73053775;

			// Ignore for now (Total data)
			break;
		default:
			Serial.printf("❌ Unknown FL-ID '%c'\n", bufferSerial.charAt(3));
		}

	} else {
		Serial.println("❌ Unknown String identifier");
	}
	bufferSerial.clear();
}

void BtClassicForumsLader::readFromSerial() {
	while (SerialBT.available()) {
		char read = SerialBT.read();
		if (read == '$') {
			updateDataFromString();
		}
		bufferSerial += read;

	}
}

void BtClassicForumsLader::loop() {
	readFromSerial();
}
