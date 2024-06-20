/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/VariablesToNtuple/VariablesToNtupleModule.h>

// analysis
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/StringWrapper.h>

// framework
#include <framework/logging/Logger.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/core/Environment.h>
#include <framework/core/RandomNumbers.h>
#include <framework/database/Database.h>
#include <framework/dataobjects/FileMetaData.h>

// framework - root utilities
#include <framework/utilities/MakeROOTCompatible.h>
#include <framework/utilities/RootFileCreationManager.h>
#include <framework/io/RootIOUtilities.h>

#include <cmath>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

// Register module in the framework
REG_MODULE(VariablesToNtuple);


VariablesToNtupleModule::VariablesToNtupleModule() :
  Module(), m_tree("", DataStore::c_Persistent), m_experimentLow(1), m_runLow(0), m_eventLow(0), m_experimentHigh(0),
  m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Calculate variables specified by the user for a given ParticleList and save them into a TNtuple. The TNtuple is candidate-based, meaning that the variables of each candidate are saved into separate rows.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  vector<string> emptylist;
  addParam("particleList", m_particleList,
           "Name of particle list with reconstructed particles. If no list is provided the variables are saved once per event (only possible for event-type variables)",
           std::string(""));
  addParam("variables", m_variables,
           "List of variables (or collections) to save. Variables are taken from Variable::Manager, and are identical to those available to e.g. ParticleSelector.",
           emptylist);

  addParam("fileName", m_fileName, "Name of ROOT file for output. Can be overridden using the -o argument of basf2.",
           string("VariablesToNtuple.root"));
  addParam("treeName", m_treeName, "Name of the NTuple in the saved file.", string("ntuple"));
  addParam("basketSize", m_basketsize, "Size of baskets in Output NTuple in bytes.", 1600);

  std::tuple<std::string, std::map<int, unsigned int>> default_sampling{"", {}};
  addParam("sampling", m_sampling,
           "Tuple of variable name and a map of integer values and inverse sampling rate. E.g. (signal, {1: 0, 0:10}) selects all signal candidates and every 10th background candidate.",
           default_sampling);

  addParam("signalSideParticleList", m_signalSideParticleList,
           "Name of signal-side particle list to store the index of the signal-side particle when one calls the module in a for_each loop over the RestOfEvent",
           std::string(""));

  addParam("fileNameSuffix", m_fileNameSuffix, "The suffix of the output ROOT file to be appended before ``.root``.",
           string(""));

  addParam("useFloat", m_useFloat,
           "Use float type for floating-point numbers.", false);

  addParam("storeEventType", m_storeEventType,
           "If true, the branch __eventType__ is added. The eventType information is available from MC16 on.", true);

  addParam("additionalDataDescription", m_additionalDataDescription,
           "Additional dictionary of "
           "name->value pairs to be added to the file metadata to describe the data",
           m_additionalDataDescription);

}

