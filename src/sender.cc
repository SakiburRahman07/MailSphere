#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class Sender : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 0;       // address of dns1
    const char* mtaQName = "mta_client_s"; // logical name to resolve
    std::string content;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(Sender);

// ---- Implementations ----
void Sender::initialize() {
    addr = par("address");
    content = par("message").stdstringValue();
    // assume dns1 reachable via Router1 based on addressing; set logical address
    dnsAddr = 100; // will be configured in ini via routes; used as dst
    // Kick off DNS query for MTA_Client_S
    auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
    q->addPar("qname").setStringValue(mtaQName);
    q->addPar("content").setStringValue(content.c_str());
    send(q, "ppp$o");
}

void Sender::handleMessage(cMessage *msg) {
    if (msg->getKind() == DNS_RESPONSE) {
        long httpAddr = msg->par("answer").longValue();
        auto *get = mk("HTTP_GET", HTTP_GET, addr, httpAddr);
        get->addPar("path").setStringValue("/submit");
        get->addPar("content").setStringValue(content.c_str());
        send(get, "ppp$o");
    }
    delete msg;  
}