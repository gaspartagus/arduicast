/*
  Running Node.js processes asynchronously using the Process class.

  created 28 Aug 2014

  by Ludovic Muncherji

 */

#include <Process.h>

Process bonjour;
Process socket;
Process p;

int blue_ledPin = 8;
int red_ledPin = 9;

int start_btnPin = 4;
int menu_btnPin = 2;
int exit_btnPin = 3;
int progP_btnPin = 6;
int progM_btnPin = 5;

bool launched = false;
bool socketConnected = false;

int start = 0;
int progP = 0;
int progM = 0;
int menu = 0;
int sortie = 0;

void setup() {
  pinMode(blue_ledPin, OUTPUT);
  pinMode(red_ledPin, OUTPUT);
  pinMode(start_btnPin, INPUT_PULLUP);
  pinMode(progP_btnPin, INPUT_PULLUP);
  pinMode(progM_btnPin, INPUT_PULLUP);
  pinMode(menu_btnPin, INPUT_PULLUP);
  pinMode(exit_btnPin, INPUT_PULLUP);

  Bridge.begin();	// Initialize the Bridge
  Serial.begin(9600);	// Initialize the Serial

  // Wait until a Serial Monitor is connected.
  while (!Serial) {
    blinkAllLeds();
  }

  Serial.println("Started process");
}

void loop() {
  start = digitalRead(start_btnPin);

  if(start == LOW) {
    if(launched == false) {
      bonjour.runShellCommandAsynchronously("node /mnt/sda1/arduino/node/bonjour.js");
      launched = true;
      Serial.println("App launched");
      delay(10000);
      blinkAllLeds();
    }
    else {
      sendMessage(123);
      launched = false;
      Serial.println("App closed");
    }
  }

  if(launched == true) {
    if(socketConnected == false) {
      Serial.println("Connecting to socket ...");
      socket.runShellCommandAsynchronously("node /mnt/sda1/arduino/node/socket.js");
      socketConnected = true;
      delay(10000);
      blinkAllLeds();
    }

    menu = digitalRead(menu_btnPin);
    sortie = digitalRead(exit_btnPin);
    progP = digitalRead(progP_btnPin);
    progM = digitalRead(progM_btnPin);

    // pass any bytes that come in from the serial port
    // to the running node process:
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

  // pass any incoming bytes from the running node process
  // to the serial port:
  while (socket.available()) {
    Serial.write(socket.read());
  }

  if(socket.running()) {
    digitalWrite(red_ledPin, HIGH);
  }
  else {
    digitalWrite(red_ledPin, LOW);
  }
}

void blinkAllLeds() {
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
}

void sendMessage(int key) {
  String request = "http://localhost:8080/key/" + String(key);
  Serial.println(request);
  blinkLed(blue_ledPin);
  p.begin("curl");
  p.addParameter(request);
  p.runAsynchronously();
  delay(500);
}
