#Requires -Version 5.1
[CmdletBinding()]
param(
    [string]$EngineRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# PowerShell 7 can turn native non-zero exits into errors; we inspect them explicitly.
if (Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

function Resolve-EngineRoot {
    param([string]$ExplicitRoot)

    if (-not [string]::IsNullOrWhiteSpace($ExplicitRoot)) {
        return $ExplicitRoot
    }
    if (-not [string]::IsNullOrWhiteSpace($env:UE_5_8_ROOT)) {
        return $env:UE_5_8_ROOT
    }

    # Use the same Windows Launcher installation manifest as DesktopPlatform in UE 5.8.
    $manifestPath = Join-Path ([Environment]::GetFolderPath('CommonApplicationData')) 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'
    if (Test-Path -LiteralPath $manifestPath -PathType Leaf) {
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        $installations = @($manifest.InstallationList | Where-Object { $_.AppName -ceq 'UE_5.8' })
        if ($installations.Count -eq 1 -and
            -not [string]::IsNullOrWhiteSpace($installations[0].InstallLocation)) {
            return $installations[0].InstallLocation
        }
    }
    throw 'Cannot resolve a unique Launcher UE 5.8 installation.'
}

function Get-ReportCount {
    param($Report, [string]$Name)

    $property = $Report.PSObject.Properties[$Name]
    if ($null -eq $property -or
        ($property.Value -isnot [int] -and $property.Value -isnot [long]) -or
        $property.Value -lt 0 -or $property.Value -gt [int]::MaxValue) {
        throw "Automation report must contain a non-negative int32 count '$Name'."
    }
    return [long]$property.Value
}

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$projectPath = Join-Path $repoRoot 'BertaDevKitHost\BertaDevKitHost.uproject'
$savedPath = Join-Path $repoRoot 'BertaDevKitHost\Saved'
$artifactPath = Join-Path $savedPath 'Validation\BertaDevKit'
$buildLog = Join-Path $artifactPath 'Build.log'
$automationLog = Join-Path $artifactPath 'Automation.log'
$reportPath = Join-Path $artifactPath 'Automation'
$reportIndex = Join-Path $reportPath 'index.json'
$stage = 'Setup'

try {
    if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf)) {
        throw "Host project not found: $projectPath"
    }

    $stage = 'Artifacts'
    # Never clean outside this runner's fixed Saved directory or through directory links.
    $resolvedArtifacts = [IO.Path]::GetFullPath($artifactPath)
    $savedPrefix = [IO.Path]::GetFullPath($savedPath).TrimEnd('\') + '\'
    if (-not $resolvedArtifacts.StartsWith($savedPrefix, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Artifact directory is outside Saved: $resolvedArtifacts"
    }
    $ancestor = $resolvedArtifacts
    while ($ancestor) {
        if (Test-Path -LiteralPath $ancestor) {
            $item = Get-Item -LiteralPath $ancestor -Force
            if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
                throw "Refusing artifact cleanup through a directory link: $ancestor"
            }
        }
        $ancestor = Split-Path $ancestor -Parent
    }
    if (Test-Path -LiteralPath $artifactPath) {
        $links = @(Get-ChildItem -LiteralPath $artifactPath -Recurse -Force |
            Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint })
        if ($links.Count -gt 0) {
            throw "Refusing artifact cleanup containing a link: $($links[0].FullName)"
        }
        Get-ChildItem -LiteralPath $artifactPath -Force | ForEach-Object {
            Remove-Item -LiteralPath $_.FullName -Recurse -Force
        }
    }
    New-Item -ItemType Directory -Path $reportPath -Force | Out-Null

    $stage = 'Engine'
    try {
        $EngineRoot = [IO.Path]::GetFullPath((Resolve-EngineRoot $EngineRoot))
        $buildTool = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
        $editorTool = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
        $versionPath = Join-Path $EngineRoot 'Engine\Build\Build.version'
        foreach ($requiredFile in @($buildTool, $editorTool, $versionPath)) {
            if (-not (Test-Path -LiteralPath $requiredFile -PathType Leaf)) {
                throw "Required engine file not found: $requiredFile"
            }
        }
        $version = Get-Content -LiteralPath $versionPath -Raw | ConvertFrom-Json
        if ($version.MajorVersion -ne 5 -or $version.MinorVersion -ne 8) {
            throw "Only UE 5.8 is supported; Build.version reports $($version.MajorVersion).$($version.MinorVersion)."
        }
    }
    catch {
        throw "$($_.Exception.Message) Supply -EngineRoot '<UE_5.8 installation>' or set UE_5_8_ROOT."
    }
    Write-Host "Engine: $EngineRoot (UE 5.8)"

    $stage = 'Build'
    Write-Host 'Building BertaDevKitHostEditor Win64 Development...'
    # Continue permits redirected native stderr on Windows PowerShell 5.1.
    # Tee itself must still fail on log-write errors. Nothing runs between the pipeline and exit capture.
    $ErrorActionPreference = 'Continue'
    & $buildTool BertaDevKitHostEditor Win64 Development "-Project=$projectPath" -WaitMutex 2>&1 |
        Tee-Object -FilePath $buildLog -ErrorAction Stop
    $buildExitCode = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    if ($buildExitCode -ne 0) {
        throw "Build exited with code $buildExitCode. Automation Tests were not started."
    }
    Write-Host 'Build: PASSED'

    $stage = 'Automation'
    # BertaGASCompanionExt also uses BertaDevKit.*. Select only this plugin's current branches.
    # Keep this list in sync when adding a new top-level test branch to BertaDevKit.
    $testBranches = @(
        'Actor', 'AI', 'AssetCleaner', 'AssetInsights', 'AssetNaming', 'BlueprintAudit',
        'Collision', 'Component', 'Controller', 'DelegateInspector', 'Editor',
        'Localization', 'Math', 'ObjectGraph', 'TickGraph', 'World'
    )
    $testFilter = ($testBranches | ForEach-Object { "StartsWith:BertaDevKit.$_" }) -join '+'
    $editorArguments = @(
        $projectPath,
        "-ExecCmds=Automation RunTests $testFilter",
        '-TestExit=Automation Test Queue Empty',
        '-unattended', '-nop4', '-nosplash', '-NullRHI', '-stdout', '-FullStdOutLogOutput',
        "-ReportExportPath=$reportPath",
        "-ABSLOG=$automationLog"
    )
    Write-Host "Running Automation Tests: $testFilter"
    $ErrorActionPreference = 'Continue'
    & $editorTool @editorArguments
    $automationExitCode = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    Write-Host "Automation process exit code: $automationExitCode"

    $stage = 'Automation report'
    if (-not (Test-Path -LiteralPath $reportIndex -PathType Leaf)) {
        throw "Automation report missing: $reportIndex (process exit code: $automationExitCode)."
    }
    $report = Get-Content -LiteralPath $reportIndex -Raw | ConvertFrom-Json
    $succeeded = Get-ReportCount $report 'succeeded'
    $warnings = Get-ReportCount $report 'succeededWithWarnings'
    $failed = Get-ReportCount $report 'failed'
    $notRun = Get-ReportCount $report 'notRun'
    $inProcess = Get-ReportCount $report 'inProcess'
    $executed = $succeeded + $warnings + $failed
    Write-Host "Tests: $succeeded passed, $warnings succeeded with warnings, $failed failed, $notRun not run, $inProcess in process"
    if ($automationExitCode -ne 0 -or $executed -eq 0 -or
        $failed -gt 0 -or $notRun -gt 0 -or $inProcess -gt 0) {
        throw "Automation validation failed (process exit code: $automationExitCode; executed: $executed)."
    }

    Write-Host 'BertaDevKit validation PASSED'
    Write-Host "Engine: $EngineRoot"
    Write-Host 'Build: PASSED'
    Write-Host "Report: $reportIndex"
    Write-Host "Build log: $buildLog"
    Write-Host "Automation log: $automationLog"
    exit 0
}
catch {
    Write-Host "BertaDevKit validation FAILED [$stage]: $($_.Exception.Message)"
    Write-Host "Engine: $EngineRoot"
    Write-Host "Build log: $buildLog"
    Write-Host "Automation log: $automationLog"
    Write-Host "Report: $reportIndex"
    exit 1
}
