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

// Traits
#include <type_traits>
#include <concepts>
#include <variant>

// Structures
#include <vector>
#include <unordered_map>

// String/IO
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>

// General functionality
#include <functional>
#include <limits>
#include <cmath>
#include <memory>
#include <chrono>
#include <cassert>

#define LOG_ENABLED true
#define LOG(msg) if (LOG_ENABLED) { std::cerr << "[LOG] " << msg << std::endl; }

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

/**
 * @brief Memory pooling template
 * 
 * @tparam T 
 */
template <typename T>
class Pool {
    static std::vector<std::unique_ptr<T>> pool;
    static std::mutex mutex;
public:
    template<typename... Args>
    static std::unique_ptr<T> get(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!pool.empty()) {
            std::unique_ptr<T> ptr = std::move(pool.back());
            pool.pop_back();
            new (ptr.get()) T(std::forward<Args>(args)...);
            return ptr;
        }
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    static void release(std::unique_ptr<T>&& obj) {
        std::lock_guard<std::mutex> lock(mutex);
        pool.push_back(std::move(obj));
    }
};

// Forward declaration of promotion function
std::unique_ptr<dbase> promote(double lhs, double rhs);

template<typename T>
std::vector<std::unique_ptr<T>> Pool<T>::pool;

template<typename T>
std::mutex Pool<T>::mutex;

#define DEFINE_NUMERIC_CLASS(NAME, TYPE) \
class dy##NAME : public dbase { \
    TYPE value; \
public: \
    explicit dy##NAME(TYPE v) : value(v) {} \
    std::unique_ptr<dbase> add(const dbase& other) const override { \
        return promote(this->value, other.toDouble()); \
    } \
    std::unique_ptr<dbase> subtract(const dbase& other) const override { \
        return promote(this->value - other.toDouble(), 0); \
    } \
    std::unique_ptr<dbase> multiply(const dbase& other) const override { \
        return promote(this->value * other.toDouble(), 0); \
    } \
    std::unique_ptr<dbase> divide(const dbase& other) const override { \
        double denom = other.toDouble(); \
        if (denom == 0.0) throw std::runtime_error("Division by zero"); \
        return promote(this->value / denom, 0); \
    } \
    bool lessThan(const dbase& other) const override { return this->value < other.toDouble(); } \
    bool equals(const dbase& other) const override { return this->value == other.toDouble(); } \
    double toDouble() const override { return static_cast<double>(value); } \
    std::string typeName() const override { return #TYPE; } \
    size_t sizeInBytes() const override { return sizeof(TYPE); } \
    std::string serialize() const override { return std::to_string(value); } \
    std::unique_ptr<dbase> clone() const override { return std::make_unique<dy##NAME>(*this); } \
};

DEFINE_NUMERIC_CLASS(Short, short)
DEFINE_NUMERIC_CLASS(Int, int)
DEFINE_NUMERIC_CLASS(Float, float)
DEFINE_NUMERIC_CLASS(Double, double)

/**
 * @brief Promotion logic
 * 
 * @param value 
 * @return std::unique_ptr<dbase> 
 */
std::unique_ptr<dbase> promote(double lhs, double rhs) {
    double result = lhs + rhs;

    // Non-floating point number promotion
    if (std::floor(result) == result) {
        if (result >= std::numeric_limits<short>::min() && result <= std::numeric_limits<short>::max())
            return Pool<dyShort>::get(static_cast<short>(result));
        if (result >= std::numeric_limits<int>::min() && result <= std::numeric_limits<int>::max())
            return Pool<dyInt>::get(static_cast<int>(result));
    }

    // Floating point number promotion
    if (result >= std::numeric_limits<float>::lowest() && result <= std::numeric_limits<float>::max())
        return Pool<dyFloat>::get(static_cast<float>(result));
    if (result >= std::numeric_limits<double>::lowest() && result <= std::numeric_limits<double>::max()) 
        return Pool<dyDouble>::get(static_cast<double>(result));

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
    std::string serialize() const { return value->typeName() + ":" + value->serialize(); }
    static dnum deserialize(const std::string& s) {
        auto delim = s.find(':');
        if (delim == std::string::npos) throw std::invalid_argument("Invalid format");

        std::string type = s.substr(0, delim);
        std::string val = s.substr(delim + 1);
        std::istringstream iss(val);

        if (type == "short") { short v; iss >> v; return dnum(Pool<dyShort>::get(v)); }
        if (type == "int")   { int v;   iss >> v; return dnum(Pool<dyInt>::get(v)); }
        if (type == "float") { float v; iss >> v; return dnum(Pool<dyFloat>::get(v)); }
        if (type == "double"){ double v;iss >> v; return dnum(Pool<dyDouble>::get(v)); }

        throw std::invalid_argument("Unknown type: " + type);
    }

    friend std::ostream& operator<<(std::ostream& os, const dnum& n) {
        os << n.value->toDouble() << " (" << n.value->typeName() << ", " << n.value->sizeInBytes() << " bytes)";
        return os;
    }
};

/**
 * @brief Unit tests
 * 
 */
void runUnitTests() {
    LOG("Running unit tests...");
    dnum a(10), b(5);

    std::cout << (a+b).serialize() << std::endl;
    assert((a + b).serialize() == "short:15");
    assert((a - b).serialize() == "short:5");
    assert((a * b).serialize() == "short:50");
    assert((a / b).serialize() == "short:2");
    assert((a < b) == false);
    assert((a == b) == false);
    assert((a == dnum(10)) == true);
    LOG("All unit tests passed.");
}

/**
 * @brief Benchmark test
 * 
 */
void runBenchmark() {
    LOG("Running benchmark...");
    auto start = std::chrono::high_resolution_clock::now();
    dnum sum(0);
    for (int i = 0; i < 1000000; ++i) {
        sum = sum + dnum(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    LOG("Benchmark completed in " << elapsed.count() << " seconds");
}

int main() {
    runUnitTests();
    runBenchmark();
 
    return 0;
}