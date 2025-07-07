import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import (
    CONF_ID,
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_TEMPERATURE,
    CONF_STATE_OF_CHARGE,
    CONF_UART_ID,
    CONF_MAX_VOLTAGE,
    CONF_MIN_VOLTAGE,
)

# Define the namespace for our custom component
pylontech_rs485_ns = cg.esphome_ns.namespace("pylontech_rs485")

# Define the C++ class for our component
PylontechRS485 = pylontech_rs485_ns.class_(
    "PylontechRS485", cg.Component, uart.UARTDevice
)

# Define custom keys
CONF_MAX_CHARGE_CURRENT = "max_charge_current"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_UPDATE_TIMEOUT = "update_timeout"

# Define the configuration schema
# All sensor and limit keys are now REQUIRED. No defaults.
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechRS485),
            # Links to live sensor data (REQUIRED)
            cv.Required(CONF_STATE_OF_CHARGE): cv.use_id(sensor.Sensor),
            cv.Required(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
            cv.Required(CONF_CURRENT): cv.use_id(sensor.Sensor),
            cv.Required(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
            
            # Configurable battery limits (REQUIRED)
            cv.Required(CONF_MAX_VOLTAGE): cv.voltage,
            cv.Required(CONF_MIN_VOLTAGE): cv.voltage,
            cv.Required(CONF_MAX_CHARGE_CURRENT): cv.current,
            cv.Required(CONF_MAX_DISCHARGE_CURRENT): cv.current,

            # Optional timeout for sensor updates
            cv.Optional(CONF_UPDATE_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # Link the required sensor IDs to the C++ setter methods
    sens = await cg.get_variable(config[CONF_STATE_OF_CHARGE])
    cg.add(var.set_soc_sensor(sens))

    sens = await cg.get_variable(config[CONF_VOLTAGE])
    cg.add(var.set_voltage_sensor(sens))

    sens = await cg.get_variable(config[CONF_CURRENT])
    cg.add(var.set_current_sensor(sens))

    sens = await cg.get_variable(config[CONF_TEMPERATURE])
    cg.add(var.set_temperature_sensor(sens))

    # Set the required battery limits
    cg.add(var.set_max_charge_voltage(config[CONF_MAX_VOLTAGE]))
    cg.add(var.set_min_discharge_voltage(config[CONF_MIN_VOLTAGE]))
    cg.add(var.set_max_charge_current(config[CONF_MAX_CHARGE_CURRENT]))
    cg.add(var.set_max_discharge_current(config[CONF_MAX_DISCHARGE_CURRENT]))

    # Set the update timeout
    cg.add(var.set_update_timeout(config[CONF_UPDATE_TIMEOUT]))