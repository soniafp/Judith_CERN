#ifndef PROCESSORS_H
#define PROCESSORS_H

#include <TVirtualPad.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>

namespace Mechanics { class Device; }
namespace Mechanics { class Sensor; }
namespace Storage { class StorageIO; }
namespace Storage { class Event; }
namespace Storage { class Plane; }
namespace Storage { class Track; }
namespace Storage { class Cluster; }

namespace Processors {

TF1* fitPixelBeam(TH1D* hist, double pixWidth, double beamSigma, bool display = false);

void fitGaussian(
    TH1D* hist,
    double& mean,
    double& sigma,
    double& max,
    double& background,
    bool display = false);

void fitGaussian(
    TH1D* hist,
    double& mean,
    double& sigma,
    bool display = false);

void fitBox(
    TH1D* hist,
    double& mean,
    double& sigma,
    double& max,
    double& background,
    double sensorWidth, //width in um
    bool display = false);

void fitBox(
    TH1D* hist,
    double& mean,
    double& sigma,
    double sensorWidth, //width in um
    bool display = false);

void residualAlignment(TH2D* residualX, TH2D* residualY, double& offsetX,
                       double& offsetY, double& rotation,
                       double relaxation = 0.8, bool display = false);

void applyAlignment(Storage::Event* event, const Mechanics::Device* device);

void pixelToSlope(const Mechanics::Device* device, double &slopeX, double &slopeY);

int lineSensorIntercept(double posX, double posY, double posZ,
                        double slopeX, double slopeY,
                        const Mechanics::Sensor* sensor,
                        double& x, double& y, double& z);

int trackSensorIntercept(const Storage::Track* track,
                         const Mechanics::Sensor* sensor,
                         double& x, double& y, double& z);

void trackClusterDistance(const Storage::Track* track,
                          const Storage::Cluster* cluster,
                          const Mechanics::Sensor* sensor,
                          double& distX, double& distY,
                          double& errX, double& errY);

void trackError(const Storage::Track* track, double z, double& errX, double& errY);

}

#endif // PROCESSORS_H
