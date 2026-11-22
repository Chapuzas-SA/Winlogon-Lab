[CmdletBinding(SupportsShouldProcess)]
param()

$ErrorActionPreference = 'Stop'
$DllPath = Join-Path $PSScriptRoot 'build\Provider.dll'
$Guid = '{e2e9a68f-4d2a-4b6e-9ebe-0c2f19e92d23}'
$Name = 'Pre-Auth'
$ClsidPath = "SOFTWARE\Classes\CLSID\$Guid"
$CpPath = "SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\$Guid"
$registry = $classKey = $inproc = $provider = $null

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

    $source = Get-Item -LiteralPath $DllPath
    if ($source.PSIsContainer) { throw 'DllPath must refer to a DLL file.' }
    $destination = Join-Path $env:SystemRoot 'System32\Provider.dll'

    if (!$PSCmdlet.ShouldProcess($destination, "Install $Name from $($source.FullName) and register $Guid")) {
        return
    }

    # Copy before updating the registry: if the DLL is in use, the update fails here.
    if ($source.FullName -ine $destination) {
        Copy-Item -LiteralPath $source.FullName -Destination $destination -Force
    }

    $registry = [Microsoft.Win32.RegistryKey]::OpenBaseKey(
        [Microsoft.Win32.RegistryHive]::LocalMachine,
        [Microsoft.Win32.RegistryView]::Registry64
    )
    $classKey = $registry.CreateSubKey($ClsidPath)
    $inproc = $classKey.CreateSubKey('InprocServer32')
    $classKey.SetValue('', $Name)
    $inproc.SetValue('', $destination)
    $inproc.SetValue('ThreadingModel', 'Apartment')
    $provider = $registry.CreateSubKey($CpPath)
    $provider.SetValue('', $Name)

    Write-Host "[OK] $Name registered: $destination" -ForegroundColor Green
    Write-Host 'Sign out or restart Windows to load the new version.'
}
catch {
    Write-Error "Installation failed: $($_.Exception.Message)" -ErrorAction Continue
    exit 1
}
finally {
    foreach ($key in @($provider, $inproc, $classKey, $registry)) {
        if ($null -ne $key) { $key.Dispose() }
    }
}
