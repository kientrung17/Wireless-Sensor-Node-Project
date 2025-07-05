
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

// Replace with your network credentials (STATION)
const char* ssid = "Zk47";
const char* password = "12345689";

esp_now_peer_info_t slave;
int chan; 

enum MessageType {PAIRING, DATA,};
MessageType messageType;

int counter = 0;

uint8_t clientMacAddress[6];

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  float temp;
  bool status; // Change from unsigned int readingId to bool status
} struct_message;

typedef struct struct_pairing {       // new structure for pairing
    uint8_t msgType;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;

struct_message incomingReadings;
struct_message outgoingSetpoints;
struct_pairing pairingData;

AsyncWebServer server(80);
AsyncEventSource events("/events");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {  font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 1200px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .packet { color: #1b78e2; }
    .card.temperature { color: #fd7e14; }
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ESP-NOW DASHBOARD</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #1 - TEMPERATURE</h4><p><span class="reading"><span id="t1"></span> &deg;C</span></p><p class="packet">Status: <span id="s1"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #2 - TEMPERATURE</h4><p><span class="reading"><span id="t2"></span> &deg;C</span></p><p class="packet">Status: <span id="s2"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #3 - TEMPERATURE</h4><p><span class="reading"><span id="t3"></span> &deg;C</span></p><p class="packet">Status: <span id="s3"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #4 - TEMPERATURE</h4><p><span class="reading"><span id="t4"></span> &deg;C</span></p><p class="packet">Status: <span id="s4"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #5 - TEMPERATURE</h4><p><span class="reading"><span id="t5"></span> &deg;C</span></p><p class="packet">Status: <span id="s5"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #6 - TEMPERATURE</h4><p><span class="reading"><span id="t6"></span> &deg;C</span></p><p class="packet">Status: <span id="s6"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #7 - TEMPERATURE</h4><p><span class="reading"><span id="t7"></span> &deg;C</span></p><p class="packet">Status: <span id="s7"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #8 - TEMPERATURE</h4><p><span class="reading"><span id="t8"></span> &deg;C</span></p><p class="packet">Status: <span id="s8"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #9 - TEMPERATURE</h4><p><span class="reading"><span id="t9"></span> &deg;C</span></p><p class="packet">Status: <span id="s9"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #10 - TEMPERATURE</h4><p><span class="reading"><span id="t10"></span> &deg;C</span></p><p class="packet">Status: <span id="s10"></span></p>
      </div>
      <div class="card temperature">
        <h4><i class="fas fa-thermometer-half"></i> BOARD #11 - TEMPERATURE</h4><p><span class="reading"><span id="t11"></span> &deg;C</span></p><p class="packet">Status: <span id="s11"></span></p>
      </div>
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.temperature.toFixed(2);
  document.getElementById("s"+obj.id).innerHTML = obj.status ? "Connected" : "Disconnected";
 }, false);
}
</script>
</body>
</html>)rawliteral";

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

// ---------------------------- esp_ now -------------------------
void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer(const uint8_t *peer_addr) {      // add pairing
  memset(&slave, 0, sizeof(slave));
  const esp_now_peer_info_t *peer = &slave;
  memcpy(slave.peer_addr, peer_addr, 6);
  
  slave.channel = chan; // pick a channel
  slave.encrypt = 0; // no encryption
  // check if the peer exists
  bool exists = esp_now_is_peer_exist(slave.peer_addr);
  if (exists) {
    // Slave already paired.
    Serial.println("Already Paired");
    return true;
  }
  else {
    esp_err_t addStatus = esp_now_add_peer(peer);
    if (addStatus == ESP_OK) {
      // Pair success
      Serial.println("Pair success");
      return true;
    }
    else 
    {
      Serial.println("Pair failed");
      return false;
    }
  }
} 



struct NodeStatus {
  uint8_t mac[6];
  unsigned long lastSeen;
};

NodeStatus nodes[10];  // Adjust the size according to the number of nodes
const unsigned long NODE_TIMEOUT = 8000; // Timeout for considering a node as disconnected (in milliseconds)

void initNodes() {
  for (int i = 0; i < 10; i++) {
    memset(nodes[i].mac, 0, 6);
    nodes[i].lastSeen = 0;
  }
}

