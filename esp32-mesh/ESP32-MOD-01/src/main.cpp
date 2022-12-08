#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include "painlessMesh.h"

#define NODE_ID "N01"
#define MESH_PREFIX "FATE"
#define MESH_PASSWORD "HeavensFeel"
#define MESH_PORT 5555

const byte led_gpio = 32;

class sensorReadings{
  protected:
    sensorReadings() = default;
  public:
  
    static sensorReadings& get_instance(){
      static sensorReadings instance;  
      return instance;
    }
    
    sensorReadings(const sensorReadings&) = delete;
    sensorReadings(sensorReadings&&) = delete;
    sensorReadings& operator=(const sensorReadings&) = delete;
    sensorReadings& operator=(sensorReadings&&) = delete;

    void setButtonState1(int input){buttonState1 = input;}
    int getButtonState1(){return buttonState1;}

    void setButtonState2(int input){buttonState2 = input;}
    int getButtonState2(){return buttonState2;}

  private:
    int buttonState1;
    int buttonState2;
};

// Temp & Humidity
DHT dht(27, DHT22); 
float humidity, temperature;

void led(){
  sensorReadings &sensor = sensorReadings::get_instance();
  
  int sensorState = sensor.getButtonState1();
  if (sensorState == 1){
    digitalWrite(led_gpio, HIGH);
  } else {
    digitalWrite(led_gpio, LOW);}
}

void processData(std::string str, uint32_t from){
  sensorReadings &sensor = sensorReadings::get_instance();
  //main while
  char *array = new char[str.length() + 1];
  strcpy(array, str.c_str());
  char *strings[3]; 
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(array, ",");

  while (ptr != NULL){
    strings[index] = ptr;
    index++;
    ptr = strtok(NULL, ",");
  }

  Serial.printf("ID: %u\n", from);
  for (int n = 0; n < index; n++){
    Serial.print("data ");
    Serial.print(n);
    Serial.print(" = ");
    Serial.println(strings[n]);
    //Add if statement to set the values
    std::string s = strings[n];
    if (s == "bA0"){
      sensor.setButtonState1(0);
    }else if (s == "bA1"){
      sensor.setButtonState1(1);
    }
  led();
  }
}


Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage();

Task taskSendMessage(TASK_SECOND*1 , TASK_FOREVER, &sendMessage);

void sendMessage() {

  String msg;
  
  //Move to a function
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  //

  Serial.print(temperature);

  msg += 'H';
  msg += String(humidity);
  msg += ',';
  msg += 'T';
  msg += String(temperature);

  //msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND*1, TASK_SECOND*5));
}

void receivedCallback( uint32_t from, String &msg ) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  processData(msg.c_str(), from);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  // Start sensors
  dht.begin();
  pinMode(led_gpio, OUTPUT);

  // Start mesh network
  mesh.setDebugMsgTypes(ERROR | STARTUP); 
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // Sensor workings
  mesh.update();
}