#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Client_SS : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 500;     // central DNS
    const char* serverQName = "mta_server_rs";
    long pendingSmtpDst = -1;
    enum State { IDLE=0, RESOLVING, DH_HANDSHAKE, GREETING, ENVELOPE_FROM, ENVELOPE_RCPT, DATA_CMD, DATA_BODY, DONE, RETRY_WAIT };
    State state = IDLE;
    std::string content;
    std::string mailFrom, mailTo, mailSubject, mailBody;
    long declaredSize = -1;
    int attempt = 0;
    simtime_t backoff = 0;
    long dhPriv = 0;
    long dhPub = 0;
    std::string sessionKey;
    
    // RSA Keys
    RSAKeyPair myRSAKeys;
    unsigned long long serverRSAPublicE = 0;
    unsigned long long serverRSAPublicN = 0;
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void startResolution(cMessage* orig);
    void sendCmd(const char* verb, long dst);
    void sendMailFrom(long dst);
    void sendRcptTo(long dst);
    void sendDataCmd(long dst);
    void sendDataBody(long dst);
};

Define_Module(MTA_Client_SS);

// ---- Implementations ----
void MTA_Client_SS::initialize() {
    addr = par("address");
    
    // Generate RSA key pair
    myRSAKeys = generateRSAKeys();
    EV << "MTA_Client_SS[" << addr << "] Generated RSA keys: n=" << myRSAKeys.n 
       << " e=" << myRSAKeys.e << " d=" << myRSAKeys.d << endl;
}

