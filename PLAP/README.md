<pre>
PreAuth PLAP                                           November 22, 2026
Category: Informational


                            PREAUTH PLAP   

ABSTRACT

   A PLAP provider for Windows x64 that displays the Open dialog when
   you select PreAuth PLAP and press Connect on the sign-in screen.
   Windows hosts the provider and manages its execution.

                           TABLE OF CONTENTS

   <a href="#overview">1.  OVERVIEW</a> ...................................................... <a href="#overview">1</a>
   <a href="#requirements-and-build">2.  REQUIREMENTS AND BUILD</a> ........................................ <a href="#requirements-and-build">1</a>
   <a href="#installation">3.  INSTALLATION</a> .................................................. <a href="#installation">1</a>
   <a href="#startup-settings">4.  STARTUP SETTINGS</a> .............................................. <a href="#startup-settings">2</a>
   <a href="#usage">5.  USAGE</a> ......................................................... <a href="#usage">2</a>
   <a href="#uninstallation">6.  UNINSTALLATION</a> ................................................ <a href="#uninstallation">2</a>


PreAuth PLAP                 Informational                      [Page i]
</pre>

<pre>
PreAuth PLAP                   User Guide              November 22, 2026


<a name="overview"></a>1.  OVERVIEW

   A PLAP provider for Windows x64 that displays the Open dialog when
   you select PreAuth PLAP and press Connect on the sign-in screen.
   Windows hosts the provider and manages its execution.


<a name="requirements-and-build"></a>2.  REQUIREMENTS AND BUILD

   Building requires Make and MinGW-w64 on Linux, including the Windows
   x64 C++ compiler and binutils.

   From the project root:

      make

   The output is build/PreAuthPLAP.dll, with statically linked MinGW
   runtimes. Copy Install.ps1, Uninstall.ps1, and the build folder to
   Windows, keeping the scripts alongside that folder.

   The installation and uninstallation scripts require 64-bit PowerShell
   and administrator privileges. Execution must be allowed by the
   computer&#x27;s policies.


<a name="installation"></a>3.  INSTALLATION

   Open 64-bit PowerShell as administrator and run from the project
   root:

      .\Install.ps1

   The installer:

   o  Copies the DLL to %SystemRoot%\System32\PreAuthPLAP.dll.

   o  Registers the COM class as PreAuth PLAP under the registry
      location listed below.

   o  Sets InprocServer32 to the installed DLL path and ThreadingModel
      to Apartment.

   o  Registers PreAuth PLAP as a pre-logon access provider in the
      64-bit registry view.

   COM class registration location:

   HKLM\SOFTWARE\Classes\CLSID\{8B0C5DF7-3F70-4B90-9B70-61A63A3B5F25}

   PLAP provider registration location:

   HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\PLAP Providers\{8B0C5DF7-3F70-4B90-9B70-61A63A3B5F25}


PreAuth PLAP                 Informational                      [Page 1]
</pre>

<pre>
PreAuth PLAP                   User Guide              November 22, 2026


<a name="startup-settings"></a>4.  STARTUP SETTINGS

   The provider is registered for the Windows pre-logon access
   interface and appears as PreAuth PLAP.

   Access is available through the network sign-in option on the local
   sign-in screen. The entry point and its availability depend on the
   Windows build, configuration, and computer&#x27;s policies.


<a name="usage"></a>5.  USAGE

   o  Go to the sign-in screen by signing out or restarting the
      computer.

   o  Open the network sign-in option and select PreAuth PLAP.

   o  Press Connect to display the Open dialog. Windows may display
      the button as an arrow and localize its text.

   o  Browse and select a file, or click Cancel to close the dialog.

   o  Use the Windows Back/Cancel controls to return to normal sign-in
      when needed.

   The file picker opens Windows\System32 and hides pinned locations
   in the navigation pane. Selecting a file closes the dialog and
   completes the PLAP interaction.

   Windows manages the provider&#x27;s lifecycle and supplies the owner
   window for the dialog.


<a name="uninstallation"></a>6.  UNINSTALLATION

   From 64-bit PowerShell as administrator, in the project root:

      .\Uninstall.ps1

   The script removes the provider&#x27;s PLAP and COM registration
   entries in HKLM and deletes PreAuthPLAP.dll from System32.


PreAuth PLAP                 Informational                      [Page 2]
</pre>
