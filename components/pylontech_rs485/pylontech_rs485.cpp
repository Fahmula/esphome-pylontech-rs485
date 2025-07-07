#include "pylontech_rs485.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart_component.h"

namespace esphome {
namespace pylontech_rs485 {

static const char *const TAG = "pylontech_rs485";
static const std::string PROTOCOL_VERSION = "20";
static const std::string RESPONSE_ADDRESS = "02";

void PylontechRS485::setup() { ESP_LOGCONFIG(TAG, "Pylontech RS485 component starting."); }

void PylontechRS485::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech RS485:");
  // LOG_UART_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  UART Bus is configured."); // Add a simple replacement log
  ESP_LOGCONFIG(TAG, "  Update Timeout: %u ms", this->update_timeout_ms_);
  LOG_SENSOR("  ", "SoC Sensor", this->soc_sensor_);
  LOG_SENSOR("  ", "Voltage Sensor", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current Sensor", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
  LOG_SENSOR("  ", "Max Voltage Sensor", this->max_voltage_sensor_);
  LOG_SENSOR("  ", "Min Voltage Sensor", this->min_voltage_sensor_);
  LOG_SENSOR("  ", "Max Charge Current Sensor", this->max_charge_current_sensor_);
  LOG_SENSOR("  ", "Max Discharge Current Sensor", this->max_discharge_current_sensor_);
}

float PylontechRS485::get_setup_priority() const { return setup_priority::LATE; }

void PylontechRS485::loop() {
  // 1. Attempt to update state from sensors
  if (this->update_state_from_sensors_()) {
    this->last_update_ms_ = millis();
    if (!this->is_data_valid_) {
      ESP_LOGI(TAG, "Valid data received from all sensors. Starting communication with inverter.");
      this->is_data_valid_ = true;
    }
  }

  // 2. Check for sensor data timeout
  if (this->is_data_valid_ && (millis() - this->last_update_ms_ > this->update_timeout_ms_)) {
    ESP_LOGW(TAG, "Sensor data timeout! Halting communication with inverter to trigger fail-safe.");
    this->is_data_valid_ = false;
  }

  // 3. Process incoming UART data
  while (this->available()) {
    uint8_t byte;
    if (this->read_byte(&byte)) {
      char c = static_cast<char>(byte);
      if (c == '~') {
        this->rx_buffer_.clear();
        this->rx_buffer_ += c;
      } else if (!this->rx_buffer_.empty()) {
        this->rx_buffer_ += c;
        if (c == '\r') {
          ESP_LOGV(TAG, "Received frame: %s", this->rx_buffer_.c_str());
          this->route_frame_request_(this->rx_buffer_);
          this->rx_buffer_.clear();
        }
      }
    }
  }
}

bool PylontechRS485::update_state_from_sensors_() {
  // Check if all required sensors have a valid state (are not NaN)
  if (std::isnan(this->soc_sensor_->state) || std::isnan(this->voltage_sensor_->state) ||
      std::isnan(this->current_sensor_->state) || std::isnan(this->temperature_sensor_->state) ||
      std::isnan(this->max_voltage_sensor_->state) || std::isnan(this->min_voltage_sensor_->state) ||
      std::isnan(this->max_charge_current_sensor_->state) || std::isnan(this->max_discharge_current_sensor_->state)) {
    return false; // At least one sensor is not ready
  }

  // All sensors are valid, update member variables
  // All sensors are valid, update member variables
  this->soc_percent_ = this->soc_sensor_->state;
  this->voltage_mv_ = this->voltage_sensor_->state * 1000;
  this->current_ca_ = this->current_sensor_->state * 100;
  this->temp_deci_k_ = (this->temperature_sensor_->state + 273.15) * 10;
  
  // Update limits from their sensors
  this->max_charge_v_mv_ = this->max_voltage_sensor_->state * 1000;
  this->min_discharge_v_mv_ = this->min_voltage_sensor_->state * 1000;
  this->max_charge_i_ca_ = this->max_charge_current_sensor_->state * 100;
  this->max_discharge_i_ca_ = this->max_discharge_current_sensor_->state * 100;
  
  return true;
}

void PylontechRS485::route_frame_request_(const std::string &frame_str) {
  // FAIL-SAFE: If data is not valid, do not respond to anything. Go silent.
  if (!this->is_data_valid_) {
    ESP_LOGV(TAG, "Ignoring inverter request, data is not valid.");
    return;
  }

  if (frame_str.length() < 18) {
    ESP_LOGW(TAG, "Received frame is too short.");
    return;
  }
  std::string cid2 = frame_str.substr(7, 2);

  if (cid2 == "61") {
    this->handle_command_61_();
  } else if (cid2 == "62") {
    this->handle_command_62_();
  } else if (cid2 == "63") {
    this->handle_command_63_();
  } else {
    ESP_LOGD(TAG, "Ignoring unknown command: %s", cid2.c_str());
  }
}

void PylontechRS485::handle_command_61_() {
  // This function is only called if is_data_valid_ is true
  char info_payload[99];
  snprintf(info_payload, sizeof(info_payload),
           "%04X%04X%02X%04X%04X%02X%02X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X",
           this->voltage_mv_, (uint16_t) this->current_ca_, this->soc_percent_, 100, 100, 99, 99, 3280, 0x0101, 3280,
           0x0108, this->temp_deci_k_, this->temp_deci_k_, 0x0103, this->temp_deci_k_, 0x0109, this->temp_deci_k_,
           this->temp_deci_k_, 0x0101, this->temp_deci_k_, 0x0101, this->temp_deci_k_, this->temp_deci_k_, 0x0101,
           this->temp_deci_k_, 0x0101);
  std::string info_payload_str(info_payload);
  std::string length_field = this->calculate_length_field_(info_payload_str.length());
  std::string frame_data = PROTOCOL_VERSION + RESPONSE_ADDRESS + "4600" + length_field + info_payload_str;
  std::string checksum = this->calculate_checksum_(frame_data);
  std::string full_frame = "~" + frame_data + checksum + "\r";
  this->write_str(full_frame.c_str());
}

void PylontechRS485::handle_command_62_() {
  std::string info_payload = "00000000";
  std::string length_field = this->calculate_length_field_(info_payload.length());
  std::string frame_data = PROTOCOL_VERSION + RESPONSE_ADDRESS + "4600" + length_field + info_payload;
  std::string checksum = this->calculate_checksum_(frame_data);
  std::string full_frame = "~" + frame_data + checksum + "\r";
  this->write_str(full_frame.c_str());
}

void PylontechRS485::handle_command_63_() {
  // This function is only called if is_data_valid_ is true
  char info_payload[19];
  uint8_t status_byte = 0xC0;
  snprintf(info_payload, sizeof(info_payload), "%04X%04X%04X%04X%02X", this->max_charge_v_mv_,
           this->min_discharge_v_mv_, this->max_charge_i_ca_, this->max_discharge_i_ca_, status_byte);
  std::string info_payload_str(info_payload);
  std::string length_field = this->calculate_length_field_(info_payload_str.length());
  std::string frame_data = PROTOCOL_VERSION + RESPONSE_ADDRESS + "4600" + length_field + info_payload_str;
  std::string checksum = this->calculate_checksum_(frame_data);
  std::string full_frame = "~" + frame_data + checksum + "\r";
  this->write_str(full_frame.c_str());
}

std::string PylontechRS485::calculate_checksum_(const std::string &frame_data) {
  uint16_t sum = 0;
  for (char c : frame_data) {
    sum += c;
  }
  sum = ~sum;
  sum += 1;
  char checksum_str[5];
  snprintf(checksum_str, sizeof(checksum_str), "%04X", sum);
  return std::string(checksum_str);
}

std::string PylontechRS485::calculate_length_field_(int info_len) {
  uint8_t n1 = (info_len >> 8) & 0x0F;
  uint8_t n2 = (info_len >> 4) & 0x0F;
  uint8_t n3 = info_len & 0x0F;
  uint8_t sum = n1 + n2 + n3;
  uint8_t lchksum = (~sum & 0x0F) + 1;
  char length_str[5];
  snprintf(length_str, sizeof(length_str), "%1X%03X", lchksum, info_len);
  return std::string(length_str);
}

}  // namespace pylontech_rs485
}  // namespace esphome