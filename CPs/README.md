<pre>
Pre-Auth Project                                           November 2026
Category: Informational


                 Pre-Auth: Windows Credential Provider

ABSTRACT

   This memo describes the function, features, requirements,
   installation, and use of the Pre-Auth Windows Credential Provider.

                           TABLE OF CONTENTS

   <a href="#overview">1.  OVERVIEW</a> ...................................................... <a href="#overview">1</a>
   <a href="#features">2.  FEATURES</a> ...................................................... <a href="#features">1</a>
   <a href="#usage">3.  USAGE</a> ......................................................... <a href="#usage">1</a>
   <a href="#requirements">    3.1.  Requirements</a> ............................................ <a href="#requirements">1</a>
   <a href="#building">    3.2.  Building</a> ................................................ <a href="#building">1</a>
   <a href="#installation">    3.3.  Installation</a> ............................................ <a href="#installation">2</a>
   <a href="#interaction">    3.4.  Interaction</a> ............................................. <a href="#interaction">2</a>
   <a href="#updating">    3.5.  Updating</a> ................................................ <a href="#updating">2</a>
   <a href="#uninstallation">    3.6.  Uninstallation</a> .......................................... <a href="#uninstallation">3</a>
   <a href="#preview">    3.7.  Previewing Operations</a> ................................... <a href="#preview">3</a>
   <a href="#references">4.  REFERENCES</a> .................................................... <a href="#references">3</a>


Pre-Auth Project           Informational                      [Page i]
</pre>

<pre>
Pre-Auth               Windows Credential Provider         November 2026


<a name="overview"></a>1.  OVERVIEW

   Pre-Auth is a Windows Credential Provider that adds a custom visual
   option to the Sign-in options selector for each available user.
   Selecting it displays the Pre-Auth command link in LogonUI. Clicking
   the link validates the field identifier and retrieves the LogonUI
   owner window.

<a name="features"></a>2.  FEATURES

   o  Integration with the Windows sign-in method selector.

   o  Association with users enumerated by Windows.

   o  Support for sign-in and workstation unlock scenarios.

   o  An icon with a transparent background, embedded in the DLL.

<a name="usage"></a>3.  USAGE

<a name="requirements"></a>3.1.  Requirements

   To install and use the provider:

   o  Windows x64.

   o  A 64-bit PowerShell session running as administrator for
      installation and removal.

   To build from Linux:

   o  GNU Make.

   o  MinGW-w64: x86_64-w64-mingw32-g++ and
      x86_64-w64-mingw32-windres.

<a name="building"></a>3.2.  Building

   C++ source files, headers, and the resource definition are in src/.
   Icons are in resources/. The Makefile and installation scripts are in
   the project root.

   From the project directory, run:

       make

   The output is build/Provider.dll, including its embedded graphics
   resources.


Pre-Auth Project           Informational                      [Page 1]
</pre>

<pre>
Pre-Auth               Windows Credential Provider         November 2026


<a name="installation"></a>3.3.  Installation

   Copy the scripts and DLL to Windows using this directory structure:

       Pre-Auth/
       |-- Install.ps1
       |-- Uninstall.ps1
       `-- build/
           `-- Provider.dll

   From that directory, open 64-bit PowerShell as administrator and run:

       .\Install.ps1

   The installer copies the DLL to %SystemRoot%\System32\Provider.dll
   and registers the component as Pre-Auth. Sign out or restart Windows
   to load it.

<a name="interaction"></a>3.4.  Interaction

   1.  Select a user on the sign-in screen.
   2.  Open Sign-in options.
   3.  Select the skull icon, labeled Pre-Auth.
   4.  The Pre-Auth command link appears.

   The click handler is CMinimoCredential::CommandLinkClicked in
   src/Provider.cpp. It uses OnCreatingWindow to retrieve the owner
   window.

<a name="updating"></a>3.5.  Updating

   Uninstall the installed version, restart Windows if the DLL is in
   use, and run Install.ps1 with the new build at build/Provider.dll.


Pre-Auth Project           Informational                      [Page 2]
</pre>

<pre>
Pre-Auth               Windows Credential Provider         November 2026


<a name="uninstallation"></a>3.6.  Uninstallation

   From a 64-bit PowerShell session running as administrator:

       .\Uninstall.ps1

   The script removes the provider registration from LogonUI, deletes
   System32\Provider.dll, and removes the component's COM registration.
   If the DLL is still in use, restart Windows and run the uninstaller
   again.

<a name="preview"></a>3.7.  Previewing Operations

   Both scripts support -WhatIf:

       .\Install.ps1 -WhatIf
       .\Uninstall.ps1 -WhatIf

<a name="references"></a>4.  REFERENCES

   [1] Microsoft,
       <a href="https://learn.microsoft.com/en-us/windows/win32/api/credentialprovider/nn-credentialprovider-icredentialprovidercredential2">ICredentialProviderCredential2 interface</a>.
       User association, provider icons, and labels.


Pre-Auth Project           Informational                      [Page 3]
</pre>
