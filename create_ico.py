import struct
import sys
import os

def create_ico(png_path, ico_path):
    with open(png_path, 'rb') as f:
        png_data = f.read()

    # Verify PNG signature
    if png_data[:8] != b'\x89PNG\r\n\x1a\n':
        print("Not a PNG file")
        return

    # Parse IHDR
    # Chunk length (4), Chunk type (4), Width (4), Height (4)
    # IHDR is the first chunk usually
    ihdr_offset = 8
    ihdr_len = struct.unpack('>I', png_data[ihdr_offset:ihdr_offset+4])[0]
    ihdr_type = png_data[ihdr_offset+4:ihdr_offset+8]
    if ihdr_type != b'IHDR':
        print("First chunk is not IHDR")
        return

    width = struct.unpack('>I', png_data[ihdr_offset+8:ihdr_offset+12])[0]
    height = struct.unpack('>I', png_data[ihdr_offset+12:ihdr_offset+16])[0]

    print(f"PNG Size: {width}x{height}")

    # ICO Header
    # Reserved (2), Type (2=1 for ICO), Count (2)
    ico_header = struct.pack('<HHH', 0, 1, 1)

    # ICO Entry
    # Width (1), Height (1), Palette (1), Reserved (1)
    # Planes (2), BPP (2), Size (4), Offset (4)

    w = width if width < 256 else 0
    h = height if height < 256 else 0

    # Planes=1, BPP=32 (typical for PNG in ICO)
    planes = 1
    bpp = 32
    size = len(png_data)
    offset = 6 + 16 # Header + 1 Entry

    entry = struct.pack('<BBBBHHII', w, h, 0, 0, planes, bpp, size, offset)

    with open(ico_path, 'wb') as f:
        f.write(ico_header)
        f.write(entry)
        f.write(png_data)

    print(f"Created {ico_path}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: create_ico.py <input.png> <output.ico>")
    else:
        create_ico(sys.argv[1], sys.argv[2])
