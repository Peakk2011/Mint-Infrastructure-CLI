param(
    [string]$SourceDir = "src\renderer",
    [string]$TargetDir = "dist\renderer"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $SourceDir)) {
    throw "Renderer source folder not found at '$SourceDir'"
}

if (-not (Test-Path $TargetDir)) {
    New-Item -Path $TargetDir -ItemType Directory -Force | Out-Null
}

Copy-Item -Path (Join-Path $SourceDir "*") -Destination $TargetDir -Recurse -Force

Write-Host "Synced renderer assets to $TargetDir"