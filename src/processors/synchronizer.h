#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <Rtypes.h>

namespace Storage { class Event; }
namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }

namespace Processors {

class Synchronizer
{
private:
  const Mechanics::Device* _refDevice;
  const Mechanics::Device* _dutDevice;

  double _threshold;
  const unsigned int _buffSize;
  unsigned int _pos;
  Storage::Event** _refEventBuff;
  Storage::Event** _dutEventBuff;
  double* _refDiffBuff;
  double* _dutDiffBuff;
  double* _diffBuff;
  bool _bufferEmpty;
  bool _bufferFull;
  unsigned int _numOverThreshold;

  unsigned int offsetToIndex(int offset) const;

public:
  Synchronizer(const Mechanics::Device* refDevice,
               const Mechanics::Device* dutDevice,
               double threshold, unsigned int buffSize = 20);
  ~Synchronizer();

  void processEvent(Storage::Event* refEvent,
                    Storage::Event* dutEvent);

  bool calculateOffset(unsigned int& desync, unsigned int& refOffset,
                       unsigned int& dutOffset) const;

  void clearBuffers();

  Storage::Event* getRefTargetEvent() const;
  Storage::Event* getDutTargetEvent() const;
  double getTargetDiff() const;
  int getOffsetToDesync() const;
  unsigned int getNumDiff() const;
  unsigned int getSize() const;
  bool getFull() const;
};

}

#endif // SYNCHRONIZER_H
