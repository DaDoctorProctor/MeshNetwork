/* Files*/
#include "OLED.h"

/**/
#include <Arduino.h>
#include "painlessMesh.h"
#include <vector>
#include <string>

#include <TFT_eSPI.h> 
#include <SPI.h>

#define NODE_ID "NOLED"
#define MESH_PREFIX "FATE"
#define MESH_PASSWORD "HeavensFeel"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage();

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage);

class Oled{
  protected:
    Oled() = default;

  public:
  
    TFT_eSPI tft = TFT_eSPI();

    static Oled& get_instance(){
      static Oled instance;  
      return instance;
    }
    
    Oled(const Oled&) = delete;
    Oled(Oled&&) = delete;
    Oled& operator=(const Oled&) = delete;
    Oled& operator=(Oled&&) = delete;
};

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

    void setData1(std::string input){data1 = input;}
    std::string getData1(){return data1;}
    
    void setData2(std::string input){data2 = input;}
    std::string getData2(){return data2;}

    void setData3(std::string input){data3 = input;}
    std::string getData3(){return data3;}

    void setButtonState1(int input){buttonState1 = input;}
    int getButtonState1(){return buttonState1;}

    void setButtonState2(int input){buttonState2 = input;}
    int getButtonState2(){return buttonState2;}

  private:
    std::string data1;
    std::string data2;
    std::string data3;
    int buttonState1;
    int buttonState2;

};

void saveData(std::string data, uint32_t from){
  sensorReadings &sensor = sensorReadings::get_instance();
  switch (from) {
  case 1835340161: //No antenna module
    sensor.setData1(data);
    break;
  case 2224751097: //H&T sensor
    sensor.setData2(data);
    break;
  case 2224766733: //US sensor
    sensor.setData3(data);
    break;
  }
}

void printOled(){
  Oled &oled1 = Oled::get_instance();
  sensorReadings &sensor = sensorReadings::get_instance();
  oled1.tft.fillScreen(TFT_BLACK);
  oled1.tft.setTextColor(TFT_BLUE, TFT_BLACK);
  oled1.tft.drawString(sensor.getData1().c_str(), 0, 0, 2);
  oled1.tft.drawString(sensor.getData2().c_str(), 0, 16, 2);
  oled1.tft.drawString(sensor.getData3().c_str(), 0, 32, 2);
}

void readButtons(){ 
  int buttonPin1 = 0;
  int buttonPin2 = 35;
  bool buttonState1 = digitalRead(buttonPin1);
  bool buttonState2 = digitalRead(buttonPin2);
  sensorReadings &sensor = sensorReadings::get_instance();
  
  if (buttonState1 == HIGH) {
    sensor.setButtonState1(0);} 
  else {sensor.setButtonState1(1);}
  if (buttonState2 == HIGH) {
    sensor.setButtonState2(1);} 
  else {sensor.setButtonState2(0);}

}

void processData(std::string str, uint32_t from){
  saveData(str,from);
  readButtons();
  printOled();
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


void sendMessage() {
  sensorReadings &sensor = sensorReadings::get_instance();
  String msg = "LilyGO:";

  String btn1 = String(sensor.getButtonState1());
  String btn2 = String(sensor.getButtonState2());

  // Send the button data
  msg += ",bA";
  msg += btn1;
  msg += ",bB";
  msg += btn2;
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

void receivedCallback(uint32_t from, String &msg) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  //Serial.printf("ID: %u msg= %s\n", from, msg.c_str());
  processData(msg.c_str(),from);
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

void oledInit(){
    Oled &oled1 = Oled::get_instance();
    oled1.tft.init(); 
    oled1.tft.setRotation(3);
}

/* Main*/
void setup(void) {
  
  oledInit();
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

