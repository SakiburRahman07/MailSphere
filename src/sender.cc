#include <omnetpp.h>
using namespace omnetpp;

class Sender : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(Sender);

// ---- Implementations ----
void Sender::initialize() {
    EV << "Sender initialized\n";
}

void Sender::handleMessage(cMessage *msg) {
    EV << "Sender received a message\n";
    delete msg;  
}