void VariablesToNtupleModule::initialize()
{
  m_eventMetaData.isRequired();
  if (not m_particleList.empty())
    StoreObjPtr<ParticleList>().isRequired(m_particleList);

  // Initializing the output root file

  // override the output file name with what's been provided with the -o option
  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_fileName = outputFileArgument;

  if (!m_fileNameSuffix.empty())
    m_fileName = m_fileName.insert(m_fileName.rfind(".root"), m_fileNameSuffix);

  if (m_fileName.empty()) {
    B2FATAL("Output root file name is not set. Please set a valid root output file name (\"fileName\" module parameter).");
  }
  // See if there is already a file in which case add a new tree to it ...
  // otherwise create a new file (all handled by framework)
  m_file =  RootFileCreationManager::getInstance().getFile(m_fileName);
  if (!m_file) {
    B2ERROR("Could not create file \"" << m_fileName <<
            "\". Please set a valid root output file name (\"fileName\" module parameter).");
    return;
  }

  TDirectory::TContext directoryGuard(m_file.get());

  // check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2FATAL("Tree with the name \"" << m_treeName
            << "\" already exists in the file \"" << m_fileName << "\"\n"
            << "\nYou probably want to either set the output fileName or the treeName to something else:\n\n"
            << "   from modularAnalysis import variablesToNtuple\n"
            << "   variablesToNtuple('pi+:all', ['p'], treename='pions', filename='variablesToNtuple.root')\n"
            << "   variablesToNtuple('gamma:all', ['p'], treename='photons', filename='variablesToNtuple.root') # two trees, same file\n"
            << "\n == Or ==\n"
            << "   from modularAnalysis import variablesToNtuple\n"
            << "   variablesToNtuple('pi+:all', ['p'], filename='pions.root')\n"
            << "   variablesToNtuple('gamma:all', ['p'], filename='photons.root') # two files\n"
           );
    return;
  }

  // set up tree and register it in the datastore
  m_tree.registerInDataStore(m_fileName + m_treeName, DataStore::c_DontWriteOut);
  m_tree.construct(m_treeName.c_str(), "");
  m_tree->get().SetCacheSize(100000);

  // declare counter branches - pass through variable list, remove counters added by user
  m_tree->get().Branch("__experiment__", &m_experiment, "__experiment__/I");
  m_tree->get().Branch("__run__", &m_run, "__run__/I");
  m_tree->get().Branch("__event__", &m_event, "__event__/i");
  m_tree->get().Branch("__production__", &m_production, "__production__/I");
  if (not m_particleList.empty()) {
    m_tree->get().Branch("__candidate__", &m_candidate, "__candidate__/I");
    m_tree->get().Branch("__ncandidates__", &m_ncandidates, "__ncandidates__/I");
  }

  if (not m_signalSideParticleList.empty()) {
    StoreObjPtr<ParticleList>().isRequired(m_signalSideParticleList);
    m_tree->get().Branch("__signalSideCandidate__", &m_signalSideCandidate, "__signalSideCandidate__/I");
    m_tree->get().Branch("__nSignalSideCandidates__", &m_nSignalSideCandidates, "__nSignalSideCandidates__/I");
    if (not m_roe.isOptional("RestOfEvent")) {
      B2WARNING("The signalSideParticleList is set outside of a for_each loop over the RestOfEvent. "
                << "__signalSideCandidates__ and __nSignalSideCandidate__ will be always -1 and 0, respectively.");
    }
  }

  if (m_stringWrapper.isOptional("MCDecayString"))
    m_tree->get().Branch("__MCDecayString__", &m_MCDecayString);

  if (m_storeEventType) {
    m_tree->get().Branch("__eventType__", &m_eventType);
    if (not m_eventExtraInfo.isOptional())
      B2INFO("EventExtraInfo is not registered. __eventType__ will be empty. The eventType is available from MC16 on.");
  }

  for (const auto& variable : m_variables)
    if (Variable::isCounterVariable(variable)) {
      B2WARNING("The counter '" << variable
                << "' is handled automatically by VariablesToNtuple, you don't need to add it.");
    }

  // declare branches and get the variable strings
  m_variables = Variable::Manager::Instance().resolveCollections(m_variables);
  // remove duplicates from list of variables but keep the previous order
  unordered_set<string> seen;
  auto newEnd = remove_if(m_variables.begin(), m_variables.end(), [&seen](const string & varStr) {
    if (seen.find(varStr) != std::end(seen)) return true;
    seen.insert(varStr);
    return false;
  });
  m_variables.erase(newEnd, m_variables.end());

  if (m_useFloat)
    m_branchAddressesFloat.resize(m_variables.size() + 1);
  else
    m_branchAddressesDouble.resize(m_variables.size() + 1);
  m_branchAddressesInt.resize(m_variables.size() + 1);
  if (m_useFloat) {
    m_tree->get().Branch("__weight__", &m_branchAddressesFloat[0], "__weight__/F");
  } else {
    m_tree->get().Branch("__weight__", &m_branchAddressesDouble[0], "__weight__/D");
  }
  size_t enumerate = 1;
  for (const string& varStr : m_variables) {
    string branchName = MakeROOTCompatible::makeROOTCompatible(varStr);

    // Check for deprecated variables
    Variable::Manager::Instance().checkDeprecatedVariable(varStr);

    // also collection function pointers
    const Variable::Manager::Var* var = Variable::Manager::Instance().getVariable(varStr);
    if (!var) {
      B2ERROR("Variable '" << varStr << "' is not available in Variable::Manager!");
    } else {
      if (m_particleList.empty() && var->description.find("[Eventbased]") == string::npos) {
        B2ERROR("Variable '" << varStr << "' is not an event-based variable, "
                "but you are using VariablesToNtuple without a decay string, i.e. in the event-wise mode.\n"
                "If you have created an event-based alias you can wrap your alias with `eventCached` to "
                "declare it as event based, which avoids this error.\n\n"
                "vm.addAlias('myAliasName', 'eventCached(myAlias)')");
        continue;
      }
      if (var->variabletype == Variable::Manager::VariableDataType::c_double) {
        if (m_useFloat) {
          m_tree->get().Branch(branchName.c_str(), &m_branchAddressesFloat[enumerate], (branchName + "/F").c_str());
        } else {
          m_tree->get().Branch(branchName.c_str(), &m_branchAddressesDouble[enumerate], (branchName + "/D").c_str());
        }
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_int) {
        m_tree->get().Branch(branchName.c_str(), &m_branchAddressesInt[enumerate], (branchName + "/I").c_str());
      } else if (var->variabletype == Variable::Manager::VariableDataType::c_bool) {
        m_tree->get().Branch(branchName.c_str(), &m_branchAddressesInt[enumerate], (branchName + "/O").c_str());
      }
      m_functions.push_back(std::make_pair(var->function, var->variabletype));
    }
    enumerate++;
  }
  m_tree->get().SetBasketSize("*", m_basketsize);

  m_sampling_name = std::get<0>(m_sampling);
  m_sampling_rates = std::get<1>(m_sampling);

  if (m_sampling_name != "") {
    m_sampling_variable = Variable::Manager::Instance().getVariable(m_sampling_name);
    if (m_sampling_variable == nullptr) {
      B2FATAL("Couldn't find sample variable " << m_sampling_name << " via the Variable::Manager. Check the name!");
    }
    for (const auto& pair : m_sampling_rates)
      m_sampling_counts[pair.first] = 0;
  } else {
    m_sampling_variable = nullptr;
  }

  m_outputFileMetaData = new FileMetaData;
  // create persistent tree if it doesn't already exist
  //to-do: c_treeNames[DataStore::c_Persistent].c_str() throws error - RootIOUtilities issue?
  if (!m_file->Get("persistent")) {
    m_persistent = new TTree(c_treeNames[DataStore::c_Persistent].c_str(), c_treeNames[DataStore::c_Persistent].c_str());
    m_persistent->Branch("FileMetaData", &m_outputFileMetaData);
    m_eventTree = new TTree("tree", "tree");
    m_eventTree->Branch("EventMetaData", m_eventMetaData);
  } else {
    m_oldPersistent = dynamic_cast<TTree*>(m_file->Get("persistent"));
    m_oldFileMetaData = dynamic_cast<FileMetaData*>(m_file->Get("FileMetaData"));
    m_persistent = m_oldPersistent->CloneTree(0);
    m_oldPersistent->GetEntry(0);
    TBranch* fileMetaDataBranch = m_persistent->GetBranch("FileMetaData");
    fileMetaDataBranch->SetAddress(&m_outputFileMetaData);
  }

  // create the event tree and fill its branches
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
  set<string> branchList;
  for (const auto& pair : map)
    branchList.insert(pair.first);

  //create the tree and branches
  m_eventTree = new TTree(c_treeNames[DataStore::c_Event].c_str(), c_treeNames[DataStore::c_Event].c_str());
  for (auto& iter : map) {
    const std::string& branchName = iter.first;
    //skip transient entries (allow overriding via branchNames)
    if (iter.second.dontWriteOut)
      continue;
    //skip branches the user doesn't want
    if (branchList.count(branchName) == 0) {
      //make sure EventMetaData is always included in the output
      if (branchName != "EventMetaData") {
        continue;
      }
    }
    m_eventTree->Branch(branchName.c_str(), &iter.second.object);
    m_entries.push_back(&iter.second);
  }

}


