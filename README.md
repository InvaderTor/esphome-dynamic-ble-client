# esphome-dynamic-ble-client

Overrides ESPHome BLE client headers to allow setting the target MAC address at runtime.

## Files

- `components/esp32_ble_client/ble_client_base.h` – upstream header plus a small `set_target_address(...)` method
- `components/ble_client/ble_client.h` – upstream header plus a small `set_address(...)` wrapper that safely reconnects

## Use in ESPHome

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/<your-username>/esphome-dynamic-ble-client
    refresh: 0s
  - source: github://syssi/esphome-jk-bms@main
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
            if (id(client0).set_address(mac)) {
              ESP_LOGI("ble", "MAC updated to %s", mac.c_str());
            } else {
              ESP_LOGE("ble", "Invalid MAC: %s", mac.c_str());
            }
        - switch.turn_on: ble_client_switch0
