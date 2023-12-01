/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/CDCWireHitVarSet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCWireHitVarSet::CDCWireHitVarSet() : Super()
{
}

void CDCWireHitVarSet::initialize()
{
  Super::initialize();
}

bool CDCWireHitVarSet::extract(const CDCWireHit* wireHit)
{
  const auto* cdcHit = wireHit->getHit();
  var<named("tot")>() = cdcHit->getTOT();
  var<named("adc")>() = cdcHit->getADCCount();
  var<named("tdc")>() = cdcHit->getTDCCount();

  return true;
}
