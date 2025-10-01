#include <omnetpp.h>
using namespace omnetpp;

class spool : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(spool);

// ---- Implementations ----
void spool::initialize() {
    EV << "spool initialized\n";
}

void spool::handleMessage(cMessage *msg) {
    EV << "spool received a message\n";
    delete msg;  
}