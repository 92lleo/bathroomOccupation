#include "wcoccupationlight.h"

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

const char* ssid = "your-ssid";
const char* password = "your-password";

const int sensorSwitch = D3;

Light red    { "red", D5, false, false };
Light green  { "green", D6, false, false };

bool isOccupied = false;
bool isActive = true;

long lastSwitch = 0;
long lastReservation = 0;
long lastBlink = 0;
long lastCheck = 0;

const long reservationTimeout = 10*60*1000; // 10 min
const long activeTimeout = 2*60*60*1000; // 2 std
const long checkIntervall = 100; // 100 ms

const int readingCount = 5;
bool readings[readingCount];
int currentReading = 0;
const double readingOffset = 0.5;

const int sensorPin = D1;

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient client;

//The setup function is called once at startup of the sketch
void setup() {

	pinMode(sensorPin, INPUT);

	pinMode(red.switchPin, OUTPUT);
	pinMode(green.switchPin, OUTPUT);

	Serial.begin(9600);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(sensorSwitch, LOW);

	setLightOn(red, true);
	setLightOn(green, true);

	WiFi.mode(WIFI_STA);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);
	WiFi.hostname("toiletTrafficLight");

//	while (WiFi.status() != WL_CONNECTED) {
//		delay(500);
//		Serial.print(".");
//	}
//	Serial.println("");
//	Serial.println("WiFi connected");

	// web server stuff
	webServer.on("/", handleRoot);
	webServer.onNotFound(handleRoot);
	webServer.on("/json", handleJson);

	webServer.on("/set/red/on", []() {
		setLightOn(red, true);
		webServer.send(200, "text/plain", "");
	});
	webServer.on("/set/red/off", []() {
		setLightOn(red, false);
		webServer.send(200, "text/plain", "");
	});
	webServer.on("/set/green/on", []() {
		setLightOn(green, true);
		webServer.send(200, "text/plain", "");
	});
	webServer.on("/set/green/off", []() {
		setLightOn(green, false);
		webServer.send(200, "text/plain", "");
	});

	httpUpdater.setup(&webServer);

	webServer.begin();

	Serial.println("Server started");
	Serial.println(WiFi.localIP());

	// pre-fill array
	for(int i = 0; i < readingCount; i++){
		readSensor();
	}

	// set initial state
	if (getOccupied()) {
		// sensor reports occupied, but light was not changed yet
		isOccupied = true;
		lastSwitch = millis();
		setLightOn(red, true);
		setLightOn(green, false);
	} else {
		// sensor reports not occupied, but light was not changed yet
		isOccupied = false;
		lastSwitch = millis();
		setLightOn(red, false);
		setLightOn(green, true);
	}
}
// The loop function is called in an endless loop
void loop() {

	if (millis()-lastCheck > checkIntervall) {
		// check active timeout -> save energy
		if (isActive && !isOccupied && (millis() - lastSwitch > activeTimeout)) {
			setLightOn(green, false);
			isActive = false;
		}

		// check occupation, only act on changes
		readSensor();
		if(getOccupied() && !isOccupied){
			// sensor reports occupied, but light was not changed yet
			isOccupied = true;
			lastSwitch = millis();
			setLightOn(red, true);
			setLightOn(green, false);
			isActive = true;
		} else if(!getOccupied() && isOccupied){
			// sensor reports not occupied, but light was not changed yet
			isOccupied = false;
			lastSwitch = millis();
			setLightOn(red, false);
			setLightOn(green, true);
		}

		handleBlink();
		handleReservation();
		lastCheck = millis();
	}

	webServer.handleClient();
}

void readSensor(){
	// currentReading is not changed elsewhere, no check for oob
	readings[currentReading++] = readSensorRaw();
	if (currentReading > readingCount) {
		currentReading = 0;
	}
}

int readSensorRaw(){
	return digitalRead(sensorPin);
}

void setLightOn(Light& light, bool on){
	digitalWrite(light.switchPin, on ? LOW : HIGH);
	light.isOn = on;
}

void setLightBlinking(Light& light, bool on) {
	light.isBlinking = on;
}

void setAllBlinking(bool on){
	setLightBlinking(red, on);
	setLightBlinking(green, on);
}

bool getOccupied(){
	double result = 0;
	for(int i = 0; i < readingCount; i++){
		result += readings[i];
	}
	result = result / readingCount;
	return (result < readingOffset);
}

void handleBlink() {

}

void handleReservation() {

}

void handleRoot() {
	String response;

	response += "Toilet occupation<br><br>";
	String redOn = (red.isOn ? "on" : "off");
	response += "red light: "+ redOn + "<br>";
	String greenOn = (green.isOn ? "on" : "off");
	response += "green light: "+ greenOn + "<br>";
	response += "<br>";
	String occupied = (isOccupied ? "yes" : "no");
	response += "occupied: "+ occupied + "<br>";
	response += "<br>";
	if((red.isOn != green.isOn) && isActive){
		if(isOccupied){
			response += "occupied for ";
		} else {
			response += "free for ";
		}
		response += String((millis() - lastSwitch)/60/1000) + " minutes";
	}

	if (!isActive) {
		response += "inactive for "+ String((millis() - activeTimeout)/60/1000) + " minutes";
	}
	response += "<br><br>";

	response += "uptime: " + String(millis() / 60 / 60 / 1000) + " hours";
	if (isActive) {
		response += "<br>";
		response += "will go inactive in " + String(-(millis() - lastSwitch - activeTimeout) / 60 / 1000) + " minutes";
	}

	webServer.send(200, "text/html", response);
}

void handleJson() {
	String response;
	DynamicJsonDocument doc(1024);

	doc["red"] = red.isOn;
	doc["green"] = green.isOn;
	doc["occupied"] = isOccupied;
	doc["lastSwitchMinutes"] = ((millis() - lastSwitch) / 60 / 1000);
	doc["active"] = isActive;
	doc["uptimeMillis"] = millis();
	doc["timeUntilInactiveMinutes"] = (-(millis() - lastSwitch - activeTimeout) / 60 / 1000);

	serializeJson(doc, response);
	webServer.send(200, "application/json", response);
}
