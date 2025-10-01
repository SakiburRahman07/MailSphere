#include <omnetpp.h>
using namespace omnetpp;

class MAA_Server : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MAA_Server);

// ---- Implementations ----
void MAA_Server::initialize() {
    EV << "MAA_Server initialized\n";
}

void MAA_Server::handleMessage(cMessage *msg) {
    EV << "MAA_Server received a message\n";
    delete msg;  
}