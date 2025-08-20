#pragma once
#include <string>

double calc(double a, char op, double b);

// String-based arithmetic functions
std::string add(const std::string& operand1, const std::string& operand2);
std::string subtract(const std::string& operand1, const std::string& operand2);
std::string multiply(const std::string& operand1, const std::string& operand2);
std::string divide(const std::string& operand1, const std::string& operand2);
std::string power(const std::string& base, const std::string& exponent);
std::string powerDecimal(const std::string& base, const std::string& exponent);
std::string nthRoot(const std::string& number, const std::string& root);

// Generic operation function
std::string operate(const std::string& operand1, char op, const std::string& operand2);

