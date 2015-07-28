#ifndef LARGESYNCHRONIZER_H
#define LARGESYNCHRONIZER_H

#include <Rtypes.h>

namespace Storage { class Event; }
namespace Mechanics { class Device; }

namespace Processors {

class LargeSynchronizer
{
private:
  const Mechanics::Device* _shiftDevice;
  const Mechanics::Device* _staticDevice;

  double _threshold;
  unsigned int _bufferSize;
  unsigned int _readOutWindow;

  ULong64_t _lastShiftTime;
  ULong64_t _lastStaticTime;
  double* _shiftBuffer;
  double* _staticBuffer;
  unsigned int _numEvents;

  unsigned int offsetToIndex(int offset) const;

public:
  LargeSynchronizer(const Mechanics::Device* shiftDevice,
                    const Mechanics::Device* staticDevice,
                    double threshold,
                    unsigned int bufferSize,
                    unsigned int readOutWindow,
                    ULong64_t initialShiftTime,
                    ULong64_t initialStaticTime);
  ~LargeSynchronizer();

  void addEvents(const Storage::Event* shiftEvent,
                 const Storage::Event* staticEvent);
  bool checkBuffer() const;
  void printBuffers() const;
};

}

#endif // LARGESYNCHRONIZER_H
