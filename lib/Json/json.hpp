#ifndef JSON_HPP
#define JSON_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace json_ {
// Forward declarations
class Value;
using Array = std::vector<Value>;
using Object = std::map<std::string, Value>;

// Forward declaration for pretty-print configuration
struct PrettyPrintConfig;

// Forward declarations for iterators
class ObjectIterator;
class ArrayIterator;

// JSON value class
class Value {
public:
    // Value types
    using variant_type =
        std::variant<std::nullptr_t, bool, int64_t, double, std::string,
                     std::shared_ptr<Array>, std::shared_ptr<Object>>;

    // Constructors
    Value() : m_value(nullptr) {}
    Value(std::nullptr_t) : m_value(nullptr) {}
    Value(bool value) : m_value(value) {}
    Value(int value) : m_value(static_cast<int64_t>(value)) {}
    Value(int64_t value) : m_value(value) {}
    Value(long long value) : m_value(static_cast<int64_t>(value)) {}
    Value(double value) : m_value(value) {}
    Value(const std::string &value) : m_value(value) {}
    Value(const char *value) : m_value(std::string(value)) {}
    Value(const Array &array) : m_value(std::make_shared<Array>(array)) {}
    Value(const Object &object) : m_value(std::make_shared<Object>(object)) {}

    // STL container support
    template <typename T, typename Alloc>
    Value(const std::vector<T, Alloc> &vec) {
        Array array;
        for (const auto &item : vec) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename T, typename Alloc>
    Value(const std::list<T, Alloc> &list) {
        Array array;
        for (const auto &item : list) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename T, typename Alloc>
    Value(const std::deque<T, Alloc> &deque) {
        Array array;
        for (const auto &item : deque) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename T, size_t N>
    Value(const std::array<T, N> &arr) {
        Array array;
        for (const auto &item : arr) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename T, typename Compare, typename Alloc>
    Value(const std::set<T, Compare, Alloc> &set) {
        Array array;
        for (const auto &item : set) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename T, typename Hash, typename Equal, typename Alloc>
    Value(const std::unordered_set<T, Hash, Equal, Alloc> &uset) {
        Array array;
        for (const auto &item : uset) {
            array.push_back(Value(item));
        }
        m_value = std::make_shared<Array>(array);
    }

    template <typename K, typename V, typename Compare, typename Alloc>
    Value(const std::map<K, V, Compare, Alloc> &map) {
        Object object;
        for (const auto &[key, value] : map) {
            object[std::to_string(key)] = Value(value);
        }
        m_value = std::make_shared<Object>(object);
    }

    template <typename K, typename V, typename Hash, typename Equal,
              typename Alloc>
    Value(const std::unordered_map<K, V, Hash, Equal, Alloc> &umap) {
        Object object;
        for (const auto &[key, value] : umap) {
            object[std::to_string(key)] = Value(value);
        }
        m_value = std::make_shared<Object>(object);
    }

    // Special case for string keys
    template <typename V, typename Compare, typename Alloc>
    Value(const std::map<std::string, V, Compare, Alloc> &map) {
        Object object;
        for (const auto &[key, value] : map) {
            object[key] = Value(value);
        }
        m_value = std::make_shared<Object>(object);
    }

    template <typename V, typename Hash, typename Equal, typename Alloc>
    Value(const std::unordered_map<std::string, V, Hash, Equal, Alloc> &umap) {
        Object object;
        for (const auto &[key, value] : umap) {
            object[key] = Value(value);
        }
        m_value = std::make_shared<Object>(object);
    }

    // Type checking
    bool is_null() const {
        return std::holds_alternative<std::nullptr_t>(m_value);
    }
    bool is_bool() const {
        return std::holds_alternative<bool>(m_value);
    }
    bool is_int() const {
        return std::holds_alternative<int64_t>(m_value);
    }
    bool is_double() const {
        return std::holds_alternative<double>(m_value);
    }
    bool is_string() const {
        return std::holds_alternative<std::string>(m_value);
    }
    bool is_array() const {
        return std::holds_alternative<std::shared_ptr<Array>>(m_value);
    }
    bool is_object() const {
        return std::holds_alternative<std::shared_ptr<Object>>(m_value);
    }

    // Value accessors
    bool as_bool() const {
        if (!is_bool()) throw std::runtime_error("Value is not a boolean");
        return std::get<bool>(m_value);
    }

    int64_t as_int() const {
        if (!is_int()) throw std::runtime_error("Value is not an integer");
        return std::get<int64_t>(m_value);
    }

    double as_double() const {
        if (is_double()) return std::get<double>(m_value);
        if (is_int()) return static_cast<double>(std::get<int64_t>(m_value));
        throw std::runtime_error("Value is not a number");
    }

    const std::string &as_string() const {
        if (!is_string()) throw std::runtime_error("Value is not a string");
        return std::get<std::string>(m_value);
    }

    const Array &as_array() const {
        if (!is_array()) throw std::runtime_error("Value is not an array");
        return *std::get<std::shared_ptr<Array>>(m_value);
    }

    Array &as_array() {
        if (!is_array()) throw std::runtime_error("Value is not an array");
        return *std::get<std::shared_ptr<Array>>(m_value);
    }

    const Object &as_object() const {
        if (!is_object()) throw std::runtime_error("Value is not an object");
        return *std::get<std::shared_ptr<Object>>(m_value);
    }

    Object &as_object() {
        if (!is_object()) throw std::runtime_error("Value is not an object");
        return *std::get<std::shared_ptr<Object>>(m_value);
    }

    // Array and object accessors
    Value &operator[](size_t index) {
        if (!is_array()) throw std::runtime_error("Value is not an array");
        auto &array = *std::get<std::shared_ptr<Array>>(m_value);
        if (index >= array.size())
            throw std::runtime_error("Array index out of bounds");
        return array[index];
    }

    Value &operator[](const std::string &key) {
        if (!is_object()) throw std::runtime_error("Value is not an object");
        auto &object = *std::get<std::shared_ptr<Object>>(m_value);
        return object[key];  // This creates a new entry if key doesn't exist
    }

    // Add this const version of operator[] for the array access
    const Value &operator[](size_t index) const {
        if (!is_array()) throw std::runtime_error("Value is not an array");
        auto &array = *std::get<std::shared_ptr<Array>>(m_value);
        if (index >= array.size())
            throw std::runtime_error("Array index out of bounds");
        return array[index];
    }

    // Add this const version of operator[] for the object access
    const Value &operator[](const std::string &key) const {
        if (!is_object()) throw std::runtime_error("Value is not an object");
        auto &object = *std::get<std::shared_ptr<Object>>(m_value);
        if (object.find(key) == object.end()) {
            throw std::runtime_error("Key not found: " + key);
        }
        return object.at(key);
    }

    // Add this to your Value class in json.hpp
    size_t size() const {
        if (is_array()) {
            return as_array().size();
        } else if (is_object()) {
            return as_object().size();
        }
        return 0;  // Non-container types have size 0
    }

    // Iterator support
    // Array iterators
    ::json_::ArrayIterator begin_array();
    ::json_::ArrayIterator end_array();
    ::json_::ArrayIterator begin_array() const;
    ::json_::ArrayIterator end_array() const;

    // Object iterators
    ::json_::ObjectIterator begin_object();
    ::json_::ObjectIterator end_object();
    ::json_::ObjectIterator begin_object() const;
    ::json_::ObjectIterator end_object() const;

    // Generic begin/end that work with range-based for loops
    template <typename T = void>
    auto begin() {
        if (is_array()) return begin_array();
        if (is_object()) return begin_object();
        throw std::runtime_error("Value is neither an array nor an object");
    }

    template <typename T = void>
    auto end() {
        if (is_array()) return end_array();
        if (is_object()) return end_object();
        throw std::runtime_error("Value is neither an array nor an object");
    }

    template <typename T = void>
    auto begin() const {
        if (is_array()) return begin_array();
        if (is_object()) return begin_object();
        throw std::runtime_error("Value is neither an array nor an object");
    }

    template <typename T = void>
    auto end() const {
        if (is_array()) return end_array();
        if (is_object()) return end_object();
        throw std::runtime_error("Value is neither an array nor an object");
    }

    // Serialization
    std::string to_string() const;
    std::string to_string(const PrettyPrintConfig &config) const;

private:
    variant_type m_value;

    // Helper functions for pretty printing
    void to_string_pretty(std::ostringstream &oss,
                          const PrettyPrintConfig &config, int depth) const;
    void format_string(std::ostringstream &oss, const std::string &str) const;
};

// Iterator implementation for Array
class ArrayIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Value;
    using difference_type = std::ptrdiff_t;
    using pointer = Value *;
    using reference = Value &;

    ArrayIterator() : array_ptr(nullptr), index(0) {}
    ArrayIterator(Array *array, size_t idx) : array_ptr(array), index(idx) {}

    reference operator*() const {
        return (*array_ptr)[index];
    }
    pointer operator->() const {
        return &((*array_ptr)[index]);
    }

    ArrayIterator &operator++() {
        ++index;
        return *this;
    }
    ArrayIterator operator++(int) {
        ArrayIterator tmp = *this;
        ++index;
        return tmp;
    }

    ArrayIterator &operator--() {
        --index;
        return *this;
    }
    ArrayIterator operator--(int) {
        ArrayIterator tmp = *this;
        --index;
        return tmp;
    }

    ArrayIterator operator+(difference_type n) const {
        return ArrayIterator(array_ptr, index + n);
    }
    ArrayIterator operator-(difference_type n) const {
        return ArrayIterator(array_ptr, index - n);
    }

    ArrayIterator &operator+=(difference_type n) {
        index += n;
        return *this;
    }
    ArrayIterator &operator-=(difference_type n) {
        index -= n;
        return *this;
    }

    difference_type operator-(const ArrayIterator &other) const {
        return index - other.index;
    }

    reference operator[](difference_type n) const {
        return (*array_ptr)[index + n];
    }

    bool operator==(const ArrayIterator &other) const {
        return array_ptr == other.array_ptr && index == other.index;
    }
    bool operator!=(const ArrayIterator &other) const {
        return !(*this == other);
    }
    bool operator<(const ArrayIterator &other) const {
        return index < other.index;
    }
    bool operator>(const ArrayIterator &other) const {
        return index > other.index;
    }
    bool operator<=(const ArrayIterator &other) const {
        return index <= other.index;
    }
    bool operator>=(const ArrayIterator &other) const {
        return index >= other.index;
    }

private:
    Array *array_ptr;
    size_t index;
};

// Iterator implementation for Object
class ObjectIterator {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::pair<const std::string, Value>;
    using difference_type = std::ptrdiff_t;
    using pointer = std::pair<const std::string, Value> *;
    using reference = std::pair<const std::string, Value> &;

    ObjectIterator() : object_ptr(nullptr), it() {}
    ObjectIterator(Object *obj, Object::iterator iter)
        : object_ptr(obj), it(iter) {}

    reference operator*() const {
        return *it;
    }
    pointer operator->() const {
        return &(*it);
    }

    ObjectIterator &operator++() {
        ++it;
        return *this;
    }
    ObjectIterator operator++(int) {
        ObjectIterator tmp = *this;
        ++it;
        return tmp;
    }

    ObjectIterator &operator--() {
        --it;
        return *this;
    }
    ObjectIterator operator--(int) {
        ObjectIterator tmp = *this;
        --it;
        return tmp;
    }

    bool operator==(const ObjectIterator &other) const {
        return object_ptr == other.object_ptr && it == other.it;
    }
    bool operator!=(const ObjectIterator &other) const {
        return !(*this == other);
    }

private:
    Object *object_ptr;
    Object::iterator it;
};

// Configuration for pretty-printing
struct PrettyPrintConfig {
    bool pretty = true;            // Enable pretty printing (false = compact)
    std::string indent = "  ";     // Indentation string (default = 2 spaces)
    bool sort_keys = false;        // Sort object keys alphabetically
    bool escape_non_ascii = true;  // Escape non-ASCII characters

    // Static methods for commonly used configurations
    static PrettyPrintConfig Compact() {
        PrettyPrintConfig config;
        config.pretty = false;
        return config;
    }

    static PrettyPrintConfig Readable() {
        PrettyPrintConfig config;
        config.pretty = true;
        config.indent = "  ";
        return config;
    }

    static PrettyPrintConfig WithTabs() {
        PrettyPrintConfig config;
        config.pretty = true;
        config.indent = "\t";
        return config;
    }
};

// Implement array iterator methods
inline ::json_::ArrayIterator Value::begin_array() {
    if (!is_array()) throw std::runtime_error("Value is not an array");
    return ::json_::ArrayIterator(&as_array(), 0);
}

inline ::json_::ArrayIterator Value::end_array() {
    if (!is_array()) throw std::runtime_error("Value is not an array");
    return ::json_::ArrayIterator(&as_array(), as_array().size());
}

inline ::json_::ArrayIterator Value::begin_array() const {
    if (!is_array()) throw std::runtime_error("Value is not an array");
    return ::json_::ArrayIterator(const_cast<Array *>(&as_array()), 0);
}

inline ::json_::ArrayIterator Value::end_array() const {
    if (!is_array()) throw std::runtime_error("Value is not an array");
    return ::json_::ArrayIterator(const_cast<Array *>(&as_array()),
                                 as_array().size());
}

// Implement object iterator methods
inline ::json_::ObjectIterator Value::begin_object() {
    if (!is_object()) throw std::runtime_error("Value is not an object");
    return ::json_::ObjectIterator(&as_object(), as_object().begin());
}

inline ::json_::ObjectIterator Value::end_object() {
    if (!is_object()) throw std::runtime_error("Value is not an object");
    return ::json_::ObjectIterator(&as_object(), as_object().end());
}

inline ::json_::ObjectIterator Value::begin_object() const {
    if (!is_object()) throw std::runtime_error("Value is not an object");
    return ::json_::ObjectIterator(
        const_cast<Object *>(&as_object()),
        Object::iterator(const_cast<Object &>(as_object()).begin()));
}

inline ::json_::ObjectIterator Value::end_object() const {
    if (!is_object()) throw std::runtime_error("Value is not an object");
    return ::json_::ObjectIterator(
        const_cast<Object *>(&as_object()),
        Object::iterator(const_cast<Object &>(as_object()).end()));
}

// String formatting helper
inline void Value::format_string(std::ostringstream &oss,
                                 const std::string &str) const {
    oss << "\"";
    for (char c : str) {
        switch (c) {
            case '\"':
                oss << "\\\"";
                break;
            case '\\':
                oss << "\\\\";
                break;
            case '\b':
                oss << "\\b";
                break;
            case '\f':
                oss << "\\f";
                break;
            case '\n':
                oss << "\\n";
                break;
            case '\r':
                oss << "\\r";
                break;
            case '\t':
                oss << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 32 ||
                    static_cast<unsigned char>(c) >= 128) {
                    // Format control characters and non-ASCII as \uXXXX
                    oss << "\\u" << std::hex << std::setw(4)
                        << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c))
                        << std::dec;
                } else {
                    oss << c;
                }
                break;
        }
    }
    oss << "\"";
}

// Implement pretty printing
inline void Value::to_string_pretty(std::ostringstream &oss,
                                    const PrettyPrintConfig &config,
                                    int depth) const {
    if (is_null()) {
        oss << "null";
    } else if (is_bool()) {
        oss << (as_bool() ? "true" : "false");
    } else if (is_int()) {
        oss << as_int();
    } else if (is_double()) {
        // Ensure proper formatting of doubles (avoid scientific notation for
        // small values)
        auto val = as_double();
        if (std::abs(val) < 0.0001 || std::abs(val) > 1e7) {
            oss << std::scientific << val;
        } else {
            oss << std::fixed << val;
        }
        // Remove trailing zeros and decimal point if needed
        std::string str = oss.str();
        size_t pos = str.find_last_not_of('0');
        if (pos != std::string::npos && str[pos] == '.') pos--;
        str.erase(pos + 1);
        oss.str(str);
    } else if (is_string()) {
        format_string(oss, as_string());
    } else if (is_array()) {
        const auto &array = as_array();
        if (array.empty()) {
            oss << "[]";
            return;
        }

        oss << "[";
        if (config.pretty) oss << "\n";

        for (size_t i = 0; i < array.size(); ++i) {
            if (config.pretty) {
                for (int j = 0; j < depth + 1; ++j)
                    oss << config.indent;
            }

            std::ostringstream tmp;
            array[i].to_string_pretty(tmp, config, depth + 1);
            oss << tmp.str();

            if (i < array.size() - 1) {
                oss << ",";
            }

            if (config.pretty) oss << "\n";
        }

        if (config.pretty) {
            for (int j = 0; j < depth; ++j)
                oss << config.indent;
        }
        oss << "]";
    } else if (is_object()) {
        const auto &object = as_object();
        if (object.empty()) {
            oss << "{}";
            return;
        }

        oss << "{";
        if (config.pretty) oss << "\n";

        // Process keys
        std::vector<std::string> keys;
        for (const auto &[key, _] : object) {
            keys.push_back(key);
        }

        // Sort keys if needed
        if (config.sort_keys) {
            std::sort(keys.begin(), keys.end());
        }

        for (size_t i = 0; i < keys.size(); ++i) {
            const auto &key = keys[i];
            const auto &value = object.at(key);

            if (config.pretty) {
                for (int j = 0; j < depth + 1; ++j)
                    oss << config.indent;
            }

            format_string(oss, key);
            oss << ":";
            if (config.pretty) oss << " ";

            std::ostringstream tmp;
            value.to_string_pretty(tmp, config, depth + 1);
            oss << tmp.str();

            if (i < keys.size() - 1) {
                oss << ",";
            }

            if (config.pretty) oss << "\n";
        }

        if (config.pretty) {
            for (int j = 0; j < depth; ++j)
                oss << config.indent;
        }
        oss << "}";
    }
}

// Basic to_string() implementation that calls the pretty printing version
inline std::string Value::to_string() const {
    // Default to compact printing
    return to_string(PrettyPrintConfig::Compact());
}

// Pretty printing to_string() implementation
inline std::string Value::to_string(const PrettyPrintConfig &config) const {
    std::ostringstream oss;
    to_string_pretty(oss, config, 0);
    return oss.str();
}

// Parser implementation
class Parser {
public:
    static Value parse(const std::string &json) {
        size_t pos = 0;
        skip_whitespace(json, pos);
        Value result = parse_value(json, pos);
        skip_whitespace(json, pos);

        if (pos != json.size()) {
            throw std::runtime_error("Unexpected trailing characters");
        }

        return result;
    }

private:
    static void skip_whitespace(const std::string &json, size_t &pos) {
        while (pos < json.size() && std::isspace(json[pos])) {
            ++pos;
        }
    }

