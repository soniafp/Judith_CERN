#include "efficiency.h"

#include <cassert>
#include <sstream>
#include <math.h>
#include <vector>
#include <float.h>

#include <TDirectory.h>
#include <TH2D.h>
#include <TH1D.h>
#include <TError.h>
#include <TMath.h>

// Access to the device being analyzed and its sensors
#include "../mechanics/device.h"
#include "../mechanics/sensor.h"
// Access to the data stored in the event
#include "../storage/hit.h"
#include "../storage/cluster.h"
#include "../storage/plane.h"
#include "../storage/track.h"
#include "../storage/event.h"
// Some generic processors to calcualte typical event related things
#include "../processors/processors.h"
// This header defines all the cuts
#include "cuts.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

//using namespace std;
using std::cout;
using std::endl;

namespace Analyzers {

void Efficiency::processEvent(const Storage::Event* refEvent,
                              const Storage::Event* dutEvent)
{
  assert(refEvent && dutEvent && "Analyzer: can't process null events");

  // Throw an error for sensor / plane mismatch
  eventDeivceAgree(refEvent, dutEvent);
  //fill in the amplitude distribution histogram
  for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
  {
    for (unsigned int nhit = 0; nhit < dutEvent->getNumHits(); nhit++)
    {
      const Storage::Hit* hit = dutEvent->getHit(nhit);

      _amplDist.at(nsensor)->Fill( hit->getValue() );
    }
  }

  //
  // Fill occupancy based
  //
  bool pass_track_selection = false;
  for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
  {
    Storage::Track* track = refEvent->getTrack(ntrack);

    // Check if the track passes the cuts
    bool pass = true;
    for (unsigned int ncut = 0; ncut < _numTrackCuts; ncut++)
      if (!_trackCuts.at(ncut)->check(track)) { pass = false; break; }
    if (!pass) continue;
    
    pass_track_selection=true;

    for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
    {
      Mechanics::Sensor* sensor = _dutDevice->getSensor(nsensor);

      double tx = -9999.0, ty = -9999.0, tz = -9999.0;
      Processors::trackSensorIntercept(track, sensor, tx, ty, tz);
      
      // Fill Track occupancy at DUT
      _trackOcc.at(nsensor)->Fill(tx - sensor->getOffX(), ty - sensor->getOffY());
      _trackRes.at(nsensor)->Fill(tx - sensor->getOffX(), ty - sensor->getOffY());

      // Fill track residual
      Storage::Plane* plane = dutEvent->getPlane(nsensor);	  
      for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++){
	Storage::Cluster* cluster = plane->getCluster(ncluster);	  

	// Check if the cluster passes the cuts
	bool cluster_pass = true;
	for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
	  if (!_clusterCuts.at(ncut)->check(cluster)) { cluster_pass = false; break; }
	if (!cluster_pass) continue;
	//std::cout << "getclustX: " << cluster->getPosX() << " tx: " << tx << std::endl;
	//std::cout << "getclustY: " << cluster->getPosY() << " ty: " << ty << std::endl;	
	_trackResHit.at(nsensor)->Fill(tx - cluster->getPosX(), ty - cluster->getPosY());
      } // end cluster loop

    }
  }// end track loop
  
