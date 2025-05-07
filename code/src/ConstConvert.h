#ifndef CONST_CONVERT_H
#define CONST_CONVERT_H

#include <memory>

#include "Types.h"
#include "AST.h"
#include "Const.h"

namespace ConstConvert
{
    inline std::shared_ptr<Constants::Const> convert(const Types::DataType &targetType, const std::shared_ptr<Constants::Const> &c)
    {
        if (auto constInt = Constants::getConstInt(*c))
        {
            if (Types::isIntType(targetType))
                return std::make_shared<Constants::Const>(constInt.value());
            else
                return std::make_shared<Constants::Const>(Constants::makeConstLong(static_cast<int64_t>(constInt->val)));
        }
        else if (auto constLong = Constants::getConstLong(*c))
        {
            if (Types::isLongType(targetType))
                return std::make_shared<Constants::Const>(constLong.value());
            else
                return std::make_shared<Constants::Const>(Constants::makeConstInt(static_cast<int32_t>(constLong->val)));
        }
        else
            throw std::runtime_error("Internal error: Unknown constant type");
    }
}

#endif