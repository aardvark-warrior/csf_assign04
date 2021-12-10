/** CSF Assignment 6 Calculator Implementation 
 * 
 * Arthur Wang - awang91
 * Mason Albert - malber20
 */

#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "calc.h"
 
struct Calc {
private:
    // fields
    std::map<std::string, int> vars;
    pthread_mutex_t lock;

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
    int operand(std::vector<std::string> &tokens, int &result);
    int operand_op_operand(std::vector<std::string> &tokens, int &result);
    int var_eq_operand(std::vector<std::string> &tokens, int &result);
    int var_eq_operand_op_operand(std::vector<std::string> &tokens, int &result);
    int is_operand(std::string &str);
};

/**
 * Default constructor. 
 */
Calc::Calc() {
    pthread_mutex_init(&lock, NULL);
}

/**
 * Default destructor. 
 */
Calc::~Calc() {
    pthread_mutex_destroy(&lock);
}

/**
 * Function to evaluate expression.
 * @param expr expression to be evaluated as a string
 * @param result variable to store result of evaluation
 * @return 1 if valid expresion, else 0
 */ 
int Calc::evalExpr(const std::string &expr, int &result) {
    pthread_mutex_lock(&lock);
    std::vector<std::string> tokens = tokenize(expr);
    int type = token_type(tokens);
    int return_code;
    if (!type) return_code = 0; // invalid expr
    else if (type == 1) return_code = operand(tokens, result); // operand
    else if (type == 2) return_code = operand_op_operand(tokens, result); // operand op operand
    else if (type == 3) return_code = var_eq_operand(tokens, result); // var = operand 
    else return_code = var_eq_operand_op_operand(tokens, result); // var = operand op operand

    pthread_mutex_unlock(&lock);
    return return_code;
}

/** 
 * Evaluate expression of format: operand.
 * @param tokens vector of tokens for expression
 * @param result variable to store result
 * @return 1 if valid evaluation, else 0
 */
int Calc::operand(std::vector<std::string> &tokens, int &result) {
    result = get_val(tokens.at(0)); // get value of int/variable
    return 1;
}

/**
 * Determine if a string is a valid operand.
 * @param str string operand
 * @return 1 if valid operand, else 0
 */
int Calc::is_operand(std::string &str) {
    return str_is_num(str) || vars.find(str) != vars.end();
}

/** 
 * Evaluate expression of format: operand op operand.
 * @param tokens vector of tokens for expression
 * @param result variable to store result
 * @return 1 if valid evaluation, else 0
 */
int Calc::operand_op_operand(std::vector<std::string> &tokens, int &result) {
    std::string op = tokens.at(1);
    if (!str_is_num(tokens.at(0)) && vars.find(tokens.at(0)) == vars.end()) return 0;
    else if ((!str_is_num(tokens.at(2)) && vars.find(tokens.at(2)) == vars.end())) return 0;

    // compute result
    if (op == "+") result = get_val(tokens.at(0)) + get_val(tokens.at(2));
    else if (op == "-") result = get_val(tokens.at(0)) - get_val(tokens.at(2));
    else if (op == "*") result = get_val(tokens.at(0)) * get_val(tokens.at(2));
    else if (op == "/") {
        if (get_val(tokens.at(2)) == 0) return 0; // invalid if division by 0
        result = get_val(tokens.at(0)) / get_val(tokens.at(2));
    }
    return 1; // otherwise valid
}

/** 
 * Evaluate expression of format: var = operand.
 * @param tokens vector of tokens for expression
 * @param result variable to store result
 * @return 1 if valid evaluation, else 0
 */
int Calc::var_eq_operand(std::vector<std::string> &tokens, int &result) {
    // compute result and assign var
    result = get_val(tokens.at(2));
    vars[tokens.at(0)] = result;
    return 1;
}

/** 
 * Evaluate expression of format: var = operand op operand.
 * @param tokens vector of tokens for expression
 * @param result variable to store result
 * @return 1 if valid evaluation, else 0
 */
int Calc::var_eq_operand_op_operand(std::vector<std::string> &tokens, int &result) {
    std::string op = tokens.at(3);

    // compute result
    if (op == "+") result = get_val(tokens.at(2)) + get_val(tokens.at(4));
    else if (op == "-") result = get_val(tokens.at(2)) - get_val(tokens.at(4));
    else if (op == "*") result = get_val(tokens.at(2)) * get_val(tokens.at(4));
    else if (op == "/") {
        if (get_val(tokens.at(4)) == 0) return 0; // invalid if division by 0
        result = get_val(tokens.at(2)) / get_val(tokens.at(4));
    }
    vars[tokens.at(0)] = result; // insert var
    return 1; // otherwise valid
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

/**
 * Helper function to convert expression into vector of relevant tokens. 
 * @param expr string expression input.
 */
std::vector<std::string> Calc::tokenize(const std::string &expr) {
    std::vector<std::string> vec;
    std::stringstream s(expr);

    std::string tok;
    while (s >> tok) {
        vec.push_back(tok);
    }

    return vec;
}

/**
 * Helper function to determine the validity and form of a given vector of tokens. 
 * @param expr string expression input.
 * @return format code: 
 *  case 0: invalid 
 *  case 1: operand 
 *  case 2: operand op operand
 *  case 3: var = operand 
 *  case 4: var = operand op operand
 */
int Calc::token_type(std::vector<std::string> &tokens) {


    if (tokens.size() == 1) {
        if (is_operand(tokens.at(0))) {
            return 1;
        }
        return 0;
    } else if (tokens.size() == 3) {
        std::string op = tokens.at(1);
        if (var_is_valid(tokens.at(0)) && op == "=" && is_operand(tokens.at(2))) {
            return 3;
        } else if (is_operand(tokens.at(0)) && is_operand(tokens.at(2)) && 
            (op == "+" || op == "-" || op == "/" || op == "*")) {
            return 2;
        } else return 0;
    } else if (tokens.size() == 5) {
        std::string op = tokens.at(3);
        if (var_is_valid(tokens.at(0)) && is_operand(tokens.at(2)) && is_operand(tokens.at(4)) &&
            tokens.at(1) == "=" && (op == "+" || op == "-" || op == "/" || op == "*")) {
                return 4;
            }
    } else return 0;
    return 0;
}

/**
 * Helper function to determine if variable name is valid.
 * @param var variable name as a string
 * @return 1 if valid, else 0
 */
int Calc::var_is_valid(std::string &var) {
    std::string::iterator it;
    for (it = var.begin(); it != var.end(); it++) {
        if (!isalpha(*it)) return 0;
    }
    return 1;
}

/**
 * Helper function to determine if a string is also an integer.
 * @param str string to be checked
 * @return 1 if int, else 0
 */ 
int Calc::str_is_num(std::string &str) {
    std::stringstream stream(str);
    int num;
    stream >> num;
    return !stream.fail();
}

/**
 * Helper function to get value of a valid token.
 * @param token variable or integer as string to be converted
 * @return value of variable, or string converted to integer
 */
int Calc::get_val(std::string &token) {
    int val;
    if (!str_is_num(token)) val = vars[token];
    else {
        std::stringstream str(token);
        str >> val;
    }
    return val;
}
