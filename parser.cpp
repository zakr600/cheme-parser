#include <memory>
#include <vector>

#include "object.h"
#include "error.h"
#include "tokenizer.h"
#include "parser.h"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Syntax error!");
    }
    Token token = tokenizer->GetToken();
    tokenizer->Next();
    if (BracketToken* bracket_token = std::get_if<BracketToken>(&token)) {
        if (*bracket_token == BracketToken::OPEN) {
            return ReadList(tokenizer);
        } else if (*bracket_token == BracketToken::CLOSE) {
            throw SyntaxError("Syntax error!");
        }
    }
    if (ConstantToken* constant_token = std::get_if<ConstantToken>(&token)) {
        return std::make_shared<Number>(constant_token->value);
    }
    if (SymbolToken* symbol_token = std::get_if<SymbolToken>(&token)) {
        return std::make_shared<Symbol>(symbol_token->name);
    }
    if (DotToken* dot_token = std::get_if<DotToken>(&token)) {
        return std::make_shared<Dot>();
    }
    if (QuoteToken* quote_token = std::get_if<QuoteToken>(&token)) {
        std::shared_ptr<Object> first = std::make_shared<Symbol>("quote");
        std::shared_ptr<Object> second = Read(tokenizer);
        std::shared_ptr<Object> empt;
        std::shared_ptr<Object> real_second = std::make_shared<Cell>(second, empt);
        return std::make_shared<Cell>(first, real_second);
    }
    throw SyntaxError("Syntax error!");
}

bool IsCloseBracketToken(Token token) {
    if (BracketToken* bracket_token = std::get_if<BracketToken>(&token)) {
        if (*bracket_token == BracketToken::CLOSE) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    std::vector<std::shared_ptr<Object>> items;
    if (tokenizer->IsEnd()) {
        throw SyntaxError("Syntax error!");
    }
    while (!IsCloseBracketToken(tokenizer->GetToken())) {
        items.push_back(Read(tokenizer));
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Syntax error!");
        }
    }
    tokenizer->Next();
    std::shared_ptr<Object> t;
    for (size_t i = 0; i < items.size(); ++i) {
        if ((items.size() == 2 || i + 2 != items.size()) && Is<Dot>(items[i])) {
            throw SyntaxError("Syntax error!");
        }
    }
    if (items.size() == 3 && Is<Dot>(items[1])) {
        return std::make_shared<Cell>(items[0], items[2]);
    }
    if (items.size() >= 3 && Is<Dot>(items[items.size() - 2])) {
        t = std::make_shared<Cell>(items[items.size() - 3], items[items.size() - 1]);
        for (size_t pi = items.size(); pi >= 4; --pi) {
            int i = pi - 4;
            t = std::make_shared<Cell>(items[i], t);
        }
    } else {
        for (size_t pi = items.size(); pi > 0; --pi) {
            int i = pi - 1;
            t = std::make_shared<Cell>(items[i], t);
        }
    }
    return t;
}