#include <Reflecta.h>
#include "ReflectaTest.h"

namespace reflectaTest
{
  bool infinite = false;

  // basic methods
  void setup()
  {
    reflectaFunctions::bind("test1", SetInfinite);
  }

  void loop()
  {
    while (infinite)
    {
      delay(100L);
    }
  }

  // interface methods
  void SetInfinite()
  {
    infinite = true;
  }
};

