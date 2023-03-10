#pragma once

#include <string>
#include <map>
#include "object.h"

class Interpreter {
public:
    std::string Run(const std::string&);

private:
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>> vars_;
};
