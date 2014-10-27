/*
  Running Node.js processes asynchronously using the Process class.

  created 28 Aug 2014

  by Ludovic Muncherji

 */

#include <Process.h>

Process server;
Process p;

const int touches[] = {
  123, // Shutdown
  13,  // OK
  38,  // Up down left right
  40,
  37,
  39,
  8,   // Back
  27,  // Exit
  120, // Menu
  87,  // P+
  34,  // P-
  36,  // V+
  35,  // V-
}; // length: 13

long before[13][2] = {
  { 1,0 },  
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 },
  { 1,0 }
};

long after[13] = {
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

int blue_ledPin = 12;
int red_ledPin = 11;

int start_btnPin = 2;

bool serverLaunched = false;
bool launched = false;

int start = 0;
int progP = 0;
int progM = 0;
int menu = 0;
int sortie = 0;
int colonne = 0;
int firstCode = 0;
int etatServeur[2] = {0,0};

int debounce = 50;

String IP_CHROMECAST = "wedontcare"; // "192.168.0.156";
String script = "node /mnt/sda1/arduino/node/remote2.js " + IP_CHROMECAST;

void setup() {

  pinMode(blue_ledPin, OUTPUT);
  pinMode(red_ledPin, OUTPUT);
  for(int i = 2; i <= 10; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  
  Serial.begin(9600);
  Serial.println("Serial started");
  Bridge.begin();
  Serial.println("Bridge started");
  /*Console.begin(); 

  while (!Console){
    ; // wait for Console port to connect.
  }
  Console.println("You're connected to the Console!!!!");*/

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
    //digitalWrite(13,HIGH);
  start = digitalRead(start_btnPin);
  

  if(start == LOW) {
    //digitalWrite(13,HIGH);
    if(launched == false) {
      requestServer("cast");
      launched = true;
      Serial.println("App launched");
      while(digitalRead(start_btnPin) != HIGH);
    }
    // else {
    //   //digitalWrite(13,LOW);
    //   requestServer("stop");
    //   launched = false;
    //   //Serial.println("App closed");
    // }
  }
  
  if(launched == true) {
    digitalWrite(blue_ledPin, HIGH);

    // for(int i = 3; i < 11; i++) {
    //   if(digitalRead(i) == LOW) {
    //     sendMessage(touches[i-2]);
    //   }
    // }

    for(int i = 3; i < 11; i++) {
      before[i][0] = after[i];
      after[i] = digitalRead(i);

      long now = millis();
      if( after[i]-before[i][0] == -1 &&  now - before[i][1] > debounce)
      {
        // Serial.println(after[i]-before[i][0]);
        // Serial.println(now);
        digitalWrite(13, after[i]);
        before[i][1] = now;
        sendMessage(touches[i-2]);
      }
    }

    
    


  }
  
  /*while (server.available()) {
    Serial.write(server.read());
  }*/

  if(etatServeur[0]) {
    digitalWrite(red_ledPin, HIGH);
  }
  
  if(server.running()) {
    // Serial.println("Server Running");
    serverLaunched = true;
    int output = server.read();
    if(output > -1)
    {
      colonne++;
      Serial.print(char(output));
    }
    if(colonne == 1) {
      firstCode = output - 48;
    } else if( colonne == 2) {
      etatServeur[firstCode] = output - 48;
    }
    if(output == 10) {
      colonne = 0;
    }
  }
  else {
    digitalWrite(red_ledPin, LOW);
    serverLaunched = false;
    launchServer();
  }

  // Serial.print(", before: ");
  // for(int i = 3; i < 11; i++) {
  //   Serial.print(before[i][0]);
  // }

  // Serial.print(", timestamps: ");
  // for(int i = 3; i < 11; i++) {
  //   Serial.print(before[i][1]);
  // }

  // Serial.print(", launched: ");
  // Serial.print(launched);
  
  // Serial.print(", serverLaunched: ");
  // Serial.print(serverLaunched);


  // Serial.print(", ");
  // Serial.print(millis());
  // Serial.println("");
}

void launchServer() {
  server.runShellCommandAsynchronously(script);
  //delay(21000);
  //requestServer("discover");
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
  Serial.println(request);
  requestServer(request);
  digitalWrite(blue_ledPin, HIGH);
  digitalWrite(blue_ledPin, LOW);
}
