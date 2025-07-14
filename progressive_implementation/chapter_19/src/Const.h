#ifndef CONST_H
#define CONST_H

#include <variant>
#include <string>
#include <cstdint>
#include <optional>

#include "Types.h"
#include "./utils/VariantHelper.h"

namespace Constants
{
    struct ConstChar
    {
        int8_t val;
        explicit ConstChar(int8_t v) : val{v} {}
        std::string toString() const { return "ConstChar(" + std::to_string(val) + ")"; }
    };

    struct ConstUChar
    {
        uint8_t val;
        explicit ConstUChar(uint8_t v) : val{v} {}
        std::string toString() const { return "ConstUChar(" + std::to_string(val) + ")"; }
    };

    struct ConstInt
    {
        int32_t val;
        explicit ConstInt(int32_t v) : val{v} {}
        std::string toString() const { return "ConstInt(" + std::to_string(val) + ")"; }
    };

    struct ConstLong
    {
        int64_t val;
        explicit ConstLong(int64_t v) : val{v} {}
        std::string toString() const { return "ConstLong(" + std::to_string(val) + ")"; }
    };

    struct ConstUInt
    {
        uint32_t val;
        explicit ConstUInt(uint32_t v) : val{v} {}
        std::string toString() const { return "ConstUInt(" + std::to_string(val) + ")"; }
    };

    struct ConstULong
    {
        uint64_t val;
        explicit ConstULong(uint64_t v) : val{v} {}
        std::string toString() const { return "ConstULong(" + std::to_string(val) + ")"; }
    };

    struct ConstDouble
    {
        double val;
        explicit ConstDouble(double v) : val{v} {}
        std::string toString() const { return "ConstDouble(" + std::to_string(val) + ")"; }
    };

    using Const = std::variant<ConstChar, ConstUChar, ConstInt, ConstLong, ConstUInt, ConstULong, ConstDouble>;

    inline std::string toString(const Const &c)
    {
        return std::visit([](const auto &obj)
                          { return obj.toString(); }, c);
    }

    // Factory functions
    inline Const makeConstChar(int8_t val) { return ConstChar{static_cast<int8_t>(val)}; }
    inline Const makeConstUChar(uint8_t val) { return ConstUChar{static_cast<uint8_t>(val)}; }
    inline Const makeConstInt(int32_t val) { return ConstInt{static_cast<int32_t>(val)}; }
    inline Const makeConstLong(int64_t val) { return ConstLong{static_cast<int64_t>(val)}; }
    inline Const makeConstUInt(uint32_t val) { return ConstUInt{static_cast<uint32_t>(val)}; }
    inline Const makeConstULong(uint64_t val) { return ConstULong{static_cast<uint64_t>(val)}; }
    inline Const makeConstDouble(double val) { return ConstDouble{static_cast<double>(val)}; }

    inline ConstInt makeIntZero() { return ConstInt{0}; }
    inline ConstLong makeLongZero() { return ConstLong{0}; }
    inline ConstUInt makeUIntZero() { return ConstUInt{0}; }
    inline ConstULong makeULongZero() { return ConstULong{0}; }

    // Type checkers
    inline bool isConstChar(const Const &c) { return isVariant<ConstChar>(c); }
    inline bool isConstUChar(const Const &c) { return isVariant<ConstUChar>(c); }
    inline bool isConstInt(const Const &c) { return isVariant<ConstInt>(c); }
    inline bool isConstLong(const Const &c) { return isVariant<ConstLong>(c); }
    inline bool isConstUInt(const Const &c) { return isVariant<ConstUInt>(c); }
    inline bool isConstULong(const Const &c) { return isVariant<ConstULong>(c); }
    inline bool isConstDouble(const Const &c) { return isVariant<ConstDouble>(c); }

    // Getters
    inline std::optional<ConstChar> getConstChar(const Const &c) { return getVariant<ConstChar>(c); }
    inline std::optional<ConstUChar> getConstUChar(const Const &c) { return getVariant<ConstUChar>(c); }
    inline std::optional<ConstInt> getConstInt(const Const &c) { return getVariant<ConstInt>(c); }
    inline std::optional<ConstLong> getConstLong(const Const &c) { return getVariant<ConstLong>(c); }
    inline std::optional<ConstUInt> getConstUInt(const Const &c) { return getVariant<ConstUInt>(c); }
    inline std::optional<ConstULong> getConstULong(const Const &c) { return getVariant<ConstULong>(c); }
    inline std::optional<ConstDouble> getConstDouble(const Const &c) { return getVariant<ConstDouble>(c); }

    // Type-of-const
    inline Types::DataType typeOfConst(const Const &c)
    {
        if (isConstChar(c))
            return Types::makeSCharType();
        if (isConstUChar(c))
            return Types::makeUCharType();
        if (isConstInt(c))
            return Types::makeIntType();
        if (isConstLong(c))
            return Types::makeLongType();
        if (isConstUInt(c))
            return Types::makeUIntType();
        if (isConstULong(c))
            return Types::makeULongType();
        if (isConstDouble(c))
            return Types::makeDoubleType();
        throw std::runtime_error("Internal error: unknown const type");
    }

