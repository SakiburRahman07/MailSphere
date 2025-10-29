# Malicious Sniffer Attack Demo

This demonstrates a **Man-in-the-Middle (MitM)** attack on the email system using weak RSA encryption.

## What Does It Do?

The `MaliciousSniffer` module intercepts network traffic between `mta_Client_SS` and `mta_Server_RS`, and:
- Captures all SMTP messages in real-time
- Decrypts the RSA-encrypted content using the hardcoded private key
- Logs all stolen data to `intercepted_traffic.log`
- Shows the attack happening live in the simulation output

## Files Added

1. **src/sniffer.cc** - The malicious sniffer module implementation
2. **src/email.ned** - Updated to include the sniffer in the network
3. **analyze_traffic.py** - Python script to analyze the intercepted data
4. **SNIFFER_README.md** - This file

## How to Run

### Step 1: Rebuild the Project

Open PowerShell in the project directory and run:

```powershell
cd "d:\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\samples\project"

# Clean previous build
mingw32-make clean

# Rebuild with the new sniffer module
mingw32-make
```

### Step 2: Run the Simulation

```powershell
# Run in command-line mode
cd simulations
..\src\project.exe -u Cmdenv -c General

# OR run in GUI mode (if using OMNeT++ IDE)
# Just open the project in IDE and click Run
```

### Step 3: Analyze the Intercepted Traffic

After the simulation completes, run the Python analyzer:

```powershell
cd ..
python analyze_traffic.py
```

## What You'll See

### During Simulation

You'll see console output like:

```
╔═══════════════════════════════════════════════════════════╗
║  ⚠️  MALICIOUS SNIFFER ACTIVE  ⚠️                         ║
║  Intercepting traffic at t=0.000000s                      ║
╚═══════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────┐
│ 🔴 PACKET INTERCEPTED! 🔴                               │
│ Time: 0.016800 seconds                                  │
│ Packet #1                                               │
└─────────────────────────────────────────────────────────┘
│ SMTP Command: MAIL
│ 🔒 Encrypted FROM: a5f3e892c1d4b7a6e9f2c8d5b3...
│ 🔓 CRACKED FROM: alice@example.com
│    ⚠️  ATTACK SUCCESSFUL at t=0.016800s
```

### After Running Python Script

```
🔴 MALICIOUS TRAFFIC ANALYSIS - INTERCEPTED EMAIL DATA 🔴
======================================================================

📊 Total packets intercepted: 5

✅ ENCRYPTION SUCCESSFULLY BROKEN!

📧 STOLEN EMAIL DETAILS:
======================================================================

🔓 FROM Address(es):
   [1] alice@example.com
       ⏱️  Cracked at t=0.016800s

🔓 TO Address(es):
   [1] bob@example.com
       ⏱️  Cracked at t=0.017100s

🔓 SUBJECT Line(s):
   [1] Greetings

🔓 MESSAGE BODY:
   [1] This is the body.

🔓 CONTENT:
   [1] Hello from Sender

📊 ATTACK SUMMARY:
   ✓ 1 FROM addresses stolen
   ✓ 1 TO addresses stolen
   ✓ 1 subjects stolen
   ✓ 1 message bodies stolen
   ✓ 1 message contents stolen

⚠️  SECURITY WARNING:
   Your toy RSA encryption (256-bit modulus) provides NO security!
   The private key is hardcoded and publicly known.
   Real attackers can decrypt ALL messages instantly.
```

## Why Does This Work?

The attack succeeds because:

1. **Hardcoded Private Key**: The RSA private key (d=7) is hardcoded in `helpers.h`
2. **Tiny Key Size**: 256-bit modulus is extremely weak
3. **No Per-User Keys**: Everyone uses the same key pair
4. **Transparent Positioning**: The sniffer acts as a transparent proxy

## Files to Check

1. **intercepted_traffic.log** - Detailed log of all intercepted packets
2. **Simulation console output** - Real-time attack notifications
3. **analyze_traffic.py output** - Summary of stolen data

## To Remove the Sniffer

If you want to run without the sniffer:

1. Edit `src/email.ned`
2. Remove or comment out the sniffer submodule and its connections
3. Restore the direct connection: `mta_Client_SS.ppp[1] <--> P2P <--> router.pppg[0];`
4. Rebuild: `mingw32-make clean && mingw32-make`

## Educational Purpose

This demonstration shows:
- How weak encryption provides no security
- Why hardcoded keys are dangerous
- How Man-in-the-Middle attacks work
- The importance of proper TLS/SSL with strong keys
