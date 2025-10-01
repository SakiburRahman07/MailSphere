#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class Receiver : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 950;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(Receiver);

// ---- Implementations ----
void Receiver::initialize() {
    addr = par("address");
    auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
    q->addPar("qname").setStringValue("maa_client");
    send(q, "ppp$o");
}

void Receiver::handleMessage(cMessage *msg) {
    if (msg->getKind() == DNS_RESPONSE) {
        long maaClientAddr = msg->par("answer").longValue();
        auto *get = mk("HTTP_GET", HTTP_GET, addr, maaClientAddr);
        get->addPar("path").setStringValue("/read");
        send(get, "ppp$o");
    } else if (msg->getKind() == HTTP_RESPONSE) {
        const char* c = msg->hasPar("content") ? msg->par("content").stringValue() : "<empty>";
        EV << "Receiver got content: " << c << "\n";
    }
    delete msg;  
}