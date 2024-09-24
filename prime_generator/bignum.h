#ifndef BIGNUM_H
#define BIGNUM_H

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstdint>  // <-- Add this for fixed-width integer types

class BigNum {
public:
    std::vector<uint64_t> data;
    static const uint64_t BASE = 1ULL << 32;

    BigNum(uint64_t value = 0);
    BigNum(const std::string& str);

    std::string toString() const;
    int toInt() const;

    // Arithmetic operators
    BigNum operator+(const BigNum& other) const;
    BigNum operator-(const BigNum& other) const;
    BigNum operator*(const BigNum& other) const;
    BigNum operator/(const BigNum& other) const;
    BigNum operator%(const BigNum& other) const;

    // Shift operator
    BigNum operator<<(int shift) const;

    // Comparison operators
    bool operator<(const BigNum& other) const;
    bool operator<=(const BigNum& other) const;
    bool operator>(const BigNum& other) const;
    bool operator>=(const BigNum& other) const;
    bool operator==(const BigNum& other) const;
    bool operator!=(const BigNum& other) const;

private:
    uint64_t divideSingleDigit(const BigNum& other) const;
    BigNum fromString(const std::string& str);
};

#endif // BIGNUM_H
