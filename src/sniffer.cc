#include <omnetpp.h>
#include "helpers.h"
#include <fstream>
#include <iomanip>
using namespace omnetpp;

class MaliciousSniffer : public cSimpleModule {
  private:
    std::ofstream logFile;
    int packetsIntercepted = 0;
    
    // Diffie-Hellman MITM Attack variables
    long dhPrivSender = 0;    // Sniffer's private key for Sender session
    long dhPrivReceiver = 0;  // Sniffer's private key for Receiver session
    long dhPubSender = 0;     // Sniffer's public key sent to Sender
    long dhPubReceiver = 0;   // Sniffer's public key sent to Receiver
    long sharedSecretSender = 0;    // Shared secret with Sender (mta_Client_SS)
    long sharedSecretReceiver = 0;  // Shared secret with Receiver (mta_Server_RS)
    std::string sessionKeySender;   // Session key with Sender
    std::string sessionKeyReceiver; // Session key with Receiver
    
    // RSA MITM Attack variables
    RSAKeyPair snifferRSAKeys;  // Sniffer's own RSA keys
    unsigned long long senderRSAPublicE = 0;
    unsigned long long senderRSAPublicN = 0;
    unsigned long long receiverRSAPublicE = 0;
    unsigned long long receiverRSAPublicN = 0;
    
    bool mitmActive = false;
    
    // Store addresses
    int senderAddr = 0;    // mta_Client_SS address
    int receiverAddr = 0;  // mta_Server_RS address
    
    // Captured RSA public keys
    unsigned long long clientRSAPublicE = 0;
    unsigned long long clientRSAPublicN = 0;
    unsigned long long serverRSAPublicE = 0;
    unsigned long long serverRSAPublicN = 0;
    
    // Derived private keys (from cryptographic attack)
    unsigned long long clientRSAPrivateD = 0;
    unsigned long long serverRSAPrivateD = 0;
    
    bool clientKeysCompromised = false;
    bool serverKeysCompromised = false;
    
    // Attack function: Factor n and derive private key
    bool attackRSA(unsigned long long e, unsigned long long n, bool isClient) {
        EV << "\n";
        EV << "+-----------------------------------------------------------+\n";
        EV << "| CRYPTOGRAPHIC ATTACK IN PROGRESS!                         |\n";
        EV << "| Target: " << (isClient ? "Client" : "Server") << " RSA Key                                    |\n";
        EV << "| Method: Integer Factorization                             |\n";
        EV << "+-----------------------------------------------------------+\n";
        
        logFile << "\n[=== CRYPTOGRAPHIC ATTACK ===]" << std::endl;
        logFile << "Target: " << (isClient ? "Client" : "Server") << " RSA Encryption" << std::endl;
        logFile << "Public Key: e=" << e << ", n=" << n << std::endl;
        logFile << "Attack Method: Brute-force integer factorization" << std::endl;
        
        simtime_t startTime = simTime();
        
        // Factor n to find p and q
        auto [p, q] = factorize(n);
        
        if (p != 0 && q != 0) {
            simtime_t attackDuration = simTime() - startTime;
            
            EV << "| [SUCCESS] Factored n = p * q                              |\n";
            EV << "|   p = " << p << "                                                 |\n";
            EV << "|   q = " << q << "                                                 |\n";
            EV << "| Attack duration: " << attackDuration << " seconds               |\n";
            
            logFile << "\n[FACTORIZATION SUCCESSFUL!]" << std::endl;
            logFile << "  p = " << p << std::endl;
            logFile << "  q = " << q << std::endl;
            logFile << "  Attack duration: " << attackDuration << " seconds" << std::endl;
            
            // Calculate phi(n) = (p-1)(q-1)
            unsigned long long phi = (p - 1) * (q - 1);
            
            // Calculate private exponent d = e^(-1) mod phi(n)
            unsigned long long d = modInverse(e, phi);
            
            if (d != 0) {
                EV << "|\n";
                EV << "| [CRITICAL] PRIVATE KEY DERIVED:                           |\n";
                EV << "|   d = " << d << "                                              |\n";
                EV << "|\n";
                EV << "+-----------------------------------------------------------+\n";
                EV << "| *** RSA ENCRYPTION COMPLETELY BROKEN! ***                 |\n";
                EV << "| *** ALL FUTURE MESSAGES WILL BE DECRYPTED! ***            |\n";
                EV << "+-----------------------------------------------------------+\n\n";
                
                logFile << "\n[PRIVATE KEY SUCCESSFULLY DERIVED]" << std::endl;
                logFile << "  Private key d = " << d << std::endl;
                logFile << "  *** ENCRYPTION COMPROMISED ***" << std::endl;
                logFile << std::string(50, '=') << std::endl;
                
                // Store the derived private key
                if (isClient) {
                    clientRSAPrivateD = d;
                    clientKeysCompromised = true;
                } else {
                    serverRSAPrivateD = d;
                    serverKeysCompromised = true;
                }
                
                return true;
            }
        }
        
        EV << "| [FAILED] Could not factor n (prime or too large)         |\n";
        EV << "+-----------------------------------------------------------+\n\n";
        
        logFile << "\n[ATTACK FAILED]" << std::endl;
        logFile << "Could not factor n - may be prime or too large" << std::endl;
        
        return false;
    }
    
