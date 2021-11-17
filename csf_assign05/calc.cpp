#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "calc.h"
// #include <stdio>
 
struct Calc {
private:
    // fields
    std::map<std::string, int> vars;

public:
    // public member functions
    Calc();
    ~Calc();

    int evalExpr(const std::string &expr, int &result);

private:
    // private member functions
    std::vector<std::string> tokenize(const std::string &expr);
    int token_type(std::vector<std::string> &tokens);
    int var_is_valid(std::string &var);
    int str_is_num(std::string &str);
    int get_val(std::string &token);

    // void clear();
};

Calc::Calc() {
    return;
}

Calc::~Calc() {
    return;
}

int Calc::evalExpr(const std::string &expr, int &result) {
    std::vector<std::string> tokens = tokenize(expr);
    int type = token_type(tokens);

    if (!type) return 0;
        // std::cout << "hey there" << std::endl;


    if (type == 1) result = get_val(tokens.at(0));

    else if (type == 2) {
        std::string op = tokens.at(1);
        if (op == "+") {
            result = get_val(tokens.at(0)) + get_val(tokens.at(2));
        }
        else if (op == "-") result = get_val(tokens.at(0)) - get_val(tokens.at(2));
        else if (op == "*") result = get_val(tokens.at(0)) * get_val(tokens.at(2));
        else if (op == "/") {
            if (get_val(tokens.at(2)) == 0) return 0;
            result = get_val(tokens.at(0)) / get_val(tokens.at(2));
        }
    }

    else if (type == 3) {
        result = get_val(tokens.at(2));
        vars[tokens.at(0)] = result;
    }

    else if (type == 4) {
        std::string op = tokens.at(3);
        if (op == "+") result = get_val(tokens.at(2)) + get_val(tokens.at(4));
        else if (op == "-") result = get_val(tokens.at(2)) - get_val(tokens.at(4));
        else if (op == "*") result = get_val(tokens.at(2)) * get_val(tokens.at(4));
        else if (op == "/") {
            if (get_val(tokens.at(4)) == 0) return 0;
            result = get_val(tokens.at(2)) / get_val(tokens.at(4));
        }
        vars[tokens.at(0)] = result;
    }
    return 1;
}


extern "C" struct Calc *calc_create(void) {
    return new Calc();
}

extern "C" void calc_destroy(struct Calc *calc) {
    delete calc;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
    return calc->evalExpr(expr, *result);
}

std::vector<std::string> Calc::tokenize(const std::string &expr) {
    std::vector<std::string> vec;
    std::stringstream s(expr);

    std::string tok;
    while (s >> tok) {
        vec.push_back(tok);
    }

    return vec;
}

int Calc::token_type(std::vector<std::string> &tokens) {
    // case 0: invalid
    // case 1: operand
    // case 2: operand op operand
    // case 3: var = operand
    // case 4: var = operand op operand

    if (tokens.size() != 1 && tokens.size() != 3 && tokens.size() != 5) return 0;
    if (str_is_num(tokens.at(0))) {
        if (tokens.size() == 1) return 1; 
        if (tokens.size() == 5) return 0;
        std::string op = tokens.at(1);
        if (op != "+" && op != "-" && op != "*" && op != "/") return 0;
        if(str_is_num(tokens.at(2)) || vars.find(tokens.at(2)) != vars.end()) return 2;
        return 0;
    }

    if (!var_is_valid(tokens.at(0))) return 0;
    if (tokens.size() == 1) return vars.find(tokens.at(0)) != vars.end();
    std::string op = tokens.at(1);

    if (op != "=") {
        if (tokens.size() == 3 && (op == "+" || op == "-" || op == "*" || op == "/")) return 2;
        return 0;
    }

    if (!str_is_num(tokens.at(2)) && (vars.find(tokens.at(2)) == vars.end())) return 0;
    if (tokens.size() == 3) return 3;

    op = tokens.at(3);
    if (op != "+" && op != "-" && op != "*" && op != "/") return 0;

    if (!str_is_num(tokens.at(4)) && (vars.find(tokens.at(2)) == vars.end())) return 0;
    return 4;
}

int Calc::var_is_valid(std::string &var) {
    std::string::iterator it;
    for (it = var.begin(); it != var.end(); it++) {
        if (!isalpha(*it)) return 0;
    }
    return 1;
}

int Calc::str_is_num(std::string &str) {
    std::stringstream stream(str);
    int num;
    stream >> num;
    return !stream.fail();
}

int Calc::get_val(std::string &token) {
    int val;
    if (!str_is_num(token)) val = vars[token];
    else {
        std::stringstream str(token);
        str >> val;
    }
    return val;
}
