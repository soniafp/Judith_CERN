#ifndef ANA_TRACKRESIDUALS
#define ANA_TRACKRESIDUALS

#include <vector>
#include <string>

#include <TDirectory.h>
#include <TH1D.h>

#include "analyzers/analyzer.h"

namespace Mechanics { class Device; }

namespace Analyzers {

/**
  * Build the residual histograms of tracks from a reference device to clusters
  * in sensors.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class TrackResiduals : public Analyzer {
private:
  TrackResiduals(const TrackResiduals&);
  TrackResiduals& operator=(const TrackResiduals&);

  /** Residual plot for each sensor */
  std::vector<TH1D*> m_hResidualsX;
  std::vector<TH1D*> m_hResidualsY;

  /** Index of sensor for a device in the vector of residuals */
  size_t toGlobal(size_t idevice, size_t isensor) const;

  /** Constructor calls this to initialize memory */
  void initialize();

  /** Base virtual method defined, gives code to run at each loop */
  void process();

public:
  /** Automatically calls the correct base constuctor */
  template <class T>
  TrackResiduals(const T& t) : Analyzer(t) { initialize(); }
  ~TrackResiduals() {}

  void setOutput(TDirectory* dir, const std::string& name="TrackResiduals") {
    // Just adds the default name
    Analyzer::setOutput(dir, name);
  }

  /** Get the residual for isensor in idevice, relative to iref sensor in the
    * reference device */
  TH1D& getResidualX(size_t idevice, size_t isensor) const;
  TH1D& getResidualY(size_t idevice, size_t isensor) const;
};

}

#endif  // ANA_TRACKRESIDUALS

