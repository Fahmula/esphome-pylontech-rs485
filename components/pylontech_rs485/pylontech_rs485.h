#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include <string>
#include <vector>

namespace esphome {
namespace pylontech_rs485 {

class PylontechRS485 : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  // Setters to link required sensor components
  void set_soc_sensor(sensor::Sensor *sensor) { this->soc_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }

  // Setters for required limits
  void set_max_charge_voltage(float voltage) { this->max_charge_v_mv_ = voltage * 1000; }
  void set_min_discharge_voltage(float voltage) { this->min_discharge_v_mv_ = voltage * 1000; }
  void set_max_charge_current(float current) { this->max_charge_i_ca_ = current * 100; }
  void set_max_discharge_current(float current) { this->max_discharge_i_ca_ = current * 100; }

  // Setter for the update timeout
  void set_update_timeout(uint32_t timeout_ms) { this->update_timeout_ms_ = timeout_ms; }

 protected:
  // Pointers to the required sensor components
  sensor::Sensor *soc_sensor_;
  sensor::Sensor *voltage_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *temperature_sensor_;

  // Member variables to hold the battery state.
  // Initialized to 0, requires live sensor data to operate.
  uint16_t voltage_mv_{0};
  int16_t current_ca_{0};
  uint8_t soc_percent_{0};
  uint16_t temp_deci_k_{0};

  // Member variables for required battery limits
  uint16_t max_charge_v_mv_{0};
  uint16_t min_discharge_v_mv_{0};
  uint16_t max_charge_i_ca_{0};
  uint16_t max_discharge_i_ca_{0};

  // State-tracking for fail-silent operation
  bool is_data_valid_{false};
  uint32_t last_update_ms_{0};
  uint32_t update_timeout_ms_{60000};

  std::string rx_buffer_;

  // Private helper methods
  bool update_state_from_sensors_();
  void route_frame_request_(const std::string &frame_str);
  void handle_command_61_();
  void handle_command_62_();
  void handle_command_63_();
  std::string calculate_checksum_(const std::string &frame_data);
  std::string calculate_length_field_(int info_len);
};

}  // namespace pylontech_rs485
}  // namespace esphome