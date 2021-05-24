#include <httplib.h>
#include <iostream>
#include <GraphShell.h>

using namespace httplib;
using namespace std;

std::string getRequestName(const std::string& incompleteName) {
  std::string temp = "/" + incompleteName;
  return temp;
}

#define REACTION(SYMBOL, METHOD) \
  svr.Post(getRequestName(SYMBOL).c_str() , [&model, &nullResp](const Request& req, Response& res) { \
    std::cout << "request ID: " << SYMBOL << std::endl; \
    std::cout << "request:" << std::endl << req.body << std::endl; \
    res.set_header("Access-Control-Allow-Origin", "*"); \
    gui::RequestPtr opt = gui::RequestOptions::parse(req.body); \
    if(nullptr == opt) { \
      res.set_content(nullResp.str(), "text/plain"); \
    } \
    res.set_content(model.METHOD(*opt), "text/plain"); \
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl; \
  });

int main() {

  Server svr;
  GraphShell model;

  const GraphJSON nullResp;

  REACTION("X", Import)

  REACTION("A", Append)

  REACTION("V", CreateIsolatedVar)

  REACTION("M", RecomputeMap)

  REACTION("O", ResetObservations)

  REACTION("P", AddFactor)

  REACTION("R", Export)

  REACTION("Q", GetVariableInfo)

  REACTION("I", GetMarginals)

  svr.Post(getRequestName("S").c_str(), [&model, &nullResp](const Request& req, Response& res) { \
    std::cout << "request ID: " << "S" << std::endl;
    res.set_header("Access-Control-Allow-Origin", "*");
    GraphJSON resp;
    std::unique_ptr<gui::json::structJSON> folder = std::make_unique<gui::json::structJSON>();
    folder->addElement("folder", gui::json::String(SOURCE_FOLDER));
    resp.setInfo(std::move(folder));
    res.set_content(resp.str(), "text/plain");
    std::cout << "response:" << std::endl << res.body << std::endl << std::endl;
  });

  cout << "Application started" << endl;
  svr.listen("localhost", 3000);

  return EXIT_FAILURE;
}