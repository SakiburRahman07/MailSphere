#ifndef MODULES_HELPERS_H_
#define MODULES_HELPERS_H_

#include <omnetpp.h>
#include <map>
#include <sstream>
#include <string>
#include <random>
#include <functional>
#include <iomanip>
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
    NOTIFY_NEWMAIL=60,
    DH_HELLO=65, DH_HANDSHAKE=66, DH_HANDSHAKE_ACK=67,
    CERT_REQUEST=70, CERT_RESPONSE=71
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

// --- Toy crypto utilities (simulation only, not real security) ---
// Derive a pseudo-random byte stream from a shared key string
static inline void deriveKeystream(const std::string &key, std::vector<unsigned char> &out, size_t length) {
    out.resize(length);
    std::seed_seq seed(key.begin(), key.end());
    std::mt19937 rng(seed);
    for (size_t i = 0; i < length; ++i) out[i] = static_cast<unsigned char>(rng() & 0xFF);
}

static inline std::string xorEncrypt(const std::string &plain, const std::string &key) {
    if (key.empty()) return plain;
    std::vector<unsigned char> ks; deriveKeystream(key, ks, plain.size());
    std::string out; out.resize(plain.size());
    for (size_t i=0;i<plain.size();++i) out[i] = static_cast<char>( (unsigned char)plain[i] ^ ks[i] );
    return out;
}

static inline std::string xorDecrypt(const std::string &cipher, const std::string &key) {
    return xorEncrypt(cipher, key);
}

// Hex encoding helpers to safely carry binary data in cMessage string params
static inline std::string toHex(const std::string &bytes) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : bytes) oss << std::setw(2) << (int)c;
    return oss.str();
}

static inline std::string fromHex(const std::string &hex) {
    std::string out;
    if (hex.size() % 2 != 0) return out;
    out.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned int byte = 0;
        std::istringstream iss(hex.substr(i, 2));
        iss >> std::hex >> byte;
        out.push_back(static_cast<char>(byte & 0xFF));
    }
    return out;
}

// Very small toy Diffie-Hellman using additive group over integers mod P (SIMULATION ONLY)
// P and G are small for simplicity; do not use for real crypto.
static inline long toyDH_P() { return 104729; } // a prime
static inline long toyDH_G() { return 5; }
static inline long toyDH_pub(long priv) { return ( (toyDH_G() * priv) % toyDH_P() ); }
static inline long toyDH_shared(long myPriv, long otherPub) { return ( (otherPub * myPriv) % toyDH_P() ); }

static inline std::string keyFromShared(long shared) {
    return std::string("k:") + std::to_string(shared);
}

// --- Toy RSA helpers (simulation only) ---
// Classic small RSA example: p=61, q=53 => n=3233, phi=3120, e=17, d=2753
static inline unsigned long long rsaN() { return 3233ULL; }
static inline unsigned long long rsaE() { return 17ULL; }
static inline unsigned long long rsaD() { return 2753ULL; }

// RSA Key Pair structure
struct RSAKeyPair {
    unsigned long long n;  // modulus (p * q)
    unsigned long long e;  // public exponent
    unsigned long long d;  // private exponent
};

