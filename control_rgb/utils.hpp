void connectServer() {
  Serial.println();
  Serial.println();
  Serial.print("Conectandose a la red: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexion establecida");

  Serial.println("IP del servicio: ");
  Serial.println(WiFi.localIP());
}

void print(String text, String param) {
  Serial.print(text);
  Serial.println(param);
}

int len(JsonArray jsonArray) {
  int count = 0;

  for (JsonVariant item: jsonArray) {
    count += 1;
  }

  return count;
}

int getColor(int color) {
  return 255 - color;
}