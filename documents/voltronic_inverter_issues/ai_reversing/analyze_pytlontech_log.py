from __future__ import annotations

import argparse
import re
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


UART_RE = re.compile(
    r"^\[(?P<ts>[^\]]+)\]\[D\]\[uart_debug:\d+\]:\s+(?P<dir><<<|>>>)\s+(?P<frame>[0-9A-F:]+)\s*$"
)
COMPONENT_RE = re.compile(
    r"^\[(?P<ts>[^\]]+)\]\[(?P<level>[DWI])\]\[pylontech_rs485:\d+\]:\s+(?P<message>.*)$"
)


CID2_NAMES = {
    0x42: "Get analog values, fixed-point",
    0x44: "Get alarm info",
    0x47: "Get system parameters, fixed-point",
    0x4F: "Get protocol version",
    0x51: "Get manufacturer info",
    0x60: "Get battery system basic info",
    0x61: "Get battery system analog data",
    0x62: "Get battery system alarm info",
    0x63: "Get system charge/discharge management info",
    0x64: "System shutdown",
    0x92: "Get charge/discharge management info",
    0x93: "Get battery serial number",
    0x94: "Set charge/discharge management info",
    0x95: "Turn off",
    0x96: "Get firmware version",
}

RESPONSE_CODES = {
    0x00: "Normal",
    0x01: "Version error",
    0x02: "CHKSUM error",
    0x03: "LCHKSUM error",
    0x04: "Invalid CID2",
    0x05: "Command format error",
    0x06: "Invalid data",
    0x90: "Address error",
    0x91: "Communication error",
}

MASTER_ADDRESSES = {0x02 + (0x10 * group) for group in range(8)}
SYSTEM_LEVEL_CIDS = {0x60, 0x61, 0x62, 0x63, 0x64}


@dataclass(frozen=True)
class RawFrame:
    timestamp: str
    direction: str
    raw: bytes
    line_number: int


@dataclass(frozen=True)
class DecodedFrame:
    timestamp: str
    direction: str
    line_number: int
    frame_text: str
    ver: int | None
    adr: int | None
    cid1: int | None
    cid2: int | None
    length_word: int | None
    lenid: int | None
    lchksum: int | None
    lchksum_ok: bool
    info_text: str | None
    checksum_wire: int | None
    checksum_calc: int | None
    checksum_ok: bool
    frame_ok: bool
    issues: tuple[str, ...]


def calc_length_checksum(lenid: int) -> int:
    nibble_sum = ((lenid >> 8) & 0xF) + ((lenid >> 4) & 0xF) + (lenid & 0xF)
    return (~nibble_sum + 1) & 0xF


def calc_ascii_checksum(content_without_soi_eoi_and_checksum: bytes) -> int:
    total = sum(content_without_soi_eoi_and_checksum) & 0xFFFF
    return (~total + 1) & 0xFFFF


def parse_uart_frames(text: str) -> list[RawFrame]:
    frames: list[RawFrame] = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        match = UART_RE.match(line)
        if not match:
            continue
        frames.append(
            RawFrame(
                timestamp=match.group("ts"),
                direction=match.group("dir"),
                raw=bytes.fromhex(match.group("frame").replace(":", " ")),
                line_number=line_number,
            )
        )
    return frames


def parse_component_messages(text: str) -> Counter[str]:
    messages: Counter[str] = Counter()
    for line in text.splitlines():
        match = COMPONENT_RE.match(line)
        if match:
            messages[match.group("message")] += 1
    return messages


