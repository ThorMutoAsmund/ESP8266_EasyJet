#include <Arduino.h>

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266HTTPClient.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// #define HORSEKILDEN
#define BOLVERK

#define LED_ON LOW
#define LED_OFF HIGH
#define RELAY_ON LOW
#define RELAY_OFF HIGH
#define RELAY1_PORT D2
#define RELAY2_PORT D3

#ifdef HORSEKILDEN
const char* ssid = "Fukushima";
const char* password = "";
#endif

#ifdef BOLVERK
const char* ssid = "Bolverk";
const char* password = "********";
#endif
const char* website = "<html><head><meta name=\"viewport\" content=\"width=device-width\"></head><body>R1: <form action=\"post\" method=\"post\"><input type=\"submit\" name=\"r1\" value=\"on\" /><input type=\"submit\" name=\"r1\" value=\"off\" /></form><br/><br/>R2: <form action=\"post\" method=\"post\"><input type=\"submit\" name=\"r2\" value=\"on\" /><input type=\"submit\" name=\"r2\" value=\"off\" /></form></body></html>";

const char* PARAM_R1 = "r1";
const char* PARAM_R2 = "r2";
const char* VALUE_ON = "on";
const char* VALUE_OFF = "off";

const char* github_host = "api.github.com";
const char* bolverk_host = "bolverkxr.azurewebsites.net";

const uint8_t github_fingerprint[20] = { 0x29,0x70,0x30,0x74,0xCA,0x3C,0x48,0xF5,0x4A,0x79,0xC6,0x2D,0x11,0x57,0xA2,0x41,0x2A,0x2D,0x7D,0x5C };
const uint8_t bolverk_fingerprint[20] = { 0x0B,0xAE,0xB8,0xB3,0xEA,0x3C,0x84,0x9D,0x87,0x80,0x89,0x12,0x2C,0x2B,0x63,0xB5,0x07,0xB1,0x06,0x8A };

void ledDisplay(const String ip);
void postIPAddress(const String ip);
void getIPAddress();
void setupServer();

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void setup()
{
    Serial.begin(9600);
    // while (!Serial) ;
    
    pinMode(LED_BUILTIN, OUTPUT); 
    pinMode(RELAY1_PORT, OUTPUT); 
    pinMode(RELAY2_PORT, OUTPUT); 
    digitalWrite(LED_BUILTIN, LED_OFF);
    digitalWrite(RELAY1_PORT, RELAY_OFF);
    digitalWrite(RELAY2_PORT, RELAY_OFF);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    // ledDisplay(WiFi.localIP().toString());
    postIPAddress(WiFi.localIP().toString());

    setupServer();
}

void postIPAddress(const String ip)
{
    WiFiClientSecure client;
    HTTPClient http;
    
    client.setFingerprint(bolverk_fingerprint);

    Serial.print("connecting to ");
    Serial.println(bolverk_host);
    if (!client.connect(bolverk_host, 443)) 
    {
        Serial.println("Secure connection failed");
        return;
    }

    Serial.println("Secure connection OK");

    client.print(String("GET ") + "/api/easyjet?value=" + ip + " HTTP/1.1\r\n" + "Host: " + bolverk_host + "\r\n" + "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0)
    {
        if (millis() - timeout > 5000)
        {
            Serial.println("Client timeout");
            client.stop();
            return;
        }
    }

    Serial.println("Client request OK");
}

void setupServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/html", website);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
    {
        String message;
        if (request->hasParam(PARAM_R1, true)) 
        {
            message = request->getParam(PARAM_R1, true)->value();
            if (message == VALUE_ON)
            {
                digitalWrite(RELAY1_PORT, RELAY_ON);
                digitalWrite(LED_BUILTIN, LED_ON);
                delay(200);
                digitalWrite(LED_BUILTIN, LED_OFF);
                request->send(200, "text/html", website);
            }
            else if (message == VALUE_OFF)
            {
                digitalWrite(RELAY1_PORT, RELAY_OFF);
                digitalWrite(LED_BUILTIN, LED_ON);
                delay(200);
                digitalWrite(LED_BUILTIN, LED_OFF);
                request->send(200, "text/html", website);
            }
            else
            {
                request->send(200, "text/plain", "Unknown r1 value");
            }
        } 
        else if (request->hasParam(PARAM_R2, true)) 
        {
            message = request->getParam(PARAM_R2, true)->value();
            if (message == VALUE_ON)
            {
                digitalWrite(RELAY2_PORT, RELAY_ON);
                digitalWrite(LED_BUILTIN, LED_ON);
                delay(200);
                digitalWrite(LED_BUILTIN, LED_OFF);
                request->send(200, "text/html", website);
            }
            else if (message == VALUE_OFF)
            {
                digitalWrite(RELAY2_PORT, RELAY_OFF);
                digitalWrite(LED_BUILTIN, LED_ON);
                delay(200);
                digitalWrite(LED_BUILTIN, LED_OFF);
                request->send(200, "text/html", website);
            }
            else
            {
                request->send(200, "text/plain", "Unknown r1 value");
            }
        } 
        else 
        {
            request->send(200, "text/plain", "Unknown command");
        }
    });

    server.onNotFound(notFound);

    server.begin();
}

void loop() 
{
}

void ledDisplay(const String ip)
{
    delay(2000);
    for (uint16_t c = 0; c < ip.length(); ++c)
    {
        char t = ip[c];
        if (t >= '0' && t <= '9')
        {
            int cnt = t == '0' ? 10 : t - '0';
            for (int n = 0; n < cnt; ++n)
            {
                digitalWrite(LED_BUILTIN, LED_ON);
                delay(150);
                digitalWrite(LED_BUILTIN, LED_OFF);
                delay(350);
            }
        }
        else
        {
            delay(1200);
        }
        delay(800);
    }
}
