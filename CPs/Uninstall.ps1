<#
.SYNOPSIS
Removes the V2 installation created by Install.ps1.
.DESCRIPTION
Removes the LogonUI registration, Windows\System32\Provider.dll, and
the COM registration. If the DLL is in use, retry after restarting Windows.
.EXAMPLE
.\Uninstall.ps1 -WhatIf
#>
[CmdletBinding(SupportsShouldProcess)]
param()

$ErrorActionPreference = 'Stop'
$Guid = '{e2e9a68f-4d2a-4b6e-9ebe-0c2f19e92d23}'
$Name = 'Pre-Auth'
$ClsidPath = "SOFTWARE\Classes\CLSID\$Guid"
$CpPath = "SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\$Guid"
$registry = $null

try {
    if (![Environment]::Is64BitOperatingSystem -or ![Environment]::Is64BitProcess) {
        throw 'Run this script in 64-bit PowerShell on 64-bit Windows.'
    }
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    try {
        $principal = [Security.Principal.WindowsPrincipal]::new($identity)
        if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
            throw 'Open PowerShell as administrator.'
        }
    }
    finally { $identity.Dispose() }

    $destination = Join-Path $env:SystemRoot 'System32\Provider.dll'
    if (!$PSCmdlet.ShouldProcess("$Name ($Guid), $destination", 'Unregister the provider and delete its DLL')) { return }

    $registry = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine,
        [Microsoft.Win32.RegistryView]::Registry64
    )
    # Prevent new loads before attempting to delete a DLL that may be in use.
    $registry.DeleteSubKeyTree($CpPath, $false)
    Write-Host "[OK] $Name removed from the LogonUI registry."
    try {
        if (Test-Path -LiteralPath $destination) {
            Remove-Item -LiteralPath $destination -Force
            Write-Host "[OK] DLL deleted: $destination"
        }
    }
    catch {
        throw "Provider removed from LogonUI; could not delete ${destination}. Restart Windows and run this script again. $($_.Exception.Message)"
    }
    $registry.DeleteSubKeyTree($ClsidPath, $false)
    Write-Host "[OK] $Name uninstalled." -ForegroundColor Green
}
catch {
    Write-Error "Uninstallation incomplete: $($_.Exception.Message)" -ErrorAction Continue
    exit 1
}
finally {
    if ($null -ne $registry) { $registry.Dispose() }
}
