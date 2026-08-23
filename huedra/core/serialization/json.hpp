#pragma once

#include "core/types.hpp"

#include <memory>
#include <variant>

namespace huedra {

class JsonValue;
using JsonArray = std::vector<JsonValue>;

enum class JsonValueType
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

class JsonObject
{
    friend class JsonValue;

public:
    JsonObject() = default;
    virtual ~JsonObject() = default;

    JsonObject(const JsonObject& rhs);
    JsonObject(const JsonObject&& rhs);
    JsonObject& operator=(const JsonObject& rhs);
    JsonObject& operator=(JsonObject&& rhs);

    JsonValue& operator[](const std::string& identifier);
    bool hasMember(const std::string& identifier) const;
    bool hasMember(const std::string& identifier, JsonValueType type) const;

    std::vector<std::string> getMembers() const { return m_keys; }

private:
    std::vector<std::string> m_keys; // Keeping track of insert order
    std::map<std::string, JsonValue> m_members;
};

class JsonValue
{
public:
    explicit JsonValue(JsonValueType type = JsonValueType::NIL);
    virtual ~JsonValue() = default;

    JsonValue(const JsonValue& rhs) = default;
    JsonValue& operator=(const JsonValue& rhs) = default;
    JsonValue(JsonValue&& rhs) = default;
    JsonValue& operator=(JsonValue&& rhs) = default;

    template <typename T>
        requires std::is_integral_v<T> && std::is_signed_v<T>
    JsonValue& operator=(T value)
    {
        return (*this) = static_cast<i64>(value);
    }

    template <typename T>
        requires std::is_integral_v<T> && std::is_unsigned_v<T>
    JsonValue& operator=(T value)
    {
        return (*this) = static_cast<u64>(value);
    }

    JsonValue& operator=(std::nullptr_t null);
    JsonValue& operator=(i64 value);
    JsonValue& operator=(u64 value);
    JsonValue& operator=(f64 value);
    JsonValue& operator=(bool value);
    JsonValue& operator=(const std::string& value);
    JsonValue& operator=(const char* value);
    JsonValue& operator=(const std::string_view& value);
    JsonValue& operator=(const JsonArray& values);
    JsonValue& operator=(const JsonObject& value);

    i64& asInt();
    u64& asUint();
    f64& asFloat();
    bool& asBool();
    std::string& asString();
    JsonArray& asArray();
    JsonObject& asObject();

    template <typename T>
        requires std::is_integral_v<T>
    JsonValue& operator[](T index)
    {
        return (*this)[static_cast<u64>(index)];
    }

    JsonValue& operator[](u64 index);                     // Only ARRAY type
    JsonValue& operator[](const std::string& identifier); // Only OBJECT type
    JsonValue& operator[](const char* str);               // Only OBJECT type

    JsonValueType getType() const { return m_type; }
    JsonObject* getParent() const { return m_parent; }

    void setParent(JsonObject* parent) { m_parent = parent; }

private:
    JsonValueType m_type{JsonValueType::NIL};
    std::variant<i64, u64, f64, bool, std::string, JsonArray, JsonObject> m_value{0ULL};
    JsonObject* m_parent{nullptr};
};

// TODO: Support \u characters
JsonObject parseJson(const std::vector<u8>& bytes);
std::vector<u8> serializeJson(const JsonObject& json);

} // namespace huedra