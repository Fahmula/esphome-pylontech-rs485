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
  ESP_LOGCONFIG(TAG, "  UART Bus is configured.");
  ESP_LOGCONFIG(TAG, "  Update Timeout: %u ms", this->update_timeout_ms_);
  // Log all configured sensors
  LOG_SENSOR("  ", "SoC Sensor", this->soc_sensor_);
  LOG_SENSOR("  ", "Voltage Sensor", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current Sensor", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature Sensor", this->temperature_sensor_);
  LOG_SENSOR("  ", "SoH Sensor", this->soh_sensor_);
  LOG_SENSOR("  ", "Cycle Count Sensor", this->cycle_count_sensor_);
  LOG_SENSOR("  ", "Max Temperature Sensor", this->max_temperature_sensor_);
  LOG_SENSOR("  ", "Min Temperature Sensor", this->min_temperature_sensor_);
  LOG_SENSOR("  ", "Max Cell Voltage Sensor", this->max_cell_voltage_sensor_);
  LOG_SENSOR("  ", "Min Cell Voltage Sensor", this->min_cell_voltage_sensor_);
  LOG_SENSOR("  ", "MOSFET Temperature Sensor", this->mosfet_temperature_sensor_);
  LOG_SENSOR("  ", "Max MOSFET Temperature Sensor", this->max_mosfet_temperature_sensor_);
  LOG_SENSOR("  ", "Min MOSFET Temperature Sensor", this->min_mosfet_temperature_sensor_);
  LOG_SENSOR("  ", "BMS Temperature Sensor", this->bms_temperature_sensor_);
  LOG_SENSOR("  ", "Max BMS Temperature Sensor", this->max_bms_temperature_sensor_);
  LOG_SENSOR("  ", "Min BMS Temperature Sensor", this->min_bms_temperature_sensor_);
  LOG_SENSOR("  ", "Max Voltage (Limit) Sensor", this->max_voltage_sensor_);
  LOG_SENSOR("  ", "Min Voltage (Limit) Sensor", this->min_voltage_sensor_);
  LOG_SENSOR("  ", "Max Charge Current Sensor", this->max_charge_current_sensor_);
  LOG_SENSOR("  ", "Max Discharge Current Sensor", this->max_discharge_current_sensor_);
  LOG_BINARY_SENSOR("  ", "Total Voltage High Alarm", this->total_voltage_high_alarm_);
  LOG_BINARY_SENSOR("  ", "Total Voltage Low Alarm", this->total_voltage_low_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Voltage High Alarm", this->cell_voltage_high_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Voltage Low Alarm", this->cell_voltage_low_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Temperature High Alarm", this->cell_temp_high_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Temperature Low Alarm", this->cell_temp_low_alarm_);
  LOG_BINARY_SENSOR("  ", "MOSFET Temperature High Alarm", this->mosfet_temp_high_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Imbalance Alarm", this->cell_imbalance_alarm_);
  LOG_BINARY_SENSOR("  ", "Cell Temperature Imbalance Alarm", this->cell_temp_imbalance_alarm_);
  LOG_BINARY_SENSOR("  ", "Charge Overcurrent Alarm", this->charge_overcurrent_alarm_);
  LOG_BINARY_SENSOR("  ", "Discharge Overcurrent Alarm", this->discharge_overcurrent_alarm_);
  LOG_BINARY_SENSOR("  ", "Module Overvoltage Protection", this->module_overvoltage_protection_);
  LOG_BINARY_SENSOR("  ", "Module Undervoltage Protection", this->module_undervoltage_protection_);
  LOG_BINARY_SENSOR("  ", "Cell Overvoltage Protection", this->cell_overvoltage_protection_);
  LOG_BINARY_SENSOR("  ", "Cell Undervoltage Protection", this->cell_undervoltage_protection_);
  LOG_BINARY_SENSOR("  ", "Cell Overtemperature Protection", this->cell_overtemp_protection_);
  LOG_BINARY_SENSOR("  ", "Cell Undertemperature Protection", this->cell_undertemp_protection_);
  LOG_BINARY_SENSOR("  ", "MOSFET Overtemperature Protection", this->mosfet_overtemp_protection_);
  LOG_BINARY_SENSOR("  ", "Charge Overcurrent Protection", this->charge_overcurrent_protection_);
  LOG_BINARY_SENSOR("  ", "Discharge Overcurrent Protection", this->discharge_overcurrent_protection_);
  LOG_BINARY_SENSOR("  ", "System Fault Protection", this->system_fault_protection_);
}

