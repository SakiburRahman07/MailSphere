#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Server_RS : public cSimpleModule {
  private:
    int addr = 0;
    int mailboxAddr = 700;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Server_RS);

// ---- Implementations ----
void MTA_Server_RS::initialize() {
    addr = par("address");
}

void MTA_Server_RS::handleMessage(cMessage *msg) {
    if (msg->getKind() == SMTP_SEND) {
        auto *ack = mk("SMTP_ACK", SMTP_ACK, addr, SRC(msg));
        send(ack, "ppp$o", 0);
        auto *toMb = mk("SMTP_SEND", SMTP_SEND, addr, mailboxAddr);
        send(toMb, "ppp$o", 1);
    }
    delete msg;  
}