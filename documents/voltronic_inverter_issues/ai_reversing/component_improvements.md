# Pylontech RS485 Component Improvements

## Summary

The inverter requests captured in `pylontech_dump.txt` are valid Pylontech RS485 frames.

Observed behavior:
- 493 UART request frames were captured.
- All 493 frames are valid protocol frames with correct `LCHKSUM` and `CHKSUM`.
- All 493 requests use `CID2 = 0x4F`.
- `0x4F` is `Get protocol version` in both protocol documents.
- No outbound UART frames were captured.

Conclusion:
- The inverter appears to be stuck retrying the protocol-version query because the current component does not implement `0x4F`.

## Root Cause In The Component

The current component only handles commands `61`, `62`, and `63`.

Relevant logic:
- `route_frame_request_()` only dispatches `61`, `62`, and `63`.
- Any other `CID2` falls into `Ignoring unknown command`.
- `write_str()` is only called inside `handle_command_61_()`, `handle_command_62_()`, and `handle_command_63_()`.

Impact:
- When the inverter sends `4F`, the component logs `Ignoring unknown command: 4F` and does not transmit anything.
- Because no bytes are written to the UART, there are no `>>>` lines in the UART debug output.

## What The Protocol Says About `0x4F`

From `pylontech_rs485_protocol_v3_3.md`:
- Request `CID2 = 0x4F`
- Request `LENID = 0`
- Request `INFO = empty`
- Request `VER` is arbitrary and may be ignored by the battery
- Response `CID2 = RTN`
- Response `VER` contains the protocol version
- Example meaning: protocol version `V2.1` is reported as `0x21`

This means the current request value `VER = 0x00` from the inverter should not be treated as the protocol version to return.

## Recommended Changes

### 1. Implement command `0x4F`

Add a `handle_command_4f_()` response path.

Recommended response shape:
- `SOI = ~`
- `VER = 0x20` initially
- `ADR = request ADR`
- `CID1 = 0x46`
- `CID2 = 0x00`
- `LENGTH = 0x0000`
- `INFO = empty`
- valid `CHKSUM`
- `EOI = \r`

Why `0x20` first:
- the component already uses `PROTOCOL_VERSION = "20"`
- the protocol says the response `VER` should carry the version
- the request `VER = 0x00` is arbitrary and should not be echoed for this command

Fallback if needed:
- If the inverter still does not advance after a valid `0x20` response, test `0x21` next.

### 2. Stop hardcoding response address `02`

The component currently always responds with address `02`.

Observed requests target these master addresses:
- `0x02`
- `0x12`
- `0x22`
- `0x32`
- `0x42`
- `0x52`

Recommended behavior:
- Echo the request `ADR` in the response.

### 3. Parse the full request header

Instead of only extracting `CID2`, parse and retain:
- `VER`
- `ADR`
- `CID1`
- `CID2`

This enables:
- proper address echoing
- cleaner validation
- future support for additional commands

### 4. Keep `61`, `62`, and `63` aligned with the same framing strategy

Once request parsing is improved, reuse the same response builder for:
- `0x4F`
- `0x61`
- `0x62`
- `0x63`

Recommended common behavior:
- use request `ADR` in the response
- use component protocol version in `VER`
- keep `CID2 = 00` for normal responses
- compute `LENGTH` and `CHKSUM` exactly once in a shared helper

## Practical Expectation After The Fix

If `0x4F` is implemented correctly, the most likely next behavior is:
- the inverter stops repeating only `4F`
- the UART debug log starts showing outbound `>>>` response frames
- the inverter moves on to request operational data such as `61`, `62`, or `63`

## Suggested Implementation Order

1. Add request parsing for `VER` and `ADR`
2. Add `handle_command_4f_()`
3. Echo request `ADR` in all responses
4. Refactor response construction into a shared helper
5. Re-test and confirm the inverter progresses beyond protocol-version requests
