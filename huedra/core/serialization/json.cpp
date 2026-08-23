#include "json.hpp"
#include "core/log.hpp"

#include <functional>
#include <iomanip>
#include <sstream>

namespace huedra {

JsonValue::JsonValue(JsonValueType type) : m_type(type)
{
    switch (m_type)
    {
    case JsonValueType::INT:
        m_value = 0LL;
        break;
    case JsonValueType::UINT:
        m_value = 0ULL;
        break;
    case JsonValueType::FLOAT:
        m_value = 0.0;
        break;
    case JsonValueType::BOOL:
        m_value = false;
        break;
    case JsonValueType::STRING:
        m_value = "";
        break;
    case JsonValueType::ARRAY:
        m_value = JsonArray();
        break;
    case JsonValueType::OBJECT:
        m_value = JsonObject();
        break;
    default:
        break;
    }
}

JsonValue& JsonValue::operator=(std::nullptr_t /*null*/)
{
    m_type = JsonValueType::NIL;
    return *this;
}

JsonValue& JsonValue::operator=(i64 value)
{
    m_type = JsonValueType::INT;
    m_value = value;
    return *this;
}

JsonValue& JsonValue::operator=(u64 value)
{
    m_type = JsonValueType::UINT;
    m_value = value;
    return *this;
}

JsonValue& JsonValue::operator=(f64 value)
{
    m_type = JsonValueType::FLOAT;
    m_value = value;
    return *this;
}

JsonValue& JsonValue::operator=(bool value)
{
    m_type = JsonValueType::BOOL;
    m_value = value;
    return *this;
}

JsonValue& JsonValue::operator=(const std::string& value)
{
    m_type = JsonValueType::STRING;
    m_value = value;
    return *this;
}

JsonValue& JsonValue::operator=(const char* value)
{
    m_type = JsonValueType::STRING;
    m_value = std::string(value);
    return *this;
}

JsonValue& JsonValue::operator=(const std::string_view& value)
{
    m_type = JsonValueType::STRING;
    m_value = std::string(value);
    return *this;
}

JsonValue& JsonValue::operator=(const JsonArray& values)
{
    m_type = JsonValueType::ARRAY;
    m_value = values;
    return *this;
}

JsonValue& JsonValue::operator=(const JsonObject& value)
{
    m_type = JsonValueType::OBJECT;
    m_value = value;
    return *this;
}

i64& JsonValue::asInt()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::INT;
        m_value = 0LL;
    }
    return std::get<i64>(m_value);
}

u64& JsonValue::asUint()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::UINT;
        m_value = 0ULL;
    }
    return std::get<u64>(m_value);
}

f64& JsonValue::asFloat()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::FLOAT;
        m_value = 0.0;
    }
    return std::get<f64>(m_value);
}

bool& JsonValue::asBool()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::BOOL;
        m_value = false;
    }
    return std::get<bool>(m_value);
}

std::string& JsonValue::asString()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::STRING;
        m_value = "";
    }
    return std::get<std::string>(m_value);
}

JsonArray& JsonValue::asArray()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::ARRAY;
        m_value = JsonArray({});
    }
    return std::get<JsonArray>(m_value);
}

JsonObject& JsonValue::asObject()
{
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::OBJECT;
        m_value = JsonObject();
    }
    return std::get<JsonObject>(m_value);
}

JsonValue& JsonValue::operator[](u64 index)
{
    static JsonValue invalid;
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::ARRAY;
        m_value = JsonArray({});
    }
    else if (m_type != JsonValueType::ARRAY)
    {
        return invalid;
    }

    JsonArray& arr = std::get<JsonArray>(m_value);
    if (index >= arr.size())
    {
        JsonValueType type = JsonValueType::NIL;
        if (!arr.empty())
        {
            type = arr.back().getType();
        }
        JsonValue value{type};
        arr.resize(index + 1, value);
    }
    return arr[index];
}

JsonValue& JsonValue::operator[](const std::string& identifier)
{
    static JsonValue invalid;
    if (m_type == JsonValueType::NIL)
    {
        m_type = JsonValueType::OBJECT;
        m_value = JsonObject();
    }
    else if (m_type != JsonValueType::OBJECT)
    {
        return invalid;
    }
    return std::get<JsonObject>(m_value)[identifier];
}

