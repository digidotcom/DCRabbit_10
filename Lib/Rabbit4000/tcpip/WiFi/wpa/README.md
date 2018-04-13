This directory contains the libraries used for accessing WPA (Wi-Fi
Protected Access) and WPA2 networks.

It's split into three groups of files:

- `tcpip/WiFi/wpa`: Libraries common to both WPA configurations and
  used when `WIFI_USE_WPA` is defined.

- `tcpip/WiFi/wpa/enterprise`: Libraries based on wpa_supplicant 0.5.x
  and used when `WPA_USE_EAP` is defined.  Required to connect
  to WPA Enterprise networks (those that use a RADIUS server for
  authentication and a username/password to connect instead of a
  common password).

- `tcpip/WiFi/wpa/personal`: Libraries based on wpa_supplicant 0.2.x
  and used when `WPA_USE_EAP` isn't defined.  Uses less code space
  than the EAP-enabled version, but only supports WPA-PSK (pre-shared
  key) mode.

