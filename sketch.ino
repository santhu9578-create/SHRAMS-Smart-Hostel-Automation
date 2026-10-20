/*
===========================================================
SHRAMS V2.0
Smart Hostel Resource Automation and Monitoring System

Features
---------
✓ WiFi Connection
✓ MQTT Publish
✓ MQTT Subscribe
✓ DHT22
✓ MQ2
✓ Occupancy Ultrasonic
✓ Door Ultrasonic
✓ Automatic Light Control
===========================================================
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

//--------------------------------------------------
// WiFi Credentials
//--------------------------------------------------

const char* ssid = "Wokwi-GUEST";
const char* password = "";

//--------------------------------------------------
// MQTT Broker
//--------------------------------------------------

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;

//--------------------------------------------------
// MQTT Topics
//--------------------------------------------------

const char* TOPIC_TEMP = "hostel/room1/temperature";
const char* TOPIC_HUM = "hostel/room1/humidity";
const char* TOPIC_GAS = "hostel/room1/gas";
const char* TOPIC_OCC = "hostel/room1/occupancy";
const char* TOPIC_DOOR = "hostel/room1/door";

const char* TOPIC_LIGHT = "hostel/room1/control/light";
const char* TOPIC_FAN = "hostel/room1/control/fan";
const char* TOPIC_ALARM = "hostel/room1/control/alarm";

//--------------------------------------------------
// Pin Definitions
//--------------------------------------------------

#define DHTPIN 4
#define DHTTYPE DHT22

#define MQ2_PIN 34

#define OCC_TRIG 5
#define OCC_ECHO 18

#define DOOR_TRIG 19
#define DOOR_ECHO 21

#define SERVO_PIN 13

#define LIGHT_LED 25
#define FAN_LED 26

#define BUZZER 27

//--------------------------------------------------
// Automation Switches
//--------------------------------------------------

#define LIGHT_MODE_SWITCH 14
#define FAN_MODE_SWITCH 12

//--------------------------------------------------

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

//--------------------------------------------------
// Automation Modes
//--------------------------------------------------

bool lightAutoMode = true;
bool fanAutoMode = true;

bool previousLightMode = true;
bool previousFanMode = true;

//--------------------------------------------------
// Previous States (Event Logger)
//--------------------------------------------------

String previousOccupancy = "";
String previousDoor = "";

bool previousLightState = false;
bool previousFanState = false;

int previousGas = -1;

float previousTemperature = -1000;
float previousHumidity = -1000;

//==================================================
// WiFi
//==================================================

void connectWiFi()
{
  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
}

//==================================================
// MQTT Callback
//==================================================

void callback(char* topic, byte* payload, unsigned int length)
{
  String message = "";

  for(int i=0;i<length;i++)
  {
    message += (char)payload[i];
  }

  Serial.println();
  Serial.print("Topic : ");
  Serial.println(topic);

  Serial.print("Message : ");
  Serial.println(message);

  //------------------------------------------------
  // Light Control
  //------------------------------------------------

  if(String(topic)==TOPIC_LIGHT)
{

    if(lightAutoMode)
    {

        if(message=="LIGHT_ON")
        {

            digitalWrite(LIGHT_LED,HIGH);

            Serial.println("Room Light ON (AUTO)");

        }

        else if(message=="LIGHT_OFF")
        {

            digitalWrite(LIGHT_LED,LOW);

            Serial.println("Room Light OFF (AUTO)");

        }

    }

    else
    {

        Serial.println("Automation Disabled");

        Serial.println("LIGHT command ignored.");

    }

}

//------------------------------------------------
// Fan Control
//------------------------------------------------

if(String(topic) == TOPIC_FAN)
{

    if(fanAutoMode)
    {

        if(message == "FAN_ON")
        {

            digitalWrite(FAN_LED, HIGH);

            Serial.println("Room Fan ON (AUTO)");

        }

        else if(message == "FAN_OFF")
        {

            digitalWrite(FAN_LED, LOW);

            Serial.println("Room Fan OFF (AUTO)");

        }

    }
    else
    {

        Serial.println("Fan Automation Disabled");

        Serial.println("FAN command ignored.");

    }

}

//------------------------------------------------
// Alarm Control
//------------------------------------------------

if(String(topic) == TOPIC_ALARM)
{

    if(message == "SAFE")
    {

        digitalWrite(BUZZER, LOW);

        Serial.println("Safety Status : SAFE");

    }

    else if(message == "WARNING_GAS")
    {

        Serial.println("Safety Status : GAS WARNING");

        digitalWrite(BUZZER, HIGH);
        delay(200);
        digitalWrite(BUZZER, LOW);

    }

    else if(message == "WARNING_TEMP")
    {

        Serial.println("Safety Status : HIGH TEMPERATURE");

        digitalWrite(BUZZER, HIGH);
        delay(150);
        digitalWrite(BUZZER, LOW);
        delay(150);
        digitalWrite(BUZZER, HIGH);
        delay(150);
        digitalWrite(BUZZER, LOW);

    }

    else if(message == "FIRE")
    {

        Serial.println("***** FIRE RISK DETECTED *****");

        digitalWrite(BUZZER, HIGH);
        delay(1500);
        digitalWrite(BUZZER, LOW);

    }

}

}

//==================================================
// MQTT Connection
//==================================================

void reconnectMQTT()
{

  while(!client.connected())
  {

      Serial.print("Connecting MQTT...");

      String clientID="SHRAMS_ROOM1_";
      clientID+=String(random(1000));

      if(client.connect(clientID.c_str()))
      {

          Serial.println("Connected");

          client.subscribe(TOPIC_LIGHT);
          client.subscribe(TOPIC_FAN);
          client.subscribe(TOPIC_ALARM);

      }

      else
      {
          Serial.print("Failed : ");
          Serial.println(client.state());

          delay(2000);
      }

  }

}

//==================================================
// Distance Function
//==================================================

float getDistance(int trig,int echo)
{

    digitalWrite(trig,LOW);
    delayMicroseconds(2);

    digitalWrite(trig,HIGH);
    delayMicroseconds(10);

    digitalWrite(trig,LOW);

    long duration=pulseIn(echo,HIGH);

    float distance=duration*0.034/2;

    return distance;

}

//==================================================
// Read Automation Switches (Debounced)
//==================================================

void updateAutomationMode()
{
    static unsigned long lastRead = 0;

    // Read only every 50 ms
    if (millis() - lastRead < 50)
        return;

    lastRead = millis();

    bool newLightMode = digitalRead(LIGHT_MODE_SWITCH);
    bool newFanMode   = digitalRead(FAN_MODE_SWITCH);

    //--------------------------------------------

    if (newLightMode != previousLightMode)
    {
        previousLightMode = newLightMode;
        lightAutoMode = newLightMode;

        if (lightAutoMode)
            Serial.println("Light Mode : AUTO");
        else
            Serial.println("Light Mode : MANUAL");
    }

    //--------------------------------------------

    if (newFanMode != previousFanMode)
    {
        previousFanMode = newFanMode;
        fanAutoMode = newFanMode;

        if (fanAutoMode)
            Serial.println("Fan Mode : AUTO");
        else
            Serial.println("Fan Mode : MANUAL");
    }
}

//==================================================
// Publish Sensor Data
//==================================================

void publishSensors()
{

    float temperature=dht.readTemperature();
    float humidity=dht.readHumidity();

    int gas=analogRead(MQ2_PIN);

    float occDistance=getDistance(OCC_TRIG,OCC_ECHO);

    float doorDistance=getDistance(DOOR_TRIG,DOOR_ECHO);

    String occupancy;

    if(occDistance<100)
        occupancy="PRESENT";
    else
        occupancy="ABSENT";

    String door;

    if(doorDistance<15)
        door="CLOSED";
    else
        door="OPEN";

    client.publish(TOPIC_TEMP,String(temperature).c_str());

    client.publish(TOPIC_HUM,String(humidity).c_str());

    client.publish(TOPIC_GAS,String(gas).c_str());

    client.publish(TOPIC_OCC,occupancy.c_str());

    client.publish(TOPIC_DOOR,door.c_str());

    //------------------------------------------------

//------------------------------------------------
// Event Logger
//------------------------------------------------

// Occupancy

if(occupancy != previousOccupancy)
{

    Serial.print("Occupancy Changed : ");

    Serial.print(previousOccupancy);

    Serial.print(" -> ");

    Serial.println(occupancy);

    previousOccupancy = occupancy;

}

// Door

if(door != previousDoor)
{

    Serial.print("Door Changed : ");

    Serial.print(previousDoor);

    Serial.print(" -> ");

    Serial.println(door);

    previousDoor = door;

}

}

//==================================================
// Setup
//==================================================

void setup()
{

    Serial.begin(115200);

    dht.begin();

    pinMode(OCC_TRIG,OUTPUT);
    pinMode(OCC_ECHO,INPUT);

    pinMode(DOOR_TRIG,OUTPUT);
    pinMode(DOOR_ECHO,INPUT);

    pinMode(LIGHT_LED,OUTPUT);
    pinMode(FAN_LED,OUTPUT);

    pinMode(BUZZER,OUTPUT);

    pinMode(LIGHT_MODE_SWITCH, INPUT_PULLUP);
    pinMode(FAN_MODE_SWITCH, INPUT_PULLUP);

    connectWiFi();

    client.setServer(mqttServer,mqttPort);

    client.setCallback(callback);

}

//==================================================
// Loop
//==================================================

void loop()
{

    if(!client.connected())
    {
        reconnectMQTT();
    }

    client.loop();

    updateAutomationMode();

    if(millis()-lastPublish>5000)
    {
        lastPublish=millis();

        publishSensors();
    }

}