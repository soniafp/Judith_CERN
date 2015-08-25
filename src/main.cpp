#include <iostream>
#include <vector>
#include <string.h>
#include <iomanip>

#include <TFile.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TColor.h>

#include "storage/storageio.h"
#include "storage/event.h"
#include "storage/plane.h"
#include "storage/cluster.h"
#include "storage/hit.h"
#include "converters/kartelconvert.h"
#include "mechanics/configmechanics.h"
#include "mechanics/sensor.h"
#include "mechanics/device.h"
#include "mechanics/noisemask.h"
#include "mechanics/alignment.h"
#include "processors/configprocessors.h"
#include "processors/eventdepictor.h"
#include "processors/clustermaker.h"
#include "processors/trackmaker.h"
#include "processors/trackmatcher.h"
#include "processors/processors.h"
#include "analyzers/configanalyzers.h"
#include "loopers/analysis.h"
#include "loopers/analysisdut.h"
#include "loopers/coarsealign.h"
#include "loopers/coarsealigndut.h"
#include "loopers/finealign.h"
#include "loopers/chi2align.h"
#include "loopers/finealigndut.h"
#include "loopers/applymask.h"
#include "loopers/noisescan.h"
#include "loopers/processevents.h"
#include "loopers/synchronize.h"
#include "loopers/synchronizerms.h"
#include "loopers/configloopers.h"
#include "configparser.h"
#include "inputargs.h"

using namespace std;

