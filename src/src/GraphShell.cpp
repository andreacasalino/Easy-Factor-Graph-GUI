#include <GraphShell.h>
#include <io/xml/Exporter.h>
#include <io/xml/Importer.h>
#include <iostream>

static const std::string IMAGE_FOLDER = "./image/Model/";

std::string GraphJSON::str() const {
  gui::json::structJSON resp;
  if (nullptr == this->net)
    resp.addElement("n", gui::json::Null());
  else
    resp.addElement("n", *this->net.get());
  if (nullptr == this->info)
    resp.addElement("i", gui::json::Null());
  else
    resp.addElement("i", *this->info.get());
  return resp.str();
}

std::unique_ptr<gui::json::streamJSON> GraphShell::getNetworkJSON() {
  gui::json::arrayJSON nodes, edges;
  auto addNode = [&nodes](const std::string &label, const std::string &id,
                          const std::string &image) {
    gui::json::structJSON temp;
    temp.addElement("label", gui::json::String(label));
    temp.addElement("shape", gui::json::String("image"));
    temp.addElement("image", gui::json::String(image));
    temp.addElement("color", gui::json::String("#000000"));
    temp.addElement("id", gui::json::String(id));
    nodes.addElement(temp);
  };
  // process variables
  if (nullptr != this->graph) {
    auto hiddenVars = this->graph->getHiddenVariables();
    for (auto it = hiddenVars.begin(); it != hiddenVars.end(); ++it) {
      auto itMap = this->lastMap.find(*it);
      if (itMap == this->lastMap.end()) {
        addNode((*it)->name(), (*it)->name(), IMAGE_FOLDER + "Variable.svg");
      } else {
        addNode((*it)->name() + "=" + std::to_string(itMap->second) + " (MAP)",
                (*it)->name(), IMAGE_FOLDER + "Variable.svg");
      }
    }
    auto obVars = this->graph->getEvidences();
    for (auto it = obVars.begin(); it != obVars.end(); ++it) {
      addNode(it->first->name() + "=" + std::to_string(it->second),
              it->first->name(), IMAGE_FOLDER + "Variable_Observed.svg");
    }
  }
  for (auto it = this->isolatedVars.begin(); it != this->isolatedVars.end();
       ++it) {
    addNode((*it)->name(), (*it)->name(), IMAGE_FOLDER + "Variable.svg");
  }
  // process potentials
  std::size_t counter = 0;
  auto addEdge = [&edges, &nodes, &counter,
                  &addNode](const EFG::distribution::Distribution &pot,
                            const std::string &image) {
    std::string edgeName = "__edge" + std::to_string(counter);
    addNode("", edgeName, image);
    auto vars = pot.getGroup().getVariables();
    if (vars.size() == 1) {
      gui::json::structJSON e;
      e.addElement("from", gui::json::String(edgeName));
      e.addElement("to", gui::json::String((*vars.begin())->name()));
      edges.addElement(e);
    } else {
      gui::json::structJSON e1;
      e1.addElement("from", gui::json::String(edgeName));
      e1.addElement("to", gui::json::String((*vars.begin())->name()));
      edges.addElement(e1);
      gui::json::structJSON e2;
      e2.addElement("from", gui::json::String(edgeName));
      e2.addElement("to", gui::json::String((*vars.rbegin())->name()));
      edges.addElement(e2);
    }
    ++counter;
  };
  if (nullptr != this->graph) {
    auto factors = this->graph->getConstFactors();
    for (auto it = factors.begin(); it != factors.end(); ++it) {
      addEdge(**it, IMAGE_FOLDER + "Potential_Shape.svg");
    }
    auto factorsExp = this->graph->getConstFactorsExp();
    for (auto it = factorsExp.begin(); it != factorsExp.end(); ++it) {
      addEdge(**it, IMAGE_FOLDER + "Potential_Exp_Shape_fixed.svg");
    }
  }
  std::unique_ptr<gui::json::structJSON> completeJSON =
      std::make_unique<gui::json::structJSON>();
  completeJSON->addElement("nodes", nodes);
  completeJSON->addElement("edges", edges);
  return completeJSON;
}

std::string GraphShell::Import(const gui::RequestOptions &opt) {
  GraphJSON resp;
  auto itF = opt.getValues().find('f');
  if (itF == opt.getValues().end())
    return resp.str();
  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>();
    EFG::io::xml::Importer::importFromXml(*model, itF->second[0]);
  } catch (...) {
    model.reset();
  }
  if (nullptr != model) {
    this->graph = std::move(model);
    this->isolatedVars.clear();
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
  }
  return resp.str();
}

