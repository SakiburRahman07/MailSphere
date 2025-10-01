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
    HTTP_GET=20, HTTP_RESPONSE=21
};


static cMessage* mk(const char* name, int kind, long src, long dst) {
    auto *m = new cMessage(name, kind);
    m->addPar("src").setLongValue(src);
    m->addPar("dst").setLongValue(dst);
    return m;
}
static inline long SRC(cMessage* m){ return m->par("src").longValue(); }
static inline long DST(cMessage* m){ return m->par("dst").longValue(); }


#endif /* MODULES_HELPERS_H_ */
