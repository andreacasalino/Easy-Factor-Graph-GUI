#include "../efgAddon.h"

std::string efgJS::Command::getAsString(const Napi::Value& val, Napi::Env& env) {
    if(!val.IsString()) Napi::TypeError::New(env, "found input that is not a string").ThrowAsJavaScriptException();
    return std::string(val.As<Napi::String>().Utf8Value());
}

efgJS::Command::Command(const Napi::CallbackInfo& args) {
  Napi::Env env = args.Env(); 
  auto parseSymbol = [this, &args, &env](){
    std::string symbolRaw = getAsString(args[0], env);
    if(symbolRaw.size() != 1) Napi::TypeError::New(env, "command name too long").ThrowAsJavaScriptException();
    this->symbol = symbolRaw.front();
  };
  if(args.Length() == 1){
    parseSymbol();
  }
  else if(args.Length() == 3){
    parseSymbol();
    if(!args[1].IsArray()) Napi::TypeError::New(env, "second input not array").ThrowAsJavaScriptException();
    Napi::Array option_names = args[1].As<Napi::Array>();
    if(!args[1].IsArray()) Napi::TypeError::New(env, "thirs input not array").ThrowAsJavaScriptException();
    Napi::Array option_values = args[2].As<Napi::Array>();
    if(option_names.Length() != option_values.Length()) Napi::TypeError::New(env, "second and third input should be arrys of the same size").ThrowAsJavaScriptException();
    for(uint32_t  k=0; k<option_names.Length(); ++k) {
        std::string nameRaw = getAsString(option_names[k], env);
        if(nameRaw.size() != 1) Napi::TypeError::New(env, "option name too long").ThrowAsJavaScriptException();
        std::string value = getAsString(option_values[k], env);
        auto it = this->options.find(nameRaw.front());
        if(it == this->options.end()) {
          this->options.emplace(nameRaw.front(), std::vector<std::string>{ value });
        }
        else {
          it->second.push_back(value);
        }
    }
  }
  else {
    Napi::TypeError::New(env, "wrong number of inputs for a command").ThrowAsJavaScriptException(); 
  }
}

std::string efgJS::Command::str() const {
  json::structJSON commandJSON;
  commandJSON.addElement("s", json::Number<char>(this->symbol));
  json::structJSON optionJSON;
  for(auto it = this->options.begin(); it!=this->options.end(); ++it) {
    json::arrayJSON vals;
    for(std::size_t k=0; k<it->second.size(); ++k) {
      vals.addElement(json::String(it->second[k]));
    }
    optionJSON.addElement(std::to_string(it->first), vals);
  }
  commandJSON.addElement("opt", optionJSON);
  return commandJSON.str();
}