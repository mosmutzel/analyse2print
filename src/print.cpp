/**
 * print.cpp - Niimbot B1 Printer Implementation
 *
 * This module implements all printing operations for the Niimbot B1 printer
 * including bitmap generation, label printing, and printer communication.
 */

#include "print.h"
#include "display.h"
#include "lvgl.h"
#include "ui/custom_fonts.h"

// ============================================================================
// Protocol Constants
// ============================================================================

#define PACKET_HEADER_1 0x55
#define PACKET_HEADER_2 0x55
#define PACKET_FOOTER_1 0xAA
#define PACKET_FOOTER_2 0xAA

// Command codes
#define CMD_CONNECT        0xDC
#define CMD_HEARTBEAT      0xDC
#define CMD_SET_DENSITY    0x21
#define CMD_SET_LABEL_TYPE 0x23
#define CMD_PRINT_START    0x01
#define CMD_PAGE_START     0x03
#define CMD_SET_PAGE_SIZE  0x13
#define CMD_PRINT_EMPTY_ROW 0x84
#define CMD_PRINT_BITMAP_ROW 0x85
#define CMD_PAGE_END       0xE3
#define CMD_PRINT_END      0xF3

// Label types
#define LABEL_WITH_GAPS    1
#define LABEL_CONTINUOUS   2

// ============================================================================
// Global Variables
// ============================================================================

NimBLERemoteCharacteristic* pCharacteristic = nullptr;
uint8_t bitmapBuffer[MAX_HEIGHT][MAX_WIDTH_BYTES];
uint8_t responseBuffer[256];
int responseLength = 0;
volatile bool responseReceived = false;

// Print buffer for Core 1 (separate from rendering buffer)
static uint8_t printBuffer[MAX_HEIGHT][MAX_WIDTH_BYTES];

// Multi-core variables
TaskHandle_t printTaskHandle = nullptr;
static QueueHandle_t printQueue = nullptr;
static SemaphoreHandle_t bitmapMutex = nullptr;
volatile bool printerBusy = false;

// ============================================================================
// Simple 5x7 Font Data
// ============================================================================

const uint8_t font5x7[][5] PROGMEM = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (idx 0)
  {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A (idx 1)
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7F, 0x09, 0x09, 0x01, 0x01}, // F
  {0x3E, 0x41, 0x41, 0x51, 0x32}, // G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
  {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z (idx 26)
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0 (idx 27)
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9 (idx 36)
  {0x23, 0x13, 0x08, 0x64, 0x62}, // % (idx 37)
  {0x00, 0x60, 0x60, 0x00, 0x00}, // . (idx 38)
  {0x60, 0x10, 0x08, 0x04, 0x03}, // / (idx 39)
  {0x7C, 0x04, 0x78, 0x04, 0x78}, // m lowercase (idx 40)
};

// ============================================================================
// Packet Functions
// ============================================================================

int buildPacket(uint8_t* buffer, uint8_t command, uint8_t* data, int dataLength) {
  int index = 0;
  buffer[index++] = PACKET_HEADER_1;
  buffer[index++] = PACKET_HEADER_2;
  buffer[index++] = command;
  buffer[index++] = (uint8_t)dataLength;

  uint8_t checksum = command ^ (uint8_t)dataLength;
  for (int i = 0; i < dataLength; i++) {
    buffer[index++] = data[i];
    checksum ^= data[i];
  }
  buffer[index++] = checksum;
  buffer[index++] = PACKET_FOOTER_1;
  buffer[index++] = PACKET_FOOTER_2;

  return index;
}

// ============================================================================
// Communication Functions
// ============================================================================

bool sendPacketRaw(uint8_t command, uint8_t* data, int dataLength) {
  if (!pCharacteristic) {
    displayDebug("sendPacket: no characteristic!");
    return false;
  }

  uint8_t packet[256];
  int packetLength = buildPacket(packet, command, data, dataLength);

  // IMPORTANT: Connect command has special prefix byte 0x03
  uint8_t finalPacket[257];
  int finalLength = packetLength;

  if (command == CMD_CONNECT) {
    finalPacket[0] = 0x03;  // Special prefix for Connect command
    memcpy(finalPacket + 1, packet, packetLength);
    finalLength = packetLength + 1;
  } else {
    memcpy(finalPacket, packet, packetLength);
  }

  return pCharacteristic->writeValue(finalPacket, finalLength, false);
}

