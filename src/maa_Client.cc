#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MAA_Client : public cSimpleModule {
  private:
    int addr = 0;
    int serverAddr = 800;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MAA_Client);

// ---- Implementations ----
void MAA_Client::initialize() {
    addr = par("address");
}

void MAA_Client::handleMessage(cMessage *msg) {
    if (msg->getKind() == HTTP_GET) {
        // Receiver asked: pull from server
        auto *get = mk("HTTP_GET", HTTP_GET, addr, serverAddr);
        get->addPar("path").setStringValue("/inbox");
        send(get, "ppp$o", 0);
    } else if (msg->getKind() == HTTP_RESPONSE) {
        // pass to Receiver
        auto *toRx = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, 900 /* receiver */);
        toRx->addPar("bytes").setLongValue(msg->par("bytes").longValue());
        if (msg->hasPar("content")) toRx->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) toRx->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) toRx->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) toRx->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) toRx->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(toRx, "ppp$o", 1);
    } else if (msg->getKind() == NOTIFY_NEWMAIL) {
        // proactive pull on notification
        auto *get = mk("HTTP_GET", HTTP_GET, addr, serverAddr);
        get->addPar("path").setStringValue("/inbox");
        send(get, "ppp$o", 0);
    }
    delete msg;  
}