#pragma once

#include "core/types.hpp"

#include <memory>

namespace huedra {

class JsonObject;
class JsonValue;

using JsonArray = std::vector<JsonValue>;

class JsonValue
{

    union Value
    {
        i32 iNum;
        u32 uNum;
        double dNum;
        bool boolean;
        std::string* str;
        JsonArray* array;
        JsonObject* object;
    };

public:
    enum class Type
    {
        NIL,
        INT,
        UINT,
        FLOAT,
        BOOL,
        STRING,
        ARRAY,
        OBJECT
    };

    JsonValue(JsonObject* parent, Type desiredType = Type::NIL);
    virtual ~JsonValue() = default;
    JsonValue(const JsonValue& rhs) = default;

    JsonValue& operator=(std::nullptr_t null);
    JsonValue& operator=(i32 value);
    JsonValue& operator=(u32 value);
    JsonValue& operator=(double value);
    JsonValue& operator=(bool value);
    JsonValue& operator=(const std::string& value);
    JsonValue& operator=(const char* value);
    JsonValue& operator=(const JsonArray& values);
    JsonValue& operator=(const JsonObject& value);

    i32& asInt();
    u32& asUint();
    double& asFloat();
    bool& asBool();
    std::string& asString();
    JsonArray& asArray();
    JsonObject& asObject();

    template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    JsonValue& operator[](T index)
    {
        return (*this)[static_cast<uint64_t>(index)];
    }

    JsonValue& operator[](u64 index);                     // Only ARRAY type
    JsonValue& operator[](const std::string& identifier); // Only OBJECT type
    JsonValue& operator[](const char* str);               // Only OBJECT type

    Type getType() const { return m_type; }
    JsonObject* getParent() const { return m_parent; }

    void setParent(JsonObject* parent) { m_parent = parent; }

private:
    Type m_type{Type::NIL};
    Value m_value{0};
    JsonObject* m_parent{nullptr};
};

class JsonObject
{
    friend class JsonValue;

public:
    JsonObject() = default;
    virtual ~JsonObject() = default;

    JsonObject(const JsonObject& rhs);
    JsonObject(const JsonObject&& rhs);
    JsonObject& operator=(const JsonObject& rhs);
    JsonObject& operator=(const JsonObject&& rhs);

    JsonValue& operator[](const std::string& identifier);
    bool hasMember(const std::string& identifier) const;

    std::vector<std::string> getMembers() const { return m_keys; }

private:
    std::string* addString(const std::string& value);
    JsonArray* addArray(const JsonArray& values);
    JsonObject* addObject(const JsonObject& value);

    // Collection of pointers to json members with composed of a string/array/object value
    std::vector<std::shared_ptr<std::string>> m_strings;
    std::vector<std::shared_ptr<JsonArray>> m_arrays;
    std::vector<std::shared_ptr<JsonObject>> m_objects;

    std::vector<std::string> m_keys; // Keeping track of insert order
    std::map<std::string, JsonValue> m_members;
};

// TODO: Support \u characters
JsonObject parseJson(const std::vector<u8>& bytes);

std::vector<u8> serializeJson(const JsonObject& json);

} // namespace huedra