#include "tokenizer.h"
#include "error.h"
#include <set>
#include <cctype>

SymbolToken::SymbolToken(const std::string &name) {
    this->name = name;
}

bool SymbolToken::operator==(const SymbolToken &other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

ConstantToken::ConstantToken(int value) {
    this->value = value;
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream *in) : last_token_(DotToken()) {
    this->in_ = in;
    Next();
}

bool Tokenizer::IsEnd() const {
    return end_;
}

void Tokenizer::Next() {
    auto is_head = [](const char c) {
        static std::set<char> head = {'<', '=', '>', '*', '/', '#'};
        return isalpha(c) || head.find(c) != head.end();
    };
    auto is_symb = [](const char c) {
        static std::set<char> symb = {'<', '=', '>', '*', '/', '#', '?', '!', '-'};
        return isalpha(c) || isdigit(c) || symb.find(c) != symb.end();
    };

    int c;
    while ((c = in_->get()) != -1) {
        if (!isspace(c)) {
            break;
        }
    }
    if (c == -1) {
        end_ = true;
        return;
    }
    if (c == '(') {
        last_token_ = BracketToken::OPEN;
        return;
    } else if (c == ')') {
        last_token_ = BracketToken::CLOSE;
        return;
    } else if (c == '.') {
        last_token_ = DotToken();
        return;
    } else if (c == '\'') {
        last_token_ = QuoteToken();
        return;
    }

    if (isdigit(c) || c == '-' || c == '+') {
        bool number = isdigit(c);
        if (!number) {
            int next = in_->get();
            if (isdigit(next)) {
                number = true;
            }
            in_->unget();
        }
        if (number) {
            std::string num(1, c);
            while ((c = in_->get()) != -1 && isdigit(c)) {
                num += c;
            }
            if (c != -1) {
                in_->unget();
            }
            last_token_ = ConstantToken(stoi(num));
            return;
        } else {
            last_token_ = SymbolToken(std::string(1, c));
            return;
        }
    }

    if (is_head(c)) {
        in_->unget();
        std::string symbol;
        while ((c = in_->get()) != -1 && is_symb(c)) {
            symbol += c;
        }
        if (c != -1) {
            in_->unget();
        }
        last_token_ = SymbolToken(symbol);
        return;
    } else {
        throw SyntaxError("Invalid syntax!");
    }
}

Token Tokenizer::GetToken() const {
    return last_token_;
}