float VariablesToNtupleModule::getInverseSamplingRateWeight(const Particle* particle)
{
  if (m_sampling_variable == nullptr)
    return 1.0;

  long target = 0;
  if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_double) {
    target = std::lround(std::get<double>(m_sampling_variable->function(particle)));
  } else if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_int) {
    target = std::lround(std::get<int>(m_sampling_variable->function(particle)));
  } else if (m_sampling_variable->variabletype == Variable::Manager::VariableDataType::c_bool) {
    target = std::lround(std::get<bool>(m_sampling_variable->function(particle)));
  }
  if (m_sampling_rates.find(target) != m_sampling_rates.end() and m_sampling_rates[target] > 0) {
    m_sampling_counts[target]++;
    if (m_sampling_counts[target] % m_sampling_rates[target] != 0)
      return 0;
    else {
      m_sampling_counts[target] = 0;
      return m_sampling_rates[target];
    }
  }
  return 1.0;
}

void VariablesToNtupleModule::event()
{
  // Fill event tree
  fillTree();

  m_event = m_eventMetaData->getEvent();
  m_run = m_eventMetaData->getRun();
  m_experiment = m_eventMetaData->getExperiment();
  m_production = m_eventMetaData->getProduction();

  if (m_experimentLow > m_experimentHigh) { //starting condition
    m_experimentLow = m_experimentHigh = m_experiment;
    m_runLow = m_runHigh = m_run;
    m_eventLow = m_eventHigh = m_event;
  } else {
    if ((m_experiment < m_experimentLow) || ((m_experiment == m_experimentLow) && ((m_run < m_runLow) || ((m_run == m_runLow)
                                             && (m_event < m_eventLow))))) {
      m_experimentLow = m_experiment;
      m_runLow = m_run;
      m_eventLow = m_event;
    }
    if ((m_experiment > m_experimentHigh) || ((m_experiment == m_experimentHigh) && ((m_run > m_runHigh) || ((m_run == m_runHigh)
                                              && (m_event > m_eventHigh))))) {
      m_experimentHigh = m_experiment;
      m_runHigh = m_run;
      m_eventHigh = m_event;
    }
  }

  // check if the event is a full event or not: if yes, increase the counter
  if (m_eventMetaData->getErrorFlag() == 0) // no error flag -> this is a full event
    m_nFullEvents++;

  if (m_stringWrapper.isValid())
    m_MCDecayString = m_stringWrapper->getString();
  else
    m_MCDecayString = "";

  if (m_storeEventType and m_eventExtraInfo.isValid())
    m_eventType = m_eventExtraInfo->getEventType();
  else
    m_eventType = "";

  if (not m_signalSideParticleList.empty()) {
    if (m_roe.isValid()) {
      StoreObjPtr<ParticleList> signaSideParticleList(m_signalSideParticleList);
      auto signal = m_roe->getRelatedFrom<Particle>();
      m_signalSideCandidate = signaSideParticleList->getIndex(signal);
      m_nSignalSideCandidates = signaSideParticleList->getListSize();
    } else {
      m_signalSideCandidate = -1;
      m_nSignalSideCandidates = 0;
    }
  }

  if (m_particleList.empty()) {
    double weight = getInverseSamplingRateWeight(nullptr);
    if (m_useFloat) {
      m_branchAddressesFloat[0] = weight;
    } else {
      m_branchAddressesDouble[0] = weight;
    }
    if (weight > 0) {
      for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
        auto var_result = std::get<0>(m_functions[iVar])(nullptr);
        auto var_type = std::get<1>(m_functions[iVar]);
        if (std::holds_alternative<double>(var_result)) {
          if (var_type != Variable::Manager::VariableDataType::c_double)
            B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                      "'. Expected Variable::Manager::VariableDataType::c_double. Exported data for this variable might be incorrect.");
          if (m_useFloat) {
            m_branchAddressesFloat[iVar + 1] = std::get<double>(var_result);
          } else {
            m_branchAddressesDouble[iVar + 1] = std::get<double>(var_result);
          }
        } else if (std::holds_alternative<int>(var_result)) {
          if (var_type != Variable::Manager::VariableDataType::c_int)
            B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                      "'. Expected Variable::Manager::VariableDataType::c_int. Exported data for this variable might be incorrect.");
          m_branchAddressesInt[iVar + 1] = std::get<int>(var_result);
        } else if (std::holds_alternative<bool>(var_result)) {
          if (var_type != Variable::Manager::VariableDataType::c_bool)
            B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                      "'. Expected Variable::Manager::VariableDataType::c_bool. Exported data for this variable might be incorrect.");
          m_branchAddressesInt[iVar + 1] = std::get<bool>(var_result);
        }
      }
      m_tree->get().Fill();
    }

  } else {
    StoreObjPtr<ParticleList> particlelist(m_particleList);
    m_ncandidates = particlelist->getListSize();
    for (unsigned int iPart = 0; iPart < m_ncandidates; iPart++) {
      m_candidate = iPart;
      const Particle* particle = particlelist->getParticle(iPart);
      double weight = getInverseSamplingRateWeight(particle);
      if (m_useFloat) {
        m_branchAddressesFloat[0] = weight;
      } else {
        m_branchAddressesDouble[0] = weight;
      }
      if (weight > 0) {
        for (unsigned int iVar = 0; iVar < m_variables.size(); iVar++) {
          auto var_result = std::get<0>(m_functions[iVar])(particle);
          auto var_type = std::get<1>(m_functions[iVar]);
          if (std::holds_alternative<double>(var_result)) {
            if (var_type != Variable::Manager::VariableDataType::c_double)
              B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                        "'. Expected Variable::Manager::VariableDataType::c_double. Exported data for this variable might be incorrect.");
            if (m_useFloat) {
              m_branchAddressesFloat[iVar + 1] = std::get<double>(var_result);
            } else {
              m_branchAddressesDouble[iVar + 1] = std::get<double>(var_result);
            }
          } else if (std::holds_alternative<int>(var_result)) {
            if (var_type != Variable::Manager::VariableDataType::c_int)
              B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                        "'. Expected Variable::Manager::VariableDataType::c_int. Exported data for this variable might be incorrect.");
            m_branchAddressesInt[iVar + 1] = std::get<int>(var_result);
          } else if (std::holds_alternative<bool>(var_result)) {
            if (var_type != Variable::Manager::VariableDataType::c_bool)
              B2WARNING("Wrong registered data type for variable '" + m_variables[iVar] +
                        "'. Expected Variable::Manager::VariableDataType::c_bool. Exported data for this variable might be incorrect.");
            m_branchAddressesInt[iVar + 1] = std::get<bool>(var_result);
          }
        }
        m_tree->get().Fill();
      }
    }
  }
}

