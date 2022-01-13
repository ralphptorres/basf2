/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once


/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>
#include <klm/dataobjects/KLMClusterShape.h>
#include <mdst/dataobjects/KLMCluster.h>

namespace Belle2 {

  /**
   * Module for KLM cluster reconstruction efficiency studies.
   */
  class KLMClusterAnaModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMClusterAnaModule();

    /**
     * Destructor.
     */
    ~KLMClusterAnaModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;




  private:


    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** Output per cluster. */
    StoreArray<KLMClusterShape> m_KLMClusterShape;

    /** BKLMhits. */
    StoreArray<BKLMHit2d> m_bklmHit2ds;

    /** EKLMhits. */
    StoreArray<EKLMHit2d> m_eklmHit2ds;


  };

}

/* Foward declarations. */
double expectation(std::vector<double> vec);
std::vector<double> addition(std::vector<double> vec1, std::vector<double> vec2);
std::vector<double> product(std::vector<double> vec1, std::vector<double> vec2);
std::vector<double> covariance_matrix3x3(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord);
TMatrixT<double> eigenvectors3x3(std::vector<double> matrix);
TMatrixT<double> spatialVariances(std::vector<double> xcoord, std::vector<double> ycoord, std::vector<double> zcoord);



