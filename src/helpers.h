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
    NOTIFY_NEWMAIL=60
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
static inline std::string rsaEncryptToHex(const std::string &plain) {
    std::string raw;
    raw.reserve(plain.size() * 2);
    for (unsigned char ch : plain) {
        unsigned long long c = modExp((unsigned long long)ch, rsaE(), rsaN());
        unsigned char hi = (unsigned char)((c >> 8) & 0xFF);
        unsigned char lo = (unsigned char)(c & 0xFF);
        raw.push_back((char)hi);
        raw.push_back((char)lo);
    }
    return toHex(raw);
}

// Decrypt from hex of 2-byte blocks (big-endian) into original bytes
static inline std::string rsaDecryptFromHex(const std::string &hexCipher) {
    std::string raw = fromHex(hexCipher);
    std::string out;
    if (raw.size() % 2 != 0) return out;
    out.reserve(raw.size() / 2);
    for (size_t i = 0; i < raw.size(); i += 2) {
        unsigned int hi = (unsigned char)raw[i];
        unsigned int lo = (unsigned char)raw[i+1];
        unsigned long long c = ((unsigned long long)hi << 8) | (unsigned long long)lo;
        unsigned long long m = modExp(c, rsaD(), rsaN());
        out.push_back((char)(m & 0xFF));
    }
    return out;
}


#endif /* MODULES_HELPERS_H_ */