void updateNodeStatus(const uint8_t *mac_addr) {
  bool found = false;
  for (int i = 0; i < 10; i++) {
    if (memcmp(nodes[i].mac, mac_addr, 6) == 0) {
      nodes[i].lastSeen = millis();
      found = true;
      break;
    }
  }
  if (!found) {
    // Add new node to the list
    for (int i = 0; i < 10; i++) {
      if (nodes[i].lastSeen == 0) {  // Empty slot
        memcpy(nodes[i].mac, mac_addr, 6);
        nodes[i].lastSeen = millis();
        break;
      }
    }
  }
}

void checkNodeTimeouts() {
  unsigned long currentTime = millis();
  for (int i = 0; i < 10; i++) {
    if (nodes[i].lastSeen != 0 && (currentTime - nodes[i].lastSeen) > NODE_TIMEOUT) {

      Serial.print("Node disconnected: ");

      StaticJsonDocument<1000> root;
      String payload;

      root["id"] = i+1;
      root["temperature"] = 0;
      root["status"] = false; // Update status
      serializeJson(root, payload);
      Serial.print("event send :");
      serializeJson(root, Serial);
      events.send(payload.c_str(), "new_readings", millis());

      Serial.println();
      // Handle node disconnection (e.g., remove peer, notify, etc.)
      memset(nodes[i].mac, 0, 6);
      nodes[i].lastSeen = 0;
    }
  }
}


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success to " : "Delivery Fail to ");
  printMAC(mac_addr);
  Serial.println();
}

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  Serial.print(len);
  Serial.println(" bytes of new data received.");
  StaticJsonDocument<1000> root;
  String payload;
  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA :                           // the message is data type
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    // create a JSON document with received data and send it by event to the web page
    root["id"] = incomingReadings.id;
    root["temperature"] = round(incomingReadings.temp * 100) / 100.0;
    root["status"] = incomingReadings.status; // Update status

    serializeJson(root, payload);
    Serial.print("event send :");
    serializeJson(root, Serial);
    events.send(payload.c_str(), "new_readings", millis());
    Serial.println();
    break;
  
  case PAIRING:                            // the message is a pairing request 
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    Serial.println(pairingData.msgType);
    Serial.println(pairingData.id);
    Serial.print("Pairing request from MAC Address: ");
    printMAC(pairingData.macAddr);
    Serial.print(" on channel ");
    Serial.println(pairingData.channel);

    clientMacAddress[0] = pairingData.macAddr[0];
    clientMacAddress[1] = pairingData.macAddr[1];
    clientMacAddress[2] = pairingData.macAddr[2];
    clientMacAddress[3] = pairingData.macAddr[3];
    clientMacAddress[4] = pairingData.macAddr[4];
    clientMacAddress[5] = pairingData.macAddr[5];

    if (pairingData.id > 0) {     // do not replay to server itself
      if (pairingData.msgType == PAIRING) { 
        pairingData.id = 0;       // 0 is server
        // Server is in AP_STA mode: peers need to send data to server soft AP MAC address 
        WiFi.softAPmacAddress(pairingData.macAddr);
        Serial.print("Pairing MAC Address: ");
        printMAC(clientMacAddress);
        pairingData.channel = chan;
        Serial.println(" send response");
        esp_err_t result = esp_now_send(clientMacAddress, (uint8_t *) &pairingData, sizeof(pairingData));
        addPeer(clientMacAddress);
      }  
    }  
    break; 
  }
  updateNodeStatus(mac_addr);
}

void initESP_NOW(){
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
} 

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("Server MAC Address: ");
  readMacAddress();

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }

  Serial.print("Server SOFT AP MAC Address:  ");
  Serial.println(WiFi.softAPmacAddress());

  chan = WiFi.channel();
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  initESP_NOW();
  
  // Start Web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  // Events 
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  // start server
  server.begin();
}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 200;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping", NULL, millis());
    lastEventTime = millis();
    // readDataToSend();
    // esp_now_send(NULL, (uint8_t *) &outgoingSetpoints, sizeof(outgoingSetpoints));
  }
  checkNodeTimeouts(); 
}
