param(
    [string]$SourceRoot = "..\Mint Infrastructure CLI\dist",
    [string]$TargetRoot = "vendor\mintif"
)

$ErrorActionPreference = "Stop"

$sourceExe = Join-Path $SourceRoot "mintif.exe"
$sourceCss = Join-Path $SourceRoot "styles.css"

if (-not (Test-Path $sourceExe)) {
    throw "mintif.exe not found at '$sourceExe'"
}

if (-not (Test-Path $sourceCss)) {
    throw "styles.css not found at '$sourceCss'"
}

if (-not (Test-Path $TargetRoot)) {
    New-Item -Path $TargetRoot -ItemType Directory | Out-Null
}

Copy-Item -Path $sourceExe -Destination (Join-Path $TargetRoot "mintif.exe") -Force
Copy-Item -Path $sourceCss -Destination (Join-Path $TargetRoot "styles.css") -Force

Write-Host "Synced mintif.exe and styles.css to $TargetRoot"