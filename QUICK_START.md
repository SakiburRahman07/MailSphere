# 🚀 QUICK START GUIDE

## Build and Run Both Scenarios

### **Step 1: Build the Project**
```bash
cd /d/omnetpp-6.2.0-windows-x86_64/omnetpp-6.2.0/samples/project
make MODE=release clean
make MODE=release
cd simulations
```

### **Step 2: Run Scenario 1 (MITM Attack Succeeds)**
```bash
../src/project.exe -u Cmdenv -c Scenario1_MITM_Attack
```

**What you'll see:**
- ✅ Sniffer intercepts key exchange
- ✅ Sniffer substitutes its own keys
- ✅ Two separate encrypted sessions created
- ✅ Sniffer decrypts all email content
- ❌ Attack succeeds - email stolen!

---

### **Step 3: Run Scenario 2 (Certificates Prevent MITM)**
```bash
../src/project.exe -u Cmdenv -c Scenario2_With_Certificates
```

**What you'll see:**
- ✅ CA issues certificates to Sender and Receiver
- ✅ Certificates exchanged during key exchange
- ✅ Both parties verify certificates
- ✅ Sniffer cannot forge valid certificates
- ✅ Attack fails - secure communication established!

---

## 📊 Key Differences

| Feature | Scenario 1 | Scenario 2 |
|---------|-----------|-----------|
| **Certificates** | ❌ Disabled | ✅ Enabled |
| **CA Active** | ❌ No | ✅ Yes |
| **Key Exchange** | Direct | Authenticated |
| **Sniffer** | ✅ Succeeds | ❌ Fails |
| **Email Security** | ❌ Compromised | ✅ Secure |

---

## 🎯 What This Demonstrates

**Scenario 1 shows:**
- Why Diffie-Hellman alone is vulnerable
- How MITM attacks work in practice
- The importance of authentication

**Scenario 2 shows:**
- How certificates bind keys to identities
- The role of Certificate Authority
- Why TLS/SSL uses certificates

---

## 📖 For More Details

- `SCENARIO2_IMPLEMENTATION_COMPLETE.md` - Full implementation details
- `HOW_TO_RUN_SCENARIOS.md` - Comprehensive guide
- `CERTIFICATE_DEMO_README.md` - Technical documentation

---

## ⚡ Quick Commands Reference

```bash
# Build
make MODE=release clean && make MODE=release

# Run MITM attack (vulnerable)
cd simulations && ../src/project.exe -u Cmdenv -c Scenario1_MITM_Attack

# Run with certificates (secure)
cd simulations && ../src/project.exe -u Cmdenv -c Scenario2_With_Certificates

# Check for errors
cd src && make MODE=release 2>&1 | grep error
```

---

Happy simulating! 🎉
