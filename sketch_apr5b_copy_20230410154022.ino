#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

// Define IP
IPAddress IP(192, 168, 4, 1);
IPAddress NETMASK(255, 255, 255, 0);
IPAddress NETWORK(192, 168, 4, 1);
IPAddress DNS(192, 168, 4, 1);

// Define PIN
#define led D0
#define PIN_SPI_CS D8
SoftwareSerial SerialGPS(5, 4);

// Define Library
TinyGPSPlus gps;
File file, file_config;  // File config

// Define Esp Web Server PORT
ESP8266WebServer server(80);

// Variable Definition
// String id_module;
// String APpassword = "12345678";
// String password = "0001";

// Variable Command Config
String strs[20];
int StringCount = 0;
String waktu, id_module, password, mode_module, ip_set, client_ssid, client_password;

// Variable Time
const int timezone_hours = 3;
const int timezone_minute = 0;

// Variable GPS
float Latitude, Longitude, Altitude, Speed;
int i, j, tahun, bulan, tgl, jam, menit, detik, interval;
String datax, database, DateString, TimeString, LatitudeString, LongitudeString;

// ---------------------------------------------------------
// Program
// ---------------------------------------------------------

//Setting IP static
void changeIP(String ipAddress = "192.168.4.1") {

  IP.fromString(ipAddress);
  NETWORK.fromString(ipAddress);
  DNS.fromString(ipAddress);

  Serial.println("IP ADDRESS: ");
  Serial.println(ipAddress);
  Serial.println("IP: ");
  Serial.println(IP);
  Serial.println("NETWORK: ");
  Serial.println(NETWORK);
  Serial.println("NETMASK: ");
  Serial.println(NETMASK);
  Serial.println("DNS: ");
  Serial.println(DNS);

  Serial.println("IP berhasil diupdate");
}

// Setup
void setup() {
  pinMode(led, OUTPUT);

  Serial.begin(9600);
  SerialGPS.begin(9600);

  // TODO LIST (FALSE): INIT CONFIG
  initConfig();

  // Check Module
  // // Mode 1: Server
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP(id_module);
  // WiFi.config(IP, NETWORK, NETMASK, DNS);

  // server.on("/", handleTugas);
  // server.begin();

  // ledStart();

  // if (!SD.begin(PIN_SPI_CS)) {
  //   Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
  // }

  // mode 2: Client


  printConfig();
}

// Loop
void loop() {
  server.handleClient();

  while (SerialGPS.available() > 0) {
    gps123();
  }

  if (interval == 0) {
    interval = 25000;
  }

  if (LatitudeString != 0) {
    datax = (DateString + " " + TimeString + "," + LatitudeString + "," + LongitudeString + "," + Altitude + "," + Speed + "#");
    i += 1;
    if (i > interval)  // Ambil data Setiap 5 detik
    {
      file = SD.open("test.txt", FILE_WRITE);

      // if the file opened okay, write to it:
      if (file) {
        file.print(datax);

        // close the file:
        file.close();

        Serial.println("done.");
        digitalWrite(led, HIGH);
        delay(100);
        digitalWrite(led, LOW);
      } else {
        Serial.println("error opening test.txt");
      }
      i = 0;
    }
  }
}

