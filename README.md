# esphome-dynamic-ble-client

ESPHome External BLE Client with set_address_str Support

This repository provides an external component override for ESPHome’s built-in ble_client package, adding a convenience method for setting the BLE address from a string while keeping the rest of the upstream implementation intact.
I was building an ESP32 useing ESPHome for a project where I wanted to give an ESP to a friend who wasnt using home assistant or anything, just wanted to view details though the webserver component.

    set_address_str(const std::string&) method for BLEClient
    Allows passing a human-readable MAC address (e.g. "C8:47:80:16:99:CC") directly, instead of manually parsing into a uint8_t[6].

    Fully functional ble_client and ble_client.switch components preserved by including upstream .cpp source files in the external repo.

    Drop-in replacement — no need to modify ESPHome’s core files.
    The .cpp files are copied verbatim from ESPHome 2025.7.5 to ensure all symbols and vtables are present.
    
## Use in ESPHome

```yaml

substitutions:
#This is just a default initial mac, you might want to change it dynamically in the webpage
bms0_mac_address: C8:47:80:16:99:CC

external_components:
  - source:
      type: git
      url: https://github.com/InvaderTor/esphome-dynamic-ble-client
    refresh: 0s

ble_client:
  - id: client0
    mac_address: ${bms0_mac_address}

text:
  - platform: template
    id: bms_mac_text
    name: "BMS BLE MAC"
    mode: text
    initial_value: "${bms0_mac_address}"
    restore_value: true
    optimistic: true

button: 
  - platform: template
    id: apply_bms_mac
    name: "Apply BMS MAC"
    on_press:
      then:
        - lambda: |-
            const std::string mac = id(bms_mac_text).state;
            if (id(client0).set_address_str(mac)) {
              ESP_LOGI("ble", "MAC updated to %s", mac.c_str());
            } else {
              ESP_LOGE("ble", "Invalid MAC: %s", mac.c_str());
            }
        - switch.turn_on: ble_client_switch0

switch:
  - platform: ble_client
    ble_client_id: client0
    id: ble_client_switch0
    name: "${bms0} enable bluetooth connection"
