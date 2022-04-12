/*
 * AtmESP32TouchButton.h
 *
 *  Created on: 27.03.2022
 *      Author: ian
 */

#pragma once

#include <Atm_button.hpp>
#include <esp32_touch.hpp>


class AtmESP32TouchButton: public Atm_button {
// Static part
public:
          static void diagnostics();
          static void setup();
private:
          static ESP32Touch touchpins;
          static uint8_t thre;

// object part

public:
protected:
          virtual void initButton() override;
          virtual bool isPressed() override;
          virtual bool isReleased() override;


};
