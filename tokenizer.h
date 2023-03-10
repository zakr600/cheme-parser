#pragma once

#include <variant>
#include <optional>
#include <istream>

struct SymbolToken {
    SymbolToken(const std::string& name);

    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    ConstantToken(int value);

    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd() const;

    void Next();

    Token GetToken() const;

private:
    std::istream* in_;
    Token last_token_;
    bool end_ = false;
};