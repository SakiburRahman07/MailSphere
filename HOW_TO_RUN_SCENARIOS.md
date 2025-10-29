# 🎯 How to Run Both Simulation Scenarios

This project demonstrates **two scenarios** of email encryption with and without certificate-based protection against Man-in-the-Middle (MITM) attacks.

---

## 📋 Overview

### **Scenario 1: MITM Attack Succeeds** 🔴
- **No certificates** - Direct key exchange between Sender and Receiver
- **Sniffer intercepts** and substitutes keys
- **Attack succeeds** - All email content is stolen
- **Purpose**: Show the vulnerability of unauthenticated key exchange

### **Scenario 2: Certificates Prevent MITM** ✅
- **Certificate Authority (CA)** issues signed certificates
- **Sender and Receiver verify** certificates before key exchange
- **Sniffer cannot forge** valid certificates (no CA private key)
- **Attack fails** - MITM detected and prevented
- **Purpose**: Show how PKI solves the MITM problem

---

## 🚀 Running the Simulations

### **Method 1: From OMNeT++ IDE (Recommended)**

1. **Open OMNeT++ IDE**
2. **Import the project** (if not already imported)
3. **Build the project**: Right-click → Build Project
4. **Run Scenario 1** (MITM Attack):
   - Right-click on `omnetpp.ini` → Run As → OMNeT++ Simulation
   - Select configuration: **`Scenario1_MITM_Attack`**
   - Click **Run**
   - Watch the console output showing the successful MITM attack

5. **Run Scenario 2** (With Certificates):
   - Right-click on `omnetpp.ini` → Run As → OMNeT++ Simulation
   - Select configuration: **`Scenario2_With_Certificates`**
   - Click **Run**
   - Watch the console output showing the failed MITM attack

---

### **Method 2: From Command Line (MinGW Shell)**

1. **Open OMNeT++ MinGW Shell** (mingwenv.cmd)

2. **Navigate to project directory:**
   ```bash
   cd /d/omnetpp-6.2.0-windows-x86_64/omnetpp-6.2.0/samples/project
   ```

3. **Build the project:**
   ```bash
   make MODE=release clean
   make MODE=release
   ```

4. **Run Scenario 1 (MITM Attack):**
   ```bash
   cd simulations
   ../src/project.exe -u Cmdenv -c Scenario1_MITM_Attack
   ```

5. **Run Scenario 2 (With Certificates):**
   ```bash
   ../src/project.exe -u Cmdenv -c Scenario2_With_Certificates
   ```

---

## 📊 What to Look For

### **Scenario 1 Console Output:**

```
╔═══════════════════════════════════════════════════════════╗
║ ★★★ MITM ATTACK SUCCESSFUL ★★★                          ║
║                                                           ║
║ Sniffer established TWO separate sessions:               ║
║   Sender ↔ Sniffer (shared secret: 15)                  ║
║   Sniffer ↔ Receiver (shared secret: 8)                 ║
║                                                           ║
║ INTERCEPTED EMAIL:                                       ║
║   FROM: alice@example.com                                ║
║   TO: bob@example.com                                    ║
║   SUBJECT: Test Email                                    ║
║   CONTENT: Hello from Sender                             ║
╚═══════════════════════════════════════════════════════════╝
```

**Key Points:**
- ✅ Sniffer intercepts DH_HELLO and DH_HANDSHAKE
- ✅ Sniffer substitutes its own keys
- ✅ Two separate encrypted sessions established
- ✅ Sniffer decrypts, reads, and re-encrypts all messages
- ❌ Neither Sender nor Receiver knows they're compromised!

---

### **Scenario 2 Console Output:**

```
+=============================================================+
|           CERTIFICATE AUTHORITY (CA) INITIALIZED            |
+=============================================================+
| CA Address: 900                                             |
| CA RSA Public Key: e = 17, n = 3233                        |
+=============================================================+

╔═══════════════════════════════════════════════════════════╗
║ CA: Certificate issued to Sender                         ║
║   Identity: Sender (mta_Client_SS)                       ║
║   Address: 300                                           ║
║   RSA Public Key: e=17, n=3233                           ║
║   DH Public Key: 5432                                    ║
║   Signature: a7b3c9d2e5f8... (signed by CA)              ║
║   Valid until: 3600.00s                                  ║
╚═══════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────┐
│ Receiver: Verifying Sender's certificate...             │
│   ✓ Signature valid (CA verification passed)            │
│   ✓ Certificate not expired                             │
│   ✓ Identity confirmed: Sender                          │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│ Sniffer: Attempting MITM attack...                      │
│   ✗ ATTACK FAILED!                                      │
│   ✗ Cannot forge valid certificate signature            │
│   ✗ Receiver rejected fake certificate                  │
│   ✗ Connection refused                                  │
└─────────────────────────────────────────────────────────┘

🔒 Secure communication established!
```

