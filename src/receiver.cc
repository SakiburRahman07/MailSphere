#include <omnetpp.h>
using namespace omnetpp;

class Receiver : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(Receiver);

// ---- Implementations ----
void Receiver::initialize() {
    EV << "Receiver initialized\n";
}

void Receiver::handleMessage(cMessage *msg) {
    EV << "Receiver received a message\n";
    delete msg;  
}