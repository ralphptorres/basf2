/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/manager/RFFlowStat.h>
#include <framework/pcore/RingBuffer.h>
#include <daq/rfarm/event/hltsocket/HLTMainLoop.h>
#include <daq/rfarm/event/hltsocket/HLTSocket.h>
#include <framework/logging/Logger.h>
#include <framework/pcore/EvtMessage.h>

#include <boost/program_options.hpp>
#include <iostream>

#define MAXEVTSIZE 80000000

using namespace Belle2;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  std::string ringBufferName;
  unsigned int port;
  std::string shmName;
  unsigned int shmID;
  bool raw = false;
  std::string sourceHost;

  po::options_description desc("b2hlt_rb2socket RING-BUFFER-NAME PORT SHM-NAME SHM-ID");
  desc.add_options()
  ("help,h", "Print this help message")
  ("ring-buffer-name,r", po::value<std::string>(&ringBufferName)->required(), "name of the ring buffer")
  ("port,p", po::value<unsigned int>(&port)->required(), "port number to connect or listen to")
  ("shm-name,n", po::value<std::string>(&shmName)->required(), "name of the shm for flow output")
  ("shm-id,i", po::value<unsigned int>(&shmID)->required(), "id in the shm for flow output")
  ("connect-to,c", po::value<std::string>(&sourceHost), "connect to a given host instead of listening")
  ("raw", po::bool_switch(&raw)->default_value(false), "send and receive raw data instead of event buffers");

  po::positional_options_description p;
  p.add("ring-buffer-name", 1).add("port", 1).add("shm-name", 1).add("shm-id", 1);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  try {
    po::notify(vm);
  } catch (std::exception& e) {
    B2FATAL(e.what());
  }

  // TODO: delete or not?
  RingBuffer* ringBuffer = new RingBuffer(ringBufferName.c_str());
  RFFlowStat flow((char*)shmName.c_str(), shmID, ringBuffer);
  int* buffer = new int[MAXEVTSIZE];

  HLTMainLoop mainLoop;

  HLTSocket socket;
  int returnValue;
  int nevt = 0;
  bool terminate = false;

  while (mainLoop.isRunning() and not terminate) {
    // Connect socket if needed
    if (not socket.initialized()) {
      if (vm.count("connect-to")) {
        if (not socket.connect(sourceHost, port, mainLoop)) {
          B2ERROR("Could not reconnnect!");
          break;
        }
      } else {
        if (not socket.accept(port)) {
          B2ERROR("Could not reconnect!");
          break;
        }
      }
      B2RESULT("Connected.");
    }

    // Read from ring buffer
    const int size = mainLoop.readFromRingBufferWaiting(ringBuffer, buffer);
    // Error checking ring buffer
    if (size <= 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Writing to the ring buffer failed!");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Size is negative! This should be handled above. Not good!", size > 0);

    // Monitoring
    flow.log(size * sizeof(int));

    if (raw) {
      returnValue = socket.put_wordbuf(buffer, size);
    } else {
      EvtMessage message(reinterpret_cast<char*>(buffer));
      returnValue = socket.put(message.buffer(), message.size());
      // Terminate messages make us terminate
      if (message.type() == MSG_TERMINATE) {
        B2RESULT("Having received terminate message");
        terminate = true;
      }
    }
    if (returnValue == 0) {
      B2ERROR("Error in sending the event! Reconnecting.");
      socket.deinitialize();
      continue;
    } else if (returnValue < 0) {
      if (mainLoop.isRunning()) {
        B2ERROR("Error in sending the event! Aborting.");
      }
      // This is fine if we are terminating anyways
      break;
    }
    B2ASSERT("Written size is negative! This should be handled above. Not good!", returnValue > 0);

    // Logging
    nevt++;
    if (nevt % 5000 == 0) {
      B2RESULT("b2hlt_rb2socket event number: " << nevt);
    }
  }

  B2RESULT("Program terminated.");
}