// Extended Euclidean Algorithm for modular inverse
static inline long long extGCD(long long a, long long b, long long &x, long long &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    long long x1, y1;
    long long gcd = extGCD(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

// Modular inverse: find d such that (e * d) mod phi = 1
static inline unsigned long long modInverse(unsigned long long e, unsigned long long phi) {
    long long x, y;
    long long gcd = extGCD((long long)e, (long long)phi, x, y);
    if (gcd != 1) return 0; // Inverse doesn't exist
    return (unsigned long long)((x % (long long)phi + (long long)phi) % (long long)phi);
}

// Factor n to find p and q (brute force for small primes)
static inline std::pair<unsigned long long, unsigned long long> factorize(unsigned long long n) {
    for (unsigned long long i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return {i, n / i};
        }
    }
    return {0, 0}; // Failed to factor
}

// Generate RSA key pair with small primes (for educational attack demonstration)
static inline RSAKeyPair generateRSAKeys() {
    // Use small primes for toy RSA (easy to factor for demonstration)
    unsigned long long p = 61;   // First prime
    unsigned long long q = 53;   // Second prime
    unsigned long long n = p * q; // n = 3233
    
    // phi(n) = (p-1) * (q-1)
    unsigned long long phi = (p - 1) * (q - 1); // phi = 3120
    
    // Public exponent (commonly 65537, but we use 17 for simplicity)
    unsigned long long e = 17;
    
    // Calculate private exponent d such that (e * d) mod phi = 1
    unsigned long long d = modInverse(e, phi);
    
    return {n, e, d};
}

static inline unsigned long long modExp(unsigned long long base, unsigned long long exp, unsigned long long mod) {
    unsigned long long result = 1ULL % mod;
    unsigned long long b = base % mod;
    while (exp > 0ULL) {
        if (exp & 1ULL) result = (result * b) % mod;
        b = (b * b) % mod;
        exp >>= 1ULL;
    }
    return result;
}

// Encrypt bytes with RSA (per-byte) and output as hex of 2-byte blocks (big-endian)
// Default parameters for backward compatibility
static inline std::string rsaEncryptToHex(const std::string &plain, 
                                          unsigned long long e = 0, 
                                          unsigned long long n = 0) {
    // Use default values if not provided
    if (e == 0) e = rsaE();
    if (n == 0) n = rsaN();
    
    std::string raw;
    raw.reserve(plain.size() * 2);
    for (unsigned char ch : plain) {
        unsigned long long c = modExp((unsigned long long)ch, e, n);
        unsigned char hi = (unsigned char)((c >> 8) & 0xFF);
        unsigned char lo = (unsigned char)(c & 0xFF);
        raw.push_back((char)hi);
        raw.push_back((char)lo);
    }
    return toHex(raw);
}

// Decrypt from hex of 2-byte blocks (big-endian) into original bytes
// Default parameters for backward compatibility
static inline std::string rsaDecryptFromHex(const std::string &hexCipher,
                                            unsigned long long d = 0,
                                            unsigned long long n = 0) {
    // Use default values if not provided
    if (d == 0) d = rsaD();
    if (n == 0) n = rsaN();
    
    std::string raw = fromHex(hexCipher);
    std::string out;
    if (raw.size() % 2 != 0) return out;
    out.reserve(raw.size() / 2);
    for (size_t i = 0; i < raw.size(); i += 2) {
        unsigned int hi = (unsigned char)raw[i];
        unsigned int lo = (unsigned char)raw[i+1];
        unsigned long long c = ((unsigned long long)hi << 8) | (unsigned long long)lo;
        unsigned long long m = modExp(c, d, n);
        out.push_back((char)(m & 0xFF));
    }
    return out;
}

// --- Certificate Structure for PKI ---
struct Certificate {
    std::string identity;           // "Sender" or "Receiver"
    int address;                    // Network address
    unsigned long long rsaPublicE;  // RSA public exponent
    unsigned long long rsaPublicN;  // RSA modulus
    long dhPublicKey;              // DH public key
    std::string signature;          // CA's signature (encrypted hash)
    double timestamp;               // Issue time
    double validUntil;              // Expiration time
    
    Certificate() : address(0), rsaPublicE(0), rsaPublicN(0), dhPublicKey(0), 
                    timestamp(0), validUntil(0) {}
};

// Create certificate data string for signing/verification
static inline std::string certificateDataString(const Certificate& cert) {
    std::ostringstream oss;
    oss << cert.identity << ":"
        << cert.address << ":"
        << cert.rsaPublicE << ":"
        << cert.rsaPublicN << ":"
        << cert.dhPublicKey << ":"
        << std::fixed << std::setprecision(6) << cert.timestamp;
    return oss.str();
}

// Sign certificate with CA's private key
static inline std::string signCertificate(const Certificate& cert, 
                                         unsigned long long caPrivateD,
                                         unsigned long long caPublicN) {
    std::string dataToSign = certificateDataString(cert);
    return rsaEncryptToHex(dataToSign, caPrivateD, caPublicN);
}

// Verify certificate signature with CA's public key
static inline bool verifyCertificate(const Certificate& cert,
                                    unsigned long long caPublicE,
                                    unsigned long long caPublicN,
                                    double currentTime) {
    // Check if certificate has expired
    if (currentTime > cert.validUntil) {
        return false;
    }
    
    // Verify signature
    std::string expectedData = certificateDataString(cert);
    // Use provided CA pubkey if available; otherwise fall back to default CA pubkey (rsaE/rsaN)
    unsigned long long useE = (caPublicE != 0ULL) ? caPublicE : rsaE();
    unsigned long long useN = (caPublicN != 0ULL) ? caPublicN : rsaN();
    std::string decryptedSig = rsaDecryptFromHex(cert.signature, useE, useN);
    
    return (decryptedSig == expectedData);
}


#endif /* MODULES_HELPERS_H_ */