float PylontechRS485::get_setup_priority() const { return setup_priority::LATE; }

void PylontechRS485::loop() {
  if (this->update_state_from_sensors_()) {
    this->last_update_ms_ = millis();
    if (!this->is_data_valid_) {
      ESP_LOGI(TAG, "Valid data received. Starting communication with inverter.");
      this->is_data_valid_ = true;
    }
  }

  if (this->is_data_valid_ && (millis() - this->last_update_ms_ > this->update_timeout_ms_)) {
    ESP_LOGW(TAG, "Sensor data timeout! Halting communication to trigger fail-safe.");
    this->is_data_valid_ = false;
  }

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
  // Check only the most basic sensors to enable communication.
  // The handle_command_61_ function will check each optional sensor individually.
  if (this->soc_sensor_ == nullptr || std::isnan(this->soc_sensor_->state) ||
      this->voltage_sensor_ == nullptr || std::isnan(this->voltage_sensor_->state) ||
      this->current_sensor_ == nullptr || std::isnan(this->current_sensor_->state) ||
      this->temperature_sensor_ == nullptr || std::isnan(this->temperature_sensor_->state)) {
    return false; // Core data is not ready
  }
  
  // Also check sensors for command 63
  if (this->max_voltage_sensor_ == nullptr || std::isnan(this->max_voltage_sensor_->state) ||
      this->min_voltage_sensor_ == nullptr || std::isnan(this->min_voltage_sensor_->state) ||
      this->max_charge_current_sensor_ == nullptr || std::isnan(this->max_charge_current_sensor_->state) ||
      this->max_discharge_current_sensor_ == nullptr || std::isnan(this->max_discharge_current_sensor_->state)) {
    return false; // Limit data is not ready
  }

  // Update core member variables
  this->soc_percent_ = this->soc_sensor_->state;
  this->voltage_mv_ = this->voltage_sensor_->state * 1000;
  this->current_ca_ = this->current_sensor_->state * 100;
  this->temp_deci_k_ = (this->temperature_sensor_->state + 273.15) * 10;
  
  // Update limits from their sensors for command 63
  this->max_charge_v_mv_ = this->max_voltage_sensor_->state * 1000;
  this->min_discharge_v_mv_ = this->min_voltage_sensor_->state * 1000;
  this->max_charge_i_ca_ = this->max_charge_current_sensor_->state * 100;
  this->max_discharge_i_ca_ = this->max_discharge_current_sensor_->state * 100;
  
  return true;
}

