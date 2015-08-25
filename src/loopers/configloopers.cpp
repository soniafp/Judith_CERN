#include "configloopers.h"

#include "coarsealign.h"
#include "coarsealigndut.h"
#include "finealign.h"
#include "chi2align.h"
#include "finealigndut.h"
#include "synchronize.h"
#include "synchronizerms.h"
#include "noisescan.h"
#include "../configparser.h"

namespace Loopers {

void configChi2Align(const ConfigParser& config, Chi2Align& chi2Align)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Chi2 Align")) continue;

    if (!row->key.compare("max chi2"))
      chi2Align.setMaxChi2(ConfigParser::valueToNumerical(row->value));
    else
      throw "Loopers: can't parse fine align row";
  }
}

void configFineAlign(const ConfigParser& config, FineAlign& fineAlign)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Fine Align")) continue;

    if (!row->key.compare("num iterations"))
      fineAlign.setNumIteratioins(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num bins y"))
      fineAlign.setNumBinsY(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num pixels"))
      fineAlign.setNumPixX(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("bins per pixel"))
      fineAlign.setBinsPerPix(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num pixels broad"))
      fineAlign.setNumPixXBroad(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("bins per pixel broad"))
      fineAlign.setBinsPerPixBroad(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("display fits"))
      fineAlign.setDisplayFits(ConfigParser::valueToLogical(row->value));
    else if (!row->key.compare("relaxation"))
      fineAlign.setRelaxation(ConfigParser::valueToNumerical(row->value));
    else
      throw "Loopers: can't parse fine align row";
  }
}

void configFineAlign(const ConfigParser& config, FineAlignDut& fineAlign)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Fine Align")) continue;

    if (!row->key.compare("num iterations"))
      fineAlign.setNumIteratioins(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num bins y"))
      fineAlign.setNumBinsY(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num pixels"))
      fineAlign.setNumPixX(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("bins per pixel"))
      fineAlign.setBinsPerPix(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("num pixels broad"))
      fineAlign.setNumPixXBroad(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("bins per pixel broad"))
      fineAlign.setBinsPerPixBroad(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("display fits"))
      fineAlign.setDisplayFits(ConfigParser::valueToLogical(row->value));
    else if (!row->key.compare("relaxation"))
      fineAlign.setRelaxation(ConfigParser::valueToNumerical(row->value));
    else
      throw "Loopers: can't parse fine align row";
  }
}

void configCoarseAlign(const ConfigParser& config, CoarseAlign& coarseAlign)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Coarse Align")) continue;

    if (!row->key.compare("display fits"))
      coarseAlign.setDisplayFits(ConfigParser::valueToLogical(row->value));
    else
      throw "Loopers: can't parse coarse align row";
  }
}

void configCoarseAlign(const ConfigParser& config, CoarseAlignDut& coarseAlign)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Coarse Align")) continue;

    if (!row->key.compare("display fits"))
      coarseAlign.setDisplayFits(ConfigParser::valueToLogical(row->value));
    else
      throw "Loopers: can't parse coarse align row";
  }
}

void configNoiseScan(const ConfigParser& config, NoiseScan& noiseScan)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Noise Scan")) continue;

    if (!row->key.compare("max factor"))
      noiseScan.setMaxFactor(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max occupancy"))
      noiseScan.setMaxOccupancy(ConfigParser::valueToNumerical(row->value));
    else
      throw "Loopers: can't parse coarse align row";
  }
}

void configSynchronize(const ConfigParser& config, Synchronize& sync)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("Synchronize")) continue;

    if (!row->key.compare("sync sample"))
      sync.setSyncSample(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max offset"))
      sync.setMaxOffset(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("threshold"))
      sync.setThreshold(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("buffer size"))
      sync.setBufferSize(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("pre discards"))
      sync.setPreDiscards(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max regular fails"))
      sync.setMaxConsecutiveFails(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max large attempts"))
      sync.setMaxLargeSyncAttempts(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("display"))
      sync.setDisplayDistributions(ConfigParser::valueToLogical(row->value));
    else
      throw "Loopers: can't parse synchronize row";
  }
}

  void configSynchronizeRMS(const ConfigParser& config, SynchronizeRMS& sync)
{
  for (unsigned int i = 0; i < config.getNumRows(); i++)
  {
    const ConfigParser::Row* row = config.getRow(i);

    if (row->isHeader) continue;
    if (row->header.compare("SynchronizeRMS")) continue;

    if (!row->key.compare("sync sample"))
      sync.setSyncSample(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max offset"))
      sync.setMaxOffset(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("threshold"))
      sync.setThreshold(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("buffer size"))
      sync.setBufferSize(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("pre discards"))
      sync.setPreDiscards(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max regular fails"))
      sync.setMaxConsecutiveFails(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("max large attempts"))
      sync.setMaxLargeSyncAttempts(ConfigParser::valueToNumerical(row->value));
    else if (!row->key.compare("display"))
      sync.setDisplayDistributions(ConfigParser::valueToLogical(row->value));
    else
      throw "Loopers: can't parse synchronize row";
  }
}

}
