#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class spool : public cSimpleModule {
  private:
    int addr = 0;
    int nextHopAddr = 0; // MTA_Client_SS
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(spool);

// ---- Implementations ----
void spool::initialize() {
    addr = par("address");
    nextHopAddr = 401; // align with config
}

void spool::handleMessage(cMessage *msg) {
    if (msg->getKind() == PUSH_REQUEST) {
        // simulate queueing delay then forward
        auto *fwd = mk("PUSH_REQUEST", PUSH_REQUEST, addr, nextHopAddr);
        sendDelayed(fwd, 0.01, "ppp$o", 1);
    }
    delete msg;  
}