bool sendAndWait(uint8_t command, uint8_t* data, int dataLength, int timeoutMs = 500) {
  responseReceived = false;
  if (!sendPacketRaw(command, data, dataLength)) return false;

  unsigned long start = millis();
  while (!responseReceived && (millis() - start) < timeoutMs) {
    delay(10);
  }
  return responseReceived;
}

// ============================================================================
// Printer Commands
// ============================================================================

bool connectToPrinter() {
  uint8_t data[] = {0x01};
  return sendAndWait(CMD_CONNECT, data, 1);
}

void sendHeartbeat() {
  uint8_t data[] = {0x01};
  sendAndWait(CMD_HEARTBEAT, data, 1);
}

bool setDensity(uint8_t density) {
  uint8_t data[] = {density};
  return sendAndWait(CMD_SET_DENSITY, data, 1);
}

bool setLabelType(uint8_t type) {
  uint8_t data[] = {type};
  return sendAndWait(CMD_SET_LABEL_TYPE, data, 1);
}

bool printStart() {
  // Format: totalPages(2 bytes big-endian), 0, 0, 0, 0, pageColor
  uint8_t data[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};  // 1 page
  return sendAndWait(CMD_PRINT_START, data, 7);
}

bool pageStart() {
  uint8_t data[] = {0x01};
  return sendAndWait(CMD_PAGE_START, data, 1);
}

bool setPageSize(uint16_t w, uint16_t h) {
  uint8_t data[] = {
    (uint8_t)(h >> 8), (uint8_t)(h & 0xFF),
    (uint8_t)(w >> 8), (uint8_t)(w & 0xFF),
    0x00, 0x01
  };
  return sendAndWait(CMD_SET_PAGE_SIZE, data, 6);
}

bool pageEnd() {
  uint8_t data[] = {0x01};
  return sendAndWait(CMD_PAGE_END, data, 1);
}

bool printEnd() {
  uint8_t data[] = {0x01};
  return sendAndWait(CMD_PRINT_END, data, 1);
}

bool printEmptyRows(uint16_t pos, uint8_t repeats) {
  uint8_t data[] = {(uint8_t)(pos >> 8), (uint8_t)(pos & 0xFF), repeats};
  return sendPacketRaw(CMD_PRINT_EMPTY_ROW, data, 3);
}

bool isRowEmpty(uint8_t* rowData, int len) {
  for (int i = 0; i < len; i++) {
    if (rowData[i] != 0xFF) return false;
  }
  return true;
}

bool printBitmapRow(uint16_t row, uint8_t* rowData, int len, uint8_t repeatCount) {
  uint8_t packet[256];
  int idx = 0;

  // Row number (2 bytes, High byte first)
  packet[idx++] = (uint8_t)((row >> 8) & 0xFF);
  packet[idx++] = (uint8_t)(row & 0xFF);

  // Black pixel count (3 bytes) - use 0,0,0 for "total mode"
  packet[idx++] = 0x00;
  packet[idx++] = 0x00;
  packet[idx++] = 0x00;

  // Repeat count (1 byte)
  packet[idx++] = repeatCount;

  // Copy bitmap data - INVERT because printer expects 1=black, 0=white
  for (int i = 0; i < len; i++) {
    packet[idx++] = ~rowData[i];
  }

  return sendPacketRaw(CMD_PRINT_BITMAP_ROW, packet, idx);
}

// ============================================================================
// Graphics Functions
// ============================================================================

void clearBitmap() {
  memset(bitmapBuffer, 0xFF, sizeof(bitmapBuffer));
}

void setPixel(int x, int y) {
  if (x < 0 || x >= labelWidth || y < 0 || y >= labelHeight) return;
  bitmapBuffer[y][x / 8] &= ~(1 << (7 - (x % 8)));
}

