#include "LCDIC2.h"

LCDIC2::LCDIC2(uint8_t address, uint8_t width, uint8_t height, uint8_t dots) {
  _address = address;
  _dots = (dots == 10) & (height == 1);
  _height = height;
  _width = width;
}

bool LCDIC2::begin() {
  wait(20000);
  Wire.begin(_address);
  Wire.beginTransmission(_address);
  return  (_height && _width)
          & !Wire.endTransmission(1)
          & send(0b11, 4100)
          & send(0b11, 100)
          & send(0b11, 100)
          & send(0b10, 100)
          & write(LCDIC2_FUNCTION | LCDIC2_BITS_4 | (_height > 1) << 3 | _dots << 2)
          & write(LCDIC2_DISPLAY | _display << 2 | _cursor << 1 | _blink)
          & write(0b1)
          & write(LCDIC2_MODE | _gain << 1 | _shift)
          ;
}

bool LCDIC2::backlight(bool state) {
  Wire.beginTransmission(_address);
  Wire.write((_backlight = state) << 3);
  return !Wire.endTransmission(0) && flag();
}

bool LCDIC2::blink(bool state) {
  return write(LCDIC2_DISPLAY | _display << 2 | _cursor << 1 | (_blink = state));
}

bool LCDIC2::clear() {
  return write(0b1);
}

bool LCDIC2::cursor(bool state) {
  return write(LCDIC2_DISPLAY | _display << 2 | (_cursor = state) << 1 | _blink);
}

bool LCDIC2::cursor(uint8_t x, uint8_t y) {
  x = x < uint8_t(_width - 1) ? x : uint8_t(_width - 1);
  y = y < uint8_t(_height - 1) ? y : uint8_t(_height - 1);


  
  return write(LCDIC2_DDRAM | (y % 2) << 6 | ((y / 2) * _width) | x);
}

bool LCDIC2::cursorLeft() {
  return write(LCDIC2_MOVE | LCDIC2_CURSOR | LCDIC2_LEFT);
}

bool LCDIC2::cursorRight() {
  return write(LCDIC2_MOVE | LCDIC2_CURSOR | LCDIC2_RIGHT);
}

bool LCDIC2::display(bool state) {
  return write(LCDIC2_DISPLAY | (_display = state) << 2 | _cursor << 1 | _blink);
}

bool LCDIC2::flag() {
  Wire.requestFrom(uint8_t(_address), uint8_t(1));
  while (!Wire.available());
  Wire.beginTransmission(_address);
  return !Wire.endTransmission(1);
}

bool LCDIC2::glyph(uint8_t character) {
  return write(character, true);
}

bool LCDIC2::glyph(uint8_t id, uint8_t map[]) {
  if (!write(LCDIC2_CGRAM | id << 3)) return false;
  for (uint8_t i = 0; i < 8; i++) if (!write(map[i], true)) return false;
  return write(LCDIC2_DDRAM);
}

bool LCDIC2::home() {
  return write(0b10);
}

bool LCDIC2::leftToRight() {
  return write(LCDIC2_MODE | (_gain = LCDIC2_INC) << 1 | _shift);
}

bool LCDIC2::moveLeft() {
  return write(LCDIC2_MOVE | LCDIC2_SHIFT | LCDIC2_LEFT);
}

bool LCDIC2::moveRight() {
  return write(LCDIC2_MOVE | LCDIC2_SHIFT | LCDIC2_RIGHT);
}

size_t LCDIC2::print(String data) {
  size_t i = 0;
  while (write(data[i], 0b1) && (++i < data.length()));
  return i;
}

bool LCDIC2::rightToLeft() {
  return write(LCDIC2_MODE | (_gain = LCDIC2_DEC) << 1 | _shift);
}

bool LCDIC2::send(uint8_t data, uint16_t us) {
  Wire.beginTransmission(_address);
  writeLow(data);
  wait(us);
  return !Wire.endTransmission(0);
}

bool LCDIC2::shift(bool state) {
  return write(LCDIC2_MODE | _gain << 1 | (_shift = state));
}

void LCDIC2::wait(uint16_t us) {
  for (uint32_t i = 1; i < ((us * (F_CPU / 1000L)) / 1000L); i++);
}

bool LCDIC2::write(uint8_t data, uint8_t rs) {
  Wire.beginTransmission(_address);
  writeHigh(data, rs);
  writeLow(data, rs);
  return !Wire.endTransmission(0) && flag();
}

void LCDIC2::writeHigh(uint8_t data, uint8_t rs) {
  Wire.write(data & 0b11110000 | _backlight << 3 | 0b100 | rs);
  Wire.write(data & 0b11110000 | _backlight << 3);
}

void LCDIC2::writeLow(uint8_t data, uint8_t rs) {
  Wire.write(data << 4 | _backlight << 3 | 0b100 | rs);
  Wire.write(data << 4 | _backlight << 3);
}
