//by lisper <lisper.li@dfrobot.com>
//GPL 3.0

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   7  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
		SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "DFRobot-Internal"           // cannot be longer than 32 characters!
#define WLAN_PASS       "zwrobot2014"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           80   // What TCP port to listen on for connections.

Adafruit_CC3000_Server webServer(LISTEN_PORT);

void setup(void) {
	pinMode (13, OUTPUT);
	pinMode (12, OUTPUT);
	Serial.begin(115200);
	Serial.println(F("Hello, CC3000!\n")); 
	//while (!Serial);
	//while (!Serial.available ());
	//Serial.println ("Input any key to start:");
	Serial.print("Free RAM: "); 
	Serial.println(getFreeRam(), DEC);

	/* Initialise the module */
	Serial.println(F("\nInitializing..."));
	if (!cc3000.begin()) {
		Serial.println(F("Couldn't begin()! Check your wiring?"));
		while(1);
	}

	Serial.print(F("\nAttempting to connect to ")); 
	Serial.println(WLAN_SSID);
	if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
		Serial.println(F("Failed!"));
		while(1);
	}

	Serial.println(F("Connected!"));

	Serial.println(F("Request DHCP"));
	while (!cc3000.checkDHCP()) {
		delay(100); // ToDo: Insert a DHCP timeout!
	}  

	/* Display the IP address DNS, Gateway, etc. */
	while (! displayConnectionDetails()) {
		delay(1000);
	}

	/*********************************************************/
	/* You can safely remove this to save some flash memory! */
	/*********************************************************/


	// Start listening for connections
	webServer.begin();

	Serial.println(F("Listening for connections..."));
}

void loop(void) {
	char databuffer[45];
	// Try to get a client which is connected.
	Adafruit_CC3000_ClientRef client = webServer.available();
	if (client) {
		while (client.available ()) {
			client.read (databuffer, 40);
			char* sub = strchr (databuffer, '\r');
			if (sub > 0)
				*sub = '\0';
			sub = strstr (databuffer, "control");
			if (!sub)
				break;
			sub = strstr (sub, "led");
			if (!sub)
				break; 
			sub++;
			if (strncmp (sub, "open", 4) == 0) {
				Serial.println ("clicked open");
				digitalWrite (12, HIGH);  
				digitalWrite (13, HIGH);  
			} 
			else if (strncmp (sub, "close", 5) == 0) {
				Serial.println ("clicked close");
				digitalWrite (12, LOW);
				digitalWrite (13, LOW);
			}
			break;

		}
		webServer.write ("<!DOCTYPE html>");
		webServer.write ("<html>");
		webServer.write ("<body>");
		webServer.write ("<form action=\"control\" method=\"get\">");
		webServer.write ("<button name=\"led\" type=\"submit\" value=\"open\">Open</button><br />");
		webServer.write ("<button name=\"led\" type=\"submit\" value=\"close\">Close</button>");
		webServer.write ("</form>");
		webServer.write ("</body>");
		webServer.write ("</html>");
		client.close();
	}
}


/**************************************************************************/
/*!
  @brief  Tries to read the IP address and other connection details
  */
/**************************************************************************/
bool displayConnectionDetails(void) {
	uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

	if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv)) {
		Serial.println(F("Unable to retrieve the IP Address!\r\n"));
		return false;
	} else {
		Serial.print(F("\nIP Addr: ")); 
		cc3000.printIPdotsRev(ipAddress);
		Serial.print(F("\nNetmask: ")); 
		cc3000.printIPdotsRev(netmask);
		Serial.print(F("\nGateway: ")); 
		cc3000.printIPdotsRev(gateway);
		Serial.print(F("\nDHCPsrv: ")); 
		cc3000.printIPdotsRev(dhcpserv);
		Serial.print(F("\nDNSserv: ")); 
		cc3000.printIPdotsRev(dnsserv);
		Serial.println();
		return true;
	}
}


