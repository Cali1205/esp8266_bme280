#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/BME280/BME280.h>
//Connections: GPIO0/D3 SCL, GPIO2/D4 SDA
// 3.3V Supply

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
	#define WIFI_SSID "<SSID>" // Put you SSID and Password here
	#define WIFI_PWD "<WIFI_PWD>"
#endif

#define BROKER "<server>"   // We can use a hostname! -> "mqtt.myserver.com"
#define BRPORT 1883
#define BRUSER "<User>"               // Empty string if broker doesn't use authentication
#define BRPWD  "<Password>"


void onMqttReceived(String topic, String message); // Forward declaration for our callback
void connectOk();
void BME280_readout();

String currentPressure_str;
String currentTemperature_str;
String currentHumidity_str;

// Time to sleep (in seconds):
const int sleepTimeS = 30;

Timer procTimer;

// MQTT client
// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient mqtt(BROKER, BRPORT, onMqttReceived);

BME280 environment;

// Publish our message
void publishMessage()
{
	if (mqtt.getConnectionState() != eTCS_Connected)
		connectOk(); // Auto reconnect
    BME280_readout();
	Serial.println("Let's publish message now!");
	bool a = mqtt.publish("weather_1/temperature", currentTemperature_str);
	bool b = mqtt.publish("weather_1/pressure", currentPressure_str);
	bool c = mqtt.publish("weather_1/humidity", currentHumidity_str);

//	if (a == 1 & b == 1 & c == 1)
//		delay(1000);
//    	system_deep_sleep(60000000);

}

// Callback for messages, arrived from MQTT server
void onMqttReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Prettify alignment for printing
	Serial.println(message);
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.println("I'm CONNECTED");

	// Run MQTT client
    mqtt.connect("esp8266_1", BRUSER , BRPWD );
	mqtt.subscribe("status/#");

	// Start publishing loop
	//publishMessage();
	procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds

	// 5min deepsleep system_deep_sleep(time in µs)
	//system_deep_sleep(30000000);
}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	Serial.println("I'm NOT CONNECTED. Need help :(");

	// .. some you code for device configuration ..
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	Wire.begin();

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 20, connectFail); // We recommend 20+ seconds for connection timeout at start


	//publishMessage();
	//procTimer.initializeMs(20 * 1000, publishMessage).start(); // every 20 seconds

}


void BME280_readout()
{

	if(!environment.EnsureConnected())
		Serial.println("Could not connect to BME280.");

	// When we have connected, we reset the device to ensure a clean start.
   //barometer.SoftReset();
   // Now we initialize the sensor and pull the calibration data.
	environment.Initialize();
	//barometer.PrintCalibrationData();

	Serial.print("Start reading\n");

	// Retrive the current pressure in Pascals.
	float currentPressure = environment.GetPressure();
	currentPressure_str = String(currentPressure);

	float currentTemperature = environment.GetTemperature();
	currentTemperature_str = String(currentTemperature);

	float currentHumidity = environment.GetHumidity();
	currentHumidity_str = String(currentHumidity);

	//Serial.println(); // Start a new line.
}



