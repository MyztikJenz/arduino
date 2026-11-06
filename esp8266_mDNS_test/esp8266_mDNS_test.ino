#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "wifi_credentials.h"

#define SERVICE_PORT 1234
WiFiServer server(SERVICE_PORT);
bool bHostDomainConfirmed = false;
char *pcHostDomain = NULL;
MDNSResponder::hMDNSService hMDNSService = NULL;
MDNSResponder::hMDNSServiceQuery hMDNSServiceQuery = NULL;

void setup() {
    Serial.begin(9600);
    delay(100);
    Serial.println(F("Well, hello there..."));

    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Netmask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());

    server.begin();

    MDNS.setHostProbeResultCallback(hostProbeCallback);
    
//    hMDNSService = MDNS.addService(NULL, "http", "tcp", 1234);
//    if (hMDNSService) {
//        MDNS.setServiceProbeResultCallback(hMDNSService, serviceProbeResult);
//    }

    if (!MDNSResponder::indexDomain(pcHostDomain, 0, "arduino_1") || !MDNS.begin(pcHostDomain)) { 
        Serial.println("Crud... mDNS did not start");
    }
    else {
        Serial.println("mDNS should be working");
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    MDNS.update();
}


void serviceProbeResult(String p_pcServiceName, const MDNSResponder::hMDNSService p_hMDNSService, bool p_bProbeResult) {
    (void) p_hMDNSService;
    Serial.printf("MDNSServiceProbeResultCallback: Service '%s' probe %s\n", p_pcServiceName.c_str(), (p_bProbeResult ? "succeeded." : "failed!"));
}


void hostProbeCallback(String p_pcDomainName, bool p_bProbeResult) {

  Serial.printf("MDNSHostProbeResultCallback: Host domain '%s.local' is %s\n", p_pcDomainName.c_str(), (p_bProbeResult ? "free" : "already USED!"));

  if (true == p_bProbeResult) {
    // Set station hostname
//    setStationHostname(pcHostDomain);

    if (!bHostDomainConfirmed) {
      // Hostname free -> setup clock service
      bHostDomainConfirmed = true;

      if (!hMDNSService) {
        // Add a 'http.tcp' service to port 'SERVICE_PORT', using the host domain as instance domain
        hMDNSService = MDNS.addService(0, "jimt", "tcp", SERVICE_PORT);
        if (hMDNSService) {
          MDNS.setServiceProbeResultCallback(hMDNSService, serviceProbeResult);

          // Add some '_http._tcp' protocol specific MDNS service TXT items
          // See: http://www.dns-sd.org/txtrecords.html#http
//          MDNS.addServiceTxt(hMDNSService, "user", "");
//          MDNS.addServiceTxt(hMDNSService, "password", "");
//          MDNS.addServiceTxt(hMDNSService, "path", "/");
        }

        // Install dynamic 'http.tcp' service query
        if (!hMDNSServiceQuery) {
          hMDNSServiceQuery = MDNS.installServiceQuery("jimt", "tcp", MDNSServiceQueryCallback);
          if (hMDNSServiceQuery) {
            Serial.printf("MDNSProbeResultCallback: Service query for 'http.tcp' services installed.\n");
          } else {
            Serial.printf("MDNSProbeResultCallback: FAILED to install service query for 'http.tcp' services!\n");
          }
        }
      }
    }
  } else {
    // Change hostname, use '-' as divider between base name and index
    if (MDNSResponder::indexDomain(pcHostDomain, "-", 0)) {
      MDNS.setHostname(pcHostDomain);
    } else {
      Serial.println("MDNSProbeResultCallback: FAILED to update hostname!");
    }
  }
}


void MDNSServiceQueryCallback(MDNSResponder::MDNSServiceInfo serviceInfo, MDNSResponder::AnswerType answerType, bool p_bSetContent) {
  String answerInfo;
  switch (answerType) {
    case MDNSResponder::AnswerType::ServiceDomain :
      answerInfo = "ServiceDomain " + String(serviceInfo.serviceDomain());
      break;
    case MDNSResponder::AnswerType::HostDomainAndPort :
      answerInfo = "HostDomainAndPort " + String(serviceInfo.hostDomain()) + ":" + String(serviceInfo.hostPort());
      break;
    case MDNSResponder::AnswerType::IP4Address :
      answerInfo = "IP4Address ";
      for (IPAddress ip : serviceInfo.IP4Adresses()) {
        answerInfo += "- " + ip.toString();
      };
      break;
    case MDNSResponder::AnswerType::Txt :
      answerInfo = "TXT " + String(serviceInfo.strKeyValue());
      for (auto kv : serviceInfo.keyValues()) {
        answerInfo += "\nkv : " + String(kv.first) + " : " + String(kv.second);
      }
      break;
    default :
      answerInfo = "Unknown Answertype";
  }
  Serial.printf("Answer %s %s\n", answerInfo.c_str(), p_bSetContent ? "Modified" : "Deleted");
}
