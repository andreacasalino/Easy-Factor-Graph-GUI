#include <EFG-model.h>
#include <iostream>

bool operator<(const EFG::CategoricVariable& a, const EFG::CategoricVariable& b) {
  return (a.GetName() < b.GetName());
}

EFG_model::EFG_model() {
}

std::string EFG_model::ResponseJSON::str() const {
  gui::json::structJSON resp;
  if(nullptr == this->net) resp.addElement("n", gui::json::Null());
  else                     resp.addElement("n", *this->net.get());
  if(nullptr == this->info) resp.addElement("i", gui::json::Null());
  else                      resp.addElement("i", *this->info.get());
  return resp.str();
}

std::unique_ptr<gui::json::streamJSON> EFG_model::getNetworkJSON() {
  gui::json::arrayJSON nodes, edges;
  auto addNode = [&nodes](const std::string& label, const std::string& id, const std::string& image) {
    gui::json::structJSON temp;
    temp.addElement("label", gui::json::String(label));
    temp.addElement("shape", gui::json::String("image"));
    temp.addElement("image", gui::json::String(image));
    temp.addElement("color", gui::json::String("#000000"));
    temp.addElement("id", gui::json::String(id));
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
      gui::json::structJSON e;
      e.addElement("from", gui::json::String(edgeName));
      e.addElement("to", gui::json::String(vars.front()->GetName()));
      edges.addElement(e);
    }
    else {
      gui::json::structJSON e1;
      e1.addElement("from", gui::json::String(edgeName));
      e1.addElement("to", gui::json::String(vars.front()->GetName()));
      edges.addElement(e1);
      gui::json::structJSON e2;
      e2.addElement("from", gui::json::String(edgeName));
      e2.addElement("to", gui::json::String(vars.back()->GetName()));
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
  std::unique_ptr<gui::json::structJSON> completeJSON = std::make_unique<gui::json::structJSON>();
  completeJSON->addElement("nodes", nodes);
  completeJSON->addElement("edges", edges);
  return completeJSON;
}

std::string EFG_model::Import(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  auto itF = opt.getValues().find('f');
  if(itF == opt.getValues().end()) return resp.str();
  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>(itF->second[0]);
  }
  catch(...) {
    model.reset();
  }
  if(nullptr != model) {
    this->graph = std::move(model);
    this->isolatedVars.clear();
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
  }
  return resp.str();
}

std::string EFG_model::Append(const gui::RequestOptions& opt) {
  if(nullptr == this->graph) return this->Import(opt);
  ResponseJSON resp;
  auto itF = opt.getValues().find('f');
  if(itF == opt.getValues().end()) return resp.str();
  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>(itF->second[0]);
  }
  catch(...) {
    model.reset();
  }
  if(nullptr != model) {
    this->graph->Insert(model->GetStructure(), false);
    this->isolatedVars.clear();
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
  }
  return resp.str();
}

std::string EFG_model::Export(const gui::RequestOptions& opt){
  ResponseJSON resp;
  if(nullptr == this->graph) return resp.str();
  auto itF = opt.getValues().find('f');
  if(itF == opt.getValues().end()) return resp.str();
  this->graph->Reprint(itF->second[0]);
  return resp.str();
}

std::string EFG_model::CreateIsolatedVar(const gui::RequestOptions& opt){
  ResponseJSON resp;
  auto itV = opt.getValues().find('v');
  if(itV == opt.getValues().end()) return resp.str();
  auto itS = opt.getValues().find('s');
  if(itS == opt.getValues().end()) return resp.str();
  VariableFinder finder(*this, itV->second[0]);
  if(nullptr == finder.get()) {
    this->isolatedVars.emplace(itV->second[0] , EFG::CategoricVariable( std::atoi(itS->second[0].c_str()), itV->second[0]));
    resp.setNet(this->getNetworkJSON());
  }
  finder.release();
  return resp.str();
}

