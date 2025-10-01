
#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class DNS : public cSimpleModule {
  private:
    int addr = 0;
    int answer = 3; // HTTP server address
  protected:
    void initialize() override {
        addr   = par("address");
        answer = par("answerAddr");
    }

    void handleMessage(cMessage *msg) override {
        if (msg->getKind() == DNS_QUERY) {
            long src = SRC(msg);
            auto *resp = mk("DNS_RESPONSE", DNS_RESPONSE, addr, src);
            resp->addPar("qname").setStringValue(msg->par("qname").stringValue());
            resp->addPar("answer").setLongValue(answer);
            send(resp, "ppp$o");
        } else {
            EV_WARN << "DNS unexpected kind=" << msg->getKind() << "\n";
        }
        delete msg;
    }
};
Define_Module(DNS);



