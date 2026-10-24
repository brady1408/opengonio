; Inno Setup script for OpenGonio. Compile with:
;   iscc /DVersion=0.1.0 /DVst3Dir=path\to\OpenGonio.vst3 /DOutDir=out opengonio.iss
#ifndef Version
  #define Version "0.0.0"
#endif
#ifndef Vst3Dir
  #define Vst3Dir "..\..\build\OpenGonio_artefacts\Release\VST3\OpenGonio.vst3"
#endif
#ifndef OutDir
  #define OutDir "."
#endif

[Setup]
AppId={{B7A0C9E2-6F1D-4A3B-9C1E-0F2D3E4A5B6C}
AppName=OpenGonio
AppVersion={#Version}
AppPublisher=Brady Cole
AppPublisherURL=https://github.com/brady1408/opengonio
AppSupportURL=https://github.com/brady1408/opengonio/issues
DefaultDirName={commoncf64}\VST3\OpenGonio.vst3
DisableDirPage=yes
DisableProgramGroupPage=yes
OutputDir={#OutDir}
OutputBaseFilename=OpenGonio-{#Version}-Windows-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
; A DAW that has OpenGonio loaded keeps the .vst3 locked; ask the user to close it
; instead of failing the file replace.
CloseApplications=yes
CloseApplicationsFilter=*.vst3
RestartApplications=no
UninstallDisplayName=OpenGonio {#Version}
LicenseFile=..\..\LICENSE

#ifndef DocsDir
  #define DocsDir "."
#endif

[Files]
Source: "{#Vst3Dir}\*"; DestDir: "{commoncf64}\VST3\OpenGonio.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#DocsDir}\LICENSE.txt"; DestDir: "{commoncf64}\VST3\OpenGonio.vst3"; Flags: ignoreversion
Source: "{#DocsDir}\README.txt"; DestDir: "{commoncf64}\VST3\OpenGonio.vst3"; Flags: ignoreversion

[Messages]
FinishedLabel=OpenGonio is installed. Open your DAW (in Audacity: Effect > Plugin manager > Rescan plugins) and add OpenGonio as a realtime effect.
