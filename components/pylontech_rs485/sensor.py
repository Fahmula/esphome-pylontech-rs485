import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import CONF_ID


# --- The C++ namespace and get a handle to the class ---
pylontech_rs485_ns = cg.esphome_ns.namespace("esphome::pylontech_rs485")
PylontechRS485 = pylontech_rs485_ns.class_("PylontechRS485", cg.Component, uart.UARTDevice)

# --- Primary dynamic values ---
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"

# --- Charge/Discharge Limits (for command 63) ---
CONF_MAX_CHARGE_CURRENT = "max_charge_current"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_MAX_VOLTAGE = "max_voltage"
CONF_MIN_VOLTAGE = "min_voltage"

# --- Sensor values --- 
CONF_MAX_TEMPERATURE = "max_temperature"
CONF_MIN_TEMPERATURE = "min_temperature"

# --- General Health values ---
CONF_SOH = "state_of_health"
CONF_CYCLE_COUNT = "cycle_count"
CONF_TEMPERATURE = "temperature"

# --- Cell-level values ---
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"

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

# --- Main Configuration Schema ---
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechRS485),
            
            # --- Make all sensors OPTIONAL to allow for progressive setup ---
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
            
            cv.Optional(CONF_MAX_VOLTAGE): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_MIN_VOLTAGE): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_MAX_CHARGE_CURRENT): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_MAX_DISCHARGE_CURRENT): cv.use_id(sensor.Sensor),

            cv.Optional(CONF_UPDATE_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

# --- Function to generate C++ code from the config ---
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], PylontechRS485)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # --- Helper dictionary to avoid repetition ---
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

    # Loop through the map and generate code if the key exists in the config
    for key, setter_name in SENSOR_MAP.items():
        if key in config:
            sens = await cg.get_variable(config[key])
            cg.add(getattr(var, setter_name)(sens))

    # Set the update timeout
    cg.add(var.set_update_timeout(config[CONF_UPDATE_TIMEOUT]))