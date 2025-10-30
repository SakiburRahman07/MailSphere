#include <omnetpp.h>
#include "helpers.h"
using namespace omnetpp;

class MTA_Client_SS : public cSimpleModule {
  private:
    int addr = 0;
    int dnsAddr = 650;     // central DNS (CORRECTED from 500 to 650)
    int caAddr = 950;      // Certificate Authority address (CORRECTED from 900 to 950)
    const char* serverQName = "mta_server_rs";
    long pendingSmtpDst = -1;
    enum State { IDLE=0, RESOLVING, STATE_CERT_REQUEST, STATE_CERT_WAIT, DH_HANDSHAKE, GREETING, ENVELOPE_FROM, ENVELOPE_RCPT, DATA_CMD, DATA_BODY, DONE, RETRY_WAIT };
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
    
    // Certificate support
    bool useCertificates = false;
    Certificate myCertificate;
    Certificate serverCertificate;
    bool certificateReceived = false;
    unsigned long long caPublicE = 0;
    unsigned long long caPublicN = 0;
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
    
    // Check if certificates are enabled
    useCertificates = hasPar("useCertificates") && par("useCertificates").boolValue();
    
    // Generate RSA key pair
    myRSAKeys = generateRSAKeys();
    EV << "MTA_Client_SS[" << addr << "] Generated RSA keys: n=" << myRSAKeys.n 
       << " e=" << myRSAKeys.e << " d=" << myRSAKeys.d << endl;
    
    // Generate DH keys
    dhPriv = 12345 + (addr % 1000);
    dhPub = toyDH_pub(dhPriv);
    
    // If certificates enabled, request certificate from CA
    if (useCertificates) {
        EV << "\n┌─────────────────────────────────────────────────────────┐\n";
        EV << "│ SENDER: Requesting certificate from CA...              │\n";
        EV << "└─────────────────────────────────────────────────────────┘\n";
        
        auto *certReq = mk("CERT_REQUEST", CERT_REQUEST, addr, caAddr);
        certReq->addPar("identity").setStringValue("Sender");
        certReq->addPar("rsa_e").setDoubleValue((double)myRSAKeys.e);
        certReq->addPar("rsa_n").setDoubleValue((double)myRSAKeys.n);
        certReq->addPar("dh_pub").setLongValue(dhPub);
        
        send(certReq, "ppp$o", 1);
        state = STATE_CERT_REQUEST;
    }
}

