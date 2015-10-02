#include <Ticker.h>

#include <Time.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "mainPage.h"

#define CLOCK_NAME "ESP-CLOCK"
#define WIFI_AP_NAME CLOCK_NAME

#define SETUP_PIN 3

#define MODE_SETUP 0
#define MODE_CLOCK 1
int clockMode;
time_t syncInterval = 3600;

ESP8266WebServer server (80);

String clockName = "";
String w_ssid;
String w_psk;
String httpUpdateResponse;
long timezone;
IPAddress timeServer(129, 6, 15, 28);
time_t prevDisplay = 0;

void handleRoot() {
  String s = MAIN_page;
  s.replace("@@SSID@@", w_ssid);
  s.replace("@@PSK@@", w_psk);
  s.replace("@@TZ@@", String(timezone));
  s.replace("@@HOUR@@", String(hour()));
  s.replace("@@MIN@@", String(minute()));
  s.replace("@@NTPSRV@@", ipToString(timeServer));
  s.replace("@@NTPINT@@", String(syncInterval));
  s.replace("@@SYNCSTATUS@@", timeStatus() == timeSet ? "OK" : "Overdue");
  s.replace("@@CLOCKNAME@@", clockName);
  s.replace("@@UPDATERESPONSE@@", httpUpdateResponse);
  httpUpdateResponse = "";
  server.send(200, "text/html", s);
}

void handleForm() {
  String update_wifi = server.arg("update_wifi");
  String t_ssid = server.arg("ssid");
  String t_psk = server.arg("psk");
  timeServer = parseIP(server.arg("ntpsrv"));
  if (update_wifi == "1") {
    w_ssid = t_ssid;
    w_psk = t_psk;
  }
  String tz = server.arg("timezone");

  if (tz.length()) {
    timezone = tz.toInt();
  }
  setTime(getNtpTime());

  String syncInt = server.arg("ntpint");
  syncInterval = syncInt.toInt();

  clockName = server.arg("clockname");

  httpUpdateResponse = "The configuration was updated.";

  //server.send(200, "text/plain", response);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Moved");
  //handleRoot();
  saveSettings();
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
  readSettings();
  // Wait up to 5s for GPIO0 to go low to enter AP/setup mode.
  displayBusy(0);
  while (millis() < 5000) {
    if (digitalRead(SETUP_PIN) == 0 || !w_ssid.length()) {
      stopDisplayBusy();
      return setupAP();
    }
    delay(50);
  }
  stopDisplayBusy();
  setupSTA();
}

#define EEPROM_WIFI_SIZE 512
#define EEPROM_MAGIC "NtPc"
#define EEPROM_MAGIC_OFFSET 0
#define EEPROM_MAGIC_LENGTH 4
#define EEPROM_SSID_OFFSET EEPROM_MAGIC_OFFSET + EEPROM_MAGIC_LENGTH
#define EEPROM_SSID_LENGTH 32
#define EEPROM_PSK_OFFSET EEPROM_SSID_OFFSET + EEPROM_SSID_LENGTH
#define EEPROM_PSK_LENGTH 64
#define EEPROM_TZ_OFFSET EEPROM_PSK_OFFSET + EEPROM_PSK_LENGTH
#define EEPROM_TIMESERVER_OFFSET EEPROM_TZ_OFFSET + 1
#define EEPROM_TIMESERVER_LENGTH 4
#define EEPROM_INTERVAL_OFFSET EEPROM_TIMESERVER_OFFSET + EEPROM_TIMESERVER_LENGTH
#define EEPROM_INTERVAL_LENGTH 2
#define EEPROM_NAME_OFFSET EEPROM_INTERVAL_OFFSET + EEPROM_INTERVAL_LENGTH
#define EEPROM_NAME_LENGTH 32

