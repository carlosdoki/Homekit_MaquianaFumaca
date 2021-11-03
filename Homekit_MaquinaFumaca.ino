#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <arduino_homekit_server.h>;

// Set web server port number to 80
WiFiServer server(80);

WiFiManager wifiManager;

#define LOG_D(fmt, ...) printf_P(PSTR(fmt "\n"), ##__VA_ARGS__);

String output4State = "off";
int first = 1;

const int SHORT_PRESS_TIME = 15000; // 1000 milliseconds
const int LONG_PRESS_TIME = 15000;  // 1000 milliseconds

// Variable to store the HTTP request
String header;

const int led = 2;

const int button = 5;

const int switch1 = 4;

int lastState = LOW;

int currentState;

unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

bool isPressing = false;
bool isLongDetected = true;

extern "C" homekit_server_config_t config;

extern "C" homekit_characteristic_t cha_switch_on1;

//Called when the switch value is changed by iOS Home APP
void cha_switch_on1_setter(const homekit_value_t value)
{
  cha_switch_on1.value.bool_value = !value.bool_value; //sync the value
  LOG_D("Switch2: %s", !value.bool_value ? "ON" : "OFF");
  digitalWrite(switch1, value.bool_value);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Initialize the output variables as outputs
  pinMode(led, OUTPUT);

  pinMode(switch1, OUTPUT);

  pinMode(button, INPUT);

  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveParamsCallback(saveWifiCallback);
  wifiManager.autoConnect("Prophetic-00003");

  cha_switch_on1.setter = cha_switch_on1_setter;

  arduino_homekit_setup(&config);

  digitalWrite(switch1, LOW);

  server.begin();
}

void loop()
{
  if (first == 1)
  {
    LOG_D(" *************** First *********************");
    digitalWrite(switch1, HIGH);
    delay(1000);
    digitalWrite(switch1, LOW);
    delay(1000);
    cha_switch_on1.value.bool_value = 0;
    homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    first = 0;
  }

  wifi_doki();
  arduino_homekit_loop();

  // put your main code here, to run repeatedly:
  currentState = digitalRead(button);

  if (lastState == HIGH && currentState == LOW)
  { // button is pressed
    pressedTime = millis();
    isPressing = false;
    // isLongDetected = false;
  }
  else if (lastState == LOW && currentState == HIGH)
  { // button is released
    isPressing = true;
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration < SHORT_PRESS_TIME)
    {
      // Serial.println("A short press is detected");
      isLongDetected = false;
      digitalWrite(led, LOW);
      invert_light(1);
    }
  }

  if (isPressing == true && isLongDetected == false)
  {
    long pressDuration = millis() - pressedTime;

    if (pressDuration > LONG_PRESS_TIME)
    {
      Serial.println("A long press is detected");
      wifiManager.resetSettings();
      ESP.restart();
      wifiManager.autoConnect("Prophetic-00003");
      isLongDetected = true;
      isPressing == false;
    }
  }

  // save the the last state
  lastState = currentState;
  delay(500);
}

extern "C" void identify_switch_1(homekit_value_t _value)
{
  invert_light(1);
  printf("identify switch 1\n");
  isLongDetected = true;
}

extern "C" void identify_accessory(homekit_value_t _value)
{
  invert_light(1);
  printf("identify accessory\n");
}

void invert_light(int button)
{
  switch (button)
  {
  case 1:
    Serial.println("A short press 1 is detected");
    LOG_D("Switch1: %s", cha_switch_on1.value.bool_value ? "ON" : "OFF");
    if (!cha_switch_on1.value.bool_value)
    {
      output4State = "ON";
    }
    else
    {
      output4State = "OFF";
    }
    cha_switch_on1.value.bool_value = !cha_switch_on1.value.bool_value;
    digitalWrite(switch1, cha_switch_on1.value.bool_value);
    homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
    break;
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  digitalWrite(led, HIGH);
}

void saveWifiCallback()
{
  digitalWrite(led, LOW);
}

void wifi_doki()
{
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client)
  {                                // If a new client connects,
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /4/on") >= 0)
            {
              Serial.println("GPIO 4 on");
              output4State = "on";
              cha_switch_on1.value.bool_value = 1;
              digitalWrite(switch1, cha_switch_on1.value.bool_value);
              homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
            }
            else if (header.indexOf("GET /4/off") >= 0)
            {
              Serial.println("GPIO 4 off");
              output4State = "off";
              cha_switch_on1.value.bool_value = 0;
              digitalWrite(switch1, cha_switch_on1.value.bool_value);
              homekit_characteristic_notify(&cha_switch_on1, cha_switch_on1.value);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>Prophetic Church</h1>");

            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<p>Maquina Fumaca " + output4State + "</p>");
            // If the output4State is off, it displays the ON button
            if (output4State == "off")
            {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            }
            else
            {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}