  protected:
    void initialize() override {
        logFile.open("intercepted_traffic.log", std::ios::out);
        if (logFile.is_open()) {
            logFile << "============================================================" << std::endl;
            logFile << "    MALICIOUS SNIFFER (Man-in-the-Middle) ACTIVATED        " << std::endl;
            logFile << "============================================================" << std::endl;
            logFile << "Attack Type: Man-in-the-Middle on DH + RSA Key Exchange" << std::endl;
            logFile << "Position: Between Sender (mta_Client_SS) and Receiver (mta_Server_RS)" << std::endl;
            logFile << "Simulation Start Time: " << simTime() << std::endl;
            logFile << "============================================================" << std::endl << std::endl;
        }
        
        // Generate Sniffer's DH private keys for both sessions
        dhPrivSender = intuniform(2, 100);
        dhPrivReceiver = intuniform(2, 100);
        
        // Generate Sniffer's RSA keys
        snifferRSAKeys = generateRSAKeys();
        
        EV << "\n";
        EV << "+=============================================================+\n";
        EV << "|                                                             |\n";
        EV << "|        MALICIOUS SNIFFER ACTIVATED (MITM ATTACK)            |\n";
        EV << "|                                                             |\n";
        EV << "+=============================================================+\n";
        EV << "| Attack: Man-in-the-Middle on DH + RSA                       |\n";
        EV << "| Position: Sender <-> Sniffer <-> Receiver                   |\n";
        EV << "+=============================================================+\n";
        EV << "\n";
        EV << "Sniffer's DH private keys:\n";
        EV << "  For Sender (mta_Client_SS) session: " << dhPrivSender << "\n";
        EV << "  For Receiver (mta_Server_RS) session: " << dhPrivReceiver << "\n";
        EV << "\n";
        EV << "Sniffer's RSA keys:\n";
        EV << "  e = " << snifferRSAKeys.e << "\n";
        EV << "  n = " << snifferRSAKeys.n << "\n";
        EV << "  d = " << snifferRSAKeys.d << " (private - will be kept secret)\n";
        EV << "\n";
    }
    
    void finish() override {
        if (logFile.is_open()) {
            logFile << "\n============================================================" << std::endl;
            logFile << "           MITM ATTACK SUMMARY                              " << std::endl;
            logFile << "============================================================" << std::endl;
            logFile << "Total packets intercepted: " << packetsIntercepted << std::endl;
            logFile << "MITM Active: " << (mitmActive ? "YES - ATTACK SUCCESSFUL" : "NO") << std::endl;
            if (mitmActive) {
                logFile << "\nTwo separate sessions established:" << std::endl;
                logFile << "  Sender <-> Sniffer:" << std::endl;
                logFile << "    Shared secret: " << sharedSecretSender << std::endl;
                logFile << "    Session key: " << sessionKeySender << std::endl;
                logFile << "  Sniffer <-> Receiver:" << std::endl;
                logFile << "    Shared secret: " << sharedSecretReceiver << std::endl;
                logFile << "    Session key: " << sessionKeyReceiver << std::endl;
                logFile << "\nSender thinks: Sender <-> Receiver" << std::endl;
                logFile << "Receiver thinks: Sender <-> Receiver" << std::endl;
                logFile << "Reality: Sender <-> Sniffer <-> Receiver" << std::endl;
            }
            logFile << "============================================================" << std::endl;
            logFile.close();
        }
        
        EV << "\n";
        EV << "+=============================================================+\n";
        EV << "|  Malicious Sniffer Statistics:                              |\n";
        EV << "|  Total packets intercepted: " << packetsIntercepted << "                            |\n";
        EV << "|  MITM Attack Status: " << (mitmActive ? "SUCCESS" : "INACTIVE") << "                            |\n";
        EV << "+=============================================================+\n";
    }
    
