/*
 * AtmESP32TouchButton.cpp
 *
 *  Created on: 27.03.2022
 *      Author: ian
 */

#include <AtmESP32TouchButton.h>


// static fields
ESP32Touch AtmESP32TouchButton::touchpins;
uint8_t AtmESP32TouchButton::thre = 95;

void AtmESP32TouchButton::diagnostics() {
	touchpins.diagnostics();
}
void AtmESP32TouchButton::setup() {
    touchpins.begin();
}


void AtmESP32TouchButton::initButton() {
    touchpins.configure_input(pin, thre, [this]() {Serial.printf("TouchPin %d pressed.", pin);});
    return;
}

bool AtmESP32TouchButton::isPressed() {
	return touchpins.isTouched(pin);

}

bool AtmESP32TouchButton::isReleased() {
	return !isPressed();
}
