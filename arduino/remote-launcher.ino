/*
  Running Node.js processes asynchronously using the Process class.

  created 28 Aug 2014

  by Ludovic Muncherji

 */

#include <Process.h>

Process server;
Process p;

int blue_ledPin = 8;
int red_ledPin = 9;

int start_btnPin = 4;
int menu_btnPin = 2;
int exit_btnPin = 3;
int progP_btnPin = 6;
int progM_btnPin = 5;

bool serverLaunched = false;
bool launched = false;

int start = 0;
int progP = 0;
int progM = 0;
int menu = 0;
int sortie = 0;

String IP_CHROMECAST = "10.0.2.113";
String script = "node /mnt/sda1/arduino/node/remote.js " + IP_CHROMECAST;

void setup() {
  //pinMode(blue_ledPin, OUTPUT);
  //pinMode(red_ledPin, OUTPUT);
  pinMode(start_btnPin, INPUT_PULLUP);
  pinMode(progP_btnPin, INPUT_PULLUP);
  pinMode(progM_btnPin, INPUT_PULLUP);
  pinMode(menu_btnPin, INPUT_PULLUP);
  pinMode(exit_btnPin, INPUT_PULLUP);
  
  Bridge.begin();
  Serial.begin(9600);

  /*while (!Serial) {
    blinkAllLeds();
  }*/
  
  //Serial.println("Started process");
  
  //server.runShellCommandAsynchronously(script);
  //delay(20000);
  //requestServer("discover");
  launchServer();
}

void loop() {
  start = digitalRead(start_btnPin);
  
  if(start == LOW) {
    if(launched == false) {
      requestServer("cast");
      launched = true;
      //Serial.println("App launched");
      //blinkAllLeds();
    }
    else {
      requestServer("stop");
      launched = false;
      //Serial.println("App closed");
    }
  }
  
  if(launched == true) {
    digitalWrite(blue_ledPin, HIGH);
    menu = digitalRead(menu_btnPin);
    sortie = digitalRead(exit_btnPin);
    progP = digitalRead(progP_btnPin);
    progM = digitalRead(progM_btnPin);
    
    if (progP == LOW) {
      sendMessage(33);
    }
    if (progM == LOW) {
      sendMessage(34);
    }
    if (menu == LOW) {
      sendMessage(120);
    }
    if (sortie == LOW) {
      sendMessage(27);
    }
  }
  
  /*while (server.available()) {
    Serial.write(server.read());
  }*/
  
  if(server.running()) {
    digitalWrite(red_ledPin, HIGH);
    serverLaunched = true;
  }
  else {
    digitalWrite(red_ledPin, LOW);
    serverLaunched = false;
    launchServer();
  }
}

void launchServer() {
  server.runShellCommandAsynchronously(script);
  delay(21000);
  requestServer("discover");
}

/*void blinkAllLeds() {
  digitalWrite(red_ledPin, HIGH);
  digitalWrite(blue_ledPin, HIGH);
  delay(500);
  digitalWrite(red_ledPin, LOW);
  digitalWrite(blue_ledPin, LOW);
  delay(500);
}

void blinkLed(int pin) {
  digitalWrite(red_ledPin, HIGH);
  digitalWrite(blue_ledPin, HIGH);
  delay(500);
  digitalWrite(red_ledPin, LOW);
  digitalWrite(blue_ledPin, LOW);
  delay(500);
}*/

void requestServer(String command) {
  String request = "http://localhost:8080/" + command;
  //Serial.println(request);
  //blinkLed(blue_ledPin);
  p.begin("curl");
  p.addParameter(request);
  p.runAsynchronously();
}

void sendMessage(int key) {
  String request = "key/" + String(key);
  requestServer(request);
  digitalWrite(blue_ledPin, HIGH);
  delay(100);
  digitalWrite(blue_ledPin, LOW);
}
