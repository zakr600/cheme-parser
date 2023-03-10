#include "scheme.h"

#include <parser.h>
#include <sstream>
#include <string>

void ToVectorHelper(std::vector<std::shared_ptr<Object>>& list, std::shared_ptr<Object> tree) {
    if (Is<Cell>(tree)) {
        std::shared_ptr<Cell> cell = As<Cell>(tree);
        list.push_back(cell->GetFirst());
        ToVectorHelper(list, cell->GetSecond());
    } else if (tree.operator bool()) {
        list.push_back(tree);
    }
}

std::shared_ptr<Object> Evaluate(
    std::shared_ptr<Object> tree,
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& vars);

using Iterator = std::vector<std::shared_ptr<Object>>::iterator;

void CheckHasArgumentsHelper(const Iterator& begin, const Iterator& end,
                             std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>&) {
    if (begin == end) {
        throw RuntimeError("Runtime error!");
    }
}

void CheckHasHasOneArgumentHelper(
    const Iterator& begin, const Iterator& end,
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>&) {
    if (begin == end || begin + 1 != end) {
        throw RuntimeError("Runtime error!");
    }
}

template <class T>
void CheckEvaluatedArgumentsHelper(
    const Iterator& begin, const Iterator& end,
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& vars) {
    for (Iterator it = begin; it != end; ++it) {
        *it = Evaluate(*it, vars);
        if (!Is<T>(*it)) {
            throw RuntimeError("Runtime error!");
        }
    }
}

void EvaluteAllHelper(const Iterator& begin, const Iterator& end,
                      std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& vars) {
    for (Iterator it = begin; it != end; ++it) {
        *it = Evaluate(*it, vars);
    }
}

bool ToBooleanHelper(std::shared_ptr<Object> item) {
    if (Is<Symbol>(item)) {
        std::string name = As<Symbol>(item)->GetName();
        if (name == "#f") {
            return false;
        }
        if (name == "#t") {
            return true;
        }
        return true;
        //        throw RuntimeError("Runtime error!");
    }
    return true;
}

bool IsTrueList(std::shared_ptr<Object> item) {
    if (item == nullptr) {
        return true;
    }
    if (!Is<Cell>(item)) {
        return false;
    }
    return IsTrueList(As<Cell>(item)->GetSecond());
}

