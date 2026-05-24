<#
.SYNOPSIS
    Builds TableEngine in Release mode and creates a distributable ZIP package.

.DESCRIPTION
    This script:
      1. Configures the project (if not yet configured) using the
         "Visual Studio Community 2022 Release - amd64" CMake preset.
      2. Builds the Release configuration.
      3. Installs the exe + support DLLs to a staging directory, then runs
         windeployqt6 to collect all Qt runtime files.
      4. Runs CPack to produce a ZIP archive in the build output directory.

.PARAMETER QtDir
    Root of the MSVC 64-bit Qt kit, e.g. C:\Qt\6.10.2\msvc2022_64
    Defaults to C:\Qt\6.10.2\msvc2022_64

.PARAMETER SkipConfigure
    Pass this switch to skip the cmake configure step (use when the project
    is already configured).

.EXAMPLE
    .\scripts\package-release.ps1
    .\scripts\package-release.ps1 -QtDir "C:\Qt\6.10.2\msvc2022_64" -SkipConfigure
#>
param(
    [string]$QtDir = "C:\Qt\6.10.2\msvc2022_64",
    [switch]$SkipConfigure
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
$RepoRoot    = Split-Path -Parent $PSScriptRoot
$PresetName  = "Visual Studio Community 2022 Release - amd64"
$BuildDir    = Join-Path $RepoRoot "out\build\$PresetName"
$InstallDir  = Join-Path $RepoRoot "out\install\$PresetName\Release"
$Windeployqt = Join-Path $QtDir "bin\windeployqt6.exe"

# Make sure the Visual Studio developer tools are on PATH
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $VsWhere) {
    $vsInstallPath = & $VsWhere -latest -property installationPath
    $vcvars = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $vcvars) {
        # Import VS environment variables into the current session
        $envDump = cmd /c "`"$vcvars`" >nul 2>&1 && set"
        foreach ($line in $envDump) {
            if ($line -match '^([^=]+)=(.*)$') {
                [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
            }
        }
        Write-Host "[OK] Visual Studio 2022 environment loaded." -ForegroundColor Green
    }
}

# Add Qt bin to PATH so cmake can find Qt6Config.cmake, etc.
$env:PATH = "$QtDir\bin;$env:PATH"
$env:Qt6_DIR = "$QtDir\lib\cmake\Qt6"

# ---------------------------------------------------------------------------
# 1. Configure
# ---------------------------------------------------------------------------
if (-not $SkipConfigure) {
    Write-Host "`n=== Configuring (preset: '$PresetName') ===" -ForegroundColor Cyan
    cmake --preset $PresetName `
        -DCMAKE_PREFIX_PATH="$QtDir" | Tee-Object -Variable _out
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed (exit $LASTEXITCODE)" }
}

# ---------------------------------------------------------------------------
# 2. Build Release
# ---------------------------------------------------------------------------
Write-Host "`n=== Building Release ===" -ForegroundColor Cyan
cmake --build $BuildDir --config Release --target TableEngine -j
if ($LASTEXITCODE -ne 0) { throw "cmake build failed (exit $LASTEXITCODE)" }

# ---------------------------------------------------------------------------
# 3. Install to staging directory
# ---------------------------------------------------------------------------
Write-Host "`n=== Installing to staging directory ===" -ForegroundColor Cyan
if (Test-Path $InstallDir) {
    Remove-Item -Recurse -Force $InstallDir
}
cmake --install $BuildDir --config Release --prefix $InstallDir
if ($LASTEXITCODE -ne 0) { throw "cmake install failed (exit $LASTEXITCODE)" }

# ---------------------------------------------------------------------------
# 4. Run windeployqt6 (in case cmake install CODE block could not run it)
# ---------------------------------------------------------------------------
if (Test-Path $Windeployqt) {
    $ExePath = Join-Path $InstallDir "TableEngine.exe"
    if (Test-Path $ExePath) {
        Write-Host "`n=== Running windeployqt6 ===" -ForegroundColor Cyan
        # Options explained:
        #   --release           : assume Release binaries
        #   --no-translations   : skip Qt translation files (reduce size)
        #   --no-quick-import   : skip QML Quick modules (not used in this project)
        #   --no-opengl-sw      : skip software OpenGL rasterizer (opengl32sw.dll)
        #   --no-ffmpeg         : skip FFmpeg multimedia libs (not used)
        #   --skip-plugin-types qmltooling  : skip QML debugging plugin
        #   --exclude-plugins qtposition_nmea  : this plugin requires Qt6SerialPort
        #                                        which is not in this Qt installation
        & $Windeployqt `
            --release `
            --no-translations `
            --no-quick-import `
            --no-opengl-sw `
            --no-ffmpeg `
            --skip-plugin-types qmltooling `
            --exclude-plugins qtposition_nmea `
            --dir $InstallDir `
            $ExePath
        if ($LASTEXITCODE -ne 0) { throw "windeployqt6 failed (exit $LASTEXITCODE)" }
    }
} else {
    Write-Warning "windeployqt6 not found at: $Windeployqt"
    Write-Warning "Qt runtime DLLs were not collected. Deploy them manually."
}

# ---------------------------------------------------------------------------
# 5. Create ZIP with CPack
# ---------------------------------------------------------------------------
Write-Host "`n=== Creating ZIP package with CPack ===" -ForegroundColor Cyan
Push-Location $BuildDir
cpack --config CPackConfig.cmake -G ZIP -C Release
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "cpack failed (exit $LASTEXITCODE)"
}
Pop-Location

# Show result
$ZipFiles = Get-ChildItem $BuildDir -Filter "TableEngine-*.zip"
if ($ZipFiles) {
    Write-Host "`n=== Package created ===" -ForegroundColor Green
    $ZipFiles | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
} else {
    Write-Warning "No ZIP file found in $BuildDir"
}
