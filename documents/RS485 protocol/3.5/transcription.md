# PYLON Low Voltage Protocol (RS485)

## Version History

| Date | Version | Chapter | Notes | Author |
|---|---|---|---|---|
| 2008-11-20 | V2.2 | - | First version | - |
| 2008-12-23 | V2.3 | 1 | 1. Added module quantity retrieval and adjusted related data positions. 2. Added cell under-voltage threshold and total voltage under-voltage threshold to system parameters. 3. Added definition in commands `0x42` and `0x44` when Command is not `0xFF`. 4. Added Pack power-supply indication to warning info `State2`. | - |
| 2009-03-26 | V2.4 | - | 1. Clarified that `ADR` in the command format refers to the host/master address. 2. Added command for buzzer enable/disable. 3. Added buzzer indication at `State3 bit0`. | - |
| 2009-12-07 | V2.4 | - | 1. Added `State4` and `State5` bytes in alarm data to indicate individual cell faults. 2. Interpreted `State3 bit6` in alarm data as AC power loss alarm. | - |
| 2016-06-20 | V2.5 | - | 1. Revised current unit description to use: actual value = transmitted value × 100. 2. Added example parsing. 3. Added communication interface and baud rate description. 4. Removed unsupported commands. | - |
| 2016-08-19 | V2.6 | - | Added commands: get charge/discharge management information; get serial number. | - |
| 2016-10-13 | V2.7 | - | Added command: set charge/discharge parameters. | - |
| 2016-12-15 | V2.8 | - | Added command: turn off. | - |
| 2017-01-17 | V2.9 | - | Added command: get firmware version. | - |
| 2017-11-22 | V2.9 | - | Improved translation. | - |
| 2018-03-08 | V3.0 | 2.5.3, 3.3, 3.4, 3.5 | 1. Added multi-group parallel mode and expanded address range. 2. Corrected "cell low voltage" to "cell under-voltage". 3. Removed old Chapter 4 and moved its notes into each analog item for easier parsing. | Wang Wanxiang, Ye Wen, Wang Zhonghe |
| 2018-04-08 | V3.1 | 3.6 | Added "charge immediately 2" flag and "full charge request". | Wang Yakun, Wang Zhonghe |
| 2018-06-04 | V3.2 | 3.3, 3.4, 3.5, 3.6 | 1. Added support for battery capacity values greater than 65 Ah. 2. Modified examples. 3. Added explanatory reading notes. | Ye Wen, Wang Zhonghe |
| 2018-08-21 | V3.3 | 3.6 | Added description for bits 6 and 7. | Wang Zhonghe |
| 2018-09-27 | V3.4 | 3.2, 3.10 | 1. Added reading instructions. 2. Corrected description errors. | Wang Zhonghe |
| 2019-08-07 | V3.5 | - | Added a CAN-like design approach for obtaining system-level information by querying the master, to support expansion. | Zou Huixing |

## Notes for Batteries Using This Protocol

- When a battery communicates with an inverter or upper computer, the upper device is treated as the master by default, and the first battery address starts from `2`.
- In multi-group mode, the master battery must be configured with the correct DIP-switch group address.

Definitions:
- `Module` / `Battery module`: a 48 V or 24 V battery module with BMS
- `Cell`: a 3.2 V cell

## Contents

1. Protocol
   - 1.1 Port settings
   - 1.2 Basic format
     - 1.2.1 Basic frame format
     - 1.2.2 Frame field definitions
   - 1.3 Data format
     - 1.3.1 Basic data format
     - 1.3.2 LENGTH format
     - 1.3.3 CHKSUM format
     - 1.3.4 DATA INFO format
     - 1.3.5 DATA TIME and COMMAND TIME format
   - 1.4 Module introduction
   - 1.5 Encoding table
     - 1.5.1 CID1
     - 1.5.2 CID2
     - 1.5.3 Address settings
2. System communication protocol
   - 2.1 Get battery system basic information
   - 2.2 Get battery system analog data
   - 2.3 Get battery system status and alarm information
   - 2.4 Get battery system charge/discharge management information
   - 2.5 Control battery system shutdown

