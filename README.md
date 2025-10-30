# 🔐 MailSphere - Secure Email Network Simulation

[![OMNeT++](https://img.shields.io/badge/OMNeT%2B%2B-6.2.0-blue.svg)](https://omnetpp.org/)
[![License](https://img.shields.io/badge/license-LGPL-green.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

> **A comprehensive network simulation demonstrating email security, cryptographic protocols, and man-in-the-middle (MITM) attack scenarios using OMNeT++**

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Key Features](#-key-features)
- [Architecture](#-architecture)
- [Security Scenarios](#-security-scenarios)
- [Getting Started](#-getting-started)
- [Usage](#-usage)
- [Project Structure](#-project-structure)
- [Implementation Details](#-implementation-details)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

---

## 🌟 Overview

**MailSphere** is an educational network simulation project built with OMNeT++ that demonstrates:

- 📧 Complete email communication flow (SMTP-based)
- 🔒 RSA encryption and Diffie-Hellman key exchange
- 🎭 Man-in-the-middle (MITM) attack scenarios
- 🛡️ Certificate-based authentication and PKI infrastructure
- 🌐 DNS resolution and multi-hop routing
- 📊 Protocol implementations: SMTP, DNS, HTTP, IMAP

This simulation provides a hands-on learning environment for understanding network security concepts, cryptographic protocols, and attack-defense mechanisms in email systems.

---

## ✨ Key Features

### 🔐 Cryptography
- **RSA Encryption**: Asymmetric encryption for secure message exchange
- **Diffie-Hellman**: Secure key exchange protocol
- **Digital Signatures**: Message authentication and integrity
- **Certificate Authority**: PKI-based identity verification

### 🌐 Network Protocols
- **SMTP**: Mail transfer protocol simulation
- **IMAP**: Mailbox access protocol
- **DNS**: Domain name resolution system
- **HTTP**: Web service communication

### 🎯 Attack Scenarios
- **Scenario 1**: Successful MITM attack (no certificates)
- **Scenario 2**: MITM attack blocked by PKI authentication

### 📊 Monitoring & Logging
- Traffic interception logging
- Certificate request/response tracking
- Routing table visualization
- Detailed debug output for all modules

---

## 🏗️ Architecture

### Network Topology

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         SENDER DOMAIN                                    │
│  ┌──────┐    ┌────────┐    ┌──────────┐    ┌───────────┐    ┌───────┐ │
│  │Sender│───→│Router1 │───→│MTA_Client│───→│MTA_Server │───→│ Spool │ │
│  │ (100)│    │        │    │_S (200)  │    │_S (300)   │    │ (400) │ │
│  └──────┘    │DNS1(150)│    └──────────┘    └───────────┘    └───────┘ │
│              └────────┘                                                  │
└─────────────────────────────────────────────────────────────────────────┘
                                     │
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    ENCRYPTED COMMUNICATION ZONE                          │
│  ┌───────────┐    ┌─────────┐    ┌───────────┐    ┌────┐               │
│  │MTA_Client │───→│ Sniffer │───→│   Router  │───→│ CA │               │
│  │_SS (500)  │    │  (550)  │    │           │    │(950)│               │
│  └───────────┘    │  [MITM] │    │DNS (650)  │    └────┘               │
│                   └─────────┘    │MTA_Server │                          │
│                                   │_RS (600)  │                          │
│                                   └───────────┘                          │
└─────────────────────────────────────────────────────────────────────────┘
                                     │
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        RECEIVER DOMAIN                                   │
│  ┌─────────┐    ┌──────────┐    ┌──────────┐    ┌────────┐             │
│  │ Mailbox │───→│MAA_Server│───→│MAA_Client│───→│Router2 │             │
│  │  (700)  │    │  (800)   │    │  (810)   │    │        │             │
│  └─────────┘    └──────────┘    └──────────┘    │DNS2    │             │
│                                                   │(860)   │             │
│                                    ┌──────────┐  │Receiver│             │
│                                    │ Receiver │←─┤(850)   │             │
│                                    │  (850)   │  └────────┘             │
│                                    └──────────┘                          │
└─────────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Address | Role |
|-----------|---------|------|
| **Sender** | 100 | Initiates email messages |
| **DNS1/DNS2/DNS** | 150/860/650 | Resolve logical names to addresses |
| **MTA_Client_S** | 200 | Client-side mail transfer agent (sender domain) |
| **MTA_Server_S** | 300 | Server-side mail transfer agent (sender domain) |
| **Spool** | 400 | Message queue/storage |
| **MTA_Client_SS** | 500 | Client-side secure mail transfer (encrypted zone) |
| **Sniffer** | 550 | 🎭 Man-in-the-middle attacker |
| **DNS** | 650 | Middle network DNS |
| **MTA_Server_RS** | 600 | Server-side secure mail transfer (encrypted zone) |
| **CA** | 950 | Certificate Authority (PKI) |
| **Mailbox** | 700 | Receiver's mailbox storage |
| **MAA_Server** | 800 | Mail access agent server |
| **MAA_Client** | 810 | Mail access agent client |
| **Receiver** | 850 | Final message recipient |
| **Router/Router1/Router2** | - | Network routing nodes |

---

## 🛡️ Security Scenarios

### Scenario 1: MITM Attack Succeeds (No Certificates)

**Configuration**: `useCertificates = false`

```ini
[Config Scenario1]
extends = General
description = "MITM attack succeeds - no certificate validation"
**.useCertificates = false
```

**What Happens**:
1. ✉️ Sender sends email to receiver
2. 🔓 No certificate authentication
3. 🎭 Sniffer intercepts and modifies traffic
4. ⚠️ **Attack succeeds** - sniffer can read/modify messages
5. 📝 Logs intercepted traffic to `intercepted_traffic.log`

**Learning Outcome**: Demonstrates vulnerability without PKI

---

### Scenario 2: MITM Attack Blocked (With Certificates)

**Configuration**: `useCertificates = true`

```ini
[Config Scenario2]
extends = General
description = "MITM attack blocked by certificate validation"
**.useCertificates = true
```

**What Happens**:
1. 🔐 MTA_Client_SS requests certificate from CA (kind=70)
2. ✅ CA issues signed certificate (kind=71)
3. 🔒 Encrypted communication established
4. 🛡️ **Attack blocked** - sniffer cannot decrypt without valid certificate
5. ✉️ Email delivered securely

**Certificate Flow**:
```
MTA_Client_SS → CA: CERT_REQUEST (kind=70)
CA → MTA_Client_SS: CERT_RESPONSE (kind=71) [contains signed certificate]
MTA_Client_SS ↔ MTA_Server_RS: Encrypted communication (verified identities)
```

**Learning Outcome**: Shows effectiveness of PKI in preventing MITM attacks

---

## 🚀 Getting Started

### Prerequisites

- **OMNeT++ 6.2.0** or higher ([Download](https://omnetpp.org/download/))
- **C++ Compiler** (GCC/Clang/MSVC)
- **Make** build system
- **Git** (optional, for cloning)

### Installation

1. **Clone or extract the project**:
   ```bash
   cd omnetpp-6.2.0/samples/project
   ```

2. **Build the project**:
   ```bash
   # From project root directory
   make makefiles
   make
   ```

   Or using OMNeT++ IDE:
   - Open project in OMNeT++ IDE
   - Right-click project → Build Project

3. **Verify build**:
   ```bash
   # Should create executable in 'out' directory
   ls out/gcc-release/src/
   ```

---

## 📖 Usage

### Running Simulations

#### Command Line

**Run Scenario 1 (MITM succeeds)**:
```bash
cd simulations
./run -u Cmdenv -c Scenario1
```

**Run Scenario 2 (MITM blocked)**:
```bash
cd simulations
./run -u Cmdenv -c Scenario2
```

**Run with GUI**:
```bash
./run -u Qtenv -c Scenario1
# or
./run -u Qtenv -c Scenario2
```

#### OMNeT++ IDE

1. Open `simulations/omnetpp.ini`
2. Click **Run** → **Run Configurations**
3. Select configuration: `Scenario1` or `Scenario2`
4. Click **Run**

### Configuration Options

Edit `simulations/omnetpp.ini` to customize:

```ini
# Simulation time
sim-time-limit = 100s

# Enable/disable certificates
**.useCertificates = true  # or false

# Adjust addresses (if needed)
**.sender.address = 100
**.ca.address = 950

# Routing tables (pre-configured for all scenarios)
**.router1.routes = "100:0, 150:1, 200:2"
```

### Viewing Results

**Console Output**:
- Real-time module activity
- Certificate requests/responses
- Routing decisions
- Attack attempts

**Log Files**:
- `intercepted_traffic.log` - Traffic captured by sniffer
- `results/*.vec` - Vector data (if enabled)
- `results/*.sca` - Scalar data

**OMNeT++ GUI**:
- Message animations
- Module state visualization
- Log window filtering

---

## 📁 Project Structure

```
project/
├── src/                        # Source code
│   ├── CA.cc                   # Certificate Authority implementation
│   ├── dns.cc                  # DNS server
│   ├── http.cc                 # HTTP protocol
│   ├── router.cc               # Network router with routing tables
│   ├── sender.cc               # Email sender
│   ├── mta_Client_SS.cc        # Secure MTA client (certificates)
│   ├── mta_Server_SS.cc        # Secure MTA server
│   ├── maa_Client.cc           # Mail access agent client
│   ├── maa_Server.cc           # Mail access agent server
│   ├── sniffer.cc              # MITM attacker
│   ├── spool.cc                # Message queue
│   ├── mailbox.cc              # Mailbox storage
│   ├── email.ned               # Network topology definition
│   ├── helpers.h               # Utility functions & constants
│   └── Makefile                # Build configuration
│
├── simulations/                # Simulation configurations
│   ├── omnetpp.ini             # Main configuration file
│   ├── package.ned             # Package declaration
│   ├── run                     # Simulation runner script
│   ├── intercepted_traffic.log # Sniffer output log
│   └── results/                # Simulation results
│
├── out/                        # Build output directory
│   └── gcc-release/            # Compiled binaries
│
├── MailSphere.pdf              # Project documentation
├── report.tex                  # LaTeX report source
├── Makefile                    # Top-level build file
└── README.md                   # This file
```

---

## 🔧 Implementation Details

### Message Types (helpers.h)

```cpp
// Core message types
#define DNS_QUERY          10
#define DNS_RESPONSE       11
#define HTTP_REQUEST       20
#define HTTP_RESPONSE      21
#define SMTP_MSG           30
#define IMAP_REQUEST       40
#define IMAP_RESPONSE      41

// Certificate messages
#define CERT_REQUEST       70  // Request certificate from CA
#define CERT_RESPONSE      71  // CA sends signed certificate

// Cryptographic messages
#define DH_KEY_EXCHANGE    80  // Diffie-Hellman key exchange
#define RSA_ENCRYPTED      90  // RSA encrypted message
```

### Routing Configuration

Routers use static routing tables defined in `omnetpp.ini`:

```ini
# Format: "destination:gate,destination:gate,..."
**.router1.routes = "100:0, 150:1, 200:2"
**.router.routes = "550:0, 500:0, 400:0, 300:0, 200:0, 650:1, 600:2, 700:2, 800:2, 810:2, 950:3"
**.router2.routes = "850:0, 860:1, 810:2, 800:2, 700:2, 600:2"
```

### Certificate Flow (Scenario 2)

1. **MTA_Client_SS** enters `STATE_CERT_REQ` state
2. Sends `CERT_REQUEST` (kind=70) to CA (address 950)
3. **CA** receives request:
   ```cpp
   handleCertificateRequest() {
       // Generate signed certificate
       // Send CERT_RESPONSE (kind=71)
   }
   ```
4. **MTA_Client_SS** receives certificate, enters `STATE_CERT_WAIT`
5. Communication proceeds with verified identity

### Sniffer Behavior

**Scenario 1** (no certificates):
- Intercepts all traffic
- Can read and modify messages
- Logs to `intercepted_traffic.log`

**Scenario 2** (with certificates):
- Transparently forwards certificate messages
- Cannot decrypt encrypted traffic
- Attack blocked

---

## 🐛 Troubleshooting

### Common Issues

#### 1. Build Errors

**Problem**: `make: *** No rule to make target`

**Solution**:
```bash
make clean
make makefiles
make
```

---

#### 2. DNS Warning Messages

**Problem**: `WARN: DNS unexpected kind=21`

**Status**: ✅ **Fixed** - Changed to debug level (expected during routing)

**Explanation**: DNS receives non-DNS messages during router flooding (normal behavior)

---

#### 3. Certificate Not Received

**Problem**: `mta_Client_SS` doesn't receive certificate response

**Causes & Solutions**:

✅ **Fixed Issues**:
- ~~Enum name collision~~ - `State::CERT_REQUEST` shadowed global constant (renamed to `STATE_CERT_REQ`)
- ~~Sniffer blocking~~ - Fixed forwarding logic for certificate messages
- ~~Wrong message kind~~ - Now correctly uses `kind=70/71` for cert messages

**Verification**:
```bash
# Check console output for:
CA: Received certificate request from: 500
CA: Sending certificate response to: 500
MTA_Client_SS: Received certificate (kind=71)
```

---

#### 4. Message Flooding to All Routers

**Problem**: Messages sent to all router ports instead of specific destination

**Status**: ✅ **Fixed** - Complete routing tables added to `omnetpp.ini`

**Solution**: All routers now have comprehensive routing tables configured

---

#### 5. Wrong DNS Address

**Problem**: Sender queries wrong DNS address

**Status**: ✅ **Fixed** - Changed `sender.cc` line 28 from `100` to `150`

---

### Debug Mode

Enable detailed logging:

```cpp
// In any module .cc file:
EV_DEBUG << "Detailed debug message" << endl;
EV_INFO << "Informational message" << endl;
EV_WARN << "Warning message" << endl;
```

Run with debug output:
```bash
./run -u Cmdenv -c Scenario2 --debug-on-errors=true
```

---

## 🧪 Testing

### Verification Steps

1. **Build Test**:
   ```bash
   make clean && make
   # Should complete without errors
   ```

2. **Scenario 1 Test**:
   ```bash
   ./run -u Cmdenv -c Scenario1
   # Expected: MITM attack succeeds, traffic intercepted
   # Check: intercepted_traffic.log contains data
   ```

3. **Scenario 2 Test**:
   ```bash
   ./run -u Cmdenv -c Scenario2
   # Expected: Certificate exchange occurs
   # Look for: "CA: Received certificate request"
   #           "MTA_Client_SS: Received certificate"
   # Expected: Secure communication, MITM blocked
   ```

4. **Routing Test**:
   ```bash
   # Check console for routing decisions
   # Should show specific gate selections, not flooding
   ```

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Report Bugs**: Open an issue with detailed reproduction steps
2. **Suggest Features**: Propose new scenarios or protocol implementations
3. **Submit PRs**: Fork, create feature branch, submit pull request
4. **Improve Docs**: Help clarify documentation or add examples

### Development Guidelines

- Follow existing code style
- Add comments for complex logic
- Test both scenarios before submitting
- Update README if adding features

---

## 📚 Further Reading

### OMNeT++ Resources
- [OMNeT++ Manual](https://omnetpp.org/doc/omnetpp/manual/)
- [INET Framework](https://inet.omnetpp.org/)
- [Simulation Tutorials](https://omnetpp.org/documentation)

### Network Security Concepts
- **PKI**: Public Key Infrastructure
- **RSA**: Rivest–Shamir–Adleman encryption
- **Diffie-Hellman**: Key exchange protocol
- **MITM Attacks**: Man-in-the-middle threat model
- **Digital Certificates**: X.509 standard

### Email Protocols
- **SMTP**: Simple Mail Transfer Protocol (RFC 5321)
- **IMAP**: Internet Message Access Protocol (RFC 3501)
- **DNS**: Domain Name System (RFC 1035)

---

## 📄 License

This project is licensed under the **LGPL License** - see the LICENSE file for details.

---

## 👥 Authors & Acknowledgments

**Project**: MailSphere - Secure Email Network Simulation  
**Framework**: OMNeT++ 6.2.0  
**Purpose**: Educational network security demonstration

**Special Thanks**:
- OMNeT++ development team
- Network security research community
- All contributors and testers

---

## 📞 Support

- **Issues**: Open a GitHub issue
- **Questions**: Check documentation first, then ask in discussions
- **Updates**: Watch repository for latest changes

---

<div align="center">

**⭐ If this project helps your learning, please star it! ⭐**

Made with ❤️ using OMNeT++

[🔝 Back to Top](#-mailsphere---secure-email-network-simulation)

</div>