void chi2Align(const char* inputName, ULong64_t startEvent, ULong64_t numEvents,
               const char* deviceCfg, const char* tbCfg)
{
  try
  {
    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);

    ConfigParser deviceConfig(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);

    if (device->getAlignment()) device->getAlignment()->readFile();

    unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
      
    std::vector<bool> planeMask;
    planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
//planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    Storage::StorageIO input(inputName, Storage::INPUT, 0, treeMask, &planeMask);

    Loopers::Chi2Align looper(device, clusterMaker, &input,
                              startEvent, numEvents);
    Loopers::configChi2Align(runConfig, looper);
    looper.loop();

    delete device;
    delete clusterMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void fineAlign(const char* inputName, ULong64_t startEvent, ULong64_t numEvents,
               const char* deviceCfg, const char* tbCfg)
{
  try
  {
    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);

    ConfigParser deviceConfig(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);

    if (device->getAlignment()) device->getAlignment()->readFile();

    Processors::TrackMaker* trackMaker = Processors::generateTrackMaker(runConfig, true);

    unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
    std::vector<bool> planeMask;
    planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    //planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    Storage::StorageIO input(inputName, Storage::INPUT, 0, treeMask, &planeMask);

    Loopers::FineAlign looper(device, clusterMaker, trackMaker, &input,
                              startEvent, numEvents);
    Loopers::configFineAlign(runConfig, looper);
    looper.loop();

    delete trackMaker;
    delete device;
    delete clusterMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void fineAlignDUT(const char* refInputName, const char* dutInputName,
                  ULong64_t startEvent, ULong64_t numEvents,
                  const char* refDeviceCfg, const char* dutDeviceCfg,
                  const char* tbCfg)
{
  try
  {
    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);

    ConfigParser refConfig(refDeviceCfg);
    Mechanics::Device* refDevice = Mechanics::generateDevice(refConfig);

    ConfigParser dutConfig(dutDeviceCfg);
    Mechanics::Device* dutDevice = Mechanics::generateDevice(dutConfig);

    Processors::TrackMaker* trackMaker = Processors::generateTrackMaker(runConfig, true);

    unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
    Storage::StorageIO refInput(refInputName, Storage::INPUT, 0, treeMask);
    Storage::StorageIO dutInput(dutInputName, Storage::INPUT, 0, treeMask);

    // Get the current alignment (coarse alignment should've been performed)
    if (refDevice->getAlignment()) refDevice->getAlignment()->readFile();
    if (dutDevice->getAlignment()) dutDevice->getAlignment()->readFile();

    Loopers::FineAlignDut looper(refDevice, dutDevice, clusterMaker, trackMaker,
                                 &refInput, &dutInput, startEvent, numEvents);
    Loopers::configFineAlign(runConfig, looper);
    looper.loop();

    delete trackMaker;
    delete refDevice;
    delete dutDevice;
    delete clusterMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void coarseAlign(const char* inputName, ULong64_t startEvent, ULong64_t numEvents,
                 const char* deviceCfg, const char* tbCfg)
{
  try
  {
    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);

    ConfigParser deviceConfig(deviceCfg);
    //  deviceConfig.print();
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);
     // device->print();// device has 6 planes
    unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
      
    std::vector<bool> planeMask;
    planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    //planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    Storage::StorageIO input(inputName, Storage::INPUT, 0, treeMask, &planeMask);

    Loopers::CoarseAlign looper(device, clusterMaker, &input, startEvent, numEvents);
    Loopers::configCoarseAlign(runConfig, looper);
    looper.loop();

    delete device;
    delete clusterMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void coarseAlignDUT(const char* refInputName, const char* dutInputName,
                    ULong64_t startEvent, ULong64_t numEvents,  Long64_t skipEvent,
                    const char* refDeviceCfg, const char* dutDeviceCfg,
                    const char* tbCfg)
{
  try
  {
    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);

    ConfigParser refConfig(refDeviceCfg);
    Mechanics::Device* refDevice = Mechanics::generateDevice(refConfig);

    ConfigParser dutConfig(dutDeviceCfg);
    Mechanics::Device* dutDevice = Mechanics::generateDevice(dutConfig);

    unsigned int treeMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
    Storage::StorageIO refInput(refInputName, Storage::INPUT, 0, treeMask);
    Storage::StorageIO dutInput(dutInputName, Storage::INPUT, 0, treeMask);

    if (refDevice->getAlignment()) refDevice->getAlignment()->readFile();

    Loopers::CoarseAlignDut looper(refDevice, dutDevice, clusterMaker,
                                   &refInput, &dutInput, startEvent, numEvents, skipEvent);
    Loopers::configCoarseAlign(runConfig, looper);
    looper.loop();

    delete refDevice;
    delete dutDevice;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void process(const char* inputName, const char* outputName,
             ULong64_t startEvent, ULong64_t numEvents,
             const char* deviceCfg, const char* tbCfg,
             const char* resultsName)
{
  try
  {
   
    ConfigParser deviceConfig(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);
    std::cout << "S.F #######"<< std::endl;

    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);
  
    Processors::TrackMaker* trackMaker = 0;
    if (device->getNumSensors() > 2)
     trackMaker = Processors::generateTrackMaker(runConfig);
    device->print();
      
    std::cout << "S.F #######1"<< std::endl;
    unsigned int inMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
      std::vector<bool> planeMask;
      //planeMask.push_back(true);
      planeMask.push_back(false);
      planeMask.push_back(false);
      planeMask.push_back(false);
      //planeMask.push_back(true);
      planeMask.push_back(false);
      planeMask.push_back(false);
      planeMask.push_back(false);
    Storage::StorageIO input(inputName, Storage::INPUT, 0, inMask, &planeMask);
    
    unsigned int outMask = 0;
    if (device->getNumSensors() <= 2) outMask = Storage::Flags::TRACKS;
    Storage::StorageIO output(outputName, Storage::OUTPUT, device->getNumSensors(), outMask);
    
    if (device->getAlignment()) device->getAlignment()->readFile();

    Loopers::ProcessEvents looper(device, &output, clusterMaker, trackMaker,
                                  &input, startEvent, numEvents);
    const Storage::Event* start = input.readEvent(looper.getStartEvent());
    const Storage::Event* end = input.readEvent(looper.getEndEvent());
    device->setTimeStart(start->getTimeStamp());
    device->setTimeEnd(end->getTimeStamp());
    delete start;
    delete end;
    
    TFile* results = 0;
    if (strlen(resultsName))
      results = new TFile(resultsName, "RECREATE");
    std::cout << "S.F #######2"<< std::endl;
    Analyzers::configLooper(runConfig, &looper, device, 0, results);
    std::cout << "S.F #######3"<< std::endl;
    looper.loop();

    if (results)
    {
      results->Write();
      delete results;
    }

    delete device;
    delete clusterMaker;
    if (trackMaker) delete trackMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void applyMask(const char* inputName, const char* outputName,
               ULong64_t startEvent, ULong64_t numEvents,
               const char* deviceCfg, const char* tbCfg)
{
  try
  {
    ConfigParser deviceConfig(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);
    device->getNoiseMask()->readMask();

    ConfigParser runConfig(tbCfg);

    unsigned int inMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
    Storage::StorageIO input(inputName, Storage::INPUT, 0, inMask,
                             device->getSensorMask());

    Storage::StorageIO output(outputName, Storage::OUTPUT, device->getNumSensors(),
                              inMask);

    Loopers::ApplyMask looper(device, &output, &input, startEvent, numEvents);

    const Storage::Event* start = input.readEvent(looper.getStartEvent());
    const Storage::Event* end = input.readEvent(looper.getEndEvent());
    device->setTimeStart(start->getTimeStamp());
    device->setTimeEnd(end->getTimeStamp());
    delete start;
    delete end;

    looper.loop();

    delete device;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void convert(const char* input, const char* output, Long64_t triggers,
             const char* deviceCfg = "")
{
  try
  {
    Mechanics::Device* device = 0;
    if (strlen(deviceCfg))
    {
      ConfigParser config(deviceCfg);
      device = Mechanics::generateDevice(config);
      device->getNoiseMask()->readMask();
    }
    Converters::KartelConvert convert(input, output, device);
    convert.processFile(triggers);
  }
  catch (const char* e)
  {
    cout << "ERR :: " <<  e << endl;
  }
}

void synchronize(const char* refInputName, const char* dutInputName,
                 const char* refOutputName, const char* dutOutputName,
                 ULong64_t startEvent, ULong64_t numEvents,
                 const char* refDeviceCfg, const char* dutDeviceCfg,
                 const char* tbCfg)
{
  try
  {
    
    ConfigParser refConfig(refDeviceCfg);
    Mechanics::Device* refDevice = Mechanics::generateDevice(refConfig);
    ConfigParser dutConfig(dutDeviceCfg);
    
    Mechanics::Device* dutDevice = Mechanics::generateDevice(dutConfig);
      
    if (refDevice->getAlignment()) refDevice->getAlignment()->readFile();
    if (dutDevice->getAlignment()) dutDevice->getAlignment()->readFile();
 
    unsigned int inMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;
    std::vector<bool> planeMask;
    planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    //planeMask.push_back(true);
    planeMask.push_back(false);
    planeMask.push_back(false);
    planeMask.push_back(false);
    ConfigParser runConfig(tbCfg);
    Storage::StorageIO refInput(refInputName, Storage::INPUT,0 , inMask, &planeMask);
    Storage::StorageIO dutInput(dutInputName, Storage::INPUT);
    Storage::StorageIO refOutput(refOutputName, Storage::OUTPUT,
                                 refInput.getNumPlanes());

    Storage::StorageIO dutOutput(dutOutputName, Storage::OUTPUT,
                                 dutInput.getNumPlanes());
    Loopers::Synchronize looper(refDevice, dutDevice, &refOutput, &dutOutput,
                                &refInput, &dutInput, startEvent, numEvents);

    Loopers::configSynchronize(runConfig, looper);
    looper.loop();
    delete refDevice;
    delete dutDevice;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void synchronizeRMS(const char* refInputName, const char* dutInputName,
                 const char* refOutputName, const char* dutOutputName,
                 ULong64_t startEvent, ULong64_t numEvents,
                 const char* refDeviceCfg, const char* dutDeviceCfg,
                 const char* tbCfg)
{
  try
  {

    ConfigParser runConfig(tbCfg);
    Processors::ClusterMaker* clusterMaker = Processors::generateClusterMaker(runConfig);
    
    ConfigParser refConfig(refDeviceCfg);
    Mechanics::Device* refDevice = Mechanics::generateDevice(refConfig);

    ConfigParser dutConfig(dutDeviceCfg);
    Mechanics::Device* dutDevice = Mechanics::generateDevice(dutConfig);
      
 
    unsigned int inMask = Storage::Flags::TRACKS | Storage::Flags::CLUSTERS;

    Storage::StorageIO refInput(refInputName, Storage::INPUT, 0, inMask);
    Storage::StorageIO dutInput(dutInputName, Storage::INPUT, 0, inMask);
    Storage::StorageIO refOutput(refOutputName, Storage::OUTPUT,
                                 refInput.getNumPlanes());

    Storage::StorageIO dutOutput(dutOutputName, Storage::OUTPUT,
                                 dutInput.getNumPlanes());

    if (refDevice->getAlignment()) refDevice->getAlignment()->readFile();
    if (dutDevice->getAlignment()) dutDevice->getAlignment()->readFile();
    
    Loopers::SynchronizeRMS looper(refDevice, dutDevice, clusterMaker, &refOutput, &dutOutput,
				   &refInput, &dutInput, startEvent, numEvents);

    Loopers::configSynchronizeRMS(runConfig, looper);
    looper.loop();

    // cleanup
    delete refDevice;
    delete dutDevice;
    delete clusterMaker;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void analysisDUT(const char* refInputName, const char* dutInputName,
                 ULong64_t startEvent, ULong64_t numEvents,
                 const char* refDeviceCfg, const char* dutDeviceCfg,
                 const char* tbCfg, const char* resultsName)
{
  try
  {
    ConfigParser refConfig(refDeviceCfg);
    Mechanics::Device* refDevice = Mechanics::generateDevice(refConfig);

    ConfigParser dutConfig(dutDeviceCfg);
    Mechanics::Device* dutDevice = Mechanics::generateDevice(dutConfig);

    ConfigParser runConfig(tbCfg);

    Storage::StorageIO refInput(refInputName, Storage::INPUT);
    Storage::StorageIO dutInput(dutInputName, Storage::INPUT);

    if (refDevice->getAlignment()) refDevice->getAlignment()->readFile();
    if (dutDevice->getAlignment()) dutDevice->getAlignment()->readFile();

    Processors::TrackMatcher* trackMatcher = new Processors::TrackMatcher(dutDevice);
    
    Loopers::AnalysisDut looper(&refInput, &dutInput, trackMatcher, startEvent, numEvents);

    const Storage::Event* start = refInput.readEvent(looper.getStartEvent());
    const Storage::Event* end = refInput.readEvent(looper.getEndEvent());
    refDevice->setTimeStart(start->getTimeStamp());
    refDevice->setTimeEnd(end->getTimeStamp());
    delete start;
    delete end;

    TFile* results = 0;
    if (strlen(resultsName))
      results = new TFile(resultsName, "RECREATE");
    Analyzers::configLooper(runConfig, &looper, refDevice, dutDevice, results);
    looper.loop();
    if (results)
    {
      results->Write();
      delete results;
    }
    delete trackMatcher;
    delete refDevice;
    delete dutDevice;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void analysis(const char* inputName,
              ULong64_t startEvent, ULong64_t numEvents,
              const char* deviceCfg,
              const char* tbCfg, const char* resultsName)
{
  try
  {
    ConfigParser deviceConfig(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(deviceConfig);

    ConfigParser runConfig(tbCfg);

    Storage::StorageIO input(inputName, Storage::INPUT);

    if (device->getAlignment()) device->getAlignment()->readFile();

    Loopers::Analysis looper(&input, startEvent, numEvents);

    const Storage::Event* start = input.readEvent(looper.getStartEvent());
    const Storage::Event* end = input.readEvent(looper.getEndEvent());
    device->setTimeStart(start->getTimeStamp());
    device->setTimeEnd(end->getTimeStamp());
    delete start;
    delete end;

    TFile* results = 0;
    if (strlen(resultsName))
      results = new TFile(resultsName, "RECREATE");

    Analyzers::configLooper(runConfig, &looper, device, 0, results);

    looper.loop();

    if (results)
    {
      results->Write();
      delete results;
    }

    delete device;
  }
  catch (const char* e)
  {
    cout << "ERR :: " << e << endl;
  }
}

void noiseScan(const char* inputName, const char* deviceCfg,
               const char* tbCfg,
               ULong64_t startEvent, ULong64_t numEvents)
{
  try
  {
    ConfigParser config(deviceCfg);
    Mechanics::Device* device = Mechanics::generateDevice(config);

    ConfigParser runConfig(tbCfg);

    Storage::StorageIO input(inputName, Storage::INPUT, device->getNumSensors());

    Loopers::NoiseScan looper(device, &input, startEvent, numEvents);
    Loopers::configNoiseScan(runConfig, looper);
    looper.loop();

    delete device;
  }
  catch (const char* e)
  {
    cout << "ERR :: " <<  e << endl;
  }
}

void printDevice(const char* configName)
{
  try
  {
    ConfigParser config(configName);
    Mechanics::Device* device = Mechanics::generateDevice(config);
    device->print();
    delete device;
  }
  catch (const char* e)
  {
    cout << "ERR :: " <<  e << endl;
  }
}

int main(int argc, char** argv)
{
  cout << "\nStarting Judith\n" << endl;

  TApplication app("App", 0, 0);
  gStyle->SetOptStat("mre");

  InputArgs inArgs;
  inArgs.parseArgs(&argc, argv);

  // Static variables

  if (inArgs.getNoBar()) Loopers::Looper::noBar = true;

  if ( !inArgs.getCommand().compare("convert") )
  {
    convert( // converts kartel->ROOT using a noise mask specified in cfg file
             // if cfg file is not present --> no noise is subtracted
             // convert NumEvents (-1,0=ALL)
                inArgs.getInputRef().c_str(),
                inArgs.getOutputRef().c_str(),
                inArgs.getNumEvents() ? inArgs.getNumEvents() : -1,
                inArgs.getCfgRef().c_str() );
  }
  else if ( !inArgs.getCommand().compare("synchronize") )
  {
    synchronize( // synchronizes DUT with Ref (2 inputs, 2 outputs)
                 // start at EvOffset, do NumEvents (0=ALL)
                 // 2 configs (Ref and DUT) synched data goes to
                 // OutputRef/DUT
                inArgs.getInputRef().c_str(),
                inArgs.getInputDUT().c_str(),
                inArgs.getOutputRef().c_str(),
                inArgs.getOutputDUT().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgDUT().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("synchronizeRMS") )
  {
    synchronizeRMS( // synchronizes DUT with Ref (2 inputs, 2 outputs)
                 // start at EvOffset, do NumEvents (0=ALL)
                 // 2 configs (Ref and DUT) synched data goes to
                 // OutputRef/DUT
                inArgs.getInputRef().c_str(),
                inArgs.getInputDUT().c_str(),
                inArgs.getOutputRef().c_str(),
                inArgs.getOutputDUT().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgDUT().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }  
  else if ( !inArgs.getCommand().compare("noiseScan") )
  {
    noiseScan( // produse a noise mask with a specified cut (prompted for)
                inArgs.getInputRef().c_str(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents() );
  }
  else if ( !inArgs.getCommand().compare("applyMask") )
  {
    applyMask(
                inArgs.getInputRef().c_str(),
                inArgs.getOutputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("coarseAlign") )
  {
    coarseAlign( // coarse align a detector (Ref or DUT)
                inArgs.getInputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("fineAlign") )
  {
    fineAlign( // fine align Ref detector
                inArgs.getInputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("chi2Align") )
  {
    chi2Align( // chi2 alignment
                inArgs.getInputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("coarseAlignDUT") )
  {
    coarseAlignDUT( // coarse align DUT to Ref detecor
                inArgs.getInputRef().c_str(),
                inArgs.getInputDUT().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
		inArgs.getSynchroEventsOffset(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgDUT().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("fineAlignDUT") )
  {
    fineAlignDUT( // fine align DUT to Ref detector
                inArgs.getInputRef().c_str(),
                inArgs.getInputDUT().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgDUT().c_str(),
                inArgs.getCfgTestbeam().c_str() );
  }
  else if ( !inArgs.getCommand().compare("process") )
  {
    process( // makes clusters and tracks in detector planes (DUT and Red).
             // If no. of planes < 3, no tracks are made
                inArgs.getInputRef().c_str(),
                inArgs.getOutputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str(),
                inArgs.getResults().c_str() );
  }
  else if ( !inArgs.getCommand().compare("analysis") )
  {
    analysis( // runs over events and stores histos in OutputRef
              // (runs over one device: DUT OR ref)
                inArgs.getInputRef().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgTestbeam().c_str(),
                inArgs.getResults().c_str() );
  }
  else if ( !inArgs.getCommand().compare("analysisDUT") )
  {
    analysisDUT( // fills all the histograms: correletions, efficiencies, tracks,
                 // alignments plots, residuals,...
                inArgs.getInputRef().c_str(),
                inArgs.getInputDUT().c_str(),
                inArgs.getEventOffset(),
                inArgs.getNumEvents(),
                inArgs.getCfgRef().c_str(),
                inArgs.getCfgDUT().c_str(),
                inArgs.getCfgTestbeam().c_str(),
                inArgs.getResults().c_str() );
  }
  else if (inArgs.getCommand().size())
  {
    inArgs.usage();
    std::cout << "Unknown command! " << inArgs.getCommand() << std::endl;
  }

  cout << "\nEnding Judith\n" << endl;

  return 0;
}
