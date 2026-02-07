import struct

def create_bmp(filename, width, height, r, g, b):
    # File Header (14)
    # BM, Size, Res, Res, Offset
    # Row size must be multiple of 4 bytes
    row_size = (width * 3 + 3) & ~3
    image_size = row_size * height
    filesize = 14 + 40 + image_size
    offset = 54
    file_header = struct.pack('<2sIHHI', b'BM', filesize, 0, 0, offset)

    # DIB Header (40 - BITMAPINFOHEADER)
    # Size, W, H, Planes, BPP, Compression, ImageSize, XRes, YRes, ClrUsed, ClrImportant
    dib_header = struct.pack('<IIIHHIIIIII', 40, width, height, 1, 24, 0, image_size, 2835, 2835, 0, 0)

    # Pixel Data (BGR)
    # BMP is stored bottom-to-top usually, but top-to-bottom if height is negative (but here we use positive)
    # We'll just fill with solid color so direction doesn't matter

    pixel = bytes([b, g, r])
    row_pixels = pixel * width
    padding_len = row_size - (width * 3)
    padding = b'\x00' * padding_len

    with open(filename, 'wb') as f:
        f.write(file_header)
        f.write(dib_header)
        for _ in range(height):
            f.write(row_pixels)
            f.write(padding)

create_bmp('extras/package/win32/NSIS/vlc_branding.bmp', 164, 314, 40, 40, 40)
print("Created vlc_branding.bmp")
