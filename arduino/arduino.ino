
#include <Process.h>

#include <math.h>

#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);

Process server;
Process p;
Process wireless;

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


int blue_ledPin = 13;

float frequence = 2;
float Pi = 3.14159;

int start_btnPin = 2;

int colonne = 0;
int etatServeur[2] = {0,0}; // [foundDevice,socketConnected]
String channel = "";
String ssid = "goran";
String encryption = "psk";
String key = "DEADBEEF75";

String mime1 = "";
String mime2 = "";

int debounce = 50;

String script = "node /mnt/sda1/arduino/node/remote2.js >> /mnt/sda1/arduino/node/output.txt";
String config = "/mnt/sda1/arduino/node/echo.sh";
String tempConfig = "";

// Main loop variables
int etat = 0;
int done = 0;
int casted = 0;
long graphNow = 2147483600;
int currentButton = 0;
// Variables for the blink functionnality
long commandNow = 0;
int commandLed = false;
// NFC action variables : indicates if a nfc action has ended
bool nfcZapToken = true;
bool nfcWifiToken = true;

void setup() {

  pinMode(blue_ledPin, OUTPUT);
  //pinMode(red_ledPin, OUTPUT);
  for(int i = 2; i <= 12; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  Serial.begin(9600);
  Bridge.begin();
  Serial.println("Bridge started");
  // wireless.runShellCommandAsynchronously("sh /mnt/sda1/arduino/node/echo.sh freeboxchh psk newdecfreebox");
  // wireless.runShellCommandAsynchronously("sh /mnt/sda1/arduino/node/echo.sh goran psk DEADBEEF75");


  nfc.begin();

  // server.begin("node");
  // server.addParameter(script);
  server.runShellCommandAsynchronously(script);

  // delay(5000);

}


void loop() {

  // Capture button states
  for(int i = 2; i <= 12; i++) {
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
    Serial.print(String(char(output)));
  }
  if(colonne == 1 && output != 120) {
    etatServeur[0] = output - 48;
  } else if( colonne == 2 && output != 120) {
    etatServeur[1] = output - 48;
  }
  if(output == 10) {
    colonne = 0;
  }

  // if(true){
  //   int o = wireless.read();
  //   if(o > -1)
  //   {
  //     Serial.print(char(o));
  //   }
  // }


  // Capture NFC tag
  if( nfcZapToken && nfcWifiToken ) { // && etatServeur[0] == 1
    // Serial.print(".");
    if (nfc.tagPresent(50))
    {
      NfcTag tag = nfc.read();
      if(tag.hasNdefMessage()){

        // Fetch the NFC payload
        NdefMessage message = tag.getNdefMessage();
        NdefRecord record = message.getRecord(0);
        int longueur = record.getPayloadLength();

        byte* payload = new byte[longueur];
        record.getPayload(payload);

        int typeLength = record.getTypeLength();

        byte* type = new byte[typeLength];
        record.getType(type);

        if(longueur >= 0 && typeLength >= 0){
          int i = 0;
          mime1 = "";
          mime2 = "";
          while(type[i] != 47 && i < typeLength) {
            mime1 = mime1 + String(char(type[i]));
            i++;
          }
          i++;
          while(i < typeLength) {
            mime2 = mime2 + String(char(type[i]));
            i++;
          }
          Serial.println(mime1);
          Serial.println(mime2);

          if(mime2 == "channel") {
            Serial.println("mime-type : channel.");
            // Record the channel
            channel = "";
            for (int i = 0; i < longueur; ++i)
            {
              channel = channel + String(char(payload[i]));
            }
            // channel = String(char(payload[3]))+String(char(payload[4]));
            Serial.println(channel + String("."));
            nfcZapToken = false;
          } else if( mime2 == "wifi") {
            int i = 0;
            ssid = "";
            encryption = "";
            key = "";
            while(payload[i] != 47 && i < longueur) {
              ssid = ssid + String(char(payload[i]));
              i++;
            }
            i++;
            while(payload[i] != 47 && i < longueur) {
              encryption = encryption + String(char(payload[i]));
              i++;
            }
            i++;
            while(payload[i] != 47 && i < longueur) {
              key = key + String(char(payload[i]));
              i++;
            }
            Serial.print(ssid);
            Serial.print(encryption);
            Serial.println(key);
            nfcWifiToken = false;
          }
        }

        // Record the wifi credentials
        if(longueur > 8) {
        }

        // Print the whole payload

        Serial.print("Longueur :");
        Serial.print(longueur);
        Serial.print(", payload :");
        for(int i = 0; i<longueur; i++){
          Serial.print(char(payload[i]));
        }
        Serial.println(".");

        Serial.print("TypeLength :");
        Serial.print(typeLength);
        Serial.print(", type :");
        for(int i = 0; i<typeLength; i++){
          Serial.print(char(type[i]));
        }
        Serial.println(".");

        delete payload;
        delete type;
      }
    }
  }

  // A parallel "interrupt" loop which turns off the webapp
  if( casted && after[start_btnPin] == 0 ) {
    requestServer("stop");
    casted = 0;

    goTo(0,true);
  }

  // A blink parallel loop
  if(etatServeur[1] == 1 && commandLed == false) {
    analogWrite(blue_ledPin,160);
  }
  else if(commandLed) {
    if(millis() < commandNow + 100) {
      digitalWrite(blue_ledPin,LOW);
    }
    else {
      commandLed = false;
    }
  } else {
    digitalWrite(blue_ledPin,LOW);
  }

  // The main Graphcet loop
  switch(etat) {
    case 0:
      if(!done) {
        graphNow = millis();
        nfcZapToken = true;
        //wireless.runShellCommandAsynchronously(String("cat /etc/config/wireless"));
        // writeWifiConfig("ssid","enc","key");
        done=1;
      }
      if(etatServeur[0] == 1)
        sine(0.5);
      if( // etatServeur[0] == 1 &&
          fronts[start_btnPin][0] == -1 &&
          fronts[start_btnPin][1] > graphNow) {
        goTo(1,true);
      } else if( etatServeur[0] == 1 &&
          nfcZapToken == false) {
        goTo(12,true);
      } else if( nfcWifiToken == false) {
        goTo(21,true);
      }
    break;
    case 1:
      if(!done) {
        graphNow = millis();
        requestServer("cast");
        blink();
        done=1;
      }
      if(millis()-graphNow > 10000) {
        casted = 1;
        goTo(2,true);
      }
    break;
    case 12:
      if(!done) {
        graphNow = millis();
        requestServer("cast");
        commandLed = true;
        done=1;
      }
      if(millis()-graphNow > 15000) {
        casted = 1;
        goTo(13,true);
      }
    break;
    case 13:
      if(!done) {
        requestServer("zap/"+channel);
        done=1;
      }
      if(millis()-graphNow > 8000) {
        nfcZapToken = true;
        goTo(2,true);
      }
    break;
    case 21:
      server.close();
      digitalWrite(blue_ledPin,HIGH);
      wireless.begin("sh");
      wireless.addParameter("/mnt/sda1/arduino/node/echo.sh");
      wireless.addParameter(ssid);
      wireless.addParameter(encryption);
      wireless.addParameter(key);
      wireless.run();
      wireless.close();
      digitalWrite(blue_ledPin,LOW);
      nfcWifiToken = true;
      goTo(22,true);
    break;
    case 22:
      server.runShellCommandAsynchronously(script);
      goTo(0,true);
    break;
    case 2:
      for(int i = 3; i <= 12; i++) {
        if(after[i] == 0) {
          currentButton = i;
          goTo(3,true);
          break;
        }
      }
      if(nfcZapToken == false) {
        goTo(32,true);
      }
    break;
    case 3:
      if(!done) {
        graphNow = millis();
        sendMessage(touches[currentButton-2]);
        done=1;
      }
      if(fronts[currentButton][0] == 1 && after[currentButton] == 1) {
        goTo(2,true);
      } else if(millis()-graphNow > 1000) {
        goTo(31,true);
      }
    break;
    case 31:
      if(!done) {
        graphNow = millis();
        sendMessage(touches[currentButton-2]);
        done=1;
      }
      if(fronts[currentButton][0] == 1 && after[currentButton] == 1) {
        goTo(2,true);
      } else if(millis()-graphNow > 400) {
        goTo(31,true);
      }
    break;
    case 32:
      if(!done) {
        graphNow = millis();
        requestServer("zap/"+channel);
        blink();
        done=1;
      }
      if(millis()-graphNow > 8000) {
        nfcZapToken = true;
        goTo(2,true);
      }
    break;
  }
}

void requestServer(String command) {
  String request = "http://localhost:8080/" + command;
  p.begin("curl");
  p.addParameter(request);
  p.runAsynchronously();
  Serial.println(command);
}

void sendMessage(int key) {
  String request = "key/" + String(key);
  requestServer(request);
  blink();
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
  analogWrite(blue_ledPin,val);
}

void blink(){
  commandNow = millis();
  commandLed = true;
}