void drawChar(int x, int y, char c, int scale) {
  int idx = -1;
  if (c == ' ') idx = 0;
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 1;
  else if (c == 'm') idx = 40;
  else if (c >= 'a' && c <= 'z') idx = c - 'a' + 1;
  else if (c >= '0' && c <= '9') idx = c - '0' + 27;
  else if (c == '%') idx = 37;
  else if (c == '.') idx = 38;
  else if (c == '/') idx = 39;
  else return;

  for (int col = 0; col < 5; col++) {
    uint8_t line = pgm_read_byte(&font5x7[idx][col]);
    for (int row = 0; row < 7; row++) {
      if (line & (1 << row)) {
        for (int sy = 0; sy < scale; sy++) {
          for (int sx = 0; sx < scale; sx++) {
            setPixel(x + col * scale + sx, y + row * scale + sy);
          }
        }
      }
    }
  }
}

void drawText(int x, int y, const char* text, int scale) {
  while (*text) {
    drawChar(x, y, *text, scale);
    x += 6 * scale;
    text++;
  }
}

void drawRect(int x, int y, int w, int h) {
  for (int i = x; i < x + w; i++) {
    setPixel(i, y);
    setPixel(i, y + h - 1);
  }
  for (int j = y; j < y + h; j++) {
    setPixel(x, j);
    setPixel(x + w - 1, j);
  }
}

void fillRect(int x, int y, int w, int h) {
  for (int j = y; j < y + h; j++) {
    for (int i = x; i < x + w; i++) {
      setPixel(i, j);
    }
  }
}

void drawLine(int x1, int y1, int x2, int y2) {
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    setPixel(x1, y1);
    if (x1 == x2 && y1 == y2) break;
    int e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x1 += sx; }
    if (e2 < dx) { err += dx; y1 += sy; }
  }
}

// ============================================================================
// LVGL Montserrat Font Rendering
// ============================================================================

// Get Montserrat font for a given size
// Uses fonts enabled in lv_conf.h: 12, 14, 16, 18, 20, 24, 32, 48
static const lv_font_t* getFontForSize(int size) {
  if (size <= 12) return &lv_font_montserrat_12;
  if (size <= 14) return &lv_font_montserrat_14;
  if (size <= 16) return &lv_font_montserrat_16;
  if (size <= 18) return &lv_font_montserrat_18;
  if (size <= 20) return &lv_font_montserrat_20;
  if (size <= 24) return &lv_font_montserrat_24;
  if (size <= 32) return &lv_font_montserrat_32;
  if (size <= 48) return &lv_font_montserrat_48;
  // Custom Montserrat SemiBold fonts for larger sizes
  if (size <= 56) return &MONTSERRAT_SemiBolt_56;
  if (size <= 64) return &MONTSERRAT_SemiBolt_64;
  if (size <= 72) return &MONTSERRAT_SemiBolt_72;
  return &MONTSERRAT_SemiBolt_72;  // Maximum available size
}

// Calculate text width using LVGL font metrics (with scaling for sizes > 72)
static int getTextWidthMontserrat(const char* text, int fontSize) {
  // For sizes > 72, use 72px font with scaling
  int baseSize = (fontSize > 72) ? 72 : fontSize;
  float scale = (fontSize > 72) ? (float)fontSize / 72.0f : 1.0f;

  const lv_font_t* font = getFontForSize(baseSize);
  int width = 0;

  while (*text) {
    uint32_t letter = (uint8_t)*text;
    lv_font_glyph_dsc_t glyph_dsc;
    bool has_glyph = lv_font_get_glyph_dsc(font, &glyph_dsc, letter, 0);
    if (has_glyph) {
      width += (int)(glyph_dsc.adv_w * scale);
    }
    text++;
  }
  return width;
}

