#ifndef ANA_CORRELATIONS
#define ANA_CORRELATIONS

#include <vector>
#include <string>

#include <TDirectory.h>
#include <TH1D.h>
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
  std::vector<TH1D*> m_hResidualsX;
  std::vector<TH1D*> m_hResidualsY;

  /** Find the global id of the sensor of a given device */
  size_t toGlobal(size_t idevice, size_t isensor) const;

  /** Constructor calls this to initialize memory */
  void initialize();

  /** Base virtual method defined, gives code to run at each loop */
  void process();

public:
  /** All constructors take in a const object, be it a vector of const devices,
    * a vector of devices, or a single device. Accept any, and let the compiler
    * choose the correct base class constructor */
  template <class T>
  Correlations(const T& t) : Analyzer(t) {
    initialize();
  }
  /** Memory managed by base class */
  ~Correlations() {}

  void setOutput(TDirectory* dir, const std::string& name="Correlations") {
    // Just adds the default name
    Analyzer::setOutput(dir, name);
  }

  const std::vector<size_t>& getRelative() const;

  TH2D& getCorrX(size_t idevice, size_t isensor) const;
  TH2D& getCorrY(size_t idevice, size_t isensor) const;
  TH1D& getResX(size_t idevice, size_t isensor) const;
  TH1D& getResY(size_t idevice, size_t isensor) const;
};

}

#endif  // ANA_CORRELATIONS

