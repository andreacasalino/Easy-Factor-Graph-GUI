#include <GraphShell.h>
#include <httplib.h>
#include <iostream>

using namespace httplib;
using namespace std;

std::string getRequestName(const std::string &incompleteName) {
  std::string temp = "/" + incompleteName;
  return temp;
}

#define REACTION(SYMBOL, METHOD)                                               \
  svr.Post(getRequestName(SYMBOL).c_str(),                                     \
           [&model, &nullResp](const Request &req, Response &res) {            \
             std::cout << "request ID: " << SYMBOL << std::endl;               \
             std::cout << "request:" << std::endl << req.body << std::endl;    \
             res.set_header("Access-Control-Allow-Origin", "*");               \
             gui::RequestPtr opt = gui::RequestOptions::parse(req.body);       \
             if (nullptr == opt) {                                             \
               res.set_content(nullResp.str(), "text/plain");                  \
             }                                                                 \
             res.set_content(METHOD(*opt), "text/plain");                      \
             std::cout << "response:" << std::endl                             \
                       << res.body << std::endl                                \
                       << std::endl;                                           \
           });

static const std::string default_example =
    std::string(EXAMPLE_FOLDER) +
    std::string("Sample06-Learning-A/graph_3.xml");

int main() {

  Server svr;
  GraphShell model;

  const GraphJSON nullResp;

  svr.Post(getRequestName("D").c_str(), [&model, &nullResp](const Request &req,
                                                            Response &res) {
    std::cout << "request ID: "
              << "D" << std::endl;
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(default_example, "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  REACTION("X", model.Import)

  REACTION("A", model.Append)

  REACTION("V", model.CreateIsolatedVar)

  REACTION("M", model.RecomputeMap)

  REACTION("O", model.ResetObservations)

  REACTION("P", model.AddFactor)

  REACTION("R", model.Export)

  REACTION("Q", model.GetVariableInfo)

  REACTION("I", model.GetMarginals)

  svr.Post(getRequestName("S").c_str(), [&model, &nullResp](const Request &req,
                                                            Response &res) {
    std::cout << "request ID: "
              << "S" << std::endl;
    res.set_header("Access-Control-Allow-Origin", "*");
    GraphJSON resp;
    std::unique_ptr<gui::json::structJSON> folder =
        std::make_unique<gui::json::structJSON>();
    folder->addElement("folder", gui::json::String(SOURCE_FOLDER));
    resp.setInfo(std::move(folder));
    res.set_content(resp.str(), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  cout << "Application started" << endl;
  svr.listen("localhost", 3000);

  return EXIT_FAILURE;
}