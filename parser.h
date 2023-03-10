#pragma once

#include <memory>
#include <vector>

#include "object.h"
#include "error.h"
#include "tokenizer.h"

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer);

std::shared_ptr<Object> Read(Tokenizer* tokenizer);