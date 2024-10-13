#include "json.hpp"

#include "core/log.hpp"

#include <functional>
#include <iomanip>
#include <sstream>

namespace huedra {

JsonValue::JsonValue(JsonObject* parent, Type desiredType) : m_parent(parent), m_type(desiredType)
{
    if (m_type == Type::STRING)
    {
        m_value.str = m_parent->addString("");
    }
    else if (m_type == Type::ARRAY)
    {
        m_value.array = m_parent->addArray({});
    }
    else if (m_type == Type::OBJECT)
    {
        m_value.object = m_parent->addObject(JsonObject());
    }
}

JsonValue& JsonValue::operator=(std::nullptr_t null)
{
    m_type = Type::NIL;
    return *this;
}

JsonValue& JsonValue::operator=(i32 value)
{
    m_type = Type::INT;
    m_value.iNum = value;
    return *this;
}

JsonValue& JsonValue::operator=(u32 value)
{
    m_type = Type::UINT;
    m_value.uNum = value;
    return *this;
}

JsonValue& JsonValue::operator=(double value)
{
    m_type = Type::FLOAT;
    m_value.dNum = value;
    return *this;
}

JsonValue& JsonValue::operator=(bool value)
{
    m_type = Type::BOOL;
    m_value.boolean = value;
    return *this;
}

JsonValue& JsonValue::operator=(const std::string& value)
{
    m_type = Type::STRING;
    m_value.str = m_parent->addString(value);
    return *this;
}

JsonValue& JsonValue::operator=(const char* value)
{
    m_type = Type::STRING;
    m_value.str = m_parent->addString(std::string(value));
    return *this;
}

JsonValue& JsonValue::operator=(const JsonArray& values)
{
    m_type = Type::ARRAY;
    m_value.array = m_parent->addArray(values);
    return *this;
}

JsonValue& JsonValue::operator=(const JsonObject& value)
{
    m_type = Type::OBJECT;
    m_value.object = m_parent->addObject(value);
    return *this;
}

i32& JsonValue::asInt()
{
    if (m_type != Type::INT)
    {
        log(LogLevel::ERR, "json value can't be accessed as i32");
    }
    return m_value.iNum;
}

u32& JsonValue::asUint()
{
    if (m_type != Type::UINT)
    {
        log(LogLevel::ERR, "json value can't be accessed as u32");
    }
    return m_value.uNum;
}

double& JsonValue::asFloat()
{
    if (m_type != Type::FLOAT)
    {
        log(LogLevel::ERR, "json value can't be accessed as double");
    }
    return m_value.dNum;
}

bool& JsonValue::asBool()
{
    if (m_type != Type::BOOL)
    {
        log(LogLevel::ERR, "json value can't be accessed as bool");
    }
    return m_value.boolean;
}

std::string& JsonValue::asString()
{
    if (m_type != Type::STRING)
    {
        log(LogLevel::ERR, "json value can't be accessed as string");
    }
    return *m_value.str;
}

JsonArray& JsonValue::asArray()
{
    if (m_type != Type::ARRAY)
    {
        log(LogLevel::ERR, "json value can't be accessed as array");
    }
    return *m_value.array;
}

JsonObject& JsonValue::asObject()
{
    if (m_type != Type::OBJECT)
    {
        log(LogLevel::ERR, "json value can't be accessed as object");
    }
    return *m_value.object;
}

JsonValue& JsonValue::operator[](u64 index)
{
    static JsonValue invalid(nullptr);
    if (m_type != Type::ARRAY)
    {
        log(LogLevel::ERR, "Can't return at index: %llu of json member, not an array", index);
        return invalid;
    }
    if (index >= m_value.array->size())
    {
        Type type = Type::NIL;
        if (!m_value.array->empty())
        {
            type = m_value.array->back().getType();
        }
        m_value.array->resize(index + 1, JsonValue(m_parent, type));
    }
    return (*m_value.array)[index];
}

JsonValue& JsonValue::operator[](const std::string& identifier)
{
    static JsonValue invalid(nullptr);
    if (m_type != Type::OBJECT)
    {
        log(LogLevel::ERR, "Can't return with identifier: %s of json member, not an object", identifier.c_str());
        return invalid;
    }
    return (*m_value.object)[identifier];
}

JsonValue& JsonValue::operator[](const char* str) { return (*this)[std::string(str)]; }

JsonObject::JsonObject(const JsonObject& rhs)
{
    m_strings = rhs.m_strings;
    m_arrays = rhs.m_arrays;
    m_objects = rhs.m_objects;
    m_keys = rhs.m_keys;
    m_members = rhs.m_members;

    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
}

JsonObject::JsonObject(const JsonObject&& rhs)
{
    m_strings = rhs.m_strings;
    m_arrays = rhs.m_arrays;
    m_objects = rhs.m_objects;
    m_keys = rhs.m_keys;
    m_members = rhs.m_members;

    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
}

JsonObject& JsonObject::operator=(const JsonObject& rhs)
{
    m_strings = rhs.m_strings;
    m_arrays = rhs.m_arrays;
    m_objects = rhs.m_objects;
    m_keys = rhs.m_keys;
    m_members = rhs.m_members;

    for (auto& [key, value] : m_members)
    {
        value.setParent(this);
    }
    return *this;
}

JsonObject& JsonObject::operator=(const JsonObject&& rhs)
{
    m_strings = rhs.m_strings;
    m_arrays = rhs.m_arrays;
    m_objects = rhs.m_objects;
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
        m_members.insert(std::pair<std::string, JsonValue>(identifier, JsonValue(this)));
        m_keys.push_back(identifier);
    }
    return m_members.at(identifier);
}

