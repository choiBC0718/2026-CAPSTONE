$ErrorActionPreference = 'Stop'

$engineBuildBat = 'C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\Build.bat'
$projectFile = Join-Path $PSScriptRoot 'P_CAP.uproject'
$buildTemp = Join-Path $PSScriptRoot 'Intermediate\BuildTemp'

if (-not (Test-Path -LiteralPath $engineBuildBat)) {
    throw "Unreal Engine 5.4 Build.bat was not found: $engineBuildBat"
}

New-Item -ItemType Directory -Force -Path $buildTemp | Out-Null

$oldUbtTemp = $env:UnrealBuildTool_TMP
$oldTemp = $env:TEMP
$oldTmp = $env:TMP
try {
    $env:UnrealBuildTool_TMP = $buildTemp
    $env:TEMP = $buildTemp
    $env:TMP = $buildTemp

    & $engineBuildBat P_CAPEditor Win64 Development "-Project=$projectFile" -WaitMutex -FromMsBuild -architecture=x64
    $buildExitCode = $LASTEXITCODE
}
finally {
    $env:UnrealBuildTool_TMP = $oldUbtTemp
    $env:TEMP = $oldTemp
    $env:TMP = $oldTmp
}

exit $buildExitCode