// Draw text using LVGL Montserrat font (uses LVGL API for proper decompression)
// fontSize: actual pixel height of font (e.g., 12, 24, 36, 48, 56, 64, 72, 80)
// For sizes > 72, the 72px font is scaled up using nearest-neighbor interpolation
void drawTextMontserrat(int x, int y, const char* text, int fontSize) {
  // For sizes > 72, use 72px font with scaling
  int baseSize = (fontSize > 72) ? 72 : fontSize;
  float scale = (fontSize > 72) ? (float)fontSize / 72.0f : 1.0f;

  const lv_font_t* font = getFontForSize(baseSize);

  while (*text) {
    uint32_t letter = (uint8_t)*text;

    // Get glyph descriptor using LVGL API
    lv_font_glyph_dsc_t glyph_dsc;
    bool has_glyph = lv_font_get_glyph_dsc(font, &glyph_dsc, letter, 0);

    if (!has_glyph || glyph_dsc.box_w == 0 || glyph_dsc.box_h == 0) {
      // Skip unknown character or space, advance cursor
      x += (int)(glyph_dsc.adv_w * scale);
      text++;
      continue;
    }

    // Get bitmap using LVGL API (handles decompression)
    const uint8_t* bitmap = lv_font_get_glyph_bitmap(font, letter);
    if (!bitmap) {
      x += (int)(glyph_dsc.adv_w * scale);
      text++;
      continue;
    }

    int gw = glyph_dsc.box_w;
    int gh = glyph_dsc.box_h;
    int ox = (int)(glyph_dsc.ofs_x * scale);
    int oy = (int)(glyph_dsc.ofs_y * scale);
    uint8_t bpp = glyph_dsc.bpp;

    // Scaled dimensions
    int scaled_gw = (int)(gw * scale);
    int scaled_gh = (int)(gh * scale);

    // Render glyph pixels
    // ofs_y is relative to baseline (positive = above baseline)
    int scaled_line_height = (int)(font->line_height * scale);
    int scaled_base_line = (int)(font->base_line * scale);
    int baseline_y = y + scaled_line_height - scaled_base_line;
    int glyph_top_y = baseline_y - oy - scaled_gh;

    for (int row = 0; row < scaled_gh; row++) {
      for (int col = 0; col < scaled_gw; col++) {
        // Map scaled coordinates back to original bitmap (nearest-neighbor)
        int src_row = (int)(row / scale);
        int src_col = (int)(col / scale);
        if (src_row >= gh) src_row = gh - 1;
        if (src_col >= gw) src_col = gw - 1;

        // Calculate pixel value from bitmap
        uint32_t pixel_idx = src_row * gw + src_col;
        uint8_t pixel_val = 0;

        if (bpp == 1) {
          uint32_t byte_idx = pixel_idx / 8;
          uint32_t bit_idx = 7 - (pixel_idx % 8);
          pixel_val = (bitmap[byte_idx] >> bit_idx) & 0x01;
          pixel_val = pixel_val ? 255 : 0;
        } else if (bpp == 2) {
          uint32_t byte_idx = pixel_idx / 4;
          uint32_t shift = 6 - (pixel_idx % 4) * 2;
          pixel_val = (bitmap[byte_idx] >> shift) & 0x03;
          pixel_val = pixel_val * 85;
        } else if (bpp == 4) {
          uint32_t byte_idx = pixel_idx / 2;
          uint32_t shift = (pixel_idx % 2) ? 0 : 4;
          pixel_val = (bitmap[byte_idx] >> shift) & 0x0F;
          pixel_val = pixel_val * 17;
        } else if (bpp == 8) {
          pixel_val = bitmap[pixel_idx];
        }

        // Draw pixel if above threshold (anti-aliasing threshold)
        if (pixel_val > 127) {
          int px = x + ox + col;
          int py = glyph_top_y + row;
          setPixel(px, py);
        }
      }
    }

    // Advance cursor
    x += (int)(glyph_dsc.adv_w * scale);
    text++;
  }
}