void VariablesToNtupleModule::fillFileMetaData()
{
  bool isMC = (m_inputFileMetaData) ? m_inputFileMetaData->isMC() : true;
  new (m_outputFileMetaData) FileMetaData;
  if (!isMC) m_outputFileMetaData->declareRealData();

  unsigned long numEntries = m_tree->get().GetEntries();
  m_outputFileMetaData->setNFullEvents(m_nFullEvents);
  m_outputFileMetaData->setNEvents(numEntries);

  if (m_experimentLow > m_experimentHigh) {
    //starting condition so apparently no events at all
    m_outputFileMetaData->setLow(-1, -1, 0);
    m_outputFileMetaData->setHigh(-1, -1, 0);
  } else {
    m_outputFileMetaData->setLow(m_experimentLow, m_runLow, m_eventLow);
    m_outputFileMetaData->setHigh(m_experimentHigh, m_runHigh, m_eventHigh);
  }

  //fill more file level metadata
  RootIOUtilities::setCreationData(*m_outputFileMetaData);
  m_outputFileMetaData->setRandomSeed(RandomNumbers::getSeed());
  m_outputFileMetaData->setSteering(Environment::Instance().getSteering());
  auto mcEvents = Environment::Instance().getNumberOfMCEvents();
  m_outputFileMetaData->setMcEvents(mcEvents);
  m_outputFileMetaData->setDatabaseGlobalTag(Database::Instance().getGlobalTags());
  for (const auto& item : m_additionalDataDescription) {
    m_outputFileMetaData->setDataDescription(item.first, item.second);
  }
  if (m_inputFileMetaData.isValid()) {
    std::string lfn = m_inputFileMetaData->getLfn();
    if (not lfn.empty() and (m_parentLfns.empty() or (m_parentLfns.back() != lfn))) {
      m_parentLfns.push_back(lfn);
    }
  }
  m_outputFileMetaData->setParents(m_parentLfns);
}

