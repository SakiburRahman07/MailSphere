# Email Simulation with Man-in-the-Middle Attack and Certificate-Based Mitigation

This project demonstrates a Man-in-the-Middle (MITM) attack on an email system and shows how certificate-based authentication can prevent it.

## 📊 Project Structure

```
project/
├── src/
│   ├── CA.cc                    # Certificate Authority module (NEW)
│   ├── sniffer.cc               # Malicious MITM attacker
│   ├── mta_Client_SS.cc         # Sender (SMTP client)
│   ├── mta_Server_RS.cc         # Receiver (SMTP server)
│   ├── helpers.h                # Crypto utilities (RSA, DH, certificates)
│   └── email.ned                # Network topology
└── simulations/
    └── omnetpp.ini              # Simulation configuration
```

## 🎯 Two Simulation Scenarios

### **Scenario 1: Vulnerable System (MITM Attack Succeeds)** 🔴

**What happens:**
1. **Sender** (mta_Client_SS) and **Receiver** (mta_Server_RS) perform Diffie-Hellman key exchange
2. **Sniffer** (positioned between them) intercepts the key exchange
3. **Sniffer substitutes** the public keys with its own keys
4. **Result:** Sniffer establishes TWO separate sessions:
   - Sender ↔ Sniffer (Sender thinks it's talking to Receiver)
   - Sniffer ↔ Receiver (Receiver thinks it's talking to Sender)
5. **Sniffer decrypts** all email content (FROM, TO, SUBJECT, BODY, CONTENT)
6. **Sniffer re-encrypts** and forwards messages transparently
7. **Neither party knows** they've been compromised!

**Console Output:**
```
╔═══════════════════════════════════════════════════════════╗
║ ★★★ MITM ATTACK SUCCESSFUL ★★★                          ║
║                                                           ║
║ Sniffer established TWO separate sessions:               ║
║   Sender ↔ Sniffer (shared secret: 15)                  ║
║   Sniffer ↔ Receiver (shared secret: 8)                 ║
║                                                           ║
║ DECRYPTED FROM: alice@example.com                        ║
║ DECRYPTED TO: bob@example.com                            ║
║ DECRYPTED CONTENT: Hello from Sender                     ║
╚═══════════════════════════════════════════════════════════╝
```

---

### **Scenario 2: Secure System with Certificates (MITM Attack Fails)** ✅

**What happens:**
1. **Certificate Authority (CA)** issues signed certificates to Sender and Receiver
2. Certificates contain: identity, address, RSA public key, DH public key, CA signature
3. **Sender** and **Receiver** request certificates from CA before key exchange
4. During key exchange, they **verify each other's certificates** using CA's public key
5. **Sniffer tries to substitute** keys, but the certificate signature doesn't match!
6. **Certificate verification fails** → Connection rejected
7. **Result:** MITM attack detected and prevented!

**Console Output:**
```
╔═══════════════════════════════════════════════════════════╗
║ CA: Certificate issued to Sender                         ║
║   Identity: Sender (mta_Client_SS)                       ║
║   RSA Public Key: e=17 n=3233                            ║
║   Signature: a7b3c9d2e5f8... (signed by CA)              ║
╚═══════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────┐
│ Receiver: Verifying Sender's certificate...             │
└─────────────────────────────────────────────────────────┘
  ✓ Certificate signature valid
  ✓ Certificate not expired
  ✓ Identity matches: Sender

┌─────────────────────────────────────────────────────────┐
│ Sniffer: Attempting MITM attack...                      │
└─────────────────────────────────────────────────────────┘
  ✗ ATTACK FAILED: Certificate verification rejected!
  ✗ Sniffer's fake certificate has invalid signature
  ✗ Connection refused by Receiver
```

---

## 🚀 How to Run

### **Step 1: Compile the project**
```bash
cd src
make clean
make MODE=release
```

### **Step 2: Run Scenario 1 (No Certificates - Attack Succeeds)**
```bash
cd ../simulations
../src/project.exe -u Cmdenv -c WithoutCertificates
```

### **Step 3: Run Scenario 2 (With Certificates - Attack Fails)**
```bash
../src/project.exe -u Cmdenv -c WithCertificates
```

---

## 📖 Educational Value

This simulation demonstrates:

1. **Diffie-Hellman Key Exchange** - How it works and its vulnerability to MITM
2. **Man-in-the-Middle Attack** - Active attack where attacker intercepts and modifies traffic
3. **Certificate-Based Authentication** - How digital certificates prevent MITM attacks
4. **Public Key Infrastructure (PKI)** - Role of Certificate Authority in establishing trust
5. **Practical Cryptography** - Real-world implementation of RSA, DH, and certificates

---

## 🔐 Cryptographic Details

### **Diffie-Hellman Parameters:**
- Generator (g): 5
- Prime modulus (p): 23
- Private keys: Random in range [2, 100]
- Shared secret: (g^a mod p)^b mod p

### **RSA Parameters:**
- Modulus (n): 3233 (p=61, q=53)
- Public exponent (e): 17
- Private exponent (d): 2753
- **Note:** Weak keys for educational purposes!

### **Certificate Structure:**
```
Certificate {
    identity: "Sender" or "Receiver"
    address: Network address
    rsaPublicE: RSA public exponent
    rsaPublicN: RSA modulus
    dhPublicKey: DH public key
    signature: RSA signature by CA
    timestamp: Issue time
    validUntil: Expiration time
}
```

---

## 🎓 Learning Outcomes

After running both scenarios, students will understand:

✅ Why Diffie-Hellman alone is vulnerable to MITM attacks
✅ How certificates bind public keys to identities
✅ The role of a Certificate Authority in establishing trust
✅ Why certificate verification is critical in secure communications
✅ How real-world protocols like TLS/SSL prevent MITM attacks

---

## 📝 Files Modified for Certificate Support

- `helpers.h` - Added certificate message types (CERT_REQUEST, CERT_RESPONSE)
- `CA.cc` - NEW: Certificate Authority module
- `email.ned` - Added CA module and connections
- `Makefile` - Added CA.o to build
- `mta_Client_SS.cc` - Will add certificate request and verification
- `mta_Server_RS.cc` - Will add certificate request and verification
- `sniffer.cc` - Will show MITM failure with certificates

---

## ⚠️ Security Note

**This is a TOY implementation for educational purposes!**

Real-world secure communications use:
- 2048-4096 bit RSA keys (not 12-bit!)
- Proper certificate chains and revocation
- TLS 1.3 with Perfect Forward Secrecy
- Hardware security modules for key storage
- And much more...

**Never use this code in production!**

---

## 📧 Network Topology

```
Sender → router1 → mta_Client_S → mta_Server_S → spool → mta_Client_SS
                                                              ↓
                                                    [Key Exchange]
                                                              ↓
                                                         🔴 Sniffer
                                                              ↓
                   ┌─────────────────────────────────────────┤
                   │                                          ↓
                   CA ← router →                          Router
                   │              ↓                           ↓
                   └──────────────┴────────────────→ mta_Server_RS
                                                              ↓
                                                         Mailbox
                                                              ↓
                                                        Receiver
```

---

## 🎯 Next Steps

1. ✅ Compile and run Scenario 1 (see MITM attack succeed)
2. ✅ Analyze `intercepted_traffic.log` to see stolen data
3. ✅ Run Scenario 2 (see MITM attack fail with certificates)
4. ✅ Compare console outputs to understand the difference
5. ✅ Experiment with certificate parameters and expiration

Enjoy learning about network security! 🔐
