#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class HTTP : public cSimpleModule {
  private:
    int addr = 0;
  protected:
    void initialize() override { addr = par("address"); }

    void handleMessage(cMessage *msg) override {
        if (msg->getKind() == HTTP_GET) {
            auto *resp = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, SRC(msg));
            resp->addPar("bytes").setLongValue(par("pageSizeBytes").intValue()); // module param
            sendDelayed(resp, SimTime(par("serviceTime").doubleValue()), "ppp$o");
        } else {
            EV_WARN << "HTTP unexpected kind=" << msg->getKind() << "\n";
        }
        delete msg;
    }
};
Define_Module(HTTP);