void VariablesToNtupleModule::terminate()
{
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isOutputProcess()) {

    fillFileMetaData();
    TDirectory::TContext directoryGuard(m_file.get());
    m_persistent->Fill();
    m_persistent->Write("persistent", TObject::kWriteDelete);
    m_eventTree->Write("tree", TObject::kWriteDelete);

    B2INFO("Writing NTuple " << m_treeName);
    m_tree->write(m_file.get());

    const bool writeError = m_file->TestBit(TFile::kWriteError);
    m_file.reset();
    if (writeError) {
      B2FATAL("A write error occurred while saving '" << m_fileName  << "', please check if enough disk space is available.");
    }
  }
}

void VariablesToNtupleModule::fillTree()
{
  if (!m_eventTree) return;

  TTree& tree = *m_eventTree;
  for (auto entry : m_entries) {
    // Check for entries whose object was not created and mark them as invalid.
    // We still have to write them in the file due to the structure we have. This could be done better
    if (!entry->ptr) {
      entry->object->SetBit(kInvalidObject);
    }
    //FIXME: Do we need this? in theory no but it crashes in parallel processing otherwise ¯\_(ツ)_/¯
    if (entry->name == "FileMetaData") {
      tree.SetBranchAddress(entry->name.c_str(), &m_outputFileMetaData);
    } else {
      tree.SetBranchAddress(entry->name.c_str(), &entry->object);
    }
  }
  tree.Fill();
  for (auto entry : m_entries) {
    entry->object->ResetBit(kInvalidObject);
  }

  const bool writeError = m_file->TestBit(TFile::kWriteError);
  if (writeError) {
    //m_file deleted first so we have a chance of closing it (though that will probably fail)
    const std::string filename = m_file->GetName();
    // delete m_file;
    B2FATAL("A write error occured while saving '" << filename << "', please check if enough disk space is available.");
  }
}