  // if there is a track, then plot the occupancy
  if(pass_track_selection){
    for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++){
      Storage::Plane* plane = dutEvent->getPlane(nplane);	  
      for (unsigned int ncluster = 0; ncluster < plane->getNumClusters(); ncluster++){
	Storage::Cluster* cluster = plane->getCluster(ncluster);	  

        // Check if the cluster passes the cuts
        bool pass = true;
        for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
          if (!_clusterCuts.at(ncut)->check(cluster)) { pass = false; break; }
        if (!pass) continue;
	
	// DUT hit occupancy

	_dutHitOcc.at(nplane)->Fill(cluster->getPosX() - _dutDevice->getSensor(nplane)->getOffX(), cluster->getPosY() - _dutDevice->getSensor(nplane)->getOffY());
      }
    }
  } // end has a track
  //
  // end occupancy filling
  //
  
  // Check if the event passes the cuts
  for (unsigned int ncut = 0; ncut < _numEventCuts; ncut++)
    if (!_eventCuts.at(ncut)->check(refEvent)) return;

  for (unsigned int ntrack = 0; ntrack < refEvent->getNumTracks(); ntrack++)
  {
    Storage::Track* track = refEvent->getTrack(ntrack);

    // Check if the track passes the cuts
    bool pass = true;
    for (unsigned int ncut = 0; ncut < _numTrackCuts; ncut++)
      if (!_trackCuts.at(ncut)->check(track)) { pass = false; break; }
    if (!pass) continue;

    // Make a list of the planes with their matched cluster
    std::vector<Storage::Cluster*> matches;
    for (unsigned int nplane = 0; nplane < dutEvent->getNumPlanes(); nplane++)
      matches.push_back(0); // No matches

    // Get the matches from the track
    for (unsigned int nmatch = 0; nmatch < track->getNumMatchedClusters(); nmatch++)
    {
      Storage::Cluster* cluster = track->getMatchedCluster(nmatch);

      // Check if this cluster passes the cuts
      bool pass = true;
      for (unsigned int ncut = 0; ncut < _numClusterCuts; ncut++)
        if (!_clusterCuts.at(ncut)->check(cluster)) { pass = false; break; }
      if (!pass) continue;

      matches.at(cluster->getPlane()->getPlaneNum()) = cluster;
    }

    assert(matches.size() == _dutDevice->getNumSensors() &&
           _relativeToSensor < (int)_dutDevice->getNumSensors() &&
           "Efficiency: matches has the wrong size");

    // If asking for relative efficiency to a plane, look for a match in that one first
    if (_relativeToSensor >= 0 && !matches.at(_relativeToSensor)) continue;

    for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
    {
      Mechanics::Sensor* sensor = _dutDevice->getSensor(nsensor);

      double tx = 0, ty = 0, tz = 0;
      Processors::trackSensorIntercept(track, sensor, tx, ty, tz);

      double px = 0, py = 0;
      sensor->spaceToPixel(tx, ty, tz, px, py);

      const Storage::Cluster* match = matches.at(nsensor);

      //MAtevz 20141202 added 9-pixel efficiency plot.
      // Check if the intercepted pixel is hit
      bool trackMatchesPixel = false;
      unsigned int pixelX = px;
      unsigned int pixelY = py;
      double ninepixelX = 5.0;
      double ninepixelY = 5.0;

      if (match)
      {
        for (unsigned int nhit = 0; nhit < match->getNumHits(); nhit++)
        {
          const Storage::Hit* hit = match->getHit(nhit);

          if (abs( abs(hit->getPixX()) - abs(pixelX) ) <= 1 && abs( abs(hit->getPixY()) - abs(pixelY) ) <= 1)
          {
            if (abs(hit->getPixX()) - abs(pixelX) == 0)
              ninepixelX = px - (int)px;
            else if ( abs(hit->getPixX()) - abs(pixelX) == 1)
              ninepixelX = (px) - (int)px - 1.0; //)*(-1.0);
            else if (abs(hit->getPixX()) - abs(pixelX) == -1)
              ninepixelX = px - (int)px + 1.0;
            else cout<<"ERR: unexpected hit on X in efficiency.cpp line 128\n";

            if (abs(hit->getPixY()) - abs(pixelY) == 0)
              ninepixelY = py - (int)py;
            else if (abs(hit->getPixY()) - abs(pixelY) == 1)
              ninepixelY = py - (int)py - 1.0; //)*(-1.0);
            else if (abs(hit->getPixY()) - abs(pixelY) == -1)
              ninepixelY = py - (int)py + 1.0;
            else cout<<"ERR: unexpected hit on Y in efficiency.cpp line 133\n";

            _inPixelEfficiencyExtended.at(nsensor)->Fill(
                              (bool)match,
                              ninepixelX * sensor->getPitchX(),
                              ninepixelY * sensor->getPitchY());
            _inPixelCCE.at(nsensor)->Fill(
                              ninepixelX * sensor->getPitchX(),
                              ninepixelY * sensor->getPitchY(),
                              hit->getValue()  ); //ampl. value as weight
          }


          // make a spatial cut to get rid of the noisy amplitudes.
          // for now hardcode the cut to be smaller than 4x4 mm.
          double cutX = 0.8*(sensor->getPitchX()/2); //80% of the Xaxis
          double cutY = 0.8*(sensor->getPitchY()/2); //80% of the Yaxis

          if (  abs(ninepixelX) < cutX && abs(ninepixelY) < cutY )
            _amplDistCuts.at(nsensor)->Fill( hit->getValue() );


          if (hit->getPixX() == pixelX && hit->getPixY() == pixelY)
          {
           trackMatchesPixel = true;
           break;
          }
        }
      } // end of 9-pixel efficiency plotting.


      // Get the location within the pixel where the track interception
      const double trackPosX = px - (int)px;
      const double trackPosY = py - (int)py;

      _inPixelEfficiency.at(nsensor)->Fill((bool)match,
                                           trackPosX * sensor->getPitchX(),
                                           trackPosY * sensor->getPitchY());

      if (match)
      {
        _efficiencyMap.at(nsensor)->Fill(true, tx, ty);
        _matchedTracks.at(nsensor)->Fill(1);
        if (_efficiencyTime.size())
          _efficiencyTime.at(nsensor)->Fill(true, _refDevice->tsToTime(refEvent->getTimeStamp()));
      }
      else
      {
        _efficiencyMap.at(nsensor)->Fill(false, tx, ty);
        _matchedTracks.at(nsensor)->Fill(0);
        if (_efficiencyTime.size())
          _efficiencyTime.at(nsensor)->Fill(false, _refDevice->tsToTime(refEvent->getTimeStamp()));
      }
    }
  }
}

