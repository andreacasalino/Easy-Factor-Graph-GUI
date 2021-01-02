#include "../efgAddon.h"
#include <iostream>
using namespace Napi;

bool operator<(const EFG::CategoricVariable& a, const EFG::CategoricVariable& b) {
  return (a.GetName() < b.GetName());
}

#define SEARCH_OPT(VAR_NAME, KEY)  \
  auto VAR_NAME = request.options.find(KEY); \
  if(VAR_NAME == request.options.end()) return;

efgJS::efgJS(const Napi::CallbackInfo& info) 
  : ObjectWrap(info) {  

  this->commands.emplace('X' , [this](Request& request) { 
    SEARCH_OPT(fOpt, 'f');
    if(this->Import(fOpt->second[0])) request.newNetwork = this->getJSON();
  });

  this->commands.emplace('A' , [this](Request& request) { 
    SEARCH_OPT(fOpt, 'f');
    if(this->Append(fOpt->second[0])) request.newNetwork = this->getJSON();
  });

  this->commands.emplace('V' , [this](Request& request) { 
    SEARCH_OPT(vOpt, 'v');
    SEARCH_OPT(sOpt, 's');
    if(this->CreateIsolatedVar(vOpt->second[0], std::atoi(sOpt->second[0].c_str()) )) request.newNetwork = this->getJSON();
  });

  this->commands.emplace('M' , [this](Request& request) { 
    if(this->RecomputeMap()) request.newNetwork = this->getJSON();
  });

  this->commands.emplace('O' , [this](Request& request) {
    auto vOpt = request.options.find('v');
    auto oOpt = request.options.find('o');
    if((vOpt == request.options.end()) && (oOpt == request.options.end())) {
      if(this->DeleteObservation()) request.newNetwork = this->getJSON();
      return;
    }
    if(vOpt != request.options.end()) return;
    if(oOpt != request.options.end()) return;
    if(vOpt->second.size() != oOpt->second.size()) return;

    std::vector<std::pair<std::string, std::size_t>> obs;
    obs.reserve(vOpt->second.size());
    for(std::size_t k=0; k<vOpt->second.size(); ++k) {
      obs.emplace_back(std::make_pair(vOpt->second[k], std::atoi(oOpt->second[k].c_str()) ));
    }
    if(this->AddObservation(obs)) request.newNetwork = this->getJSON();
  });

  this->commands.emplace('R' , [this](Request& request) { 
    SEARCH_OPT(fOpt, 'f');
    this->Export(fOpt->second[0]);
  });

  this->commands.emplace('Q' , [this](Request& request) { 
    SEARCH_OPT(vOpt, 'v');
    auto info = this->GetNodeInfo(vOpt->second[0]);
    if(nullptr == info) return;
    std::shared_ptr<json::structJSON> infoJSON = std::make_shared<json::structJSON>();
    infoJSON->addElement("s", json::Number<std::size_t>(info->size));
    infoJSON->addElement("i", json::Number<bool>(info->isIsolated));
    request.info = infoJSON;
  });

  this->commands.emplace('I' , [this](Request& request) { 
    SEARCH_OPT(vOpt, 'v');
    auto marg = this->GetMarginals(vOpt->second[0]);
    std::shared_ptr<json::arrayJSON> margJSON = std::make_shared<json::arrayJSON>();
    for(std::size_t k=0; k<marg.size(); ++k) {
      margJSON->addElement(json::Number<float>(marg[k]));
    }
    request.info = margJSON;
  });

  this->commands.emplace('P' , [this](Request& request) { 
    SEARCH_OPT(vOpt, 'v');
    float w = 0.f;
    auto wOpt = request.options.find('w');
    if(wOpt != request.options.end()) {
      w = std::atoi(wOpt->second[0].c_str());
    }
    auto fOpt = request.options.find('f');
    if(fOpt != request.options.end()) {
      if(vOpt->second.size() == 1) {
        if(this->AddFactor(vOpt->second[0], fOpt->second[0], w)) request.newNetwork = this->getJSON();
      }
      else {
        if(this->AddFactor(vOpt->second[0], vOpt->second[1], fOpt->second[0], w)) request.newNetwork = this->getJSON();
      }
      return;
    }
    auto cOpt = request.options.find('c');
    if(cOpt != request.options.end()) {
      if(cOpt->second[0].size() != 1) return;
      if( ('T' == cOpt->second[0].front()) && (this->AddFactor(vOpt->second[0], vOpt->second[1], true, w)) ){
        request.newNetwork = this->getJSON();
      } 
      if( ('F' == cOpt->second[0].front()) && (this->AddFactor(vOpt->second[0], vOpt->second[1], true, w)) ){
        request.newNetwork = this->getJSON();
      } 
      return;
    }
  });

}

