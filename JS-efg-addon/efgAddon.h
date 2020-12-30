// https://napi.inspiredware.com/getting-started/objectwrap.html#src-object-wrap-demo-cc-and-src-object-wrap-demo-h

#include <napi.h>
#include <Graph.h>
#include <set>

class efgJS : public Napi::ObjectWrap<efgJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    efgJS(const Napi::CallbackInfo& info);

    Napi::Value ProcessRequest(const Napi::CallbackInfo&);

private:
    std::string getJSON();

    void Import(const std::string& fileName);

    void Append(const std::string& fileName);

    void Export(const std::string& fileName);

    void CreateIsolatedVar(const std::string& name, const std::size_t& size);

    void AddObservation(const std::vector<std::pair<std::string, std::size_t>>& obs);

    void DeleteObservation();

    std::vector<float> GetMarginals(const std::string& name);

    std::vector<std::size_t> GetMap();

    void AddFactor(const std::string& name, const std::string& fileName, const float& weight = 0.f);

    void AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight = 0.f);

    void AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight = 0.f);

private:
    std::map<std::string, std::function<std::string(const Napi::CallbackInfo&)>> commands;

// data
    std::unique_ptr<EFG::model::Graph> graph;
    std::set<EFG::CategoricVariable> isolatedVars;
};
