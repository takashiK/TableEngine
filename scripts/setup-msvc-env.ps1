param(
    [ValidateSet('x64', 'x86', 'amd64')]
    [string]$Arch = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found. This runner image does not provide Visual Studio Build Tools."
}

$vsInstallPath = & $vswhere -products * -latest -property installationPath 2>$null
if (-not $vsInstallPath) {
    throw 'No Visual Studio installation found by vswhere.'
}

$vcvars = Join-Path $vsInstallPath 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vcvars)) {
    throw "vcvars64.bat not found at: $vcvars"
}

$tmpEnvFile = [System.IO.Path]::GetTempFileName()
try {
    cmd /c "`"$vcvars`" $Arch > nul 2>&1 && set > `"$tmpEnvFile`""

    Get-Content $tmpEnvFile -Encoding Default | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            $name = $Matches[1]
            $value = $Matches[2]

            if ($name.StartsWith('=')) {
                return
            }

            # Update current process and persist for subsequent workflow steps.
            [System.Environment]::SetEnvironmentVariable($name, $value, 'Process')
            "${name}=${value}" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
        }
    }
}
finally {
    Remove-Item $tmpEnvFile -ErrorAction SilentlyContinue
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'MSVC initialization failed: cl.exe is not available in PATH.'
}

Write-Host '[OK] MSVC developer environment initialized.' -ForegroundColor Green
