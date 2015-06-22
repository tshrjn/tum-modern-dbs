#include "operator/Projection.hpp"

using namespace std;

Projection::Projection(
        unique_ptr < Operator > && input,
const
vector <int> &registerIdzs
)
:

input(move(input)), registerIdzs(registerIdzs) {
}

Projection::~Projection() {
}

void Projection::open() {
    input->open();
}

bool Projection::next() {
    return input->next();
}

void Projection::close() {
    input->close();
}

// Simply fetch the output of the operator and copy a subset by using the previously stored target idzs 
vector<const Register *> Projection::getOutput() const {
    vector<const Register *> all = input->getOutput();
    vector<const Register *> result(registerIdzs.size());
    for (auto iter = registerIdzs.begin(), limit = registerIdzs.end(); iter != limit; ++iter)
        result.push_back(all[*iter]);

    return result;
}
