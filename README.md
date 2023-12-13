# ESP32-Deauther
A project for the ESP32 that allows you to deauthenticate stations connected to WiFi network
# DISCLAIMER
This tool has been made for educational and testing purposes only. Any misuse or illegal activities conducted with the tool are strictly prohibited. I am **not** responsible for any consequences arising from the use of the tool, which is done at your own risk.
## Building
Clone this repo using:

`git clone https://github.com/tesa-klebeband/ESP32-Deauther`

1) Install vscode and PlatformIO
2) Install the PlatformIO extension in vscode
3) Open the cloned folder in vscode and press the upload button

## Using ESP32-Deauther
The ESP32 hosts a WiFi network with the name of `ESP32-Deauther` and a password of `esp32wroom32`. Connect to this network and type the IP of your ESP32 (typically **192.168.4.1**) into a webbrowser of a choice. You will see the following options:
* Rescan networks: Rescan and detect all WiFi networks in your area. After a successful scan, the networks are listed in the above table.
* Launch Deauth-Attack: Deauthenticates all clients connected to a network. Enter the network number from the table at the top and a reason code from the table at the bottom of the page. After that click the button and your ESP32's LED will flash when it deauthenticates a station.
* Deauth all networks: Launches a Deauth-Attack on all networks and stations with a specific reason. In order to stop this, you have to reset your ESP32 (no other way to code this since the ESP32 rapidly changes through all network channels and has to disable its AP)
* Stop Deauth-Attack: Stops an ongoing deauthentication attack
## License
All files within this repo are released under the GNU GPL V3 License as per the LICENSE file stored in the root of this repo.
