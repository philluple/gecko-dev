import hashlib
import json
import os

# The path to your crate directory
CRATE_DIR = "."

# Files and dirs to exclude from checksum calculation
EXCLUDE = {
    ".cargo-checksum.json",
    "target",
    "vendor",
    ".git",
    ".DS_Store",
    "gen_checksum.py",
}

hasher = hashlib.sha256()

for root, dirs, files in os.walk(CRATE_DIR):
    dirs[:] = [d for d in dirs if d not in EXCLUDE]
    for file in files:
        if file in EXCLUDE:
            continue
        path = os.path.join(root, file)
        with open(path, "rb") as f:
            while chunk := f.read(8192):
                hasher.update(chunk)

checksum = hasher.hexdigest()
checksum_data = {"files": {}, "package": f"sha256:{checksum}"}

with open(".cargo-checksum.json", "w") as f:
    json.dump(checksum_data, f, indent=2)

print("✅ Created .cargo-checksum.json with package hash:", checksum)
