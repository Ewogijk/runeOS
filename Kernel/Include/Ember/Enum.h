/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef EMBER_ENUM_H
#define EMBER_ENUM_H


#include <stddef.h>


/**
 * @brief
 * @param str C string.
 * @return Length of the c string.
 */
size_t e_strlen(const char* str);


int e_memcmp(const void* lhs, const void* rhs, size_t count);


#define ENUM_VALUE(ClassName, Name, Value) Name = Value,

#define ENUM_TO_STRING(ClassName, Name, Value) if (_value == ClassName::Name) return #Name;

#define ENUM_FROM_STRING(ClassName, Name, Value) if (e_memcmp(str, #Name, e_strlen(str)) == 0) return ClassName::Name;

#define ENUM_CONSTRUCT_FROM_STRING(ClassName, Name, Value) if (e_memcmp(str, #Name, e_strlen(str)) == 0) _value = ClassName::Name;

#define ENUM_FROM_VALUE(ClassName, Name, Value)  if (value == Value) return ClassName::Name;

#define ENUM_CONSTRUCT_FROM_VALUE(ClassName, Name, Value)  if (value == Value) _value = ClassName::Name;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Typed Enum
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


/**
 * Preprocessor macro black magic that automatically declares a class with a nested typed enum class and conversion
 * functions. The enum will have a "None" value the represents the absence of an enum value, the numerical value of
 * "None" can be chosen.
 *
 * <p>
 *  The macro parameters are:
 *  <ol>
 *      <li>ClassName: Name of the class.</li>
 *      <li>EnumType:  Type for the nested typed eum class, e.g. uint8_t</li>
 *      <li>EnumDefs:  X Macro with enum definitions, will be explained in the next section.</li>
 *      <li>NoneValue: The numerical value of the "None" enum value.</li>
 *  </ol>
 * </p>
 *
 * <p>
 *  The EnumDefs X macro defines a list with all enum values for the nested enum class and requires the following
 *  parameters:
 *  <ol>
 *      <li>ClassName: Name of the class, must match with the one provided to "DECLARE_TYPED_ENUM".</li>
 *      <li>Name:  Name of the enum value.</li>
 *      <li>Value: Numerical value of the enum value.</li>
 *  </ol>
 * </p>
 *
 * <p>
 *  The following conversion (static and member) functions are automatically generated:
 *  <ol>
 *      <li>to_string (Member): The "Name" parameter of the enum value provided to "EnumDefs".</li>
 *      <li>to_value (Member):    The "Value" parameter of the enum value provided to "EnumDefs"</li>
 *      <li>from_string (Static): The enum value whose "Name" matches with the given string else "None".</li>
 *      <li>from_value (Static): The enum value whose "Value" matches with the given number else "None".</li>
 *  </ol>
 * </p>
 *
 * <p>
 * Usage: <br><br>
 * #define FRUITS(X) \ <br>
 *     X(Fruit, Strawberry, 2)\ <br>
 *     X(Fruit, Banana, 4)\ <br><br>
 *  In header file: <br>
 *  DECLARE_TYPED_ENUM(Fruit, U8, FRUITS, 0) <br><br>
 *  In Source file: <br>
 *  DEFINE_TYPED_ENUM(Fruit, U8, FRUITS, 0)
 * </p>
 */
#define DECLARE_TYPED_ENUM(ClassName, EnumType, EnumDefs, NoneValue)                \
class ClassName {                                                                   \
public:                                                                             \
    enum _E : EnumType {                                                            \
        NONE = NoneValue,                                                           \
        EnumDefs(ENUM_VALUE)                                                        \
    };                                                                              \
                                                                                    \
                                                                                    \
    ClassName() = default;                                                          \
                                                                                    \
                                                                                    \
    explicit ClassName(const char* str);                                            \
                                                                                    \
                                                                                    \
    explicit ClassName(EnumType value);                                             \
                                                                                    \
                                                                                    \
    constexpr ClassName(_E value) : _value(value)  { }                              \
                                                                                    \
                                                                                    \
    constexpr operator _E() const { return _value; }                                \
                                                                                    \
                                                                                    \
    [[nodiscard]] const char* to_string() const;                                    \
                                                                                    \
                                                                                    \
    [[nodiscard]] EnumType to_value() const;                                        \
                                                                                    \
                                                                                    \
    [[nodiscard]] static ClassName from_string(const char* str);                    \
                                                                                    \
                                                                                    \
    [[nodiscard]] static ClassName from_value(EnumType value);                      \
                                                                                    \
private:                                                                            \
    _E _value = _E::NONE;                                                           \
};


#define DEFINE_TYPED_ENUM(ClassName, EnumType, EnumDefs, NoneValue)     \
ClassName::ClassName(const char* str) {                                 \
    EnumDefs(ENUM_CONSTRUCT_FROM_STRING)                                \
}                                                                       \
                                                                        \
                                                                        \