bool JsonObject::hasMember(const std::string& identifier) const { return m_members.contains(identifier); }

std::string* JsonObject::addString(const std::string& value)
{
    m_strings.push_back(std::make_shared<std::string>(value));
    return m_strings.back().get();
}

JsonArray* JsonObject::addArray(const JsonArray& values)
{
    m_arrays.push_back(std::make_shared<JsonArray>(values));
    return m_arrays.back().get();
}

JsonObject* JsonObject::addObject(const JsonObject& value)
{
    m_objects.push_back(std::make_shared<JsonObject>(value));
    return m_objects.back().get();
}

JsonObject parseJson(const std::vector<u8>& bytes)
{
    if (bytes.front() != '{' || bytes.back() != '}')
    {
        log(LogLevel::WARNING, "parseJson(): json data is not encapsulated by an object => { ... }");
        return JsonObject();
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
    for (u64 i = 1; i < bytes.size() - 1; ++i)
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
                        log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected control character: \'%c\'", line,
                            i - lineStart, static_cast<char>(bytes[i]));
                        return JsonObject();
                    }
                }
                else
                {
                    str.push_back(static_cast<char>(bytes[i++]));
                }
            }
            if (static_cast<char>(bytes[i]) != '\"')
            {
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Could not find closing \" for string/identifier",
                    line, i - lineStart);
                return JsonObject();
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
                JsonValue& val = curArrays.back()->emplace_back(curValues.back()->getParent());
                val = str;
                states.back() = State::ARRAY_VALUE_SET;
                break;
            }

            default:
                char expected = states.back() == State::IDENTIFIER_SET ? ':' : ',';
                log(LogLevel::WARNING,
                    "parseJson(): (%llu, %llu) Found unexpected string value: \"%s\", expected \'%c\'", line,
                    i - lineStart, str.c_str(), expected);
                return JsonObject();
            }
            break;
        }

        case ':': // assignment to member
            if (states.back() != State::IDENTIFIER_SET)
            {
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected \':\', no identifier defined", line,
                    i - lineStart);
                return JsonObject();
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
                    "parseJson(): (%llu, %llu) Unexpected \',\', no value has been set in identifier or array", line,
                    i - lineStart);
                return JsonObject();
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
                JsonValue& val = curArrays.back()->emplace_back(curValues.back()->getParent());
                val = JsonArray();
                curArrays.push_back(&val.asArray());
                states.push_back(State::IN_ARRAY);
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected \'[\', no identifier or array defined",
                    line, i - lineStart);
                return JsonObject();
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
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected \']\'", line, i - lineStart);
                return JsonObject();
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
                JsonValue& val = curArrays.back()->emplace_back(curValues.back()->getParent());
                val = JsonObject();
                curObjects.push_back(&val.asObject());
                states.push_back(State::IN_OBJECT);
            }
            else
            {
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected \'{\', no identifier or array defined",
                    line, i - lineStart);
                return JsonObject();
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
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected \'}\'", line, i - lineStart);
                return JsonObject();
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
                    log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected character: \'%c\'", line,
                        i - lineStart, static_cast<char>(bytes[i]));
                    return JsonObject();
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
                    log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected keyword: \"%s\"", line, i - lineStart,
                        buf.c_str());
                    return JsonObject();
                }
                --i;
                states.back() = State::VALUE_SET;
                break;
            }

            std::string buf;
            JsonValue::Type type = JsonValue::Type::UINT;
            if (static_cast<char>(bytes[i]) == '-')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                type = JsonValue::Type::INT;
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
                log(LogLevel::WARNING, "parseJson(): (%llu, %llu) Unexpected character: \'%c\'", line, i - lineStart,
                    static_cast<char>(bytes[i]));
                return JsonObject();
            }

            // Fraction
            if (static_cast<char>(bytes[i]) == '.')
            {
                buf.push_back(static_cast<char>(bytes[i++]));
                type = JsonValue::Type::FLOAT;
                if (static_cast<char>(bytes[i]) < '0' || static_cast<char>(bytes[i]) > '9')
                {
                    log(LogLevel::WARNING, "parseJson(): (%llu, %llu) No number defined in fraction", line,
                        i - lineStart);
                    return JsonObject();
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
                type = JsonValue::Type::FLOAT;
                if (static_cast<char>(bytes[i]) == '-' || static_cast<char>(bytes[i]) == '+')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }

                if (static_cast<char>(bytes[i]) < '0' || static_cast<char>(bytes[i]) > '9')
                {
                    log(LogLevel::WARNING, "parseJson(): (%llu, %llu) No number defined in exponent", line,
                        i - lineStart);
                    return JsonObject();
                }
                while (static_cast<char>(bytes[i]) >= '0' && static_cast<char>(bytes[i]) <= '9')
                {
                    buf.push_back(static_cast<char>(bytes[i++]));
                }
            }

            if (states.back() == State::ASSIGNMENT_SET)
            {
                if (type == JsonValue::Type::UINT)
                {
                    *curValues.back() = static_cast<u32>(std::stoul(buf));
                }
                else if (type == JsonValue::Type::INT)
                {
                    *curValues.back() = static_cast<i32>(std::stol(buf));
                }
                else if (type == JsonValue::Type::FLOAT)
                {
                    *curValues.back() = std::stod(buf);
                }
                --i;
                states.back() = State::VALUE_SET;
            }
            else if (states.back() == State::IN_ARRAY || states.back() == State::ARRAY_COMMA_SET)
            {
                JsonValue& val = curArrays.back()->emplace_back(curValues.back()->getParent());
                if (type == JsonValue::Type::UINT)
                {
                    val = static_cast<u32>(std::stoul(buf));
                }
                else if (type == JsonValue::Type::INT)
                {
                    val = static_cast<i32>(std::stoul(buf));
                }
                else if (type == JsonValue::Type::FLOAT)
                {
                    val = std::stod(buf);
                }
                --i;
                states.back() = State::ARRAY_VALUE_SET;
            }
            else
            {
                log(LogLevel::WARNING,
                    "parseJson(): (%llu, %llu) Unexpected number: %s, not setting identifier/array value", line,
                    i - lineStart, buf.c_str());
                return JsonObject();
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
                bytes.resize(bytes.size() + 4 * level, ' ');
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
            bytes.resize(bytes.size() + 4 * (level - 1), ' ');
        }
        bytes.push_back('}');
    };

    serializeValue = [&](JsonValue& value, u32 level) {
        std::string str;
        switch (value.getType())
        {
        case JsonValue::Type::NIL:
            bytes.push_back('n');
            bytes.push_back('u');
            bytes.push_back('l');
            bytes.push_back('l');
            break;
        case JsonValue::Type::INT:
            str = std::to_string(value.asInt());
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValue::Type::UINT:
            str = std::to_string(value.asUint());
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValue::Type::FLOAT: {
            std::ostringstream oss;
            oss << std::setprecision(std::numeric_limits<double>::digits10 + 1) << value.asFloat();
            str = oss.str();
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        }
        case JsonValue::Type::BOOL:
            str = value.asBool() ? "true" : "false";
            for (auto& c : str)
            {
                bytes.push_back(c);
            }
            break;
        case JsonValue::Type::STRING:
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
        case JsonValue::Type::ARRAY: {
            bytes.push_back('[');
            JsonArray& array = value.asArray();
            if (!array.empty())
            {
                bytes.push_back('\n');
                for (u64 i = 0; i < array.size(); ++i)
                {
                    bytes.resize(bytes.size() + 4 * (level + 1), ' ');
                    serializeValue(value[i], level + 1);
                    if (i != array.size() - 1)
                    {
                        bytes.push_back(',');
                    }
                    bytes.push_back('\n');
                }
                bytes.resize(bytes.size() + 4 * level, ' ');
            }
            bytes.push_back(']');
            break;
        }
        case JsonValue::Type::OBJECT:
            serializeObject(value.asObject(), level + 1);
            break;
        }
    };

    serializeObject(const_cast<JsonObject&>(json));

    return bytes;
}

} // namespace huedra