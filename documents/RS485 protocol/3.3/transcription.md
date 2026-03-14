# PYLON low voltage Protocol (RS485)

This document describes the Pylontech low-voltage RS485 protocol used between a battery system and an inverter or upper computer (master).

Notes and terminology:

- The inverter or upper computer acts as the master.
- The first battery acts as a slave, and addresses start from 2.
- "Module" or "Battery module" refers to a 48 V battery module with BMS.
- "Cell" refers to a 3.2 V cell.

## Version history

| Date       | Version | Chapter(s)         | Notes |
|------------|---------|--------------------|-------|
| 2008-11-20 | V2.2    | -                  | First version. |
| 2008-12-23 | V2.3    | -                  | Adjusted module-count acquisition (data positions changed). Added cell undervoltage threshold and total-voltage undervoltage threshold to system parameters. Added definition for "Command != 0xFF" in commands 0x42 and 0x44. Added "pack power supply in use" indication in alarm State2. |
| 2009-03-26 | V2.4    | -                  | In command formats, ADR refers to the master address. Added buzzer enable/disable command. Added buzzer indication in State3 bit0. |
| 2009-12-07 | V2.4    | -                  | Added State4 and State5 bytes in alarm data to indicate single-cell faults. Re-interpreted State3 bit6 (effective discharge current) as an AC power outage alarm. |
| 2016-06-20 | V2.5    | -                  | Unified current unit description: actual value = transmitted value * 100. Added example parsing. Added interface and baud rate description. Removed unsupported commands. |
| 2016-08-19 | V2.6    | -                  | Added commands: get charge/discharge management info, get serial number. |
| 2016-10-13 | V2.7    | -                  | Added command: set charge/discharge management parameters. |
| 2016-12-15 | V2.8    | -                  | Added command: turn off. |
| 2017-01-17 | V2.9    | -                  | Added command: get firmware/software version. |
| 2017-11-22 | V2.9    | -                  | Translation improvements. |
| 2018-03-08 | V3.0    | ADR settings, alarm status | Added multi-group parallel mode (expanded addressing). Corrected "cell low voltage" wording to "cell under voltage". Moved general notes into each analog item for easier parsing. |
| 2018-04-08 | V3.1    | Charge/discharge management | Added force charge request 2 and full charge request. |
| 2018-06-04 | V3.2    | Analog values, system parameters, alarms, management | Added capacity reporting for batteries larger than 65 Ah. Modified examples. Added reading guidance. |
| 2018-08-21 | V3.3    | Charge/discharge management | Added explanation for bits 6-7. |

## Table of contents

- Protocol background
- Protocol
  - Port settings
  - Basic frame format
  - Data formats
  - Module introduction
  - Encoding tables (CID1, CID2, ADR)
- Communication commands
  - Get protocol version
  - Get manufacturer info
  - Get analog values (fixed point)
  - Get system parameters (fixed point)
  - Get alarm info
  - Get charge/discharge management info
  - Get module serial number
  - Set charge/discharge management info
  - Turn off module
  - Get software version
- Example

## Protocol background

The battery system provides communication items for monitoring and management, including:

- Status: charge/discharge status, SOC (capacity), output voltage, output current
- Environment: pack temperature, ambient temperature, cell temperatures
- Alarm and protection: charge over-voltage/current, discharge under-voltage, polarity reversal, discharge over-current, high temperature (pack and ambient), low SOC, sensor failures (temperature/voltage/current), cell over/under-voltage, etc.

## Protocol

This RS485 protocol follows a unified frame structure and defines commands and responses for battery data access.

### Port settings

Asynchronous serial communication:

- RS485 baud rates: 115.2 kbps (default), 500 kbps, 9.6 kbps
- Frame format: 1 start bit, 8 data bits, 1 stop bit, no parity

### Basic frame format

#### Frame layout

| Field  | SOI | VER | ADR | CID1 | CID2 | LENGTH | INFO    | CHKSUM | EOI |
|--------|-----|-----|-----|------|------|--------|---------|--------|-----|
| Bytes  | 1   | 1   | 1   | 1    | 1    | 2      | LENID/2 | 2      | 1   |

