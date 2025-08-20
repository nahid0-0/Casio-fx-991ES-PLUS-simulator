#pragma once
#include <string>
#include <vector>

// Forward declarations
struct Token;
struct ComplexNumber;

// Main evaluation function - receives postfix tokens and returns result
std::string evaluatePostfixExpression(const std::vector<Token>& postfixTokens);

// Helper functions for complex number operations
ComplexNumber addComplex(const ComplexNumber& a, const ComplexNumber& b);
ComplexNumber subtractComplex(const ComplexNumber& a, const ComplexNumber& b);
ComplexNumber multiplyComplex(const ComplexNumber& a, const ComplexNumber& b);
ComplexNumber divideComplex(const ComplexNumber& a, const ComplexNumber& b);

// Mathematical functions
ComplexNumber applyFunction(const std::string& functionName, const ComplexNumber& operand);
ComplexNumber parseVariable(const std::string& variableName);
