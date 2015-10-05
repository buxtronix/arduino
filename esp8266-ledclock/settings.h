#include <EEPROM.h>

#ifndef SETTINGS_H
#define SETTINGS_H

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
#define EEPROM_TIMESERVER_LENGTH 32
#define EEPROM_INTERVAL_OFFSET EEPROM_TIMESERVER_OFFSET + EEPROM_TIMESERVER_LENGTH
#define EEPROM_INTERVAL_LENGTH 2
#define EEPROM_NAME_OFFSET EEPROM_INTERVAL_OFFSET + EEPROM_INTERVAL_LENGTH
#define EEPROM_NAME_LENGTH 32

#define DEFAULT_TIMESERVER "time.nist.gov"
#define MINIMUM_INTERVAL 60

#define CLOCK_NAME "ESP-CLOCK"
#define WIFI_AP_NAME CLOCK_NAME

class Settings {

  public:
    Settings() {};

    void Load() {
      char buffer[EEPROM_WIFI_SIZE];
      EEPROM.begin(EEPROM_WIFI_SIZE);
      for (int i = 0 ; i < EEPROM_WIFI_SIZE ; i++) {
        buffer[i] = EEPROM.read(i);
      }
      EEPROM.end();

      // Verify magic;
      String magic;
      for (int i = EEPROM_MAGIC_OFFSET ; i < EEPROM_MAGIC_OFFSET+EEPROM_MAGIC_LENGTH ; i++) {
        magic += buffer[i];
      }
      if (magic != EEPROM_MAGIC) {
        return;
      }
      // Read SSID
      ssid = "";
      for (int i = EEPROM_SSID_OFFSET ; i < EEPROM_SSID_OFFSET+EEPROM_SSID_LENGTH ; i++) {
        if (buffer[i]) ssid += buffer[i];
      }
      // Read PSK
      psk = "";
      for (int i = EEPROM_PSK_OFFSET ; i < EEPROM_PSK_OFFSET+EEPROM_PSK_LENGTH ; i++) {
        if (buffer[i]) psk += buffer[i];
      }

      timezone = long(buffer[EEPROM_TZ_OFFSET]);

      strncpy(timeserver, &buffer[EEPROM_TIMESERVER_OFFSET], EEPROM_TIMESERVER_LENGTH);
      if (strlen(timeserver) < 1) {
        strcpy(timeserver, DEFAULT_TIMESERVER);
      }

      interval = time_t(buffer[EEPROM_INTERVAL_OFFSET]) << 8;
      interval |= buffer[EEPROM_INTERVAL_OFFSET+1];
      if (interval < MINIMUM_INTERVAL) {
        interval = MINIMUM_INTERVAL;
      }
      // Clock name.
      name = "";
      for (int i = EEPROM_NAME_OFFSET ; i < EEPROM_NAME_OFFSET+EEPROM_NAME_LENGTH ; i++) {
        if (buffer[i]) name += buffer[i];
      }
    }

    void Save() {
      unsigned char buffer[EEPROM_WIFI_SIZE];
      memset(buffer, 0, EEPROM_WIFI_SIZE);

      // Copy magic to buffer;
      strncpy((char *)buffer, EEPROM_MAGIC, EEPROM_MAGIC_LENGTH);

      // Copy SSID to buffer;
      ssid.getBytes(&buffer[EEPROM_SSID_OFFSET], EEPROM_SSID_LENGTH, 0);
      // Copy PSK to buffer.
      psk.getBytes(&buffer[EEPROM_PSK_OFFSET], EEPROM_PSK_LENGTH, 0);
      // Copy timezone.
      buffer[EEPROM_TZ_OFFSET] = timezone;
      // Copy timeserver.
      strncpy((char *)&buffer[EEPROM_TIMESERVER_OFFSET], (char *)timeserver, EEPROM_TIMESERVER_LENGTH);
      // Copy interval.
      buffer[EEPROM_INTERVAL_OFFSET] = interval >> 8;
      buffer[EEPROM_INTERVAL_OFFSET+1] = interval & 0xff;
      // Copy clock name.
      name.getBytes(&buffer[EEPROM_NAME_OFFSET], EEPROM_NAME_LENGTH, 0);

      // Write to EEPROM.
      EEPROM.begin(EEPROM_WIFI_SIZE);
      for (int i = 0 ; i < EEPROM_WIFI_SIZE ; i++) {
        EEPROM.write(i, buffer[i]);
      }
      EEPROM.end();
    }

    String ssid;
    String psk;
    long timezone;
    char timeserver[64];
    int interval;
    String name;
};

Settings settings;

#endif