---

# 1. Protocol

## 1.1 Port Settings

Transmission rate:
- RS485: `115200 bps` (recommended)
- RS485: `9600 bps`

Serial format:
- Start bit: 1
- Data bits: 8
- Stop bit: 1
- Parity: none

## 1.2 Basic Format

### 1.2.1 Basic Frame Format

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID / 2 | 2 | 1 |
| Field | SOI | VER | ADR | CID1 | CID2 | LENGTH | INFO | CHKSUM | EOI |

### 1.2.2 Frame Field Definitions

| Field | Meaning |
|---|---|
| `SOI` | Start marker |
| `VER` | Protocol version |
| `ADR` | Address (`0` and `255` reserved). In a single group, addresses start from `2`. |
| `CID1` | Control identifier code |
| `CID2` | In commands: command type identifier. In responses: return code. |
| `LENGTH` | Length of INFO in ASCII bytes, including `LENID` and `LCHKSUM` encoding rules |
| `INFO` | Command payload or response payload |
| `CHKSUM` | Checksum |
| `EOI` | End marker: `CR (0x0D)` |

### Command INFO Structure

| Field | Size | Meaning |
|---|---:|---|
| Command group | 1 byte | Group number within the same device type |
| Command type | 1 byte | Remote control command type or history/control command type |
| Command ID | 1 byte | Monitoring point within the same device group |
| Command time | 7 bytes | Time field |

### Response DATA INFO Categories

| Name | Meaning |
|---|---|
| `DATAI` | Fixed-point response data |
| `DATAF` | Floating-point response data |
| `DATA FLAG` | Data flag information |
| `RUN STATE` | Battery operating state |
| `WARN STATE` | Alarm information |
| `DATA TIME` | Event timestamp (not used in this protocol) |

### Data Flag Bit Meaning

The original table is partially corrupted in OCR, but the intended meaning is:

- Bit 4:
  - `0`: no unread digital/switch state change
  - `1`: unread digital/switch state change exists
- Bit 0:
  - `0`: no unread alarm state change
  - `1`: unread alarm state change exists

All other bits are reserved in this document.

## 1.3 Data Format

### 1.3.1 Basic Data Format

`SOI` and `EOI` are interpreted and transmitted directly in hexadecimal.

All other fields are interpreted as hexadecimal values but transmitted as hexadecimal ASCII. Each byte is represented by two ASCII characters.

Example:

If `CID2 = 0x4B`, it is transmitted as:
- `0x34` (`'4'`)
- `0x42` (`'B'`)

### 1.3.2 LENGTH Format

`LENGTH` is 2 bytes:

- Upper 4 bits: `LCHKSUM`
- Lower 12 bits: `LENID`

`LENID` is the number of ASCII bytes in the `INFO` field.  
If `LENID = 0`, then `INFO` is empty.

Because `LENID` is 12 bits, the maximum packet size is `4095` bytes.

Transmission order:
1. High byte first
2. Low byte second
3. Sent as 4 ASCII characters total

`LCHKSUM` is calculated from the 12-bit `LENID` as:

- Split `LENID` into 3 nibbles:
  - `D11..D8`
  - `D7..D4`
  - `D3..D0`
- Sum the three nibbles
- Take modulo 16
- Invert
- Add 1

Example:

If `INFO` contains `18` ASCII bytes:

- `LENID = 0b000000010010`
- Nibble sum:
  - `0000 + 0001 + 0010 = 0011`
- Modulo 16 result = `0011`
- Invert + 1 = `1101`

So:
- `LCHKSUM = 1101`
- `LENGTH = 1101 0000 0001 0010`
- Transmitted as: `D012`

### 1.3.3 CHKSUM Format

`CHKSUM` is calculated as follows:

- Sum the ASCII values of all transmitted characters except:
  - `SOI`
  - `EOI`
  - `CHKSUM`