void PylontechRS485::route_frame_request_(const std::string &frame_str) {
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
  // --- Create local variables with SAFE DEFAULTS ---
  uint16_t cycles = 100;
  uint8_t  soh = 100;
  uint16_t max_cell_v = 3350;
  uint16_t min_cell_v = 3350;
  uint16_t avg_cell_temp = this->temp_deci_k_;
  uint16_t max_cell_temp = avg_cell_temp;
  uint16_t min_cell_temp = avg_cell_temp;
  uint16_t avg_mosfet_temp = avg_cell_temp;
  uint16_t max_mosfet_temp = avg_cell_temp;
  uint16_t min_mosfet_temp = avg_cell_temp;
  uint16_t avg_bms_temp = avg_cell_temp;
  uint16_t max_bms_temp = avg_cell_temp;
  uint16_t min_bms_temp = avg_cell_temp;

  // --- Overwrite defaults with real data IF the sensors are provided and valid ---
  if (this->cycle_count_sensor_ != nullptr && !std::isnan(this->cycle_count_sensor_->state))
    cycles = this->cycle_count_sensor_->state;
  if (this->soh_sensor_ != nullptr && !std::isnan(this->soh_sensor_->state))
    soh = this->soh_sensor_->state;
  if (this->max_cell_voltage_sensor_ != nullptr && !std::isnan(this->max_cell_voltage_sensor_->state))
    max_cell_v = this->max_cell_voltage_sensor_->state * 1000;
  if (this->min_cell_voltage_sensor_ != nullptr && !std::isnan(this->min_cell_voltage_sensor_->state))
    min_cell_v = this->min_cell_voltage_sensor_->state * 1000;
  if (this->max_temperature_sensor_ != nullptr && !std::isnan(this->max_temperature_sensor_->state))
    max_cell_temp = (this->max_temperature_sensor_->state + 273.15) * 10;
  if (this->min_temperature_sensor_ != nullptr && !std::isnan(this->min_temperature_sensor_->state))
    min_cell_temp = (this->min_temperature_sensor_->state + 273.15) * 10;
  if (this->mosfet_temperature_sensor_ != nullptr && !std::isnan(this->mosfet_temperature_sensor_->state))
    avg_mosfet_temp = (this->mosfet_temperature_sensor_->state + 273.15) * 10;
  if (this->max_mosfet_temperature_sensor_ != nullptr && !std::isnan(this->max_mosfet_temperature_sensor_->state))
    max_mosfet_temp = (this->max_mosfet_temperature_sensor_->state + 273.15) * 10;
  if (this->min_mosfet_temperature_sensor_ != nullptr && !std::isnan(this->min_mosfet_temperature_sensor_->state))
    min_mosfet_temp = (this->min_mosfet_temperature_sensor_->state + 273.15) * 10;
  if (this->bms_temperature_sensor_ != nullptr && !std::isnan(this->bms_temperature_sensor_->state))
    avg_bms_temp = (this->bms_temperature_sensor_->state + 273.15) * 10;
  if (this->max_bms_temperature_sensor_ != nullptr && !std::isnan(this->max_bms_temperature_sensor_->state))
    max_bms_temp = (this->max_bms_temperature_sensor_->state + 273.15) * 10;
  if (this->min_bms_temperature_sensor_ != nullptr && !std::isnan(this->min_bms_temperature_sensor_->state))
    min_bms_temp = (this->min_bms_temperature_sensor_->state + 273.15) * 10;

  // Build the full, structurally-correct payload
  char info_payload[99];
  snprintf(info_payload, sizeof(info_payload),
           "%04X%04X%02X%04X%04X%02X%02X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X%04X",
           this->voltage_mv_, (uint16_t) this->current_ca_, this->soc_percent_, cycles, cycles, soh, soh,
           max_cell_v, 0x0101, min_cell_v, 0x0108,
           avg_cell_temp, max_cell_temp, 0x0103, min_cell_temp, 0x0109,
           avg_mosfet_temp, max_mosfet_temp, 0x0101, min_mosfet_temp, 0x0101,
           avg_bms_temp, max_bms_temp, 0x0101, min_bms_temp, 0x0101);
  
  std::string info_payload_str(info_payload);
  std::string length_field = this->calculate_length_field_(info_payload_str.length());
  std::string frame_data = PROTOCOL_VERSION + RESPONSE_ADDRESS + "4600" + length_field + info_payload_str;
  std::string checksum = this->calculate_checksum_(frame_data);
  std::string full_frame = "~" + frame_data + checksum + "\r";
  this->write_str(full_frame.c_str());
}