JsonValue& JsonValue::operator[](const char* str) { return (*this)[std::string(str)]; }

JsonObject::JsonObject(const JsonObject& rhs) : m_keys(rhs.m_keys), m_members(rhs.m_members)
{
    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
}

JsonObject::JsonObject(const JsonObject&& rhs) : m_keys(rhs.m_keys), m_members(rhs.m_members)
{
    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
}

JsonObject& JsonObject::operator=(const JsonObject& rhs)
{
    if (this == &rhs)
    {
        return *this;
    }

    m_keys = rhs.m_keys;
    m_members = rhs.m_members;

    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
    return *this;
}

JsonObject& JsonObject::operator=(JsonObject&& rhs)
{
    m_keys = rhs.m_keys;
    m_members = rhs.m_members;
    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
    return *this;
}

JsonValue& JsonObject::operator[](const std::string& identifier)
{
    if (!m_members.contains(identifier))
    {
        JsonValue value;
        value.setParent(this);
        m_members.insert(std::pair<std::string, JsonValue>(identifier, value));
        m_keys.push_back(identifier);
    }
    return m_members.at(identifier);
}

bool JsonObject::hasMember(const std::string& identifier) const { return m_members.contains(identifier); }

bool JsonObject::hasMember(const std::string& identifier, JsonValueType type) const
{
    if (!m_members.contains(identifier))
    {
        return false;
    }
    return m_members.at(identifier).getType() == type;
}

