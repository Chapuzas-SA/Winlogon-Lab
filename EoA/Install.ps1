#Requires -RunAsAdministrator
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
if (-not [Environment]::Is64BitProcess) { throw 'Use 64-bit PowerShell.' }
$source = Join-Path $PSScriptRoot 'build\EoASysRec.exe'
if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw 'Build the executable first: make'
}

$name = 'EoA_SysRecovery'
$configurationName = $name.ToLowerInvariant()
$directory = Join-Path $env:SystemRoot 'System32'
$executable = Join-Path $directory 'EoASysRec.exe'
$key = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility\ATs\$name"
Copy-Item -LiteralPath $source -Destination $executable -Force

if (-not (Test-Path -LiteralPath $key -ErrorAction Stop)) {
    New-Item -Path $key -ErrorAction Stop | Out-Null
}
$values = @{
    ApplicationName = "@$executable,-101"
    Description = "@$executable,-102"
    ATExe = 'EoASysRec.exe'
    StartExe = $executable
    Profile = '<HCIModel><Accommodation type="mild dexterity"/></HCIModel>'
    SimpleProfile = 'eoasysrec'
}
foreach ($entry in $values.GetEnumerator()) {
    New-ItemProperty -Path $key -Name $entry.Key -Value $entry.Value -PropertyType String -Force | Out-Null
}
New-ItemProperty -Path $key -Name TerminateOnDesktopSwitch -Value 1 -PropertyType DWord -Force | Out-Null
New-ItemProperty -Path $key -Name PassiveAutoStartBehavior -Value 1 -PropertyType DWord -Force | Out-Null

$accessibility = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility'
if (-not (Test-Path -LiteralPath $accessibility -ErrorAction Stop)) {
    New-Item -Path $accessibility -ErrorAction Stop | Out-Null
}
$current = (Get-ItemProperty -Path $accessibility -ErrorAction Stop).Configuration
$entries = @(([string]$current -split ',') | Where-Object { $_ -and $_ -ne $name })
$entries += $configurationName

New-ItemProperty -Path $accessibility -Name Configuration -Value ($entries -join ',') -PropertyType String -Force | Out-Null
Write-Host "AT registered: $executable"
