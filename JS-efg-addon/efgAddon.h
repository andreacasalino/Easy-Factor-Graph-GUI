// https://napi.inspiredware.com/getting-started/objectwrap.html#src-object-wrap-demo-cc-and-src-object-wrap-demo-h

#include <napi.h>
#include <model/Graph.h>
#include "src/JSONstream.h"

class efgJS : public Napi::ObjectWrap<efgJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    efgJS(const Napi::CallbackInfo& info);

    Napi::Value ProcessRequest(const Napi::CallbackInfo&);

private:
    std::shared_ptr<json::streamJSON> getJSON();

    // the below methods returning a boolean, returns true if currentJSON should be updated

    bool Import(const std::string& fileName);

    bool Append(const std::string& fileName);

    void Export(const std::string& fileName);

    bool CreateIsolatedVar(const std::string& name, const std::size_t& size);

    bool AddObservation(const std::vector<std::pair<std::string, std::size_t>>& obs);

    bool DeleteObservation();

    struct NodeInfo {
        bool isIsolated;
        std::size_t size;
    };
    std::unique_ptr<NodeInfo> GetNodeInfo(const std::string& name); // node can be both variable and potentials

    std::vector<float> GetMarginals(const std::string& name);

    bool RecomputeMap();

    bool AddFactor(const std::string& name, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight = 0.f);

private:
    class Command;
    struct Request {
        const std::map<char, std::vector<std::string>>& options;
        std::shared_ptr<json::streamJSON> newNetwork;
        std::shared_ptr<json::streamJSON> info;
    };
    std::map<char, std::function<void(Request& )>> commands;

    class VariableFinder;

// data
    std::unique_ptr<EFG::model::Graph> graph;
    std::map<std::string, EFG::CategoricVariable> isolatedVars;
    std::map<std::string, std::size_t> lastMap;
};


class efgJS::Command {
public:
    Command(const Napi::CallbackInfo& args);

    inline const char& getSymbol() const { return this->symbol; };
    inline const std::map<char, std::vector<std::string>>& getOptions() const { return this->options; };
    
private:
    static std::string getAsString(const Napi::Value& val, Napi::Env& env);

    char symbol;
    std::map<char, std::vector<std::string>> options;
};


class efgJS::VariableFinder {
public:
    VariableFinder(efgJS& user, const std::string& name);
    ~VariableFinder();

    inline EFG::CategoricVariable* get() const { return this->varPtr; };
    void release();
    inline bool isIsolated() const { return this->isIsolatedVar; };
private:
    efgJS& user;
    EFG::CategoricVariable* varPtr;
    bool isIsolatedVar;
};
