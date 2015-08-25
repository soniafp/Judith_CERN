#include "inputargs.h"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <iomanip>

using namespace std;

InputArgs::InputArgs() :
  _inFileRef(""),
  _outFileRef(""),
  _inFileDUT(""),
  _outFileDUT(""),
  _results(""),
  _cfgRef(""),
  _cfgDUT(""),
  _cfgTestbeam(""),
  _command(""),
  _numEvents(0),
  _eventOffset(0),
  _synchroEventsOffset(1),  
  _noBar(false)
{ }

void InputArgs::usage()
{
  cout << left;
  cout << "\nJudith usage: ./Judith -c command [arguments]" << endl;

  const unsigned int w1 = 16;
  cout << "\nCommands (required arguments, [optional arguments]):\n";
  cout << setw(w1) << "  convert"
       << " : convert KarTel data (-i, -o, [-r, -n])\n";
  cout << setw(w1) << "  synchronize"
       << " : synchronize DUT and ref. files (-i, -o, -I, -O, -r, -d, -t, [-n, -s])\n";
  cout << setw(w1) << "  noiseScan"
       << " : scan for noisy pixels (-i, -r, -t, [-n, -s])\n";
  cout << setw(w1) << "  applyMask"
       << " : mask sensors and noisy pixels (-i, -o, -r, -t, [-n, -s])\n";
  cout << setw(w1) << "  coarseAlign"
       << " : coarse align device planes (-i, -r, -t, [-n, -s])\n";
  cout << setw(w1) << "  chi2Align"
       << " : chi2 align device planes (-i, -r, -t, [-n, -s])\n";
  cout << setw(w1) << "  fineAlign"
       << " : fine align device planes (-i, -r, -t, [-n, -s])\n";
  cout << setw(w1) << "  coarseAlignDUT"
       << " : coarse align DUT to ref. device (-i, -I, -r, -d, -t, [-n, -s, -y])\n";
  cout << setw(w1) << "  fineAlignDUT"
       << " : fine align DUT planes to ref. tracks (-i, -I, -r, -d, -t, [-n, -s])\n";
  cout << setw(w1) << "  process"
       << " : generate cluster and/or tracks (-i, -o, -r, -t, [-R, -n, -s])\n";
  cout << setw(w1) << "  analysis"
       << " : analyze device events (-i, -r, -t, -R, [-n, -s])\n";
  cout << setw(w1) << "  analysisDUT"
       << " : analyze DUT events with ref. data (-i, -I, -r, -d, -t, -R, [-n, -s])\n";
  cout << endl;

  const unsigned int w2 = 13;
  cout << "Arguments:\n";
  cout << "  -i  " << setw(w2) << "--input" << " : path to data input\n";
  cout << "  -o  " << setw(w2) << "--output" << " : path to store data output\n";
  cout << "  -I  " << setw(w2) << "--inputDUT" << " : path to DUT data input\n";
  cout << "  -O  " << setw(w2) << "--outputDUT" << " : path to store DUT data output\n";
  cout << "  -R  " << setw(w2) << "--results" << " : path to store analyzed results\n";
  cout << "  -r  " << setw(w2) << "--cfgRef" << " : path to reference configuration\n";
  cout << "  -d  " << setw(w2) << "--cfgDUT" << " : path to DUT configuration\n";
  cout << "  -t  " << setw(w2) << "--cfgTestbeam" << " : path to testbeam configuration\n";
  cout << "  -n  " << setw(w2) << "--numEvents" << " : number of events to process\n";
  cout << "  -s  " << setw(w2) << "--eventOffset" << " : starting at this event\n";
  cout << "  -y  " << setw(w2) << "--synchroOffset" << " : skipping this many events for the telescope\n";  
  cout << "  -h  " << setw(w2) << "--help" << " : print this help message\n";
  cout << endl;

  cout << "Additional options:\n";
  cout << "  -b  " << setw(w2) << "--noBar" << " : do not print the progress bar\n";
  cout << endl;

  cout << right;
}

