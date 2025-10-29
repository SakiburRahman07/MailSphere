# 🚀 QUICK START GUIDE - Malicious Sniffer Attack Demo

## ✅ Step-by-Step Instructions

### Option 1: Automated (Recommended)

Open PowerShell in the project directory and run:

```powershell
cd "d:\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\samples\project"
.\run_attack_demo.ps1
```

### Option 2: Manual Steps

If the automated script doesn't work, follow these steps:

#### Step 1: Clean and Build

```powershell
cd "d:\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\samples\project\src"
make clean
make
```

**Expected output:**
```
Creating executable: project.exe
Creating shared library: project.dll
```

If you see errors, make sure:
- OMNeT++ environment is set up correctly
- You're in the correct directory
- `sniffer.o` is being compiled

#### Step 2: Run the Simulation

```powershell
cd ..\simulations
..\src\project.exe -u Cmdenv -c General
```

**Watch for these messages:**

```
╔═══════════════════════════════════════════════════════════╗
║  ⚠️  MALICIOUS SNIFFER ACTIVE  ⚠️                         ║
╚═══════════════════════════════════════════════════════════╝

┌─────────────────────────────────────────────────────────┐
│ 🔴 PACKET INTERCEPTED! 🔴                               │
│ Time: 0.016800 seconds                                  │
│ 🔓 CRACKED FROM: alice@example.com
└─────────────────────────────────────────────────────────┘
```

#### Step 3: Analyze the Results

```powershell
cd ..
python analyze_traffic.py
```

**You should see:**

```
🔴 MALICIOUS TRAFFIC ANALYSIS - INTERCEPTED EMAIL DATA 🔴
======================================================================

📧 STOLEN EMAIL DETAILS:
🔓 FROM: alice@example.com
🔓 TO: bob@example.com
🔓 SUBJECT: Greetings
🔓 BODY: This is the body.
🔓 CONTENT: Hello from Sender
```

---

## ❗ Troubleshooting

### Error: "Class MaliciousSniffer not found"

**Solution:** The sniffer wasn't compiled. Run:

```powershell
cd src
make clean
make
```

Check that you see `sniffer.o` being compiled in the output.

### Error: "make: command not found"

**Solution:** You need to run from OMNeT++ MinGW shell or ensure make is in your PATH.

Try opening "mingw64.exe" from OMNeT++ directory first, then run the commands.

### Error: "Gate 'out' not found"

**Solution:** Already fixed! The gate is now correctly named `ppp$o`.

### No intercepted_traffic.log file

**Solution:** 
- Make sure the simulation ran successfully
- Check that the sniffer module was initialized
- Look for the "MALICIOUS SNIFFER ACTIVE" message in console

---

## 📋 Checklist

Before running, verify:

- [x] `src/sniffer.cc` exists
- [x] `src/email.ned` contains MaliciousSniffer definition
- [x] `src/Makefile` includes `sniffer.o` in OBJS
- [x] Project builds without errors
- [x] Python is installed (for analysis script)

---

## 🎯 What to Look For

### During Simulation:

1. **"MALICIOUS SNIFFER ACTIVE"** message at start
2. **"PACKET INTERCEPTED"** messages during simulation
3. **"CRACKED FROM/TO/SUBJECT"** showing decrypted data
4. Simulation completes successfully

### After Running Python Script:

1. Summary of stolen email addresses
2. Decrypted message content
3. Exact timestamps when data was cracked
4. Security warning about weak encryption

---

## 📁 Important Files

- **`src/sniffer.cc`** - The malicious module code
- **`src/email.ned`** - Network topology
- **`intercepted_traffic.log`** - Detailed log (created after simulation)
- **`analyze_traffic.py`** - Analysis script
- **`SNIFFER_README.md`** - Full documentation

---

## 🔧 If Something Goes Wrong

1. **Clean everything:**
   ```powershell
   cd src
   make clean
   ```

2. **Check Makefile** has `sniffer.o` in OBJS list

3. **Rebuild:**
   ```powershell
   make
   ```

4. **Check for errors** in the build output

5. **Run simulation** and watch console for sniffer messages

---

## ✨ Success Indicators

You'll know it's working when you see:

✅ "MALICIOUS SNIFFER ACTIVE" at simulation start  
✅ Multiple "PACKET INTERCEPTED" messages  
✅ Decrypted email addresses and content  
✅ `intercepted_traffic.log` file created  
✅ Python script shows stolen data summary  

Enjoy the demonstration! 🔴