// Draw inverted text (white on black) using Montserrat font (uses LVGL API)
// Supports scaling for sizes > 72px
void drawTextMontserratInverted(int x, int y, const char* text, int fontSize) {
  // For sizes > 72, use 72px font with scaling
  int baseSize = (fontSize > 72) ? 72 : fontSize;
  float scale = (fontSize > 72) ? (float)fontSize / 72.0f : 1.0f;

  const lv_font_t* font = getFontForSize(baseSize);

  while (*text) {
    uint32_t letter = (uint8_t)*text;

    // Get glyph descriptor using LVGL API
    lv_font_glyph_dsc_t glyph_dsc;
    bool has_glyph = lv_font_get_glyph_dsc(font, &glyph_dsc, letter, 0);

    if (!has_glyph || glyph_dsc.box_w == 0 || glyph_dsc.box_h == 0) {
      x += (int)(glyph_dsc.adv_w * scale);
      text++;
      continue;
    }

    // Get bitmap using LVGL API (handles decompression)
    const uint8_t* bitmap = lv_font_get_glyph_bitmap(font, letter);
    if (!bitmap) {
      x += (int)(glyph_dsc.adv_w * scale);
      text++;
      continue;
    }

    int gw = glyph_dsc.box_w;
    int gh = glyph_dsc.box_h;
    int ox = (int)(glyph_dsc.ofs_x * scale);
    int oy = (int)(glyph_dsc.ofs_y * scale);
    uint8_t bpp = glyph_dsc.bpp;

    // Scaled dimensions
    int scaled_gw = (int)(gw * scale);
    int scaled_gh = (int)(gh * scale);

    // ofs_y is relative to baseline (positive = above baseline)
    int scaled_line_height = (int)(font->line_height * scale);
    int scaled_base_line = (int)(font->base_line * scale);
    int baseline_y = y + scaled_line_height - scaled_base_line;
    int glyph_top_y = baseline_y - oy - scaled_gh;

    for (int row = 0; row < scaled_gh; row++) {
      for (int col = 0; col < scaled_gw; col++) {
        // Map scaled coordinates back to original bitmap (nearest-neighbor)
        int src_row = (int)(row / scale);
        int src_col = (int)(col / scale);
        if (src_row >= gh) src_row = gh - 1;
        if (src_col >= gw) src_col = gw - 1;

        uint32_t pixel_idx = src_row * gw + src_col;
        uint8_t pixel_val = 0;

        if (bpp == 1) {
          uint32_t byte_idx = pixel_idx / 8;
          uint32_t bit_idx = 7 - (pixel_idx % 8);
          pixel_val = (bitmap[byte_idx] >> bit_idx) & 0x01;
          pixel_val = pixel_val ? 255 : 0;
        } else if (bpp == 2) {
          uint32_t byte_idx = pixel_idx / 4;
          uint32_t shift = 6 - (pixel_idx % 4) * 2;
          pixel_val = (bitmap[byte_idx] >> shift) & 0x03;
          pixel_val = pixel_val * 85;
        } else if (bpp == 4) {
          uint32_t byte_idx = pixel_idx / 2;
          uint32_t shift = (pixel_idx % 2) ? 0 : 4;
          pixel_val = (bitmap[byte_idx] >> shift) & 0x0F;
          pixel_val = pixel_val * 17;
        } else if (bpp == 8) {
          pixel_val = bitmap[pixel_idx];
        }

        if (pixel_val > 127) {
          int px = x + ox + col;
          int py = glyph_top_y + row;
          // Inverted: clear the pixel (make white on black background)
          if (px >= 0 && px < labelWidth && py >= 0 && py < labelHeight) {
            bitmapBuffer[py][px / 8] |= (1 << (7 - (px % 8)));
          }
        }
      }
    }

    x += (int)(glyph_dsc.adv_w * scale);
    text++;
  }
}

