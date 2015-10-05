#include <Ticker.h>

#include <Time.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "settings.h"
#include "mainPage.h"

#define SETUP_PIN 3

#define MODE_SETUP 0
#define MODE_CLOCK 1
int clockMode;

ESP8266WebServer server (80);

String httpUpdateResponse;

time_t prevDisplay = 0;

void handleRoot() {
  String s = MAIN_page;
  s.replace("@@SSID@@", settings.ssid);
  s.replace("@@PSK@@", settings.psk);
  s.replace("@@TZ@@", String(settings.timezone));
  s.replace("@@HOUR@@", String(hour()));
  s.replace("@@MIN@@", String(minute()));
  s.replace("@@NTPSRV@@", settings.timeserver);
  s.replace("@@NTPINT@@", String(settings.interval));
  s.replace("@@SYNCSTATUS@@", timeStatus() == timeSet ? "OK" : "Overdue");
  s.replace("@@CLOCKNAME@@", settings.name);
  s.replace("@@UPDATERESPONSE@@", httpUpdateResponse);
  httpUpdateResponse = "";
  server.send(200, "text/html", s);
}

void handleForm() {
  String update_wifi = server.arg("update_wifi");
  String t_ssid = server.arg("ssid");
  String t_psk = server.arg("psk");
  String t_timeserver = server.arg("ntpsrv");
  t_timeserver.toCharArray(settings.timeserver, EEPROM_TIMESERVER_LENGTH, 0);
  if (update_wifi == "1") {
    settings.ssid = t_ssid;
    settings.psk = t_psk;
  }
  String tz = server.arg("timezone");

  if (tz.length()) {
    settings.timezone = tz.toInt();
  }
  
  time_t newTime = getNtpTime();
  if (newTime) {
    setTime(newTime);
  }
  String syncInt = server.arg("ntpint");
  settings.interval = syncInt.toInt();

  settings.name = server.arg("clockname");
  settings.name.replace("+", " ");

  httpUpdateResponse = "The configuration was updated.";

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Moved");

  settings.Save();
  if (update_wifi == "1") {
    delay(500);
    setupWiFi();
  }
}

void setup() {
  setupDisplay();
  pinMode(SETUP_PIN, INPUT);
  digitalWrite(SETUP_PIN, HIGH);
  setupWiFi();
  setupTime();
  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.begin();
}

void loop() {
  server.handleClient();
  if (displayIP()) return;
  if (clockMode == MODE_CLOCK) {
    if (timeStatus() != timeNotSet) {
      if (now() != prevDisplay) { //update the display only if time has changed
        prevDisplay = now();
        displayClock();  
      }
    }
  }
}

void setupWiFi() {
  settings.Load();
  // Wait up to 5s for GPIO0 to go low to enter AP/setup mode.
  displayBusy(0);
  while (millis() < 5000) {
    if (digitalRead(SETUP_PIN) == 0 || !settings.ssid.length()) {
      stopDisplayBusy();
      return setupAP();
    }
    delay(50);
  }
  stopDisplayBusy();
  setupSTA();
}

void setupSTA()
{
  char ssid[32];
  char psk[64];
  memset(ssid, 0, 32);
  memset(psk, 0, 64);
  displayBusy(1);

  clockMode = MODE_CLOCK;
  WiFi.mode(WIFI_STA);
  settings.ssid.toCharArray(ssid, 32);
  settings.psk.toCharArray(psk, 64);
  if (settings.psk.length()) {
    WiFi.begin(ssid, psk);
  } else {
    WiFi.begin(ssid);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  stopDisplayBusy();
  displayDash();
}

void setupAP() {
  clockMode = MODE_SETUP;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_NAME);
  displayAP();
}