void readSettings() {
  EEPROM.begin(EEPROM_WIFI_SIZE);
  // Read the magic header and validate it.
  String magic;
  for (int i = 0 ; i < EEPROM_MAGIC_LENGTH ; i++) {
    magic += char(EEPROM.read(EEPROM_MAGIC_OFFSET + i));
  }
  if (magic != EEPROM_MAGIC) return; // Invalid settings.
  w_ssid = "";
  w_psk = "";
  for (int i = EEPROM_SSID_OFFSET ;  i < EEPROM_SSID_LENGTH ; i++) {
    char c = char(EEPROM.read(i));
    if (c) {
      w_ssid += c;
    }
  }
  for (int i = EEPROM_PSK_OFFSET ; i < EEPROM_PSK_LENGTH ; i++) {
    char c = char(EEPROM.read(i));
    if (c) {
      w_psk += c;
    }
  }
  timezone = long(EEPROM.read(EEPROM_TZ_OFFSET));
  uint32_t ntpAddr;
  ntpAddr = EEPROM.read(EEPROM_TIMESERVER_OFFSET) << 24;
  ntpAddr |= uint32_t(EEPROM.read(EEPROM_TIMESERVER_OFFSET+1)) << 16;
  ntpAddr |= uint32_t(EEPROM.read(EEPROM_TIMESERVER_OFFSET+2)) << 8;
  ntpAddr |= uint32_t(EEPROM.read(EEPROM_TIMESERVER_OFFSET+3));
  if (ntpAddr > 0) {
    timeServer = IPAddress(ntpAddr);
  }
  syncInterval;
  syncInterval = time_t(EEPROM.read(EEPROM_INTERVAL_OFFSET)) << 8;
  syncInterval |= EEPROM.read(EEPROM_INTERVAL_OFFSET+1);

  clockName = "";
  for (int i = 0 ; i < EEPROM_NAME_LENGTH ; i++) {
    char c = EEPROM.read(EEPROM_NAME_OFFSET + i);
    if (c) {
      clockName += c;
    }
  }
  clockName.replace("+", " ");
  EEPROM.end();
}

void saveSettings() {
  EEPROM.begin(EEPROM_WIFI_SIZE);
  // Zero the eeprom.
  for (int i = 0 ; i < EEPROM_WIFI_SIZE ; i++) {
    EEPROM.write(i, 0);
  }
  // Write the magic header.
  for (int i = 0 ; i < EEPROM_MAGIC_LENGTH ; i++) {
    EEPROM.write(EEPROM_MAGIC_OFFSET + i, EEPROM_MAGIC[i]);
  }
  // Write the SSID.
  for (int i = 0 ; i < w_ssid.length() ; i++) {
    EEPROM.write(i + EEPROM_SSID_OFFSET, w_ssid.charAt(i));
  }
  // Write the PSK.
  for (int i = 0 ; i < w_psk.length() ; i++) {
    EEPROM.write(i + EEPROM_PSK_OFFSET, w_psk.charAt(i));
  }
  // Write the TZ offset.
  EEPROM.write(EEPROM_TZ_OFFSET, timezone);
  // Write the timeserver IP.
  EEPROM.write(EEPROM_TIMESERVER_OFFSET, timeServer >> 24);
  EEPROM.write(EEPROM_TIMESERVER_OFFSET+1, (timeServer >> 16) & 0xff);
  EEPROM.write(EEPROM_TIMESERVER_OFFSET+2, (timeServer >> 8) & 0xff);
  EEPROM.write(EEPROM_TIMESERVER_OFFSET+3, timeServer & 0xff);
  // Write the interval.
  EEPROM.write(EEPROM_INTERVAL_OFFSET, (syncInterval >> 8) & 0xff);
  EEPROM.write(EEPROM_INTERVAL_OFFSET+1, syncInterval & 0xff);
  // Write the clock name.
  for (int i = 0 ; i < clockName.length() ; i++) {
    EEPROM.write(EEPROM_NAME_OFFSET + i, clockName.charAt(i));
  }
  EEPROM.end();
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
  w_ssid.toCharArray(ssid, 32);
  w_psk.toCharArray(psk, 64);
  if (w_psk.length()) {
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