void InputArgs::parseArgs(int* argc, char** argv)
{
  // input argument handling
  string arg;

  const unsigned int w = 20;
  cout << left;

  if (*argc > 1)
  {
    for ( int i = 1; i < *argc; i++ )
    {
      arg = argv[i];

      if ( (!arg.compare("-i") || !arg.compare("--input")) &&
         !_inFileRef.compare("") )
      {
        _inFileRef = argv[++i];
        cout << setw(w) << "  input name" << " : " << _inFileRef << endl;
      }
      else if ( (!arg.compare("-o") || !arg.compare("--output")) &&
                !_outFileRef.compare("") )
      {
        _outFileRef = argv[++i];
        cout << setw(w) << "  output name" << " : " << _outFileRef << endl;
      }
      else if ( (!arg.compare("-I") || !arg.compare("--inputDUT")) &&
                !_inFileDUT.compare("") )
      {
        _inFileDUT = argv[++i];
        cout << setw(w) << "  input name DUT" << " : " << _inFileDUT << endl;
      }
      else if ( (!arg.compare("-O") || !arg.compare("--outputDUT")) &&
                !_outFileDUT.compare("") )
      {
        _outFileDUT = argv[++i];
        cout << setw(w) << "  output name DUT" << " : " << _outFileDUT << endl;
      }
      else if ( (!arg.compare("-R") || !arg.compare("--results")) &&
                !_results.compare("") )
      {
        _results = argv[++i];
        cout << setw(w) << "  results name" << " : " << _results << endl;
      }
      else if ( (!arg.compare("-c") || !arg.compare("--command")) &&
                !_command.compare("") )
      {
        _command = argv[++i];
        cout << setw(w) << "  command" << " : " << _command << endl;
      }
      else if ( (!arg.compare("-r") || !arg.compare("--cfgRef")) &&
                 !_cfgRef.compare("") )
      {
        _cfgRef = argv[++i];
        cout << setw(w) << "  cfgRef" << " : " << _cfgRef << endl;
      }
      else if ( (!arg.compare("-d") || !arg.compare("--cfgDUT")) &&
                 !_cfgDUT.compare("") )
      {
        _cfgDUT = argv[++i];
        cout << setw(w) << "  cfgDUT" << " : " << _cfgDUT << endl;
      }
      else if ( (!arg.compare("-t") || !arg.compare("--cfgTestbeam")) &&
                 !_cfgTestbeam.compare("") )
      {
        _cfgTestbeam = argv[++i];
        cout << setw(w) << "  cfgTestbeam" << " : " << _cfgTestbeam << endl;
      }
      else if ( (!arg.compare("-n") || !arg.compare("--numEvents")) &&
                 !_numEvents )
      {
        _numEvents = atoi( argv[++i] );
        cout << setw(w) << "  numEvents" << " : " << _numEvents << endl;
      }
      else if ( (!arg.compare("-s") || !arg.compare("--eventOffset")) &&
                 !_eventOffset )
      {
        _eventOffset = atoi( argv[++i] );
        cout << setw(w) << "  eventOffset" << " : " << _eventOffset << endl;
      }
      else if ( (!arg.compare("-y") || !arg.compare("--synchroOffset")))
      {
        _synchroEventsOffset = atoi( argv[++i] );
        cout << setw(w) << "  synchroOffset" << " : " << _synchroEventsOffset << endl;
      }      
      else if ( (!arg.compare("-b") || !arg.compare("--noBar")) &&
                !_noBar)
      {
        _noBar = true;
        cout << setw(w) << "  noBar" << " : true" << endl;
      }
      else if ( (!arg.compare("-h")) || !arg.compare("--help"))
      {
        usage();
      }
      else
      {
        usage();
        cout << "Unknown or duplicate argument! " << arg << endl;
      }
    }
  }

  cout << endl; // Space before the next cout
  cout << right;
}

string InputArgs::getInputRef() const { return _inFileRef; }
string InputArgs::getOutputRef() const { return _outFileRef; }
string InputArgs::getInputDUT() const { return _inFileDUT; }
string InputArgs::getOutputDUT() const { return _outFileDUT; }
string InputArgs::getResults() const { return _results; }
string InputArgs::getCommand() const { return _command; }
string InputArgs::getCfgRef() const { return _cfgRef; }
string InputArgs::getCfgDUT() const { return _cfgDUT; }
string InputArgs::getCfgTestbeam() const { return _cfgTestbeam; }
ULong64_t InputArgs::getNumEvents() const { return _numEvents; }
ULong64_t InputArgs::getEventOffset() const { return _eventOffset; }
Long64_t InputArgs::getSynchroEventsOffset() const { return _synchroEventsOffset; }
bool InputArgs::getNoBar() const { return _noBar; }
