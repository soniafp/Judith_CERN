#ifndef ANA_CLUSTERRESIDUALS
#define ANA_CLUSTERRESIDUALS

#include <vector>
#include <string>

#include <TDirectory.h>
#include <TH1D.h>

#include "analyzers/analyzer.h"

namespace Mechanics { class Device; }

namespace Analyzers {

/**
  * Build the residual histograms between planes based on nearest cluster
  * offsets in sequential planes.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class ClusterResiduals : public Analyzer {
private:
  ClusterResiduals(const ClusterResiduals&);
  ClusterResiduals& operator=(const ClusterResiduals&);

  std::vector<TH1D*> m_hResidualsX;
  std::vector<TH1D*> m_hResidualsY;

  /** Get the global index of the residual between isensor and iref, for
    * idevice */
  size_t toGlobal(size_t idevice, size_t isensor, size_t iref) const;

  /** Constructor calls this to initialize memory */
  void initialize();

  /** Base virtual method defined, gives code to run at each loop */
  void process();

public:
  template <class T>
  ClusterResiduals(const T& t) : Analyzer(t) {
    initialize();
  }
  ~ClusterResiduals() {}

  void setOutput(TDirectory* dir, const std::string& name="ClusterResiduals") {
    // Just adds the default name
    Analyzer::setOutput(dir, name);
  }

  /** Get the residual for isensor in idevice, relative to iref sensor in the
    * reference device, along the x directoion. */
  TH1D& getResidualX(size_t idevice, size_t isensor, size_t iref) const;
  /** Get the residual for isensor in idevice, relative to iref sensor in the
    * reference device, along the y directoion. */
  TH1D& getResidualY(size_t idevice, size_t isensor, size_t iref) const;
};

}

#endif  // ANA_CLUSTERRESIDUALS

