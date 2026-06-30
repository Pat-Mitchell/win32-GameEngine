/// @file Mouse.cpp

#include "Mouse.h"

void Mouse::onMove(int x, int y) {
  // Accumulate motion relative to the previous position. Skip the very first
  // move so we don't report a delta measured against an uninitialized origin.
  if(m_HasLast) {
    m_DeltaX += (float)(x - m_X);
    m_DeltaY += (float)(y - m_Y);
  }
  m_X = x;
  m_Y = y;
  m_HasLast = true;
}

void Mouse::onButtonDown(Button button) {
  if(button >= 0 && button < BUTTON_COUNT) {
    m_Buttons[button] = true;
  }
}

void Mouse::onButtonUp(Button button) {
  if(button >= 0 && button < BUTTON_COUNT) {
    m_Buttons[button] = false;
  }
}

bool Mouse::isButtonDown(Button button) const {
  if(button < 0 || button >= BUTTON_COUNT) {
    return false;
  }
  return m_Buttons[button];
}

void Mouse::consumeDelta() {
  m_DeltaX = 0.0f;
  m_DeltaY = 0.0f;
}
