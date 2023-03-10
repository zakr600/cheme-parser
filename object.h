#pragma once

#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <typeinfo>
#include <map>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(int number) : number_(number) {
    }

    int GetValue() const {
        return number_;
    }

private:
    int number_;
};

class Symbol : public Object {
public:
    Symbol(const std::string& name) : name_(name) {
    }

    const std::string& GetName() const {
        return name_;
    }

private:
    std::string name_;
};

class Dot : public Object {};

class Cell : public Object {
public:
    Cell() {
    }

    Cell(std::shared_ptr<Object>& first, std::shared_ptr<Object>& second)
        : first_(first), second_(second) {
    }

    std::shared_ptr<Object> GetFirst() const {
        return first_;
    }

    std::shared_ptr<Object> GetSecond() const {
        return second_;
    }

    void SetFirst(std::shared_ptr<Object> first) {
        first_ = first;
    }

    void SetSecond(std::shared_ptr<Object> second) {
        second_ = second;
    }

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

class Lambda : public Object {
public:
    Lambda(const std::vector<std::string>& vars, const std::vector<std::shared_ptr<Object>>& func,
           std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& local_vars)
        : vars_(vars), local_vars_(local_vars), func_(func) {
    }

    const std::vector<std::string>& GetVars() const {
        return vars_;
    }

    const std::vector<std::shared_ptr<Object>>& GetFunc() const {
        return func_;
    }

    const std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>>& GetLocalVars() const {
        return local_vars_;
    }

private:
    std::vector<std::string> vars_;
    std::map<std::string, std::shared_ptr<std::shared_ptr<Object>>> local_vars_;
    std::vector<std::shared_ptr<Object>> func_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return As<T>(obj) != nullptr;
}
