#pragma once

#include "esphome/components/esp32_ble_client/ble_client_base.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

#ifdef USE_ESP32

#include <esp_bt_defs.h>
#include <esp_gap_ble_api.h>
#include <esp_gatt_common_api.h>
#include <esp_gattc_api.h>
#include <array>
#include <string>
#include <vector>
#include <cstring>

namespace esphome {
namespace ble_client {

namespace espbt = esphome::esp32_ble_tracker;

using namespace esp32_ble_client;

class BLEClient;

class BLEClientNode {
 public:
  virtual void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                   esp_ble_gattc_cb_param_t *param){};
  virtual void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {}
  virtual void loop() {}
  void set_address(uint64_t address) { address_ = address; }
  espbt::ESPBTClient *client;
  // This should be transitioned to Established once the node no longer needs
  // the services/descriptors/characteristics of the parent client. This will
  // allow some memory to be freed.
  espbt::ClientState node_state;

  BLEClient *parent() { return this->parent_; }
  void set_ble_client_parent(BLEClient *parent) { this->parent_ = parent; }

 protected:
  BLEClient *parent_;
  uint64_t address_;
};

class BLEClient : public BLEClientBase {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

  bool gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;

  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
  bool parse_device(const espbt::ESPBTDevice &device) override;

  void set_enabled(bool enabled);

  void register_ble_node(BLEClientNode *node) {
    node->client = this;
    node->set_ble_client_parent(this);
    this->nodes_.push_back(node);
  }

  bool enabled;

  void set_state(espbt::ClientState state) override;

 // This is the patch start
// --- Added: runtime MAC change from string without clashing with core ---
bool set_address_str(const std::string &addr_str) {
  // Parse "AA:BB:CC:DD:EE:FF" into 6 bytes
  uint8_t mac[6] = {0};
  int bi = 0, nyb = 0;
  uint8_t acc = 0;
  for (char c : addr_str) {
    if (c == ':' || c == '-') {
      if (nyb == 2 && bi < 6) { mac[bi++] = acc; acc = 0; nyb = 0; }
      else if (nyb != 0) return false;
      continue;
    }
    uint8_t v;
    if      (c >= '0' && c <= '9') v = c - '0';
    else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
    else return false;
    acc = (acc << 4) | v;
    if (++nyb == 2) {
      if (bi >= 6) return false;
      mac[bi++] = acc; acc = 0; nyb = 0;
    }
  }
  if (bi != 6 || nyb != 0) return false;

  bool was_enabled = this->enabled;
  this->set_enabled(false);                 // cleanly disconnect

  // These member names come from the upstream base; adjust only if yours differ
  std::memcpy(this->remote_bda_, mac, sizeof(mac));
  this->address_str_ = addr_str;

  this->set_enabled(was_enabled);           // reconnect if it was enabled
  return true;
}

// This is the patch end

 protected:
  bool all_nodes_established_();

  std::vector<BLEClientNode *> nodes_;
};

}  // namespace ble_client
}  // namespace esphome

#endif
