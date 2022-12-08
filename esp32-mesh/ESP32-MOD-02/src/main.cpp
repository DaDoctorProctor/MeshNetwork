#include <Arduino.h>
#include "painlessMesh.h"

#define NODE_ID "N02"
#define MESH_PREFIX "FATE"
#define MESH_PASSWORD "HeavensFeel"
#define MESH_PORT 5555

// DEFINE PINS
// *Ultrasonic Sensor
const int trigPin = 26;
const int echoPin = 27;

int UltrasonicScan(){
  long duration;
  int distance;
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Returns the distance
  return distance;
}

void processData(std::string str, uint32_t from){
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
  }
}


Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage();

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {

  String msg;
  msg += "US";
  msg += String(UltrasonicScan());

  //msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void receivedCallback( uint32_t from, String &msg ) {
  // Received message module
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  //processData(msg.c_str(), from);

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
  mesh.update();
}