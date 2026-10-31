# Garasi Amp - Windows x64 VST3 builder for Visual Studio 2026 and 2022.
# Use: double-click BUILD_WINDOWS.cmd (build, package, per-user install)
#      .\build-windows.ps1 -BuildOnly  (build and package, no install)
#      .\build-windows.ps1 -SystemWide (requires elevated PowerShell)
param(
    [switch]$BuildOnly,
    [switch]$SystemWide
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root 'build'
$dist = Join-Path $root 'dist'

Write-Host '=== GARASI AMP | WINDOWS VST3 BUILDER ===' -ForegroundColor Cyan

# Find Visual Studio with the C++ compiler installed.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Visual Studio Installer was not found. Install Visual Studio 2026 with Desktop development with C++.'
}
$vsDir = @(& $vswhere -latest -products '*' -requires 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64' -property installationPath) | Select-Object -First 1
$vsVersion = @(& $vswhere -latest -products '*' -requires 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64' -property installationVersion) | Select-Object -First 1
if (-not $vsDir -or -not $vsVersion) {
    throw 'C++ tools were not found. In Visual Studio Installer choose Modify and enable Desktop development with C++.'
}
$vsMajor = [int](($vsVersion -split '\.')[0])
switch ($vsMajor) {
    18 { $generator = 'Visual Studio 18 2026' }
    17 { $generator = 'Visual Studio 17 2022' }
    default { throw "Visual Studio version $vsVersion is not supported by this build script (requires 2026 or 2022)." }
}
Write-Host "Visual Studio detected: $vsVersion ($generator)" -ForegroundColor Cyan

# A CMake executable on PATH is not guaranteed when CMake comes from Visual Studio.
# For VS 2026 the CMake generator was added in CMake 4.2.
$cmakeCandidates = @()
$pathCmake = Get-Command 'cmake.exe' -ErrorAction SilentlyContinue
if ($pathCmake) { $cmakeCandidates += $pathCmake.Source }
$cmakeCandidates += (Join-Path $vsDir 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe')
$cmakeCandidates += (Join-Path $env:ProgramFiles 'CMake\bin\cmake.exe')
$cmakeExe = $null
foreach ($candidate in ($cmakeCandidates | Select-Object -Unique)) {
    if (-not (Test-Path -LiteralPath $candidate)) { continue }
    $helpText = (& $candidate --help | Out-String)
    if ($LASTEXITCODE -eq 0 -and $helpText.Contains($generator)) {
        $cmakeExe = $candidate
        break
    }
}
if (-not $cmakeExe) {
    if ($vsMajor -eq 18) {
        throw 'Compatible CMake not found. VS 2026 requires CMake 4.2 or newer for its generator. Install from https://cmake.org/download/ (add to PATH), then rerun BUILD_WINDOWS.cmd.'
    }
    throw 'Compatible CMake not found. Install CMake 3.24 or newer from https://cmake.org/download/.'
}
Write-Host "CMake detected: $cmakeExe" -ForegroundColor Cyan

# Git must be accessible to CMake FetchContent to fetch JUCE on the first build.
$gitCandidates = @()
$pathGit = Get-Command 'git.exe' -ErrorAction SilentlyContinue
if ($pathGit) { $gitCandidates += $pathGit.Source }
$gitCandidates += (Join-Path $env:ProgramFiles 'Git\cmd\git.exe')
$gitCandidates += (Join-Path $vsDir 'Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd\git.exe')
$gitCandidates += (Join-Path $vsDir 'Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\bin\git.exe')
$gitExe = $null
foreach ($candidate in ($gitCandidates | Select-Object -Unique)) {
    if (Test-Path -LiteralPath $candidate) {
        $gitExe = $candidate
        break
    }
}
if (-not $gitExe) {
    throw 'Git was not found. Install Git for Windows from https://git-scm.com/downloads/win then rerun BUILD_WINDOWS.cmd.'
}
$env:PATH = (Split-Path -Parent $gitExe) + ';' + $env:PATH
Write-Host "Git detected: $gitExe" -ForegroundColor Cyan

# Avoid CMake's generator mismatch after upgrading Visual Studio.
$cache = Join-Path $build 'CMakeCache.txt'
if (Test-Path -LiteralPath $cache) {
    $oldGenerator = @(Select-String -LiteralPath $cache -Pattern '^CMAKE_GENERATOR:INTERNAL=(.*)$' | Select-Object -First 1)
    if ($oldGenerator.Count -gt 0 -and $oldGenerator[0].Matches[0].Groups[1].Value -ne $generator) {
        Write-Host 'Removing outdated CMake build cache...' -ForegroundColor Yellow
        Remove-Item -LiteralPath $build -Recurse -Force
    }
}

Write-Host '[1/4] Configure project (first build downloads JUCE; internet required)' -ForegroundColor Yellow
& $cmakeExe -S $root -B $build -G $generator -A x64 -DGARASI_BUILD_PLUGIN=ON -DGARASI_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed. Please send the last error lines or a screenshot.' }

Write-Host '[2/4] Compile plugin (Release, Windows 64-bit)' -ForegroundColor Yellow
& $cmakeExe --build $build --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed. Please send the last error lines or a screenshot.' }

Write-Host '[3/4] Run DSP tests' -ForegroundColor Yellow
$ctestExe = Join-Path (Split-Path -Parent $cmakeExe) 'ctest.exe'
& $ctestExe --test-dir $build --config Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'DSP tests failed. Plugin was not installed.' }

Write-Host '[4/4] Package VST3' -ForegroundColor Yellow
$bundles = @(Get-ChildItem -Path $build -Recurse -Directory -Filter '*.vst3' |
    Where-Object { $_.FullName -match 'artefacts[\\/].*[\\/]VST3[\\/]' })
if ($bundles.Count -eq 0) { throw 'VST3 bundle not found under the build artefacts folder.' }
$bundle = $bundles[0]
New-Item -ItemType Directory -Force -Path $dist | Out-Null
$zipFile = Join-Path $dist 'GarasiAmp-Windows-x64-VST3.zip'
if (Test-Path -LiteralPath $zipFile) { Remove-Item -LiteralPath $zipFile -Force }
Compress-Archive -LiteralPath $bundle.FullName -DestinationPath $zipFile -CompressionLevel Optimal
Write-Host "VST3 package: $zipFile" -ForegroundColor Green

if ($BuildOnly) {
    Write-Host 'BuildOnly: plugin was packaged but not installed.'
    exit 0
}

if ($SystemWide) {
    $principal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'For -SystemWide, open PowerShell as Administrator, then run this script again.'
    }
    $target = Join-Path $env:ProgramFiles 'Common Files\VST3'
} else {
    $target = Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3'
}
New-Item -ItemType Directory -Force -Path $target | Out-Null
$installed = Join-Path $target $bundle.Name
if (Test-Path -LiteralPath $installed) { Remove-Item -LiteralPath $installed -Recurse -Force }
Copy-Item -LiteralPath $bundle.FullName -Destination $target -Recurse -Force
Write-Host "INSTALLED: $installed" -ForegroundColor Green
Write-Host 'Restart Studio One, then open Effects and search for Garasi Amp.' -ForegroundColor Green
