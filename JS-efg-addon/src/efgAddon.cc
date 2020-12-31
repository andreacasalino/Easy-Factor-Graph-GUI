#include "../efgAddon.h"
#include "VariableFinder.h"
#include <iostream>
using namespace Napi;

#define ARGS_CHECK(NUMBERS_EXPECTED) \
  Napi::Env env = info.Env(); \
    if (info.Length() != NUMBERS_EXPECTED) { \
      Napi::TypeError::New(env, "Wrong number of arguments")\
      .ThrowAsJavaScriptException();\
  }

#define STRING_CHECK(POSITION) \
    if (!info[POSITION].IsString()) { \
      Napi::TypeError::New(env, "input should be a string") \
      .ThrowAsJavaScriptException();\
  }

#define AS_STRING(POSITION) std::string(info[POSITION].As<Napi::String>().Utf8Value())

bool operator<(const EFG::CategoricVariable& a, const EFG::CategoricVariable& b) {
  return (a.GetName() < b.GetName());
}

efgJS::efgJS(const Napi::CallbackInfo& info) 
  : ObjectWrap(info) {
  ARGS_CHECK(0)
  this->updateJsonNodes();

  this->commands.emplace("/getJSON" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing getJSON" << std::endl;
    return this->dataJSON; 
  });
  this->commands.emplace("/getNodeType" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing getNodeType" << std::endl;
    ARGS_CHECK(2)
    nodeType nodeT = this->GetNodeType(AS_SIZE_T(1));
    switch (nodeT) {
      case nodeType::tag:
      return "t";
    case nodeType::attribute:
      return "a";
    }
    return "n"; 
  });
  this->commands.emplace("/import" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing import" << std::endl;
    ARGS_CHECK(2)
    this->Import(AS_STRING(1));
    return this->dataJSON; 
  });
  this->commands.emplace("/export" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing export" << std::endl;
    ARGS_CHECK(2)
    this->Export(AS_STRING(1)); 
    return ""; 
  });
  this->commands.emplace("/delete" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing delete" << std::endl;
    ARGS_CHECK(2)
    this->Delete(AS_SIZE_T(1));
    return this->dataJSON; 
  });
  this->commands.emplace("/rename" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing rename" << std::endl;
    ARGS_CHECK(3)
    this->Rename(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/nestTag" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing nestTag" << std::endl;
    ARGS_CHECK(3)
    this->NestTag(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/nestAttribute" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing nestAttribute" << std::endl;
    ARGS_CHECK(3)
    this->NestAttribute(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
  this->commands.emplace("/setValue" , [this](const Napi::CallbackInfo& info) -> std::string { 
    std::cout << "processing setValue" << std::endl;
    ARGS_CHECK(3)
    this->SetValue(AS_SIZE_T(1), AS_STRING(2));
    return this->dataJSON; 
  });
}

Napi::Value efgJS::ProcessRequest(const Napi::CallbackInfo& info){
  Napi::Env env = info.Env(); 
    if (info.Length() == 0) { 
      Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
  }
  for(std::size_t k=0; k<info.Length(); ++k){
    STRING_CHECK(k)
  }

  auto it = this->commands.find(AS_STRING(0));
  if(it == this->commands.end()){
    return Napi::String::New(env, "");
  }
  return Napi::String::New(env, it->second(info).c_str());
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
    this->graph->Insert(model->GetStructure());
    this->isolatedVars.clear();
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
  if(nullptr == finder) {
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
  return true;
}

bool efgJS::DeleteObservation(){
  if(nullptr == this->graph) return false;
  this->graph->SetEvidences(std::vector<std::pair<std::string, std::size_t>>{});
  return true;
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

std::vector<std::size_t> efgJS::GetMap() {
  if(nullptr == this->graph) return {};
  std::vector<std::size_t> map;
  try {
    map = this->graph->GetMAP();
  }
  catch(...) {
    return  {};
  }
  return map;
}

bool efgJS::AddFactor(const std::string& name, const std::string& fileName, const float& weight) {
  VariableFinder finder(*this, name);
  if(nullptr == finder) return false;

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
  return true;
}

bool efgJS::AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight) {
  VariableFinder finderA(*this, nameA);
  if(nullptr == finderA) return false;
  VariableFinder finderB(*this, nameB);
  if(nullptr == finderB) return false;

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
  return true;
}

bool efgJS::AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight) {
  VariableFinder finderA(*this, nameA);
  if(nullptr == finderA) return false;
  VariableFinder finderB(*this, nameB);
  if(nullptr == finderB) return false;

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
