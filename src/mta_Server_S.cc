#include <omnetpp.h>
using namespace omnetpp;

class MTA_Server_S : public cSimpleModule {
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Server_S);

// ---- Implementations ----
void MTA_Server_S::initialize() {
    // TODO: initialization code
    EV << "MTA_Server_S initialized\n";
}

void MTA_Server_S::handleMessage(cMessage *msg) {
    // TODO: handle incoming messages
    EV << "MTA_Server_S received a message\n";
    delete msg;  // or send(), depending on logic
}
