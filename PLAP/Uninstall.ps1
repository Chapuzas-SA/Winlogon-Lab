#Requires -Version 5.1
[CmdletBinding(SupportsShouldProcess)]
param()

$ErrorActionPreference = 'Stop'
$Guid = '{8B0C5DF7-3F70-4B90-9B70-61A63A3B5F25}'
$Nombre = 'PreAuth PLAP'
$ClsidPath = "SOFTWARE\Classes\CLSID\$Guid"
$PlapPath = "SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\PLAP Providers\$Guid"
$registro = $null

try {
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT -or
        ![Environment]::Is64BitOperatingSystem -or ![Environment]::Is64BitProcess) {
        throw 'Run the script in 64-bit PowerShell on 64-bit Windows.'
    }
    $identidad = [Security.Principal.WindowsIdentity]::GetCurrent()
    try {
        $principal = [Security.Principal.WindowsPrincipal]::new($identidad)
        if (!$principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
            throw 'Run PowerShell as administrator.'
        }
    }
    finally { $identidad.Dispose() }

    $destino = Join-Path $env:SystemRoot 'System32\PreAuthPLAP.dll'

    if (!$PSCmdlet.ShouldProcess($destino, "Uninstall $Nombre and remove its registry entries for $Guid")) {
        return
    }

    # Remove registry entries first to prevent new loads, even if the DLL is in use.
    $registro = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine,
        [Microsoft.Win32.RegistryView]::Registry64
    )
    $registro.DeleteSubKeyTree($PlapPath, $false)
    $registro.DeleteSubKeyTree($ClsidPath, $false)

    # Allow uninstalling again if the keys or file no longer exist.
    if (Test-Path -LiteralPath $destino) {
        try { Remove-Item -LiteralPath $destino }
        catch {
            throw "Registry entries removed, but the DLL could not be deleted: $($_.Exception.Message). Sign out, sign back in, and rerun Uninstall.ps1; if the DLL is still in use, restart Windows."
        }
    }

    Write-Host "[OK] $Nombre uninstalled: $destino" -ForegroundColor Green
    Write-Host 'Sign out to refresh the sign-in screen. If the provider still appears, restart Windows.'
}
catch {
    Write-Error "Uninstallation failed: $($_.Exception.Message)" -ErrorAction Continue
    exit 1
}
finally {
    if ($null -ne $registro) { $registro.Dispose() }
}