void Efficiency::postProcessing()
{
  if (_postProcessed) return;

  for (unsigned int nsensor = 0; nsensor < _dutDevice->getNumSensors(); nsensor++)
  {
    //divide accumulated amplitude histogram with
    //number of hits per bin.
    _inPixelCCE.at(nsensor)->Divide(
            _inPixelEfficiencyExtended.at(nsensor)->GetTotalHistogram() );

    //superimpose the two amplitude histograms onto a new one.
    _amplDistCommon.at(nsensor) = (TH1D*)_amplDistCuts.at(nsensor)->DrawNormalized();
    _amplDistCommon.at(nsensor) = (TH1D*)_amplDist.at(nsensor)->DrawNormalized("same");



    // Get efficiency per pixel
    TEfficiency* efficiency = _efficiencyMap.at(nsensor);
    const TH1* values = efficiency->GetTotalHistogram();
    TH1D* distribution = _efficiencyDistribution.at(nsensor);

    // Loop over all pixel groups
    for (Int_t binx = 1; binx <= values->GetNbinsX(); binx++)
    {
      for (Int_t biny = 1; biny <= values->GetNbinsY(); biny++)
      {
        const Int_t bin = values->GetBin(binx, biny);
        if (values->GetBinContent(binx, biny) < 1) continue;
        const double value = efficiency->GetEfficiency(bin);
        const double sigmaLow = efficiency->GetEfficiencyErrorLow(bin);
        const double sigmaHigh = efficiency->GetEfficiencyErrorUp(bin);

        // Find the probability of this pixel group being found in all bins of the distribution
        double normalization = 0;
        for (Int_t distBin = 1; distBin <= distribution->GetNbinsX(); distBin++)
        {
          const double evaluate = distribution->GetBinCenter(distBin);
          const double sigma = (evaluate < value) ? sigmaLow : sigmaHigh;
          const double weight = TMath::Gaus(evaluate, value, sigma);
          normalization += weight;
        }
        for (Int_t distBin = 1; distBin <= distribution->GetNbinsX(); distBin++)
        {
          const double evaluate = distribution->GetBinCenter(distBin);
          const double sigma = (evaluate < value) ? sigmaLow : sigmaHigh;
          const double weight = TMath::Gaus(evaluate, value, sigma);
          distribution->Fill(evaluate, weight / normalization);
        }
      }
    }
  }

  // Compute the Efficiency
  assert(_dutHitOcc.size()==_trackOcc.size() && "Analyzer::Efficiency: track occupancy different in size from hit occ.");
  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo
  
  for(unsigned nplane = 0; nplane<_trackOcc.size(); ++nplane){
    TH2D *htmp = static_cast<TH2D *>(_dutHitOcc.at(nplane)->Clone());
    htmp->Divide(_trackOcc.at(nplane));

    name.str(""); title.str("");
    name << "sensor" << nplane << "_"
         << "hitEff" << _nameSuffix;
    title << "sensor" << nplane 
          << " Hit Efficiency";    
    htmp->SetName(name.str().c_str());
    htmp->SetTitle(title.str().c_str());
    htmp->SetDirectory(_dutHitOcc.at(nplane)->GetDirectory());
    _dutHitEff.push_back(htmp);

    // Track residual
    TH2D *htmp_res = static_cast<TH2D *>(_trackResHit.at(nplane)->Clone());
    htmp_res->Divide(_trackRes.at(nplane));

    name.str(""); title.str("");
    name << "sensor" << nplane << "_"
         << "TrackResEff" << _nameSuffix;
    title << "sensor" << nplane 
          << " Track Residual Efficiency";    
    htmp_res->SetName(name.str().c_str());
    htmp_res->SetTitle(title.str().c_str());
    htmp_res->SetDirectory(_trackRes.at(nplane)->GetDirectory());
    _trackResEff.push_back(htmp_res);
  }
  
  _postProcessed = true;
}

