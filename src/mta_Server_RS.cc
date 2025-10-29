#include <omnetpp.h>
#include "helpers.h"
using std::map;
using std::string;
using namespace omnetpp;

class MTA_Server_RS : public cSimpleModule {
  private:
    int addr = 0;
    int mailboxAddr = 700;
    long maxMessageSizeBytes = 1000000;
    
    // RSA Keys
    RSAKeyPair myRSAKeys;

    enum SessionState { STATE_INIT=0, STATE_GREETED, STATE_MAIL, STATE_RCPT, STATE_DATA_PENDING };
    struct SessionContext {
        SessionState state = STATE_INIT;
        string heloName;
        string mailFrom;
        string mailFromFmt; // remembers how MAIL FROM was encoded
        string rcptTo;
        string rcptToFmt;   // remembers how RCPT TO was encoded
        long declaredSize = -1;
        // toy DH
        long dhPriv = 0;
        long dhPub = 0;
        long peerPub = 0;
        string sessionKey;
        // Client's RSA public key
        unsigned long long clientRSAPublicE = 0;
        unsigned long long clientRSAPublicN = 0;
    };
    map<long, SessionContext> sessions; // key: client logical addr
  protected:
    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void handleSmtpCmd(cMessage* msg);
    void respond(long dst, int code, const char* message, bool includeExtensions=false);
};

Define_Module(MTA_Server_RS);

// ---- Implementations ----
void MTA_Server_RS::initialize() {
    addr = par("address");
    if (hasPar("maxMessageSizeBytes")) maxMessageSizeBytes = par("maxMessageSizeBytes");
    
    // Generate RSA key pair
    myRSAKeys = generateRSAKeys();
    EV << "MTA_Server_RS[" << addr << "] Generated RSA keys: n=" << myRSAKeys.n 
       << " e=" << myRSAKeys.e << " d=" << myRSAKeys.d << endl;
}

void MTA_Server_RS::handleMessage(cMessage *msg) {
    if (msg->getKind() == SMTP_CMD) {
        handleSmtpCmd(msg);
        delete msg;
        return;
    } else if (msg->getKind() == SMTP_SEND) {
        // Legacy direct send: accept, ack, and deliver to mailbox
        auto *ack = mk("SMTP_ACK", SMTP_ACK, addr, SRC(msg));
        send(ack, "ppp$o", 0);
        auto *toMb = mk("SMTP_SEND", SMTP_SEND, addr, mailboxAddr);
        if (msg->hasPar("content")) toMb->addPar("content").setStringValue(msg->par("content").stringValue());
        if (msg->hasPar("mail_from")) toMb->addPar("mail_from").setStringValue(msg->par("mail_from").stringValue());
        if (msg->hasPar("mail_to")) toMb->addPar("mail_to").setStringValue(msg->par("mail_to").stringValue());
        if (msg->hasPar("mail_subject")) toMb->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) toMb->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(toMb, "ppp$o", 1);
    }
    delete msg;  
}

void MTA_Server_RS::respond(long dst, int code, const char* message, bool includeExtensions) {
    auto *resp = mk("SMTP_RESP", SMTP_RESP, addr, dst);
    resp->addPar("code").setLongValue(code);
    resp->addPar("message").setStringValue(message);
    if (includeExtensions) {
        resp->addPar("ext_size").setLongValue(maxMessageSizeBytes);
        resp->addPar("ext_8bitmime").setBoolValue(true);
        resp->addPar("ext_starttls").setBoolValue(true);
    }
    send(resp, "ppp$o", 0);
}

