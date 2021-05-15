#define MQTT_SERVER_IP "broker.emqx.io"
#define MQTT_CLIENT_ID "tleb_"
#define MQTT_SUB_TOPIC "smarthome/tleb/1"

void  mqttInit() {
  client.setServer(MQTT_SERVER_IP, 1883);

  while (!client.connected()) {
    mqttConnect();
    client.setCallback(mqttListerner);
  }
}

void mqttConnect() {
  Serial.print("[MQTT] Attempting to connect...");

  String mqttrId = MQTT_CLIENT_ID + String(random(1000, 9999));
  char * buf = (char *) malloc (mqttrId.length());
  mqttrId.toCharArray(buf, mqttrId.length() + 1);

  if (client.connect(buf)) {
    Serial.println("OK");
    client.subscribe(MQTT_SUB_TOPIC);
    free(buf);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    delay(2000);
  }
}

void mqttListerner(char* topic, byte* payload, unsigned int length) {
  if (length == 0) return;

  StaticJsonDocument<256> json;
  deserializeJson(json, payload);

  for (int i = 0; i < json.size(); i++) {
    int id = json[i]["id"];
    int type = json[i]["type"];
    int data = json[i]["data"];

    switch (type) {
      case 1: //act on slave's power state
        if (data) sendCmd(searchAddrById(id), "6");
        else sendCmd(searchAddrById(id), "5");

        break;

      case 2: //change emergency cutdown order

        break;
    }
  }
}

void mqttLoop() {
  if (!checkWiFiConnc()) wifiInit();

  StaticJsonDocument<256> json;
  bool empty = true;
  char buf[256];

  for (int i = 1; i < sys_info.num_modules; i++) {
    JsonArray slaveData = json.createNestedArray(int2str(sys_info.modules[i][0], 4));
    JsonObject data = slaveData.createNestedObject();

    data["state"] = sys_info.modules[i][1];
    data["current"] = sys_info.modules[i][2];

    empty = false;
  }

  if (!empty) {
    serializeJson(json, buf);
    client.publish(MQTT_SUB_TOPIC, buf);
  }
}