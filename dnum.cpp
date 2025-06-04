/**
 * @file dnum.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-06-03
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <type_traits>
#include <concepts>
#include <vector>
#include <variant>

#include <iostream>
#include <memory>
#include <cmath>
#include <limits>

#include <string>
#include <stdexcept>
#include <sstream>
#include <unordered_map>

#include <functional>

/**
 * @brief Abstract base class
 * 
 */
class dbase {
public:
    virtual ~dbase() = default;
    virtual std::unique_ptr<dbase> add(const dbase& other) const = 0;
    virtual std::unique_ptr<dbase> subtract(const dbase& other) const = 0;
    virtual std::unique_ptr<dbase> multiply(const dbase& other) const = 0;
    virtual std::unique_ptr<dbase> divide(const dbase& other) const = 0;

    virtual bool lessThan(const dbase& other) const = 0;
    virtual bool equals(const dbase& other) const = 0;

    virtual double toDouble() const = 0;
    virtual std::string typeName() const = 0;
    virtual size_t sizeInBytes() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::unique_ptr<dbase> clone() const = 0;
};


// Forward declaration of promotion function
std::unique_ptr<dbase> promote(double lhs, double rhs);

/**
 * @brief DynamicShort class
 * 
 */
class dyshort : public dbase {
    short value;

public:
    explicit dyshort(short v) : value(v) {}

    std::unique_ptr<dbase> add(const dbase& other) const override {
        return promote(this->toDouble() + other.toDouble());
    }

    std::unique_ptr<dbase> subtract(const dbase& other) const override {
        return promote(this->toDouble() - other.toDouble());
    }

    std::unique_ptr<dbase> multiply(const dbase& other) const override {
        return promote(this->toDouble() * other.toDouble());
    }

    std::unique_ptr<dbase> divide(const dbase& other) const override {
        double denominator = other.toDouble();
        if (denominator == 0.0) throw std::runtime_error("Division by zero");
        return promote(this->toDouble() / denominator);
    }

    double toDouble() const override {
        return static_cast<double>(value);
    }

    std::string typeName() const override {
        return "short";
    }

    size_t sizeInBytes() const override {
        return sizeof(short);
    }

    std::string serialize() const override {
        return std::to_string(value);
    }
};

/**
 * @brief DynamicInt class
 * 
 */
class dyint : public dbase {
    int value;

public:
    explicit dyint(int v) : value(v) {}

    std::unique_ptr<dbase> add(const dbase& other) const override {
        return promote(this->toDouble() + other.toDouble());
    }

    std::unique_ptr<dbase> subtract(const dbase& other) const override {
        return promote(this->toDouble() - other.toDouble());
    }

    std::unique_ptr<dbase> multiply(const dbase& other) const override {
        return promote(this->toDouble() * other.toDouble());
    }

    std::unique_ptr<dbase> divide(const dbase& other) const override {
        double denominator = other.toDouble();
        if (denominator == 0.0) throw std::runtime_error("Division by zero");
        return promote(this->toDouble() / denominator);
    }

    double toDouble() const override {
        return static_cast<double>(value);
    }

    std::string typeName() const override {
        return "int";
    }

    size_t sizeInBytes() const override {
        return sizeof(int);
    }

    std::string serialize() const override {
        return std::to_string(value);
    }
};


/**
 * @brief DynamicFloat class
 * 
 */
class dyfloat : public dbase {
    float value;

public:
    explicit dyfloat(float v) : value(v) {}

    std::unique_ptr<dbase> add(const dbase& other) const override {
        return promote(this->toDouble() + other.toDouble());
    }

    std::unique_ptr<dbase> subtract(const dbase& other) const override {
        return promote(this->toDouble() - other.toDouble());
    }

    std::unique_ptr<dbase> multiply(const dbase& other) const override {
        return promote(this->toDouble() * other.toDouble());
    }

    std::unique_ptr<dbase> divide(const dbase& other) const override {
        double denominator = other.toDouble();
        if (denominator == 0.0) throw std::runtime_error("Division by zero");
        return promote(this->toDouble() / denominator);
    }

    double toDouble() const override {
        return static_cast<double>(value);
    }

    std::string typeName() const override {
        return "float";
    }

    size_t sizeInBytes() const override {
        return sizeof(float);
    }

    std::string serialize() const override {
        return std::to_string(value);
    }
};

/**
 * @brief DynamicDouble class
 * 
 */
class dydouble : public dbase {
    double value;
public:
    explicit dydouble(double v) : value(v) {}

    std::unique_ptr<dbase> add(const dbase& other) const override {
        return promote(this->toDouble() + other.toDouble());
    }

    std::unique_ptr<dbase> subtract(const dbase& other) const override {
        return promote(this->toDouble() - other.toDouble());
    }

    std::unique_ptr<dbase> multiply(const dbase& other) const override {
        return promote(this->toDouble() * other.toDouble());
    }

    std::unique_ptr<dbase> divide(const dbase& other) const override {
        double denominator = other.toDouble();
        if (denominator == 0.0) throw std::runtime_error("Division by zero");
        return promote(this->toDouble() / denominator);
    }

    double toDouble() const override {
        return value;
    }

    std::string typeName() const override {
        return "double";
    }

    size_t sizeInBytes() const override {
        return sizeof(double);
    }

    std::string serialize() const override {
        return std::to_string(value);
    }
};


/**
 * @brief Promotion logic
 * 
 * @param value 
 * @return std::unique_ptr<dbase> 
 */
std::unique_ptr<dbase> promote(double value) {
    if (std::floor(value) == value) {
        if (value >= std::numeric_limits<short>::min() && value <= std::numeric_limits<short>::max())
            return std::make_unique<dyshort>(static_cast<short>(value));

        if (value >= std::numeric_limits<int>::min() && value <= std::numeric_limits<int>::max())
            return std::make_unique<dyint>(static_cast<int>(value));
    }

    if (value >= std::numeric_limits<float>::lowest() && value <= std::numeric_limits<float>::max())
        return std::make_unique<dyfloat>(static_cast<float>(value));
    if (value >= std::numeric_limits<double>::lowest() && value <= std::numeric_limits<double>::max()) 
        return std::make_unique<dydouble>(static_cast<double>(value));

    throw std::overflow_error("Value too large for supported types");
}

/**
 * @brief Wrapper class
 * 
 */
class dnum {
    std::unique_ptr<dbase> value;

public:
    dnum(double v) : value(promote(v, 0)) {}
    dnum(std::unique_ptr<dbase> v) : value(std::move(v)) {}

    // Operators
    dnum operator+(const dnum& other) const { return dnum(value->add(*other.value)); }
    dnum operator-(const dnum& other) const { return dnum(value->subtract(*other.value)); }
    dnum operator*(const dnum& other) const { return dnum(value->multiply(*other.value)); }
    dnum operator/(const dnum& other) const { return dnum(value->divide(*other.value)); }

    bool operator<(const dnum& other) const { return value->lessThan(*other.value); }
    bool operator==(const dnum& other) const { return value->equals(*other.value); }

    // Type Query
    std::string getTypeName() const { return value->typeName(); }
    size_t getSizeInBytes() const { return value->sizeInBytes(); }

    // Serialization
    std::string serialize() const { return value->serialize(); }

    friend std::ostream& operator<<(std::ostream& os, const dnum& n) {
        os << n.value->toDouble() << " (" << n.value->typeName() << ", " << n.value->sizeInBytes() << " bytes)";
        return os;
    }
};