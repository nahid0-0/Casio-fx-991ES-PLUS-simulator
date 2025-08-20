#include "evaluator.h"
#include "calc.h"
#include <string>
#include <stack>
#include <vector>
#include <stdexcept>
#include <android/log.h>

#define LOG_TAG "CalculatorEvaluator"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;

// Token types (duplicated from parsing.cpp - could be moved to common header)
enum TokenType {
    NUMBER, OPERATOR, FUNCTION, LEFT_PAREN, RIGHT_PAREN, VARIABLE
};

// Token structure (duplicated from parsing.cpp)
struct Token {
    TokenType type;
    string value;
    int precedence;
    bool rightAssociative;
    
    Token(TokenType t, const string& v, int p = 0, bool ra = false) 
        : type(t), value(v), precedence(p), rightAssociative(ra) {}
};

// Memory-optimized Complex number structure
struct ComplexNumber {
    string real;
    string imaginary;
    
    // Default constructor
    ComplexNumber() : real("0"), imaginary("not initiated") {}
    
    // Single string constructor (real number)
    ComplexNumber(const string& r) : real(r), imaginary("not initiated") {}
    
    // Two string constructor (complex number)
    ComplexNumber(const string& r, const string& i) : real(r), imaginary(i) {}
    
    // Move constructors for efficiency
    ComplexNumber(string&& r) : real(std::move(r)), imaginary("not initiated") {}
    ComplexNumber(string&& r, string&& i) : real(std::move(r)), imaginary(std::move(i)) {}
    
    // Copy constructor (explicit)
    ComplexNumber(const ComplexNumber& other) : real(other.real), imaginary(other.imaginary) {}
    
    // Move constructor
    ComplexNumber(ComplexNumber&& other) noexcept 
        : real(std::move(other.real)), imaginary(std::move(other.imaginary)) {}
    
    // Assignment operators
    ComplexNumber& operator=(const ComplexNumber& other) {
        if (this != &other) {
            real = other.real;
            imaginary = other.imaginary;
        }
        return *this;
    }
    
    ComplexNumber& operator=(ComplexNumber&& other) noexcept {
        if (this != &other) {
            real = std::move(other.real);
            imaginary = std::move(other.imaginary);
        }
        return *this;
    }
    
    bool isReal() const {
        return imaginary == "not initiated" || imaginary == "0";
    }
    
    string toString() const {
        if (isReal()) return real;
        if (real == "0" && imaginary == "1") return "i";
        if (real == "0" && imaginary == "-1") return "-i";
        if (real == "0") return imaginary + "i";
        if (imaginary == "0") return real;
        
        string sign = (imaginary[0] == '-') ? "" : "+";
        if (imaginary == "1") {
            return real + "+i";
        } else if (imaginary == "-1") {
            return real + "-i";
        } else {
            return real + sign + imaginary + "i";
        }
    }
};

// Memory-optimized complex arithmetic functions using move semantics
ComplexNumber addComplex(const ComplexNumber& a, const ComplexNumber& b) {
    string realPart = add(a.real, b.real);
    string imagPart = add(a.isReal() ? "0" : a.imaginary, 
                         b.isReal() ? "0" : b.imaginary);
    
    if (imagPart == "0") {
        return ComplexNumber(std::move(realPart));
    }
    return ComplexNumber(std::move(realPart), std::move(imagPart));
}

ComplexNumber subtractComplex(const ComplexNumber& a, const ComplexNumber& b) {
    string realPart = subtract(a.real, b.real);
    string imagPart = subtract(a.isReal() ? "0" : a.imaginary,
                              b.isReal() ? "0" : b.imaginary);
    
    if (imagPart == "0") {
        return ComplexNumber(std::move(realPart));
    }
    return ComplexNumber(std::move(realPart), std::move(imagPart));
}

ComplexNumber multiplyComplex(const ComplexNumber& a, const ComplexNumber& b) {
    // Use const references to avoid string copying
    const string& ar = a.real;
    const string& ai = a.isReal() ? static_cast<const string&>("0") : a.imaginary;
    const string& br = b.real;
    const string& bi = b.isReal() ? static_cast<const string&>("0") : b.imaginary;
    
    string realPart = subtract(multiply(ar, br), multiply(ai, bi));
    string imagPart = add(multiply(ar, bi), multiply(ai, br));
    
    if (imagPart == "0") {
        return ComplexNumber(std::move(realPart));
    }
    return ComplexNumber(std::move(realPart), std::move(imagPart));
}