// Legacy drawTextInverted using 5x7 font (kept for compatibility)
void drawTextInverted(int x, int y, const char* text, int scale) {
  while (*text) {
    int idx = -1;
    char c = *text;

    if (c == ' ') idx = 0;
    else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 1;
    else if (c == 'm') idx = 40;
    else if (c >= 'a' && c <= 'z') idx = c - 'a' + 1;
    else if (c >= '0' && c <= '9') idx = c - '0' + 27;
    else if (c == '%') idx = 37;
    else if (c == '.') idx = 38;
    else if (c == '/') idx = 39;

    if (idx >= 0) {
      for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&font5x7[idx][col]);
        for (int row = 0; row < 7; row++) {
          if (line & (1 << row)) {
            for (int sy = 0; sy < scale; sy++) {
              for (int sx = 0; sx < scale; sx++) {
                int px = x + col * scale + sx;
                int py = y + row * scale + sy;
                if (px >= 0 && px < labelWidth && py >= 0 && py < labelHeight) {
                  bitmapBuffer[py][px / 8] |= (1 << (7 - (px % 8)));
                }
              }
            }
          }
        }
      }
    }
    x += 6 * scale;
    text++;
  }
}

// ============================================================================
// Print Functions
// ============================================================================

bool printLabelDirect() {
  displayDebug("Starting print...");

  displayDebug("setDensity(3)...");
  if (!setDensity(3)) {
    displayDebug("setDensity failed");
    return false;
  }
  displayDebug("setDensity OK");
  delay(50);

  displayDebug("setLabelType(1)...");
  if (!setLabelType(LABEL_WITH_GAPS)) {
    displayDebug("setLabelType failed");
    return false;
  }
  displayDebug("setLabelType OK");
  delay(50);

  displayDebug("printStart()...");
  if (!printStart()) {
    displayDebug("printStart failed");
    return false;
  }
  displayDebug("printStart OK");
  delay(100);

  displayDebug("pageStart()...");
  if (!pageStart()) {
    displayDebug("pageStart failed");
    return false;
  }
  displayDebug("pageStart OK");
  delay(100);

  displayDebug("setPageSize()...");
  if (!setPageSize(labelWidth, labelHeight)) {
    displayDebug("setPageSize failed");
    return false;
  }
  displayDebug("setPageSize OK");
  delay(100);

  displayDebug("Sending bitmap...");
  int widthBytes = labelWidth / 8;
  int row = 0;
  int totalRowsSent = 0;
  int consecutiveDataRows = 0;

  while (row < labelHeight) {
    bool isEmpty = isRowEmpty(bitmapBuffer[row], widthBytes);

    if (isEmpty) {
      uint8_t repeatCount = 1;
      while (row + repeatCount < labelHeight && repeatCount < 255) {
        if (!isRowEmpty(bitmapBuffer[row + repeatCount], widthBytes)) break;
        repeatCount++;
      }

      if (!printEmptyRows(row, repeatCount)) {
        displayDebug("printEmptyRows failed");
        return false;
      }

      row += repeatCount;
      consecutiveDataRows = 0;
      delay(5);

    } else {
      if (!printBitmapRow(row, bitmapBuffer[row], widthBytes, 1)) {
        displayDebug("printBitmapRow failed");
        return false;
      }

      row += 1;
      consecutiveDataRows++;

      if (consecutiveDataRows > 20) {
        delay(25);
      } else if (consecutiveDataRows > 10) {
        delay(20);
      } else {
        delay(10);
      }

      if (consecutiveDataRows % 50 == 0) {
        delay(200);
      }
    }

    totalRowsSent++;

    if (responseReceived) {
      if (responseLength >= 3 && responseBuffer[2] == 0xD3) {
        delay(1500);
      }
      responseReceived = false;
    }
  }

  char msg[32];
  snprintf(msg, sizeof(msg), "Sent %d rows", totalRowsSent);
  displayDebug(msg);

  // Wait for D3 completion
  displayDebug("Waiting for printer...");
  responseReceived = false;
  unsigned long waitStart = millis();

  while ((millis() - waitStart) < 15000) {
    if (responseReceived) {
      if (responseLength >= 3 && responseBuffer[2] == 0xD3) {
        displayDebug("D3 received");
        break;
      }
      responseReceived = false;
    }
    delay(50);
  }

  delay(1500);
  pageEnd();
  delay(1500);
  printEnd();

  displayDebug("Print completed!");
  return true;
}

// Legacy wrapper
void printLabel() {
  printLabelDirect();
}

// ============================================================================
// Multi-core Print Task
// ============================================================================

