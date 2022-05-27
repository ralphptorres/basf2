/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZERPREPC_H
#define DESERIALIZERPREPC_H

#include <string>
#include <vector>
#include <stdlib.h>

#include <framework/datastore/StoreArray.h>

#include <daq/rawdata/modules/DeSerializer.h>
#include <daq/dataflow/EvtSocket.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>

#include <rawdata/dataobjects/RawFTSW.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerPrePCModule : public DeSerializerModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerPrePCModule();
    virtual ~DeSerializerPrePCModule();

    //! Module functions to be called from main process
    void initialize() override;

    //! Module functions to be called from event process
    void event() override;


  protected :
    //! Accept connection
    virtual int Connect();

    //! receive data
    virtual int recvFD(int fd, char* buf, int data_size_byte, int flag);


    //! receive data
    virtual int* recvData(int* delete_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);

    //! attach buffer to RawDataBlock
    virtual void setRecvdBuffer(RawDataBlock* raw_datablk, int* delete_flag);

    //! check data contents
    virtual void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);

    //! # of connections
    int m_num_connections;

    //! Reciever basf2 Socket
    std::vector<EvtSocketRecv*> m_recv;

    //! Reciever Socket
    std::vector<int> m_socket;

    //! hostname of upstream Data Sources
    std::vector<std::string> m_hostname_from;

    //! port # to connect data sources
    std::vector<int> m_port_from;

    StoreArray<RawFTSW> raw_ftswarray;

    StoreArray<RawCOPPER> rawcprarray;

    //    StoreArray<ReducedRawCOPPER> rawcprarray;
    ///
    PreRawCOPPERFormat_latest m_pre_rawcpr;

    int event_diff;

    unsigned int m_prev_copper_ctr;

    unsigned int m_prev_evenum;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