def decode_frame(frame: RawFrame) -> DecodedFrame:
    issues: list[str] = []

    if len(frame.raw) < 10:
        return DecodedFrame(
            timestamp=frame.timestamp,
            direction=frame.direction,
            line_number=frame.line_number,
            frame_text=frame.raw.hex(":").upper(),
            ver=None,
            adr=None,
            cid1=None,
            cid2=None,
            length_word=None,
            lenid=None,
            lchksum=None,
            lchksum_ok=False,
            info_text=None,
            checksum_wire=None,
            checksum_calc=None,
            checksum_ok=False,
            frame_ok=False,
            issues=(f"too short for protocol frame ({len(frame.raw)} bytes)",),
        )

    if frame.raw[0] != 0x7E:
        issues.append(f"bad SOI 0x{frame.raw[0]:02X}")
    if frame.raw[-1] != 0x0D:
        issues.append(f"bad EOI 0x{frame.raw[-1]:02X}")

    body = frame.raw[1:-1]
    try:
        body_text = body.decode("ascii")
    except UnicodeDecodeError:
        return DecodedFrame(
            timestamp=frame.timestamp,
            direction=frame.direction,
            line_number=frame.line_number,
            frame_text=frame.raw.hex(":").upper(),
            ver=None,
            adr=None,
            cid1=None,
            cid2=None,
            length_word=None,
            lenid=None,
            lchksum=None,
            lchksum_ok=False,
            info_text=None,
            checksum_wire=None,
            checksum_calc=None,
            checksum_ok=False,
            frame_ok=False,
            issues=tuple(issues + ["body is not ASCII"]),
        )

    if len(body_text) < 16:
        issues.append(f"ASCII body too short ({len(body_text)} chars)")
        return DecodedFrame(
            timestamp=frame.timestamp,
            direction=frame.direction,
            line_number=frame.line_number,
            frame_text=body_text,
            ver=None,
            adr=None,
            cid1=None,
            cid2=None,
            length_word=None,
            lenid=None,
            lchksum=None,
            lchksum_ok=False,
            info_text=None,
            checksum_wire=None,
            checksum_calc=None,
            checksum_ok=False,
            frame_ok=False,
            issues=tuple(issues),
        )

    try:
        ver = int(body_text[0:2], 16)
        adr = int(body_text[2:4], 16)
        cid1 = int(body_text[4:6], 16)
        cid2 = int(body_text[6:8], 16)
        length_word = int(body_text[8:12], 16)
        checksum_wire = int(body_text[-4:], 16)
    except ValueError:
        return DecodedFrame(
            timestamp=frame.timestamp,
            direction=frame.direction,
            line_number=frame.line_number,
            frame_text=body_text,
            ver=None,
            adr=None,
            cid1=None,
            cid2=None,
            length_word=None,
            lenid=None,
            lchksum=None,
            lchksum_ok=False,
            info_text=None,
            checksum_wire=None,
            checksum_calc=None,
            checksum_ok=False,
            frame_ok=False,
            issues=tuple(issues + ["one or more fields are not valid hex ASCII"]),
        )

    info_text = body_text[12:-4]
    lenid = length_word & 0x0FFF
    lchksum = (length_word >> 12) & 0xF
    lchksum_calc = calc_length_checksum(lenid)
    lchksum_ok = lchksum == lchksum_calc
    checksum_calc = calc_ascii_checksum(body[:-4])
    checksum_ok = checksum_wire == checksum_calc

    if len(info_text) != lenid:
        issues.append(f"LENID says {lenid} ASCII bytes, frame carries {len(info_text)}")
    if not lchksum_ok:
        issues.append(f"bad LCHKSUM wire=0x{lchksum:X} calc=0x{lchksum_calc:X}")
    if not checksum_ok:
        issues.append(
            f"bad CHKSUM wire=0x{checksum_wire:04X} calc=0x{checksum_calc:04X}"
        )
    if cid1 != 0x46:
        issues.append(f"unexpected CID1 0x{cid1:02X}")
    if adr in {0x00, 0xFF}:
        issues.append(f"reserved address 0x{adr:02X}")
    if cid2 in SYSTEM_LEVEL_CIDS and adr not in MASTER_ADDRESSES:
        issues.append(
            f"system-level CID2 0x{cid2:02X} sent to non-master address 0x{adr:02X}"
        )

    frame_ok = not issues

    return DecodedFrame(
        timestamp=frame.timestamp,
        direction=frame.direction,
        line_number=frame.line_number,
        frame_text=body_text,
        ver=ver,
        adr=adr,
        cid1=cid1,
        cid2=cid2,
        length_word=length_word,
        lenid=lenid,
        lchksum=lchksum,
        lchksum_ok=lchksum_ok,
        info_text=info_text,
        checksum_wire=checksum_wire,
        checksum_calc=checksum_calc,
        checksum_ok=checksum_ok,
        frame_ok=frame_ok,
        issues=tuple(issues),
    )