    // Equality and less-than for each struct
    inline bool operator==(const ConstChar &a, const ConstChar &b) { return a.val == b.val; }
    inline bool operator==(const ConstUChar &a, const ConstUChar &b) { return a.val == b.val; }
    inline bool operator==(const ConstInt &a, const ConstInt &b) { return a.val == b.val; }
    inline bool operator==(const ConstLong &a, const ConstLong &b) { return a.val == b.val; }
    inline bool operator==(const ConstUInt &a, const ConstUInt &b) { return a.val == b.val; }
    inline bool operator==(const ConstULong &a, const ConstULong &b) { return a.val == b.val; }
    inline bool operator==(const ConstDouble &a, const ConstDouble &b) { return a.val == b.val; }

    inline bool operator<(const ConstChar &a, const ConstChar &b) { return a.val < b.val; }
    inline bool operator<(const ConstUChar &a, const ConstUChar &b) { return a.val < b.val; }
    inline bool operator<(const ConstInt &a, const ConstInt &b) { return a.val < b.val; }
    inline bool operator<(const ConstLong &a, const ConstLong &b) { return a.val < b.val; }
    inline bool operator<(const ConstUInt &a, const ConstUInt &b) { return a.val < b.val; }
    inline bool operator<(const ConstULong &a, const ConstULong &b) { return a.val < b.val; }
    inline bool operator<(const ConstDouble &a, const ConstDouble &b) { return a.val < b.val; }