std::shared_ptr<Object> Evaluate(
    std::shared_ptr<Object> tree,
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& vars) {
    //    tree = InsertVariables(tree, vars);
    if (!tree) {
        throw RuntimeError("Runtime error!");
    }
    if (Is<Cell>(tree)) {
        std::shared_ptr<Object> l = As<Cell>(tree)->GetFirst();
        if (Is<Symbol>(l) && As<Symbol>(l)->GetName() == "quote") {
            return (As<Cell>(As<Cell>(tree)->GetSecond())->GetFirst());
        }
    }
    if (Is<Number>(tree)) {
        return tree;
    }
    if (Is<Symbol>(tree)) {
        std::shared_ptr<Symbol> symbol = As<Symbol>(tree);
        std::string name = symbol.get()->GetName();
        if (name == "#t" || name == "#f") {
            return tree;
        }
        if (vars.find(name) != vars.end()) {
            return *vars.at(name);
        }
        throw NameError("Name error");
    }
    if (Is<Cell>(tree)) {
        std::vector<std::shared_ptr<Object>> list;
        ToVectorHelper(list, tree);
        if (Is<Cell>(list[0])) {
            list[0] = Evaluate(list[0], vars);
        }
        if (Is<Lambda>(list[0])) {
            std::shared_ptr<Lambda> lmbd = As<Lambda>(list[0]);
            list = std::vector<std::shared_ptr<Object>>(list.begin() + 1, list.end());
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (lmbd.get()->GetVars().size() != list.size()) {
                throw SyntaxError("Syntax error!");
            }
            std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>> new_vars =
                lmbd->GetLocalVars();
            for (size_t i = 0; i < lmbd.get()->GetVars().size(); ++i) {
                std::shared_ptr<std::shared_ptr<Object>> sp =
                    std::make_shared<std::shared_ptr<Object>>(list[i]);
                new_vars[lmbd.get()->GetVars()[i]] = sp;
            }
            for (size_t i = 0; i + 1 < lmbd->GetFunc().size(); ++i) {
                Evaluate(lmbd->GetFunc()[i], new_vars);
            }
            return Evaluate(lmbd->GetFunc().back(), new_vars);
        }
        if (!Is<Symbol>(list[0])) {
            throw RuntimeError("Runtime error!");
        }

        std::string func = As<Symbol>(list[0]).get()->GetName();
        list = std::vector<std::shared_ptr<Object>>(list.begin() + 1, list.end());
        if (func == "+") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = 0;
            for (size_t i = 0; i < list.size(); ++i) {
                ans += As<Number>(list[i]).get()->GetValue();
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "-") {
            CheckHasArgumentsHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = (list.empty() ? 0 : As<Number>(list[0]).get()->GetValue());
            for (size_t i = 1; i < list.size(); ++i) {
                ans -= As<Number>(list[i]).get()->GetValue();
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "*") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = 1;
            for (size_t i = 0; i < list.size(); ++i) {
                ans *= As<Number>(list[i]).get()->GetValue();
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "/") {
            CheckHasArgumentsHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = As<Number>(list[0]).get()->GetValue();
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i]).get()->GetValue() == 0) {
                    throw RuntimeError("Runtime error!");
                }
                ans /= As<Number>(list[i]).get()->GetValue();
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "number?") {
            if (list.size() != 1) {
                throw RuntimeError("Runtime error!");
            }
            EvaluteAllHelper(list.begin(), list.end(), vars);
            bool b = Is<Number>(list[0]);
            return std::make_shared<Symbol>(b ? "#t" : "#f");
        }
        if (func == "=") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i]).get()->GetValue() !=
                    As<Number>(list[0]).get()->GetValue()) {
                    return std::make_shared<Symbol>("#f");
                }
            }
            return std::make_shared<Symbol>("#t");
        }
        if (func == ">") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i - 1]).get()->GetValue() <=
                    As<Number>(list[i]).get()->GetValue()) {
                    return std::make_shared<Symbol>("#f");
                }
            }
            return std::make_shared<Symbol>("#t");
        }
        if (func == ">=") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i - 1]).get()->GetValue() <
                    As<Number>(list[i]).get()->GetValue()) {
                    return std::make_shared<Symbol>("#f");
                }
            }
            return std::make_shared<Symbol>("#t");
        }
        if (func == "<") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i - 1]).get()->GetValue() >=
                    As<Number>(list[i]).get()->GetValue()) {
                    return std::make_shared<Symbol>("#f");
                }
            }
            return std::make_shared<Symbol>("#t");
        }
        if (func == "<=") {
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            for (size_t i = 1; i < list.size(); ++i) {
                if (As<Number>(list[i - 1]).get()->GetValue() >
                    As<Number>(list[i]).get()->GetValue()) {
                    return std::make_shared<Symbol>("#f");
                }
            }
            return std::make_shared<Symbol>("#t");
        }
        if (func == "max") {
            CheckHasArgumentsHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = As<Number>(list[0])->GetValue();
            for (size_t i = 1; i < list.size(); ++i) {
                int now = As<Number>(list[i])->GetValue();
                if (now > ans) {
                    ans = now;
                }
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "min") {
            CheckHasArgumentsHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            int ans = As<Number>(list[0])->GetValue();
            for (size_t i = 1; i < list.size(); ++i) {
                int now = As<Number>(list[i])->GetValue();
                if (now < ans) {
                    ans = now;
                }
            }
            return std::make_shared<Number>(ans);
        }
        if (func == "abs") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Number>(list.begin(), list.end(), vars);
            return std::make_shared<Number>(abs(As<Number>(list[0])->GetValue()));
        }
        if (func == "boolean?") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (Is<Symbol>(list[0])) {
                std::string name = As<Symbol>(list[0])->GetName();
                if (name == "#f" || name == "#t") {
                    return std::make_shared<Symbol>("#t");
                }
            }
            return std::make_shared<Symbol>("#f");
        }
        if (func == "not") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            EvaluteAllHelper(list.begin(), list.end(), vars);
            return std::make_shared<Symbol>(ToBooleanHelper(list[0]) ? "#f" : "#t");
        }
        if (func == "and") {
            if (list.empty()) {
                return std::make_shared<Symbol>("#t");
            }
            for (size_t i = 0; i < list.size(); ++i) {
                list[i] = Evaluate(list[i], vars);
                bool b = ToBooleanHelper(list[i]);
                if (!b || i + 1 == list.size()) {
                    return list[i];
                }
            }
        }
        if (func == "or") {
            if (list.empty()) {
                return std::make_shared<Symbol>("#f");
            }
            for (size_t i = 0; i < list.size(); ++i) {
                list[i] = Evaluate(list[i], vars);
                bool b = ToBooleanHelper(list[i]);
                if (b || i + 1 == list.size()) {
                    return list[i];
                }
            }
        }
        if (func == "pair?") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (Is<Cell>(list[0])) {
                std::shared_ptr<Cell> cell = As<Cell>(list[0]);
                std::vector<std::shared_ptr<Object>> list2;
                ToVectorHelper(list2, cell);
                if (list2.size() == 2) {
                    return std::make_shared<Symbol>("#t");
                }
            }
            return std::make_shared<Symbol>("#f");
        }
        if (func == "null?") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (list[0] == nullptr) {
                return std::make_shared<Symbol>("#t");
            }
            return std::make_shared<Symbol>("#f");
        }
        if (func == "list?") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (list[0] == nullptr) {
                return std::make_shared<Symbol>("#t");
            }
            if (Is<Cell>(list[0])) {
                if (IsTrueList(list[0])) {
                    return std::make_shared<Symbol>("#t");
                }
            }
            return std::make_shared<Symbol>("#f");
        }
        if (func == "cons") {
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (list.size() != 2) {
                throw RuntimeError("Runtime error!");
            }
            return std::make_shared<Cell>(list[0], list[1]);
        }
        if (func == "car") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Cell>(list.begin(), list.end(), vars);
            return As<Cell>(list[0])->GetFirst();
        }
        if (func == "cdr") {
            CheckHasHasOneArgumentHelper(list.begin(), list.end(), vars);
            CheckEvaluatedArgumentsHelper<Cell>(list.begin(), list.end(), vars);
            return As<Cell>(list[0])->GetSecond();
        }
        if (func == "list") {
            return As<Cell>(tree)->GetSecond();
        }
        if (func == "list-ref") {
            if (list.size() != 2) {
                throw RuntimeError("Runtime error!");
            }
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (!Is<Cell>(list[0]) || !Is<Number>(list[1])) {
                throw RuntimeError("Runtime error!");
            }
            std::shared_ptr<Cell> t = As<Cell>(list[0]);
            for (int i = 0; i < As<Number>(list[1])->GetValue(); ++i) {
                std::shared_ptr<Object> nxt = t->GetSecond();
                if (nxt == nullptr || !Is<Cell>(nxt)) {
                    throw RuntimeError("Runtime error!");
                }
                t = As<Cell>(nxt);
            }
            return t->GetFirst();
        }
        if (func == "list-tail") {
            if (list.size() != 2) {
                throw RuntimeError("Runtime error!");
            }
            EvaluteAllHelper(list.begin(), list.end(), vars);
            if (!Is<Cell>(list[0]) || !Is<Number>(list[1])) {
                throw RuntimeError("Runtime error!");
            }
            std::shared_ptr<Cell> t = As<Cell>(list[0]);
            for (int i = 0; i + 1 < As<Number>(list[1])->GetValue(); ++i) {
                std::shared_ptr<Object> nxt = t->GetSecond();
                if (nxt == nullptr || !Is<Cell>(nxt)) {
                    throw RuntimeError("Runtime error!");
                }
                t = As<Cell>(nxt);
            }
            return t->GetSecond();
        }
        if (func == "if") {
            if (list.size() != 2 && list.size() != 3) {
                throw SyntaxError("Syntax error!");
            }
            if (ToBooleanHelper(Evaluate(list[0], vars))) {
                return Evaluate(list[1], vars);
            } else if (list.size() == 3) {
                return Evaluate(list[2], vars);
            } else {
                return nullptr;
            }
        }
        if (func == "define") {
            if (list.size() < 2) {
                throw SyntaxError("Syntax error!");
            }
            if (Is<Cell>(list[0])) {
                std::vector<std::shared_ptr<Object>> list2;
                ToVectorHelper(list2, As<Cell>(list[0]));
                if (list2.empty() || !Is<Symbol>(list2[0])) {
                    throw SyntaxError("Syntax error!");
                }
                std::string fn_name = As<Symbol>(list2[0])->GetName();
                list2 = std::vector<std::shared_ptr<Object>>(list2.begin() + 1, list2.end());

                std::vector<std::string> lambda_vars(list2.size());
                for (size_t i = 0; i < lambda_vars.size(); ++i) {
                    if (!Is<Symbol>(list2[i])) {
                        throw RuntimeError("Runtime error!");
                    }
                    lambda_vars[i] = As<Symbol>(list2[i])->GetName();
                }
                std::shared_ptr<Lambda> lmbd = std::make_shared<Lambda>(
                    lambda_vars, std::vector<std::shared_ptr<Object>>(list.begin() + 1, list.end()),
                    vars);
                vars[fn_name] = std::make_shared<std::shared_ptr<Object>>(lmbd);

                return nullptr;
            }
            if (list.size() != 2) {
                throw SyntaxError("Syntax error!");
            }
            if (!Is<Symbol>(list[0])) {
                throw RuntimeError("Runtime error!");
            }
            if (Is<Cell>(list[1])) {
                std::shared_ptr<Cell> cell = As<Cell>(list[1]);
                std::vector<std::shared_ptr<Object>> list2;
                ToVectorHelper(list2, cell);
                //                if (Is<Symbol>(list2[0]) && As<Symbol>(list2[0])->GetName() ==
                //                "lambda") {
                //                    list2 = std::vector<std::shared_ptr<Object>>(list2.begin() +
                //                    1, list2.end()); if (list2.size() != 2) {
                //                        throw SyntaxError("Syntax error!");
                //                    }
                //                    if (!Is<Cell>(list2[0])) {
                //                        throw RuntimeError("Runtime error!");
                //                    }
                //                    std::vector<std::shared_ptr<Object>> args_v;
                //                    ToVectorHelper(args_v, list2[0]);
                //                    std::vector<std::string> arguments;
                //                    for (std::shared_ptr<Object>& obj : args_v) {
                //                        if (!Is<Symbol>(obj)) {
                //                            throw RuntimeError("Runtime error!");
                //                        }
                //                        arguments.push_back(As<Symbol>(obj)->GetName());
                //                    }
                //
                //                    std::vector<std::shared_ptr<Object>> func_list;
                //                    for (size_t i = 1; i < list2.size(); ++i) {
                //                        func_list.push_back(As<Cell>(list2[i])->GetFirst());
                //                    }
                //
                //                    vars[As<Symbol>(list[0])->GetName()] =
                //                    std::make_shared<Lambda>(arguments, func_list, vars); return
                //                    nullptr;
                //                }
            }
            std::shared_ptr<Object> val = Evaluate(list[1], vars);
            vars[As<Symbol>(list[0])->GetName()] = std::make_shared<std::shared_ptr<Object>>(val);
            return nullptr;
        }
        if (func == "set!") {
            if (list.size() != 2) {
                throw SyntaxError("Syntax error!");
            }
            if (!Is<Symbol>(list[0])) {
                throw RuntimeError("Runtime error!");
            }
            if (vars.find(As<Symbol>(list[0])->GetName()) == vars.end()) {
                throw NameError("Name error!");
            }
            std::shared_ptr<Object> val = Evaluate(list[1], vars);
            //            std::shared_ptr<Object>* pp = new std::shared_ptr<Object>(val);
            //            std::shared_ptr<Object>* pp2 = new std::shared_ptr<Object>(val);

            //            Object* ptr = val.get();

            (*vars[As<Symbol>(list[0])->GetName()]) = val;
            return nullptr;
        }
        if (func == "symbol?") {
            if (list.size() != 1) {
                throw SyntaxError("Syntax error!");
            }
            EvaluteAllHelper(list.begin(), list.end(), vars);
            bool b = Is<Symbol>(list[0]);
            return std::make_shared<Symbol>(b ? "#t" : "#f");
        }
        if (func == "lambda") {
            if (list.size() <= 1) {
                throw SyntaxError("Syntax error!");
            }
            if (list[0] != nullptr && !Is<Cell>(list[0])) {
                throw RuntimeError("Runtime error!");
            }
            std::vector<std::shared_ptr<Object>> list2;
            ToVectorHelper(list2, list[0]);
            std::vector<std::string> lambda_vars(list2.size());
            for (size_t i = 0; i < lambda_vars.size(); ++i) {
                if (!Is<Symbol>(list2[i])) {
                    throw RuntimeError("Runtime error!");
                }
                lambda_vars[i] = As<Symbol>(list2[i])->GetName();
            }
            std::shared_ptr<Lambda> lmbd = std::make_shared<Lambda>(
                lambda_vars, std::vector<std::shared_ptr<Object>>(list.begin() + 1, list.end()),
                vars);
            return lmbd;
        }
        if (func == "set-cdr!") {
            if (list.size() != 2) {
                throw SyntaxError("Syntax error!");
            }
            list[0] = Evaluate(list[0], vars);
            list[1] = Evaluate(list[1], vars);
            if (!Is<Cell>(list[0])) {
                throw RuntimeError("Runtime error!");
            }
            (As<Cell>(list[0])->SetSecond(list[1]));
            return nullptr;
        }
        if (func == "set-car!") {
            if (list.size() != 2) {
                throw SyntaxError("Syntax error!");
            }
            list[0] = Evaluate(list[0], vars);
            list[1] = Evaluate(list[1], vars);
            if (!Is<Cell>(list[0])) {
                throw RuntimeError("Runtime error!");
            }
            (As<Cell>(list[0])->SetFirst(list[1]));
            return nullptr;
        }
        if (vars.find(func) != vars.end()) {
            if (Is<Lambda>(*vars.at(func))) {
                EvaluteAllHelper(list.begin(), list.end(), vars);
                std::shared_ptr<Lambda> lmbd = As<Lambda>(*vars.at(func));
                if (lmbd.get()->GetVars().size() != list.size()) {
                    throw SyntaxError("Syntax error!");
                }
                std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>> new_vars =
                    lmbd->GetLocalVars();
                for (size_t i = 0; i < lmbd.get()->GetVars().size(); ++i) {
                    new_vars[lmbd.get()->GetVars()[i]] =
                        std::make_shared<std::shared_ptr<Object>>(list[i]);
                }
                for (auto pr : vars) {
                    new_vars.insert(pr);
                }
                for (size_t i = 0; i + 1 < lmbd->GetFunc().size(); ++i) {
                    Evaluate(lmbd->GetFunc()[i], new_vars);
                }
                return Evaluate(lmbd->GetFunc().back(), new_vars);
            } else {
                return *vars[func];
            }
        }
        throw NameError("Name error!");
    }
    throw RuntimeError("Runtime error!");
}

std::string GetString(std::shared_ptr<Object> tree) {
    if (!tree) {
        return "()";
    }
    if (Is<Number>(tree)) {
        return std::to_string(As<Number>(tree).get()->GetValue());
    }
    if (Is<Symbol>(tree)) {
        std::shared_ptr<Symbol> symbol = As<Symbol>(tree);
        std::string name = symbol.get()->GetName();
        return name;
    }
    if (Is<Cell>(tree)) {
        bool tr = IsTrueList(tree);
        std::vector<std::shared_ptr<Object>> list;
        ToVectorHelper(list, tree);
        std::string ans = "(";
        for (size_t i = 0; i < list.size(); ++i) {
            if (i + 1 == list.size() && !tr) {
                ans += ". ";
            }
            ans += GetString(list[i]) + (i + 1 != list.size() ? " " : ")");
        }
        return ans;
    }
    throw RuntimeError("Runtime error!");
}

std::string Interpreter::Run(const std::string& s) {
    std::stringstream iss(s);
    Tokenizer tokenizer(&iss);
    std::shared_ptr<Object> tree = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Syntax error!");
    }
    std::shared_ptr<Object> evaluated = Evaluate(tree, vars_);
    return GetString(evaluated);
}
