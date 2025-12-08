import asyncio
from bleak import BleakClient, BleakScanner

DEVICE_NAME = "I-KE-V3"
CHAR_LED = "8EC91004-F315-4F60-9FB8-838830DAEA50"

MODE_STATIC = 0x20
MODE_BREATHE = 0x21
MODE_BLINK = 0x11


def clamp(x: int) -> int:
    return max(0, min(255, x))


def build_packet(
    mode_byte: int, r: int, g: int, b: int, extra: int = 0x98, tail_byte: int = 0x00
) -> bytes:
    r = clamp(r)
    g = clamp(g)
    b = clamp(b)
    extra = clamp(extra)

    return bytes(
        [mode_byte, 
        0, 0, 0, 0, 0, 0, 0, 0, 0,
        r, g, b,
        extra,
        0, 0, 0, 0, 1, tail_byte]
    )


async def send_packet(client, pkt: bytes):
    print("Sending:", pkt.hex().upper())
    await client.write_gatt_char(CHAR_LED, pkt, response=True)
    val = await client.read_gatt_char(CHAR_LED)
    print("Response:", val.hex().upper())


async def set_static(client, r: int, g: int, b: int, brightness: int = 0xFF):
    pkt = build_packet(MODE_STATIC, r, g, b, extra=brightness)
    await send_packet(client, pkt)


async def set_breathe(client, r: int, g: int, b: int, speed: int = 0x03):
    pkt = build_packet(MODE_BREATHE, r, g, b, extra=speed)
    await send_packet(client, pkt)


async def set_blink(client, r: int, g: int, b: int, speed: int = 0x03):
    pkt = build_packet(MODE_BLINK, r, g, b, extra=speed)
    await send_packet(client, pkt)


async def find_device():
    print("Scanning...")
    devices = await BleakScanner.discover(timeout=5.0)
    for d in devices:
        if DEVICE_NAME in (d.name or ""):
            print("Found:", d.address)
            return d.address
    raise RuntimeError("Device not found")


async def main():
    address = await find_device()
    async with BleakClient(address) as client:
        print("Connected:", client.is_connected)
        print("Commands:")
        print("  static R G B [brightness]")
        print("  breathe R G B [speed]")
        print("  blink R G B [speed]")
        print("  packet HEX_STRING")
        print("  off")
        print("  exit\n")

        while True:
            line = input(">>> ").strip()
            if not line:
                continue
            parts = line.split()
            cmd = parts[0].lower()

            if cmd == "exit":
                break

            if cmd == "off":
                await set_static(client, 0, 0, 0, brightness=0x00)
                continue

            if cmd == "static" and 4 <= len(parts) <= 5:
                try:
                    r = int(parts[1])
                    g = int(parts[2])
                    b = int(parts[3])
                    brightness = int(parts[4]) if len(parts) == 5 else 0xFF
                except ValueError:
                    print("Usage: static R G B [brightness]")
                    continue
                await set_static(client, r, g, b, brightness)
                continue

            if cmd == "breathe" and 4 <= len(parts) <= 5:
                try:
                    r = int(parts[1])
                    g = int(parts[2])
                    b = int(parts[3])
                    speed = int(parts[4]) if len(parts) == 5 else 0x03
                except ValueError:
                    print("Usage: breathe R G B [speed]")
                    continue
                await set_breathe(client, r, g, b, speed)
                continue
            if cmd == "blink" and 4 <= len(parts) <= 5:
                try:
                    r = int(parts[1])
                    g = int(parts[2])
                    b = int(parts[3])
                    speed = int(parts[4]) if len(parts) == 5 else 0x03
                except ValueError:
                    print("Usage: blink R G B [speed]")
                    continue
                await set_blink(client, r, g, b, speed)
                continue
            if cmd == "packet" and len(parts) == 2:
                hexstr = parts[1].replace(" ", "").replace("-", "").replace("0x", "")
                try:
                    pkt = bytes.fromhex(hexstr)
                except ValueError:
                    print("Invalid hex string.")
                    continue
                await send_packet(client, pkt)
                continue

            print("Unknown command.")


if __name__ == "__main__":
    asyncio.run(main())