std::string EFG_model::SetObservations(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  if(nullptr == this->graph) return resp.str();

  auto itV = opt.getValues().find('v');
  auto itO = opt.getValues().find('o');

  if( (itV == opt.getValues().end()) && (itO == opt.getValues().end()) ) {
    this->graph->SetEvidences(std::vector<std::pair<std::string, std::size_t>>{});
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
    return resp.str();
  }
  if(itV == opt.getValues().end()) return resp.str();
  if(itO == opt.getValues().end()) return resp.str();
  if(itO->second.size() != itV->second.size()) return resp.str();

  auto obOld = this->graph->GetObservationSet();
  std::vector<std::pair<std::string, std::size_t>> ob;
  ob.reserve(obOld.size() + itO->second.size());
  for(auto it = obOld.begin(); it!=obOld.end(); ++it){
    ob.emplace_back(std::make_pair(it->first->GetName(), it->second));
  }
  for(std::size_t k=0; k<itO->second.size(); ++k) {
    auto* var = this->graph->FindVariable(itV->second[k]);
    std::size_t o = std::atoi(itO->second[0].c_str());
    if((nullptr != var) && ( o < var->size())){
      ob.emplace_back(std::make_pair(itV->second[k], o));
    }
  }
  this->graph->SetEvidences(ob);
  this->lastMap.clear();
  resp.setNet(this->getNetworkJSON());
  return resp.str();
}

std::string EFG_model::GetNodeInfo(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  auto itV = opt.getValues().find('v');
  if(itV == opt.getValues().end()) return resp.str();
  VariableFinder finder(*this, itV->second[0]);
  if(nullptr == finder.get()) return resp.str();
  std::unique_ptr<gui::json::structJSON> inspectRes = std::make_unique<gui::json::structJSON>();
  inspectRes->addElement("s", gui::json::Number<std::size_t>(finder.get()->size()) );
  inspectRes->addElement("i", gui::json::Number<bool>(finder.isIsolated()) );
  finder.release();
  resp.setInfo(std::move(inspectRes));
  return resp.str();
}

std::string EFG_model::GetMarginals(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  if(nullptr == this->graph) return resp.str();
  auto itV = opt.getValues().find('v');
  if(itV == opt.getValues().end()) return resp.str();
  std::vector<float> marginals;
  try {
    marginals = this->graph->GetMarginalDistribution(itV->second[0]);
  }
  catch(...) {
    return  resp.str();
  }
  std::unique_ptr<gui::json::arrayJSON> margJSON = std::make_unique<gui::json::arrayJSON>();
  for(std::size_t k=0; k<marginals.size(); ++k) {
    margJSON->addElement(gui::json::Number<float>(marginals[k]));
  }
  resp.setInfo(std::move(margJSON));
  return resp.str();
}

std::string EFG_model::RecomputeMap(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  if(nullptr == this->graph) return resp.str();
  auto hiddenSet = this->graph->GetHiddenSet();
  this->lastMap.clear();
  for(auto it = hiddenSet.begin(); it!=hiddenSet.end(); ++it) {
    this->lastMap.emplace((*it)->GetName() , this->graph->GetMAP((*it)->GetName()));
  }
  resp.setNet(this->getNetworkJSON());
  return resp.str();
}

std::string EFG_model::AddFactor(const gui::RequestOptions& opt) {
  ResponseJSON resp;
  auto itV = opt.getValues().find('v');
  if(itV == opt.getValues().end()) return resp.str();
  float w = 0.0;
  {
    auto itW = opt.getValues().find('w');
    if(itW != opt.getValues().end()) w = static_cast<float>(std::atof(itW->second[0].c_str()));
  }

  auto itF = opt.getValues().find('f');
  if(itF != opt.getValues().end()){
    bool needUpdate = false;
    if(itV->second.size() == 1) needUpdate = this->AddFactor(itV->second[0], itF->second[0], w);
    else                        needUpdate = this->AddFactor(itV->second[0], itV->second[1], itF->second[0], w);
    if(needUpdate) resp.setNet(this->getNetworkJSON());
    return resp.str();
  }

  auto itC = opt.getValues().find('c');
  if(itC == opt.getValues().end()) return resp.str();
  if(itV->second.size() == 1) return resp.str();
  bool corr_anti = true;
  if(itC->second[0].front() == 'F') corr_anti = false;
  if(this->AddFactor(itV->second[0], itV->second[1], corr_anti, w)) resp.setNet(this->getNetworkJSON());

  return resp.str();
}

bool EFG_model::AddFactor(const std::string& name, const std::string& fileName, const float& weight) {
  if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
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

bool EFG_model::AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight) {
  if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
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

bool EFG_model::AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight) {
  if(nullptr == this->graph) this->graph = std::make_unique<EFG::model::Graph>();
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
