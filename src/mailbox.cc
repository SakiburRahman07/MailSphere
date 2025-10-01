#include <omnetpp.h>
using namespace omnetpp;

class mailbox : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(mailbox);

// ---- Implementations ----
void mailbox::initialize() {
    EV << "mailbox initialized\n";
}

void mailbox::handleMessage(cMessage *msg) {
    EV << "mailbox received a message\n";
    delete msg;  
}