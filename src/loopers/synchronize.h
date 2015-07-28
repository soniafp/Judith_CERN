#ifndef SYNCHRONIZE_H
#define SYNCHRONIZE_H

#include "looper.h"

#include <TH1D.h>

namespace Storage { class StorageIO; }
namespace Mechanics { class Device; }
namespace Processors { class ClusterMaker; }
namespace Processors { class TrackMaker; }
namespace Analyzers { class SyncFluctuation; }

namespace Loopers {

class Synchronize : public Looper
{
private:
  Mechanics::Device* _refDevice;
  Mechanics::Device* _dutDevice;
  Storage::StorageIO* _refOutput;
  Storage::StorageIO* _dutOutput;

  double _threshold;
  unsigned int _syncRatioSample;
  unsigned int _maxOffset;
  unsigned int _maxLargeSyncAttempts;
  unsigned int _bufferSize;
  unsigned int _preDiscards; // Discard this many events before the desynchronization
  bool _displayDistributions;
  unsigned int _maxConsecutiveFails;

  Analyzers::SyncFluctuation* setupAnalyzer();
  void displaySyncPlots(TH1D* syncHist, TH1D* unsyncHist);
  unsigned int  syncRatioLoop(ULong64_t start, ULong64_t num,
                              unsigned int refOffset, unsigned int dutOffset);
  bool findLargeOffset(unsigned int nevent,
                       unsigned int& refShift,
                       unsigned int& dutShift);

public:
  Synchronize(/* Use if you need mechanics (noise mask, pixel arrangement ...) */
              Mechanics::Device* refDevice,
              Mechanics::Device* dutDevice,
              /* Use if you will use the input to generate a new output (processing) */
              Storage::StorageIO* refOutput,
              Storage::StorageIO* dutOutput,
              /* These arguments are needed to be passed to the base looper class */
              Storage::StorageIO* refInput,
              Storage::StorageIO* dutInput,
              ULong64_t startEvent = 0,
              ULong64_t numEvents = 0,
              unsigned int eventSkip = 1);

  void loop();
  void calculateSyncRatio();

  void setThreshold(double threshold);
  void setSyncSample(unsigned int value);
  void setMaxOffset(unsigned int value);
  void setMaxLargeSyncAttempts(unsigned int value);
  void setBufferSize(unsigned int value);
  void setPreDiscards(unsigned int value);
  void setMaxConsecutiveFails(unsigned int value);
  void setDisplayDistributions(bool value);
};

}

#endif
