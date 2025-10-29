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
        if (msg->hasPar("mail_from")) ack->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        send(ack, "ppp$o", 0);
        // forward to spool
        auto *fwd = mk("PUSH_REQUEST", PUSH_REQUEST, addr, spoolAddr);
        if (msg->hasPar("content")) fwd->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) fwd->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) fwd->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) fwd->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) fwd->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(fwd, "ppp$o", 1);
    }
    delete msg;
}
