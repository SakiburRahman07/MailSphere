#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class Router : public cSimpleModule {
  private:
    map<long,int> rt; // dst -> gateIndex
  protected:
    void initialize() override {
        const char* s = par("routes").stringValue();
        stringstream ss(s ? s : "");
        string item;
        while (getline(ss, item, ',')) {
            if (item.empty()) continue;
            long d=-1; int g=-1;
            if (sscanf(item.c_str(), "%ld:%d", &d, &g) == 2) rt[d]=g;
        }
    }

    void handleMessage(cMessage *msg) override {
        long dst = DST(msg);
        auto it = rt.find(dst);
        if (it != rt.end()) {
            int g = it->second;
            if (g >= 0 && g < gateSize("pppg")) { send(msg, "pppg$o", g); return; }
        }
        // fallback: flood (skip incoming gate)
        int inIdx = msg->getArrivalGate()->getIndex();
        for (int i=0; i<gateSize("pppg"); ++i)
            if (i != inIdx) send(msg->dup(), "pppg$o", i);
        delete msg;
    }
};
Define_Module(Router);

