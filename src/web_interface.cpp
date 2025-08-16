#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"

WebServer server(80);
int num_networks;

// Move the function declaration to the top
String getEncryptionType(wifi_auth_mode_t encryptionType);

void redirect_root()
{
  server.sendHeader("Location", "/");
  server.send(301);
}

void handle_root()
{
  String html;
    File file = SPIFFS.open("/index.html", "r"); // Path is correct for SPIFFS, as files in 'data/' are uploaded to SPIFFS root
  if (!file) {
    html = "<h1>index.html not found on SPIFFS!</h1>";
  } else {
    while (file.available()) {
      html += (char)file.read();
    }
    file.close();
    // Optionally, you can still do dynamic row replacement here if needed
  }
  server.send(200, "text/html", html);
}

void handle_deauth()
{
  int wifi_number = server.arg("net_num").toInt();
  uint16_t reason = server.arg("reason").toInt();

  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Deauth Attack</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f0f0;
        }
        .alert {
            background-color: #4CAF50;
            color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
            text-align: center;
        }
        .alert.error {
            background-color: #f44336;
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin-top: 20px;
            background-color: #008CBA;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: #005f73;
        }
    </style>
</head>
<body>
    <div class="alert)";

  if (wifi_number < num_networks)
  {
    html += R"(">
        <h2>Starting Deauth-Attack!</h2>
        <p>Deauthenticating network number: )" +
            String(wifi_number) + R"(</p>
        <p>Reason code: )" +
            String(reason) + R"(</p>
    </div>)";
    start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
  }
  else
  {
    html += R"( error">
        <h2>Error: Invalid Network Number</h2>
        <p>Please select a valid network number.</p>
    </div>)";
  }

  html += R"(
    <a href="/" class="button">Back to Home</a>
</body>
</html>
  )";

  server.send(200, "text/html", html);
}

void handle_deauth_all()
{
  uint16_t reason = server.arg("reason").toInt();

  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Deauth All Networks</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f0f0;
        }
        .alert {
            background-color: #ff9800;
            color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
            text-align: center;
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin-top: 20px;
            background-color: #008CBA;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: #005f73;
        }
    </style>
</head>
<body>
    <div class="alert">
        <h2>Starting Deauth-Attack on All Networks!</h2>
        <p>WiFi will shut down now. To stop the attack, please reset the ESP32.</p>
        <p>Reason code: )" +
                String(reason) + R"(</p>
    </div>
</body>
</html>
  )";

  server.send(200, "text/html", html);
  server.stop();
  start_deauth(0, DEAUTH_TYPE_ALL, reason);
}

void handle_rescan()
{
  num_networks = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < num_networks; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"number\":" + String(i+1) + ",";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"bssid\":\"" + WiFi.BSSIDstr(i) + "\",";
    json += "\"channel\":" + String(WiFi.channel(i)) + ",";
    json += "\"rssi\":\"" + String(WiFi.RSSI(i)) + " dBm\",";
    json += "\"encryption\":\"" + getEncryptionType(WiFi.encryptionType(i)) + "\"";
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handle_stop()
{
  stop_deauth();
  redirect_root();
}

void start_web_interface()
{
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  server.on("/", handle_root);
  server.on("/deauth", handle_deauth);
  server.on("/deauth_all", handle_deauth_all);
  server.on("/rescan", handle_rescan);
  server.on("/stop", handle_stop);
  server.begin();
}

void web_interface_handle_client()
{
  server.handleClient();
}

// The function implementation can stay where it is
String getEncryptionType(wifi_auth_mode_t encryptionType)
{
  switch (encryptionType)
  {
  case WIFI_AUTH_OPEN:
    return "Open";
  case WIFI_AUTH_WEP:
    return "WEP";
  case WIFI_AUTH_WPA_PSK:
    return "WPA_PSK";
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2_PSK";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA_WPA2_PSK";
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2_ENTERPRISE";
  default:
    return "UNKNOWN";
  }
}
