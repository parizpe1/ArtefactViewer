; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Artefact Viewer"
#define MyAppVersion "0.1"
#define MyAppPublisher "University of Cologne"
#define MyAppExeName "ArtefactViewer.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{250B8E79-E1BF-4996-8430-7FB06F4B2BAB}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=D:\dwickero\Code\AV2\license.txt
OutputDir=D:\dwickero\Code\AV2\installerOutput
OutputBaseFilename=ArtefactViewer_0.1_Setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "D:\dwickero\Code\AV2\bin\ArtefactViewer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "D:\dwickero\Code\AV2\bin\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion
Source: "D:\dwickero\Code\AV2\bin\manual\*"; DestDir: "{app}\manual"; Flags: ignoreversion
Source: "D:\dwickero\Code\AV2\license.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\icudt51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\icuin51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\icuuc51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\Qt5OpenGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl\bin\Qt5Xml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x64\Microsoft.VC110.CRT\msvcp110.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\redist\x64\Microsoft.VC110.CRT\msvcr110.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
