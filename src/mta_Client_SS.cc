#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Client_SS : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 500;     // central DNS
    const char* serverQName = "mta_server_rs";
    long pendingSmtpDst = -1;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MTA_Client_SS);

// ---- Implementations ----
void MTA_Client_SS::initialize() {
    addr = par("address");
}

void MTA_Client_SS::handleMessage(cMessage *msg) {
    if (msg->getKind() == PUSH_REQUEST) {
        auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
        q->addPar("qname").setStringValue(serverQName);
        send(q, "ppp$o", 1); // to router
    } else if (msg->getKind() == DNS_RESPONSE) {
        long rsAddr = msg->par("answer").longValue();
        auto *smtp = mk("SMTP_SEND", SMTP_SEND, addr, rsAddr);
        send(smtp, "ppp$o", 1);
    }
    delete msg;
}