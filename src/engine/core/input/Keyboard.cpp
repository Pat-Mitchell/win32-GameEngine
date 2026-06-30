/// @file Keyboard.cpp

#include "Keyboard.h"

void Keyboard::onKeyDown(int virtualKey) {
  if(virtualKey >= 0 && virtualKey < KEY_COUNT) {
    m_Keys[virtualKey] = true;
  }
}

void Keyboard::onKeyUp(int virtualKey) {
  if(virtualKey >= 0 && virtualKey < KEY_COUNT) {
    m_Keys[virtualKey] = false;
  }
}

void Keyboard::reset() {
  for(int i = 0; i < KEY_COUNT; ++i) {
    m_Keys[i] = false;
  }
}

bool Keyboard::isKeyDown(int virtualKey) const {
  if(virtualKey < 0 || virtualKey >= KEY_COUNT) {
    return false;
  }
  return m_Keys[virtualKey];
}
