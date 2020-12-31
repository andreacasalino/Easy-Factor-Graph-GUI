#ifndef VARIABLE_FINDER_H
#define VARIABLE_FINDER_H

#include "../efgAddon.h"

class efgJS::VariableFinder {
public:
    VariableFinder(efgJS& user, const std::string& name);
    ~VariableFinder();

    inline EFG::CategoricVariable* get() const { return this->varPtr; };
    void release();

    friend bool operator==(std::nullptr_t, const efgJS::VariableFinder&);
private:
    efgJS& user;
    EFG::CategoricVariable* varPtr;
    bool isIsolatedVar;
};

bool operator==(std::nullptr_t, const efgJS::VariableFinder& f);

#endif