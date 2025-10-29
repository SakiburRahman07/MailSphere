#!/usr/bin/env python3
"""
Malicious Traffic Analyzer
Demonstrates how weak encryption can be broken in real-time
"""

import re
import sys
from pathlib import Path

def analyze_log(log_file):
    """Parse the intercepted traffic log and display cracked messages"""
    
    if not Path(log_file).exists():
        print(f"\n❌ Error: Log file '{log_file}' not found!")
        print(f"   Make sure you run the OMNeT++ simulation first.")
        print(f"   The log file should be created in: {Path(log_file).absolute()}")
        return
    
    with open(log_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    print("\n" + "=" * 70)
    print("🔴 MALICIOUS TRAFFIC ANALYSIS - INTERCEPTED EMAIL DATA 🔴")
    print("=" * 70)
    print()
    
    # Extract packet information
    packets = content.split("INTERCEPTED PACKET #")
    num_packets = len(packets) - 1
    
    if num_packets == 0:
        print("⚠️  No packets found in log file!")
        print("   Make sure the sniffer module intercepted traffic.")
        return
    
    print(f"📊 Total packets intercepted: {num_packets}\n")
    
    # Find all encrypted and decrypted fields
    from_enc = re.findall(r'🔒 Encrypted FROM field:.*?Hex: (\w+)', content, re.DOTALL)
    from_dec = re.findall(r'🔓 DECRYPTED FROM: \'(.+?)\'', content)
    
    to_enc = re.findall(r'🔒 Encrypted TO field:.*?Hex: (\w+)', content, re.DOTALL)
    to_dec = re.findall(r'🔓 DECRYPTED TO: \'(.+?)\'', content)
    
    subject_dec = re.findall(r'🔓 DECRYPTED SUBJECT: \'(.+?)\'', content)
    body_dec = re.findall(r'🔓 DECRYPTED BODY: \'(.+?)\'', content)
    content_dec = re.findall(r'🔓 DECRYPTED CONTENT: \'(.+?)\'', content)
    
    # Extract timestamps
    timestamps = re.findall(r'Simulation Time: ([\d.]+) seconds', content)
    
    # Display results
    if from_dec or to_dec or subject_dec or body_dec or content_dec:
        print("✅ ENCRYPTION SUCCESSFULLY BROKEN!")
        print("   The weak RSA encryption was cracked in real-time.\n")
        
        print("=" * 70)
        print("📧 STOLEN EMAIL DETAILS:")
        print("=" * 70)
        
        if from_dec:
            print(f"\n🔓 FROM Address(es):")
            for i, (decrypted, time) in enumerate(zip(from_dec, timestamps[:len(from_dec)]), 1):
                print(f"   [{i}] {decrypted}")
                print(f"       ⏱️  Cracked at t={time}s")
        
        if to_dec:
            print(f"\n🔓 TO Address(es):")
            for i, (decrypted, time) in enumerate(zip(to_dec, timestamps[:len(to_dec)]), 1):
                print(f"   [{i}] {decrypted}")
                print(f"       ⏱️  Cracked at t={time}s")
        
        if subject_dec:
            print(f"\n🔓 SUBJECT Line(s):")
            for i, subj in enumerate(subject_dec, 1):
                print(f"   [{i}] {subj}")
        
        if body_dec:
            print(f"\n🔓 MESSAGE BODY:")
            for i, body in enumerate(body_dec, 1):
                print(f"   [{i}] {body}")
        
        if content_dec:
            print(f"\n🔓 CONTENT:")
            for i, cont in enumerate(content_dec, 1):
                print(f"   [{i}] {cont}")
        
        print()
    else:
        print("⚠️  Sniffer captured encrypted data but decryption failed.")
        print("   This might happen if encryption format changed.")
    
    # DH Key Exchange
    dh_keys = re.findall(r'DH Public Key: (\d+)', content)
    if dh_keys:
        print("\n" + "=" * 70)
        print("🔑 INTERCEPTED KEY EXCHANGE:")
        print("=" * 70)
        for i, key in enumerate(dh_keys, 1):
            print(f"   DH Public Key #{i}: {key}")
        print("   ⚠️  With this information, a MitM attack is possible!")
    
    print("\n" + "=" * 70)
    print("📊 ATTACK SUMMARY:")
    print("=" * 70)
    print(f"   ✓ {len(from_dec)} FROM addresses stolen")
    print(f"   ✓ {len(to_dec)} TO addresses stolen")
    print(f"   ✓ {len(subject_dec)} subjects stolen")
    print(f"   ✓ {len(body_dec)} message bodies stolen")
    print(f"   ✓ {len(content_dec)} message contents stolen")
    print(f"   ✓ {len(dh_keys)} key exchanges intercepted")
    print()
    print("⚠️  SECURITY WARNING:")
    print("   Your toy RSA encryption (256-bit modulus) provides NO security!")
    print("   The private key is hardcoded and publicly known.")
    print("   Real attackers can decrypt ALL messages instantly.")
    print("   Use proper TLS/SSL with 2048+ bit keys for real systems.")
    print("=" * 70)
    print()

def main():
    print("\n" + "=" * 70)
    print("🔴 MALICIOUS SNIFFER - TRAFFIC ANALYZER 🔴")
    print("Demonstrating Man-in-the-Middle Attack on Email System")
    print("=" * 70)
    
    log_file = "intercepted_traffic.log"
    
    if len(sys.argv) > 1:
        log_file = sys.argv[1]
    
    print(f"\n📂 Reading log file: {log_file}")
    
    analyze_log(log_file)

if __name__ == "__main__":
    main()
