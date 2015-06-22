#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <string>

class Register {
public:

    enum class Type : unsigned {
        Int, String, Undefined
    };

private:

    // The type of a given Register
    Type type;

    int intValue;
    
    std::string stringValue;

public:
    Register();

    ~Register();

    // Setters
    void setUnbound() { type = Type::Undefined; }

    void setInt(int value) {
        type = Type::Int;
        intValue = value;
    }

    void setString(const std::string &value) {
        type = Type::String;
        stringValue = value;
    }

    // Indicator if the Register type is defined or not
    bool isUndefined() const { return type == Type::Undefined; }

    // Getters
    Type getType() const { return type; }

    int getInt() const { return intValue; }

    const std::string &getString() const { return stringValue; }

    // Comparison
    bool operator==(const Register &r) const;

    bool operator<(const Register &r) const;

    // Hashable
    unsigned getHash() const;

    struct hash {
        unsigned operator()(const Register &r) const { return r.getHash(); };
    };
};

#endif
