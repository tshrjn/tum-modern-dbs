#ifndef H_operator_Selection
#define H_operator_Selection

#include "operator/Operator.hpp"
#include <memory>

class Selection : public Operator {
private:
    std::unique_ptr <Operator> input;
    const Register *condition;
    const Register *equal;

public:
    Selection(std::unique_ptr <Operator> &&input, const Register *a, const Register *b);

    ~Selection();

    void open();

    bool next();

    void close();

    std::vector<const Register *> getOutput() const;
};

#endif
