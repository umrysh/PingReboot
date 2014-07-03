/*
  PingReboot

  Copyright 2013 Dave Umrysh & Jeff Stauffer
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>.
 

  Circuit:
  * Ethernet shield attached to pins 10, 11, 12, 13
*/

#include <SPI.h>         
#include <Ethernet.h>
#include <ICMPPing.h>
#include <morse.h>

// The pins the relay and LEDs are connected to
int relay = 6;
int ledgrn = 8;
int ledred = 7;


// Globals
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // max address for ethernet shield
byte pingAddr[] = {8,8,8,8}; // ip address to ping, using Google
SOCKET pingSocket = 0;
EthernetClient client;
LEDMorseSender sender(ledgrn);
char buffer [256];
char params[32];
int badPingCount = 0;
int flippedRelay = 0;
int flashingCounter = 0;
char RemoteReboot = '0';


// Settings for the Timers
unsigned long timeBetweenPingsBad = 60000;
unsigned long timeBetweenPingsGood = 300000;
unsigned long timeOffOnFlip = 10000;
unsigned long timeToWaitOnFlip = 600000;
unsigned long timeToWaitOnGiveUp = 3600;

// Email Variables
char serverName[] = "www.example.com"; // server domain
int serverPort = 80; // server's port
char pageName[] = "/sendTheEmail.php"; // Page on the server

// Force Reboot Variables
char rebootServer[] = "www.example.com";
char rebootPageName[] = "/remotepingreboot.htm";

// Use morse code or serial output.
// 1 = morse, 0 = serial
int morseCode = 1;


void setup() 
{
  pinMode(ledred, OUTPUT);     
  pinMode(ledgrn, OUTPUT);
  pinMode(relay, OUTPUT);

  digitalWrite(ledgrn, HIGH);
  digitalWrite(relay,HIGH);

  if(morseCode==1)
  {
    sender.setup();
    sender.setMessage(String("starting ethernet"));
    sender.startSending();
  }else{
    Serial.begin(9600);
    Serial.print(F("Starting ethernet..."));
  }
  

  while(!Ethernet.begin(mac))
  {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("failed starting ethernet"));
      sender.startSending();
    }
    else
    {
      Serial.println(F("failed"));
      Serial.print(F("Starting ethernet..."));
    }
  }


  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String(Ethernet.localIP()));
    sender.startSending();
  }
  else
  {
    Serial.println(Ethernet.localIP());
    Serial.println(F("Ready"));
  }

  delay(2000);
}

void loop()
{
  ICMPPing ping(pingSocket);
  ping(4, pingAddr, buffer);
  if (String(buffer) == "Request Timed Out") 
  {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("bad ping"));
      sender.startSending();
    }
    else
    {
     Serial.println(F("Bad Ping"));
    }
    delay(timeBetweenPingsBad);
    badPingCount = ++badPingCount;
    if(badPingCount > 8)
    {
      if (flippedRelay < 3)
      {
        badPingCount = 0;
        rebootRouter();
      }else{
        // We've flipped it a bunch and getting nothing. Wait an hour
        if(morseCode==1)
        {
          while(sender.continueSending())
          {
            delay(100);
          }
          sender.setMessage(String("already flipped it 3 times giving up"));
          sender.startSending();
          while(flashingCounter<timeToWaitOnGiveUp)
          {
            digitalWrite(ledred, HIGH);
            delay(500);
            digitalWrite(ledred, LOW);
            delay(500);

            flashingCounter = ++flashingCounter;
          }
          flashingCounter = 0;
        }
        else
        {
          Serial.println(F("Already flipped it 3 times, Giving Up!"));
          while(flashingCounter<timeToWaitOnGiveUp)
          {
            digitalWrite(ledgrn, LOW);
            digitalWrite(ledred, HIGH);
            delay(500);
            digitalWrite(ledgrn, HIGH);
            digitalWrite(ledred, LOW);
            delay(500);

            flashingCounter = ++flashingCounter;
          }
          flashingCounter = 0;
        }
      }
    }
  }else{
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("good ping"));
      sender.startSending();
    }
    else
    {
      Serial.println(F("Good Ping"));
    }
    badPingCount = 0;
    if(flippedRelay != 0)
    {
      // Alert that we had to cycle the power
      sprintf(params,"message=%i",flippedRelay);    
        if(!postPage(serverName,serverPort,pageName,params))
        {
          if(morseCode==1)
          {
            while(sender.continueSending())
            {
              delay(100);
            }
            sender.setMessage(String("email error"));
            sender.startSending();
          }
          else
          {
            Serial.println(F("Email Error"));
          }
        } 
        else 
        {
          if(morseCode==1)
          {
            while(sender.continueSending())
            {
              delay(100);
            }
            sender.setMessage(String("email sent"));
            sender.startSending();
          }
          else
          {
            Serial.println(F("Email Sent"));
          }
        }
      flippedRelay = 0;
    }
    delay(timeBetweenPingsGood);
    checkForReboot();
  }
}

