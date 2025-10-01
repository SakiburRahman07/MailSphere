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
        bool enc = msg->hasPar("enc") ? msg->par("enc").boolValue() : false;
        std::string fmt = msg->hasPar("enc_fmt") ? std::string(msg->par("enc_fmt").stringValue()) : std::string();
        std::string key = msg->hasPar("enc_key") ? std::string(msg->par("enc_key").stringValue()) : std::string();
        auto maybeDec = [&](const char* p){
            if (!p) return std::string();
            std::string s = p;
            std::string out;
            if (!s.empty()) {
                if (enc && fmt == "rsahex") {
                    out = rsaDecryptFromHex(s);
                } else if (enc && !key.empty()) {
                    if (fmt == "hex") out = xorDecrypt(fromHex(s), key);
                    else out = xorDecrypt(s, key);
                } else if (fmt == "hex") {
                    // if marked hex but not enc, at least decode hex
                    out = fromHex(s);
                } else {
                    out = s;
                }
            }
            return out;
        };
        // debug
        EV << "Receiver decrypt meta: enc=" << enc << " fmt=" << fmt << " keylen=" << key.size() << "\n";
        auto raw_from = msg->hasPar("mail_from") ? std::string(msg->par("mail_from").stringValue()) : std::string();
        auto raw_to = msg->hasPar("mail_to") ? std::string(msg->par("mail_to").stringValue()) : std::string();
        auto raw_subj = msg->hasPar("mail_subject") ? std::string(msg->par("mail_subject").stringValue()) : std::string();
        auto raw_body = msg->hasPar("mail_body") ? std::string(msg->par("mail_body").stringValue()) : std::string();
        EV << "Receiver raw sizes: from=" << raw_from.size() << " to=" << raw_to.size() << " subj=" << raw_subj.size() << " body=" << raw_body.size() << "\n";

        std::string from = msg->hasPar("mail_from") ? maybeDec(msg->par("mail_from").stringValue()) : std::string("<unknown>");
        std::string to = msg->hasPar("mail_to") ? maybeDec(msg->par("mail_to").stringValue()) : std::string("<unknown>");
        std::string subj = msg->hasPar("mail_subject") ? maybeDec(msg->par("mail_subject").stringValue()) : std::string("<none>");
        std::string body = msg->hasPar("mail_body") ? maybeDec(msg->par("mail_body").stringValue()) : std::string("<empty>");
        EV << "Receiver got mail\n  From: " << from << "\n  To: " << to << "\n  Subject: " << subj << "\n  Body: " << body << "\n";
    }
    delete msg;  
}