JsonObject parseJson(const std::vector<u8>& bytes)
{
    u64 closeIndex = bytes.size();
    for (i64 i = static_cast<i64>(bytes.size()) - 1; i >= 0; --i)
    {
        if (static_cast<char>(bytes[i]) != ' ' && static_cast<char>(bytes[i]) != '\n' &&
            static_cast<char>(bytes[i]) != '\r' && static_cast<char>(bytes[i]) != '\t')
        {
            closeIndex = i;
            break;
        }
    }

    if (bytes.empty() || static_cast<char>(bytes.front()) != '{' || closeIndex == bytes.size() ||
        static_cast<char>(bytes[closeIndex]) != '}')
    {
        log(LogLevel::WARNING, "parseJson(): json data is not encapsulated by an object => {{ ... }}");
        return {};
    }

    JsonObject root;
    std::vector<JsonObject*> curObjects{&root};
    std::vector<JsonArray*> curArrays;
    std::vector<JsonValue*> curValues;
    enum class State
    {
        IN_OBJECT,
        IDENTIFIER_SET,
        ASSIGNMENT_SET,
        VALUE_SET,
        IN_ARRAY,
        ARRAY_VALUE_SET,
        ARRAY_COMMA_SET
    };
    std::vector<State> states{State::IN_OBJECT};

    u64 line = 1;
    u64 lineStart = 1;
    for (u64 i = 1; i < closeIndex; ++i)
    {
        switch (static_cast<char>(bytes[i]))
        {
        case '\"': { // start/end of identifier or string
            ++i;
            std::string str;
            while (i < bytes.size() && static_cast<char>(bytes[i]) != '\"')
            {
                if (static_cast<char>(bytes[i]) == '\\')
                {
                    switch (static_cast<char>(bytes[++i]))
                    {
                    case '\"':
                    case '\\':
                    case '/':
                        str.push_back(static_cast<char>(bytes[i++]));
                        break;
                    case 'b':
                        ++i;
                        str.push_back('\b');
                        break;
                    case 'f':
                        ++i;
                        str.push_back('\f');
                        break;
                    case 'n':
                        ++i;
                        str.push_back('\n');
                        break;
                    case 'r':
                        ++i;
                        str.push_back('\r');
                        break;
                    case 't':
                        ++i;
                        str.push_back('\t');
                        break;
                    case 'u': {
                        std::string hex{static_cast<char>(bytes[++i]), static_cast<char>(bytes[++i]),
                                        static_cast<char>(bytes[++i]), static_cast<char>(bytes[++i])};
                        str.push_back(static_cast<char>(std::stoi(hex, nullptr, 16)));
                    }
                    break;
                    default:
                        log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected control character: \'{}\'", line,
                            i - lineStart, static_cast<char>(bytes[i]));
                        return {};
                    }
                }
                else
                {
                    str.push_back(static_cast<char>(bytes[i++]));
                }
            }
            if (static_cast<char>(bytes[i]) != '\"')
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Could not find closing \" for string/identifier", line,
                    i - lineStart);
                return {};
            }

            switch (states.back())
            {
            case State::IN_OBJECT:
                curValues.push_back(&(*curObjects.back())[str]);
                states.back() = State::IDENTIFIER_SET;
                break;

            case State::ASSIGNMENT_SET:
                *curValues.back() = str;
                states.back() = State::VALUE_SET;
                break;

            case State::IN_ARRAY:
            case State::ARRAY_COMMA_SET: {
                JsonValue& val = curArrays.back()->emplace_back();
                val = str;
                states.back() = State::ARRAY_VALUE_SET;
                break;
            }

            default:
                char expected = states.back() == State::IDENTIFIER_SET ? ':' : ',';
                log(LogLevel::WARNING, R"(parseJson(): ({}, {}) Found unexpected string value: "{}", expected '{}')",
                    line, i - lineStart, str.c_str(), expected);
                return {};
            }
            break;
        }

        case ':': // assignment to member
            if (states.back() != State::IDENTIFIER_SET)
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected \':\', no identifier defined", line,
                    i - lineStart);
                return {};
            }
            states.back() = State::ASSIGNMENT_SET;
            break;

        case ',': // end of member or array element
            if (states.back() == State::ARRAY_VALUE_SET)
            {
                states.back() = State::ARRAY_COMMA_SET;
            }
            else if (states.back() == State::VALUE_SET)
            {
                states.back() = State::IN_OBJECT;
                curValues.pop_back();
            }
            else
            {
                log(LogLevel::WARNING,
                    "parseJson(): ({}, {}) Unexpected \',\', no value has been set in identifier or array", line,
                    i - lineStart);
                return {};
            }
            break;

        case '[': // start of array
            if (states.back() == State::ASSIGNMENT_SET)
            {
                *curValues.back() = JsonArray();
                curArrays.push_back(&curValues.back()->asArray());
                states.push_back(State::IN_ARRAY);
            }
            else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
            {
                JsonValue& val = curArrays.back()->emplace_back();
                val = JsonArray();
                curArrays.push_back(&val.asArray());
                states.push_back(State::IN_ARRAY);
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected \'[\', no identifier or array defined", line,
                    i - lineStart);
                return {};
            }
            break;

        case ']': // end of array
            if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_VALUE_SET)
            {
                states.pop_back();
                curArrays.pop_back();
                if (states.back() == State::ASSIGNMENT_SET)
                {
                    states.back() = State::VALUE_SET;
                }
                else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
                {
                    states.back() = State::ARRAY_VALUE_SET;
                }
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected \']\'", line, i - lineStart);
                return {};
            }
            break;

        case '{': // start of object
            if (states.back() == State::ASSIGNMENT_SET)
            {
                *curValues.back() = JsonObject();
                curObjects.push_back(&curValues.back()->asObject());
                states.push_back(State::IN_OBJECT);
            }
            else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
            {
                JsonValue& val = curArrays.back()->emplace_back();
                val = JsonObject();
                curObjects.push_back(&val.asObject());
                states.push_back(State::IN_OBJECT);
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected \'{{\', no identifier or array defined", line,
                    i - lineStart);
                return {};
            }
            break;

        case '}': // end of object
            if (states.back() == State::IN_OBJECT || states.back() == State::VALUE_SET)
            {
                states.pop_back();
                curObjects.pop_back();
                if (states.back() == State::ASSIGNMENT_SET)
                {
                    states.back() = State::VALUE_SET;
                }
                else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
                {
                    states.back() = State::ARRAY_VALUE_SET;
                }
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected \'}}\'", line, i - lineStart);
                return {};
            }
            break;

        default:
            // Whitspace
            if (static_cast<char>(bytes[i]) == '\n')
            {
                ++line;
                lineStart = i;
                break;
            }
            else if (static_cast<char>(bytes[i]) == ' ' || static_cast<char>(bytes[i]) == '\r' ||
                     static_cast<char>(bytes[i]) == '\t')
            {
                break;
            }

            // Keywords: true, false, null
            if (static_cast<char>(bytes[i]) >= 'a' && static_cast<char>(bytes[i]) <= 'z')
            {
                if (states.back() != State::ASSIGNMENT_SET)
                {
                    log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected character: \'{}\'", line, i - lineStart,
                        static_cast<char>(bytes[i]));
                    return {};
                }

                std::string buf(1, static_cast<char>(bytes[i++]));
                while (static_cast<char>(bytes[i]) >= 'a' && static_cast<char>(bytes[i]) <= 'z')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }

                if (buf == "true")
                {
                    *curValues.back() = true;
                }
                else if (buf == "false")
                {
                    *curValues.back() = false;
                }
                else if (buf == "null")
                {
                    curValues.back() = nullptr;
                }
                else
                {
                    log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected keyword: \"{}\"", line, i - lineStart,
                        buf.c_str());
                    return {};
                }
                --i;
                states.back() = State::VALUE_SET;
                break;
            }

            std::string buf;
            JsonValueType type = JsonValueType::UINT;
            if (static_cast<char>(bytes[i]) == '-')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                type = JsonValueType::INT;
            }

            // Number
            if (static_cast<char>(bytes[i]) == '0')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
            }
            else if (static_cast<char>(bytes[i]) >= '1' && static_cast<char>(bytes[i]) <= '9')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                while (static_cast<char>(bytes[i]) >= '0' && static_cast<char>(bytes[i]) <= '9')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): ({}, {}) Unexpected character: \'{}\'", line, i - lineStart,
                    static_cast<char>(bytes[i]));
                return {};
            }

            // Fraction
            if (static_cast<char>(bytes[i]) == '.')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                type = JsonValueType::FLOAT;
                if (static_cast<char>(bytes[i]) < '0' || static_cast<char>(bytes[i]) > '9')
                {
                    log(LogLevel::WARNING, "parseJson(): ({}, {}) No number defined in fraction", line, i - lineStart);
                    return {};
                }
                while (static_cast<char>(bytes[i]) >= '0' && static_cast<char>(bytes[i]) <= '9')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }
            }

            // Exponent
            if (static_cast<char>(bytes[i]) == 'E' || static_cast<char>(bytes[i]) == 'e')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                type = JsonValueType::FLOAT;
                if (static_cast<char>(bytes[i]) == '-' || static_cast<char>(bytes[i]) == '+')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }

                if (static_cast<char>(bytes[i]) < '0' || static_cast<char>(bytes[i]) > '9')
                {
                    log(LogLevel::WARNING, "parseJson(): ({}, {}) No number defined in exponent", line, i - lineStart);
                    return {};
                }
                while (static_cast<char>(bytes[i]) >= '0' && static_cast<char>(bytes[i]) <= '9')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }
            }

            if (states.back() == State::ASSIGNMENT_SET)
            {
                if (type == JsonValueType::UINT)
                {
                    *curValues.back() = static_cast<u32>(std::stoul(buf));
                }
                else if (type == JsonValueType::INT)
                {
                    *curValues.back() = static_cast<i32>(std::stol(buf));
                }
                else if (type == JsonValueType::FLOAT)
                {
                    *curValues.back() = std::stod(buf);
                }
                --i;
                states.back() = State::VALUE_SET;
            }
            else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
            {
                JsonValue& val = curArrays.back()->emplace_back();
                if (type == JsonValueType::UINT)
                {
                    val = static_cast<u32>(std::stoul(buf));
                }
                else if (type == JsonValueType::INT)
                {
                    val = static_cast<i32>(std::stoul(buf));
                }
                else if (type == JsonValueType::FLOAT)
                {
                    val = std::stod(buf);
                }
                --i;
                states.back() = State::ARRAY_VALUE_SET;
            }
            else
            {
                log(LogLevel::WARNING,
                    "parseJson(): ({}, {}) Unexpected number: {}, not setting identifier/array value", line,
                    i - lineStart, buf.c_str());
                return {};
            }
            break;
        }
    }

    return root;
}

