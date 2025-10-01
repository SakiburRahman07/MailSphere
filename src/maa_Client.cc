#include <omnetpp.h>
using namespace omnetpp;

class MAA_Client : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MAA_Client);

// ---- Implementations ----
void MAA_Client::initialize() {
    EV << "MAA_Client initialized\n";
}

void MAA_Client::handleMessage(cMessage *msg) {
    EV << "MAA_Client received a message\n";
    delete msg;  
}