    static Value parse_value(const std::string &json, size_t &pos) {
        skip_whitespace(json, pos);

        if (pos >= json.size()) {
            throw std::runtime_error("Unexpected end of JSON data");
        }

        char c = json[pos];

        if (c == '{') {
            return parse_object(json, pos);
        } else if (c == '[') {
            return parse_array(json, pos);
        } else if (c == '"') {
            return Value(parse_string(json, pos));
        } else if (c == 't') {
            if (pos + 3 < json.size() && json.substr(pos, 4) == "true") {
                pos += 4;
                return Value(true);
            }
            throw std::runtime_error("Invalid JSON: expected 'true'");
        } else if (c == 'f') {
            if (pos + 4 < json.size() && json.substr(pos, 5) == "false") {
                pos += 5;
                return Value(false);
            }
            throw std::runtime_error("Invalid JSON: expected 'false'");
        } else if (c == 'n') {
            if (pos + 3 < json.size() && json.substr(pos, 4) == "null") {
                pos += 4;
                return Value(nullptr);
            }
            throw std::runtime_error("Invalid JSON: expected 'null'");
        } else if (c == '-' || std::isdigit(c)) {
            return parse_number(json, pos);
        }

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    static std::string parse_string(const std::string &json, size_t &pos) {
        if (json[pos] != '"') {
            throw std::runtime_error("Expected '\"' at beginning of string");
        }

        ++pos;  // Skip opening quote
        std::string result;

        while (pos < json.size()) {
            char c = json[pos++];

            if (c == '"') {
                return result;  // End of string
            } else if (c == '\\') {
                // Handle escape sequences
                if (pos >= json.size()) {
                    throw std::runtime_error(
                        "Unexpected end of JSON data in string");
                }

                char escape = json[pos++];
                switch (escape) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'u': {
                        // Unicode escape sequence (simplified - doesn't handle
                        // surrogate pairs)
                        if (pos + 3 >= json.size()) {
                            throw std::runtime_error(
                                "Unexpected end of JSON data in unicode "
                                "escape");
                        }

                        std::string hex = json.substr(pos, 4);
                        pos += 4;

                        // Convert hex to int (simplified)
                        int unicode_val = std::stoi(hex, nullptr, 16);

                        // This is a simplification - proper handling of Unicode
                        // is complex
                        if (unicode_val < 128) {
                            result += static_cast<char>(unicode_val);
                        } else {
                            result += '?';  // Placeholder for non-ASCII
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error(
                            std::string("Invalid escape sequence: \\") +
                            escape);
                }
            } else {
                result += c;
            }
        }

        throw std::runtime_error("Unterminated string");
    }

    static Value parse_number(const std::string &json, size_t &pos) {
        size_t start = pos;
        bool is_float = false;

        // Handle negative sign
        if (json[pos] == '-') {
            ++pos;
        }

        // Integer part
        while (pos < json.size() && std::isdigit(json[pos])) {
            ++pos;
        }

        // Fractional part
        if (pos < json.size() && json[pos] == '.') {
            is_float = true;
            ++pos;

            while (pos < json.size() && std::isdigit(json[pos])) {
                ++pos;
            }
        }

        // Exponent
        if (pos < json.size() && (json[pos] == 'e' || json[pos] == 'E')) {
            is_float = true;
            ++pos;

            if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) {
                ++pos;
            }

            while (pos < json.size() && std::isdigit(json[pos])) {
                ++pos;
            }
        }

        std::string num_str = json.substr(start, pos - start);

        if (is_float) {
            return Value(std::stod(num_str));
        } else {
            return Value(std::stoll(num_str));
        }
    }

    static Value parse_array(const std::string &json, size_t &pos) {
        if (json[pos] != '[') {
            throw std::runtime_error("Expected '[' at beginning of array");
        }

        ++pos;  // Skip opening bracket
        skip_whitespace(json, pos);

        Array array;

        // Check for empty array
        if (pos < json.size() && json[pos] == ']') {
            ++pos;
            return Value(array);
        }

        while (pos < json.size()) {
            array.push_back(parse_value(json, pos));
            skip_whitespace(json, pos);

            if (pos >= json.size()) {
                throw std::runtime_error("Unterminated array");
            }

            if (json[pos] == ']') {
                ++pos;  // Skip closing bracket
                return Value(array);
            }

            if (json[pos] != ',') {
                throw std::runtime_error("Expected ',' or ']' in array");
            }

            ++pos;  // Skip comma
            skip_whitespace(json, pos);
        }

        throw std::runtime_error("Unterminated array");
    }

    static Value parse_object(const std::string &json, size_t &pos) {
        if (json[pos] != '{') {
            throw std::runtime_error("Expected '{' at beginning of object");
        }

        ++pos;  // Skip opening brace
        skip_whitespace(json, pos);

        Object object;

        // Check for empty object
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return Value(object);
        }

        while (pos < json.size()) {
            // Parse key
            if (json[pos] != '"') {
                throw std::runtime_error("Expected string as object key");
            }

            std::string key = parse_string(json, pos);
            skip_whitespace(json, pos);

            // Parse colon
            if (pos >= json.size() || json[pos] != ':') {
                throw std::runtime_error("Expected ':' after object key");
            }

            ++pos;  // Skip colon

            // Parse value
            object[key] = parse_value(json, pos);
            skip_whitespace(json, pos);

            if (pos >= json.size()) {
                throw std::runtime_error("Unterminated object");
            }
            if (json[pos] == '}') {
                ++pos;  // Skip closing brace
                return Value(object);
            }
            if (json[pos] != ',') {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
            ++pos;  // Skip comma
            skip_whitespace(json, pos);
        }
        throw std::runtime_error("Unterminated object");
    }
};
}  // namespace json_

#endif  // JSON_HPP