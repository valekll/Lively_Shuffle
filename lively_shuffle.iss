[Setup]
AppName=Lively Shuffle
AppVersion=1.0
DefaultDirName={pf}\Lively Shuffle
DefaultGroupName=Lively Shuffle
OutputBaseFilename=LivelyShuffleSetup
Compression=lzma
SolidCompression=yes

[Files]
Source: "lively_shuffle.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "lively_shuffle.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "lively_paths.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "shuffle_settings.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Lively Shuffle"; Filename: "{app}\lively_shuffle.exe"; IconFilename: "{app}\lively_shuffle.ico"
Name: "{group}\Uninstall Lively Shuffle"; Filename: "{uninstallexe}"
Name: "{autostartup}\Lively Shuffle"; Filename: "{app}\lively_shuffle.exe"; Tasks: AutoStart

[Tasks]
Name: "AutoStart"; Description: "Run Lively Shuffle at system startup"

[Run]
Filename: "{app}\lively_shuffle.exe"; Description: "Launch Lively Shuffle"; Flags: nowait postinstall skipifsilent