std::string GraphShell::Append(const gui::RequestOptions &opt) {
  if (nullptr == this->graph)
    return this->Import(opt);
  GraphJSON resp;
  auto itF = opt.getValues().find('f');
  if (itF == opt.getValues().end())
    return resp.str();
  std::unique_ptr<EFG::model::Graph> model;
  try {
    model = std::make_unique<EFG::model::Graph>();
    EFG::io::xml::Importer::importFromXml(*model, itF->second[0]);
  } catch (...) {
    model.reset();
  }
  if (nullptr != model) {
    this->graph->absorbModel(*model, true);
    this->isolatedVars.clear();
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
  }
  return resp.str();
}

std::string GraphShell::Export(const gui::RequestOptions &opt) {
  GraphJSON resp;
  if (nullptr == this->graph)
    return resp.str();
  auto itF = opt.getValues().find('f');
  if (itF == opt.getValues().end())
    return resp.str();
  EFG::io::xml::Exporter::exportToXml(*this->graph, itF->second[0]);
  return resp.str();
}

std::string GraphShell::CreateIsolatedVar(const gui::RequestOptions &opt) {
  GraphJSON resp;
  auto itV = opt.getValues().find('v');
  if (itV == opt.getValues().end())
    return resp.str();
  auto itS = opt.getValues().find('s');
  if (itS == opt.getValues().end())
    return resp.str();
  EFG::categoric::VariablePtr var = std::make_shared<EFG::categoric::Variable>(
      std::atoi(itS->second[0].c_str()), itV->second[0]);
  auto itI = this->isolatedVars.find(var);
  if (itI == this->isolatedVars.end()) {
    this->isolatedVars.emplace(var);
    resp.setNet(this->getNetworkJSON());
  }
  return resp.str();
}

std::string GraphShell::ResetObservations(const gui::RequestOptions &opt) {
  GraphJSON resp;
  if (nullptr == this->graph)
    return resp.str();

  auto itV = opt.getValues().find('v');
  auto itO = opt.getValues().find('o');

  if ((itV == opt.getValues().end()) && (itO == opt.getValues().end())) {
    // clean all evidences
    this->graph->resetEvidences({});
    this->lastMap.clear();
    resp.setNet(this->getNetworkJSON());
    return resp.str();
  }
  if (itV == opt.getValues().end())
    return resp.str();
  if (itO == opt.getValues().end())
    return resp.str();
  if (itO->second.size() != itV->second.size())
    return resp.str();

  auto obOld = this->graph->getEvidences();
  std::map<std::string, std::size_t> ob;
  for (auto it = obOld.begin(); it != obOld.end(); ++it) {
    ob.emplace(it->first->name(), it->second);
  }
  for (std::size_t k = 0; k < itO->second.size(); ++k) {
    auto var = this->graph->findVariable(itV->second[k]);
    std::size_t o = std::atoi(itO->second[0].c_str());
    if ((nullptr != var) && (o < var->size())) {
      ob.emplace(var->name(), o);
    }
  }
  this->graph->resetEvidences(ob);
  this->lastMap.clear();
  resp.setNet(this->getNetworkJSON());
  return resp.str();
}

std::string GraphShell::GetVariableInfo(const gui::RequestOptions &opt) {
  GraphJSON resp;
  auto itV = opt.getValues().find('v');
  if (itV == opt.getValues().end())
    return resp.str();
  auto info = this->findVariable(itV->second[0]);
  if (nullptr == info) {
    return resp.str();
  }
  std::unique_ptr<gui::json::structJSON> inspectRes =
      std::make_unique<gui::json::structJSON>();
  inspectRes->addElement("s", gui::json::Number<std::size_t>(info->size()));
  inspectRes->addElement(
      "i", gui::json::Number<bool>(this->isolatedVars.find(info) !=
                                   this->isolatedVars.end()));
  resp.setInfo(std::move(inspectRes));
  return resp.str();
}

std::string GraphShell::GetMarginals(const gui::RequestOptions &opt) {
  GraphJSON resp;
  if (nullptr == this->graph)
    return resp.str();
  auto itV = opt.getValues().find('v');
  if (itV == opt.getValues().end())
    return resp.str();
  std::vector<float> marginals;
  try {
    marginals = this->graph->getMarginalDistribution(itV->second[0]);
  } catch (...) {
    return resp.str();
  }
  std::unique_ptr<gui::json::arrayJSON> margJSON =
      std::make_unique<gui::json::arrayJSON>();
  for (std::size_t k = 0; k < marginals.size(); ++k) {
    margJSON->addElement(gui::json::Number<float>(marginals[k]));
  }
  resp.setInfo(std::move(margJSON));
  return resp.str();
}

std::string GraphShell::RecomputeMap(const gui::RequestOptions &opt) {
  GraphJSON resp;
  if (nullptr == this->graph)
    return resp.str();
  auto hiddenSet = this->graph->getHiddenVariables();
  this->lastMap.clear();
  for (auto it = hiddenSet.begin(); it != hiddenSet.end(); ++it) {
    this->lastMap.emplace(*it, this->graph->getMAP((*it)->name()));
  }
  resp.setNet(this->getNetworkJSON());
  return resp.str();
}

