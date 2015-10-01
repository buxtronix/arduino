
unsigned int localPort = 2390;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
byte sendBuffer[] = {
  0b11100011,          // LI, Version, Mode.
  0x0,                 // Stratum unspecified.
  0x6,                 // Polling interval
  0xEC,                // Clock precision.
  0x0, 0x0, 0x0, 0x0}; // Reference ...
WiFiUDP udp;

String ipToString(IPAddress ip) {
  uint32_t addr = ip;
  String ret;
  //ret = String(addr >> 24) + "." + String(addr >> 16 & 0xff) + "." + String(addr >> 8 & 0xff) + "." + String(addr & 0xff);
  ret = String(addr & 0xff) + "." + String(addr >> 8 & 0xff) + "." + String(addr >> 16 & 0xff) + "." + String(addr >> 24);
  return ret;
}

IPAddress parseIP(String ipaddr) {
 // charArray of ipaddr.
 char ip[16];
 uint32_t address = 0;
 int o = 0;
 char octet[3];
 char digits = 0; // per octet.
 if (ipaddr.length() > 16) goto badIP;
 ipaddr.toCharArray(ip, 16);
 for (int i = 0 ; ip[i] != '\0' ; i++) {
   if (ip[i] == '.') {
     uint8_t oct = atoi(octet);
     if (digits < 1 || digits > 3 || oct > 0xff) {
       goto badIP;
     }
     address |= oct << (8 * o);
     if (o++ > 2) goto badIP;
     memset(octet, 0, 4); digits = 0;
   } else if (ip[i] < '0' || ip[i] > '9' || digits > 2) {
    goto badIP;
   } else {
     octet[digits++] = ip[i];
   }
 }
 address |= atoi(octet) << 24;
 return IPAddress(address);
 badIP:
 return IPAddress(0, 0, 0, 0);
}

void setupTime() {
  udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(syncInterval);
}

time_t getNtpTime()
{
  time_t recvTime = 0;
  while (udp.parsePacket() > 0) ; // discard any previously received packets
  for (int i = 0 ; i < 5 && recvTime == 0 ; i++) { // 5 retries.
    sendNTPpacket(timeServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      if (udp.parsePacket()) {
         udp.read(packetBuffer, NTP_PACKET_SIZE);
         // Extract seconds portion.
         unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
         unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
         unsigned long secSince1900 = highWord << 16 | lowWord;
         recvTime = secSince1900 - 2208988800UL + timezone * SECS_PER_HOUR;
         break;
      }
      yield();
    }
  }
  timeRet:
  return recvTime; // return 0 if unable to get the time
}


void sendNTPpacket(IPAddress &address) {
  // Zeroise the buffer.
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  memcpy(packetBuffer, sendBuffer, 16);

  udp.beginPacket(address, 123);  // Port 123.
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
