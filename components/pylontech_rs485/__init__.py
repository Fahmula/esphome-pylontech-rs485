import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, binary_sensor
from esphome.const import CONF_ID

pylontech_rs485_ns = cg.esphome_ns.namespace("esphome::pylontech_rs485")
PylontechRS485 = pylontech_rs485_ns.class_("PylontechRS485", cg.Component, uart.UARTDevice)

# --- Sensor Configuration Constants ---
# --- Primary dynamic values ---
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"
CONF_TEMPERATURE = "temperature"

# --- Charge/Discharge Limits ---
CONF_MAX_CHARGE_CURRENT = "max_charge_current"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_MAX_VOLTAGE = "max_voltage"
CONF_MIN_VOLTAGE = "min_voltage"

# --- General Health values ---
CONF_SOH = "state_of_health"
CONF_CYCLE_COUNT = "cycle_count"

# --- Cell-level values ---
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_TEMPERATURE = "max_temperature"
CONF_MIN_TEMPERATURE = "min_temperature"

# --- MOSFET temperature values ---
CONF_MOSFET_TEMPERATURE = "mosfet_temperature"
CONF_MAX_MOSFET_TEMPERATURE = "max_mosfet_temperature"
CONF_MIN_MOSFET_TEMPERATURE = "min_mosfet_temperature"

# --- BMS/Environment temperature values ---
CONF_BMS_TEMPERATURE = "bms_temperature"
CONF_MAX_BMS_TEMPERATURE = "max_bms_temperature"
CONF_MIN_BMS_TEMPERATURE = "min_bms_temperature"

# --- Component-specific settings ---
CONF_UPDATE_TIMEOUT = "update_timeout"

# --- Binary Sensor Configuration Constants ---
CONF_TOTAL_VOLTAGE_HIGH_ALARM = "total_voltage_high_alarm"
CONF_TOTAL_VOLTAGE_LOW_ALARM = "total_voltage_low_alarm"
CONF_CELL_VOLTAGE_HIGH_ALARM = "cell_voltage_high_alarm"
CONF_CELL_VOLTAGE_LOW_ALARM = "cell_voltage_low_alarm"
CONF_CELL_TEMP_HIGH_ALARM = "cell_temp_high_alarm"
CONF_CELL_TEMP_LOW_ALARM = "cell_temp_low_alarm"
CONF_MOSFET_TEMP_HIGH_ALARM = "mosfet_temp_high_alarm"
CONF_CELL_TEMP_IMBALANCE_ALARM = "cell_temp_imbalance_alarm"
CONF_CELL_IMBALANCE_ALARM = "cell_imbalance_alarm"
CONF_CHARGE_OVERCURRENT_ALARM = "charge_overcurrent_alarm"
CONF_DISCHARGE_OVERCURRENT_ALARM = "discharge_overcurrent_alarm"
CONF_MODULE_OVERVOLTAGE_PROTECTION = "module_overvoltage_protection"
CONF_MODULE_UNDERVOLTAGE_PROTECTION = "module_undervoltage_protection"
CONF_CELL_OVERVOLTAGE_PROTECTION = "cell_overvoltage_protection"
CONF_CELL_UNDERVOLTAGE_PROTECTION = "cell_undervoltage_protection"
CONF_CELL_OVERTEMP_PROTECTION = "cell_overtemp_protection"
CONF_CELL_UNDERTEMP_PROTECTION = "cell_undertemp_protection"
CONF_MOSFET_OVERTEMP_PROTECTION = "mosfet_overtemp_protection"
CONF_CHARGE_OVERCURRENT_PROTECTION = "charge_overcurrent_protection"
CONF_DISCHARGE_OVERCURRENT_PROTECTION = "discharge_overcurrent_protection"
CONF_SYSTEM_FAULT_PROTECTION = "system_fault_protection"

# --- Sensor Schema ---
SENSOR_KEYS_SCHEMA = cv.Schema(
    {
        # --- All sensors OPTIONAL to allow for progressive setup ---
        cv.Optional(CONF_STATE_OF_CHARGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_SOH): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CYCLE_COUNT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_CELL_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_CELL_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),

        # --- Binary sensors for alarms and protections ---
        cv.Optional(CONF_TOTAL_VOLTAGE_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_TOTAL_VOLTAGE_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_VOLTAGE_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_VOLTAGE_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MOSFET_TEMP_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_IMBALANCE_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_IMBALANCE_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CHARGE_OVERCURRENT_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_DISCHARGE_OVERCURRENT_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MODULE_OVERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MODULE_UNDERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_OVERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_UNDERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_OVERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_UNDERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MOSFET_OVERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CHARGE_OVERCURRENT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_DISCHARGE_OVERCURRENT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_SYSTEM_FAULT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),

        # --- Dynamic limit sensors ---
        cv.Optional(CONF_MAX_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_CHARGE_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_DISCHARGE_CURRENT): cv.use_id(sensor.Sensor),

        cv.Optional(CONF_UPDATE_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
    }
)