ComplexNumber divideComplex(const ComplexNumber& a, const ComplexNumber& b) {
    // Use const references to avoid copying
    const string& ar = a.real;
    const string& ai = a.isReal() ? static_cast<const string&>("0") : a.imaginary;
    const string& br = b.real;
    const string& bi = b.isReal() ? static_cast<const string&>("0") : b.imaginary;
    
    string denominator = add(multiply(br, br), multiply(bi, bi));
    
    if (denominator == "0") {
        throw domain_error("Division by zero");
    }
    
    string realNum = add(multiply(ar, br), multiply(ai, bi));
    string imagNum = subtract(multiply(ai, br), multiply(ar, bi));
    
    string realPart = divide(realNum, denominator);
    string imagPart = divide(imagNum, denominator);
    
    if (imagPart == "0") {
        return ComplexNumber(std::move(realPart));
    }
    return ComplexNumber(std::move(realPart), std::move(imagPart));
}

// Parse variable names to their values
ComplexNumber parseVariable(const string& variableName) {
    if (variableName == "i" || variableName == "j") {
        return ComplexNumber("0", "1");
    } else if (variableName == "pi") {
        return ComplexNumber("3.141592653589793238462643383279502884197169399375105820974944592307816406286");
    } else if (variableName == "e") {
        return ComplexNumber("2.718281828459045235360287471352662497757247093699959574966967627724076630353");
    } else {
        LOGE("Unknown variable: %s", variableName.c_str());
        return ComplexNumber("0"); // Default to 0 for unknown variables
    }
}

// Apply mathematical functions
ComplexNumber applyFunction(const string& functionName, const ComplexNumber& operand) {
    LOGD("Applying function: %s to %s", functionName.c_str(), operand.toString().c_str());
    
    if (functionName == "inv") {
        // 1/x
        if (operand.isReal()) {
            if (operand.real == "0") {
                throw domain_error("Cannot take inverse of zero");
            }
            return ComplexNumber(divide("1", operand.real));
        } else {
            // 1/(a+bi) = (a-bi)/(a²+b²)
            string a = operand.real;
            string b = operand.imaginary;
            string denom = add(multiply(a, a), multiply(b, b));
            
            if (denom == "0") {
                throw domain_error("Cannot take inverse of zero");
            }
            
            string realPart = divide(a, denom);
            string imagPart = divide(multiply("-1", b), denom);
            return ComplexNumber(realPart, imagPart);
        }
    } else if (functionName == "abs") {
        // |a+bi| = sqrt(a²+b²)
        if (operand.isReal()) {
            // For real numbers, return absolute value
            if (operand.real[0] == '-') {
                return ComplexNumber(operand.real.substr(1));
            }
            return operand;
        } else {
            // For complex numbers, return magnitude
            string a = operand.real;
            string b = operand.imaginary;
            string magnitude_squared = add(multiply(a, a), multiply(b, b));
            // Note: For true sqrt, would need to implement square root algorithm
            return ComplexNumber("sqrt(" + magnitude_squared + ")");
        }
    } else if (functionName == "sqrt") {
        if (operand.isReal()) {
            // Simple placeholder - real implementation would use Newton's method
            return ComplexNumber("sqrt(" + operand.real + ")");
        } else {
            // Complex square root is more involved
            return ComplexNumber("complex_sqrt(" + operand.toString() + ")");
        }
    } else if (functionName == "ln") {
        if (operand.isReal()) {
            return ComplexNumber("ln(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_ln(" + operand.toString() + ")");
        }
    } else if (functionName == "log" || functionName == "log10") {
        if (operand.isReal()) {
            return ComplexNumber("log10(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_log10(" + operand.toString() + ")");
        }
    } else if (functionName == "sin" || functionName == "cos" || functionName == "tan") {
        if (operand.isReal()) {
            return ComplexNumber(functionName + "(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_" + functionName + "(" + operand.toString() + ")");
        }
    } else if (functionName == "asin" || functionName == "acos" || functionName == "atan") {
        if (operand.isReal()) {
            return ComplexNumber(functionName + "(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_" + functionName + "(" + operand.toString() + ")");
        }
    } else if (functionName == "sinh" || functionName == "cosh" || functionName == "tanh") {
        if (operand.isReal()) {
            return ComplexNumber(functionName + "(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_" + functionName + "(" + operand.toString() + ")");
        }
    } else if (functionName == "exp") {
        if (operand.isReal()) {
            return ComplexNumber("exp(" + operand.real + ")");
        } else {
            return ComplexNumber("complex_exp(" + operand.toString() + ")");
        }
    } else if (functionName == "floor" || functionName == "ceil") {
        if (operand.isReal()) {
            return ComplexNumber(functionName + "(" + operand.real + ")");
        } else {
            LOGE("Function %s not defined for complex numbers", functionName.c_str());
            throw domain_error(functionName + " not defined for complex numbers");
        }
    } else {
        LOGE("Unknown function: %s", functionName.c_str());
        throw invalid_argument("Unknown function: " + functionName);
    }
}

