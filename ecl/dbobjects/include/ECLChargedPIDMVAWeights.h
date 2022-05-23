/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

// FRAMEWORK
#include <framework/gearbox/Const.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// MVA
#include <mva/interface/Weightfile.h>

// ROOT
#include <TObject.h>
#include <TF1.h>
#include <TParameter.h>
#include <TFile.h>

//BOOST
#include <boost/algorithm/string/predicate.hpp>

// std libraries
#include <cmath>
#include <unordered_map>
#include <tuple>

namespace Belle2 {

  /**
   * Stores the N dimensional binning in which to apply the MVAs. For example, 3D in (clusterTheta, p, charge).
   * Also provides functionality to get the bin index from the binning.
   */
  class ECLChargedPIDPhasespaceBinning : public TObject {

  public:

    /**
     * Constructor.
     * @param binEdges vector of vectors of bin edges. Bin edges should be floats.
     */
    ECLChargedPIDPhasespaceBinning(const std::vector<std::vector<float>> binEdges)
    {
      m_binEdges = binEdges;
      for (auto dimensionBinEdges : binEdges) {
        m_nBins.push_back(dimensionBinEdges.size() - 1);
      }
    }

    /**
     * Destructor.
     */
    ~ECLChargedPIDPhasespaceBinning() {};

    /**
     * Maps the vector of input values to a global bin index. If any of the values lies outside the binning -1 is returned.
     * @param values N dimensional vector of values to be mapped to a global linear bin index.
     */
    int getLinearisedBinIndex(const std::vector<float> values)
    {

      int globalBin = 0;
      std::vector<int> binIndices = getBinIndices(values);
      for (unsigned int i; i < (binIndices.size() - 1); i++) {
        if (binIndices[i] < 0) return -1;
        globalBin = (globalBin + binIndices[i]) * m_nBins[i];
      }
      globalBin = globalBin + binIndices[binIndices.size() - 1];
      return globalBin;
    }

    /**
     * Maps the vector of input values to their bin index in N dimensions.
     * If the values lie outside the covered region -1 is returned.
     * @param values N dimensional vector of values to be mapped to a global linear bin index.
     */
    std::vector<int> getBinIndices(const std::vector<float> values)
    {
      std::vector<int> binIndices(m_binEdges.size());

      for (unsigned int i; i < m_binEdges.size(); i++) {
        std::vector<float> dimBinEdges = m_binEdges[i];
        auto it = std::lower_bound(dimBinEdges.begin(), dimBinEdges.end(), values[i]);
        if (it == dimBinEdges.end() || *it != values[i]) {
          binIndices[i] = -1;
        } else {
          int index = std::distance(dimBinEdges.begin(), it);
          binIndices[i] = index;
        }
      }
      return binIndices;
    }

  private:
    /**
     * Vector of bin edges. One per dimension.
     */
    std::vector<std::vector<float>> m_binEdges;

    /**
     * Vector of number of bins per dimension.
     */
    std::vector<int> m_nBins;

    // 1: first class implementation.
    ClassDef(ECLChargedPIDPhasespaceBinning, 1); /**< ClassDef */
  };

  /**
   * Stores all required information for the ECLChargedPIDMVA for a phasespace category.
   * This includes:
   *  - MVA weightfiles for multiclass MVA.
   *  - TF1 p.d.fs for each charged particle hypothesis for each mva output variable.
   *  - MVAResponseTransform mode detailing which transformations will be applied to the MVA response.
   *  - unordered_map mapping a particle hypothesis to the output index of a MVA.
   *  - (Optional) TH1F for each charged particle hypothesis for each mva output variable for gaussianisation.
   *  - (Optional) vector of floats (flattened square matrix) for potential linear decorrelation of the gaussian transformed mva response variables.
   */
  class ECLChargedPIDPhasespaceCategory : public TObject {

