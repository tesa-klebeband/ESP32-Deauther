#include <WebServer.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"

WebServer server(80);
int num_networks;

// Move the function declaration to the top
String getEncryptionType(wifi_auth_mode_t encryptionType);
const char* deauth_reasons[][2] = {
    {"0", "Reserved"},
    {"1", "Unspecified reason"},
    {"2", "Previous authentication no longer valid"},
    {"3", "Deauthenticated - leaving network"},
    {"4", "Disassociated - inactivity"},
    {"5", "Disassociated - too many STAs"},
    {"6", "Class 2 frame from nonauth STA"},
    {"7", "Class 3 frame from nonassoc STA"},
    {"8", "Disassociated - STA is leaving"},
    {"9", "STA requesting (re)assoc without auth"},
    {"10", "Disassociated - bad power capability"},
    {"11", "Disassociated - bad supported channels"},
    {"12", "Reserved"},
    {"13", "Invalid information element"},
    {"14", "Message integrity code (MIC) failure"},
    {"15", "4-Way Handshake timeout"},
    {"16", "Group Key Handshake timeout"},
    {"17", "Information element in 4-Way differs"},
    {"18", "Invalid group cipher"},
    {"19", "Invalid pairwise cipher"},
    {"20", "Invalid AKMP"},
    {"21", "Unsupported RSN information element version"},
    {"22", "Invalid RSN information element capabilities"},
    {"23", "IEEE 802.1X authentication failed"},
    {"24", "Cipher suite rejected because of the security policy"}
};

void redirect_root() {
  server.sendHeader("Location", "/");
  server.send(301);
}

void handle_root() {
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-Deauther</title>
    <style>
        * {
            box-sizing: border-box;
        }     
        body {
            font-family: Arial, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f4f4f4;
        }
        h1, h2 {
            color: #2c3e50;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #3498db;
            color: white;
        }
        tr:nth-child(even) {
            background-color: #f2f2f2;
        }
        form {
            background-color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        input[type="text"], input[type="submit"] {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #3498db;
            color: white;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        input[type="submit"]:hover {
            background-color: #2980b9;
        }
        
        select {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <h1>ESP32-Deauther</h1>
    
    <h2>WiFi Networks</h2>
    <table>
        <tr>
            <th>Number</th>
            <th>SSID</th>
            <th>BSSID</th>
            <th>Channel</th>
            <th>RSSI</th>
            <th>Encryption</th>
        </tr>
)";

  for (int i = 0; i < num_networks; i++) {
    String encryption = getEncryptionType(WiFi.encryptionType(i));
    html += "<tr><td>" + String(i) + "</td><td>" + WiFi.SSID(i) + "</td><td>" + WiFi.BSSIDstr(i) + "</td><td>" + 
            String(WiFi.channel(i)) + "</td><td>" + String(WiFi.RSSI(i)) + "</td><td>" + encryption + "</td></tr>";
  }

  html += R"(
    </table>

    <form method="post" action="/rescan">
        <input type="submit" value="Rescan networks">
    </form>

    <form method="post" action="/deauth">
        <h2>Launch Deauth-Attack</h2>
        <input type="text" name="net_num" placeholder="Network Number">
        <select name="reason" placeholder="Reason code">
)";
  for(int i = 0; i < 25; i++){
    html += "<option value=\"" + String(deauth_reasons[i][0]) + "\">";
    html += String(deauth_reasons[i][0]) + " - " + String(deauth_reasons[i][1]);
    html += "</option>";
  }

html += R"(
        </select>
        <input type="submit" value="Launch Attack">
    </form>

    <p>Eliminated stations: )" + String(eliminated_stations) + R"(</p>

    <form method="post" action="/deauth_all">
        <h2>Deauth all Networks</h2>
        <select name="reason" placeholder="Reason code">
)";
  for(int i = 0; i < 25; i++){
    html += "<option value=\"" + String(deauth_reasons[i][0]) + "\">";
    html += String(deauth_reasons[i][0]) + " - " + String(deauth_reasons[i][1]);
    html += "</option>";
  }
  html += R"(
        <input type="submit" value="Deauth All">
    </form>

    <form method="post" action="/stop">
        <input type="submit" value="Stop Deauth-Attack">
    </form>

</body>
</html>
)";

  server.send(200, "text/html", html);
}


void handle_deauth() {
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
        * {
            box-sizing: border-box;
        }
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

  if (wifi_number < num_networks) {
    html += R"(">
        <h2>Starting Deauth-Attack!</h2>
        <p>Deauthenticating network number: )" + String(wifi_number) + R"(</p>
        <p>Reason code: )" + String(reason) + R"(</p>
    </div>)";
    start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
  } else {
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

void handle_deauth_all() {
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
        <p>Reason code: )" + String(reason) + R"(</p>
    </div>
</body>
</html>
  )";

  server.send(200, "text/html", html);
  server.stop();
  start_deauth(0, DEAUTH_TYPE_ALL, reason);
}

void handle_rescan() {
  num_networks = WiFi.scanNetworks();
  redirect_root();
}

void handle_stop() {
  stop_deauth();
  redirect_root();
}

void start_web_interface() {
  server.on("/", handle_root);
  server.on("/deauth", handle_deauth);
  server.on("/deauth_all", handle_deauth_all);
  server.on("/rescan", handle_rescan);
  server.on("/stop", handle_stop);

  server.begin();
}

void web_interface_handle_client() {
  server.handleClient();
}

// The function implementation can stay where it is
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
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
