#ifndef MODULES_HELPERS_H_
#define MODULES_HELPERS_H_

#include <omnetpp.h>
#include <map>
#include <sstream>
using namespace omnetpp;
using namespace std;

/*
Message kinds:
  10 = DNS_QUERY
  11 = DNS_RESPONSE
  20 = HTTP_GET
  21 = HTTP_RESPONSE

For all messages we set:
  par("src") : long  logical sender address
  par("dst") : long  logical destination address

Plus:
  DNS_QUERY:    par("qname"): string
  DNS_RESPONSE: par("qname"): string, par("answer"): long (HTTP addr)
  HTTP_GET:     par("path"): string (optional)
  HTTP_RESPONSE:par("bytes"): long (payload size)
*/

enum {
    DNS_QUERY=10, DNS_RESPONSE=11,
    HTTP_GET=20, HTTP_RESPONSE=21,
    PUSH_REQUEST=30, PUSH_ACK=31,
    SMTP_SEND=40, SMTP_ACK=41,
    SMTP_CMD=42, SMTP_RESP=43,
    IMAP_FETCH=50, IMAP_RESPONSE=51,
    NOTIFY_NEWMAIL=60
};


static cMessage* mk(const char* name, int kind, long src, long dst) {
    // Use cPacket for better animation in the GUI (packets are visualized on links)
    auto *m = new cPacket(name, kind);
    m->addPar("src").setLongValue(src);
    m->addPar("dst").setLongValue(dst);
    // give it a non-zero size for link animations
    switch (kind) {
        case DNS_QUERY: case DNS_RESPONSE: m->setByteLength(64); break;
        case HTTP_GET: case HTTP_RESPONSE: m->setByteLength(1200); break;
        case PUSH_REQUEST: case PUSH_ACK: m->setByteLength(300); break;
        case SMTP_SEND: case SMTP_ACK: m->setByteLength(800); break;
        case SMTP_CMD: case SMTP_RESP: m->setByteLength(200); break;
        case IMAP_FETCH: case IMAP_RESPONSE: m->setByteLength(800); break;
        case NOTIFY_NEWMAIL: m->setByteLength(64); break;
        default: m->setByteLength(256); break;
    }
    return m;
}
static inline long SRC(cMessage* m){ return m->par("src").longValue(); }
static inline long DST(cMessage* m){ return m->par("dst").longValue(); }


#endif /* MODULES_HELPERS_H_ */
