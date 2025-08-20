#include "parsing.h"
#include "evaluator.h"
#include <string>
#include <iostream>
#include <stack>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <map>
#include <android/log.h>

#define LOG_TAG "CalculatorParser"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;

// Complex number structure
struct ComplexNumber {
    string real;
    string imaginary;
    
    ComplexNumber() : real("0"), imaginary("not initiated") {}
    ComplexNumber(const string& r) : real(r), imaginary("not initiated") {}
    ComplexNumber(const string& r, const string& i) : real(r), imaginary(i) {}
    
    bool isReal() const {
        return imaginary == "not initiated" || imaginary == "0";
    }
    
    string toString() const {
        if (isReal()) return real;
        if (real == "0") return imaginary + "i";
        if (imaginary == "0") return real;
        
        string sign = (imaginary[0] == '-') ? "" : "+";
        return real + sign + imaginary + "i";
    }
};

// Token types
enum TokenType {
    NUMBER, OPERATOR, FUNCTION, LEFT_PAREN, RIGHT_PAREN, VARIABLE
};

// Token structure
struct Token {
    TokenType type;
    string value;
    int precedence;
    bool rightAssociative;
    
    Token(TokenType t, const string& v, int p = 0, bool ra = false) 
        : type(t), value(v), precedence(p), rightAssociative(ra) {}
};

// Operator precedence map
map<string, pair<int, bool>> operatorMap = {
    {"+", {1, false}}, {"-", {1, false}},
    {"*", {2, false}}, {"/", {2, false}}, {"%", {2, false}},
    {"^", {4, true}},  {"**", {4, true}}  // Power has highest precedence and is right-associative
};

// Function map
map<string, int> functionMap = {
    {"sin", 1}, {"cos", 1}, {"tan", 1},
    {"asin", 1}, {"acos", 1}, {"atan", 1},
    {"sinh", 1}, {"cosh", 1}, {"tanh", 1},
    {"log", 1}, {"ln", 1}, {"log10", 1},
    {"sqrt", 1}, {"abs", 1}, {"inv", 1},
    {"exp", 1}, {"floor", 1}, {"ceil", 1}
};

// Helper function to check if character is alphanumeric
bool isAlphaNum(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

// Tokenize the input expression
vector<Token> tokenize(const string& expression) {
    vector<Token> tokens;
    string current = "";
    
    for (int i = 0; i < expression.length(); i++) {
        char c = expression[i];
        
        if (isspace(c)) {
            continue; // Skip whitespace
        }
        
        // Numbers (including decimals and scientific notation)
        if (isdigit(c) || c == '.') {
            current = "";
            while (i < expression.length() && 
                   (isdigit(expression[i]) || expression[i] == '.' || 
                    expression[i] == 'e' || expression[i] == 'E' ||
                    (expression[i] == '-' && i > 0 && (expression[i-1] == 'e' || expression[i-1] == 'E')))) {
                current += expression[i];
                i++;
            }
            i--; // Back up one since the loop will increment
            tokens.push_back(Token(NUMBER, current));
        }
        // Variables and functions
        else if (isalpha(c)) {
            current = "";
            while (i < expression.length() && isAlphaNum(expression[i])) {
                current += expression[i];
                i++;
            }
            i--; // Back up one
            
            if (functionMap.find(current) != functionMap.end()) {
                tokens.push_back(Token(FUNCTION, current));
            } else if (current == "i" || current == "j") {
                tokens.push_back(Token(VARIABLE, current)); // Imaginary unit
            } else if (current == "pi" || current == "e") {
                tokens.push_back(Token(VARIABLE, current)); // Constants
            } else {
                tokens.push_back(Token(VARIABLE, current)); // Other variables
            }
        }
        // Operators
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            string op = "";
            op += c;
            
            // Check for ** (power operator)
            if (c == '*' && i + 1 < expression.length() && expression[i + 1] == '*') {
                op = "**";
                i++;
            }
            
            if (operatorMap.find(op) != operatorMap.end()) {
                auto opInfo = operatorMap[op];
                tokens.push_back(Token(OPERATOR, op, opInfo.first, opInfo.second));
            }
        }
        // Parentheses
        else if (c == '(') {
            tokens.push_back(Token(LEFT_PAREN, "("));
        }
        else if (c == ')') {
            tokens.push_back(Token(RIGHT_PAREN, ")"));
        }
    }
    
    return tokens;
}

// Convert infix to postfix using Shunting Yard algorithm
vector<Token> shuntingYard(const vector<Token>& tokens) {
    vector<Token> output;
    stack<Token> operators;
    
    for (const Token& token : tokens) {
        switch (token.type) {
            case NUMBER:
            case VARIABLE:
                output.push_back(token);
                break;
                
            case FUNCTION:
                operators.push(token);
                break;
                
            case OPERATOR:
                while (!operators.empty() && 
                       operators.top().type != LEFT_PAREN &&
                       ((operators.top().type == FUNCTION) ||
                        (operators.top().precedence > token.precedence) ||
                        (operators.top().precedence == token.precedence && !token.rightAssociative))) {
                    output.push_back(operators.top());
                    operators.pop();
                }
                operators.push(token);
                break;
                
            case LEFT_PAREN:
                operators.push(token);
                break;
                
            case RIGHT_PAREN:
                while (!operators.empty() && operators.top().type != LEFT_PAREN) {
                    output.push_back(operators.top());
                    operators.pop();
                }
                if (!operators.empty() && operators.top().type == LEFT_PAREN) {
                    operators.pop(); // Remove the left parenthesis
                }
                // If there's a function on top, pop it too
                if (!operators.empty() && operators.top().type == FUNCTION) {
                    output.push_back(operators.top());
                    operators.pop();
                }
                break;
        }
    }
    
    // Pop remaining operators
    while (!operators.empty()) {
        output.push_back(operators.top());
        operators.pop();
    }
    
    return output;
}

// Evaluation is now handled by evaluator.cpp

std::string parseExpression(const std::string& expression) {
    try {
        LOGD("C++ received expression: %s", expression.c_str());
        
        // Tokenize the expression
        vector<Token> tokens = tokenize(expression);
        LOGD("Tokenization complete: %d tokens", (int)tokens.size());
        
        // Convert to postfix using Shunting Yard
        vector<Token> postfix = shuntingYard(tokens);
        LOGD("Shunting Yard complete: %d postfix tokens", (int)postfix.size());
        
        // Send postfix tokens to evaluator for computation
        string result = evaluatePostfixExpression(postfix);
        
        string output = "Result: " + result;
        LOGD("C++ final output: %s", output.c_str());
        
        return output;
        
    } catch (const std::exception& e) {
        LOGE("Parsing error: %s", e.what());
        return "Error: " + string(e.what());
    }
}
