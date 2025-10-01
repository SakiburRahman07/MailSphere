#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Server_S : public cSimpleModule {
  private:
    int addr = 0;
    int spoolAddr = 0;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Server_S);

// ---- Implementations ----
void MTA_Server_S::initialize() {
    addr = par("address");
    spoolAddr = 300; // align with config
}

void MTA_Server_S::handleMessage(cMessage *msg) {
    if (msg->getKind() == PUSH_REQUEST) {
        // ACK back to client S (optional)
        auto *ack = mk("PUSH_ACK", PUSH_ACK, addr, SRC(msg));
        send(ack, "ppp$o", 0);
        // forward to spool
        auto *fwd = mk("PUSH_REQUEST", PUSH_REQUEST, addr, spoolAddr);
        send(fwd, "ppp$o", 1);
    }
    delete msg;
}
