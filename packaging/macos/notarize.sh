#!/usr/bin/env bash
# Notarize and staple a package with an App Store Connect API key.
# Usage: notarize.sh <pkg> <AuthKey.p8 path> <key id> <issuer id>
set -euo pipefail
[ $# -eq 4 ] || { echo "usage: $0 <pkg> <AuthKey.p8> <key-id> <issuer-id>" >&2; exit 2; }
pkg=$1; key=$2; kid=$3; iss=$4
xcrun notarytool submit "$pkg" --key "$key" --key-id "$kid" --issuer "$iss" --wait --timeout 30m
xcrun stapler staple "$pkg"
xcrun stapler validate "$pkg"
spctl --assess --type install --verbose=2 "$pkg"