std::vector<u8> serializeJson(const JsonObject& json)
{
    std::vector<u8> bytes;

    std::function<void(JsonValue&, u32)> serializeValue;

    auto serializeObject = [&](JsonObject& object, u32 level = 1) {
        std::vector<std::string> members = object.getMembers();
        bytes.push_back('{');
        if (!members.empty())
        {
            bytes.push_back('\n');
            for (u64 i = 0; i < members.size(); ++i)
            {
                bytes.resize(bytes.size() + static_cast<u64>(4 * level), ' ');
                bytes.push_back('\"');
                for (auto& c : members[i])
                {
                    switch (c)
                    {
                    case '\"':
                        bytes.push_back('\\');
                        bytes.push_back('\"');
                        break;
                    case '\\':
                        bytes.push_back('\\');
                        bytes.push_back('\\');
                        break;
                    case '\b':
                        bytes.push_back('\\');
                        bytes.push_back('b');
                        break;
                    case '\f':
                        bytes.push_back('\\');
                        bytes.push_back('f');
                        break;
                    case '\n':
                        bytes.push_back('\\');
                        bytes.push_back('n');
                        break;
                    case '\r':
                        bytes.push_back('\\');
                        bytes.push_back('r');
                        break;
                    case '\t':
                        bytes.push_back('\\');
                        bytes.push_back('t');
                        break;
                    default:
                        bytes.push_back(c);
                        break;
                    }
                }
                bytes.push_back('\"');
                bytes.push_back(':');
                bytes.push_back(' ');

                serializeValue(object[members[i]], level);

                if (i != members.size() - 1)
                {
                    bytes.push_back(',');
                }
                bytes.push_back('\n');
            }
            bytes.resize(bytes.size() + static_cast<u64>(4 * (level - 1)), ' ');
        }
        bytes.push_back('}');
    };

    serializeValue = [&](JsonValue& value, u32 level) {
        std::string str;
        switch (value.getType())
        {
        case JsonValueType::NIL:
            bytes.push_back('n');
            bytes.push_back('u');
            bytes.push_back('l');
            bytes.push_back('l');
            break;
        case JsonValueType::INT:
            str = std::to_string(value.asInt());
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValueType::UINT:
            str = std::to_string(value.asUint());
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValueType::FLOAT: {
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<f64>::digits10 + 1) << value.asFloat();
            str = oss.str();
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        }
        case JsonValueType::BOOL:
            str = value.asBool() ? "true" : "false";
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValueType::STRING:
            str = value.asString();
            bytes.push_back('\"');
            for (auto& c : str)
            {
                switch (c)
                {
                case '\"':
                    bytes.push_back('\\');
                    bytes.push_back('\"');
                    break;
                case '\\':
                    bytes.push_back('\\');
                    bytes.push_back('\\');
                    break;
                case '\b':
                    bytes.push_back('\\');
                    bytes.push_back('b');
                    break;
                case '\f':
                    bytes.push_back('\\');
                    bytes.push_back('f');
                    break;
                case '\n':
                    bytes.push_back('\\');
                    bytes.push_back('n');
                    break;
                case '\r':
                    bytes.push_back('\\');
                    bytes.push_back('r');
                    break;
                case '\t':
                    bytes.push_back('\\');
                    bytes.push_back('t');
                    break;
                default:
                    bytes.push_back(c);
                    break;
                }
            }
            bytes.push_back('\"');
            break;
        case JsonValueType::ARRAY: {
            bytes.push_back('[');
            JsonArray& array = value.asArray();
            if (!array.empty())
            {
                bytes.push_back('\n');
                for (u64 i = 0; i < array.size(); ++i)
                {
                    bytes.resize(bytes.size() + static_cast<u64>(4 * (level + 1)), ' ');
                    serializeValue(value[i], level + 1);
                    if (i != array.size() - 1)
                    {
                        bytes.push_back(',');
                    }
                    bytes.push_back('\n');
                }
                bytes.resize(bytes.size() + static_cast<u64>(4 * level), ' ');
            }
            bytes.push_back(']');
            break;
        }
        case JsonValueType::OBJECT:
            serializeObject(value.asObject(), level + 1);
            break;
        }
    };

    // Const cast is used here since functions of getting members are not const. Since the values aren't altered
    // in the serilization, it will still be handled as const. This could also be fixed by having json as non const
    // parameter but since it signifies to the user that the data will not be altered, this is preferred.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    serializeObject(const_cast<JsonObject&>(json));

    return bytes;
}

} // namespace huedra