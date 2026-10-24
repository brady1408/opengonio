#!/usr/bin/env bash
# Build OpenGonio-<version>-macOS.pkg from the built VST3 and AU bundles.
# Usage: build-pkg.sh <version> <path/to/OpenGonio.vst3> <path/to/OpenGonio.component> <out dir> [--sign <identity>]
# --sign is accepted and ignored for 0.1.0 (unsigned release); 0.2.0 wires it up.
set -euo pipefail

if [ $# -lt 4 ]; then
  echo "usage: $0 <version> <vst3> <component> <outdir> [--sign <identity>]" >&2
  exit 2
fi
version=$1; vst3=$2; component=$3; outdir=$4; shift 4
sign=""
while [ $# -gt 0 ]; do
  case "$1" in
    --sign) sign=$2; shift 2 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done
[ -n "$sign" ] && echo "note: --sign '$sign' ignored in 0.1.0 (unsigned release)"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
mkdir -p "$work/vst3/Library/Audio/Plug-Ins/VST3" "$work/au/Library/Audio/Plug-Ins/Components" "$outdir"
cp -R "$vst3" "$work/vst3/Library/Audio/Plug-Ins/VST3/"
cp -R "$component" "$work/au/Library/Audio/Plug-Ins/Components/"

pkgbuild --root "$work/vst3" --identifier com.bradycole.opengonio.vst3 --version "$version" \
  --install-location / "$work/OpenGonio-VST3.pkg"
pkgbuild --root "$work/au" --identifier com.bradycole.opengonio.au --version "$version" \
  --install-location / "$work/OpenGonio-AU.pkg"

cat > "$work/distribution.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="1">
  <title>OpenGonio $version</title>
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
productbuild --distribution "$work/distribution.xml" --package-path "$work" "$out"
echo "built $out"