Napi::Value efgJS::ProcessRequest(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env(); 
  Command comm(info);
  std::cout << "-------- Request --------" << std::endl << comm.str() << std::endl;

  std::shared_ptr<json::streamJSON> respNewNet = nullptr;
  std::shared_ptr<json::streamJSON> respInfo = nullptr;
  // auto it = this->commands.find(comm.getSymbol());
  // if(it != this->commands.end()){
  //   Request req = {comm.getOptions() , respNewNet, respInfo};
  //   it->second(req);
  // }
  
  json::structJSON response;
  if(nullptr == respNewNet) response.addElement("n", json::Null());
  else                      response.addElement("n", *respNewNet.get());
  if(nullptr == respInfo)   response.addElement("i", json::Null());
  else                      response.addElement("i", *respInfo.get());

  std::string respStr = response.str();
  std::cout << "-------- Response --------" << std::endl << respStr << std::endl << std::endl;
  return Napi::String::New(env, respStr);
}

std::shared_ptr<json::streamJSON> efgJS::getJSON() {
  json::arrayJSON nodes, edges;
  auto addNode = [&nodes](const std::string& label, const std::string& id, const std::string& image) {
    json::structJSON temp;
    temp.addElement("label", json::String(label));
    temp.addElement("shape", json::String("image"));
    temp.addElement("image", json::String(image));
    temp.addElement("color", json::String("#000000"));
    temp.addElement("id", json::String(id));
    nodes.addElement(temp);
  };
  // process variables
  if(nullptr != this->graph) {
    auto hiddenVars = this->graph->GetHiddenSet();
    for(auto it = hiddenVars.begin(); it!=hiddenVars.end(); ++it) {
      auto itMap = this->lastMap.find((*it)->GetName());
      if(itMap == this->lastMap.end()) {
        addNode((*it)->GetName(), (*it)->GetName(), "./image/Variable.svg");
      }
      else {
        addNode((*it)->GetName() + "=" + std::to_string(itMap->second) + " (MAP)", (*it)->GetName(), "./image/Variable.svg");
      }
    }
    auto obVars = this->graph->GetObservationSet();
    for(auto it = obVars.begin(); it!=obVars.end(); ++it) {
      addNode(it->first->GetName() + "=" + std::to_string(it->second), it->first->GetName(), "./image/Variable_Observed.svg");
    }
  }
  for(auto it = this->isolatedVars.begin(); it!=this->isolatedVars.end(); ++it) {
    addNode(it->second.GetName(), it->second.GetName(), "./image/Variable.svg");
  }
  // process potentials
    std::size_t counter = 0;
  auto addEdge = [&edges, &nodes, &counter, &addNode](const EFG::pot::IPotential& pot, const std::string& image) {
    std::string edgeName = "__edge" + std::to_string(counter);
    addNode("", edgeName, image);
    auto vars = pot.GetDistribution().GetVariables();
    if(vars.size() == 1) {
      json::structJSON e;
      e.addElement("from", json::String(edgeName));
      e.addElement("to", json::String(vars.front()->GetName()));
      edges.addElement(e);
    }
    else {
      json::structJSON e1;
      e1.addElement("from", json::String(edgeName));
      e1.addElement("to", json::String(vars.front()->GetName()));
      edges.addElement(e1);
      json::structJSON e2;
      e2.addElement("from", json::String(edgeName));
      e2.addElement("to", json::String(vars.back()->GetName()));
      edges.addElement(e2);
    }
    ++counter;
  };
  if(nullptr != this->graph) {
    auto structure = this->graph->GetStructure();
    // shapes
    for(auto it = std::get<0>(structure).begin(); it!=std::get<0>(structure).end(); ++it) {
      addEdge(**it , "./image/Potential_Shape.svg");
    }
    // tunab 
    for(auto it = std::get<1>(structure).begin(); it!=std::get<1>(structure).end(); ++it) {
      for(auto itt = it->begin(); itt!=it->end(); ++itt) {
        addEdge(**itt , "./image/Potential_Exp_Shape_tunable.svg");
      } 
    }
    // fixed
    for(auto it = std::get<2>(structure).begin(); it!=std::get<2>(structure).end(); ++it) {
      addEdge(**it , "./image/Potential_Exp_Shape_fixed.svg");
    }
  }
  std::shared_ptr<json::structJSON> completeJSON = std::make_shared<json::structJSON>();
  completeJSON->addElement("nodes", nodes);
  completeJSON->addElement("edges", edges);
  return completeJSON;
}

bool efgJS::Import(const std::string& fileName) {
  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>(fileName);
  }
  catch(...) {
    model.reset();
  }
  if(nullptr != model) {
    this->graph = std::move(model);
    this->isolatedVars.clear();
    this->lastMap.clear();
    return true;
  }
  return false;
}

bool efgJS::Append(const std::string& fileName) {
  if(nullptr == this->graph){
    return this->Import(fileName);
  }

  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>(fileName);
  }
  catch(...) {
    model.reset();
  }
  if(nullptr != model) {
    this->graph->Insert(model->GetStructure(), false);
    this->isolatedVars.clear();
    this->lastMap.clear();
    return true;
  }
  return false;
}

