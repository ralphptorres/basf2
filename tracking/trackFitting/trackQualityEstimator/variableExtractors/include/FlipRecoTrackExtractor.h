/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/variableExtractors/VariableExtractor.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <genfit/FitStatus.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {
  /// class to extract results from qualityEstimation
  class FlipRecoTrackExtractor : public VariableExtractor {
  public:

    /// Define names of variables that get extracted
    explicit FlipRecoTrackExtractor(std::vector<Named<float*>>& variableSet, const std::string& prefix = ""):
      VariableExtractor(), m_prefix(prefix)
    {
      /// Adding variables for 1st mva
      addVariable(prefix + "seed_pz_estimate", variableSet);
      addVariable(prefix + "seed_pz_variance", variableSet);
      addVariable(prefix + "seed_z_estimate", variableSet);
      addVariable(prefix + "seed_tan_lambda_estimate", variableSet);
      addVariable(prefix + "seed_pt_estimate", variableSet);
      addVariable(prefix + "seed_x_estimate", variableSet);
      addVariable(prefix + "seed_y_estimate", variableSet);
      addVariable(prefix + "seed_py_variance", variableSet);
      addVariable(prefix + "seed_d0_estimate", variableSet);
      addVariable(prefix + "seed_omega_variance", variableSet);
      addVariable(prefix + "svd_layer6_clsTime", variableSet);
      addVariable(prefix + "seed_tan_lambda_variance", variableSet);
      addVariable(prefix + "seed_z_variance", variableSet);
      addVariable(prefix + "n_svd_hits", variableSet);
      addVariable(prefix + "n_cdc_hits", variableSet);
      addVariable(prefix + "svd_layer3_positionSigma", variableSet);
      addVariable(prefix + "first_cdc_layer", variableSet);
      addVariable(prefix + "last_cdc_layer", variableSet);
      addVariable(prefix + "InOutArmTimeDifference", variableSet);
      addVariable(prefix + "InOutArmTimeDifferenceError", variableSet);
      addVariable(prefix + "inGoingArmTime", variableSet);
      addVariable(prefix + "inGoingArmTimeError", variableSet);
      addVariable(prefix + "outGoingArmTime", variableSet);
      addVariable(prefix + "outGoingArmTimeError", variableSet);

    }

    /// extract the actual variables and write into a variable set
    void extractVariables(RecoTrack& recoTrack)
    {
      const auto& cdcHitList = recoTrack.getSortedCDCHitList();
      const RecoTrack* svdcdcRecoTrack =  recoTrack.getRelated<RecoTrack>("svdcdcRecoTracks");
      // In case the CDC hit list is empty, or there is no svdcdcRecoTrack, set arbitrary default value and return
      if (cdcHitList.size() == 0 or not svdcdcRecoTrack) {
        const float errorvalue = -99999.9;
        m_variables.at(m_prefix + "InOutArmTimeDifference") = errorvalue;
        m_variables.at(m_prefix + "InOutArmTimeDifferenceError") = errorvalue;
        m_variables.at(m_prefix + "inGoingArmTime") = errorvalue;
        m_variables.at(m_prefix + "inGoingArmTimeError") = errorvalue;
        m_variables.at(m_prefix + "outGoingArmTime") = errorvalue;
        m_variables.at(m_prefix + "outGoingArmTimeError") = errorvalue;
        m_variables.at(m_prefix + "first_cdc_layer") =  errorvalue;
        m_variables.at(m_prefix + "last_cdc_layer") =  errorvalue;
        m_variables.at(m_prefix + "n_svd_hits") = errorvalue;
        m_variables.at(m_prefix + "n_cdc_hits") = errorvalue;
        m_variables.at(m_prefix + "seed_pz_variance") = errorvalue;
        m_variables.at(m_prefix + "seed_pz_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_z_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_tan_lambda_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_pt_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_x_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_y_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_py_variance") = errorvalue;
        m_variables.at(m_prefix + "seed_d0_estimate") = errorvalue;
        m_variables.at(m_prefix + "seed_omega_variance") = errorvalue;
        m_variables.at(m_prefix + "seed_tan_lambda_variance") = errorvalue;
        m_variables.at(m_prefix + "seed_z_variance") = errorvalue;
        m_variables.at(m_prefix + "svd_layer3_positionSigma") = errorvalue;
        m_variables.at(m_prefix + "svd_layer6_clsTime") = errorvalue;
        return;
      }

      m_variables.at(m_prefix + "InOutArmTimeDifference") = recoTrack.getInOutArmTimeDifference();
      m_variables.at(m_prefix + "InOutArmTimeDifferenceError") = recoTrack.getInOutArmTimeDifferenceError();
      m_variables.at(m_prefix + "inGoingArmTime") = recoTrack.getIngoingArmTime();
      m_variables.at(m_prefix + "inGoingArmTimeError") = recoTrack.getIngoingArmTimeError();
      m_variables.at(m_prefix + "outGoingArmTime") = recoTrack.getOutgoingArmTime();
      m_variables.at(m_prefix + "outGoingArmTimeError") = recoTrack.getOutgoingArmTimeError();

      m_variables.at(m_prefix + "first_cdc_layer") =  cdcHitList.front()->getICLayer();
      m_variables.at(m_prefix + "last_cdc_layer") =  cdcHitList.back()->getICLayer();

      m_variables.at(m_prefix + "n_svd_hits") = recoTrack.getNumberOfSVDHits();
      m_variables.at(m_prefix + "n_cdc_hits") = recoTrack.getNumberOfCDCHits();

      const auto& svdcdcRecoTrackCovariance = svdcdcRecoTrack->getSeedCovariance();
      const auto& svdcdcRecoTrackMomentum = svdcdcRecoTrack->getMomentumSeed();
      const auto& svdcdcRecoTrackPosition = svdcdcRecoTrack->getPositionSeed();
      const auto& svdcdcRecoTrackChargeSign = svdcdcRecoTrack->getChargeSeed() > 0 ? 1 : -1;
      const auto& bFieldValue = BFieldManager::getFieldInTesla(svdcdcRecoTrackPosition).Z();
      const uint16_t svdcdcNDF = 0xffff;
      const auto& svdcdcFitResult = TrackFitResult(svdcdcRecoTrackPosition, svdcdcRecoTrackMomentum, svdcdcRecoTrackCovariance,
                                                   svdcdcRecoTrackChargeSign, Const::pion, 0, bFieldValue, 0, 0,
                                                   svdcdcNDF);

      m_variables.at(m_prefix + "seed_pz_variance") = svdcdcRecoTrackCovariance(5, 5);
      m_variables.at(m_prefix + "seed_pz_estimate") = svdcdcRecoTrackMomentum.Z();
      m_variables.at(m_prefix + "seed_z_estimate") = svdcdcRecoTrackPosition.Z() ;
      m_variables.at(m_prefix + "seed_tan_lambda_estimate") = svdcdcFitResult.getCotTheta();

      m_variables.at(m_prefix + "seed_pt_estimate") = svdcdcRecoTrackMomentum.Rho();
      m_variables.at(m_prefix + "seed_x_estimate") = svdcdcRecoTrackPosition.X();
      m_variables.at(m_prefix + "seed_y_estimate") = svdcdcRecoTrackPosition.Y();
      m_variables.at(m_prefix + "seed_py_variance") = svdcdcRecoTrackCovariance(4, 4);
      m_variables.at(m_prefix + "seed_d0_estimate") = svdcdcFitResult.getD0();
      m_variables.at(m_prefix + "seed_omega_variance") = svdcdcFitResult.getCov()[9];
      m_variables.at(m_prefix + "seed_tan_lambda_variance") = svdcdcFitResult.getCov()[14];
      m_variables.at(m_prefix + "seed_z_variance") = svdcdcRecoTrackCovariance(2, 2);

      m_variables.at(m_prefix + "svd_layer3_positionSigma") = -99999.9;
      m_variables.at(m_prefix + "svd_layer6_clsTime") = -99999.9;
      for (const auto* svdHit : recoTrack.getSVDHitList()) {
        if (svdHit->getSensorID().getLayerNumber() == 3) {
          m_variables.at(m_prefix + "svd_layer3_positionSigma") = svdHit->getPositionSigma();
        }
        if (svdHit->getSensorID().getLayerNumber() == 6) {
          m_variables.at(m_prefix + "svd_layer6_clsTime") = svdHit->getClsTime();
        }
      }
    }


  protected:
    /// prefix for RecoTrack extracted variables
    std::string m_prefix;
  };

}
