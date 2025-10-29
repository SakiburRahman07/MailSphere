#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MAA_Server : public cSimpleModule {
  private:
    int addr = 0;
    int mailboxAddr = 700;
    long pendingClient = -1;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
};

Define_Module(MAA_Server);

// ---- Implementations ----
void MAA_Server::initialize() {
    addr = par("address");
}

void MAA_Server::handleMessage(cMessage *msg) {
    if (msg->getKind() == HTTP_GET) {
        pendingClient = SRC(msg);
        auto *fetch = mk("IMAP_FETCH", IMAP_FETCH, addr, mailboxAddr);
        send(fetch, "ppp$o", 0);
    } else if (msg->getKind() == IMAP_RESPONSE) {
        auto *resp = mk("HTTP_RESPONSE", HTTP_RESPONSE, addr, pendingClient);
        resp->addPar("bytes").setLongValue(20000);
        bool enc = msg->hasPar("enc") ? msg->par("enc").boolValue() : false;
        std::string fmt = msg->hasPar("enc_fmt") ? std::string(msg->par("enc_fmt").stringValue()) : std::string();
        std::string key = msg->hasPar("enc_key") ? std::string(msg->par("enc_key").stringValue()) : std::string();
        auto maybeDec = [&](const char* p){
            if (!p) return std::string();
            std::string s = p;
            if (enc && !key.empty()) {
                if (fmt == "hex") s = xorDecrypt(fromHex(s), key);
                else s = xorDecrypt(s, key);
            }
            return s;
        };
        // propagate metadata; Receiver will decide how to handle (RSA decrypt at edge)
        if (msg->hasPar("enc")) resp->addPar("enc").setBoolValue(msg->par("enc").boolValue());
        if (msg->hasPar("enc_fmt")) resp->addPar("enc_fmt").setStringValue(msg->par("enc_fmt").stringValue());
        if (msg->hasPar("enc_key")) resp->addPar("enc_key").setStringValue(msg->par("enc_key").stringValue());

        auto decContent = msg->hasPar("content") ? maybeDec(msg->par("content").stringValue()) : std::string();
        auto decFrom = msg->hasPar("mail_from") ? maybeDec(msg->par("mail_from").stringValue()) : std::string();
        auto decTo = msg->hasPar("mail_to") ? maybeDec(msg->par("mail_to").stringValue()) : std::string();
        auto decSubj = msg->hasPar("mail_subject") ? maybeDec(msg->par("mail_subject").stringValue()) : std::string();
        auto decBody = msg->hasPar("mail_body") ? maybeDec(msg->par("mail_body").stringValue()) : std::string();
        EV << "MAA_Server decrypt: enc=" << enc << " fmt=" << fmt << " keylen=" << key.size() 
           << " subjPreview=" << (decSubj.size()>16?decSubj.substr(0,16):decSubj) << "\n";
        // Forward original values; Receiver will RSA-decrypt
        if (msg->hasPar("content")) resp->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) resp->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_from_fmt")) resp->addPar("mail_from_fmt").setStringValue(msg->par("mail_from_fmt").stringValue());
        if (msg->hasPar("mail_to")) resp->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_to_fmt")) resp->addPar("mail_to_fmt").setStringValue(msg->par("mail_to_fmt").stringValue());
        if (msg->hasPar("mail_subject")) resp->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) resp->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(resp, "ppp$o", 1);
        pendingClient = -1;
    } else if (msg->getKind() == NOTIFY_NEWMAIL) {
        // Forward notification to MAA_Client to trigger pull
        auto *note = mk("NOTIFY_NEWMAIL", NOTIFY_NEWMAIL, addr, 850 /* maa_client */);
        send(note, "ppp$o", 1);
    }
    delete msg;  
}