#!/usr/bin/env bash
# Build OpenGonio-<version>-macOS.pkg from the built VST3 and AU bundles.
#
# Usage:
#   build-pkg.sh <version> <path/to/OpenGonio.vst3> <path/to/OpenGonio.component> <out dir> \
#       [--sign-app "<Developer ID Application identity>"] \
#       [--sign-pkg "<Developer ID Installer identity>"]
#
# With --sign-app the bundles are codesigned (hardened runtime, timestamp) before
# packaging; with --sign-pkg the product package is productsign'd. Notarization
# and stapling are a separate step: packaging/macos/notarize.sh.
set -euo pipefail

if [ $# -lt 4 ]; then
  echo "usage: $0 <version> <vst3> <component> <outdir> [--sign-app <identity>] [--sign-pkg <identity>]" >&2
  exit 2
fi
version=$1; vst3=$2; component=$3; outdir=$4; shift 4
sign_app=""; sign_pkg=""
while [ $# -gt 0 ]; do
  case "$1" in
    --sign-app) sign_app=$2; shift 2 ;;
    --sign-pkg) sign_pkg=$2; shift 2 ;;
    --sign)     sign_app=$2; shift 2 ;;   # 0.1.x compatibility
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

here=$(cd "$(dirname "$0")" && pwd)
repo=$(cd "$here/../.." && pwd)

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/vst3/Library/Audio/Plug-Ins/VST3" "$work/au/Library/Audio/Plug-Ins/Components" "$work/res" "$outdir"
cp -R "$vst3" "$work/vst3/Library/Audio/Plug-Ins/VST3/"
cp -R "$component" "$work/au/Library/Audio/Plug-Ins/Components/"

if [ -n "$sign_app" ]; then
  for b in "$work/vst3/Library/Audio/Plug-Ins/VST3/OpenGonio.vst3" "$work/au/Library/Audio/Plug-Ins/Components/OpenGonio.component"; do
    codesign --force --deep --options runtime --timestamp --sign "$sign_app" "$b"
    codesign --verify --deep --strict --verbose=2 "$b"
  done
fi

pkgbuild --root "$work/vst3" --identifier com.bradycole.opengonio.vst3 --version "$version" \
  --install-location / "$work/OpenGonio-VST3.pkg"
pkgbuild --root "$work/au" --identifier com.bradycole.opengonio.au --version "$version" \
  --install-location / "$work/OpenGonio-AU.pkg"

# Licence and readme shown by the installer (AGPL conveyance).
cp "$repo/LICENSE" "$work/res/LICENSE.txt"
"$here/make-readme-txt.sh" "$version" > "$work/res/README.txt"

cat > "$work/distribution.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
  <title>OpenGonio $version</title>
  <license file="LICENSE.txt"/>
  <readme file="README.txt"/>
  <options customize="allow" require-scripts="false" hostArchitectures="arm64,x86_64"/>
  <domains enable_localSystem="true" enable_currentUserHome="true"/>
  <choices-outline>
    <line choice="vst3"/>
    <line choice="au"/>
  </choices-outline>
  <choice id="vst3" title="VST3 plugin" description="Installs OpenGonio.vst3 into Library/Audio/Plug-Ins/VST3">
    <pkg-ref id="com.bradycole.opengonio.vst3"/>
  </choice>
  <choice id="au" title="Audio Unit plugin" description="Installs OpenGonio.component into Library/Audio/Plug-Ins/Components">
    <pkg-ref id="com.bradycole.opengonio.au"/>
  </choice>
  <pkg-ref id="com.bradycole.opengonio.vst3" version="$version" onConclusion="none">OpenGonio-VST3.pkg</pkg-ref>
  <pkg-ref id="com.bradycole.opengonio.au" version="$version" onConclusion="none">OpenGonio-AU.pkg</pkg-ref>
</installer-gui-script>
XML

out="$outdir/OpenGonio-$version-macOS.pkg"
if [ -n "$sign_pkg" ]; then
  productbuild --distribution "$work/distribution.xml" --package-path "$work" --resources "$work/res" "$work/unsigned.pkg"
  productsign --sign "$sign_pkg" --timestamp "$work/unsigned.pkg" "$out"
else
  productbuild --distribution "$work/distribution.xml" --package-path "$work" --resources "$work/res" "$out"
fi
echo "built $out"
