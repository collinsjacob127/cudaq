#include "bignum.h"

BigNum::BigNum(uint64_t value) {
    if (value > 0) {
        data.push_back(value);
    }
}

BigNum::BigNum(const std::string& str) {
    *this = fromString(str);
}

std::string BigNum::toString() const {
    std::stringstream ss;
    if (data.empty()) return "0";
    for (int i = data.size() - 1; i >= 0; --i) {
        if (i != (int)data.size() - 1)
            ss << std::setw(10) << std::setfill('0');
        ss << data[i];
    }
    return ss.str();
}

int BigNum::toInt() const {
    return (data.size() > 0) ? data[0] : 0;
}

BigNum BigNum::operator+(const BigNum& other) const {
    BigNum result;
    uint64_t carry = 0;
    size_t n = std::max(data.size(), other.data.size());
    result.data.resize(n);  // Resize to accommodate the largest number

    // Perform addition with carry propagation
    for (size_t i = 0; i < n; ++i) {
        uint64_t a = (i < data.size()) ? data[i] : 0;
        uint64_t b = (i < other.data.size()) ? other.data[i] : 0;
        uint64_t sum = a + b + carry;

        result.data[i] = sum;  // Store the lower 64 bits
        carry = (sum < a) || (sum < b) || (carry && sum == a + b);  // Correct carry calculation
    }

    // If there's still a carry left, add a new chunk to store it
    if (carry > 0) {
        result.data.push_back(carry);
    }

    return result;
}


BigNum BigNum::operator-(const BigNum& other) const {
    if (*this < other) throw std::underflow_error("Negative result in BigNum subtraction");

    BigNum result;
    int64_t borrow = 0;
    size_t n = data.size();
    result.data.resize(n);

    for (size_t i = 0; i < n; ++i) {
        uint64_t a = data[i];
        uint64_t b = (i < other.data.size()) ? other.data[i] : 0;

        if (a < b + borrow) {
            result.data[i] = BASE + a - b - borrow;
            borrow = 1;
        } else {
            result.data[i] = a - b - borrow;
            borrow = 0;
        }
    }

    while (result.data.size() > 1 && result.data.back() == 0)
        result.data.pop_back();

    return result;
}

BigNum BigNum::operator*(const BigNum& other) const {
    BigNum result;
    result.data.resize(data.size() + other.data.size(), 0);

    for (size_t i = 0; i < data.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < other.data.size() || carry; ++j) {
            uint64_t a = result.data[i + j] +
                         data[i] * (j < other.data.size() ? other.data[j] : 0) + carry;
            result.data[i + j] = a % BASE;
            carry = a / BASE;
        }
    }

    while (result.data.size() > 1 && result.data.back() == 0)
        result.data.pop_back();

    return result;
}

BigNum BigNum::operator/(const BigNum& other) const {
    if (other == BigNum(0)) throw std::overflow_error("Divide by zero exception in BigNum");

    BigNum dividend = *this;
    BigNum divisor = other;
    BigNum quotient, remainder;
    quotient.data.resize(data.size());

    for (int i = (int)data.size() - 1; i >= 0; --i) {
        remainder = remainder * BASE + dividend.data[i];
        uint64_t q_digit = remainder.divideSingleDigit(divisor);
        quotient.data[i] = q_digit;
        remainder = remainder - divisor * BigNum(q_digit);
    }

    while (quotient.data.size() > 1 && quotient.data.back() == 0)
        quotient.data.pop_back();

    return quotient;
}

BigNum BigNum::operator%(const BigNum& other) const {
    return *this - (*this / other) * other;
}

BigNum BigNum::operator<<(int shift) const {
    BigNum result = *this;
    uint64_t shift_blocks = shift / 32;
    int shift_bits = shift % 32;

    result.data.insert(result.data.begin(), shift_blocks, 0);

    if (shift_bits > 0) {
        uint64_t carry = 0;
        for (size_t i = 0; i < result.data.size(); ++i) {
            uint64_t current = result.data[i];
            result.data[i] = (current << shift_bits) | carry;
            carry = current >> (32 - shift_bits);
        }
        if (carry > 0) {
            result.data.push_back(carry);
        }
    }

    while (result.data.size() > 1 && result.data.back() == 0) {
        result.data.pop_back();
    }

    return result;
}

bool BigNum::operator<(const BigNum& other) const {
    if (data.size() != other.data.size())
        return data.size() < other.data.size();
    for (int i = data.size() - 1; i >= 0; --i) {
        if (data[i] != other.data[i])
            return data[i] < other.data[i];
    }
    return false;
}

bool BigNum::operator==(const BigNum& other) const {
    return data == other.data;
}

bool BigNum::operator!=(const BigNum& other) const {
    return !(*this == other);
}

bool BigNum::operator<=(const BigNum& other) const {
    return *this < other || *this == other;
}

bool BigNum::operator>(const BigNum& other) const {
    return !(*this <= other);
}

bool BigNum::operator>=(const BigNum& other) const {
    return !(*this < other);
}

uint64_t BigNum::divideSingleDigit(const BigNum& other) const {
    uint64_t low = 0, high = BASE - 1;
    while (low <= high) {
        uint64_t mid = (low + high) / 2;
        if (other * BigNum(mid) <= *this) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return high;
}

BigNum BigNum::fromString(const std::string& str) {
    BigNum result;
    BigNum base = 1;
    for (int i = str.size() - 1; i >= 0; --i) {
        result = result + base * BigNum(str[i] - '0');
        base = base * 10;
    }
    return result;
}
