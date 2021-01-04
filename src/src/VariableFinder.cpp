#include <EFG-model.h>

EFG_model::VariableFinder::VariableFinder(EFG_model& user, const std::string& name)
    : user(user)
    , varPtr(nullptr)
    , isIsolatedVar(false) {
    auto it = this->user.isolatedVars.find(name);
    if(it == this->user.isolatedVars.end() && (nullptr != this->user.graph)) {
        this->varPtr = this->user.graph->FindVariable(name);
    }
    else {
        this->isIsolatedVar =  true;
        this->varPtr = &it->second;
    }
}

EFG_model::VariableFinder::~VariableFinder() {
    if(this->isIsolatedVar) {
        auto it = this->user.isolatedVars.find(this->varPtr->GetName());
        this->user.isolatedVars.erase(it);
    }
}

void EFG_model::VariableFinder::release() {
    this->isIsolatedVar = false;
    this->varPtr = nullptr;
};