  public:
    /** enum of implemented transformations which can be applied to the MVA response */
    enum class MVAResponseTransformMode : unsigned int {
      /** log transform the mva responses. And take the likelihood as the product of likelihoods from all mva responses. */
      c_LogTransform = 0,
      /** log transform the mva responses. Take the likelihood from only the mva response for the hypothesis. */
      c_LogTransformSingle = 1,
      /** Gaussian transform of the log transformed mva response. */
      c_GaussianTransform = 2,
      /** Decorrelation transform of the gaussian transformed mva responses. */
      c_DecorrelationTransform = 3
    };

    /**
    * Default constructor, necessary for ROOT to stream the object.
    */
    ECLChargedPIDPhasespaceCategory() :
      m_log_transform_offset("logTransformOffset", 1e-15),
      m_max_possible_response_value("maxPossibleResponseValue", 1.0)
    {};

    /**
    * Useful constructor.
    * @param weightfilePath path to the MVA weightfile for this phasespace category.
    * @param mvaResponeTransformMode mva response transform mode booked for this phasespace.
    * @param pdfs vector of unordered_map mapping hypothesis to pdfs for each mva response.
    * @param mvaIndexForHypothesis unordered_map mapping hypothesis to index of mva response. Useful if we exclude a class from the MVA training
    *        but want to use the likelihood of a different particle. For example using the proton pdf for a deuteron.
    */
    ECLChargedPIDPhasespaceCategory(const std::string weightfilePath,
                                    const MVAResponseTransformMode& mvaResponeTransformMode,
                                    const std::vector<std::unordered_map<unsigned int, TF1>>& pdfs,
                                    const std::unordered_map<unsigned int, unsigned int>& mvaIndexForHypothesis) :
      m_log_transform_offset("logTransformOffset", 1e-15),
      m_max_possible_response_value("maxPossibleResponseValue", 1.0)

    {
      // Load and serialize the MVA::Weightfile object into a string for storage in the database,
      // otherwise there are issues w/ dictionary generation for the payload class...
      Belle2::MVA::Weightfile weightfile;
      if (boost::ends_with(weightfilePath, ".root")) {
        weightfile = Belle2::MVA::Weightfile::loadFromROOTFile(weightfilePath);
      } else  if (boost::ends_with(weightfilePath, ".xml")) {
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(weightfilePath);
      } else {
        B2WARNING("Unkown file extension for file: " << weightfilePath << ", fallback to xml...");
        weightfile = Belle2::MVA::Weightfile::loadFromXMLFile(weightfilePath);
      }
      std::stringstream ss;
      Belle2::MVA::Weightfile::saveToStream(weightfile, ss);

      // store
      m_weight = ss.str();
      m_mvaResponseTransformMode = mvaResponeTransformMode;
      m_pdfs = pdfs;
      m_mvaIndexForHypothesis = mvaIndexForHypothesis;
    }

    /**
     * Destructor.
     */
    ~ECLChargedPIDPhasespaceCategory() {};

    /**
     * getter for serialised weightfile.
     */
    const std::string getSerialisedWeight() const {return m_weight;}

    /**
     * getter for the MVA transform mode.
     */
    MVAResponseTransformMode getTransformMode() const {return m_mvaResponseTransformMode;}

    /**
     * getter for pdfs.
     * @param iMVAResponse index of MVA response.
     * @param hypoPDG, hypothesis pdg.
     */
    const TF1* getPDF(const unsigned int iMVAResponse, const unsigned int hypoPDG) const
    {
      return &m_pdfs.at(iMVAResponse).at(hypoPDG);
    }

    /**
     * gets the cdf for the hypothesis pdg for a given response value.
     * @param iMVAResponse index of MVA response.
     * @param hypoPDG, hypothesis pdg.
     */
    const TH1F* getCDF(const unsigned int iMVAResponse, const int hypoPDG) const
    {
      return &m_cdfs.at(iMVAResponse).at(hypoPDG);
    }

