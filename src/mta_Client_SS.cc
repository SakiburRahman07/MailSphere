#include <omnetpp.h>
using namespace omnetpp;

class MTA_Client_SS : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Client_SS);

// ---- Implementations ----
void MTA_Client_SS::initialize() {
    EV << "MTA_Client_SS initialized\n";
}

void MTA_Client_SS::handleMessage(cMessage *msg) {
    EV << "MTA_Client_SS received a message\n";
    delete msg;  
}