ClassName::ClassName(EnumType value) {                                  \
    EnumDefs(ENUM_CONSTRUCT_FROM_VALUE)                                 \
}                                                                       \
                                                                        \
                                                                        \
const char* ClassName::to_string() const {                              \
    EnumDefs(ENUM_TO_STRING)                                            \
    return "NONE";                                                      \
}                                                                       \
                                                                        \
EnumType ClassName::to_value() const {                                  \
    return (EnumType) _value;                                           \
}                                                                       \
                                                                        \
                                                                        \
ClassName ClassName::from_string(const char* str) {                     \
    EnumDefs(ENUM_FROM_STRING)                                          \
    return ClassName::NONE;                                             \
}                                                                       \
                                                                        \
                                                                        \
ClassName ClassName::from_value(EnumType value) {                       \
    EnumDefs(ENUM_FROM_VALUE)                                           \
    return ClassName::NONE;                                             \
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Enum
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

/**
 * Preprocessor macro black magic that automatically declares a class with a nested typed enum class and conversion
 * functions. The enum will have a "NONE" value the represents the absence of an enum value, the numerical value of
 * "NONE" can be chosen.
 *
 * <p>
 *  The macro parameters are:
 *  <ol>
 *      <li>ClassName: Name of the class.</li>
 *      <li>EnumType:  Type for the nested typed eum class, e.g. uint8_t</li>
 *      <li>EnumDefs:  X Macro with enum definitions, will be explained in the next section.</li>
 *      <li>NoneValue: The numerical value of the "NONE" enum value. Must be >0 as the conversions functions use
 *                      "size_t" as type.</li>
 *  </ol>
 * </p>
 *
 * <p>
 *  The EnumDefs X macro defines a list with all enum values for the nested enum class and requires the following
 *  parameters:
 *  <ol>
 *      <li>ClassName: Name of the class, must match with the one provided to "DECLARE_TYPED_ENUM".</li>
 *      <li>Name:  Name of the enum value.</li>
 *      <li>Value: Numerical value of the enum value.</li>
 *  </ol>
 * </p>
 *
 * <p>
 *  The following conversion (static and member) functions are automatically generated:
 *  <ol>
 *      <li>to_string (Member): The "Name" parameter of the enum value provided to "EnumDefs".</li>
 *      <li>to_value (Member):    The "Value" parameter of the enum value provided to "EnumDefs"</li>
 *      <li>from_string (Static): The enum value whose "Name" matches with the given string else "NONE".</li>
 *      <li>from_value (Static): The enum value whose "Value" matches with the given number else "NONE".</li>
 *  </ol>
 * </p>
 *
 * <p>
 * Usage: <br><br>
 * #define FRUITS(X) \ <br>
 *     X(Fruit, Strawberry, 2)\ <br>
 *     X(Fruit, Banana, 4)\ <br><br>
 *  In header file: <br>
 *  DECLARE_ENUM(Fruit, FRUITS, 0) <br><br>
 *  In Source file: <br>
 *  DEFINE_ENUM(Fruit, FRUITS, 0)
 * </p>
 */
#define DECLARE_ENUM(ClassName, EnumDefs, NoneValue)                                \
class ClassName {                                                                   \
public:                                                                             \
    enum _E {                                                                       \
        NONE = NoneValue,                                                           \
        EnumDefs(ENUM_VALUE)                                                        \
    };                                                                              \
                                                                                    \
                                                                                    \
    ClassName() = default;                                                          \
                                                                                    \
                                                                                    \
    ClassName(const char* str);                                                     \
                                                                                    \
                                                                                    \
    ClassName(size_t value);                                                        \
                                                                                    \
                                                                                    \
    constexpr ClassName(_E value) : _value(value)  { }                              \
                                                                                    \
                                                                                    \
    constexpr operator _E() const { return _value; }                                \
                                                                                    \
                                                                                    \
    [[nodiscard]] const char* to_string() const;                                    \
                                                                                    \
                                                                                    \
    [[nodiscard]] size_t to_value() const;                                          \
                                                                                    \
private:                                                                            \
    _E _value = _E::NONE;                                                           \
};

#define DEFINE_ENUM(ClassName, EnumDefs, NoneValue)                     \
ClassName::ClassName(const char* str) {                                 \
    EnumDefs(ENUM_CONSTRUCT_FROM_STRING)                                \
}                                                                       \
                                                                        \
                                                                        \
ClassName::ClassName(size_t value) {                                    \
    EnumDefs(ENUM_CONSTRUCT_FROM_VALUE)                                 \
}                                                                       \
                                                                        \
                                                                        \
const char* ClassName::to_string() const {                              \
    EnumDefs(ENUM_TO_STRING)                                            \
    return "NONE";                                                      \
}                                                                       \
                                                                        \
size_t ClassName::to_value() const {                                    \
    return (size_t) _value;                                             \
}

#endif //EMBER_ENUM_H
