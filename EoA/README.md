<pre>
Recovery System                                        November 22, 2026
Category: Informational


                            RECOVERY SYSTEM

ABSTRACT

   An accessibility application for Windows x64 that displays the Open
   dialog when you press Ctrl+Alt+F9 on the sign-in screen. Windows
   manages its execution, and the application remains hidden until you
   press the shortcut.

                           TABLE OF CONTENTS

   <a href="#overview">1.  OVERVIEW</a> ...................................................... <a href="#overview">1</a>
   <a href="#requirements-and-build">2.  REQUIREMENTS AND BUILD</a> ........................................ <a href="#requirements-and-build">1</a>
   <a href="#installation">3.  INSTALLATION</a> .................................................. <a href="#installation">1</a>
   <a href="#startup-settings">4.  STARTUP SETTINGS</a> .............................................. <a href="#startup-settings">2</a>
   <a href="#usage">5.  USAGE</a> ......................................................... <a href="#usage">2</a>
   <a href="#uninstallation">6.  UNINSTALLATION</a> ................................................ <a href="#uninstallation">2</a>


Recovery System              Informational                      [Page i]
</pre>

<pre>
Recovery System                User Guide              November 22, 2026


<a name="overview"></a>1.  OVERVIEW

   An accessibility application for Windows x64 that displays the Open
   dialog when you press Ctrl+Alt+F9 on the sign-in screen. Windows
   manages its execution, and the application remains hidden until you
   press the shortcut.


<a name="requirements-and-build"></a>2.  REQUIREMENTS AND BUILD

   Building requires Make and MinGW-w64 on Linux, including the Windows
   x64 C++ compiler and windres.

   From the project root:

      make

   The output is build/EoASysRec.exe, with an embedded manifest and
   string resources. Copy Install.ps1, Uninstall.ps1, and the build
   folder to Windows, keeping the scripts alongside that folder.

   The installation and uninstallation scripts require 64-bit PowerShell
   and administrator privileges. Execution must be allowed by the
   computer&#x27;s policies.


<a name="installation"></a>3.  INSTALLATION

   Open 64-bit PowerShell as administrator and run from the project
   root:

      .\Install.ps1

   The installer:

   o  Copies the executable to %SystemRoot%\System32\EoASysRec.exe.

   o  Registers the application as EoA_SysRecovery under the registry
      location listed below.

   o  Adds eoa_sysrecovery to the Configuration value under
      HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility,
      preserving other entries and avoiding duplicates.

   o  Sets TerminateOnDesktopSwitch=1 and PassiveAutoStartBehavior=1 so
      that Windows manages desktop transitions and automatic startup
      behavior.

   Application registration location:

   HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility\ATs


Recovery System              Informational                      [Page 1]
</pre>

<pre>
Recovery System                User Guide              November 22, 2026


<a name="startup-settings"></a>4.  STARTUP SETTINGS

   You can review the activation settings under Control Panel &gt; Ease of
   Access &gt; Ease of Access Center &gt; Change sign-in settings.

   The application appears as Recovery System. To use it exclusively on
   the sign-in screen, keep At sign-in checked and After sign-in
   unchecked. The installer configures activation at sign-in; it does
   not modify the user&#x27;s activation settings in HKCU.


<a name="usage"></a>5.  USAGE

   o  Go to the sign-in screen by signing out or restarting the
      computer. You can also lock the session with Win+L.

   o  Press Ctrl+Alt+F9 to display the Open dialog.

   o  Browse and select a file, or click Cancel to close the dialog.

   o  Do not execute files using the Open command in the context menu
      displayed by right-clicking a file, as they could run with SYSTEM
      account privileges.

   The file picker opens Windows\System32 and hides the default
   locations in the navigation pane. Selecting a file only closes the
   dialog; the application does not process or execute it. The shortcut
   remains active after the dialog closes. If the dialog is already
   open, the shortcut does not open another instance.

   The application remains hidden, and Windows manages its lifecycle.
   The executable expects to start within a Windows-managed job; install
   it as an accessibility application to use it.


<a name="uninstallation"></a>6.  UNINSTALLATION

   From 64-bit PowerShell as administrator, in the project root:

      .\Uninstall.ps1

   The script removes the application&#x27;s registration and activation
   entries in HKLM and the current account, stops instances of the
   installed executable, and deletes the binary from System32. It
   preserves entries for other accessibility applications. If you
   manually enabled Recovery System for other accounts, disable it in
   those accounts as well.


Recovery System              Informational                      [Page 2]
</pre>
