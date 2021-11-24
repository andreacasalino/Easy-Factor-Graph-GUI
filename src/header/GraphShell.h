#ifndef GRAPH_SHELL_H
#define GRAPH_SHELL_H

#include <model/Graph.h>
#include <JSONstream.h>
#include <RequestOptions.h>

class GraphJSON {
public:
    GraphJSON() = default;

    inline void setNet(std::unique_ptr<gui::json::streamJSON> n) { this->net = std::move(n); };
    inline void setInfo(std::unique_ptr<gui::json::streamJSON> i) { this->info = std::move(i); };

    std::string str() const;

private:
    std::unique_ptr<gui::json::streamJSON> net;
    std::unique_ptr<gui::json::streamJSON> info;
};



class GraphShell {
public:
    GraphShell() = default;

    std::string Import(const gui::RequestOptions& opt);

    std::string Append(const gui::RequestOptions& opt);

    std::string CreateIsolatedVar(const gui::RequestOptions& opt);

    std::string RecomputeMap(const gui::RequestOptions& opt);

    std::string ResetObservations(const gui::RequestOptions& opt);

    std::string Export(const gui::RequestOptions& opt);

    std::string GetVariableInfo(const gui::RequestOptions& opt); // node can be both variable and potentials

    std::string GetMarginals(const gui::RequestOptions& opt);

    std::string AddFactor(const gui::RequestOptions& opt);

private:
    std::unique_ptr<gui::json::streamJSON> getNetworkJSON();

    EFG::categoric::VariablePtr findVariable(const std::string& name) const; // nullptr when non existing

    void removeIfIsolated(const EFG::categoric::VariablePtr& var);

    bool AddFactor(const std::string& name, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const std::string& fileName, const float& weight = 0.f);

    bool AddFactor(const std::string& nameA, const std::string& nameB, const bool& corr_anti, const float& weight = 0.f);

// data
    std::unique_ptr<EFG::model::Graph> graph;
    std::set<EFG::categoric::VariablePtr> isolatedVars;
    std::map<EFG::categoric::VariablePtr, std::size_t> lastMap;
};

#endif