- Take the result modulo `65536`
- Invert
- Add 1

Example:

```text
~1203400456ABCEFEFC71\r
```

Where:
- `~` is `SOI`
- `\r` is `EOI`
- `FC71` is `CHKSUM`

If the sum of ASCII codes from `'1'` through the last `'E'` is `0x038F`, then:

- remainder = `0x038F`
- invert + 1 = `0xFC71`

### 1.3.4 DATA INFO Format

Analog values may be transmitted as either:
- fixed-point
- floating-point

This protocol uses fixed-point values.

Data types:
- Signed integer: `-32768` to `+32767`
- Unsigned integer: `0` to `65535`

### 1.3.5 DATA TIME and COMMAND TIME Format

| Field | Range | Type | Encoding |
|---|---|---|---|
| Year | 1 to 9999 | Integer | 2 bytes, hex |
| Month | 1 to 12 | Integer | 1 byte, hex |
| Day | 1 to 31 | Integer | 1 byte, hex |
| Hour | 0 to 23 | Integer | 1 byte, hex |
| Minute | 0 to 59 | Integer | 1 byte, hex |
| Second | 0 to 59 | Integer | 1 byte, hex |

Note:
- Year is transmitted directly as the actual integer value.

## 1.4 Module Introduction

- Uses RS485 bus communication
- Default communication rate: `9600 bps`

## 1.5 Encoding Table

### 1.5.1 CID1

| Content | CID1 |
|---|---|
| Battery data | `46H` |

### 1.5.2 CID2

#### Command Codes

| No. | Content | CID2 | Notes |
|---|---|---|---|
| 1 | Get analog values, fixed-point | `42H` | - |
| 2 | Get alarm info | `44H` | - |
| 3 | Get system parameters, fixed-point | `47H` | - |
| 4 | Get protocol version | `4FH` | - |
| 5 | Get manufacturer info | `51H` | - |
| 6 | Get charge/discharge management info | `92H` | - |
| 7 | Get battery serial number | `93H` | - |
| 8 | Set charge/discharge management info | `94H` | - |
| 9 | Turn off | `95H` | - |
| 10 | Get firmware version | `96H` | - |
| 11 | Get battery system basic info | `60H` | Only valid for master address |
| 12 | Get battery system analog data | `61H` | Only valid for master address |
| 13 | Get battery system alarm info | `62H` | Only valid for master address |
| 14 | Get system charge/discharge management info | `63H` | Only valid for master address |
| 15 | System shutdown | `64H` | Only valid for master address |

#### Response Codes

| Meaning | CID2 | Notes |
|---|---|---|
| Normal | `00H` | - |
| Version error | `01H` | - |
| CHKSUM error | `02H` | - |
| LCHKSUM error | `03H` | - |
| Invalid CID2 | `04H` | - |
| Command format error | `05H` | - |
| Invalid data | `06H` | INFO data invalid |
| Address error | `90H` | - |
| Communication error | `91H` | Internal communication error |

#### Fixed-Point Data Types

| Content | Type |
|---|---|
| Cell voltage | Signed integer |
| Temperature | Signed integer |
| Module voltage | Unsigned integer |
| Module current | Signed integer; charge current is positive |
| System parameter | Signed integer |
| Capacity | Unsigned integer |

### 1.5.3 Address Settings

Refer to the product specification for the physical DIP-switch details.

Maximum number of batteries per group: refer to the product specification.

Battery addresses within a group:

| Address | Position |
|---|---|
| `2` | Master battery |
| `3` | Slave 1 |
| `4` | Slave 2 |
| `5` | Slave 3 |
| `6` | Slave 4 |
| `7` | Slave 5 |
| `8` | Slave 6 |
| `9` | Slave 7 |
| `A` | Slave 8 |
| `B` | Slave 9 |
| `C` | Slave 10 |
| `D` | Slave 11 |
| `E` | Slave 12 |
| `F` | Slave 13 |

Master battery DIP-switch meaning:

- DIP 1:
  - `1`: RS485 baud rate = `9600`
  - `0`: RS485 baud rate = `115200`
  - restart required to take effect
- DIP 2 to DIP 4:
  - set the group address `m`

`1 = up`, `0 = down`

| DIP2 | DIP3 | DIP4 | Group address `m` | Notes |
|---|---|---|---:|---|
| 0 | 0 | 0 | 0 | For a single group, the master must use this setting. |
| 1 | 0 | 0 | 1 | In multi-group mode, the first group should start at this setting. |
| 0 | 1 | 0 | 2 | - |
| 1 | 1 | 0 | 3 | - |
| 0 | 0 | 1 | 4 | - |
| 1 | 0 | 1 | 5 | - |
| 0 | 1 | 1 | 6 | - |
| 1 | 1 | 1 | 7 | - |

Battery address calculation:

```text
ADR = battery address + group address = 0x0n + 0x10*m
```

Examples:

1. Single group, slave 4:
   - `n = 5`
   - `m = 0`
   - `ADR = 0x05 + 0x10 * 0 = 0x05`

2. Multi-group, group 3, slave 6:
   - `n = 7`
   - `m = 3`
   - `ADR = 0x07 + 0x10 * 3 = 0x37`

---

# 2. Communication Protocol for System

System-level information is obtained by querying the master battery of each group.

Important notes:
- The query command is fixed.
- The destination address depends on the master battery DIP-switch group setting.
- The following commands only respond on master addresses:
  - `0x12`, `0x22`, `0x32`, `0x42`, `0x52`, `0x62`, `0x72`
- Other addresses are invalid for these system-level commands.
- If some analog values are not supported by a specific product model, `FF` is used as a placeholder.

## 2.1 Get Battery System Basic Information

### Command

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | `60H` | LENGTH | INFO | CHKSUM | EOI |

### Response

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | RTN | LENGTH | INFO | CHKSUM | EOI |

### INFO Layout

| No. | Content | Size |
|---|---|---|
| 1 | Master battery name | 10 ASCII bytes |
| 2 | Manufacturer name | 20 ASCII bytes |
| 3 | Master software version | 2 bytes |
| 4 | Number of batteries | 1 byte |
| 5 | Battery 1 barcode | 16 ASCII bytes |
| 6 | Battery 2 barcode | 16 ASCII bytes |
| `4 + N` | Battery N barcode | 16 ASCII bytes |

### Example

Send command:

```text
7E 32 30 31 32 34 36 36 30 30 30 30 30 46 44 41 42 0D
```

Receive data:

```text
7E 32 30 31 32 34 36 30 30 36 30 38 32 34 36 36 46 37 32 36 33 36 35 35 46 34 43 30 30 30 30
30 35 30 37 39 36 43 36 46 36 45 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30
30 30 30 30 30 30 30 30 30 30 30 39 30 32 33 30 33 31 33 32 33 33 33 34 33 35 33 36 33 37 33 38
33 39 36 31 36 32 36 33 36 34 36 35 36 36 33 31 33 31 33 32 33 33 33 34 33 35 33 36 33 37 33 38
33 39 36 31 36 32 36 33 36 34 36 35 36 36 45 33 35 33 0D
```

Parsed response:

| No. | Content | Raw | Meaning |
|---|---|---|---|
| 1 | Master battery name | `34 36 36 46 37 32 36 33 36 35 35 46 34 43 30 30 30 30 30 30` | `Force_L` |
| 2 | Manufacturer name | `35 30 37 39 36 43 36 46 36 45 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30 30` | `Pylon` |
| 3 | Software version | `30 30 30 39` | `0x0009` |
| 4 | Number of batteries | `30 32` | `0x02` |
| 5 | Battery 1 barcode | `33 30 33 31 33 32 33 33 33 34 33 35 33 36 33 37 33 38 33 39 36 31 36 32 36 33 36 34 36 35 36 36` | `0123456789abcdef` |
| 6 | Battery 2 barcode | `33 31 33 31 33 32 33 33 33 34 33 35 33 36 33 37 33 38 33 39 36 31 36 32 36 33 36 34 36 35 36 36` | `1123456789abcdef` |

