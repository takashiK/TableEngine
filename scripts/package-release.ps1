<#
.SYNOPSIS
    Builds TableEngine in Release mode and creates a distributable ZIP package.

.DESCRIPTION
    This script:
      1. Configures the project (if not yet configured) using the
         "Qt-MSVC2022-amd64-Ninja" CMake preset (Ninja Multi-Config).
      2. Builds the Release configuration.
      3. Installs the exe to a staging directory, then runs windeployqt6
         to collect all Qt runtime files.
      4. Runs CPack to produce a ZIP archive in the build output directory.

.PARAMETER QtDir
    Root of the MSVC 64-bit Qt kit, e.g. C:\Qt\6.11.1\msvc2022_64
    Defaults to C:\Qt\6.11.1\msvc2022_64

.PARAMETER SkipConfigure
    Pass this switch to skip the cmake configure step (use when the project
    is already configured).

.EXAMPLE
    .\scripts\package-release.ps1
    .\scripts\package-release.ps1 -QtDir "C:\Qt\6.11.1\msvc2022_64" -SkipConfigure
#>
param(
    [string]$QtDir = "C:\Qt\6.11.1\msvc2022_64",
    [switch]$SkipConfigure
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
$RepoRoot    = Split-Path -Parent $PSScriptRoot
$PresetName  = "Qt-MSVC2022-amd64-Ninja"
$BuildDir    = Join-Path $RepoRoot "out\build\$PresetName"
$InstallDir  = Join-Path $RepoRoot "out\install\$PresetName\Release"
$Windeployqt = Join-Path $QtDir "bin\windeployqt6.exe"

# Make sure the Visual Studio / Build Tools developer environment is on PATH.
# -products * is required to detect "Build Tools for Visual Studio" installations
# (without it vswhere only returns full IDE editions).
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $VsWhere)) {
    throw "vswhere.exe not found. Install Visual Studio Build Tools 2022."
}

$vsInstallPath = & $VsWhere -products * -latest -property installationPath 2>$null
if (-not $vsInstallPath) {
    throw "No Visual Studio or Build Tools installation found by vswhere."
}

$vcvars = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at: $vcvars"
}

# Clear any stale VS environment state left by a previous (partial) run of this script
# in the same terminal session.  vcvars64.bat reads VSCMD_VER / VSINSTALLDIR on entry
# and will error out if those point to an old installation that no longer exists.
foreach ($v in @('VSINSTALLDIR', 'VCINSTALLDIR', 'VSCMD_VER', 'VisualStudioVersion',
                 'VSCMD_START_DIR', 'VSCMD_SKIP_SENDTELEMETRY', '__VSCMD_PREINIT_PATH__',
                 'VCToolsInstallDir', 'VCToolsVersion', 'VS170COMNTOOLS')) {
    [System.Environment]::SetEnvironmentVariable($v, $null, 'Process')
}

# Import MSVC environment variables.
# Writing `set` output to a temp file avoids the encoding/buffering issues
# that occur when capturing cmd.exe stdout directly in PowerShell, which can
# silently corrupt the PATH value and leave cl.exe unreachable.
$tmpEnvFile = [System.IO.Path]::GetTempFileName()
try {
    cmd /c """$vcvars"" > nul 2>&1 && set > ""$tmpEnvFile"""
    Get-Content $tmpEnvFile -Encoding Default | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
        }
    }
} finally {
    Remove-Item $tmpEnvFile -ErrorAction SilentlyContinue
}

# Verify cl.exe is actually reachable in PATH after the import.
# If not (import was incomplete), fall back to locating it via vswhere -find.
if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
    Write-Warning "cl.exe not in PATH after vcvars import — trying vswhere -find fallback."
    $clExe = & $VsWhere -products * -latest -find "VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe" 2>$null |
             Select-Object -Last 1
    if ($clExe) {
        $env:PATH = "$(Split-Path -Parent $clExe);$env:PATH"
        Write-Host "[OK] MSVC compiler directory added to PATH: $(Split-Path -Parent $clExe)" -ForegroundColor Yellow
    } else {
        throw "cl.exe not found after loading vcvars. Check your Visual Studio Build Tools 2022 installation."
    }
}

Write-Host "[OK] Build Tools for Visual Studio 2022 environment loaded." -ForegroundColor Green

# Add Qt bin and Qt-bundled Ninja to PATH so cmake can find Qt6Config.cmake
# and invoke ninja.exe during configure/build.
$env:PATH = "$QtDir\bin;C:\Qt\Tools\Ninja;$env:PATH"
$env:Qt6_DIR = "$QtDir\lib\cmake\Qt6"

# ---------------------------------------------------------------------------
# 1. Configure
# ---------------------------------------------------------------------------
if (-not $SkipConfigure) {
    Write-Host "`n=== Configuring (preset: '$PresetName') ===" -ForegroundColor Cyan
    # --fresh deletes any existing CMakeCache.txt before configuring.
    # This prevents cmake from detecting "changed variables" (e.g. when VS Code CMake
    # Tools cached a full compiler path but the preset specifies just "cl.exe"), which
    # would cause cmake to spawn an internal re-run process that may not inherit PATH.
    cmake --preset $PresetName --fresh `
        -DCMAKE_PREFIX_PATH="$QtDir"
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
