#!/usr/bin/env python3
"""
License Key Generator for Analyse2Print

Usage:
    python license_generator.py <device_id>

Example:
    python license_generator.py 1234ABCD5678

The device_id is shown on the license activation screen
or can be read via Serial when the device boots.
"""

import sys

# Must match LICENSE_SECRET in license.cpp!
LICENSE_SECRET = "A2P_SECRET_2024_CHANGE_ME"
XOR_KEY = 0xA2B1C2D3

def simple_hash(s: str) -> int:
    """DJB2 hash algorithm - must match C implementation"""
    hash_val = 5381
    for c in s:
        hash_val = ((hash_val << 5) + hash_val) + ord(c)
        hash_val &= 0xFFFFFFFF  # Keep 32-bit
    return hash_val

def generate_license(device_id: str) -> str:
    """Generate license key for a device ID"""
    combined = device_id + LICENSE_SECRET
    hash_val = simple_hash(combined)

    # Generate key (same algorithm as in license.cpp)
    part1 = hash_val
    part2 = hash_val ^ XOR_KEY

    # Format as XXXX-XXXX-XXXX-XXXX
    key = f"{(part1 >> 16) & 0xFFFF:04X}-{part1 & 0xFFFF:04X}-{(part2 >> 16) & 0xFFFF:04X}-{part2 & 0xFFFF:04X}"
    return key

def main():
    if len(sys.argv) < 2:
        print("Usage: python license_generator.py <device_id>")
        print("\nExample:")
        print("  python license_generator.py 1234ABCD5678")
        sys.exit(1)

    device_id = sys.argv[1].upper().strip()

    if len(device_id) < 8:
        print("Error: Device ID should be at least 8 characters")
        sys.exit(1)

    license_key = generate_license(device_id)

    print(f"\n{'='*40}")
    print(f"Device ID:    {device_id}")
    print(f"License Key:  {license_key}")
    print(f"{'='*40}\n")

if __name__ == "__main__":
    main()