- SOI: start of frame marker
- EOI: end of frame marker (CR, 0x0D)

#### Field descriptions

| Field  | Meaning |
|--------|---------|
| SOI    | Start marker |
| VER    | Protocol version (battery may ignore the value in requests) |
| ADR    | Address (0 and 255 reserved). Addresses start from 2. |
| CID1   | Control identifier code |
| CID2   | In requests: command code (data type or control action). In responses: return code (RTN). |
| LENGTH | INFO length, including LENID and LCHKSUM (see LENGTH format) |
| INFO   | Request: command INFO. Response: data INFO. |
| CHKSUM | Checksum (see CHKSUM format) |
| EOI    | End marker, CR (0x0D) |

INFO can contain:

- Command INFO (in requests)
- Data INFO (in responses)

Command INFO format:

| Item          | Size    | Description |
|---------------|---------|-------------|
| Command group | 1 byte  | Group number for devices of the same type |
| Command type  | 1 byte  | Remote-control command type (or history transfer control) |
| Command id    | 1 byte  | Monitoring point within the group |
| Command time  | 7 bytes | Time field (see time format) |

Data INFO types (as referenced by the protocol):

| Name       | Description |
|------------|-------------|
| DATAI      | Fixed point response data |
| DATAF      | Floating point response data |
| DATA FLAG  | Data flag information |
| RUN STATE  | Device run state |
| WARN STATE | Alarm state |
| DATA TIME  | Event time (not used in this protocol) |

Data flag bit meaning (summary):

- Indicates whether there are unread switch-value changes and unread alarm-value changes.

### Data formats

#### Basic transmission format (HEX vs HEX-ASCII)

- SOI and EOI are transmitted as raw hexadecimal bytes.
- All other fields are interpreted as hexadecimal values, but transmitted as HEX-ASCII:
  - Each byte is encoded as two ASCII characters representing the hex value.
  - Example: if CID2 = 0x4B, it is transmitted as ASCII "4" (0x34) and "B" (0x42).

#### LENGTH format (LENID and LCHKSUM)

LENGTH is a 16-bit field composed of:

- Lower 12 bits: LENID, the number of ASCII bytes in INFO.
  - If LENID = 0, INFO is empty.
  - Max INFO size is 4095 ASCII bytes.