Efficiency::Efficiency(const Mechanics::Device* refDevice,
                       const Mechanics::Device* dutDevice,
                       TDirectory* dir,
                       const char* suffix,
                       int relativeToSensor,
                       unsigned int rebinX,
                       unsigned int rebinY,
                       unsigned int pixBinsX,
                       unsigned int pixBinsY) :
  // Base class is initialized here and manages directory / device
  DualAnalyzer(refDevice, dutDevice, dir, suffix),
  // Initialize processing parameters here
  _relativeToSensor(relativeToSensor)
{
  assert(refDevice && dutDevice && "Analyzer: can't initialize with null device");

  // Makes or gets a directory called from inside _dir with this name
  TDirectory* plotDir = makeGetDirectory("Efficiency");

  std::stringstream name; // Build name strings for each histo
  std::stringstream title; // Build title strings for each histo

  if (relativeToSensor >= (int)_dutDevice->getNumSensors())
    throw "Efficiency: relative sensor exceeds range";

  // Generate a histogram for each sensor in the device
  for (unsigned int nsens = 0; nsens < _dutDevice->getNumSensors(); nsens++)
  {
    Mechanics::Sensor* sensor = _dutDevice->getSensor(nsens);

    unsigned int nx = sensor->getNumX() / rebinX;
    unsigned int ny = sensor->getNumY() / rebinY;
    if (nx < 1) nx = 1;
    if (ny < 1) ny = 1;
    const double lowX = sensor->getOffX() - sensor->getSensitiveX() / 2.0;
    const double uppX = sensor->getOffX() + sensor->getSensitiveX() / 2.0;
    const double lowY = sensor->getOffY() - sensor->getSensitiveY() / 2.0;
    const double uppY = sensor->getOffY() + sensor->getSensitiveY() / 2.0;

    // TODO: change to pixel space

    cout << "   DEVICE NAME: "<< sensor->getDevice()->getName() << endl
	 << "   sensor NAME: " << sensor->getName() << endl
	 << "   map: "<< _nameSuffix << endl;
    // Efficiency map initialization
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "Map" << _nameSuffix;
    // Special titel includes axis
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Efficiency Map"
          << ";X position" << " [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position" << " [" << _dutDevice->getSpaceUnit() << "]"
          << ";Efficiency";
    TEfficiency* map = new TEfficiency(name.str().c_str(), title.str().c_str(),
                                       nx, lowX, uppX,
                                       ny, lowY, uppY);
    map->SetDirectory(plotDir);
    map->SetStatisticOption(TEfficiency::kFWilson);
    _efficiencyMap.push_back(map);


    // Pixel grouped efficieny initialization
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "GroupedDistribution" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Grouped Efficiency";
    TH1D* pixels = new TH1D(name.str().c_str(), title.str().c_str(),
                            20, 0, 1.001);
    pixels->GetXaxis()->SetTitle("Pixel group efficiency");
    pixels->GetYaxis()->SetTitle("Pixel groups / 0.05");
    pixels->SetDirectory(plotDir);
    _efficiencyDistribution.push_back(pixels);



    //Matevz 20141202 9-pixel in-pixel efficiency histogram initialization
    //Not inPixelEfficiency - inPixelEfficiencyExtended
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "In9PixelEfficiency" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " In 9 Pixel Eff"
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TEfficiency* inPixelEfficiencyExtended = new TEfficiency(name.str().c_str(), title.str().c_str(),
                             3*pixBinsX, -sensor->getPitchX(), 2*sensor->getPitchX(),
                             3*pixBinsY, -sensor->getPitchY(), 2*sensor->getPitchY());

    inPixelEfficiencyExtended->SetDirectory(plotDir);
    _inPixelEfficiencyExtended.push_back(inPixelEfficiencyExtended);



    //Matevz 20141203 charge collection map histogram initialization
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "inPixelCCE" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " in-pixel charge collection map"
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* inPixelCCE = new TH2D(name.str().c_str(), title.str().c_str(),
                             3*pixBinsX, -sensor->getPitchX(), 2*sensor->getPitchX(),
                             3*pixBinsY, -sensor->getPitchY(), 2*sensor->getPitchY());

    inPixelCCE->SetDirectory(plotDir);
    _inPixelCCE.push_back(inPixelCCE);



    //Matevz 20141203 amplitude histograms initialisation
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "PulseAmpl" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Pulse amplitude distribution";

    TH1D* amplDist = new TH1D(name.str().c_str(), title.str().c_str(), 100, 0, 0.1);

    amplDist->SetDirectory(plotDir);
    _amplDist.push_back(amplDist);

    //Matevz 20141203 amplitude histograms with CUTS initialisation
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "PulseAmplCuts" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Pulse amplitude distribution with cuts";

    TH1D* amplDistCuts = new TH1D(name.str().c_str(), title.str().c_str(), 100, 0, 0.1);

    amplDistCuts->SetDirectory(plotDir);
    _amplDistCuts.push_back(amplDistCuts);

    //Matevz 20141203 amplitude histograms for SAMEing of both - initialisation
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "PulseAmplCommon" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Pulse amplitude distribution with cuts";

    TH1D* amplDistCommon = new TH1D(name.str().c_str(), title.str().c_str(), 100, 0, 0.1);

    amplDistCommon->SetDirectory(plotDir);
    _amplDistCommon.push_back(amplDistCommon);



    // Track matching initialization
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         << "MatchedTracks" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Matched Tracks";
    TH1D* matched = new TH1D(name.str().c_str(), title.str().c_str(),
                             2, 0 - 0.5, 2 - 0.5);
    matched->GetXaxis()->SetTitle("0 : unmatched tracks | 1 : matched tracks");
    matched->GetZaxis()->SetTitle("Tracks");
    matched->SetDirectory(plotDir);
    _matchedTracks.push_back(matched);

    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "InPixelEfficiency" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " In Pixel Efficiency"
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TEfficiency* inPixelEfficiency = new TEfficiency(name.str().c_str(), title.str().c_str(),
                                                     pixBinsX, 0, sensor->getPitchX(),
                                                     pixBinsY, 0, sensor->getPitchY());
    inPixelEfficiency->SetDirectory(plotDir);
    _inPixelEfficiency.push_back(inPixelEfficiency);

    // Track occupancy extrapolated to DUT position
    float num_pixels = _dutDevice->getNumPixels()==1 ? 5.0 : float(_dutDevice->getNumPixels());
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "TrackOccupancy" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Track Occupancy "
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* trackOcc = new TH2D(name.str().c_str(), title.str().c_str(),
			      4*pixBinsX, -2.0*num_pixels*sensor->getPitchX(), 2.0*num_pixels*sensor->getPitchX(),
			      4*pixBinsY, -2.0*num_pixels*sensor->getPitchY(), 2.0*num_pixels*sensor->getPitchY());

    trackOcc->SetDirectory(plotDir);
    _trackOcc.push_back(trackOcc);

    // Track occupancy extrapolated to DUT position
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "TrackResidual" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Track TrackResidual "
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* trackRes = new TH2D(name.str().c_str(), title.str().c_str(),
			      4*pixBinsX, -2.0*num_pixels*sensor->getPitchX(), 2.0*num_pixels*sensor->getPitchX(),
			      4*pixBinsY, -2.0*num_pixels*sensor->getPitchY(), 2.0*num_pixels*sensor->getPitchY());

    trackRes->SetDirectory(plotDir);
    _trackRes.push_back(trackRes);    

    // Track occupancy extrapolated to DUT position
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "TrackResidualHit" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " Track TrackResidual Hit "
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* trackResHit = new TH2D(name.str().c_str(), title.str().c_str(),
			      4*pixBinsX, -2.0*num_pixels*sensor->getPitchX(), 2.0*num_pixels*sensor->getPitchX(),
			      4*pixBinsY, -2.0*num_pixels*sensor->getPitchY(), 2.0*num_pixels*sensor->getPitchY());

    trackResHit->SetDirectory(plotDir);
    _trackResHit.push_back(trackResHit);     

    // DUT hit occupancy
    name.str(""); title.str("");
    name << sensor->getDevice()->getName() << sensor->getName()
         <<  "DUTHitOccupancy" << _nameSuffix;
    title << sensor->getDevice()->getName() << " " << sensor->getName()
          << " DUT Hit Occupancy "
          << ";X position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Y position [" << _dutDevice->getSpaceUnit() << "]"
          << ";Tracks";
    TH2D* dutHitOcc = new TH2D(name.str().c_str(), title.str().c_str(),
			       4*pixBinsX, -2.0*num_pixels*sensor->getPitchX(), 2.0*num_pixels*sensor->getPitchX(),
			       4*pixBinsY, -2.0*num_pixels*sensor->getPitchY(), 2.0*num_pixels*sensor->getPitchY());

    dutHitOcc->SetDirectory(plotDir);
    _dutHitOcc.push_back(dutHitOcc);    
    

    if (_refDevice->getTimeEnd() > _refDevice->getTimeStart()) // If not used, they are both == 0
    {
      // Prevent aliasing
      const unsigned int nTimeBins = 100;
      const ULong64_t timeSpan = _refDevice->getTimeEnd() - _refDevice->getTimeStart() + 1;
      const ULong64_t startTime = _refDevice->getTimeStart();
      const ULong64_t endTime = timeSpan - (timeSpan % nTimeBins) + startTime;

      name.str(""); title.str("");
      name << sensor->getDevice()->getName() << sensor->getName()
           << "EfficiencyVsTime" << _nameSuffix;
      title << sensor->getDevice()->getName() << " " << sensor->getName()
            << " Efficiency Vs. Time"
            << ";Time [" << _refDevice->getTimeUnit() << "]"
            << ";Average efficiency";
      TEfficiency* efficiencyTime = new TEfficiency(name.str().c_str(), title.str().c_str(),
                                                    nTimeBins,
                                                    _refDevice->tsToTime(startTime),
                                                    _refDevice->tsToTime(endTime + 1));
      efficiencyTime->SetDirectory(plotDir);
      efficiencyTime->SetStatisticOption(TEfficiency::kFWilson);
      _efficiencyTime.push_back(efficiencyTime);
    }
  }
}

}
