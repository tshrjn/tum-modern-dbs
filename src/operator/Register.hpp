#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <string>
#include "../src/schema/Types.cpp"

class Register {
public:


private:

    Types::Tag type;

    int intValue;

    std::string stringValue;

public:
    Register();

    ~Register();

    void load(Types::Tag type, void *data);

    void setInt(int value) {
        type = Types::Tag::Integer;
        intValue = value;
    }

    void setString(const std::string &value) {
        type = Types::Tag::Char;
        stringValue = value;
    }

    // Getters
    Types::Tag getType() const { return type; }

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
