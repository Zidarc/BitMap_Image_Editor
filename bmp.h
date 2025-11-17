//header file for BMP will store in the constructs normally used

#include <stdint.h>
// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/f8573df3-a44a-4a50-b070-ac4c3aa78e3c
// A WORD is a 16-bit unsigned integer (range: 0 through 65535 decimal). Because a WORD is unsigned, its first bit (Most Significant Bit (MSB)) is not reserved for signing.
typedef uint16_t WORD;

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/262627d8-3418-4627-9218-4ffe110850b2
// A DWORD is a 32-bit unsigned integer (range: 0 through 4294967295 decimal). Because a DWORD is unsigned, its first bit (Most Significant Bit (MSB)) is not reserved for signing.
typedef uint32_t DWORD;

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/29d44d70-382f-4998-9d76-8a1fe93e445c
// A LONG is a 32-bit signed integer, in twos-complement format (range: –2147483648 through 2147483647 decimal). The first bit (Most Significant Bit (MSB)) is the signing bit.
typedef int32_t LONG;

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/d7edc080-e499-4219-a837-1bc40b64bb04
// A BYTE is an 8-bit unsigned value that corresponds to a single octet in a network protocol.
typedef uint8_t BYTE;

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader?redirectedfrom=MSDN
// The BITMAPFILEHEADER structure contains information about the type, size, and layout of a file that contains a DIB.
#pragma pack(push, 1) // Ensure 1-byte packing
typedef struct {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
}
BITMAPFILEHEADER;
#pragma pack(pop)

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
// The BITMAPINFOHEADER structure contains information about the dimensions and color format of a device-independent bitmap (DIB).
#pragma pack(push, 1)
typedef struct {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
}
BITMAPINFOHEADER;
#pragma pack(pop)

// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbtriple
// The RGBTRIPLE structure describes a color consisting of relative intensities of red, green, and blue. The bmciColors member of the BITMAPCOREINFO structure consists of an array of RGBTRIPLE structures.
#pragma pack(push, 1)
typedef struct {
  BYTE rgbtBlue;
  BYTE rgbtGreen;
  BYTE rgbtRed;
} RGBTRIPLE;
#pragma pack(pop)