void efgJS::Export(const std::string& fileName){
  if(nullptr == this->graph) return;
  this->graph->Reprint(fileName);
}

bool efgJS::CreateIsolatedVar(const std::string& name, const std::size_t& size){
  VariableFinder finder(*this, name);
  if(nullptr == finder.get()) {
    this->isolatedVars.emplace(name , EFG::CategoricVariable(size, name));
    return true;
  }
  finder.release();
  return false;
}

bool efgJS::AddObservation(const std::vector<std::pair<std::string, std::size_t>>& obs) {
  if(nullptr == this->graph) return false;
  auto obOld = this->graph->GetObservationSet();
  std::vector<std::pair<std::string, std::size_t>> ob;
  ob.reserve(obOld.size() + obs.size());
  for(auto it = obOld.begin(); it!=obOld.end(); ++it){
    ob.push_back(std::make_pair(it->first->GetName(), it->second));
  }
  for(auto it = obs.begin(); it!=obs.end(); ++it) {
    auto* var = this->graph->FindVariable(it->first);
    if((nullptr != var) && (it->second < var->size())){
      ob.push_back(std::make_pair(it->first, it->second));
    }
  }
  this->graph->SetEvidences(ob);
  this->lastMap.clear();
  return true;
}

bool efgJS::DeleteObservation(){
  if(nullptr == this->graph) return false;
  this->graph->SetEvidences(std::vector<std::pair<std::string, std::size_t>>{});
  this->lastMap.clear();
  return true;
}

std::unique_ptr<efgJS::NodeInfo> efgJS::GetNodeInfo(const std::string& name) {
  VariableFinder finder(*this, name);
  if(nullptr == finder.get()) return nullptr;
  std::unique_ptr<NodeInfo> info = std::make_unique<NodeInfo>();
  info->isIsolated = finder.isIsolated();
  info->size = finder.get()->size();
  finder.release();
  return info;
}

std::vector<float> efgJS::GetMarginals(const std::string& name) {
  if(nullptr == this->graph) return {};
  std::vector<float> marginals;
  try {
    marginals = this->graph->GetMarginalDistribution(name);
  }
  catch(...) {
    return  {};
  }
  return marginals;
}

bool efgJS::RecomputeMap() {
  if(nullptr == this->graph) return false;
  auto hiddenSet = this->graph->GetHiddenSet();
  this->lastMap.clear();
  for(auto it = hiddenSet.begin(); it!=hiddenSet.end(); ++it) {
    this->lastMap.emplace((*it)->GetName() , this->graph->GetMAP((*it)->GetName()));
  }
  return true;
}

bool efgJS::AddFactor(const std::string& name, const std::string& fileName, const float& weight) {
  VariableFinder finder(*this, name);
  if(nullptr == finder.get()) return false;

  try {
    EFG::pot::Factor pot({finder.get()}, fileName);
    if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
    if(0.f == weight) {
      this->graph->InsertMove(pot);
    }
    else {
      this->graph->InsertMove(EFG::pot::ExpFactor(pot, weight));
    }
  }
  catch(...) {
    finder.release();
    return false;
  }
  this->lastMap.clear();
  return true;
}

bool efgJS::AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight) {
  VariableFinder finderA(*this, nameA);
  if(nullptr == finderA.get()) return false;
  VariableFinder finderB(*this, nameB);
  if(nullptr == finderB.get()) return false;

  try {
    EFG::pot::Factor pot({finderA.get(), finderB.get()}, fileName);
    if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
    if(0.f == weight) {
      this->graph->InsertMove(pot);
    }
    else {
      this->graph->InsertMove(EFG::pot::ExpFactor(pot, weight));
    }
  }
  catch(...) {
    finderA.release();
    finderB.release();
    return false;
  }
  this->lastMap.clear();
  return true;
}

bool efgJS::AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight) {
  VariableFinder finderA(*this, nameA);
  if(nullptr == finderA.get()) return false;
  VariableFinder finderB(*this, nameB);
  if(nullptr == finderB.get()) return false;

  try {
    EFG::pot::Factor pot(std::vector<EFG::CategoricVariable*>{finderA.get(), finderB.get()}, corr_anti);
    if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
    if(0.f == weight) {
      this->graph->InsertMove(pot);
    }
    else {
      this->graph->InsertMove(EFG::pot::ExpFactor(pot, weight));
    }
  }
  catch(...) {
    finderA.release();
    finderB.release();
    return false;
  }
  this->lastMap.clear();
  return true;
}

Napi::Object efgJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "efgJS", {
    InstanceMethod("ProcessRequest", &efgJS::ProcessRequest)
  });

  Napi::FunctionReference constructor;
  constructor = Napi::Persistent(func);
  exports.Set("efgJS", func);
  return exports;
}

Napi::Object Init (Napi::Env env, Napi::Object exports) {
    efgJS::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
