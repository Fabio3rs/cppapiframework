#pragma once

#include "InputValidators.hpp"

namespace ShortValidationsName {

using required = RequiredValidator;
using email = EmailValidator;
using integer = IntegerValidator;
using object = ObjectValidator;
using array = ArrayValidator;
using stringlen = StringLengthValidator;
using nullable = DefaultIfNotPresentValidator;

} // namespace ShortValidationsName