void printTask(void* parameter) {
  PrintJob job;
  displayDebug("printTask started");

  while (true) {
    if (xQueueReceive(printQueue, &job, portMAX_DELAY) == pdTRUE) {
      if (job.valid) {
        printerBusy = true;
        displayDebug("printTask: printing...");
        printLabelDirect();
        printerBusy = false;
      }
    }
  }
}

void initPrintTask() {
  bitmapMutex = xSemaphoreCreateMutex();
  printQueue = xQueueCreate(1, sizeof(PrintJob));

  xTaskCreatePinnedToCore(
    printTask,
    "PrintTask",
    8192,
    NULL,
    1,
    &printTaskHandle,
    1
  );

  displayDebug("Print task initialized");
}

bool queuePrintJob() {
  displayDebug("queuePrintJob called");

  if (!pCharacteristic) {
    displayDebug("ERROR: not connected!");
    return false;
  }

  if (!printQueue) {
    displayDebug("ERROR: printQueue NULL!");
    return false;
  }

  if (printerBusy) {
    displayDebug("Printer busy!");
    return false;
  }

  PrintJob job;
  job.valid = true;
  job.width = labelWidth;
  job.height = labelHeight;

  if (xQueueSend(printQueue, &job, 0) == pdTRUE) {
    displayDebug("Print job queued");
    return true;
  }

  displayDebug("Queue failed!");
  return false;
}

bool isPrinterBusy() {
  return printerBusy;
}

// ============================================================================
// Label Templates
// ============================================================================

void printGasLabel(const char* var1, const char* var2, const char* var3,
                   const char* var4, const char* var5) {
  clearBitmap();

  // O2 line - using Montserrat font
  // "O2" label
  drawTextMontserrat(70, 25, "O2", 28);

  // O2 value (64px) and % symbol (51px = 64 * 0.8)
  String o2NumText = String(var1);
  int o2NumWidth = getTextWidthMontserrat(o2NumText.c_str(), 64);
  int percentWidth = getTextWidthMontserrat("%", 46);
  int o2RightEdge = labelWidth - 70;

  // Draw % symbol first (smaller, right-aligned)
  int percentX = o2RightEdge - percentWidth;
  drawTextMontserrat(percentX, 25, "%", 46);

  // Draw O2 number (larger, left of %)
  int o2NumX = percentX - o2NumWidth;
  drawTextMontserrat(o2NumX, 15, o2NumText.c_str(), 64);

  // He line
  // "He" label
  drawTextMontserrat(70, 95, "He", 28);

  // He value (64px) - right-aligned to same edge as O2 value (not %)
  String heText = String(var2);
  int heWidth = getTextWidthMontserrat(heText.c_str(), 64);
  int heX = percentX - heWidth;  // Align right edge with O2 number right edge
  drawTextMontserrat(heX, 85, heText.c_str(), 64);

  // Draw % symbol for He (smaller, right-aligned)
  drawTextMontserrat(percentX, 95, "%", 46);

  // Date line - centered, small (was scale 2 = ~14px)
  // Estimate width: ~8px per character at size 14
  String name_date = String(var4) + "  " + String(var5);
  int dateWidth = getTextWidthMontserrat(name_date.c_str(), 14);
  int dateX = (labelWidth - dateWidth) / 2;
  drawTextMontserrat(dateX, 150, name_date.c_str(), 14);

  // Bottom: MOD in inverted black rectangle (full width)
  fillRect(70, 170, labelWidth - 140, 80);

  // Draw MOD text in white (inverted) - centered horizontally
  String modText = "MOD " + String(var3) + "m";
  int modWidth = getTextWidthMontserrat(modText.c_str(), 36);
  int modX = (labelWidth - modWidth) / 2;

  // Center text vertically in rectangle (rect starts at y=170, height=80)
  // Font size 36, center in 80px height: (80 - 36) / 2 = 22, so y = 170 + 22 = 192
  drawTextMontserratInverted(modX, 185, modText.c_str(), 36);

  // Queue print job
  queuePrintJob();
}


