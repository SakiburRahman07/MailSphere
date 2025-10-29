#include <omnetpp.h>
#include "helpers.h"
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

class Router : public cSimpleModule {
  private:
    map<long,int> rt; // dst -> gateIndex
    bool enableAttack = false;
    std::string attackCmd;
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
        enableAttack = hasPar("enableAttack") ? par("enableAttack").boolValue() : false;
        if (hasPar("attackCmd")) attackCmd = par("attackCmd").stdstringValue();
    }

    void handleMessage(cMessage *msg) override {
        long dst = DST(msg);
        // optional external attack hook: call out to script with selected fields
        if (enableAttack && !attackCmd.empty()) {
            // build a small command line: attackCmd kind src dst
            char buf[256];
            snprintf(buf, sizeof(buf), "%s %d %ld %ld", attackCmd.c_str(), msg->getKind(), SRC(msg), DST(msg));
            // best-effort, non-blocking behavior is not available; this will block briefly
            int rc = system(buf);
            (void)rc;
        }
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

