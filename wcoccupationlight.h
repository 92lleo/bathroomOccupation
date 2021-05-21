#ifndef _wcoccupationlight_H_
#define _wcoccupationlight_H_

#include "Arduino.h"

struct Light {
	String name;
	int switchPin;
	bool isBlinking;
	bool isOn;
};

void setLightOn(Light& light, bool on);
void setLightBlinking(Light& light, bool on);
void setAllBlinking(bool on);

bool getLightOn(Light& light);
bool getLightBliking(Light& light);

void reserve();
bool getOccupied();

int readSensorRaw();
void readSensor();

void handleReservation();
void handleBlink();

void handleRoot();
void handleJson();
void handleSetLight();

#endif /* _wcoccupationlight_H_ */
