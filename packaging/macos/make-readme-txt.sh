#!/usr/bin/env bash
# Prints the README.txt that ships inside every package. Usage: make-readme-txt.sh <version>
set -euo pipefail
v=${1:-0.0.0}
cat <<TXT
OpenGonio $v
https://github.com/brady1408/opengonio

Free, open-source goniometer, Lissajous / vectorscope and stereo field meter
for Audacity and any VST3, AU or LV2 host.

Install
  macOS:   run the .pkg; it installs OpenGonio.vst3 and OpenGonio.component
           into /Library/Audio/Plug-Ins.
  Windows: run the Setup.exe, or copy OpenGonio.vst3 into
           C:\Program Files\Common Files\VST3.
  Linux:   run ./install.sh, which copies OpenGonio.vst3 and OpenGonio.lv2
           into ~/.vst3 and ~/.lv2.
Close your DAW before upgrading; a loaded plugin file cannot be replaced.

Using it in Audacity
  Effect > Plugin manager > Rescan plugins (first time only), then in a track's
  Effects panel: Add effect > VST3 > Brady Cole > OpenGonio. Press play.

Licence
  AGPL-3.0-or-later. The full licence text is in LICENSE.txt next to this
  file. Source code: https://github.com/brady1408/opengonio
TXT