//Library GPS
void gps123() {
  if (gps.encode(SerialGPS.read())) {
    if (gps.location.isValid()) {
      setGps();
    }


    if (gps.date.isValid()) {
      setGpsDate();
    }

    if (gps.time.isValid()) {
      setGpsTime();
    }
  }
}
//Value GPS LAT LONG
void setGps() {
  // Set Latitude
  Latitude = gps.location.lat();
  LatitudeString = String(Latitude, 6);

  // Set Longitude
  Longitude = gps.location.lng();
  LongitudeString = String(Longitude, 6);

  // Set Altitude
  Altitude = gps.altitude.meters();
  //AltitudeString = String(Altitude, 3);

  // Set Speed
  Speed = gps.speed.kmph();
}
// Value GPS Tanggal
void setGpsDate() {
  DateString = "";
  tgl = gps.date.day();
  bulan = gps.date.month();
  tahun = gps.date.year();

  if (tgl < 10)
    DateString = '0';
  DateString += String(tahun);

  DateString += "/";

  if (bulan < 10)
    DateString += '0';
  DateString += String(bulan);
  DateString += "/";

  if (tahun < 10)
    DateString += '0';
  DateString += String(tgl);
}
// Value GPS Waktu Jam
void setGpsTime() {
  TimeString = "";
  jam = gps.time.hour() + 5;  //adjust UTC
  menit = gps.time.minute();
  detik = gps.time.second();

  menit = menit + timezone_minute;
  if (menit >= 60) {
    menit = menit - 60;
    jam = jam + 1;
  }

  if (menit < 0) {
    menit = menit + 60;
    jam = jam - 1;
  }

  jam = jam + timezone_hours;
  if (jam >= 24) {
    jam = jam - 24;
  } else if (jam < 0) {
    jam = jam + 24;
  }

  if (jam < 10)
    TimeString = '0';
  TimeString += String(jam);
  TimeString += ":";

  if (menit < 10)
    TimeString += '0';
  TimeString += String(menit);
  TimeString += ":";

  if (detik < 10)
    TimeString += '0';
  TimeString += String(detik);
}
// WEBSITE SERVER
void handleTugas() {
  // Check if there's any request
  if (server.args() > 0) {
    String tugasValue = server.arg("tugas");
    tugasValue = String(tugasValue);

    // Define Command
    String command = password + "set";

    // Print Request
    Serial.println("Request: " + tugasValue);
    Serial.println("Command: " + command);
    Serial.println("Compare: ");
    Serial.println(startsWith(tugasValue.c_str(), (command).c_str()));

    // Config Condition - all condition
    if (startsWith(tugasValue.c_str(), (command).c_str())) {
      String value = changeValueConfig(tugasValue);
      int indeks_action = changeActionConfig(tugasValue);

      Serial.println("VALUE: " + value + ", Indeks action: " + String(indeks_action));

      // Hapus config.txt
      hapus("config.txt");

      // Writting in
      writeConfig(indeks_action, value);

      // Print Berhasil
      success();
    } else if (startsWith(tugasValue.c_str(), (password + "config").c_str())) {
      printConfig();
    } else if (startsWith(tugasValue.c_str(), ("reset"))) {
      resetConfig();
    } else if (tugasValue == (password + "ambil")) {
      download("test.txt");
      success();
      //Serial.println("Tugas Ambil Berhasil");
    } else if (tugasValue == (password + "tampil")) {
      tampil();
    } else if (tugasValue == (password + "cek")) {
      success();
      //Serial.println("Koneksi Berhasil");
    } else if (tugasValue == (password + "hapus")) {
      hapus("test.txt");
      success();
      //Serial.println("Berhasil");
    } else {
      error();
    }
  } else {
    String html_code = "<h1>No Request received!</h1>";
    server.send(200, "text/html", html_code);
  }
}
// Configurasi Alat Melalui Website
void initConfig() {
  Serial.println("Init Config");

  if (SD.begin(10)) {
    Serial.println("SD Card is ready.");
    file = SD.open("config.txt", FILE_READ);

    if (file) {
      Serial.println("config.txt is opened.");
      String buffer = file.readStringUntil('\n');

      while (buffer.length() > 0) {
        int index = buffer.indexOf(',');
        if (index == -1)  // No space found
        {
          strs[StringCount++] = buffer;
          break;
        } else {
          strs[StringCount++] = buffer.substring(0, index);
          buffer = buffer.substring(index + 1);
        }
      }

      Serial.println("STRS: ");
      Serial.println("STRS-00: " + String(strs[0]));
      Serial.println("STRS-01: " + String(strs[1]));
      Serial.println("STRS-02: " + String(strs[2]));
      Serial.println("STRS-03: " + String(strs[3]));
      Serial.println("STRS-04: " + String(strs[4]));
      Serial.println("STRS-05: " + String(strs[5]));
      Serial.println("STRS-06: " + String(strs[6]));

      // Assigning config value to Arduino Variable
      waktu = strs[0];
      interval = waktu.toInt() * 1000;
      id_module = strs[1];
      password = strs[2];
      mode_module = strs[3];
      ip_set = strs[4];
      client_ssid = strs[5];
      client_password = strs[6];

      // SET IP
      changeIP(ip_set);

      changeModeModule(mode_module);

      printConfig();

      Serial.println("Config has been initialized successfully");

      // memset(strs, 0, sizeof(strs));
      file.close();
    } else {
      Serial.println("config.txt is failed to open");
    }
  } else {
    Serial.println("SD Card initialization failed");
  }
}

