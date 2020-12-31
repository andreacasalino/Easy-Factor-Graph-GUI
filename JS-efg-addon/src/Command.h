#ifndef COMMAND_H
#define COMMAND_H

#include "../efgAddon.h"

class efgJS::Command {
public:
    Command(const Napi::CallbackInfo& args);

    inline const char& getSymbol() const { return this->symbol; };
    inline const std::multimap<char, std::string>& getOptions() const { return this->options; };
private:
    static std::string getAsString(const Napi::Value& val, Napi::Env& env);

    char symbol;
    std::multimap<char, std::string> options;
};

#endif