#ifndef CONST_CONVERT_H
#define CONST_CONVERT_H

#include <memory>
#include <type_traits>
#include <cstdint>

#include "Types.h"
#include "AST.h"
#include "Const.h"

namespace ConstConvert
{
    template <typename T,
              typename = std::enable_if_t<
                  std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, uint32_t> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, uint64_t>>>
    inline std::shared_ptr<Constants::Const> cast(T v, const Types::DataType &targetType)
    {
        if (Types::isIntType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstInt(static_cast<int32_t>(v)));
        if (Types::isLongType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstLong(static_cast<int64_t>(v)));
        if (Types::isUIntType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstUInt(static_cast<uint32_t>(v)));
        if (Types::isULongType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstULong(static_cast<uint64_t>(v)));
        if (Types::isPointerType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstULong(static_cast<uint64_t>(v)));
        if (Types::isDoubleType(targetType))
            return std::make_shared<Constants::Const>(Constants::ConstDouble(static_cast<double>(v)));

        throw std::runtime_error("Internal error: cannot cast to non-scalar type");
    }

    inline std::shared_ptr<Constants::Const> convert(const Types::DataType &targetType, const std::shared_ptr<Constants::Const> &c)
    {
        if (auto constInt = Constants::getConstInt(*c))
            return cast(constInt->val, targetType);
        else if (auto constLong = Constants::getConstLong(*c))
            return cast(constLong->val, targetType);
        else if (auto constUInt = Constants::getConstUInt(*c))
            return cast(constUInt->val, targetType);
        else if (auto constULong = Constants::getConstULong(*c))
            return cast(constULong->val, targetType);
        else if (auto constDouble = Constants::getConstDouble(*c))
            if (Types::isDoubleType(targetType))
                return std::make_shared<Constants::Const>(Constants::ConstDouble(constDouble->val));
            else if (Types::isULongType(targetType))
                return cast(static_cast<uint64_t>(constDouble->val), targetType);
            else if (Types::isPointerType(targetType))
                throw std::runtime_error("Internal error: cannot convert double to pointer");
            else
                return cast(static_cast<int64_t>(constDouble->val), targetType);
        else
            throw std::runtime_error("Internal error: invalid const type");
    }
}

#endif