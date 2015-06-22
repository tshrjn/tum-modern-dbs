#include "operator/Printer.hpp"
#include "operator/Register.hpp"
#include <iostream>

using namespace std;

Printer::Printer(unique_ptr < Operator > && input)
        : input(move(input)),
          toPrint(this->input->getOutput()) { }

Printer::Printer(unique_ptr < Operator > && input, const vector<const Register *> &toPrint)
    : input(move(input)), toPrint(toPrint) { }

void Printer::open() {
    input->open();
}

bool Printer::next() {
    // Produce the next tuple
    if (!input->next())
        return false;

    // Print the entries
    for (unsigned index = 0, limit = toPrint.size(); index < limit; ++index) {
        if (index) cout << ' ';
        const Register &r = *toPrint[index];
        switch (r.getType()) {
            case Types::Tag::Integer:
                cout << r.getInt();
                break;
            case Types::Tag::Char:
                cout << r.getString();
                break;
        }
    }
    cout << endl;
    return true;
}

void Printer::close() {
    input->close();
}


vector<const Register *> Printer::getOutput() const {
    return input->getOutput();
}
