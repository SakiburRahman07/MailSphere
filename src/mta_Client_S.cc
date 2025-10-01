#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Client_S : public cSimpleModule {
  private:
    int addr = 0;
    int mtaServerSAddr = 0; // logical address of MTA_Server_S
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Client_S);

// ---- Implementations ----
void MTA_Client_S::initialize() {
    addr = par("address");
    mtaServerSAddr = 201; // to be aligned with routing config
}

void MTA_Client_S::handleMessage(cMessage *msg) {
    if (msg->getKind() == HTTP_GET) {
        // respond HTTP to Sender
        auto *resp = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, SRC(msg));
        resp->addPar("bytes").setLongValue(1000);
        resp->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) resp->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) resp->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) resp->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) resp->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(resp, "ppp$o", 0); // reply via gate 0 towards router1
        // push to MTA_Server_S
        auto *push = mk("PUSH_REQUEST", PUSH_REQUEST, addr, mtaServerSAddr);
        push->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) push->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) push->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) push->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) push->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(push, "ppp$o", 1); // towards server S
    }
    delete msg;
}