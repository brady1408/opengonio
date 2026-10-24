#!/usr/bin/env sh
# Installs OpenGonio.vst3 and OpenGonio.lv2 from this folder into the user plugin
# directories. Run from the extracted tarball directory.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$HOME/.vst3" "$HOME/.lv2"
if [ -d "$here/OpenGonio.vst3" ]; then
  rm -rf "$HOME/.vst3/OpenGonio.vst3"
  cp -R "$here/OpenGonio.vst3" "$HOME/.vst3/"
  echo "installed $HOME/.vst3/OpenGonio.vst3"
fi
if [ -d "$here/OpenGonio.lv2" ]; then
  rm -rf "$HOME/.lv2/OpenGonio.lv2"
  cp -R "$here/OpenGonio.lv2" "$HOME/.lv2/"
  echo "installed $HOME/.lv2/OpenGonio.lv2"
fi
echo "Done. Rescan plugins in your host."
