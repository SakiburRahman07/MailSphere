#include <omnetpp.h>
using namespace omnetpp;

class MTA_Server_RS : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Server_RS);

// ---- Implementations ----
void MTA_Server_RS::initialize() {
    EV << "MTA_Server_RS initialized\n";
}

void MTA_Server_RS::handleMessage(cMessage *msg) {
    EV << "MTA_Server_RS received a message\n";
    delete msg;  
}