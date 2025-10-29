#include <omnetpp.h>
#include "helpers.h"
#include <map>
using namespace omnetpp;

// Certificate structure is now defined in helpers.h

class CA : public cSimpleModule {
  private:
    int addr = 900;  // CA address
    RSAKeyPair caRSAKeys;  // CA's own RSA keys for signing
    std::map<int, Certificate> issuedCertificates;  // address -> certificate
    bool enabled = true;  // Can be disabled for Scenario 1
    int requestsReceived = 0;  // Counter for debugging
    int responseSent = 0;      // Counter for debugging
    
  protected:
    void initialize() override {
        addr = par("address");
        
        // Check if CA is enabled (for different scenarios)
        enabled = par("enabled").boolValue();
        
        if (!enabled) {
            EV << "\n";
            EV << "+=============================================================+\n";
            EV << "| CERTIFICATE AUTHORITY (CA) DISABLED                         |\n";
            EV << "| (Scenario 1: Demonstrating MITM vulnerability)              |\n";
            EV << "+=============================================================+\n\n";
            return;
        }
        
        // Generate CA's RSA keys
        caRSAKeys = generateRSAKeys();
        
        EV << "\n";
        EV << "+=============================================================+\n";
        EV << "|           CERTIFICATE AUTHORITY (CA) INITIALIZED            |\n";
        EV << "+=============================================================+\n";
        EV << "| CA Address: " << addr << "                                              |\n";
        EV << "| CA RSA Public Key:                                          |\n";
        EV << "|   e = " << caRSAKeys.e << "                                              |\n";
        EV << "|   n = " << caRSAKeys.n << "                                            |\n";
        EV << "| CA RSA Private Key (kept secret):                           |\n";
        EV << "|   d = " << caRSAKeys.d << "                                            |\n";
        EV << "+=============================================================+\n";
        EV << "| CA is ready to issue certificates!                          |\n";
        EV << "+=============================================================+\n\n";
    }
    
    void handleMessage(cMessage *msg) override {
        // If CA is disabled, ignore all requests
        if (!enabled) {
            delete msg;
            return;
        }
        
        EV << "CA: Received message kind=" << msg->getKind() << " name=" << msg->getName() << "\n";
        
        if (msg->getKind() == CERT_REQUEST) {  // Use constant instead of hardcoded 70
            handleCertificateRequest(msg);
        } else {
            EV << "CA: Ignoring message with kind=" << msg->getKind() << "\n";
        }
        delete msg;
    }
    
    void handleCertificateRequest(cMessage *msg) {
        requestsReceived++;
        
        int requestorAddr = msg->par("src").longValue();
        std::string identity = msg->par("identity").stringValue();
        unsigned long long rsaE = (unsigned long long)msg->par("rsa_e").doubleValue();
        unsigned long long rsaN = (unsigned long long)msg->par("rsa_n").doubleValue();
        unsigned long long dhPub = msg->par("dh_pub").longValue();
        
        EV << "\n┌─────────────────────────────────────────────────────────┐\n";
        EV << "│ CA: Received Certificate Request #" << requestsReceived << "                       │\n";
        EV << "└─────────────────────────────────────────────────────────┘\n";
        EV << "From: " << identity << " (address " << requestorAddr << ")\n";
        EV << "RSA Public Key: e=" << rsaE << " n=" << rsaN << "\n";
        EV << "DH Public Key: " << dhPub << "\n\n";
        
        // Create certificate
        Certificate cert;
        cert.identity = identity;
        cert.address = requestorAddr;
        cert.rsaPublicE = rsaE;
        cert.rsaPublicN = rsaN;
        cert.dhPublicKey = dhPub;
        cert.timestamp = simTime().dbl();
        cert.validUntil = (simTime() + 3600.0).dbl();  // Valid for 1 hour
        
        // Sign certificate with CA's private key using the standard signing function
        cert.signature = signCertificate(cert, caRSAKeys.d, caRSAKeys.n);
        
        // Store certificate
        issuedCertificates[requestorAddr] = cert;
        
        EV << "CA: Certificate issued and signed!\n";
        EV << "  Signature: " << cert.signature.substr(0, 30) << "...\n";
        EV << "  Valid until: " << cert.validUntil << "\n\n";
        
        // Send certificate back
        auto *resp = mk("CERT_RESPONSE", CERT_RESPONSE, addr, requestorAddr);  // Use constant instead of hardcoded 71
        resp->addPar("identity").setStringValue(cert.identity.c_str());
        resp->addPar("address").setLongValue(cert.address);  // Add address field
        resp->addPar("rsa_e").setDoubleValue((double)cert.rsaPublicE);
        resp->addPar("rsa_n").setDoubleValue((double)cert.rsaPublicN);
        resp->addPar("dh_pub").setLongValue(cert.dhPublicKey);
        resp->addPar("signature").setStringValue(cert.signature.c_str());
        resp->addPar("ca_rsa_e").setDoubleValue((double)caRSAKeys.e);
        resp->addPar("ca_rsa_n").setDoubleValue((double)caRSAKeys.n);
        resp->addPar("timestamp").setDoubleValue(cert.timestamp);
        resp->addPar("valid_until").setDoubleValue(cert.validUntil);
        
        EV << "CA: Preparing to send certificate response...\n";
        EV << "  From (CA): " << addr << "\n";
        EV << "  To: " << requestorAddr << "\n";
        EV << "  Message kind: " << resp->getKind() << " (CERT_RESPONSE=" << CERT_RESPONSE << ")\n";
        
        // Find the gate to send response
        cGate *gateOut = gate("ppp$o");
        send(resp, gateOut);
        responseSent++;
        
        EV << "CA: Certificate #" << responseSent << " successfully sent to " << identity << " at address " << requestorAddr << "\n";
        EV << "CA: Total requests received: " << requestsReceived << ", responses sent: " << responseSent << "\n\n";
    }
};

Define_Module(CA);