void MTA_Client_SS::handleMessage(cMessage *msg) {
    EV << "MTA_Client_SS: Received message kind=" << msg->getKind() << " name=" << msg->getName() 
       << " (CERT_RESPONSE=" << CERT_RESPONSE << ")\n";
    
    // Handle certificate response from CA
    if (msg->getKind() == CERT_RESPONSE) {
        EV << "MTA_Client_SS: Processing CERT_RESPONSE from CA\n";
        myCertificate.identity = msg->par("identity").stringValue();
        myCertificate.address = msg->par("address").longValue();
        myCertificate.rsaPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
        myCertificate.rsaPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
        myCertificate.dhPublicKey = msg->par("dh_pub").longValue();
        myCertificate.signature = msg->par("signature").stringValue();
        myCertificate.timestamp = msg->par("timestamp").doubleValue();
        myCertificate.validUntil = msg->par("valid_until").doubleValue();
        
        // Store CA's public key for verification
        caPublicE = (unsigned long long)msg->par("ca_rsa_e").doubleValue();
        caPublicN = (unsigned long long)msg->par("ca_rsa_n").doubleValue();
        
        // VERIFY THIS IS OUR CERTIFICATE!
        if (myCertificate.address != addr) {
            EV << "\n╔═══════════════════════════════════════════════════════════╗\n";
            EV << "║ ✗ ERROR: Received certificate for wrong address!        ║\n";
            EV << "║ Expected: " << addr << " but got: " << myCertificate.address << "                                     ║\n";
            EV << "║ Discarding certificate and retrying request...          ║\n";
            EV << "╚═══════════════════════════════════════════════════════════╝\n\n";
            delete msg;
            // Request certificate again after delay
            scheduleAt(simTime() + 0.1, new cMessage("retry_cert"));
            return;
        }
        
        certificateReceived = true;
        
        EV << "\n╔═══════════════════════════════════════════════════════════╗\n";
        EV << "║ ✓ SENDER: Certificate received from CA!                 ║\n";
        EV << "╚═══════════════════════════════════════════════════════════╝\n";
        EV << "Identity: " << myCertificate.identity << "\n";
        EV << "Address: " << myCertificate.address << "\n";
        EV << "Signature: " << myCertificate.signature.substr(0, 30) << "...\n";
        EV << "Valid until: " << myCertificate.validUntil << "\n\n";
        
        state = IDLE;  // Ready for email sending
        delete msg;
        return;
    }
    
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
        // choose toy private key (if not already set)
        if (dhPriv == 0) {
            dhPriv = 12345 + (addr % 1000);
            dhPub = toyDH_pub(dhPriv);
        }
        auto *hello = mk("SMTP_CMD", SMTP_CMD, addr, pendingSmtpDst);
        hello->addPar("verb").setStringValue("DH_HELLO");
        hello->addPar("dh_pub").setLongValue(dhPub);
        
        // ADD RSA PUBLIC KEY TO HANDSHAKE
        hello->addPar("rsa_e").setDoubleValue((double)myRSAKeys.e);
        hello->addPar("rsa_n").setDoubleValue((double)myRSAKeys.n);
        
        // If using certificates, attach certificate
        if (useCertificates && certificateReceived) {
            hello->addPar("has_certificate").setBoolValue(true);
            hello->addPar("cert_identity").setStringValue(myCertificate.identity.c_str());
            hello->addPar("cert_address").setLongValue(myCertificate.address);
            hello->addPar("cert_signature").setStringValue(myCertificate.signature.c_str());
            hello->addPar("cert_timestamp").setDoubleValue(myCertificate.timestamp);
            hello->addPar("cert_valid_until").setDoubleValue(myCertificate.validUntil);
            
            EV << "\n┌─────────────────────────────────────────────────────────┐\n";
            EV << "│ SENDER: Sending certificate with DH_HELLO              │\n";
            EV << "└─────────────────────────────────────────────────────────┘\n\n";
        }
        
        EV << "MTA_Client_SS[" << addr << "] Sending RSA public key: e=" << myRSAKeys.e 
           << " n=" << myRSAKeys.n << endl;
        
        send(hello, "ppp$o", 1);
        delete msg; return;
    }
    if (msg->getKind() == SMTP_RESP) {
        int code = msg->par("code").longValue();
        if (state == DH_HANDSHAKE) {
            if (code/100 == 2 && msg->hasPar("dh_pub")) {
                // If using certificates, verify server's certificate first
                if (useCertificates && msg->hasPar("has_certificate") && msg->par("has_certificate").boolValue()) {
                    serverCertificate.identity = msg->par("cert_identity").stringValue();
                    serverCertificate.address = msg->par("cert_address").longValue();
                    serverCertificate.rsaPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
                    serverCertificate.rsaPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
                    serverCertificate.dhPublicKey = msg->par("dh_pub").longValue();
                    serverCertificate.signature = msg->par("cert_signature").stringValue();
                    serverCertificate.timestamp = msg->par("cert_timestamp").doubleValue();
                    serverCertificate.validUntil = msg->par("cert_valid_until").doubleValue();
                    
                    EV << "\n┌─────────────────────────────────────────────────────────┐\n";
                    EV << "│ SENDER: Verifying Receiver's certificate...            │\n";
                    EV << "└─────────────────────────────────────────────────────────┘\n";
                    
                    // Verify certificate
                    bool certValid = verifyCertificate(serverCertificate, caPublicE, caPublicN, simTime().dbl());
                    
                    if (!certValid) {
                        EV << "\n╔═══════════════════════════════════════════════════════════╗\n";
                        EV << "║ ✗ CERTIFICATE VERIFICATION FAILED!                       ║\n";
                        EV << "║ Receiver's certificate is INVALID or FORGED!             ║\n";
                        EV << "║ Connection REJECTED - Possible MITM attack detected!     ║\n";
                        EV << "╚═══════════════════════════════════════════════════════════╝\n\n";
                        
                        state = DONE;
                        delete msg;
                        return;
                    }
                    
                    EV << "  ✓ Certificate signature valid\n";
                    EV << "  ✓ Certificate not expired\n";
                    EV << "  ✓ Identity confirmed: " << serverCertificate.identity << "\n";
                    EV << "\n🔒 SECURE CONNECTION ESTABLISHED!\n\n";
                }
                
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
                sendCmd("EHLO", SRC(msg));
            } else {
                state = RETRY_WAIT; attempt++; backoff = std::min( SimTime( (double)attempt*0.1 ), SimTime(5.0) ); scheduleAt(simTime()+backoff, new cMessage("retry", PUSH_REQUEST));
            }
        } else if (state == GREETING) {
            if (code/100 == 2) { state = ENVELOPE_FROM; sendMailFrom(SRC(msg)); }
            else { /* treat as temp fail */ state = RETRY_WAIT; attempt++; backoff = std::min( SimTime( (double)attempt*0.1 ), SimTime(5.0) ); scheduleAt(simTime()+backoff, new cMessage("retry", PUSH_REQUEST)); }
        } else if (state == ENVELOPE_FROM) {
            if (code/100 == 2) { state = ENVELOPE_RCPT; sendRcptTo(SRC(msg)); }
            else if (code == 552) { /* too big, give up permanently */ state = DONE; }
            else if (code/100 == 4) { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
            else { state = DONE; }
        } else if (state == ENVELOPE_RCPT) {
            if (code/100 == 2) { state = DATA_CMD; sendDataCmd(SRC(msg)); }
            else if (code/100 == 5) { state = DONE; }
            else { state = RETRY_WAIT; attempt++; scheduleAt(simTime()+SimTime(attempt*0.2), new cMessage("retry", PUSH_REQUEST)); }
        } else if (state == DATA_CMD) {
            if (code == 354) { state = DATA_BODY; sendDataBody(SRC(msg)); }
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