    /**
     * gets the decorrelation matrix for a given particle hypothesis.
     * @param hypoPDG, hypothesis pdg.
     */
    const std::vector<float>* getDecorrelationMatrix(const int hypoPDG) const
    {
      return &m_decorrelationMatrices.at(hypoPDG);
    }

    /**
     * set the cdfs.
     * @param cdfs vector of map of cdfs to be stored in the payload.
     */
    void setCDFs(std::vector<std::unordered_map<unsigned int, TH1F>> cdfs) {m_cdfs = cdfs;}

    /**
     * set the decorrelation matrices.
     * @param decorrelationMatrices map of decorrelation matrices to be stored in the payload.
     */
    void setDecorrelationMatrixMap(std::unordered_map<unsigned int, std::vector<float>> decorrelationMatrices)
    {
      m_decorrelationMatrices = decorrelationMatrices;
    }

    /**
     * Set the offset used in the log transformation to be consistent with the offset used when generating the p.d.f.s
     */
    void setlogTransformOffset(const float& offset)
    {
      m_log_transform_offset.SetVal(offset);
    }

    /**
     * get the log transform offset
     */
    float getLogTransformOffset() const
    {
      return m_log_transform_offset.GetVal();
    }

    /**
     * Set the max possible response value, used in log transformation of the responses.
     */
    void setMaxPossibleResponseValue(const float& offset)
    {
      m_max_possible_response_value.SetVal(offset);
    }

    /**
     * Get the max possible response value, used in log transformation of the responses.
     */
    float getMaxPossibleResponseValue() const
    {
      return m_max_possible_response_value.GetVal();
    }

    /**
     * maps a charged stable pdg code to an index of the MVA response.
     * In general this is a one-to-one mapping however in cases where we do not include all six
       stable charged particles in the MVA training we may have a many-to-one mapping.
     * For example if we take the proton response value also for deuterons.
     */
    unsigned int getMVAIndexForHypothesis(const unsigned int hypoPDG) const
    {
      return m_mvaIndexForHypothesis.at(hypoPDG);
    }

  private:

    TParameter<float> m_log_transform_offset; /**< Small offset to avoid mva response values of 1.0 being log transformed to NaN. */
    TParameter<float> m_max_possible_response_value; /**< Max possible value of the mva response. Used in the log transformation. */

    /**
     * Serialsed MVA weightfile.
     */
    std::string m_weight;

    /**
     * Stores which transformation mode to apply to the mva responses.
     */
    MVAResponseTransformMode m_mvaResponseTransformMode;

    /**
     * A vector of unodered maps. The vector corresponds to the return values of the MVA, one for each class correspondonding to charged stable particles considered by the MVA. In general this is the full six charged stable particles {e, mu, pi, K, p, d}.
     * The unordered map maps the hypothesis pdg values to their matching TF1 pdfs from which the liklihood will be taken.
     */
    std::vector<std::unordered_map<unsigned int, TF1>> m_pdfs;

    /**
     * unordered map of abs(pdg_code) for the 6 charged stable hypotheses to index of the MVA response vector.
     * needed if we do not train with all 6 species to map several to the same MVA response value.
     */
    std::unordered_map<unsigned int, unsigned int> m_mvaIndexForHypothesis;

    /**
     * CDFs for each mva return value for each hypothesis.
     * The N vector elements correspond to the N MVA return values.
     * The unordered map maps the hypothesis pdg values to their matching TH1F cdfs which can be used for a gaussianisation.
     */
    std::vector<std::unordered_map<unsigned int, TH1F>> m_cdfs;

    /**
     * Decorrelation matrices. To be used (optionally) afer gaussianisation.
     * The unordered map maps the hypothesis pdg values to their matching linearised decorrelation matrix.
     */
    std::unordered_map<unsigned int, std::vector<float>> m_decorrelationMatrices;