    // Equality and less-than for the variant
    inline bool operator==(const Const &a, const Const &b)
    {
        return a.index() == b.index() && std::visit([](const auto &lhs, const auto &rhs)
                                                    {
            using T = std::decay_t<decltype(lhs)>;
            using U = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<T, U>)
                return lhs == rhs;
            else
                return false; }, a, b);
    }

    inline bool operator<(const Const &a, const Const &b)
    {
        if (a.index() != b.index())
            return a.index() < b.index();
        return std::visit([](const auto &lhs, const auto &rhs)
                          {
            using T = std::decay_t<decltype(lhs)>;
            using U = std::decay_t<decltype(rhs)>;
            if constexpr (std::is_same_v<T, U>)
                return lhs < rhs;
            else
                return false; }, a, b);
    }

    // Comparison helpers (only need == and < for all logic)
    inline bool eq(const Const &a, const Const &b) { return a == b; }
    inline bool lt(const Const &a, const Const &b) { return a < b; }
    inline bool gt(const Const &a, const Const &b) { return b < a; }
    inline bool le(const Const &a, const Const &b) { return !(b < a); }
    inline bool ge(const Const &a, const Const &b) { return !(a < b); }

    // isZero helper
    inline bool isZero(const Const &c)
    {
        return std::visit([](auto &&val) -> bool
                          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, ConstChar> || std::is_same_v<T, ConstUChar>)
                return val.val == 0;
            else if constexpr (std::is_same_v<T, ConstInt> || std::is_same_v<T, ConstLong> ||
                               std::is_same_v<T, ConstUInt> || std::is_same_v<T, ConstULong>)
                return val.val == 0;
            else if constexpr (std::is_same_v<T, ConstDouble>)
                return val.val == 0.0;
            else
                return false; }, c);
    }

    // Addition
    inline Const add(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val + y.val)};
            if constexpr (std::is_same_v<X, ConstDouble> && std::is_same_v<Y, ConstDouble>)
                return ConstDouble{static_cast<double>(x.val + y.val)};
            throw std::runtime_error("add: unsupported types"); }, a, b);
    }

    // Subtraction
    inline Const sub(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val - y.val)};
            if constexpr (std::is_same_v<X, ConstDouble> && std::is_same_v<Y, ConstDouble>)
                return ConstDouble{static_cast<double>(x.val - y.val)};
            throw std::runtime_error("sub: unsupported types"); }, a, b);
    }

    // Multiplication
    inline Const mul(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val * y.val)};
            if constexpr (std::is_same_v<X, ConstDouble> && std::is_same_v<Y, ConstDouble>)
                return ConstDouble{static_cast<double>(x.val * y.val)};
            throw std::runtime_error("mul: unsupported types"); }, a, b);
    }

    // Division
    inline Const div(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstChar{static_cast<int8_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstUChar{static_cast<uint8_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstInt{static_cast<int32_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstLong{static_cast<int64_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstUInt{static_cast<uint32_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>) {
                if (y.val == 0) throw std::runtime_error("div by zero");
                return ConstULong{static_cast<uint64_t>(x.val / y.val)};
            }
            if constexpr (std::is_same_v<X, ConstDouble> && std::is_same_v<Y, ConstDouble>) {
                if (y.val == 0.0) throw std::runtime_error("div by zero");
                return ConstDouble{static_cast<double>(x.val / y.val)};
            }
            throw std::runtime_error("div: unsupported types"); }, a, b);
    }

    // Modulo
    inline Const modulo(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstChar{static_cast<int8_t>(x.val % y.val)};
            }
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstUChar{static_cast<uint8_t>(x.val % y.val)};
            }
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstInt{static_cast<int32_t>(x.val % y.val)};
            }
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstLong{static_cast<int64_t>(x.val % y.val)};
            }
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstUInt{static_cast<uint32_t>(x.val % y.val)};
            }
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>) {
                if (y.val == 0) throw std::runtime_error("modulo by zero");
                return ConstULong{static_cast<uint64_t>(x.val % y.val)};
            }
            throw std::runtime_error("modulo: unsupported types"); }, a, b);
    }

    // zeroLike: returns a zero constant of the same type as input
    inline Const zeroLike(const Const &a)
    {
        return std::visit([](auto &&x) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, ConstChar>)
                return ConstChar{static_cast<int8_t>(0)};
            if constexpr (std::is_same_v<X, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(0)};
            if constexpr (std::is_same_v<X, ConstInt>)
                return ConstInt{static_cast<int32_t>(0)};
            if constexpr (std::is_same_v<X, ConstLong>)
                return ConstLong{static_cast<int64_t>(0)};
            if constexpr (std::is_same_v<X, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(0)};
            if constexpr (std::is_same_v<X, ConstULong>)
                return ConstULong{static_cast<uint64_t>(0)};
            if constexpr (std::is_same_v<X, ConstDouble>)
                return ConstDouble{static_cast<double>(0.0)};
            throw std::runtime_error("zeroLike: unsupported type"); }, a);
    }

    // Bitwise AND
    inline Const bitwiseAnd(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val & y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val & y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val & y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val & y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val & y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val & y.val)};
            throw std::runtime_error("BitwiseAnd: unsupported types"); }, a, b);
    }

    // Bitwise OR
    inline Const bitwiseOr(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val | y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val | y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val | y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val | y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val | y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val | y.val)};
            throw std::runtime_error("BitwiseOr: unsupported types"); }, a, b);
    }

    // Bitwise XOR
    inline Const bitwiseXor(const Const &a, const Const &b)
    {
        return std::visit([](auto &&x, auto &&y) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            using Y = std::decay_t<decltype(y)>;
            if constexpr (std::is_same_v<X, ConstChar> && std::is_same_v<Y, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val ^ y.val)};
            if constexpr (std::is_same_v<X, ConstUChar> && std::is_same_v<Y, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val ^ y.val)};
            if constexpr (std::is_same_v<X, ConstInt> && std::is_same_v<Y, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val ^ y.val)};
            if constexpr (std::is_same_v<X, ConstLong> && std::is_same_v<Y, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val ^ y.val)};
            if constexpr (std::is_same_v<X, ConstUInt> && std::is_same_v<Y, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val ^ y.val)};
            if constexpr (std::is_same_v<X, ConstULong> && std::is_same_v<Y, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val ^ y.val)};
            throw std::runtime_error("BitwiseXor: unsupported types"); }, a, b);
    }

    inline int64_t getShiftAmount(const Const &c)
    {
        return std::visit([](auto &&val) -> int64_t
                          {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, ConstChar> || std::is_same_v<T, ConstUChar>)
                return val.val;
            if constexpr (std::is_same_v<T, ConstInt> || std::is_same_v<T, ConstLong>)
                return val.val;
            if constexpr (std::is_same_v<T, ConstUInt> || std::is_same_v<T, ConstULong>)
                return static_cast<int64_t>(val.val);
            throw std::runtime_error("Shift amount must be integer type"); }, c);
    }

    // Bitshift left
    inline Const bitshiftLeft(const Const &a, const Const &b)
    {
        int64_t shift = getShiftAmount(b);
        return std::visit([shift](auto &&x) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val << shift)};
            if constexpr (std::is_same_v<X, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val << shift)};
            if constexpr (std::is_same_v<X, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val << shift)};
            if constexpr (std::is_same_v<X, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val << shift)};
            if constexpr (std::is_same_v<X, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val << shift)};
            if constexpr (std::is_same_v<X, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val << shift)};
            throw std::runtime_error("bitshiftLeft: unsupported left operand type"); }, a);
    }

    // Bitshift right
    inline Const bitshiftRight(const Const &a, const Const &b)
    {
        int64_t shift = getShiftAmount(b);
        return std::visit([shift](auto &&x) -> Const
                          {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, ConstChar>)
                return ConstChar{static_cast<int8_t>(x.val >> shift)};
            if constexpr (std::is_same_v<X, ConstUChar>)
                return ConstUChar{static_cast<uint8_t>(x.val >> shift)};
            if constexpr (std::is_same_v<X, ConstInt>)
                return ConstInt{static_cast<int32_t>(x.val >> shift)};
            if constexpr (std::is_same_v<X, ConstLong>)
                return ConstLong{static_cast<int64_t>(x.val >> shift)};
            if constexpr (std::is_same_v<X, ConstUInt>)
                return ConstUInt{static_cast<uint32_t>(x.val >> shift)};
            if constexpr (std::is_same_v<X, ConstULong>)
                return ConstULong{static_cast<uint64_t>(x.val >> shift)};
            throw std::runtime_error("bitshiftRight: unsupported left operand type"); }, a);
    }
};

#endif