#include <httplib.h>
#include <iostream>
#include <EFG-model.h>

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
  EFG_model model;

  const EFG_model::ResponseJSON nullResp;

  REACTION("X", Import)

  REACTION("A", Append)

  REACTION("V", CreateIsolatedVar)

  REACTION("M", RecomputeMap)

  REACTION("O", SetObservations)

  REACTION("P", AddFactor)

  REACTION("R", Export)

  REACTION("Q", GetNodeInfo)

  REACTION("I", GetMarginals)

  svr.listen("localhost", 3000);

  return EXIT_FAILURE;
}