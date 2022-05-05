/*
 Basic ESP8266 MQTT door opener
 When a 1 is received via mqtt, pin D0 sends a low signal activating a relay.
 Connect both wires from your intercom door opener button to the relay.
 The relay then "presses" the door open button on your intercom.
 After 1 second the button gets released.
 This might NOT be secure and is not properly tested!
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// setup wifi
const char* ssid = "wifi-ssid";
const char* password = "wifi-password";

// setup mqtt
const char* mqtt_server = "mqtt-server-address";
const char* mqtt_user = "mqtt-user";
const char* mqtt_password = "mqtt-password";

const int door_open_mils = 1000;  // define on how many miliseconds the door opener button will be pressed (1sec = 1000 mils)
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void opendoor(){
  Serial.println("pressing door opener");
  digitalWrite(D0, LOW);  // activating relay
  delay(door_open_mils);  // wait x mils for the door to be pressed by human
  Serial.println("releasing door opener");
  digitalWrite(D0, HIGH); // deactivate relay
}

void setup_wifi() {

  delay(10);
  // define door gpio D0
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);
  // connect to wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// callback gets called when a new msg arrives at the subscribed mqtt topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Open the door, when a 1 was received as payload
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Light up builtin LED so we can see something happens
    opendoor(); // open the door
    digitalWrite(BUILTIN_LED, HIGH);  // turn off LED
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "door-esp8266-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("door_publish", "hello world");
      // ... and resubscribe
      client.subscribe("door");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200); // set baudrate
  setup_wifi();
  client.setServer(mqtt_server, 1883);  //define mqtt server
  client.setCallback(callback); //define callback
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}