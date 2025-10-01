#include <omnetpp.h>
using namespace omnetpp;

class MTA_Client_S : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Client_S);

// ---- Implementations ----
void MTA_Client_S::initialize() {
    EV << "MTA_Client_S initialized\n";
}

void MTA_Client_S::handleMessage(cMessage *msg) {
    EV << "MTA_Client_S received a message\n";
    delete msg;  
}