    // 1: first class implementation.
    ClassDef(ECLChargedPIDPhasespaceCategory, 1); /**< ClassDef */
  };

  /** Class to contain payload of everything needed for MVA based charged particle identification.
    * - ECLChargedPIDPhasespaceBinning object specifying the boundaries in N dimensions that define the categories (regions) under consideration,
    * - Vector of ECLChargedPIDPhasespaceCategory objects which contain specific settings for each category.
    */
  class ECLChargedPIDMVAWeights : public TObject {
  public:
    /**
    * Default constructor, necessary for ROOT to stream the object.
    */
    ECLChargedPIDMVAWeights() {};

    /**
     * Destructor.
     */
    ~ECLChargedPIDMVAWeights() {};


    /**
     * Set the N dimensional grid representing the categories for which weightfiles are defined.
     * A multiclass MVA is trained for each phases-space region defined by the bin boundaries.
     * @param h the N dimensional ECLChargedPIDPhasespaceBinning object.
    */
    void setWeightCategories(ECLChargedPIDPhasespaceBinning* h) {m_categories = h;}

    /**
     * store the ECLChargedPIDPhasespaceCategory objects into the payload.
     * @param phasespaceCategories a vector of ECLChargedPIDPhasespaceCategory objects, one per phasespace region.
              Each object contains all the data required to process tracks in that phasespace.
     */
    void storeMVAWeights(std::unordered_map<unsigned int, ECLChargedPIDPhasespaceCategory>& phasespaceCategories)
    {
      m_phasespaceCategories = phasespaceCategories;
    }

    /**
     * returns the ith ECLChargedPIDPhasespaceCategory.
     * @param idx, index of ECLChargedPIDPhasespaceCategory.
     */
    const ECLChargedPIDPhasespaceCategory* getPhasespaceCategory(const unsigned int idx)  const {return &m_phasespaceCategories.at(idx);}

    /**
     * returns the map of phasespaceCategories.
     */
    const std::unordered_map<unsigned int, ECLChargedPIDPhasespaceCategory>* getPhasespaceCategories() const {return &m_phasespaceCategories;}

    /**
    * returns bool whether or not the given values are within the phasespace covered by the trainings in the weightfile
    * @param linearBinIndex: global bin index.
    */
    bool isPhasespaceCovered(const int linearBinIndex) const
    {
      if (linearBinIndex < 0) return false;
      if (m_phasespaceCategories.find(linearBinIndex) == m_phasespaceCategories.end()) return false;
      return true;
    }

    /**
    * returns the flattened 1D index of the N dimensional phasespace category grid.
    * @param values: N dimensional input vector of floats to be mapped to a globalBinIndex.
    */

    unsigned int getLinearisedCategoryIndex(std::vector<float> values) const
    {
      if (!m_categories) {
        B2FATAL("No N dimensional grid was found in the ECLChargedPIDMVA DB payload. This should not happen! Abort...");
      }
      return m_categories->getLinearisedBinIndex(values);
    }

    /**
     * returns string definitions of the variables used in defining the phasespace categories.
     */
    std::vector<std::string> getBinningVariables() const {return m_binningVariables;}

  private:
    /**
     * An N Dimensional binning whose bins define the boundaries of the categories for which the training is performed.
      * It is used to lookup the correct file in the payload, given a reconstructed value tuple.
     */
    ECLChargedPIDPhasespaceBinning* m_categories = nullptr;

    /**
     * Stores the ECLChargedPIDPhasespaceCategory object for all the N dimensional categories.
     */
    std::unordered_map<unsigned int, ECLChargedPIDPhasespaceCategory> m_phasespaceCategories;

    /**
     * Stores the list of variables used to define the phasespace binning.
     */
    std::vector<std::string> m_binningVariables;

    // 1: first class implementation.
    ClassDef(ECLChargedPIDMVAWeights, 1); /**< ClassDef  */

  }; // class ECLChargedPIDMVAWeights
} // Belle 2 Namespace