void MTA_Server_RS::handleSmtpCmd(cMessage* msg) {
    long client = SRC(msg);
    auto &ctx = sessions[client];
    const char* verb = msg->hasPar("verb") ? msg->par("verb").stringValue() : "";
    if (strcmp(verb, "DH_HELLO") == 0) {
        // receive client pub, send our pub back
        ctx.dhPriv = 54321 + (addr % 1000);
        ctx.dhPub = toyDH_pub(ctx.dhPriv);
        ctx.peerPub = msg->hasPar("dh_pub") ? msg->par("dh_pub").longValue() : 0;
        long shared = toyDH_shared(ctx.dhPriv, ctx.peerPub);
        ctx.sessionKey = keyFromShared(shared);
        
        // RECEIVE CLIENT'S RSA PUBLIC KEY
        if (msg->hasPar("rsa_e") && msg->hasPar("rsa_n")) {
            ctx.clientRSAPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
            ctx.clientRSAPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
            
            EV << "MTA_Server_RS[" << addr << "] Received client RSA public key: e=" 
               << ctx.clientRSAPublicE << " n=" << ctx.clientRSAPublicN << endl;
        }
        
        auto *resp = mk("SMTP_RESP", SMTP_RESP, addr, client);
        resp->addPar("code").setLongValue(250);
        resp->addPar("message").setStringValue("dh ok");
        resp->addPar("dh_pub").setLongValue(ctx.dhPub);
        
        // SEND SERVER'S RSA PUBLIC KEY
        resp->addPar("rsa_e").setDoubleValue((double)myRSAKeys.e);
        resp->addPar("rsa_n").setDoubleValue((double)myRSAKeys.n);
        
        EV << "MTA_Server_RS[" << addr << "] Sending RSA public key: e=" << myRSAKeys.e 
           << " n=" << myRSAKeys.n << endl;
        
        send(resp, "ppp$o", 0);
        return;
    }
    if (strcmp(verb, "EHLO") == 0 || strcmp(verb, "HELO") == 0) {
        ctx.state = STATE_GREETED;
        ctx.heloName = msg->hasPar("arg1") ? msg->par("arg1").stringValue() : "";
        respond(client, 250, "ok", true);
        return;
    }
    if (strcmp(verb, "MAIL") == 0) {
        if (ctx.state < STATE_GREETED) { respond(client, 503, "bad sequence", false); return; }
        // Store as-received (may be encrypted). Decryption will be deferred until fetch time.
        ctx.mailFrom = msg->hasPar("from") ? msg->par("from").stringValue() : "";
        ctx.mailFromFmt = msg->hasPar("enc_fmt") ? msg->par("enc_fmt").stringValue() : "";
        ctx.declaredSize = msg->hasPar("size") ? msg->par("size").longValue() : -1;
        if (ctx.declaredSize >= 0 && ctx.declaredSize > maxMessageSizeBytes) { respond(client, 552, "message too big", false); return; }
        ctx.state = STATE_MAIL;
        respond(client, 250, "ok", false);
        return;
    }
    if (strcmp(verb, "RCPT") == 0) {
        if (ctx.state < STATE_MAIL) { respond(client, 503, "bad sequence", false); return; }
        // Store as-received (may be encrypted). Decryption will be deferred until fetch time.
        ctx.rcptTo = msg->hasPar("to") ? msg->par("to").stringValue() : "";
        ctx.rcptToFmt = msg->hasPar("enc_fmt") ? msg->par("enc_fmt").stringValue() : "";
        // Accept recipient; could add 550/551/553 here
        ctx.state = STATE_RCPT;
        respond(client, 250, "ok", false);
        return;
    }
    if (strcmp(verb, "DATA") == 0) {
        if (ctx.state < STATE_RCPT) { respond(client, 503, "bad sequence", false); return; }
        ctx.state = STATE_DATA_PENDING;
        respond(client, 354, "end with <CRLF>.<CRLF>", false);
        return;
    }
    if (strcmp(verb, "DATA_END") == 0) {
        if (ctx.state != STATE_DATA_PENDING) { respond(client, 503, "bad sequence", false); return; }
        long bytes = msg->hasPar("bytes") ? msg->par("bytes").longValue() : 0;
        if (bytes > maxMessageSizeBytes) { respond(client, 552, "message too big", false); return; }
        // Deliver to mailbox (keep encrypted if enc=true); attach enc metadata and session key for later decryption at fetch time
        auto *toMb = mk("SMTP_SEND", SMTP_SEND, addr, mailboxAddr);
        if (msg->hasPar("enc")) toMb->addPar("enc").setBoolValue(msg->par("enc").boolValue());
        if (msg->hasPar("enc_fmt")) toMb->addPar("enc_fmt").setStringValue(msg->par("enc_fmt").stringValue());
        if (!ctx.sessionKey.empty()) toMb->addPar("enc_key").setStringValue(ctx.sessionKey.c_str());
        if (msg->hasPar("content")) toMb->addPar("content").setStringValue(msg->par("content").stringValue());
        if (!ctx.mailFrom.empty()) toMb->addPar("mail_from").setStringValue(ctx.mailFrom.c_str());
        if (!ctx.mailFromFmt.empty()) toMb->addPar("mail_from_fmt").setStringValue(ctx.mailFromFmt.c_str());
        if (!ctx.rcptTo.empty()) toMb->addPar("mail_to").setStringValue(ctx.rcptTo.c_str());
        if (!ctx.rcptToFmt.empty()) toMb->addPar("mail_to_fmt").setStringValue(ctx.rcptToFmt.c_str());
        if (msg->hasPar("mail_subject")) toMb->addPar("mail_subject").setStringValue(msg->par("mail_subject").stringValue());
        if (msg->hasPar("mail_body")) toMb->addPar("mail_body").setStringValue(msg->par("mail_body").stringValue());
        send(toMb, "ppp$o", 1);
        respond(client, 250, "queued", false);
        sessions.erase(client);
        return;
    }
    if (strcmp(verb, "STARTTLS") == 0) {
        // No-op for TLS modeling; pretend success
        respond(client, 220, "ready to start TLS", false);
        return;
    }
    if (strcmp(verb, "RSET") == 0) {
        sessions.erase(client);
        respond(client, 250, "ok", false);
        return;
    }
    if (strcmp(verb, "QUIT") == 0) {
        sessions.erase(client);
        respond(client, 221, "bye", false);
        return;
    }
    respond(client, 502, "command not implemented", false);
}