/*
  Running Node.js processes asynchronously using the Process class.

  created 28 Aug 2014

  by Ludovic Muncherji

 */

#include <Process.h>
#include <math.h>

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

// Dans la première colonne la valeur du dernier front,
// dans la deuxième la dernière fois où il a changé
long fronts[13][2] = { 
  { 0,0 },  
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 }
};

// L'avant dernier dernier état du bouton
long before[13] = { 1,1,1,1,1,1,1,1,1,1,1,1,1 };
// Le dernier état du bouton
long after[13] = { 1,1,1,1,1,1,1,1,1,1,1,1,1 };


int blue_ledPin = 11;

float frequence = 2;
float Pi = 3.14159;

int start_btnPin = 2;

int colonne = 0;
int etatServeur[2] = {0,0}; // [foundDevice,socketConnected]

int debounce = 50;

String script = "node /mnt/sda1/arduino/node/remote2.js";

int etat = 0;
int done = 0;
int casted = 0;
long graphNow = 2147483600;
int currentButton = 0;

void setup() {

  pinMode(blue_ledPin, OUTPUT);
  //pinMode(red_ledPin, OUTPUT);
  for(int i = 2; i <= 10; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  
  Serial.begin(9600);
  Serial.println("Serial started");
  Bridge.begin();
  Serial.println("Bridge started");

  server.runShellCommandAsynchronously(script);

}


void loop() {

  // Capture button states
  for(int i = 2; i < 11; i++) {
    before[i] = after[i];
    after[i] = digitalRead(i);

    long now = millis();

    if( after[i]-before[i] != 0 &&  now - fronts[i][1] > debounce)
    {
      fronts[i][0] = after[i]-before[i];
      fronts[i][1] = now;
    }
  }

  // Capture bridge console and thus server state
  int output = server.read();
  if(output > -1)
  {
    colonne++;
    Serial.print(char(output));
  }
  if(colonne == 1 && output != 120) {
    etatServeur[0] = output - 48;
  } else if( colonne == 2 && output != 120) {
    etatServeur[1] = output - 48;
    if(etatServeur[1] == 1) {
      analogWrite(blue_ledPin,160);
    } else {
      digitalWrite(blue_ledPin,LOW);
    }
  }
  if(output == 10) {
    colonne = 0;
  }

  // A parallel "interrupt" loop
  if( casted && after[start_btnPin] == 0 ) {
    requestServer("stop");
    casted = 0;
    // etatServeur[0] = 0;
    // etatServeur[1] = 0;
    // delay(3000);
    // server.close();
    // server.runShellCommandAsynchronously(script);
    goTo(0,true);
  }

  // The main Graphcet loop
  switch(etat) {
    case 0:
      if(!done) {
        graphNow = millis();
        done=1;
      }
      if(etatServeur[0] == 1)
        sine(0.5);
      if( etatServeur[0] == 1 &&
          fronts[start_btnPin][0] == -1 &&
          fronts[start_btnPin][1] > graphNow) {
        /* Serial.print(fronts[start_btnPin][0]);
        Serial.print(" - ");
        Serial.print(fronts[start_btnPin][1]-graphNow); */
        goTo(2,true);
      }
    break;
    case 1:
    break;
    case 2:
      if(!done) {
        graphNow = millis();
        requestServer("cast");
        digitalWrite(blue_ledPin,LOW);
        done=1;
      }
      if(millis()-graphNow > 5000) {
        casted = 1;
        goTo(3,true);
      }
    break;
    case 3:
      for(int i = 3; i < 11; i++) {
        if(after[i] == 0) {
          currentButton = i;
          goTo(4,true);
          break;
        }
      }
    break;
    case 4:
      if(!done) {
        graphNow = millis();
        // requestServer("zap/12")
        sendMessage(touches[currentButton-2]);
        done=1;
      }
      if(fronts[currentButton][0] == 1 && after[currentButton] == 1) {
        goTo(3,true);
      } else if(millis()-graphNow > 1000) {
        goTo(41,true);
      }
    break;
    case 41:
      if(!done) {
        graphNow = millis();
        sendMessage(touches[currentButton-2]);
        done=1;
      }
      if(fronts[currentButton][0] == 1 && after[currentButton] == 1) {
        goTo(3,true);
      } else if(millis()-graphNow > 300) {
        goTo(41,true);
      }
    break;
  }
}

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
  // digitalWrite(blue_ledPin, HIGH);
  // digitalWrite(blue_ledPin, LOW);
}

void launchServer() {
  server.runShellCommandAsynchronously(script);
}

void goTo(int destination, bool faire) {
  etat = destination;
  Serial.println(destination);
  if(faire) {
    done = 0;
  } else done = 1;
}

void sine(float freq) {
  float now = millis();
  float omegaT = now/1000*2*Pi*freq;
  float val = floor(sin(omegaT)*sin(omegaT)*140)+30;
  // Serial.write(val);
  analogWrite(blue_ledPin,val);
}


















