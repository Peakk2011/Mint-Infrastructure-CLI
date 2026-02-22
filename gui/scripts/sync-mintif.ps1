param(
    [string]$SourceRoot = "..\dist",
    [string]$TargetRoot = "vendor\mintif"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $PSCommandPath
$guiRoot = (Resolve-Path (Join-Path $scriptDir "..")).Path

function Resolve-ProjectPath {
    param(
        [string]$BaseDir,
        [string]$CandidatePath
    )

    if ([System.IO.Path]::IsPathRooted($CandidatePath)) {
        return $CandidatePath
    }

    return (Join-Path $BaseDir $CandidatePath)
}

$resolvedSourceRoot = Resolve-ProjectPath -BaseDir $guiRoot -CandidatePath $SourceRoot
$resolvedTargetRoot = Resolve-ProjectPath -BaseDir $guiRoot -CandidatePath $TargetRoot

$sourceExe = Join-Path $resolvedSourceRoot "mintif.exe"
$sourceCss = Join-Path $resolvedSourceRoot "styles.css"

if (-not (Test-Path $sourceExe)) {
    throw "mintif.exe not found at '$sourceExe'"
}

if (-not (Test-Path $sourceCss)) {
    throw "styles.css not found at '$sourceCss'"
}

if (-not (Test-Path $resolvedTargetRoot)) {
    New-Item -Path $resolvedTargetRoot -ItemType Directory | Out-Null
}

Copy-Item -Path $sourceExe -Destination (Join-Path $resolvedTargetRoot "mintif.exe") -Force
Copy-Item -Path $sourceCss -Destination (Join-Path $resolvedTargetRoot "styles.css") -Force

Write-Host "Synced mintif.exe and styles.css to $resolvedTargetRoot"