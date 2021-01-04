#include <model/Graph.h>
#include <JSONstream.h>
#include <RequestOptions.h>

class EFG_model {
public:
    EFG_model();

    std::string Import(const gui::RequestOptions& opt);

    std::string Append(const gui::RequestOptions& opt);

    std::string CreateIsolatedVar(const gui::RequestOptions& opt);

    std::string RecomputeMap(const gui::RequestOptions& opt);

    std::string SetObservations(const gui::RequestOptions& opt);

    std::string Export(const gui::RequestOptions& opt);

    std::string GetNodeInfo(const gui::RequestOptions& opt); // node can be both variable and potentials

    std::string GetMarginals(const gui::RequestOptions& opt);

    std::string AddFactor(const gui::RequestOptions& opt);

    class ResponseJSON {
    public:
        ResponseJSON() = default;

        inline void setNet(std::unique_ptr<gui::json::streamJSON> n) { this->net = std::move(n); };
        inline void setInfo(std::unique_ptr<gui::json::streamJSON> i) { this->info = std::move(i); };

        std::string str() const;
    private:
        std::unique_ptr<gui::json::streamJSON> net;
        std::unique_ptr<gui::json::streamJSON> info;
    };
private:
    std::unique_ptr<gui::json::streamJSON> getNetworkJSON();

    bool AddFactor(const std::string& name, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight = 0.f);

private:
    class VariableFinder;

// data
    std::unique_ptr<EFG::model::Graph> graph;
    std::map<std::string, EFG::CategoricVariable> isolatedVars;
    std::map<std::string, std::size_t> lastMap;
};


class EFG_model::VariableFinder {
public:
    VariableFinder(EFG_model& user, const std::string& name);
    ~VariableFinder();

    inline EFG::CategoricVariable* get() const { return this->varPtr; };
    void release();
    inline bool isIsolated() const { return this->isIsolatedVar; };
private:
    EFG_model& user;
    EFG::CategoricVariable* varPtr;
    bool isIsolatedVar;
};