## 2.2 Get Battery System Analog Data

### Command

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | `61H` | LENGTH | INFO | CHKSUM | EOI |

### Response

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | RTN | LENGTH | INFO | CHKSUM | EOI |

### INFO Layout

| No. | Content | Bytes | Unit / Meaning | Resolution |
|---|---|---:|---|---:|
| 1 | Total average system voltage | 2 | V | 3 decimals |
| 2 | Total system current | 2 | A | 2 decimals |
| 3 | System SOC | 1 | % | integer |
| 4 | Average cycle count | 2 | cycles | integer |
| 5 | Maximum cycle count | 2 | cycles | integer |
| 6 | Average SOH | 1 | % | integer |
| 7 | Minimum SOH | 1 | % | integer |
| 8 | Highest cell voltage | 2 | V | 3 decimals |
| 9 | Module containing highest cell voltage | 2 | Encoded as group/module address | - |
| 10 | Lowest cell voltage | 2 | V | 3 decimals |
| 11 | Module containing lowest cell voltage | 2 | Encoded as group/module address | - |
| 12 | Average cell temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 13 | Highest cell temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 14 | Module containing highest cell temperature | 2 | Encoded as group/module address | - |
| 15 | Lowest cell temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 16 | Module containing lowest cell temperature | 2 | Encoded as group/module address | - |
| 17 | Average MOSFET temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 18 | Highest MOSFET temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 19 | Module containing highest MOSFET temperature | 2 | Encoded as group/module address | - |
| 20 | Lowest MOSFET temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 21 | Module containing lowest MOSFET temperature | 2 | Encoded as group/module address | - |
| 22 | Average BMS temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 23 | Highest BMS temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 24 | Module containing highest BMS temperature | 2 | Encoded as group/module address | - |
| 25 | Lowest BMS temperature | 2 | Kelvin-based encoded temperature | 0.1 °C |
| 26 | Module containing lowest BMS temperature | 2 | Encoded as group/module address | - |

Temperature encoding:
- Encoded value = `temperature_in_celsius × 10 + 2731`
- Examples:
  - `25.5 °C -> 25.5 × 10 + 2731 = 2986`
  - `-12.4 °C -> -12.4 × 10 + 2731 = 2607`

Address example:
- `0x0304` means module address `4` inside group address `3`

### Example

Send command:

```text
7E 32 30 31 32 34 36 36 31 30 30 30 30 46 44 41 41 0D
```

Receive data:

```text
7E 32 30 31 32 34 36 30 30 38 30 36 32 32 45 35 33 36 31 41 38 36 32 30 39 44 34 30 42 37 34
36 32 36 31 30 44 42 38 30 30 33 34 30 43 42 42 30 30 31 34 30 42 41 41 30 42 42 37 30 30 33 35
30 42 39 44 30 30 31 35 30 42 41 41 30 42 42 38 30 30 33 36 30 42 39 43 30 30 31 36 30 42 41 41
30 42 42 36 30 30 33 37 30 42 39 45 30 30 31 37 45 38 36 32 0D
```

Parsed response:

| No. | Content | Raw | Meaning |
|---|---|---|---|
| 1 | Total average system voltage | `32 45 35 33` | `0x2E53 = 11.859 V` |
| 2 | Total system current | `36 31 41 38` | `0x61A8 = 25.00 A` |
| 3 | System SOC | `36 32` | `0x62 = 98%` |
| 4 | Average cycle count | `30 39 44 34` | `0x09D4 = 2516` |
| 5 | Maximum cycle count | `30 42 37 34` | `0x0B74 = 2932` |
| 6 | Average SOH | `36 32` | `0x62 = 98%` |
| 7 | Minimum SOH | `36 31` | `0x61 = 97%` |
| 8 | Highest cell voltage | `30 44 42 38` | `0x0DB8 = 3.512 V` |
| 9 | Module of highest cell voltage | `30 33 30 34` | `0x0304` |
| 10 | Lowest cell voltage | `30 43 42 42` | `0x0CBB = 3.259 V` |
| 11 | Module of lowest cell voltage | `30 31 30 34` | `0x0104` |
| 12 | Average cell temperature | `30 42 41 41` | `0x0BAA = 25.5 °C` |
| 13 | Highest cell temperature | `30 42 42 37` | `0x0BB7 = 26.8 °C` |
| 14 | Module of highest cell temperature | `30 33 30 35` | `0x0305` |
| 15 | Lowest cell temperature | `30 42 39 44` | `0x0B9D = 24.2 °C` |
| 16 | Module of lowest cell temperature | `30 31 30 35` | `0x0105` |
| 17 | Average MOSFET temperature | `30 42 41 41` | `0x0BAA = 25.5 °C` |
| 18 | Highest MOSFET temperature | `30 42 42 38` | `0x0BB8 = 26.9 °C` |
| 19 | Module of highest MOSFET temperature | `30 33 30 36` | `0x0306` |
| 20 | Lowest MOSFET temperature | `30 42 39 43` | `0x0B9C = 24.1 °C` |
| 21 | Module of lowest MOSFET temperature | `30 31 30 36` | `0x0106` |
| 22 | Average BMS temperature | `30 42 41 41` | `0x0BAA = 25.5 °C` |
| 23 | Highest BMS temperature | `30 42 42 36` | `0x0BB6 = 26.7 °C` |
| 24 | Module of highest BMS temperature | `30 33 30 37` | `0x0307` |
| 25 | Lowest BMS temperature | `30 42 39 45` | `0x0B9E = 24.3 °C` |
| 26 | Module of lowest BMS temperature | `30 31 30 37` | `0x0107` |

## 2.3 Get Battery System Status and Alarm Information

### Command

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | `62H` | LENGTH | INFO | CHKSUM | EOI |

### Response

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | RTN | LENGTH | INFO | CHKSUM | EOI |

### INFO Layout

| No. | Content | Bytes |
|---|---|---:|
| 1 | System alarm status 1 | 1 |
| 2 | System alarm status 2 | 1 |
| 3 | System protection status 1 | 1 |
| 4 | System protection status 2 | 1 |

### System Alarm Status 1

| Bit | Meaning | 0 | 1 |
|---|---|---|---|
| 7 | Module total voltage high alarm | Normal | Triggered |
| 6 | Module total voltage low alarm | Normal | Triggered |
| 5 | Cell voltage high alarm | Normal | Triggered |
| 4 | Cell voltage low alarm | Normal | Triggered |
| 3 | Cell temperature high alarm | Normal | Triggered |
| 2 | Cell temperature low alarm | Normal | Triggered |
| 1 | MOSFET high temperature alarm | Normal | Triggered |
| 0 | Cell voltage inconsistency alarm | Normal | Triggered |

### System Alarm Status 2

| Bit | Meaning | 0 | 1 |
|---|---|---|---|
| 7 | Cell temperature inconsistency alarm | Normal | Triggered |
| 6 | Charge over-current alarm | Normal | Triggered |
| 5 | Discharge over-current alarm | Normal | Triggered |
| 4 | Internal communication error | Normal | Triggered |
| 3 | Reserved | - | - |
| 2 | Reserved | - | - |
| 1 | Reserved | - | - |
| 0 | Reserved | - | - |

### System Protection Status 1

| Bit | Meaning | 0 | 1 |
|---|---|---|---|
| 7 | Module total over-voltage protection | Normal | Triggered |
| 6 | Module total under-voltage protection | Normal | Triggered |
| 5 | Cell over-voltage protection | Normal | Triggered |
| 4 | Cell under-voltage protection | Normal | Triggered |
| 3 | Cell over-temperature protection | Normal | Triggered |
| 2 | Cell under-temperature protection | Normal | Triggered |
| 1 | MOSFET over-temperature protection | Normal | Triggered |
| 0 | Reserved | - | - |