void PylontechRS485::handle_command_62_() {

  // Byte 1: System Alarm Status 1
  // Byte 2: System Alarm Status 2
  // Byte 3: System Protection Status 1
  // Byte 4: System Protection Status 2

  // Initialize all status bytes to 0 (normal/no trigger)
  uint8_t alarm_status_1 = 0;
  uint8_t alarm_status_2 = 0;
  uint8_t protection_status_1 = 0;
  uint8_t protection_status_2 = 0;

  // --- Populate Alarm Status 1 ---
  // Check each binary sensor and set the corresponding bit if true.
  if (this->total_voltage_high_alarm_ && this->total_voltage_high_alarm_->state) { alarm_status_1 |= (1 << 7); }
  if (this->total_voltage_low_alarm_ && this->total_voltage_low_alarm_->state) { alarm_status_1 |= (1 << 6); }
  if (this->cell_voltage_high_alarm_ && this->cell_voltage_high_alarm_->state) { alarm_status_1 |= (1 << 5); }
  if (this->cell_voltage_low_alarm_ && this->cell_voltage_low_alarm_->state) { alarm_status_1 |= (1 << 4); }
  if (this->cell_temp_high_alarm_ && this->cell_temp_high_alarm_->state) { alarm_status_1 |= (1 << 3); }
  if (this->cell_temp_low_alarm_ && this->cell_temp_low_alarm_->state) { alarm_status_1 |= (1 << 2); }
  if (this->mosfet_temp_high_alarm_ && this->mosfet_temp_high_alarm_->state) { alarm_status_1 |= (1 << 1); }
  if (this->cell_imbalance_alarm_ && this->cell_imbalance_alarm_->state) { alarm_status_1 |= (1 << 0); }

  // --- Populate Alarm Status 2 ---
  if (this->cell_temp_imbalance_alarm_ && this->cell_temp_imbalance_alarm_->state) { alarm_status_2 |= (1 << 7); }
  if (this->charge_overcurrent_alarm_ && this->charge_overcurrent_alarm_->state) { alarm_status_2 |= (1 << 6); }
  if (this->discharge_overcurrent_alarm_ && this->discharge_overcurrent_alarm_->state) { alarm_status_2 |= (1 << 5); }
  // Bit 4 is 'Internal communication error', we can leave this as 0 for now.

  // --- Populate Protection Status 1 ---
  if (this->module_overvoltage_protection_ && this->module_overvoltage_protection_->state) { protection_status_1 |= (1 << 7); }
  if (this->module_undervoltage_protection_ && this->module_undervoltage_protection_->state) { protection_status_1 |= (1 << 6); }
  if (this->cell_overvoltage_protection_ && this->cell_overvoltage_protection_->state) { protection_status_1 |= (1 << 5); }
  if (this->cell_undervoltage_protection_ && this->cell_undervoltage_protection_->state) { protection_status_1 |= (1 << 4); }
  if (this->cell_overtemp_protection_ && this->cell_overtemp_protection_->state) { protection_status_1 |= (1 << 3); }
  if (this->cell_undertemp_protection_ && this->cell_undertemp_protection_->state) { protection_status_1 |= (1 << 2); }
  if (this->mosfet_overtemp_protection_ && this->mosfet_overtemp_protection_->state) { protection_status_1 |= (1 << 1); }

  // --- Populate Protection Status 2 ---
  if (this->charge_overcurrent_protection_ && this->charge_overcurrent_protection_->state) { protection_status_2 |= (1 << 6); }
  if (this->discharge_overcurrent_protection_ && this->discharge_overcurrent_protection_->state) { protection_status_2 |= (1 << 5); }
  if (this->system_fault_protection_ && this->system_fault_protection_->state) { protection_status_2 |= (1 << 3); }

  // Build the INFO payload string from the four status bytes.
  // The payload is 4 bytes, which is 8 ASCII characters in HEX.
  char info_payload[9];
  snprintf(info_payload, sizeof(info_payload), "%02X%02X%02X%02X", 
           alarm_status_1, alarm_status_2, protection_status_1, protection_status_2);

  std::string info_payload_str(info_payload);
  std::string length_field = this->calculate_length_field_(info_payload_str.length());
  std::string frame_data = PROTOCOL_VERSION + RESPONSE_ADDRESS + "4600" + length_field + info_payload_str;
  std::string checksum = this->calculate_checksum_(frame_data);
  std::string full_frame = "~" + frame_data + checksum + "\r";
  this->write_str(full_frame.c_str());
}

void PylontechRS485::handle_command_63_() {
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

// --- Checksum functions remain the same ---
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