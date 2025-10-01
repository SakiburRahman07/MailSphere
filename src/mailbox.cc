#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class mailbox : public cSimpleModule {
  private:
    int addr = 0;
    cQueue store;
    int maaServerAddr = 800;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(mailbox);

// ---- Implementations ----
void mailbox::initialize() {
    addr = par("address");
}

void mailbox::handleMessage(cMessage *msg) {
    if (msg->getKind() == SMTP_SEND) {
        bool wasEmpty = store.isEmpty();
        store.insert(msg); // keep SMTP envelopes (with content par)
        if (wasEmpty) {
            auto *note = mk("NOTIFY_NEWMAIL", NOTIFY_NEWMAIL, addr, maaServerAddr);
            send(note, "ppp$o", 1);
        }
        return;
    } else if (msg->getKind() == IMAP_FETCH) {
        if (!store.isEmpty()) {
            auto *m = check_and_cast<cMessage*>(store.pop());
            auto *resp = mk("IMAP_RESPONSE", IMAP_RESPONSE, addr, SRC(msg));
            resp->addPar("bytes").setLongValue(20000);
            if (m->hasPar("content")) resp->addPar("content").setStringValue(m->par("content").stdstringValue().c_str());
            send(resp, "ppp$o", 1);
            delete m;
        }
    }
    delete msg;  
}