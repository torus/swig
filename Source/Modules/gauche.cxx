#include "swigmod.h"

class GAUCHE : public Language {
public:

    virtual void main(int argc, char *argv[]) {
        printf("I'm the Gauche module.\n");
    }

    virtual int top(Node *n) {
        printf("Generating code.\n");
        return SWIG_OK;
    }

};

extern "C" Language *
swig_gauche(void) {
    return new GAUCHE();
}
