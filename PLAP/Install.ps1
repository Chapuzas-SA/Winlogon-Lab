#Requires -Version 5.1
[CmdletBinding(SupportsShouldProcess)]
param()

$ErrorActionPreference = 'Stop'
$RutaDll = Join-Path $PSScriptRoot 'build\PreAuthPLAP.dll'
$Guid = '{8B0C5DF7-3F70-4B90-9B70-61A63A3B5F25}'
$Nombre = 'PreAuth PLAP'
$ClsidPath = "SOFTWARE\Classes\CLSID\$Guid"
$PlapPath = "SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\PLAP Providers\$Guid"
$registro = $clase = $inproc = $provider = $null

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

    $origen = Get-Item -LiteralPath $RutaDll
    if ($origen.PSIsContainer) { throw 'The source path must point to a DLL file.' }
    $destino = Join-Path $env:SystemRoot 'System32\PreAuthPLAP.dll'

    if (!$PSCmdlet.ShouldProcess($destino, "Install $Nombre from $($origen.FullName) and register $Guid")) {
        return
    }

    # Copy before changing the registry: if the DLL is in use, fail here.
    if ($origen.FullName -ine $destino) {
        Copy-Item -LiteralPath $origen.FullName -Destination $destino -Force
    }

    $registro = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine,
        [Microsoft.Win32.RegistryView]::Registry64
    )
    $clase = $registro.CreateSubKey($ClsidPath)
    $inproc = $clase.CreateSubKey('InprocServer32')
    $clase.SetValue('', $Nombre)
    $inproc.SetValue('', $destino)
    $inproc.SetValue('ThreadingModel', 'Apartment')
    $provider = $registro.CreateSubKey($PlapPath)
    $provider.SetValue('', $Nombre)

    Write-Host "[OK] $Nombre registered: $destino" -ForegroundColor Green
    Write-Host 'Sign out to test the provider. If it does not refresh, restart Windows.'
}
catch {
    Write-Error "Installation failed: $($_.Exception.Message)" -ErrorAction Continue
    exit 1
}
finally {
    foreach ($clave in @($provider, $inproc, $clase, $registro)) {
        if ($null -ne $clave) { $clave.Dispose() }
    }
}