### System Protection Status 2

| Bit | Meaning | 0 | 1 |
|---|---|---|---|
| 7 | Reserved | - | - |
| 6 | Charge over-current protection | Normal | Triggered |
| 5 | Discharge over-current protection | Normal | Triggered |
| 4 | Reserved | - | - |
| 3 | System fault protection | Normal | Triggered |
| 2 | Reserved | - | - |
| 1 | Reserved | - | - |
| 0 | Reserved | - | - |

### Example

Send command:

```text
7E 32 30 31 32 34 36 36 32 30 30 30 30 46 44 41 39 0D
```

Receive data:

```text
7E 32 30 31 32 34 36 30 30 38 30 30 38 30 30 30 30 30 30 30 30 46 43 32 31 0D
```

This example corresponds to all-zero status bytes, meaning no active alarms or protections.

## 2.4 Get Battery System Charge/Discharge Management Information

### Command

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | `63H` | LENGTH | INFO | CHKSUM | EOI |

### Response

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | RTN | LENGTH | INFO | CHKSUM | EOI |

### INFO Layout

| No. | Content | Bytes | Unit | Resolution |
|---|---|---:|---|---:|
| 1 | Recommended charge voltage upper limit | 2 | V | 3 decimals |
| 2 | Recommended discharge voltage lower limit | 2 | V | 3 decimals |
| 3 | Maximum charge current | 2 | A | 1 decimal |
| 4 | Maximum discharge current | 2 | A | 1 decimal |
| 5 | Charge/discharge status | 1 | Bitfield | - |

### Charge/Discharge Status Bitfield

| Bit | Meaning | 1 | 0 |
|---|---|---|---|
| 7 | Charge enable | Charging allowed | Request stop charging |
| 6 | Discharge enable | Discharging allowed | Request stop discharging |
| 5 | Charge immediately | Immediate charge requested | Normal |
| 4 | Full charge request | Full charge requested | Normal |
| 3 | Reserved | - | - |
| 2 | Reserved | - | - |
| 1 | Reserved | - | - |
| 0 | Reserved | - | - |

### Example

Send command:

```text
7E 32 30 31 32 34 36 36 33 30 30 30 30 46 44 41 38 0D
```

Receive data:

```text
7E 32 30 31 32 34 36 30 30 38 30 30 38 44 43 44 33 35 44 43 30 30 39 43 34 30 37 45 34 42 30
46 39 38 35 0D
```

Parsed response:

| No. | Content | Raw | Meaning |
|---|---|---|---|
| 1 | Recommended charge voltage upper limit | `44 43 44 33` | `0xDCD3 = 56.531 V` |
| 2 | Recommended discharge voltage lower limit | `35 44 43 30` | `0x5DC0 = 24.00 V` |
| 3 | Maximum charge current | `30 39 43 34` | `0x09C4 = 25.0 A` |
| 4 | Maximum discharge current | `30 37 45 34` | `0x07E4 = 20.2 A` |
| 5 | Charge/discharge status | `42 30` | `0xB0 = 10110000b` |

Interpretation of `0xB0`:
- Bit 7 = `1`: charge enabled
- Bit 6 = `0`: request stop discharging
- Bit 5 = `1`: immediate charge requested
- Bit 4 = `1`: full charge requested

## 2.5 Control Battery System Shutdown

This command is intended only for systems under an energy management system.  
It will shut down one battery group.

### Command

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | `64H` | LENGTH | INFO | CHKSUM | EOI |

### Response

| No. | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|
| Bytes | 1 | 1 | 1 | 1 | 1 | 2 | LENID/2 | 2 | 1 |
| Format | SOI | VER | ADR | `46H` | RTN | LENGTH | INFO | CHKSUM | EOI |

### Example

Send command:

```text
7E 32 30 31 32 34 36 36 34 30 30 30 30 46 44 41 37 0D
```

Receive data:

```text
7E 32 30 31 32 34 36 30 30 30 30 30 30 46 44 42 31 0D
```