    void handleMessage(cMessage *msg) override {
        packetsIntercepted++;
        cGate *arrivalGate = msg->getArrivalGate();
        int arrivalIndex = arrivalGate->getIndex();
        int outGateIndex = (arrivalIndex == 0) ? 1 : 0;
        
        double currentTime = simTime().dbl();
        
        // ===== INTERCEPT DH_HELLO FROM SENDER (mta_Client_SS) =====
        if (msg->getKind() == SMTP_CMD && msg->hasPar("verb")) {
            std::string verb = msg->par("verb").stringValue();
            
            if (verb == "DH_HELLO" && arrivalIndex == 0) {
                senderAddr = msg->par("src").longValue();
                receiverAddr = msg->par("dst").longValue();
                
                // Extract Sender's real public keys
                long senderRealDHPub = msg->par("dh_pub").longValue();
                senderRSAPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
                senderRSAPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
                
                EV << "\n";
                EV << "+=============================================================+\n";
                EV << "| MITM ATTACK: Intercepted DH_HELLO from SENDER               |\n";
                EV << "| Time: " << std::fixed << std::setprecision(6) << currentTime << " seconds                                      |\n";
                EV << "+=============================================================+\n";
                EV << "| Sender's real DH public key: " << senderRealDHPub << "                             |\n";
                EV << "| Sender's RSA public key: e=" << senderRSAPublicE << " n=" << senderRSAPublicN << "                    |\n";
                EV << "+=============================================================+\n\n";
                
                logFile << "\n[MITM ATTACK - DH_HELLO FROM SENDER]" << std::endl;
                logFile << "Time: " << currentTime << " seconds" << std::endl;
                logFile << "Sender's real DH public key: " << senderRealDHPub << std::endl;
                logFile << "Sender's RSA: e=" << senderRSAPublicE << " n=" << senderRSAPublicN << std::endl;
                
                // ATTACK: Calculate shared secret with Sender
                sharedSecretSender = modExp(senderRealDHPub, dhPrivSender, 23);
                sessionKeySender = keyFromShared(sharedSecretSender);
                
                EV << "ATTACK Phase 1: Sniffer calculates shared secret with Sender:\n";
                EV << "  k_Sender-Sniffer = (Sender_pub)^sniffer_priv\n";
                EV << "                   = " << senderRealDHPub << "^" << dhPrivSender << " mod 23\n";
                EV << "                   = " << sharedSecretSender << "\n";
                EV << "  Session key: " << sessionKeySender << "\n\n";
                
                logFile << "Sniffer's shared secret with Sender: " << sharedSecretSender << std::endl;
                logFile << "Session key with Sender: " << sessionKeySender << std::endl;
                
                // SUBSTITUTE: Replace Sender's public keys with Sniffer's
                long g = 5, p = 23;
                dhPubReceiver = modExp(g, dhPrivReceiver, p);  // Sniffer's public key for Receiver
                
                EV << "ATTACK Phase 2: Sniffer substitutes Sender's DH public key:\n";
                EV << "  Original from Sender: " << senderRealDHPub << "\n";
                EV << "  Substituted by Sniffer: g^sniffer_priv = 5^" << dhPrivReceiver << " mod 23 = " << dhPubReceiver << "\n";
                EV << "  (Receiver will receive " << dhPubReceiver << " instead of " << senderRealDHPub << ")\n\n";
                
                logFile << "Sniffer substitutes DH public key: " << senderRealDHPub << " -> " << dhPubReceiver << std::endl;
                
                // Modify the message
                msg->par("dh_pub").setLongValue(dhPubReceiver);  // Replace with Sniffer's key
                msg->par("rsa_e").setDoubleValue((double)snifferRSAKeys.e);  // Sniffer's RSA
                msg->par("rsa_n").setDoubleValue((double)snifferRSAKeys.n);
                
                EV << "ATTACK Phase 3: Sniffer also substitutes RSA public keys:\n";
                EV << "  Original RSA from Sender: e=" << senderRSAPublicE << " n=" << senderRSAPublicN << "\n";
                EV << "  Substituted RSA by Sniffer: e=" << snifferRSAKeys.e << " n=" << snifferRSAKeys.n << "\n";
                EV << "  (Receiver thinks it's from Sender, but gets Sniffer's keys!)\n\n";
                
                logFile << "Sniffer substitutes RSA keys: e=" << senderRSAPublicE << " n=" << senderRSAPublicN 
                        << " -> e=" << snifferRSAKeys.e << " n=" << snifferRSAKeys.n << std::endl;
                
                EV << "+-------------------------------------------------------------+\n";
                EV << "| Phase 1 Complete: Sender <-> Sniffer session established   |\n";
                EV << "| Forwarding modified message to Receiver...                  |\n";
                EV << "+-------------------------------------------------------------+\n\n";
                
                // Forward modified message to Receiver
                send(msg, "ppp$o", outGateIndex);
                return;
            }
        }
        
        // ===== INTERCEPT DH_HANDSHAKE FROM RECEIVER (mta_Server_RS) =====
        if (msg->getKind() == SMTP_RESP && arrivalIndex == 1) {
            // Extract Receiver's real public keys
            if (msg->hasPar("dh_pub") && msg->hasPar("rsa_e") && msg->hasPar("rsa_n")) {
                long receiverRealDHPub = msg->par("dh_pub").longValue();
                receiverRSAPublicE = (unsigned long long)msg->par("rsa_e").doubleValue();
                receiverRSAPublicN = (unsigned long long)msg->par("rsa_n").doubleValue();
                
                EV << "\n";
                EV << "+=============================================================+\n";
                EV << "| MITM ATTACK: Intercepted DH_HANDSHAKE from RECEIVER         |\n";
                EV << "| Time: " << std::fixed << std::setprecision(6) << currentTime << " seconds                                      |\n";
                EV << "+=============================================================+\n";
                EV << "| Receiver's real DH public key: " << receiverRealDHPub << "                          |\n";
                EV << "| Receiver's RSA public key: e=" << receiverRSAPublicE << " n=" << receiverRSAPublicN << "                    |\n";
                EV << "+=============================================================+\n\n";
                
                logFile << "\n[MITM ATTACK - DH_HANDSHAKE FROM RECEIVER]" << std::endl;
                logFile << "Time: " << currentTime << " seconds" << std::endl;
                logFile << "Receiver's real DH public key: " << receiverRealDHPub << std::endl;
                logFile << "Receiver's RSA: e=" << receiverRSAPublicE << " n=" << receiverRSAPublicN << std::endl;
                
                // ATTACK: Calculate shared secret with Receiver
                sharedSecretReceiver = modExp(receiverRealDHPub, dhPrivReceiver, 23);
                sessionKeyReceiver = keyFromShared(sharedSecretReceiver);
                
                EV << "ATTACK Phase 4: Sniffer calculates shared secret with Receiver:\n";
                EV << "  k_Sniffer-Receiver = (Receiver_pub)^sniffer_priv\n";
                EV << "                     = " << receiverRealDHPub << "^" << dhPrivReceiver << " mod 23\n";
                EV << "                     = " << sharedSecretReceiver << "\n";
                EV << "  Session key: " << sessionKeyReceiver << "\n\n";
                
                logFile << "Sniffer's shared secret with Receiver: " << sharedSecretReceiver << std::endl;
                logFile << "Session key with Receiver: " << sessionKeyReceiver << std::endl;
                
                // SUBSTITUTE: Replace Receiver's public keys with Sniffer's
                long g = 5, p = 23;
                dhPubSender = modExp(g, dhPrivSender, p);  // Sniffer's public key for Sender
                
                EV << "ATTACK Phase 5: Sniffer substitutes Receiver's DH public key:\n";
                EV << "  Original from Receiver: " << receiverRealDHPub << "\n";
                EV << "  Substituted by Sniffer: g^sniffer_priv = 5^" << dhPrivSender << " mod 23 = " << dhPubSender << "\n";
                EV << "  (Sender will receive " << dhPubSender << " instead of " << receiverRealDHPub << ")\n\n";
                
                logFile << "Sniffer substitutes DH public key: " << receiverRealDHPub << " -> " << dhPubSender << std::endl;
                
                // Modify the message
                msg->par("dh_pub").setLongValue(dhPubSender);  // Replace with Sniffer's key
                msg->par("rsa_e").setDoubleValue((double)snifferRSAKeys.e);  // Sniffer's RSA
                msg->par("rsa_n").setDoubleValue((double)snifferRSAKeys.n);
                
                EV << "ATTACK Phase 6: Sniffer also substitutes RSA public keys:\n";
                EV << "  Original RSA from Receiver: e=" << receiverRSAPublicE << " n=" << receiverRSAPublicN << "\n";
                EV << "  Substituted RSA by Sniffer: e=" << snifferRSAKeys.e << " n=" << snifferRSAKeys.n << "\n";
                EV << "  (Sender thinks it's from Receiver, but gets Sniffer's keys!)\n\n";
                
                logFile << "Sniffer substitutes RSA keys: e=" << receiverRSAPublicE << " n=" << receiverRSAPublicN 
                        << " -> e=" << snifferRSAKeys.e << " n=" << snifferRSAKeys.n << std::endl;
                
                mitmActive = true;
                
                EV << "+=============================================================+\n";
                EV << "|                                                             |\n";
                EV << "|           *** MITM ATTACK SUCCESSFUL ***                    |\n";
                EV << "|                                                             |\n";
                EV << "+=============================================================+\n";
                EV << "| Two separate sessions established:                          |\n";
                EV << "|   Sender <-> Sniffer (shared secret: " << sharedSecretSender << ")                    |\n";
                EV << "|   Sniffer <-> Receiver (shared secret: " << sharedSecretReceiver << ")                  |\n";
                EV << "|                                                             |\n";
                EV << "| Sender thinks: Sender <-> Receiver                          |\n";
                EV << "| Receiver thinks: Sender <-> Receiver                        |\n";
                EV << "| Reality: Sender <-> Sniffer <-> Receiver                    |\n";
                EV << "|                                                             |\n";
                EV << "| Sniffer can now decrypt and re-encrypt all messages!        |\n";
                EV << "+=============================================================+\n\n";
                
                logFile << "\n" << std::string(60, '=') << std::endl;
                logFile << "MITM ATTACK SUCCESSFUL!" << std::endl;
                logFile << "Two separate sessions established:" << std::endl;
                logFile << "  Sender <-> Sniffer: shared=" << sharedSecretSender 
                        << " key=" << sessionKeySender << std::endl;
                logFile << "  Sniffer <-> Receiver: shared=" << sharedSecretReceiver 
                        << " key=" << sessionKeyReceiver << std::endl;
                logFile << std::string(60, '=') << std::endl;
                
                // Forward modified message to Sender
                send(msg, "ppp$o", outGateIndex);
                return;
            }
        }
        
        // ===== INTERCEPT ENCRYPTED SMTP MESSAGES =====
        if (msg->getKind() == SMTP_CMD && mitmActive) {
            std::string verb = msg->hasPar("verb") ? msg->par("verb").stringValue() : "";
            
            EV << "\n";
            EV << "+-------------------------------------------------------------+\n";
            EV << "| MITM: Intercepted SMTP message from " << (arrivalIndex == 0 ? "SENDER  " : "RECEIVER") << "              |\n";
            EV << "| Time: " << std::fixed << std::setprecision(6) << currentTime << " seconds                                      |\n";
            EV << "| Command: " << verb << std::string(55 - verb.length(), ' ') << "|\n";
            EV << "+-------------------------------------------------------------+\n";
            
            logFile << "\n[MITM - SMTP INTERCEPTION]" << std::endl;
            logFile << "Time: " << currentTime << " seconds" << std::endl;
            logFile << "From: " << (arrivalIndex == 0 ? "Sender" : "Receiver") << std::endl;
            logFile << "SMTP Command: " << verb << std::endl;
            
            // Decrypt message from Sender using Sniffer's RSA private key
            if (arrivalIndex == 0) {  // From Sender
                bool decryptedAny = false;
                
                if (msg->hasPar("from")) {
                    std::string encFrom = msg->par("from").stringValue();
                    std::string decrypted = rsaDecryptFromHex(encFrom, snifferRSAKeys.d, snifferRSAKeys.n);
                    if (!decrypted.empty()) {
                        EV << "| DECRYPTED FROM: " << decrypted << std::string(41 - decrypted.length(), ' ') << "|\n";
                        logFile << "DECRYPTED FROM: " << decrypted << std::endl;
                        decryptedAny = true;
                        
                        // RE-ENCRYPT for Receiver using Receiver's real RSA public key
                        std::string reEncrypted = rsaEncryptToHex(decrypted, receiverRSAPublicE, receiverRSAPublicN);
                        msg->par("from").setStringValue(reEncrypted.c_str());
                        EV << "| (Re-encrypted for Receiver)                                 |\n";
                    }
                }
                
                if (msg->hasPar("to")) {
                    std::string encTo = msg->par("to").stringValue();
                    std::string decrypted = rsaDecryptFromHex(encTo, snifferRSAKeys.d, snifferRSAKeys.n);
                    if (!decrypted.empty()) {
                        EV << "| DECRYPTED TO: " << decrypted << std::string(43 - decrypted.length(), ' ') << "|\n";
                        logFile << "DECRYPTED TO: " << decrypted << std::endl;
                        decryptedAny = true;
                        
                        std::string reEncrypted = rsaEncryptToHex(decrypted, receiverRSAPublicE, receiverRSAPublicN);
                        msg->par("to").setStringValue(reEncrypted.c_str());
                    }
                }
                
                if (msg->hasPar("mail_subject")) {
                    std::string encSubject = msg->par("mail_subject").stringValue();
                    std::string decrypted = rsaDecryptFromHex(encSubject, snifferRSAKeys.d, snifferRSAKeys.n);
                    if (!decrypted.empty()) {
                        EV << "| DECRYPTED SUBJECT: " << decrypted << std::string(38 - decrypted.length(), ' ') << "|\n";
                        logFile << "DECRYPTED SUBJECT: " << decrypted << std::endl;
                        decryptedAny = true;
                        
                        std::string reEncrypted = rsaEncryptToHex(decrypted, receiverRSAPublicE, receiverRSAPublicN);
                        msg->par("mail_subject").setStringValue(reEncrypted.c_str());
                    }
                }
                
                if (msg->hasPar("mail_body")) {
                    std::string encBody = msg->par("mail_body").stringValue();
                    std::string decrypted = rsaDecryptFromHex(encBody, snifferRSAKeys.d, snifferRSAKeys.n);
                    if (!decrypted.empty()) {
                        EV << "| DECRYPTED BODY: " << decrypted.substr(0, 41) << std::string(41 - std::min((int)decrypted.length(), 41), ' ') << "|\n";
                        logFile << "DECRYPTED BODY: " << decrypted << std::endl;
                        decryptedAny = true;
                        
                        std::string reEncrypted = rsaEncryptToHex(decrypted, receiverRSAPublicE, receiverRSAPublicN);
                        msg->par("mail_body").setStringValue(reEncrypted.c_str());
                    }
                }
                
                if (msg->hasPar("content")) {
                    std::string encContent = msg->par("content").stringValue();
                    std::string decrypted = rsaDecryptFromHex(encContent, snifferRSAKeys.d, snifferRSAKeys.n);
                    if (!decrypted.empty()) {
                        EV << "| DECRYPTED CONTENT: " << decrypted.substr(0, 38) << std::string(38 - std::min((int)decrypted.length(), 38), ' ') << "|\n";
                        logFile << "DECRYPTED CONTENT: " << decrypted << std::endl;
                        decryptedAny = true;
                        
                        std::string reEncrypted = rsaEncryptToHex(decrypted, receiverRSAPublicE, receiverRSAPublicN);
                        msg->par("content").setStringValue(reEncrypted.c_str());
                    }
                }
                
                if (decryptedAny) {
                    EV << "+-------------------------------------------------------------+\n";
                    EV << "| Message decrypted, read, and re-encrypted transparently!    |\n";
                    EV << "| Forwarding to Receiver...                                   |\n";
                    EV << "+-------------------------------------------------------------+\n\n";
                    
                    logFile << "Message re-encrypted and forwarded to Receiver" << std::endl;
                }
            }
        }
        
        // Forward all messages
        send(msg, "ppp$o", outGateIndex);
    }
};

Define_Module(MaliciousSniffer);
