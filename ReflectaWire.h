#pragma once

#include <Wire.h>
#include "Reflecta.h"

namespace reflectaWire
{
  // basic methods
  void setup();
	  
  // interface methods
  void wireBeginMaster();

  void wireRequestFrom();
  void wireRequestFromStart();
  void wireAvailable();
  void wireRead();

  void wireBeginTransmission();
  void wireWrite();
  void wireEndTransmission();
}; // namespace reflectaWire