byte postPage(char* domainBuffer,int thisPort,char* page,char* thisData)
{
  int inChar;
  char outBuf[64];

  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String("connecting"));
    sender.startSending();
  }
  else
  {
    Serial.print(F("connecting..."));
  }

  if(client.connect(domainBuffer,thisPort))
  {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("connected"));
      sender.startSending();
    }
    else
    {
      Serial.println(F("connected"));
    }

    // send the header
    sprintf(outBuf,"POST %s HTTP/1.1",page);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s",domainBuffer);
    client.println(outBuf);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuf,"Content-Length: %u\r\n",strlen(thisData));
    client.println(outBuf);

    // send the body (variables)
    client.print(thisData);
  }
  else
  {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("email failed"));
      sender.startSending();
    }
    else
    {
      Serial.println(F("Email: Failed"));
    }
    return 0;
  }

  int connectLoop = 0;

  while(client.connected())
  {
    while(client.available())
    {
      inChar = client.read();
      if(morseCode!=1)
      {
        Serial.write(inChar);
      }
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if(connectLoop > 10000)
    {
      if(morseCode==1)
      {
        while(sender.continueSending())
        {
          delay(100);
        }
        sender.setMessage(String("email timeout"));
        sender.startSending();
      }
      else
      {
        Serial.println();
        Serial.println(F("Email: Timeout"));
      }
      client.stop();
    }
  }

  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String("email disconnecting"));
    sender.startSending();
  }
  else
  {
    Serial.println();
    Serial.println(F("Email: Disconnecting."));
  }
  client.stop();
  return 1;
}

void rebootRouter(){
  // Flip Relay
  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String("flipping relay"));
    sender.startSending();
    digitalWrite(ledred, HIGH);
    digitalWrite(relay,LOW);
    delay(timeOffOnFlip);
    digitalWrite(ledred, LOW);
    digitalWrite(relay,HIGH);
  }
  else
  {
    Serial.println(F("Flipping Relay"));
    digitalWrite(ledgrn, LOW);
    digitalWrite(ledred, HIGH);
    digitalWrite(relay,LOW);
    delay(timeOffOnFlip);
    digitalWrite(ledgrn, HIGH);
    digitalWrite(ledred, LOW);
    digitalWrite(relay,HIGH);
  }
  flippedRelay = ++flippedRelay;

  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String("waiting for router to reboot"));
    sender.startSending();
  }
  else
  {
    Serial.println(F("Waiting for Router to Reboot..."));
  }
  delay(timeToWaitOnFlip);
}

void checkForReboot() {
  char outBuf[64];
  char inChar;

  if (client.connect(rebootServer, 80)) {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("checking for forced reboot"));
      sender.startSending();
    }
    else
    {
     Serial.println(F("Checking For Forced Reboot."));
    }
    // Make a HTTP request:
    sprintf(outBuf,"GET %s HTTP/1.1",rebootPageName);
    client.println(outBuf);
    sprintf(outBuf,"Host: %s HTTP/1.1",rebootServer);
    client.println(outBuf);
    client.println("Connection: close");
    client.println();

    while(client.connected()) {
      if (client.available()) {
        inChar = client.read();
        if (inChar == '<') {
          inChar = client.read();
          if(inChar != RemoteReboot)
          {
              RemoteReboot = inChar;
              // Reboot Router
              rebootRouter();
          }else{
            if(morseCode==1)
            {
              while(sender.continueSending())
              {
                delay(100);
              }
              sender.setMessage(String("no force reboot needed"));
              sender.startSending();
            }
            else
            {
              Serial.println(F("No Force Reboot Needed"));
            }
          }
        }
      }
    }
  } 
  else {
    if(morseCode==1)
    {
      while(sender.continueSending())
      {
        delay(100);
      }
      sender.setMessage(String("connection failed"));
      sender.startSending();
    }
    else
    {
     Serial.println(F("Connection Failed"));
    }
  }

  if(morseCode==1)
  {
    while(sender.continueSending())
    {
      delay(100);
    }
    sender.setMessage(String("disconnecting"));
    sender.startSending();
  }
  else
  {
   Serial.println(F("Disconnecting."));
  }
  client.stop();
}