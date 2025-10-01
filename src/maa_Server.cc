#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MAA_Server : public cSimpleModule {
  private:
    int addr = 0;
    int mailboxAddr = 700;
    long pendingClient = -1;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MAA_Server);

// ---- Implementations ----
void MAA_Server::initialize() {
    addr = par("address");
}

void MAA_Server::handleMessage(cMessage *msg) {
    if (msg->getKind() == HTTP_GET) {
        pendingClient = SRC(msg);
        auto *fetch = mk("IMAP_FETCH", IMAP_FETCH, addr, mailboxAddr);
        send(fetch, "ppp$o", 0);
    } else if (msg->getKind() == IMAP_RESPONSE) {
        auto *resp = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, pendingClient);
        resp->addPar("bytes").setLongValue(20000);
        if (msg->hasPar("content")) resp->addPar("content").setStringValue(msg->par("content").stdstringValue().c_str());
        send(resp, "ppp$o", 1);
        pendingClient = -1;
    } else if (msg->getKind() == NOTIFY_NEWMAIL) {
        // passively trigger MAA_Client pull by sending a small HTTP hint
        auto *hint = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, 850 /* maa_client */);
        hint->addPar("bytes").setLongValue(1);
        send(hint, "ppp$o", 1);
    }
    delete msg;  
}