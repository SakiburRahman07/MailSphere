#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class PC : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 2;
    string qname;
    cMessage *startEvt = nullptr;

  protected:
    void initialize() override {
        addr    = par("address");
        dnsAddr = par("dnsAddr");
        qname   = par("dnsQuery").stdstringValue(); // module param → stdstringValue() OK
        startEvt = new cMessage("start");
        scheduleAt(simTime() + SimTime(par("startAt").doubleValue()), startEvt);
        EV_INFO << "PC" << addr << " will start at " << par("startAt").doubleValue() << "s\n";
    }

    void handleMessage(cMessage *msg) override {
        if (msg->isSelfMessage()) {
            // Step 1: ask DNS
            auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
            q->addPar("qname").setStringValue(qname.c_str());
            send(q, "ppp$o");
            delete msg;
            return;
        }

        switch (msg->getKind()) {
            case DNS_RESPONSE: {
                long httpAddr = msg->par("answer").longValue();
                EV_INFO << "PC" << addr << " DNS: "
                        << msg->par("qname").stringValue()  // message param → stringValue()
                        << " -> " << httpAddr << "\n";
                // Step 2: HTTP GET to server
                auto *get = mk("HTTP_GET", HTTP_GET, addr, httpAddr);
                get->addPar("path").setStringValue("/");
                send(get, "ppp$o");
                break;
            }
            case HTTP_RESPONSE: {
                EV_INFO << "PC" << addr << " got HTTP response "
                        << msg->par("bytes").longValue() << " bytes\n";
                break;
            }
            default:
                EV_WARN << "PC" << addr << " unexpected kind=" << msg->getKind() << "\n";
        }
        delete msg;
    }

    void finish() override {
        cancelAndDelete(startEvt);
        startEvt = nullptr;
    }
};
Define_Module(PC);

