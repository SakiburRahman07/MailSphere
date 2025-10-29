#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class Sender : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 0;       // address of dns1
    const char* mtaQName = "mta_client_s"; // logical name to resolve
    std::string content;
    std::string mailFrom, mailTo, mailSubject, mailBody;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(Sender);

// ---- Implementations ----
void Sender::initialize() {
    addr = par("address");
    content = par("message").stdstringValue();
    mailFrom = par("mail_from").stdstringValue();
    mailTo = par("mail_to").stdstringValue();
    mailSubject = par("mail_subject").stdstringValue();
    mailBody = par("mail_body").stdstringValue();
    // DNS1 is at address 150 (configured in omnetpp.ini)
    dnsAddr = 150; // Fixed: was incorrectly set to 100 (sender's own address)
    // Kick off DNS query for MTA_Client_S
    auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
    q->addPar("qname").setStringValue(mtaQName);
    q->addPar("content").setStringValue(content.c_str());
    q->addPar("mail_from").setStringValue(mailFrom.c_str());
    q->addPar("mail_to").setStringValue(mailTo.c_str());
    q->addPar("mail_subject").setStringValue(mailSubject.c_str());
    q->addPar("mail_body").setStringValue(mailBody.c_str());
    send(q, "ppp$o");
}

void Sender::handleMessage(cMessage *msg) {
    if (msg->getKind() == DNS_RESPONSE) {
        long httpAddr = msg->par("answer").longValue();
        auto *get = mk("HTTP_GET", HTTP_GET, addr, httpAddr);
        get->addPar("path").setStringValue("/submit");
        get->addPar("content").setStringValue(content.c_str());
        get->addPar("mail_from").setStringValue(mailFrom.c_str());
        get->addPar("mail_to").setStringValue(mailTo.c_str());
        get->addPar("mail_subject").setStringValue(mailSubject.c_str());
        get->addPar("mail_body").setStringValue(mailBody.c_str());
        send(get, "ppp$o");
    }
    delete msg;  
}