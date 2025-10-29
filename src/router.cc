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
        
        EV << "Router: Received message kind=" << msg->getKind() 
           << " from gate " << msg->getArrivalGate()->getIndex()
           << " dst=" << dst << "\n";
        
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
            EV << "Router: Found route for dst=" << dst << " -> gate " << g << "\n";
            if (g >= 0 && g < gateSize("pppg") && gate("pppg$o", g)->isConnected()) { 
                send(msg, "pppg$o", g); 
                return; 
            }
        }
        // Only flood if destination is not in routing table
        // Skip incoming gate and unconnected gates
        int inIdx = msg->getArrivalGate()->getIndex();
        int sentCount = 0;
        for (int i=0; i<gateSize("pppg"); ++i) {
            if (i != inIdx && gate("pppg$o", i)->isConnected()) {
                send(msg->dup(), "pppg$o", i);
                sentCount++;
            }
        }
        if (sentCount == 0) {
            // No gates to flood to - drop the message
            EV_WARN << "Router: No route found for dst=" << dst << " and no gates to flood. Dropping message.\n";
            delete msg;
        } else {
            delete msg; // delete original after duplicates sent
        }
    }
};
Define_Module(Router);

