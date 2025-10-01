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
        if (msg->hasPar("content")) fwd->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) fwd->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) fwd->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) fwd->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) fwd->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        sendDelayed(fwd, 0.01, "ppp$o", 1);
    }
    delete msg;  
}