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
        auto isMostlyPrintable = [](const std::string &str){
            if (str.empty()) return false;
            int printable = 0;
            for (unsigned char c : str) {
                if (c == '\r' || c == '\n' || (c >= 32 && c <= 126)) printable++;
            }
            return printable * 1.0 / (int)str.size() > 0.8; // heuristic
        };
        auto looksLikeHex = [](const std::string &s){
            if (s.size() < 2 || (s.size() % 2) != 0) return false;
            for (unsigned char c : s) {
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
            }
            return true;
        };
        auto maybeDec = [&](const char* p, const char* field){
            if (!p) return std::string();
            std::string s = p;
            std::string out;
            if (s.empty()) return out;

            if (enc) {
                // All fields (from, to, subject, body, content) now use RSA encryption
                if (fmt == "rsahex") {
                    out = rsaDecryptFromHex(s);
                    if (!out.empty() && isMostlyPrintable(out)) {
                        return out;
                    }
                } else if (!key.empty()) {
                    // Fallback for XOR encryption if used
                    if (fmt == "hex") {
                        out = xorDecrypt(fromHex(s), key);
                    } else {
                        out = xorDecrypt(s, key);
                    }
                    if (!out.empty() && isMostlyPrintable(out)) {
                        return out;
                    }
                }
            }
            // General fallback: if still not decrypted and we have a key and input looks like hex, try hex+XOR
            if (!key.empty() && looksLikeHex(s)) {
                std::string tryX = xorDecrypt(fromHex(s), key);
                if (!tryX.empty() && isMostlyPrintable(tryX)) {
                    return tryX;
                }
            }
            // Last resort: return original or decode hex for non-encrypted
            if (!enc && fmt == "hex") {
                std::string tryHex = fromHex(s);
                if (!tryHex.empty() && isMostlyPrintable(tryHex)) {
                    return tryHex;
                }
            }
            // If all decryption attempts failed, return original encrypted string
            return s;
        };
        // debug
        EV << "Receiver decrypt meta: enc=" << enc << " fmt=" << fmt << " keylen=" << key.size() << "\n";
        auto raw_from = msg->hasPar("mail_from") ? std::string(msg->par("mail_from").stringValue()) : std::string();
        auto raw_to = msg->hasPar("mail_to") ? std::string(msg->par("mail_to").stringValue()) : std::string();
        auto raw_subj = msg->hasPar("mail_subject") ? std::string(msg->par("mail_subject").stringValue()) : std::string();
        auto raw_body = msg->hasPar("mail_body") ? std::string(msg->par("mail_body").stringValue()) : std::string();
        EV << "Receiver raw sizes: from=" << raw_from.size() << " to=" << raw_to.size() << " subj=" << raw_subj.size() << " body=" << raw_body.size() << "\n";

        // Determine per-field fmts if provided
        std::string fromFmt = msg->hasPar("mail_from_fmt") ? std::string(msg->par("mail_from_fmt").stringValue()) : fmt;
        std::string toFmt   = msg->hasPar("mail_to_fmt") ? std::string(msg->par("mail_to_fmt").stringValue()) : fmt;
        // Temporarily override fmt during per-field decode
        auto decWithFmt = [&](const char* raw, const char* field, const std::string &fieldFmt){
            std::string prevFmt = fmt;
            fmt = fieldFmt;
            std::string outv = maybeDec(raw, field);
            fmt = prevFmt;
            return outv;
        };
        std::string from = msg->hasPar("mail_from") ? decWithFmt(msg->par("mail_from").stringValue(), "mail_from", fromFmt) : std::string("<unknown>");
        std::string to = msg->hasPar("mail_to") ? decWithFmt(msg->par("mail_to").stringValue(), "mail_to", toFmt) : std::string("<unknown>");
        std::string subj = msg->hasPar("mail_subject") ? maybeDec(msg->par("mail_subject").stringValue(), "mail_subject") : std::string("<none>");
        std::string body = msg->hasPar("mail_body") ? maybeDec(msg->par("mail_body").stringValue(), "mail_body") : std::string("<empty>");
        std::string content = msg->hasPar("content") ? maybeDec(msg->par("content").stringValue(), "content") : std::string();
        EV << "Receiver got mail\n  From: " << from << "\n  To: " << to << "\n  Subject: " << subj << "\n  Body: " << body << "\n";
        if (!content.empty()) {
            EV << "Receiver decrypted content: " << content << "\n";
        }
    }
    delete msg;  
}