# --- Main Configuration Schema ---
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechRS485),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(SENSOR_KEYS_SCHEMA)
)


# --- Main Code Generation Function ---
async def to_code(config):
    # --- Standard component setup ---
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # --- Sensor setup logic ---
    # This maps the YAML key to the C++ setter function name.
    SENSOR_MAP = {
        CONF_STATE_OF_CHARGE: "set_soc_sensor",
        CONF_VOLTAGE: "set_voltage_sensor",
        CONF_CURRENT: "set_current_sensor",
        CONF_TEMPERATURE: "set_temperature_sensor",
        CONF_SOH: "set_soh_sensor",
        CONF_CYCLE_COUNT: "set_cycle_count_sensor",
        CONF_MAX_TEMPERATURE: "set_max_temperature_sensor",
        CONF_MIN_TEMPERATURE: "set_min_temperature_sensor",
        CONF_MAX_CELL_VOLTAGE: "set_max_cell_voltage_sensor",
        CONF_MIN_CELL_VOLTAGE: "set_min_cell_voltage_sensor",
        CONF_MOSFET_TEMPERATURE: "set_mosfet_temperature_sensor",
        CONF_MAX_MOSFET_TEMPERATURE: "set_max_mosfet_temperature_sensor",
        CONF_MIN_MOSFET_TEMPERATURE: "set_min_mosfet_temperature_sensor",
        CONF_BMS_TEMPERATURE: "set_bms_temperature_sensor",
        CONF_MAX_BMS_TEMPERATURE: "set_max_bms_temperature_sensor",
        CONF_MIN_BMS_TEMPERATURE: "set_min_bms_temperature_sensor",
        CONF_MAX_VOLTAGE: "set_max_voltage_sensor",
        CONF_MIN_VOLTAGE: "set_min_voltage_sensor",
        CONF_MAX_CHARGE_CURRENT: "set_max_charge_current_sensor",
        CONF_MAX_DISCHARGE_CURRENT: "set_max_discharge_current_sensor",
    }

    # This maps the YAML key to the C++ setter function name for BINARY sensors.
    BINARY_SENSOR_MAP = {
        # Alarm Status 1
        CONF_TOTAL_VOLTAGE_HIGH_ALARM: "set_total_voltage_high_alarm",
        CONF_TOTAL_VOLTAGE_LOW_ALARM: "set_total_voltage_low_alarm",
        CONF_CELL_VOLTAGE_HIGH_ALARM: "set_cell_voltage_high_alarm",
        CONF_CELL_VOLTAGE_LOW_ALARM: "set_cell_voltage_low_alarm",
        CONF_CELL_TEMP_HIGH_ALARM: "set_cell_temp_high_alarm",
        CONF_CELL_TEMP_LOW_ALARM: "set_cell_temp_low_alarm",
        CONF_MOSFET_TEMP_HIGH_ALARM: "set_mosfet_temp_high_alarm",
        CONF_CELL_IMBALANCE_ALARM: "set_cell_imbalance_alarm",
        # Alarm Status 2
        CONF_CELL_TEMP_IMBALANCE_ALARM: "set_cell_temp_imbalance_alarm",
        CONF_CHARGE_OVERCURRENT_ALARM: "set_charge_overcurrent_alarm",
        CONF_DISCHARGE_OVERCURRENT_ALARM: "set_discharge_overcurrent_alarm",
        # Protection Status 1
        CONF_MODULE_OVERVOLTAGE_PROTECTION: "set_module_overvoltage_protection",
        CONF_MODULE_UNDERVOLTAGE_PROTECTION: "set_module_undervoltage_protection",
        CONF_CELL_OVERVOLTAGE_PROTECTION: "set_cell_overvoltage_protection",
        CONF_CELL_UNDERVOLTAGE_PROTECTION: "set_cell_undervoltage_protection",
        CONF_CELL_OVERTEMP_PROTECTION: "set_cell_overtemp_protection",
        CONF_CELL_UNDERTEMP_PROTECTION: "set_cell_undertemp_protection",
        CONF_MOSFET_OVERTEMP_PROTECTION: "set_mosfet_overtemp_protection",
        # Protection Status 2
        CONF_CHARGE_OVERCURRENT_PROTECTION: "set_charge_overcurrent_protection",
        CONF_DISCHARGE_OVERCURRENT_PROTECTION: "set_discharge_overcurrent_protection",
        CONF_SYSTEM_FAULT_PROTECTION: "set_system_fault_protection",
    }

    # Loop through the SENSOR_MAP and generate code for sensors
    for key, setter_name in SENSOR_MAP.items():
        if key in config:
            sens = await cg.get_variable(config[key])
            cg.add(getattr(var, setter_name)(sens))

    # Loop through the BINARY_SENSOR_MAP and generate code for binary sensors
    for key, setter_name in BINARY_SENSOR_MAP.items():
        if key in config:
            sens = await cg.get_variable(config[key])
            cg.add(getattr(var, setter_name)(sens))

    # Set the update timeout
    cg.add(var.set_update_timeout(config[CONF_UPDATE_TIMEOUT]))