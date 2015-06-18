#ifndef ANA_CORRELATIONS
#define ANA_CORRELATIONS

#include <vector>
#include <string>

#include <TDirectory.h>
#include <TH2D.h>

#include "analyzers/analyzer.h"

namespace Mechanics { class Device; }

namespace Analyzers {

/**
  * Build up the inter-plane correlation of clusters by plotting the spatial
  * difference of all clusters on adjeacent planes in the reference device,
  * or to the nearest reference plane in the DUTs.
  *
  * @author Garrin McGoldrick (garrin.mcgoldrick@cern.ch)
  */
class Correlations : public Analyzer {
private:
  // Disable copy constructor and assignment operators otherwise ROOT histogram
  // ownership nightmares migth ensue
  Correlations(const Correlations&);
  Correlations& operator=(const Correlations&);

  /** For each plane, gives the global index (device + plane) of the plane
    * relative to which it will be correlated */
  std::vector<size_t> m_irelative;

  // Keep track of each plane's histograms, one for each content type
  std::vector<TH2D*> m_hCorrelationsX;
  std::vector<TH2D*> m_hCorrelationsY;

  /** Find the global id of the sensor of a given device */
  size_t toGlobal(size_t idevice, size_t isensor) const;

  /** Constructor calls this to initialize memory */
  void initialize();

  /** Base virtual method defined, gives code to run at each loop */
  void process();

public:
  /** Automatically calls the correct base constuctor */
  template <class T>
  Correlations(const T& t) : Analyzer(t) { initialize(); }
  /** Memory managed by base class */
  ~Correlations() {}

  void setOutput(TDirectory* dir, const std::string& name="Correlations") {
    // Just adds the default name
    Analyzer::setOutput(dir, name);
  }

  TH2D& getCorrelationX(size_t idevice, size_t isensor) const;
  TH2D& getCorrelationY(size_t idevice, size_t isensor) const;
};

}

#endif  // ANA_CORRELATIONS