//Value Config
void printConfig() {
  Serial.println("STRS: ");
  Serial.println("STRS-00: " + waktu);
  Serial.println("STRS-01: " + id_module);
  Serial.println("STRS-02: " + password);
  Serial.println("STRS-03: " + mode_module);
  Serial.println("STRS-04: " + ip_set);
  Serial.println("STRS-05: " + client_ssid);
  Serial.println("STRS-06: " + client_password);


  // waktu = strs[0];
  // Serial.print("Variable waktu :");
  // Serial.println(waktu);
  // interval = waktu.toInt();

  // id_module = strs[1];
  // Serial.print("Variable id_module :");
  // Serial.println(id_module);

  // password = strs[2];
  // Serial.print("Variable password :");
  // Serial.println(password);

  // mode_module = strs[3];
  // Serial.print("Variable mode_module :");
  // Serial.println(mode_module);

  // ip_set = strs[4];
  // Serial.print("Variable ip_set :");
  // Serial.println(ip_set);

  // client_ssid = strs[5];
  // Serial.print("Variable Client client_ssid :");
  // Serial.println(client_ssid);

  // client_password = strs[6];
  // Serial.print("Variable Client client_Password :");
  // Serial.println(client_password);
  String html_code = "<ul><li><span style=\"font-weight: bold;\">Waktu</span>: " + waktu + "</li><li><span style=\"font-weight: bold;\">ID Module</span>: " + id_module + "</li><li><span style=\"font-weight: bold;\">Password</span>: " + password + "</li><li><span style=\"font-weight: bold;\">Mode Module</span>: " + mode_module + "</li><li><span style=\"font-weight: bold;\">IP Address</span>: " + ip_set + "</li><li><span style=\"font-weight: bold;\">Client SSID</span>: " + client_ssid + "</li><li><span style=\"font-weight: bold;\">Client Password</span>: " + client_password + "</li></ul>";
  server.send(200, "text/html", html_code);
}

// Menulis Value Config ke dalam file config
void writeConfig(int indeks, String value) {
  bool status_mode_module;
  // Serial.println("masuk writeConfig, Indeks:" + String(indeks) +", value:" + String(value));
  file = SD.open("config.txt", FILE_READ);

  if (file) {
    String buffer = file.readStringUntil('\n');

    // Read Element
    while (buffer.length() > 0) {
      Serial.println("Reading Buffer");

      int index = buffer.indexOf(',');
      if (index == -1)  // No space found
      {
        strs[StringCount++] = buffer;
        break;
      } else {
        strs[StringCount++] = buffer.substring(0, index);
        buffer = buffer.substring(index + 1);
      }
    }

    Serial.println("Onto Switch" + indeks + value);
    // Swith on which side to change the value into
    switch (indeks) {
      case 1:
        strs[0] = value;
        waktu = strs[0];
        interval = value.toInt() * 1000;
        break;
      case 2:
        strs[1] = value;
        id_module = strs[1];
        // WiFi.softAP(id_module);
        break;
      case 3:
        strs[2] = value;
        password = strs[2];
        break;
      case 4:
        strs[3] = value;
        if (mode_module != strs[3]) {
          status_mode_module = true;
        }
        mode_module = strs[3];
        break;
      case 5:
        strs[4] = value;
        // changeIP(strs[4]);
        break;
      case 6:
        strs[5] = value;
        client_ssid = strs[5];
        break;
      case 7:
        strs[6] = value;
        client_password = strs[6];
        break;
      default:
        break;
    }

    Serial.println("STRS:" + String(strs[0]) + String(strs[1]) + String(strs[2]) + String(strs[3]) + String(strs[4]) + String(strs[5]) + String(strs[6]));

    // Make the result back into a single string
    String resultString = "";
    for (int i = 0; i < 7; i++) {
      resultString += strs[i];
      Serial.println(String(i) + "  " + strs[i]);
      if (i != 6) {
        resultString += ",";
      }
    }

    // Writing the result String into the file all over again
    Serial.println("result:   " + resultString);

    if (status_mode_module) {
      changeModeModule(strs[3]);
      status_mode_module = false;
    }

    file.close();
    // Write File
    writeTxt("config.txt", resultString);
  }
}
// Menulis value Config ke dalam file config
void writeTxt(String fileName, String value) {
  file = SD.open("config.txt", FILE_WRITE);

  file.println(value);

  file.close();
}

