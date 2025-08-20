#include "calc.h"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

// Legacy function for JNI compatibility (uses double precision)
double calc(double a, char op, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b == 0.0) throw std::runtime_error("division by zero");
            return a / b;
        default:
            throw std::invalid_argument("unknown operator");
    }
}

// Helper function to validate number string
bool isValidNumber(const string& str) {
    if (str.empty()) return false;
    
    int start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
    bool hasDecimal = false;
    for (int i = start; i < str.length(); i++) {
        if (str[i] == '.') {
            if (hasDecimal) return false;
            hasDecimal = true;
        } else if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

// Remove leading zeros
string removeLeadingZeros(const string& str) {
    if (str.empty()) return "0";
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    
    while (start < str.length() && str[start] == '0') {
        start++;
    }
    
    if (start == str.length()) return "0";
    
    string result = str.substr(0, (str[0] == '-' || str[0] == '+') ? 1 : 0) + str.substr(start);
    return result.empty() ? "0" : result;
}

// Remove trailing zeros from decimal
string removeTrailingZeros(const string& str) {
    if (str.find('.') == string::npos) return str;
    
    string result = str;
    while (result.length() > 1 && result.back() == '0') {
        result.pop_back();
    }
    if (result.back() == '.') {
        result.pop_back();
    }
    return result;
}

// Compare absolute values (-1: a < b, 0: a == b, 1: a > b)
int compareAbs(const string& a, const string& b) {
    // Split into integer and decimal parts
    size_t dotA = a.find('.');
    size_t dotB = b.find('.');
    
    string intA = (dotA == string::npos) ? a : a.substr(0, dotA);
    string intB = (dotB == string::npos) ? b : b.substr(0, dotB);
    
    string decA = (dotA == string::npos) ? "" : a.substr(dotA + 1);
    string decB = (dotB == string::npos) ? "" : b.substr(dotB + 1);
    
    // Remove signs for comparison
    if (!intA.empty() && (intA[0] == '-' || intA[0] == '+')) intA = intA.substr(1);
    if (!intB.empty() && (intB[0] == '-' || intB[0] == '+')) intB = intB.substr(1);
    
    // Remove leading zeros
    intA = removeLeadingZeros(intA);
    intB = removeLeadingZeros(intB);
    
    // Compare integer parts by length first
    if (intA.length() != intB.length()) {
        return (intA.length() < intB.length()) ? -1 : 1;
    }
    
    // Same length, compare digit by digit
    int intComp = intA.compare(intB);
    if (intComp != 0) return (intComp < 0) ? -1 : 1;
    
    // Integer parts equal, compare decimal parts
    int maxDecLen = max(decA.length(), decB.length());
    decA.resize(maxDecLen, '0');
    decB.resize(maxDecLen, '0');
    
    int decComp = decA.compare(decB);
    return (decComp < 0) ? -1 : (decComp > 0) ? 1 : 0;
}

// Add two positive numbers using pen-and-pencil method
string addPositive(const string& a, const string& b) {
    // Split into integer and decimal parts
    size_t dotA = a.find('.');
    size_t dotB = b.find('.');
    
    string intA = (dotA == string::npos) ? a : a.substr(0, dotA);
    string intB = (dotB == string::npos) ? b : b.substr(0, dotB);
    
    string decA = (dotA == string::npos) ? "" : a.substr(dotA + 1);
    string decB = (dotB == string::npos) ? "" : b.substr(dotB + 1);
    
    // Pad decimal parts to same length
    int maxDecLen = max(decA.length(), decB.length());
    decA.resize(maxDecLen, '0');
    decB.resize(maxDecLen, '0');
    
    string result = "";
    int carry = 0;
    
    // Add decimal part from right to left
    for (int i = maxDecLen - 1; i >= 0; i--) {
        int sum = (decA[i] - '0') + (decB[i] - '0') + carry;
        result = char(sum % 10 + '0') + result;
        carry = sum / 10;
    }
    
    if (maxDecLen > 0) {
        result = "." + result;
    }
    
    // Add integer part from right to left
    int i = intA.length() - 1;
    int j = intB.length() - 1;
    
    string intResult = "";
    while (i >= 0 || j >= 0 || carry > 0) {
        int sum = carry;
        if (i >= 0) sum += (intA[i--] - '0');
        if (j >= 0) sum += (intB[j--] - '0');
        
        intResult = char(sum % 10 + '0') + intResult;
        carry = sum / 10;
    }
    
    result = intResult + result;
    return removeTrailingZeros(removeLeadingZeros(result));
}

// Subtract two positive numbers (assumes a >= b)
string subtractPositive(const string& a, const string& b) {
    // Split into integer and decimal parts
    size_t dotA = a.find('.');
    size_t dotB = b.find('.');
    
    string intA = (dotA == string::npos) ? a : a.substr(0, dotA);
    string intB = (dotB == string::npos) ? b : b.substr(0, dotB);
    
    string decA = (dotA == string::npos) ? "" : a.substr(dotA + 1);
    string decB = (dotB == string::npos) ? "" : b.substr(dotB + 1);
    
    // Pad decimal parts
    int maxDecLen = max(decA.length(), decB.length());
    decA.resize(maxDecLen, '0');
    decB.resize(maxDecLen, '0');
    
    // Pad integer parts
    int maxIntLen = max(intA.length(), intB.length());
    intA = string(maxIntLen - intA.length(), '0') + intA;
    intB = string(maxIntLen - intB.length(), '0') + intB;
    
    string fullA = intA + decA;
    string fullB = intB + decB;
    
    string result = "";
    int borrow = 0;
    
    // Subtract from right to left
    for (int i = fullA.length() - 1; i >= 0; i--) {
        int diff = (fullA[i] - '0') - (fullB[i] - '0') - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result = char(diff + '0') + result;
    }
    
    // Insert decimal point
    if (maxDecLen > 0) {
        result = result.substr(0, result.length() - maxDecLen) + "." + result.substr(result.length() - maxDecLen);
    }
    
    return removeTrailingZeros(removeLeadingZeros(result));
}

// Multiply using pen-and-pencil long multiplication
string multiplyPositive(const string& a, const string& b) {
    if (a == "0" || b == "0") return "0";
    
    // Remove decimal points and count decimal places
    string numA = a, numB = b;
    int decPlacesA = 0, decPlacesB = 0;
    
    size_t dotA = numA.find('.');
    if (dotA != string::npos) {
        decPlacesA = numA.length() - dotA - 1;
        numA.erase(dotA, 1);
    }
    
    size_t dotB = numB.find('.');
    if (dotB != string::npos) {
        decPlacesB = numB.length() - dotB - 1;
        numB.erase(dotB, 1);
    }
    
    // Remove leading zeros
    numA = removeLeadingZeros(numA);
    numB = removeLeadingZeros(numB);
    
    // Multiply as integers
    vector<int> result(numA.length() + numB.length(), 0);
    
    // Multiply each digit
    for (int i = numA.length() - 1; i >= 0; i--) {
        for (int j = numB.length() - 1; j >= 0; j--) {
            int mul = (numA[i] - '0') * (numB[j] - '0');
            int pos1 = i + j;
            int pos2 = i + j + 1;
            
            int sum = mul + result[pos2];
            result[pos2] = sum % 10;
            result[pos1] += sum / 10;
        }
    }
    
    // Convert result to string
    string resultStr = "";
    bool leadingZero = true;
    for (int i = 0; i < result.size(); i++) {
        if (result[i] != 0) leadingZero = false;
        if (!leadingZero) resultStr += char(result[i] + '0');
    }
    
    if (resultStr.empty()) resultStr = "0";
    
    // Insert decimal point
    int totalDecPlaces = decPlacesA + decPlacesB;
    if (totalDecPlaces > 0) {
        if (totalDecPlaces >= resultStr.length()) {
            resultStr = "0." + string(totalDecPlaces - resultStr.length(), '0') + resultStr;
        } else {
            resultStr = resultStr.substr(0, resultStr.length() - totalDecPlaces) + "." + 
                       resultStr.substr(resultStr.length() - totalDecPlaces);
        }
    }
    
    return removeTrailingZeros(removeLeadingZeros(resultStr));
}

// Add function with sign handling
string add(const string& operand1, const string& operand2) {
    if (!isValidNumber(operand1)) throw invalid_argument("Invalid first operand: " + operand1);
    if (!isValidNumber(operand2)) throw invalid_argument("Invalid second operand: " + operand2);
    
    bool neg1 = operand1[0] == '-';
    bool neg2 = operand2[0] == '-';
    
    string abs1 = neg1 ? operand1.substr(1) : operand1;
    string abs2 = neg2 ? operand2.substr(1) : operand2;
    
    if (abs1[0] == '+') abs1 = abs1.substr(1);
    if (abs2[0] == '+') abs2 = abs2.substr(1);
    
    string result;
    
    if (neg1 == neg2) {
        // Same signs: add absolute values
        result = addPositive(abs1, abs2);
        if (neg1) result = "-" + result;
    } else {
        // Different signs: subtract absolute values
        if (compareAbs(abs1, abs2) >= 0) {
            result = subtractPositive(abs1, abs2);
            if (neg1) result = "-" + result;
        } else {
            result = subtractPositive(abs2, abs1);
            if (neg2) result = "-" + result;
        }
    }
    
    return result == "-0" ? "0" : result;
}

// Subtract function
string subtract(const string& operand1, const string& operand2) {
    if (!isValidNumber(operand1)) throw invalid_argument("Invalid first operand: " + operand1);
    if (!isValidNumber(operand2)) throw invalid_argument("Invalid second operand: " + operand2);
    
    // a - b = a + (-b)
    string negOperand2 = operand2;
    if (operand2[0] == '-') {
        negOperand2 = operand2.substr(1);
    } else {
        negOperand2 = "-" + operand2;
    }
    
    return add(operand1, negOperand2);
}

// Multiply function
string multiply(const string& operand1, const string& operand2) {
    if (!isValidNumber(operand1)) throw invalid_argument("Invalid first operand: " + operand1);
    if (!isValidNumber(operand2)) throw invalid_argument("Invalid second operand: " + operand2);
    
    bool neg1 = operand1[0] == '-';
    bool neg2 = operand2[0] == '-';
    
    string abs1 = neg1 ? operand1.substr(1) : operand1;
    string abs2 = neg2 ? operand2.substr(1) : operand2;
    
    if (abs1[0] == '+') abs1 = abs1.substr(1);
    if (abs2[0] == '+') abs2 = abs2.substr(1);
    
    string result = multiplyPositive(abs1, abs2);
    
    if (result != "0" && (neg1 != neg2)) {
        result = "-" + result;
    }
    
    return result;
}

// Long division helper - divides integer by integer, returns quotient and remainder
pair<string, string> divideIntegers(const string& dividend, const string& divisor) {
    if (divisor == "0") throw domain_error("Division by zero");
    if (dividend == "0") return make_pair("0", "0");
    
    // If dividend is smaller than divisor, quotient is 0
    if (compareAbs(dividend, divisor) < 0) {
        return make_pair("0", dividend);
    }
    
    string quotient = "";
    string remainder = "";
    
    // Process each digit of dividend from left to right (like long division)
    for (int i = 0; i < dividend.length(); i++) {
        // Bring down next digit
        remainder += dividend[i];
        remainder = removeLeadingZeros(remainder);
        
        // Find how many times divisor goes into current remainder
        int count = 0;
        while (compareAbs(remainder, divisor) >= 0) {
            remainder = subtractPositive(remainder, divisor);
            count++;
        }
        
        quotient += char(count + '0');
    }
    
    quotient = removeLeadingZeros(quotient);
    remainder = removeLeadingZeros(remainder);
    
    return make_pair(quotient, remainder);
}

// Division function using long division algorithm
string divide(const string& operand1, const string& operand2) {
    if (!isValidNumber(operand1)) throw invalid_argument("Invalid first operand: " + operand1);
    if (!isValidNumber(operand2)) throw invalid_argument("Invalid second operand: " + operand2);
    
    if (operand2 == "0" || operand2 == "+0" || operand2 == "-0") {
        throw domain_error("Division by zero");
    }
    
    if (operand1 == "0" || operand1 == "+0" || operand1 == "-0") {
        return "0";
    }
    
    // Handle signs
    bool neg1 = operand1[0] == '-';
    bool neg2 = operand2[0] == '-';
    bool resultNegative = (neg1 != neg2);
    
    string abs1 = neg1 ? operand1.substr(1) : operand1;
    string abs2 = neg2 ? operand2.substr(1) : operand2;
    
    if (abs1[0] == '+') abs1 = abs1.substr(1);
    if (abs2[0] == '+') abs2 = abs2.substr(1);
    
    // Split into integer and decimal parts
    size_t dot1 = abs1.find('.');
    size_t dot2 = abs2.find('.');
    
    string int1 = (dot1 == string::npos) ? abs1 : abs1.substr(0, dot1);
    string dec1 = (dot1 == string::npos) ? "" : abs1.substr(dot1 + 1);
    
    string int2 = (dot2 == string::npos) ? abs2 : abs2.substr(0, dot2);
    string dec2 = (dot2 == string::npos) ? "" : abs2.substr(dot2 + 1);
    
    // Convert to integers by removing decimal points
    // dividend = int1 + dec1, divisor = int2 + dec2
    string dividend = int1 + dec1;
    string divisor = int2 + dec2;
    
    // Calculate decimal shift needed
    int decimalShift = dec2.length() - dec1.length();
    
    // If divisor has more decimal places, pad dividend with zeros
    if (decimalShift > 0) {
        dividend += string(decimalShift, '0');
        decimalShift = 0;
    }
    
    dividend = removeLeadingZeros(dividend);
    divisor = removeLeadingZeros(divisor);
    
    // Perform integer division
    pair<string, string> result = divideIntegers(dividend, divisor);
    string quotient = result.first;
    string remainder = result.second;
    
    // Handle decimal places in result
    if (decimalShift < 0) {
        int insertPos = quotient.length() + decimalShift;
        if (insertPos <= 0) {
            quotient = "0." + string(-insertPos, '0') + quotient;
        } else if (insertPos < quotient.length()) {
            quotient = quotient.substr(0, insertPos) + "." + quotient.substr(insertPos);
        }
    }
    
    // Add decimal precision if there's a remainder
    if (remainder != "0" && quotient.find('.') == string::npos) {
        quotient += ".";
    }
    
    // Continue division for decimal places (up to 15 decimal places for precision)
    int decimalPlaces = 0;
    const int MAX_DECIMAL_PLACES = 15;
    
    while (remainder != "0" && decimalPlaces < MAX_DECIMAL_PLACES) {
        remainder += "0"; // Multiply remainder by 10
        remainder = removeLeadingZeros(remainder);
        
        int count = 0;
        while (compareAbs(remainder, divisor) >= 0) {
            remainder = subtractPositive(remainder, divisor);
            count++;
        }
        
        if (quotient.back() != '.') {
            quotient += char(count + '0');
        } else {
            quotient += char(count + '0');
        }
        
        decimalPlaces++;
    }
    
    // Clean up result
    quotient = removeTrailingZeros(removeLeadingZeros(quotient));
    
    // Apply sign
    if (resultNegative && quotient != "0") {
        quotient = "-" + quotient;
    }
    
    return quotient;
}

// Power function using repeated multiplication (pen-and-pencil method)
string power(const string& base, const string& exponent) {
    if (!isValidNumber(base)) throw invalid_argument("Invalid base: " + base);
    if (!isValidNumber(exponent)) throw invalid_argument("Invalid exponent: " + exponent);
    
    // Handle special cases
    if (exponent == "0") return "1";
    if (base == "0") return "0";
    if (base == "1") return "1";
    if (exponent == "1") return base;
    
    // Check if exponent is negative
    bool negativeExponent = false;
    string absExponent = exponent;
    if (exponent[0] == '-') {
        negativeExponent = true;
        absExponent = exponent.substr(1);
    }
    
    // Handle decimal exponents
    if (absExponent.find('.') != string::npos) {
        return powerDecimal(base, exponent);
    }
    
    // Convert exponent to integer for repeated multiplication
    if (absExponent.length() > 9) {  // Prevent very large exponents that would take too long
        return "power(" + base + ", " + exponent + ") [exponent too large]";
    }
    
    // Parse exponent as integer
    long long exp = 0;
    for (char c : absExponent) {
        if (c < '0' || c > '9') {
            throw invalid_argument("Invalid exponent format: " + exponent);
        }
        exp = exp * 10 + (c - '0');
        if (exp > 1000) {  // Reasonable limit for performance
            return "power(" + base + ", " + exponent + ") [exponent too large for computation]";
        }
    }
    
    // Handle base case
    if (exp == 0) return "1";
    if (exp == 1) return base;
    
    // Repeated multiplication (pen-and-pencil method)
    string result = "1";
    string currentBase = base;
    
    // Use exponentiation by squaring for efficiency
    while (exp > 0) {
        if (exp % 2 == 1) {
            // If exponent is odd, multiply result by current base
            result = multiply(result, currentBase);
        }
        // Square the base and halve the exponent
        if (exp > 1) {
            currentBase = multiply(currentBase, currentBase);
        }
        exp /= 2;
    }
    
    // Handle negative exponent (result = 1/result)
    if (negativeExponent) {
        result = divide("1", result);
    }
    
    return result;
}

// Helper function to compute nth root using Newton's method (pen-and-pencil iterative approach)
string nthRoot(const string& number, const string& root) {
    if (!isValidNumber(number) || !isValidNumber(root)) {
        throw invalid_argument("Invalid input for nth root");
    }
    
    // Handle special cases
    if (number == "0") return "0";
    if (number == "1") return "1";
    if (root == "1") return number;
    if (root == "0") throw domain_error("Cannot take 0th root");
    
    // For negative numbers, only odd roots are defined
    bool negativeNumber = number[0] == '-';
    string absNumber = negativeNumber ? number.substr(1) : number;
    
    // Check if root is even and number is negative
    if (negativeNumber) {
        // Parse root to check if it's even (simplified check for integer roots)
        if (root.find('.') == string::npos) {
            long long rootInt = 0;
            for (char c : root) {
                if (c >= '0' && c <= '9') {
                    rootInt = rootInt * 10 + (c - '0');
                    if (rootInt > 100) break; // Prevent overflow for check
                }
            }
            if (rootInt % 2 == 0) {
                throw domain_error("Even root of negative number is undefined in real numbers");
            }
        }
    }
    
    // Newton's method: x_{n+1} = (1/n) * ((n-1)*x_n + a/x_n^(n-1))
    // For nth root of a, we solve x^n = a
    
    // Initial guess: use the number itself divided by root as starting point
    string x = divide(absNumber, root);
    
    // Ensure initial guess is reasonable
    if (compareAbs(x, "0.001") < 0) x = "0.001";
    if (compareAbs(x, "1000") > 0) x = "1000";
    
    string prev = "";
    const int maxIterations = 50;
    int iterations = 0;
    
    // Newton's method iterations
    while (iterations < maxIterations) {
        prev = x;
        
        try {
            // Compute x^(n-1)
            string rootMinus1 = subtract(root, "1");
            string xPowerNMinus1 = power(x, rootMinus1);
            
            // Compute a / x^(n-1)
            string dividend = divide(absNumber, xPowerNMinus1);
            
            // Compute (n-1)*x + a/x^(n-1)
            string term1 = multiply(rootMinus1, x);
            string numerator = add(term1, dividend);
            
            // Compute x = numerator / n
            x = divide(numerator, root);
            
            // Check convergence
            string diff = subtract(x, prev);
            if (diff[0] == '-') diff = diff.substr(1); // abs(diff)
            
            // If difference is very small, we've converged
            if (compareAbs(diff, "0.000000001") < 0) {
                break;
            }
            
        } catch (const exception& e) {
            // If we encounter errors during iteration, return best guess
            break;
        }
        
        iterations++;
    }
    
    // Apply sign for odd roots of negative numbers
    if (negativeNumber) {
        x = "-" + x;
    }
    
    return x;
}

// Handle decimal exponents using a^(p/q) = (a^p)^(1/q) = qth_root(a^p)
string powerDecimal(const string& base, const string& exponent) {
    if (!isValidNumber(base) || !isValidNumber(exponent)) {
        throw invalid_argument("Invalid input for decimal power");
    }
    
    // Handle special cases
    if (base == "0") {
        if (exponent[0] == '-') {
            throw domain_error("0 to negative power is undefined");
        }
        return "0";
    }
    if (base == "1") return "1";
    if (exponent == "0") return "1";
    if (exponent == "1") return base;
    
    bool negativeExponent = exponent[0] == '-';
    string absExponent = negativeExponent ? exponent.substr(1) : exponent;
    
    // Split decimal exponent into integer and fractional parts
    size_t dotPos = absExponent.find('.');
    string integerPart = absExponent.substr(0, dotPos);
    string fractionalPart = absExponent.substr(dotPos + 1);
    
    if (integerPart.empty()) integerPart = "0";
    
    // Convert fractional part to a rational number (p/q)
    // For example: 0.5 = 5/10 = 1/2, 0.25 = 25/100 = 1/4
    
    // Remove trailing zeros from fractional part
    while (!fractionalPart.empty() && fractionalPart.back() == '0') {
        fractionalPart.pop_back();
    }
    
    if (fractionalPart.empty()) {
        // Pure integer exponent
        return negativeExponent ? divide("1", power(base, integerPart)) : power(base, integerPart);
    }
    
    // Create fraction: fractionalPart / 10^(length of fractionalPart)
    string denominator = "1";
    for (size_t i = 0; i < fractionalPart.length(); i++) {
        denominator = multiply(denominator, "10");
    }
    
    // Simplify the fraction by finding GCD (simplified approach)
    string numerator = fractionalPart;
    
    // Simple GCD reduction for common cases
    while (numerator.length() > 1 && numerator.back() == '0' && denominator.length() > 1) {
        numerator = numerator.substr(0, numerator.length() - 1);
        denominator = denominator.substr(0, denominator.length() - 1);
    }
    
    try {
        string result;
        
        // If we have integer part, compute base^integerPart first
        if (integerPart != "0") {
            string integerResult = power(base, integerPart);
            
            // Then compute (base^integerPart)^(numerator/denominator)
            // which is denominator_root(integerResult^numerator)
            if (numerator != "0") {
                string powered = power(integerResult, numerator);
                result = nthRoot(powered, denominator);
            } else {
                result = integerResult;
            }
        } else {
            // Pure fractional exponent: base^(numerator/denominator) = denominator_root(base^numerator)
            string powered = power(base, numerator);
            result = nthRoot(powered, denominator);
        }
        
        // Handle negative exponent
        if (negativeExponent) {
            result = divide("1", result);
        }
        
        return result;
        
    } catch (const exception& e) {
        // Fallback for complex decimal exponents
        return "decimal_power(" + base + ", " + exponent + ") [approximation needed]";
    }
}

// Generic operation function
string operate(const string& operand1, char op, const string& operand2) {
    switch (op) {
        case '+': return add(operand1, operand2);
        case '-': return subtract(operand1, operand2);
        case '*': return multiply(operand1, operand2);
        case '/': return divide(operand1, operand2);
        case '^': return power(operand1, operand2);
        default:
            throw invalid_argument("Unknown operator: " + string(1, op));
    }
}