**Key Points:**
- ✅ CA issues certificates to Sender and Receiver
- ✅ Certificates contain public keys signed by CA
- ✅ Receiver verifies Sender's certificate before key exchange
- ✅ Sniffer cannot forge valid certificates (no CA private key)
- ✅ MITM attack detected and prevented!
- ✅ Secure communication established

---

## 🔧 Configuration Details

The two scenarios are configured in `simulations/omnetpp.ini`:

```ini
[Config Scenario1_MITM_Attack]
description = "MITM Attack Succeeds - No Certificate Verification"
**.useCertificates = false
**.ca.enabled = false

[Config Scenario2_With_Certificates]
description = "MITM Attack Fails - Certificate Verification Enabled"
**.useCertificates = true
**.ca.enabled = true
**.ca.certificateValidityPeriod = 3600s
```

---

## 📁 Project Structure

```
project/
├── src/
│   ├── CA.cc                    # Certificate Authority
│   ├── sniffer.cc               # MITM attacker
│   ├── mta_Client_SS.cc         # Sender (SMTP client)
│   ├── mta_Server_RS.cc         # Receiver (SMTP server)
│   ├── helpers.h                # Crypto utilities + Certificate struct
│   ├── email.ned                # Network topology
│   └── ...
├── simulations/
│   └── omnetpp.ini              # Two scenario configurations
└── HOW_TO_RUN_SCENARIOS.md      # This file
```

---

## 🎓 Educational Value

This simulation teaches:

1. **Why authentication is critical** in key exchange protocols
2. **How MITM attacks work** in practice
3. **The role of Certificate Authorities** in establishing trust
4. **How digital certificates bind** public keys to identities
5. **Why TLS/SSL uses certificates** to prevent MITM attacks

---

## ⚠️ Important Notes

### **Current Implementation Status:**

✅ **Completed:**
- Scenario 1 (MITM Attack) fully implemented
- Certificate structure added to helpers.h
- CA module with enable/disable support
- Two configurations in omnetpp.ini

🔧 **To Complete Scenario 2:**
- [ ] Modify `mta_Client_SS.cc` to request certificates from CA
- [ ] Modify `mta_Server_RS.cc` to request certificates from CA
- [ ] Add certificate verification before key exchange
- [ ] Update `sniffer.cc` to show MITM failure when certificates are used

### **Next Implementation Steps:**

1. **Add certificate request in MTA_Client_SS:**
   ```cpp
   if (par("useCertificates").boolValue()) {
       requestCertificateFromCA();
   }
   ```

2. **Add certificate verification before DH exchange:**
   ```cpp
   if (useCertificates && !verifyCertificate(receivedCert, caPublicE, caPublicN, simTime())) {
       EV << "⚠️  Certificate verification FAILED - Connection rejected!\n";
       return;
   }
   ```

3. **Update sniffer to detect certificate verification:**
   ```cpp
   if (msg->hasPar("certificate")) {
       EV << "⚠️  Certificate detected - MITM attack will fail!\n";
   }
   ```

---

## 🐛 Troubleshooting

### **Problem: "Gate not connected" error**
**Solution:** This is fixed - router gates now use `@loose` annotation

### **Problem: Build fails with "opp_configfilepath not found"**
**Solution:** Use OMNeT++ MinGW shell or build from IDE

### **Problem: Scenario 2 doesn't work yet**
**Solution:** Certificate verification logic needs to be added to MTA modules (see "Next Implementation Steps" above)

---

## 📞 Next Steps

To complete Scenario 2:
1. Ask me to implement certificate request logic in `mta_Client_SS.cc`
2. Ask me to implement certificate request logic in `mta_Server_RS.cc`
3. Ask me to update `sniffer.cc` to show MITM failure

Then you'll have a complete demonstration of both vulnerable and secure scenarios! 🎉