EFG::categoric::VariablePtr
GraphShell::findVariable(const std::string &name) const {
  auto itIso = this->isolatedVars.find(
      std::make_unique<EFG::categoric::Variable>(2, name));
  if (itIso != this->isolatedVars.end()) {
    return *itIso;
  }
  if (nullptr == this->graph) {
    return nullptr;
  }
  return this->graph->findVariable(name);
}

void GraphShell::removeIfIsolated(const EFG::categoric::VariablePtr &var) {
  auto it = this->isolatedVars.find(var);
  if (it != this->isolatedVars.end()) {
    this->isolatedVars.erase(it);
  }
}

std::string GraphShell::AddFactor(const gui::RequestOptions &opt) {
  GraphJSON resp;
  auto itV = opt.getValues().find('v');
  if (itV == opt.getValues().end())
    return resp.str();
  float w = 0.0;
  {
    auto itW = opt.getValues().find('w');
    if (itW != opt.getValues().end())
      w = static_cast<float>(std::atof(itW->second[0].c_str()));
  }

  auto itF = opt.getValues().find('f');
  if (itF != opt.getValues().end()) {
    bool needUpdate = false;
    if (itV->second.size() == 1)
      needUpdate = this->AddFactor(itV->second[0], itF->second[0], w);
    else
      needUpdate =
          this->AddFactor(itV->second[0], itV->second[1], itF->second[0], w);
    if (needUpdate)
      resp.setNet(this->getNetworkJSON());
    return resp.str();
  }

  auto itC = opt.getValues().find('c');
  if (itC == opt.getValues().end())
    return resp.str();
  if (itV->second.size() == 1)
    return resp.str();
  bool corr_anti = true;
  if (itC->second[0].front() == 'F')
    corr_anti = false;
  if (this->AddFactor(itV->second[0], itV->second[1], corr_anti, w))
    resp.setNet(this->getNetworkJSON());

  return resp.str();
}

bool GraphShell::AddFactor(const std::string &name, const std::string &fileName,
                           const float &weight) {
  auto info = this->findVariable(name);
  if (nullptr == info)
    return false;
  try {
    std::shared_ptr<EFG::distribution::factor::cnst::Factor> pot =
        std::make_shared<EFG::distribution::factor::cnst::Factor>(
            std::set<EFG::categoric::VariablePtr>{info}, fileName);
    if (nullptr == this->graph)
      this->graph = std::make_unique<EFG::model::Graph>();
    if (0.f == weight) {
      this->graph->insert(pot);
    } else {
      this->graph->insert(
          std::make_shared<EFG::distribution::factor::cnst::FactorExponential>(
              *pot, weight));
    }
  } catch (...) {
    return false;
  }
  this->removeIfIsolated(info);
  this->lastMap.clear();
  return true;
}

bool GraphShell::AddFactor(const std::string &nameA, const std::string &nameB,
                           const std::string &fileName, const float &weight) {
  auto infoA = this->findVariable(nameA);
  if (nullptr == infoA)
    return false;
  auto infoB = this->findVariable(nameB);
  if (nullptr == infoB)
    return false;
  try {
    std::shared_ptr<EFG::distribution::factor::cnst::Factor> pot =
        std::make_shared<EFG::distribution::factor::cnst::Factor>(
            std::set<EFG::categoric::VariablePtr>{infoA, infoB}, fileName);
    if (nullptr == this->graph)
      this->graph = std::make_unique<EFG::model::Graph>();
    if (0.f == weight) {
      this->graph->insert(pot);
    } else {
      this->graph->insert(
          std::make_shared<EFG::distribution::factor::cnst::FactorExponential>(
              *pot, weight));
    }
  } catch (...) {
    return false;
  }
  this->removeIfIsolated(infoA);
  this->removeIfIsolated(infoB);
  this->lastMap.clear();
  return true;
}

bool GraphShell::AddFactor(const std::string &nameA, const std::string &nameB,
                           const bool &corr_anti, const float &weight) {
  auto infoA = this->findVariable(nameA);
  if (nullptr == infoA)
    return false;
  auto infoB = this->findVariable(nameB);
  if (nullptr == infoB)
    return false;
  try {
    std::shared_ptr<EFG::distribution::factor::cnst::Factor> pot =
        std::make_shared<EFG::distribution::factor::cnst::Factor>(
            std::set<EFG::categoric::VariablePtr>{infoA, infoB}, corr_anti);
    if (nullptr == this->graph)
      this->graph = std::make_unique<EFG::model::Graph>();
    if (0.f == weight) {
      this->graph->insert(pot);
    } else {
      this->graph->insert(
          std::make_shared<EFG::distribution::factor::cnst::FactorExponential>(
              *pot, weight));
    }
  } catch (...) {
    return false;
  }
  this->removeIfIsolated(infoA);
  this->removeIfIsolated(infoB);
  this->lastMap.clear();
  return true;
}
