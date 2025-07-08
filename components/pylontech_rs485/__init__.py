import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor # Import sensor here
from esphome.const import CONF_ID
from .sensor import SENSOR_KEYS_SCHEMA

pylontech_rs485_ns = cg.esphome_ns.namespace("esphome::pylontech_rs485")
PylontechRS485 = pylontech_rs485_ns.class_("PylontechRS485", cg.Component, uart.UARTDevice)

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

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], PylontechRS485)
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    from . import sensor as sensor_module
    await sensor_module.to_code(var, config)