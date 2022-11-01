/*
 * pindef.h
 *
 *  Created on: 31.07.2022
 *      Author: ian
 */

#pragma once

#include <stdint.h>

// ESP32 touch API uses different numbering. So this is the "touch number", not the GPIO number.
// To avoid confusing, the GPIO number is written as comment

static const uint8_t TOUCH_L  = 2; // GPIO2;
static const uint8_t TOUCH_DN = 8; // GPI33;
static const uint8_t TOUCH_UP = 0; // GPIO4;
static const uint8_t TOUCH_R  = 1; // GPIO0;

static const uint8_t PB_0  = 25;		// Pusbbutton on case
static const uint8_t LED_0 = 32;

static const uint8_t LEDSTRIP_0 = 27;
