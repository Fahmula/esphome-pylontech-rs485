# ESPHome Pylontech RS485 Battery Emulator

An ESPHome component that allows an ESP device to emulate the Pylontech RS485 low-voltage battery protocol. This enables the use of DIY batteries or other battery management systems (like YAM-BMS) with inverters that expect to communicate with a Pylontech-compatible BMS.

## Key Features

*   **Dynamic Data:** Takes live data from ESPHome sensors to report battery status.
*   **Highly Configurable:** All parameters are configurable through your ESPHome YAML file.
*   **Protocol Emulation:** Replicates specific protocol quirks for compatibility with certain inverter brands.

## Project Status & Compatibility

**⚠️ IMPORTANT:** The Pylontech RS485 protocol is not a strict standard. Different inverter manufacturers have implemented their own "dialects" with unique quirks.

This project was developed and tested specifically against an **SRNE-based inverter (rebranded as EcoWorthy)**. It successfully emulates the specific protocol quirks required by this family of inverters.

**Current Confirmed Working Inverters:**
*   SRNE
*   EcoWorthy (All-in-One models)

While the core logic may work for other inverters, it is not guaranteed. We welcome contributions and test reports for other brands.

### The "SRNE Dialect" Discovery

Through extensive reverse-engineering, we discovered that the SRNE inverter requires two key non-standard behaviors from the BMS:

1.  **Fixed Response Address:** The inverter polls multiple battery addresses, but it expects every response to come from the master battery address (`02`).
2.  **Structural Quirk in Frame `62H`:** The protocol has a non-standard format for the LENGTH field (`LCHKSUM` + `LENID`).

This emulator replicates these exact structural quirks to ensure full compatibility with SRNE-based inverters.

## Hardware Prerequisites

To use this component, you will need:

*   An ESP32 or ESP8266 based development board.
*   An RS485 to TTL converter module (e.g., MAX485 module) to interface with the inverter's RS485 port.

## Installation

To include this component in your ESPHome project, add the following to your YAML configuration file:

```yaml
external_components:
  - source: github://fahmula/esphome-pylontech-rs485@main
    refresh: 0s # Optional: Set to a duration (e.g., 1h) to periodically check for updates
```

## Configuration

The `pylontech_rs485` component is configured under its own block in your ESPHome YAML file.

```yaml
pylontech_rs485:
  uart_id: uart_bus # REQUIRED: Link to the UART bus defined elsewhere in your YAML
  
  # REQUIRED: Link to ESPHome sensor entities providing live battery data
  state_of_charge: live_soc
  voltage: live_voltage
  current: live_current
  temperature: live_temperature

  # REQUIRED: Link to ESPHome sensor entities providing dynamic battery limits
  max_voltage: live_max_v
  min_voltage: live_min_v
  max_charge_current: live_max_charge_i
  max_discharge_current: live_max_discharge_i

  # OPTIONAL: Configure the sensor update timeout
  # If no new data is received from the linked sensors for this duration,
  # the component will stop talking to the inverter.
  # Default: 60s
  update_timeout: 60s 
```

### Parameter Descriptions:

*   `uart_id` (Required): The ID of the `uart` bus component used for RS485 communication with the inverter.
*   `state_of_charge` (Required): The ID of an ESPHome `sensor` entity that provides the current State of Charge (SoC) as a percentage (0-100).
*   `voltage` (Required): The ID of an ESPHome `sensor` entity that provides the current battery voltage in Volts.
*   `current` (Required): The ID of an ESPHome `sensor` entity that provides the current battery current in Amperes. Positive for charging, negative for discharging.
*   `temperature` (Required): The ID of an ESPHome `sensor` entity that provides the current battery temperature in Celsius.
*   `max_voltage` (Required): The ID of an ESPHome `sensor` entity that provides the maximum allowed battery charge voltage in Volts.
*   `min_voltage` (Required): The ID of an ESPHome `sensor` entity that provides the minimum allowed battery discharge voltage in Volts.
*   `max_charge_current` (Required): The ID of an ESPHome `sensor` entity that provides the maximum allowed charging current in Amperes.
*   `max_discharge_current` (Required): The ID of an ESPHome `sensor` entity that provides the maximum allowed discharging current in Amperes.
*   `update_timeout` (Optional, default: `60s`): The duration after which, if no new data is received from the linked sensors, the component will stop sending data to the inverter.

## Providing Battery Data (The Core Concept)

This component is designed to be controlled by standard ESPHome `sensor` entities. This allows you to feed it live battery data from any source, such as:

*   A dedicated BMS (Battery Management System) connected to your ESP device.
*   Template sensors that calculate values based on other inputs.
*   Input helpers (e.g., `number` entities in Home Assistant) for manual testing and simulation.

The `pylontech_rs485` component simply reads the `state` of the `sensor` entities you link to its configuration parameters.

## Full Example: Manual Testing with Home Assistant Sliders

The `pylontech-emulator-example.yaml` file in this repository provides a complete configuration for manual testing. This example sets up `number` entities (sliders) in Home Assistant, allowing you to simulate various battery parameters and observe the inverter's response.

### How the Example Works:

1.  **Global Variables:** `globals` are defined to store the current values of the simulated battery parameters (e.g., `g_soc`, `g_voltage`). These values are restored on reboot.
2.  **Home Assistant Sliders (`number` entities):** `number` entities are created, linked to the global variables. These appear as sliders in Home Assistant, allowing you to manually adjust the simulated SoC, voltage, current, etc. When you move a slider, its `set_action` updates the corresponding global variable.
3.  **Live Data Sensors (`template sensor` entities):** `template sensor` entities are defined. These sensors simply read the current `state` of the Home Assistant sliders (which in turn reflect the global variables).
4.  **Pylontech Component Linkage:** The `pylontech_rs485` component is configured to read its required data (SoC, voltage, current, etc.) directly from these `template sensor` entities.

This setup creates a clear data flow, allowing you to easily test the component's behavior with your inverter.

```mermaid
graph TD
    subgraph Home Assistant UI
        A[Sliders for Voltage, SoC, etc.]
    end

    subgraph ESPHome Device
        B(Globals e.g., g_voltage)
        C(Template Sensors e.g., live_voltage)
        D[pylontech_rs485 Component]
    end

    subgraph Inverter
        E[Inverter]
    end

    A -- "set_action" --> B
    B -- "lambda" --> C
    C -- "feeds data to" --> D
    D -- "RS485" --> E
```

## Future Development

*   Add dynamic alarm generation to the `62H` handler based on live sensor data.
*   Add support for other inverter "dialects" (e.g., Growatt).

## Contributing

Contributions are welcome! If you get this working with a different inverter brand, please open an issue or pull request to share your findings.