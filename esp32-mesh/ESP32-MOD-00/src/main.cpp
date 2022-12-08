#include <Arduino.h>
#include "painlessMesh.h"

#define NODE_ID "N00"
#define MESH_PREFIX "FATE"
#define MESH_PASSWORD "HeavensFeel"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage();

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  String msg = "";

  if(Serial.available()){
    msg += Serial.readStringUntil('\n');}

  if (msg.length() <= 0){return;}

  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("ID: %u msg=%s\n", from, msg.c_str());}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);}

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