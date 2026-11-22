#Requires -RunAsAdministrator
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
if (-not [Environment]::Is64BitProcess) { throw 'Use 64-bit PowerShell.' }
$name = 'EoA_SysRecovery'
$accessibilityPaths = @(
    'HKCU:\Software\Microsoft\Windows NT\CurrentVersion\Accessibility'
    'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility'
)
foreach ($accessibility in $accessibilityPaths) {
    if (-not (Test-Path -LiteralPath $accessibility)) { continue }
    $current = (Get-ItemProperty -Path $accessibility -ErrorAction Stop).Configuration
    if ($null -ne $current) {
        $entries = @(([string]$current -split ',') | Where-Object { $_ -and $_ -ne $name })
        Set-ItemProperty -Path $accessibility -Name Configuration -Value ($entries -join ',')
    }
}
$key = "HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Accessibility\ATs\$name"
if (Test-Path -LiteralPath $key) { Remove-Item -LiteralPath $key -Recurse }
$executable = Join-Path $env:SystemRoot 'System32\EoASysRec.exe'

Get-Process -Name EoASysRec -ErrorAction SilentlyContinue | Where-Object {
    $_.Path -eq $executable
} | Stop-Process -Force
if (Test-Path -LiteralPath $executable) { Remove-Item -LiteralPath $executable }
$eventSource = "HKLM:\SYSTEM\CurrentControlSet\Services\EventLog\Application\$name"
if (Test-Path -LiteralPath $eventSource) { Remove-Item -LiteralPath $eventSource -Recurse }
Write-Host 'AT unregistered.'