def describe_address(adr: int | None) -> str:
    if adr is None:
        return "n/a"
    group = adr >> 4
    position = adr & 0x0F
    if position == 0x02:
        return f"master group={group}"
    return f"group={group} position=0x{position:X}"


def describe_cid2(decoded: DecodedFrame) -> str:
    if decoded.cid2 is None:
        return "n/a"
    if decoded.direction == ">>>":
        return RESPONSE_CODES.get(
            decoded.cid2, f"Unknown response 0x{decoded.cid2:02X}"
        )
    return CID2_NAMES.get(decoded.cid2, f"Unknown command 0x{decoded.cid2:02X}")


def print_report(
    decoded_frames: list[DecodedFrame], component_messages: Counter[str]
) -> None:
    print("Pylontech RS485 request inventory")
    print(f"  uart_debug frames: {len(decoded_frames)}")
    print(
        f"  rx frames: {sum(1 for frame in decoded_frames if frame.direction == '<<<')}"
    )
    print(
        f"  tx frames: {sum(1 for frame in decoded_frames if frame.direction == '>>>')}"
    )
    print(
        f"  structurally valid: {sum(1 for frame in decoded_frames if frame.frame_ok)}"
    )
    print(f"  invalid: {sum(1 for frame in decoded_frames if not frame.frame_ok)}")
    print()

    unique_frames: dict[str, DecodedFrame] = {}
    counts: Counter[str] = Counter()
    for frame in decoded_frames:
        counts[frame.frame_text] += 1
        unique_frames.setdefault(frame.frame_text, frame)

    print("Unique frames")
    for frame_text, count in sorted(counts.items()):
        frame = unique_frames[frame_text]
        print(f"- {frame_text} x{count}")
        print(f"    line: {frame.line_number} direction: {frame.direction}")
        print(
            f"    VER=0x{frame.ver:02X} ADR=0x{frame.adr:02X} ({describe_address(frame.adr)})"
        )
        print(
            f"    CID1=0x{frame.cid1:02X} CID2=0x{frame.cid2:02X} ({describe_cid2(frame)})"
        )
        print(
            f"    LENGTH=0x{frame.length_word:04X} LENID={frame.lenid} LCHKSUM={'ok' if frame.lchksum_ok else 'bad'}"
        )
        print(
            f"    CHKSUM wire=0x{frame.checksum_wire:04X} calc=0x{frame.checksum_calc:04X} ok={frame.checksum_ok}"
        )
        if frame.info_text is not None:
            print(f"    INFO='{frame.info_text}'")
        if frame.issues:
            print(f"    issues: {'; '.join(frame.issues)}")
        else:
            print("    issues: none")
        print()

    if component_messages:
        print("Component log messages")
        for message, count in component_messages.most_common():
            print(f"  {message} x{count}")
        print()

    known_commands = Counter(
        frame.cid2 for frame in decoded_frames if frame.cid2 is not None
    )
    print("CID2 summary")
    for cid2, count in sorted(known_commands.items()):
        print(
            f"  0x{cid2:02X} x{count} {CID2_NAMES.get(cid2, RESPONSE_CODES.get(cid2, 'unknown'))}"
        )


def build_parser() -> argparse.ArgumentParser:
    default_log = Path(__file__).resolve().parents[2] / "dumps" / "pylontech_dump.txt"
    parser = argparse.ArgumentParser(
        description="Analyze ESPHome uart_debug Pylontech RS485 frames against the Pylon protocol frame format."
    )
    parser.add_argument(
        "logfile",
        nargs="?",
        default=str(default_log),
        help="Path to the ESPHome dump containing uart_debug Pylontech frames",
    )
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    text = Path(args.logfile).read_text(encoding="utf-8")
    raw_frames = parse_uart_frames(text)
    decoded_frames = [decode_frame(frame) for frame in raw_frames]
    component_messages = parse_component_messages(text)
    print_report(decoded_frames, component_messages)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