void MTA_Client_SS::handleMessage(cMessage *msg) {
    if (msg->getKind() == PUSH_REQUEST) {
        // capture fields
        content   = msg->hasPar("content") ? msg->par("content").stringValue() : "";
        mailFrom  = msg->hasPar("mail_from") ? msg->par("mail_from").stringValue() : "";
        mailTo    = msg->hasPar("mail_to") ? msg->par("mail_to").stringValue() : "";
        mailSubject = msg->hasPar("mail_subject") ? msg->par("mail_subject").stringValue() : "";
        mailBody  = msg->hasPar("mail_body") ? msg->par("mail_body").stringValue() : "";
        declaredSize = (long)(mailBody.size());
        attempt = 0; backoff = 0;
        startResolution(msg);
        delete msg; return;
    }
    if (msg->getKind() == DNS_RESPONSE) {
        pendingSmtpDst = msg->par("answer").longValue();
        // begin toy Diffie-Hellman handshake
        state = DH_HANDSHAKE;
        // choose toy private key
        dhPriv = 12345 + (addr % 1000);
        dhPub = toyDH_pub(dhPriv);
        auto *hello = mk("SMTP_CMD", SMTP_CMD, addr, pendingSmtpDst);
        hello->addPar("verb").setStringValue("DH_HELLO");
        hello->addPar("dh_pub").setLongValue(dhPub);
        
        // ADD RSA PUBLIC KEY TO HANDSHAKE
        hello->addPar("rsa_e").setDoubleValue((double)myRSAKeys.e);
        hello->addPar("rsa_n").setDoubleValue((double)myRSAKeys.n);
        
        EV << "MTA_Client_SS[" << addr << "] Sending RSA public key: e=" << myRSAKeys.e 
           << " n=" << myRSAKeys.n << endl;
        
        send(hello, "ppp$o", 1);
        delete msg; return;
    }
    if (msg->getKind() == SMTP_RESP) {
        int code = msg->par("code").longValue();
        if (state == DH_HANDSHAKE) {
            if (code/100 == 2 && msg->hasPar("dh_pub")) {
                long serverPub = msg->par("dh_pub").longValue();
                long shared = toyDH_shared(dhPriv, serverPub);
                sessionKey = keyFromShared(shared);
                
                // RECEIVE SERVER'S RSA PUBLIC KEY
                if (msg->hasPar("rsa_e") && msg->hasPar("rsa_n")) {
                    serverRSAPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
                    serverRSAPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
                    
                    EV << "MTA_Client_SS[" << addr << "] Received server RSA public key: e=" 
                       << serverRSAPublicE << " n=" << serverRSAPublicN << endl;
                }
                
                state = GREETING;
                sendCmd("EHLO", DST(msg));
            } else {
                state = RETRY_WAIT; attempt++; backoff = std::min( SimTime( (double)attempt*0.1 ), SimTime(5.0) ); scheduleAt(simTime()+backoff, new cMessage("retry", PUSH_REQUEST));
            }
        } else if (state == GREETING) {
            if (code/100 == 2) { state = ENVELOPE_FROM; sendMailFrom(DST(msg)); }
            else { /* treat as temp fail */ state = RETRY_WAIT; attempt++; backoff = std::min( SimTime( (double)attempt*0.1 ), SimTime(5.0) ); scheduleAt(simTime()+backoff, new cMessage("retry", PUSH_REQUEST)); }
        } else if (state == ENVELOPE_FROM) {
            if (code/100 == 2) { state = ENVELOPE_RCPT; sendRcptTo(DST(msg)); }
            else if (code == 552) { /* too big, give up permanently */ state = DONE; }
            else if (code/100 == 4) { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
            else { state = DONE; }
        } else if (state == ENVELOPE_RCPT) {
            if (code/100 == 2) { state = DATA_CMD; sendDataCmd(DST(msg)); }
            else if (code/100 == 5) { state = DONE; }
            else { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
        } else if (state == DATA_CMD) {
            if (code == 354) { state = DATA_BODY; sendDataBody(DST(msg)); }
            else if (code/100 == 4) { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
            else { state = DONE; }
        } else if (state == DATA_BODY) {
            if (code/100 == 2) { state = DONE; }
            else if (code == 552) { state = DONE; }
            else if (code/100 == 4) { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
            else { state = DONE; }
        }
        delete msg; return;
    }
    if (msg->isSelfMessage() && msg->getKind() == PUSH_REQUEST) {
        startResolution(msg);
        delete msg; return;
    }
    delete msg;
}

void MTA_Client_SS::startResolution(cMessage* orig) {
    state = RESOLVING;
    auto *q = mk("DNS_QUERY", DNS_QUERY, addr, dnsAddr);
    q->addPar("qname").setStringValue(serverQName);
    send(q, "ppp$o", 1);
}

void MTA_Client_SS::sendCmd(const char* verb, long dst) {
    auto *m = mk("SMTP_CMD", SMTP_CMD, addr, dst);
    m->addPar("verb").setStringValue(verb);
    if (strcmp(verb, "EHLO")==0) m->addPar("arg1").setStringValue("client.local");
    send(m, "ppp$o", 1);
}

void MTA_Client_SS::sendMailFrom(long dst) {
    auto *m = mk("SMTP_CMD", SMTP_CMD, addr, dst);
    m->addPar("verb").setStringValue("MAIL");
    // Encrypt with SERVER's RSA public key
    auto encFrom = rsaEncryptToHex(mailFrom, serverRSAPublicE, serverRSAPublicN);
    m->addPar("from").setStringValue(encFrom.c_str());
    m->addPar("enc").setBoolValue(true);
    m->addPar("enc_fmt").setStringValue("rsahex");
    m->addPar("size").setLongValue(declaredSize);
    send(m, "ppp$o", 1);
}

void MTA_Client_SS::sendRcptTo(long dst) {
    auto *m = mk("SMTP_CMD", SMTP_CMD, addr, dst);
    m->addPar("verb").setStringValue("RCPT");
    // Encrypt with SERVER's RSA public key
    auto encTo = rsaEncryptToHex(mailTo, serverRSAPublicE, serverRSAPublicN);
    m->addPar("to").setStringValue(encTo.c_str());
    m->addPar("enc").setBoolValue(true);
    m->addPar("enc_fmt").setStringValue("rsahex");
    send(m, "ppp$o", 1);
}

void MTA_Client_SS::sendDataCmd(long dst) {
    auto *m = mk("SMTP_CMD", SMTP_CMD, addr, dst);
    m->addPar("verb").setStringValue("DATA");
    send(m, "ppp$o", 1);
}

void MTA_Client_SS::sendDataBody(long dst) {
    auto *m = mk("SMTP_CMD", SMTP_CMD, addr, dst);
    m->addPar("verb").setStringValue("DATA_END");
    m->addPar("bytes").setLongValue(declaredSize);
    // Encrypt content and headers with SERVER's RSA public key
    m->addPar("content").setStringValue(rsaEncryptToHex(content, serverRSAPublicE, serverRSAPublicN).c_str());
    m->addPar("mail_subject").setStringValue(rsaEncryptToHex(mailSubject, serverRSAPublicE, serverRSAPublicN).c_str());
    m->addPar("mail_body").setStringValue(rsaEncryptToHex(mailBody, serverRSAPublicE, serverRSAPublicN).c_str());
    m->addPar("enc").setBoolValue(true);
    m->addPar("enc_fmt").setStringValue("rsahex");
    send(m, "ppp$o", 1);
}