- Upper 4 bits: LCHKSUM, calculated from the three nibbles of LENID:
  - Compute: (D11..D8) + (D7..D4) + (D3..D0)
  - Take modulo 16 (keep the remainder)
  - Bitwise invert the 4-bit remainder, then add 1 (two's complement on 4 bits)

Transmission order:

- LENGTH is transmitted as high byte first, then low byte, encoded as 4 ASCII hex characters.

#### CHKSUM format

CHKSUM is computed over the ASCII bytes of all characters except SOI, EOI, and CHKSUM itself:

1. Sum the ASCII values of all included characters
2. Take modulo 65536
3. Bitwise invert the 16-bit remainder
4. Add 1

Example (from the document):

- Message content: "~1203400456ABCEFEFC72<CR>"
- The last 4 ASCII chars "FC72" are CHKSUM.
- Sum of ASCII values excluding SOI, EOI, and CHKSUM gives 0x038E
- Invert and add 1 gives 0xFC72

#### Data value format (fixed point)

Analog quantities can be sent using fixed point or floating point; this protocol uses fixed point.

- Signed 16-bit integer: -32768 to +32767
- Unsigned 16-bit integer: 0 to 65535

Fixed point types used:

| Telemetering item | Data type |
|-------------------|----------|
| Cell voltage      | Signed integer |
| Temperature       | Signed integer |
| Module voltage    | Unsigned integer |
| Module current    | Signed integer (positive is charge) |
| System parameter  | Signed integer |
| Capacity          | Unsigned integer |

#### Time format (DATA TIME and COMMAND TIME)

| Field  | Range  | Size    | Encoding |
|--------|--------|---------|----------|
| Year   | 1-9999 | 2 bytes | HEX integer (actual value = transmitted value) |
| Month  | 1-12   | 1 byte  | HEX |
| Day    | 1-31   | 1 byte  | HEX |
| Hour   | 0-23   | 1 byte  | HEX |
| Minute | 0-59   | 1 byte  | HEX |
| Second | 0-59   | 1 byte  | HEX |

### Module introduction

- Physical bus: RS485
- Default baud rate: 115200 bps

### Encoding tables

#### CID1

| Content      | CID1 |
|--------------|------|
| Battery data | 0x46 |

#### CID2 commands

Command codes (requests):

| Content | CID2 |
|--------|------|
| Get analog values (fixed point) | 0x42 |
| Get alarm info | 0x44 |
| Get system parameters (fixed point) | 0x47 |
| Get protocol version | 0x4F |
| Get manufacturer info | 0x51 |
| Get charge/discharge management info | 0x92 |
| Get module serial number | 0x93 |
| Set charge/discharge management info | 0x94 |
| Turn off | 0x95 |
| Get firmware/software version | 0x96 |

Return codes (responses, CID2 used as RTN):

| Meaning | RTN (CID2) |
|--------|------------|
| Normal | 0x00 |
| VER error | 0x01 |
| CHKSUM error | 0x02 |
| LCHKSUM error | 0x03 |
| CID2 invalid | 0x04 |
| Command format error | 0x05 |
| Invalid data (INFO invalid) | 0x06 |
| ADR error | 0x90 |
| Communication error (internal) | 0x91 |

### ADR settings (addressing)

Within one group, up to 8 or 12 batteries (see product spec). Position-based addressing:

| Position | Address (n) |
|----------|-------------|
| Master battery | 2 |
| Slave 1 | 3 |
| Slave 2 | 4 |
| Slave 3 | 5 |
| Slave 4 | 6 |
| Slave 5 | 7 |
| Slave 6 | 8 |
| Slave 7 | 9 |
| Slave 8 | 10 |
| Slave 9 | 11 |
| Slave 10 | 12 |
| Slave 11 | 13 |

Group addressing (multi-group mode) is configured by DIP switches on the master battery:

- DIP 1 selects RS485 baud rate:
  - 1: 9600
  - 0: 115200
  - Restart required to take effect

- DIP 2-4 define group address m (0 to 7), where 1 is up and 0 is down.
  - For single-group mode: master should be set to X000 (m = 0).
  - For multi-group mode: the first group should start at X100 (m = 1) to keep address rules consistent.

Address calculation:

Battery (module) information:

- ADR = (battery address n) + 0x10 * m
- INFO (in request) = ADR (as a 1-byte command value) for commands that require it

Examples:

- Single group, slave 4:
  - n = 5, m = 0
  - ADR = 0x05
  - INFO command value = 0x05

- Multi group, group 3, slave 6:
  - n = 7, m = 3
  - ADR = 0x07 + 0x30 = 0x37
  - INFO command value = 0x37

System information (only for get analog values and get alarm info):

- ADR = 0x02 + 0x10 * m
- INFO command value = 0xFF

Example:

- Group 2:
  - ADR = 0x02 + 0x20 = 0x22
  - INFO command value = 0xFF

## Communication commands

In all frames below:

- SOI is "~" (0x7E) as shown in examples.
- EOI is CR (0x0D).
- CID1 is 0x46 for battery data.
- Unless stated otherwise, requests use VER as an arbitrary value and the battery ignores it.

### Get protocol version (CID2 = 0x4F)

Request:

| Field | Value |
|------|-------|
| SOI | 0x7E |
| VER | arbitrary |
| ADR | target address |
| CID1 | 0x46 |
| CID2 | 0x4F |
| LENGTH | LENID = 0 |
| INFO | empty |
| CHKSUM | computed |
| EOI | 0x0D |

Response:

| Field | Value |
|------|-------|
| VER | protocol version (example: V2.1 reported as 0x21) |
| CID2 | RTN (return code) |

### Get manufacturer info (CID2 = 0x51)

Request:

- LENID = 0 (no INFO)

Response:

- LENID = 0x40 (64 ASCII bytes in INFO)

Manufacturer info payload (DATAINFO):

| Item | Field | Format |
|------|-------|--------|
| 1 | Battery name | 10 bytes, ASCII |
| 2 | Software version | 2 bytes |
| 3 | Manufacturer name | 20 bytes, ASCII |

Total binary payload is 32 bytes, transmitted as 64 ASCII characters.

### Get analog values, fixed point (CID2 = 0x42)

Request:

- LENID = 0x02 (INFO contains 1 command byte transmitted as 2 ASCII hex characters)
- INFO command byte:
  - 0x01 to get data of battery 1
  - ...
  - 0x08 to get data of battery 8
- The command value must match ADR (same target battery).

Response:

- INFO = INFOFLAG + DATAI

DATAI structure:

| Item | Field | Size |
|------|-------|------|
| 1 | Command value | 1 byte |
| 2 | Battery data | variable |

Battery data format:

Let:
- M = number of cells
- N = number of temperature points

| Order | Field | Size | Units and notes |
|------:|-------|------|-----------------|
| 1 | Number of cells (M) | 1 byte | count |
| 2..(M+1) | Cell 1 voltage .. Cell M voltage | 2 bytes each | 3-decimal volts, typically interpreted as mV (example: 3397 means 3.397 V) |
| M+2 | Number of temperatures (N) | 1 byte | count |
| M+3..(M+N+2) | Temperature 1..N | 2 bytes each | Kelvin-coded, 0.1 C resolution: value = tempC * 10 + 2731. Examples: 25.5 C -> 2986, -12.4 C -> 2607 |
| M+N+3 | Current | 2 bytes | signed. Scale: actual current in mA = raw * 100. Equivalent: actual current in A = raw * 0.1. Positive is charge, negative is discharge. Example: -4000 mA -> raw -40 -> 0xFFD8 |
| M+N+4 | Module voltage | 2 bytes | 3-decimal volts, typically mV |
| M+N+5 | Remaining capacity 1 | 2 bytes | Ah, 3 decimals |
| M+N+6 | User defined item count | 1 byte | 2 if capacity <= 65 Ah, 4 if capacity > 65 Ah |
| M+N+7 | Module total capacity 1 | 2 bytes | Ah, 3 decimals |
| M+N+8 | Cycle number | 2 bytes | unsigned |

Additional fields for batteries with capacity > 65 Ah (for backward compatibility):

| Order | Field | Size | Notes |
|------:|------|------|------|
| M+N+9 | Remaining capacity 2 | 3 bytes | Ah, 3 decimals (use this for > 65 Ah) |
| M+N+10 | Module total capacity 2 | 3 bytes | Ah, 3 decimals (use this for > 65 Ah) |

Compatibility rule:

- For US2000B and US2000B-Plus: user defined items = 2, use Remaining capacity 1 and Module total capacity 1.
- For US3000 or capacity > 65 Ah: user defined items = 4, and Remaining capacity 1 and Module total capacity 1 are sent as 0xFFFF placeholders. Use Remaining capacity 2 and Module total capacity 2 instead.

### Get system parameters, fixed point (CID2 = 0x47)

Request:

- LENID = 0

Response:

- INFO = INFOFLAG + DATAI

DATAI fields:

| No | Field | Size | Units and notes |
|----:|------|------|-----------------|
| 1 | Cell high voltage limit | 2 bytes | volts, 3 decimals |
| 2 | Cell low voltage limit (alarm) | 2 bytes | volts, 3 decimals |
| 3 | Cell under-voltage limit (protection) | 2 bytes | volts, 3 decimals |
| 4 | Charge high temperature limit | 2 bytes | Kelvin-coded as above |
| 5 | Charge low temperature limit | 2 bytes | Kelvin-coded as above |
| 6 | Charge current limit | 2 bytes | signed, raw * 0.1 A (positive is charge) |
| 7 | Module high voltage limit | 2 bytes | volts, 3 decimals |
| 8 | Module low voltage limit (alarm) | 2 bytes | volts, 3 decimals |
| 9 | Module under-voltage limit (protection) | 2 bytes | volts, 3 decimals |
| 10 | Discharge high temperature limit | 2 bytes | Kelvin-coded as above |
| 11 | Discharge low temperature limit | 2 bytes | Kelvin-coded as above |
| 12 | Discharge current limit | 2 bytes | signed, raw * 0.1 A |

### Get alarm info (CID2 = 0x44)

Request:

- LENID = 0x02 (INFO contains 1 command byte, 0x01..0x08)
- Command must match ADR.

Response:

- INFO = DATAFLAG + WARNSTATE

WARNSTATE structure:

| Item | Field | Size |
|------|-------|------|
| 1 | Command value | 1 byte |
| 2 | Module alarm info | variable |

Module alarm info format mirrors the analog layout plus status bytes:

Let:
- M = number of cells
- N = number of temperature points

The module alarm info includes:

- Cell count M (1 byte)
- Cell voltage status for each cell (M bytes, 1 byte per cell)
- Temperature count N (1 byte)
- Temperature status for each point (N bytes, 1 byte per point)
- Charge current status (1 byte)
- Module voltage status (1 byte)
- Discharge current status (1 byte)
- Status1..Status5 (5 bytes)

Alarm status code meaning for per-channel bytes:

| Value | Meaning |
|-------|---------|
| 0x00 | Normal |
| 0x01 | Below lower limit (acts as protection) |
| 0x02 | Above higher limit (acts as protection) |
| 0xF0 | Other error |

Status1 bit definitions:

| Bit | Meaning |
|-----|---------|
| 7 | Module under-voltage (UV) triggered |
| 6 | Charge over-temperature triggered |
| 5 | Discharge over-temperature triggered |
| 4 | Discharge over-current (DOC) triggered |
| 2 | Charge over-current (COC) triggered |
| 1 | Cell under-voltage triggered |
| 0 | Module over-voltage (OV) triggered |

Status2 bit definitions:

| Bit | Meaning |
|-----|---------|
| 3 | Using battery module power (1 using, 0 not) |
| 2 | Discharge MOSFET state (1 on, 0 off) |
| 1 | Charge MOSFET state (1 on, 0 off) |
| 0 | Pre-charge MOSFET (reserved, not used) |

Status3 bit definitions:

| Bit | Meaning |
|-----|---------|
| 7 | Effective charge current detected (BMS measured current > 0.1 A) |
| 6 | Effective discharge current detected (BMS measured current < -0.1 A) |
| 5 | Heater (reserved, not used) |
| 3 | Fully charged indication (SOC = 100%) |
| 0 | Buzzer (1 on, 0 off) |

Status4 and Status5 indicate per-cell voltage failure flags:

- Status4 bits 0-7 correspond to Cell 1-8
- Status5 bits 0-7 correspond to Cell 9-16
- 1 means error, 0 means normal

Cell voltage failure criteria (as described):

- Cell voltage > 4.2 V (battery charge MOS off), or
- Cell voltage < 1.0 V (battery shuts down by itself)

### Get charge/discharge management info (CID2 = 0x92)

Request:

- LENID = 0x02 (INFO contains command byte 0x01..0x08)
- Command must match ADR.

Response:

- INFO = DATAI

DATAI fields:

| No | Field | Size | Units and notes |
|----:|------|------|-----------------|
| 1 | Command value | 1 byte | echoes request |
| 2 | Charge voltage limit (recommended upper limit) | 2 bytes | volts, 3 decimals |
| 3 | Discharge voltage limit (recommended lower limit) | 2 bytes | volts, 3 decimals |
| 4 | Max charge current | 2 bytes | raw * 0.1 A |
| 5 | Max discharge current | 2 bytes | raw * 0.1 A |
| 6 | Charge/discharge status flags | 1 byte | bitfield |

Charge/discharge status flags:

| Bit | Meaning | Notes |
|-----|---------|------|
| 7 | Charge enable | 1 yes, 0 request stop charge |
| 6 | Discharge enable | 1 yes, 0 request stop discharge |
| 5 | Force charge 1 (charge immediately) | 1 active |
| 4 | Force charge 2 (charge immediately) | 1 active |
| 3 | Full charge request | 1 active |
| 2..0 | Reserved | - |

Force charge behavior notes:

- Bit 5:
  - US2000B: SOC 15% to 19%
  - US2000B-Plus and US3000B: SOC 5% to 9%
  - Intended for inverters that can wake/activate the battery by DC voltage, or that enforce their own low SOC/voltage discharge limit.

- Bit 4:
  - US2000B-Plus and US3000B: SOC 9% to 13%
  - Intended for inverters that do not have battery activation behavior or do not want the battery to shut down.

It is recommended to support both force charge bits for compatibility.

Full charge request (bit 3):

- Purpose: SOC estimation error can accumulate if the battery is not fully charged for a long time.
- Logic: if SOC never exceeds 97% within 30 days, set this flag to 1; clear it when SOC >= 97%.
- Recommendation: when set, the inverter should charge the battery (for example from the grid) to reach full charge.

### Get module serial number (CID2 = 0x93)

Request:

- LENID = 0x02, INFO command byte 0x01..0x08

Response:

- INFO = DATAI

DATAI fields:

| Item | Field | Format |
|------|-------|--------|
| 1 | Command value | 1 byte |
| 2 | Module serial number | 16 bytes, ASCII |

### Set charge/discharge management info (CID2 = 0x94)

Request:

- LENID = 0x12 (18 ASCII bytes)
- INFO contains 9 binary bytes (encoded as 18 ASCII hex chars):
  - 1 byte command (0x01..0x08)
  - 8 bytes DataF (4 values x 2 bytes)

DataF fields:

| No | Field | Size |
|----:|------|------|
| 1 | Charge voltage limit | 2 bytes |
| 2 | Discharge voltage limit | 2 bytes |
| 3 | Max charge current | 2 bytes |
| 4 | Max discharge current | 2 bytes |

Response:

- LENID = 0

Important note:

- If this command is used, it must be sent periodically. If the battery does not receive it again within 10 seconds, it will revert to automatically setting its own management values based on current conditions.
- Do not use this command unless Pylontech has clarified the recommended strategy for these values.

### Turn off module (CID2 = 0x95)

Request:

- LENID = 0x02, INFO command byte 0x01..0x08
- Command must match ADR.

Response:

- LENID = 0

### Get software version (CID2 = 0x96)

Request:

- LENID = 0x02, INFO command byte 0x01..0x08
- Command must match ADR.

Response:

- INFO = DATAI

DATAI fields:

| Item | Field | Size | Notes |
|------|-------|------|------|
| 1 | Command value | 1 byte | echoes request |
| 2 | Module software version | 5 bytes | 2 bytes manufacturer version + 3 bytes main line version |

## Example (74 Ah battery)

### Get analog values (fixed point)

Send command (hex bytes as transmitted):

```text
7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D
```

Receive data (hex bytes as transmitted):

```text
7E 32 30 30 32 34 36 30 30 46 30 37 41 31 31 30 32 30 46 30
44 34 35 30 44 34 34 30 44 34 35 30 44 34 34 30 44 34 35 30
44 34 34 30 44 34 35 30 44 34 34 30 44 33 45 30 44 34 35 30
44 34 41 30 44 34 41 30 44 34 42 30 44 34 41 30 44 34 41 30
44 34 41 30 44 34 41 30 35 30 42 43 33 30 42 43 33 30 42 43
33 30 42 43 44 30 42 43 44 30 30 30 30 43 37 32 35 46 46 46
46 30 34 46 46 46 46 30 30 30 32 30 30 43 41 35 38 30 31 32
31 31 30 45 31 41 32 0D
```

Parsed highlights (selected fields):

| Field | Raw | Meaning |
|------|-----|---------|
| Cell count | 0x0F | 15 cells |
| Cell 1 voltage | 0x0D45 | 3397 mV (3.397 V) |
| Cell 2 voltage | 0x0D44 | 3396 mV (3.396 V) |
| Temperature count N | 0x05 | 5 points |
| Temperature 1 | 0x0BC3 | 3011 -> 28 C |
| Temperature N | 0x0BCD | 3021 -> 29 C |
| Current | 0x0000 | 0 A |
| Module voltage | 0xC725 | 50981 mV (50.981 V) |
| Remaining capacity 1 | 0xFFFF | not used for > 65 Ah in this example |
| User defined count | 0x04 | 4 |
| Module total capacity 1 | 0xFFFF | not used for > 65 Ah in this example |
| Cycle number | 0x0002 | 2 |
| Remaining capacity 2 | 0xCA58 | 51.800 Ah |
| Module total capacity 2 | 0x12110 | 74.000 Ah |