// Main evaluation function - now uses references to avoid copying
string evaluatePostfixExpression(const vector<Token>& postfixTokens) {
    try {
        LOGD("Starting evaluation of postfix expression with %d tokens", (int)postfixTokens.size());
        
        stack<ComplexNumber> evalStack;
        
        for (const Token& token : postfixTokens) {
            LOGD("Processing token: type=%d, value=%s", token.type, token.value.c_str());
            
            switch (token.type) {
                case NUMBER:
                    evalStack.emplace(token.value); // Use emplace instead of push to avoid copy
                    LOGD("Pushed number: %s", token.value.c_str());
                    break;
                    
                case VARIABLE:
                    {
                        evalStack.emplace(parseVariable(token.value)); // Direct emplace
                        LOGD("Pushed variable %s", token.value.c_str());
                    }
                    break;
                    
                case OPERATOR:
                    if (evalStack.size() < 2) {
                        throw invalid_argument("Invalid expression: not enough operands for operator " + token.value);
                    }
                    
                    {
                        ComplexNumber b = std::move(evalStack.top()); evalStack.pop(); // Use move semantics
                        ComplexNumber a = std::move(evalStack.top()); evalStack.pop();
                        
                        LOGD("Applying operator %s", token.value.c_str());
                        
                        // Use references in operations to avoid copies
                        if (token.value == "+") {
                            evalStack.emplace(addComplex(a, b));
                        } else if (token.value == "-") {
                            evalStack.emplace(subtractComplex(a, b));
                        } else if (token.value == "*") {
                            evalStack.emplace(multiplyComplex(a, b));
                        } else if (token.value == "/" || token.value == "÷") {
                            evalStack.emplace(divideComplex(a, b));
                        } else if (token.value == "^" || token.value == "**") {
                            // Power operation using pen-and-pencil method
                            if (a.isReal() && b.isReal()) {
                                try {
                                    string powerResult = power(a.real, b.real);
                                    evalStack.emplace(powerResult);
                                } catch (const exception& e) {
                                    evalStack.emplace("Power Error: " + string(e.what()));
                                }
                            } else {
                                // Complex power - placeholder for now
                                evalStack.emplace("complex_pow(" + a.toString() + ", " + b.toString() + ")");
                            }
                        } else {
                            throw invalid_argument("Unknown operator: " + token.value);
                        }
                        
                        LOGD("Operator result computed");
                    }
                    break;
                    
                case FUNCTION:
                    if (evalStack.empty()) {
                        throw invalid_argument("Invalid expression: no operand for function " + token.value);
                    }
                    
                    {
                        ComplexNumber operand = std::move(evalStack.top()); evalStack.pop(); // Use move
                        evalStack.emplace(applyFunction(token.value, operand));
                        LOGD("Function %s computed", token.value.c_str());
                    }
                    break;
                    
                default:
                    LOGE("Unexpected token type in postfix expression: %d", token.type);
                    throw invalid_argument("Unexpected token type in postfix expression");
            }
        }
        
        if (evalStack.size() != 1) {
            throw invalid_argument("Invalid expression: final stack size is " + to_string(evalStack.size()) + ", expected 1");
        }
        
        // Return result directly without copying
        return evalStack.top().toString();
        
    } catch (const exception& e) {
        LOGE("Evaluation error: %s", e.what());
        throw; // Re-throw to be caught by parseExpression
    }
}
