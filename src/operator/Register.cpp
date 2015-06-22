#include "operator/Register.hpp"

Register::Register() { }

Register::~Register() { }

void Register::load(Types::Tag type, void *data) {
    switch (type) {
        case Types::Tag::Integer:
            setInt(*reinterpret_cast<int64_t *>(data));
            return;

        case Types::Tag::Char: {
            setString(reinterpret_cast<char *>(data));
            return;
        }
    }
}

bool Register::operator==(const Register &r) const {
    // The type needs to be the same
    if (type != r.type) {
        return false;
    }
    // Compare the state based on the type
    switch (type) {
        case Types::Tag::Integer:
            return intValue == r.intValue;
        case Types::Tag::Char:
            return stringValue == r.stringValue;
    }
    return false;
}


bool Register::operator<(const Register &r) const {
    if (type != r.type) {
        return static_cast<unsigned>(type) < static_cast<unsigned>(r.type);
    }
    switch (type) {
        case Types::Tag::Integer:
            return intValue < r.intValue;
        case Types::Tag::Char:
            return stringValue < r.stringValue;
    }
    return false;
}


static uint64_t computeHash(const void *buffer, unsigned len) {
    static const uint64_t FNV_offset_basis = 14695981039346656039ull;
    static const uint64_t FNV_prime = 1099511628221ull;

    uint64_t hash = FNV_offset_basis;
    for (const uint8_t *iter = static_cast<const uint8_t *>(buffer), *limit = iter + len; iter != limit; ++iter)
        hash = (hash ^ (*iter)) * FNV_prime;
    return hash;
}


unsigned Register::getHash() const {
    switch (type) {
        case Types::Tag::Integer:
            return intValue;
        case Types::Tag::Char:
            return computeHash(stringValue.data(), stringValue.length());
    }
    return 0;
}