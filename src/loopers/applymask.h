#ifndef APPLYMASK_H
#define APPLYMASK_H

#include "looper.h"

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }

namespace Loopers {

class ApplyMask : public Looper
{
private:
  Mechanics::Device* _refDevice;
  Storage::StorageIO* _refOutput;

public:
  ApplyMask(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
            Mechanics::Device* refDevice,
            /* Use if you will use the input to generate a new output (processing) */
            Storage::StorageIO* refOutput,
            /* These arguments are needed to be passed to the base looper class */
            Storage::StorageIO* refInput,
            ULong64_t startEvent = 0,
            ULong64_t numEvents = 0,
            unsigned int eventSkip = 1);

  void loop();
};

}

#endif
