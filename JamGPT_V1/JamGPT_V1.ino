// required libraries 
#include <ArduinoJson.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

//API endpoint details
const char host[] = "https://api.openai.com";
const char path[] = "/v1/chat/completions";
const int port = 443;

//prompt options
const String promptOptions[]{
  "In the style of an angry producer, write a prompt they would shout at their musicians to help overcome writer's block.",
  "In the voice of a wise philosopher, provide a thought-provoking statement that can serve as a starting point for song ideas.",
};


const int numPromptOptions = sizeof(promptOptions) / sizeof(promptOptions[0]); //dynamically changes depending on how many prompts 

char ssid[] = "X";   //network address- needs to be 2.4GHz for Arduino nano 33 IoT     
char pass[] = "XXX"; //network passworkd
char OpenAI_API_KEY[] = "XXXX"; //chatGPT api key- never show your API key 

int status = WL_IDLE_STATUS;
int ledState = LOW;
unsigned long previousMillisInfo = 0;
unsigned long previousMillisLED = 0;
const int intervalInfo = 5000;

const char *serverHostname = "api.openai.com";
const int serverPort = 443;

WiFiClient client;
HttpClient http = HttpClient(client, host, port);

//---------------------------------------------------------------

//For the thermal printer
#include "Adafruit_Thermal.h"

const int redPin = 9; //r
const int greenPin = 11;//g
const int bluePin = 10;//b

const int switchPin = 7; //footswitch
const int outputPin = 6; //

int toneKnob = analogRead(A0);
int potPin = 0;

const int hapticMotorPin = 2;

Adafruit_Thermal printer(&Serial1);

void setup() {
  Serial.begin(9600);
  //+0while (!Serial)

  //Formatting printer
  printer.wake();
  printer.justify('C');  //Center
  printer.setFont('B');  //Bold 
  printer.println();

  printer.print("□ □ □ □ □ JamGPT □ □ □ □ □");
  printer.println();
  printer.println("hello");

  printer.println();
  printer.println();
  printer.println();
  printer.println();
  printer.println();
  printer.println();
  printer.println();
  printer.println(); //new lines act as physical spacers when being printed 

  printer.sleep();

  pinMode(LED_BUILTIN, OUTPUT);

  //connect to network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(2500);

    Serial.println("You're connected to the network");
    Serial.println("---------------------------------------");
    Serial.println("Board Information:");
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    Serial.println();
    Serial.println("Network Information:");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
  }

  //---------------------------------------------
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(outputPin, OUTPUT);
  pinMode(A0, INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  int potVal = analogRead(potPin);
  int mappedVal = map(potVal, 0, 1023, 0, 8);

  int toneK = map(analogRead(A0), 0, 1023, numPromptOptions, 1);
  int promptIndex = toneK;
  String currentPrompt = promptOptions[promptIndex];

  static boolean oldSwitchState = digitalRead(switchPin);
  boolean newSwitchState = digitalRead(switchPin);

  if (newSwitchState != oldSwitchState) {
    // Switch has changed state. Remember the new state
    oldSwitchState = newSwitchState;

    Serial.print("tone = ");
    Serial.println(toneK);

    printer.wake();
    printer.justify('C');
    printer.setFont('B');
    printer.println();

    printer.print("tone : ");
    printer.println(toneK);
    printer.println("sending...");
    printer.println(currentPrompt);

    printer.println();
    printer.println();
    printer.println();
    printer.println();
    printer.println();
    printer.println();
    printer.println();
    printer.println();


    if (client.connectSSL(serverHostname, serverPort)) {
      Serial.println("Connected to OpenAI ChatGPT server");
      Serial.println("String is: " + currentPrompt);

      String requestBody = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + currentPrompt + "----\",\"max_tokens\":1200}";
      String request = "POST /v1/completions HTTP/1.1\r\n";
      request += "Host: api.openai.com\r\n";
      request += "Authorization: Bearer " + String(OpenAI_API_KEY) + "\r\n";
      request += "Content-Type: application/json\r\n";
      request += "Content-Length: " + String(requestBody.length()) + "\r\n";
      request += "Connection: close\r\n\r\n";
      request += requestBody;

      client.print(request);

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          break;
        }
      }

      String response = client.readString();
      Serial.println("Response:");
      Serial.println(response);

      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;

        printer.println("error");
      }

      const char *text = doc["choices"][0]["text"].as<const char *>();

      if (text != nullptr) {
        Serial.println("GPT-generated text:");
        Serial.println(text);

        digitalWrite(outputPin, HIGH);
        delay(100);  // Pulse length
        digitalWrite(outputPin, LOW);
        delay(500);


        printer.println(text);

        printer.sleep();
        printer.setDefault();
        delay(200L);

      } else {
        Serial.println("Failed to extract text from response.");
      }
    }
    delay(50);
  }

  //for the RGB LED- changes to what prompt it set
  //needs to have same number of cases as prompts 
  switch (mappedVal) {

    case 0:
      setColor(156, 39, 176);  // Purple
      break;
    case 1:
      setColor(244, 67, 54);  // Red
      break;
    case 2:
      setColor(233, 30, 99);  // Pink
      break;
    case 3:
      setColor(63, 81, 181);  // Indigo
      break;
    case 4:
      setColor(33, 150, 243);  // Blue
      break;
    case 5:
      setColor(76, 175, 80);  // Green
      break;
    case 6:
      setColor(255, 193, 7);  // Amber
      break;
    case 7:
      setColor(255, 87, 34);  // Deep Orange
      break;
  }
}

void setColor(int redVal, int greenVal, int blueVal) {
  analogWrite(redPin, redVal);
  analogWrite(greenPin, greenVal);
  analogWrite(bluePin, blueVal);
}