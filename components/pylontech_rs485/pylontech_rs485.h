#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace pylontech_rs485 {

class PylontechRS485 : public Component, public uart::UARTDevice {
 public:
  // --- Standard ESPHome component functions ---
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void loop() override;

  // --- Setter for component-specific settings ---
  void set_update_timeout(uint32_t timeout) { this->update_timeout_ms_ = timeout; }

  // --- Setters for ALL the sensors defined in the Python schema ---
  void set_soc_sensor(sensor::Sensor *sensor) { this->soc_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }
  void set_soh_sensor(sensor::Sensor *sensor) { this->soh_sensor_ = sensor; }
  void set_cycle_count_sensor(sensor::Sensor *sensor) { this->cycle_count_sensor_ = sensor; }
  void set_max_temperature_sensor(sensor::Sensor *sensor) { this->max_temperature_sensor_ = sensor; }
  void set_min_temperature_sensor(sensor::Sensor *sensor) { this->min_temperature_sensor_ = sensor; }
  void set_max_cell_voltage_sensor(sensor::Sensor *sensor) { this->max_cell_voltage_sensor_ = sensor; }
  void set_min_cell_voltage_sensor(sensor::Sensor *sensor) { this->min_cell_voltage_sensor_ = sensor; }
  void set_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->mosfet_temperature_sensor_ = sensor; }
  void set_max_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->max_mosfet_temperature_sensor_ = sensor; }
  void set_min_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->min_mosfet_temperature_sensor_ = sensor; }
  void set_bms_temperature_sensor(sensor::Sensor *sensor) { this->bms_temperature_sensor_ = sensor; }
  void set_max_bms_temperature_sensor(sensor::Sensor *sensor) { this->max_bms_temperature_sensor_ = sensor; }
  void set_min_bms_temperature_sensor(sensor::Sensor *sensor) { this->min_bms_temperature_sensor_ = sensor; }

  // --- Setters for Binary Sensors ---
  void set_total_voltage_high_alarm(binary_sensor::BinarySensor *sensor) { this->total_voltage_high_alarm_ = sensor; }
  void set_total_voltage_low_alarm(binary_sensor::BinarySensor *sensor) { this->total_voltage_low_alarm_ = sensor; }
  void set_cell_voltage_high_alarm(binary_sensor::BinarySensor *sensor) { this->cell_voltage_high_alarm_ = sensor; }
  void set_cell_voltage_low_alarm(binary_sensor::BinarySensor *sensor) { this->cell_voltage_low_alarm_ = sensor; }
  void set_cell_temp_high_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_high_alarm_ = sensor; }
  void set_cell_temp_low_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_low_alarm_ = sensor; }
  void set_mosfet_temp_high_alarm(binary_sensor::BinarySensor *sensor) { this->mosfet_temp_high_alarm_ = sensor; }
  void set_cell_imbalance_alarm(binary_sensor::BinarySensor *sensor) { this->cell_imbalance_alarm_ = sensor; }
  void set_cell_temp_imbalance_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_imbalance_alarm_ = sensor; }
  void set_charge_overcurrent_alarm(binary_sensor::BinarySensor *sensor) { this->charge_overcurrent_alarm_ = sensor; }
  void set_discharge_overcurrent_alarm(binary_sensor::BinarySensor *sensor) { this->discharge_overcurrent_alarm_ = sensor; }
  void set_module_overvoltage_protection(binary_sensor::BinarySensor *sensor) { this->module_overvoltage_protection_ = sensor; }
  void set_module_undervoltage_protection(binary_sensor::BinarySensor *sensor) { this->module_undervoltage_protection_ = sensor; }
  void set_cell_overvoltage_protection(binary_sensor::BinarySensor *sensor) { this->cell_overvoltage_protection_ = sensor; }
  void set_cell_undervoltage_protection(binary_sensor::BinarySensor *sensor) { this->cell_undervoltage_protection_ = sensor; }
  void set_cell_overtemp_protection(binary_sensor::BinarySensor *sensor) { this->cell_overtemp_protection_ = sensor; }
  void set_cell_undertemp_protection(binary_sensor::BinarySensor *sensor) { this->cell_undertemp_protection_ = sensor; }
  void set_mosfet_overtemp_protection(binary_sensor::BinarySensor *sensor) { this->mosfet_overtemp_protection_ = sensor; }
  void set_charge_overcurrent_protection(binary_sensor::BinarySensor *sensor) { this->charge_overcurrent_protection_ = sensor; }
  void set_discharge_overcurrent_protection(binary_sensor::BinarySensor *sensor) { this->discharge_overcurrent_protection_ = sensor; }
  void set_system_fault_protection(binary_sensor::BinarySensor *sensor) { this->system_fault_protection_ = sensor; }
  
  // Setters for dynamic limits
  void set_max_voltage_sensor(sensor::Sensor *sensor) { this->max_voltage_sensor_ = sensor; }
  void set_min_voltage_sensor(sensor::Sensor *sensor) { this->min_voltage_sensor_ = sensor; }
  void set_max_charge_current_sensor(sensor::Sensor *sensor) { this->max_charge_current_sensor_ = sensor; }
  void set_max_discharge_current_sensor(sensor::Sensor *sensor) { this->max_discharge_current_sensor_ = sensor; }

 protected:
  // --- Helper and state functions ---
  bool update_state_from_sensors_();
  void route_frame_request_(const std::string &frame_str);
  void handle_command_61_();
  void handle_command_62_();
  void handle_command_63_();
  std::string calculate_checksum_(const std::string &frame_data);
  std::string calculate_length_field_(int info_len);

  // --- Member variables for state ---
  uint32_t last_update_ms_{0};
  uint32_t update_timeout_ms_{60000};
  bool is_data_valid_{false};
  std::string rx_buffer_;

  // --- Member variables for dynamic data ---
  uint16_t soc_percent_{0};
  uint16_t voltage_mv_{0};
  int16_t  current_ca_{0}; // current can be negative
  uint16_t temp_deci_k_{0};
  uint16_t max_charge_v_mv_{0};
  uint16_t min_discharge_v_mv_{0};
  uint16_t max_charge_i_ca_{0};
  uint16_t max_discharge_i_ca_{0};

  // --- Pointers to all the sensors from the YAML config ---
  // Initialize them all to nullptr to be safe.
  sensor::Sensor *soc_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *soh_sensor_{nullptr};
  sensor::Sensor *cycle_count_sensor_{nullptr};
  sensor::Sensor *max_temperature_sensor_{nullptr};
  sensor::Sensor *min_temperature_sensor_{nullptr};
  sensor::Sensor *max_cell_voltage_sensor_{nullptr};
  sensor::Sensor *min_cell_voltage_sensor_{nullptr};
  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *max_mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *min_mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *bms_temperature_sensor_{nullptr};
  sensor::Sensor *max_bms_temperature_sensor_{nullptr};
  sensor::Sensor *min_bms_temperature_sensor_{nullptr};

  // --- Pointers for Binary Sensors ---
  binary_sensor::BinarySensor *total_voltage_high_alarm_{nullptr};
  binary_sensor::BinarySensor *total_voltage_low_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_voltage_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_voltage_low_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_low_alarm_{nullptr};
  binary_sensor::BinarySensor *mosfet_temp_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_imbalance_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_imbalance_alarm_{nullptr};
  binary_sensor::BinarySensor *charge_overcurrent_alarm_{nullptr};
  binary_sensor::BinarySensor *discharge_overcurrent_alarm_{nullptr};
  binary_sensor::BinarySensor *module_overvoltage_protection_{nullptr};
  binary_sensor::BinarySensor *module_undervoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_overvoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_undervoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_overtemp_protection_{nullptr};
  binary_sensor::BinarySensor *cell_undertemp_protection_{nullptr};
  binary_sensor::BinarySensor *mosfet_overtemp_protection_{nullptr};
  binary_sensor::BinarySensor *charge_overcurrent_protection_{nullptr};
  binary_sensor::BinarySensor *discharge_overcurrent_protection_{nullptr};
  binary_sensor::BinarySensor *system_fault_protection_{nullptr};

  // --- Pointers for dynamic limit sensors ---
  sensor::Sensor *max_voltage_sensor_{nullptr};
  sensor::Sensor *min_voltage_sensor_{nullptr};
  sensor::Sensor *max_charge_current_sensor_{nullptr};
  sensor::Sensor *max_discharge_current_sensor_{nullptr};
};

}  // namespace pylontech_rs485
}  // namespace esphome