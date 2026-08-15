# Pylontech RS485 Request Check

Analyzed with:
- `./analyze_pylontech_log.py`
- `./analysis_output.txt`

Source material:
- `../voltronic_inverter_gateway_logs.txt`
- `/documents/RS485 protocol/3.5/transcription.md`

## Result

The bytes sent by the inverter match the Pylontech RS485 protocol framing shown in the spec.

What was verified from the dump:
- all 493 captured UART frames are valid protocol frames
- all 493 frames have valid `LCHKSUM`
- all 493 frames have valid `CHKSUM`
- all 493 frames use `CID1 = 0x46`, which matches the battery-data class in the spec
- all 493 frames use `CID2 = 0x4F`, which the spec defines as `Get protocol version`
- the polled addresses are `0x02`, `0x12`, `0x22`, `0x32`, `0x42`, `0x52`, which are valid master addresses under the documented address formula
- all observed requests have `LENGTH = 0x0000`, which is correct for an empty `INFO` field

Representative request:

```text
7E 30 30 30 32 34 36 34 46 30 30 30 30 46 44 39 41 0D
```

ASCII payload:

```text
~0002464F0000FD9A\r
```

Decoded fields:
- `VER = 0x00`
- `ADR = 0x02`
- `CID1 = 0x46`
- `CID2 = 0x4F`
- `LENGTH = 0x0000`
- `INFO = empty`
- `CHKSUM = 0xFD9A`

## Important caveat

The log also shows the local ESPHome component repeatedly printing `Ignoring unknown command: 4F`.

That is a mismatch between the current component implementation and the protocol document, because the spec explicitly lists `0x4F` as a valid command. It does not indicate the inverter request is malformed.

## Limits of this capture

- No `>>>` UART responses were captured in this dump.
- This means the request side can be validated fully, but the battery response behavior cannot be checked from this file.