// Menampilkan Isi file GPS ke Website
void tampil() {
  file = SD.open("test.txt", FILE_READ);
  if (file) {
    server.sendHeader("Content-Type", "text/plain");
    server.streamFile(file, "text/plain");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}
// Website Download file GPS
void download(String filename) {
  file = SD.open(filename, FILE_READ);
  if (file) {
    server.sendHeader("Content-Type", "text/text");
    server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
    server.sendHeader("Connection", "close");
    server.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}
// Website untuk menghapus file GPS dan membuat file baru
void hapus(String filename) {
  SD.remove(filename);
  // Serial.println("menghapus file");
  file = SD.open(filename, FILE_WRITE);
  file.close();
}

// StartWith -> Mengecek apabila string dimulai dengan string tertentu
bool startsWith(const char* str, const char* prefix) {
  size_t prefixLen = strlen(prefix);
  return strncmp(str, prefix, prefixLen) == 0;
}

int changeActionConfig(String request) {
  char indeks_char = (request[request.indexOf("set") + 3]);  //char on index
  String indeks_string = String(indeks_char);                //string on index
  int indeks_action = indeks_string.toInt();                 //string on index

  return indeks_action;
}

String changeValueConfig(String request) {
  // Searching for Value
  int indeks_mulai = request.indexOf("set") + 4;             // +2 karena command indeks ada di +1
  int indeks_akhir = request.length();                       // panjang string
  char indeks_char = (request[request.indexOf("set") + 3]);  //char on index
  String indeks_string = String(indeks_char);                //string on index
  int indeks_action = indeks_string.toInt();                 //string on index
  String value;

  for (int i = indeks_mulai; i < indeks_akhir; i++) {
    value += request[i];
  }

  return value;
}

// Mereset Config
void resetConfig() {
  // Hapus config.txt
  hapus("config.txt");

  // Writting in
  file = SD.open("config.txt", FILE_WRITE);

  String reset_value = "1,STTY01,0001,1,192.168.4.1,user,12345678";

  file.println(reset_value);

  file.close();

  // Print Berhasil
  success();
}

void success() {
  String html_code = "<h1>berhasil</h1>";
  server.send(200, "text/html", html_code);
}

void error() {
  String html_code = "Code gagal - salah password / URL";
  server.send(404, "text/html", html_code);
}
// Menyalakan LED
void ledStart() {
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(500);
  digitalWrite(led, HIGH);
  delay(1000);
  digitalWrite(led, LOW);
  delay(500);
}
// Merubah Mode WIFI AP dan STA
void changeModeModule(String module_type) {
  // WiFi.mode(WIFI_OFF);
  Serial.println("Testing Mode Module");

  Serial.println("Module Type: ");
  Serial.println(module_type);
  Serial.println("WIFI GETMODE: ");
  Serial.println(WiFi.getMode());

  if ((module_type == "1" && (WiFi.getMode() != 2)) || WiFi.getMode() == 3) {
    Serial.println("Starting Mode Module: Server");

    // Mode 1: Server
    WiFi.mode(WIFI_AP);
    Serial.println("Starting Mode Module: Mode has been set");

    WiFi.disconnect();
    WiFi.softAP(id_module);
    Serial.println("Starting Mode Module: SoftAP has been set");

    WiFi.config(IP, NETWORK, NETMASK, DNS);
    Serial.println("Starting Mode Module: config has been set");

    server.on("/", handleTugas);
    server.begin();

    ledStart();

    if (!SD.begin(PIN_SPI_CS)) {
      Serial.println(F("SD CARD FAILED, OR NOT PRESENT!"));
    }
  } else if (module_type == "2" && (WiFi.getMode() != 1)) {
    Serial.println("Starting Mode Module: Client");
    WiFi.begin(client_ssid, client_password);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi.");
  }
}