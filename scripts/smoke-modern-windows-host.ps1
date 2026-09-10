[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PlaytestDirectory,

    [ValidateRange(5, 120)]
    [int]$Seconds = 20
)

$ErrorActionPreference = 'Stop'
$playtestDirectory = (Resolve-Path -LiteralPath $PlaytestDirectory).Path
$executable = Join-Path $playtestDirectory 'th08-reconstructed.exe'

foreach ($requiredFile in @('th08-reconstructed.exe', 'd3dx8d.dll', 'th08.dat', 'thbgm.dat')) {
    $requiredPath = Join-Path $playtestDirectory $requiredFile
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Required playtest file not found: $requiredPath"
    }
}

$crashPath = Join-Path $playtestDirectory 'modern-crash.txt'
$crashTimestampBefore = if (Test-Path -LiteralPath $crashPath) {
    (Get-Item -LiteralPath $crashPath).LastWriteTimeUtc
} else {
    [datetime]::MinValue
}

$quotedDataDirectory = '"{0}"' -f $playtestDirectory
$process = Start-Process -FilePath $executable `
    -ArgumentList @('--windowed', '--data-dir', $quotedDataDirectory) `
    -WorkingDirectory $playtestDirectory `
    -PassThru

$deadline = [datetime]::UtcNow.AddSeconds($Seconds)
$windowObserved = $false
$windowTitle = ''

try {
    while ([datetime]::UtcNow -lt $deadline) {
        Start-Sleep -Milliseconds 500
        $process.Refresh()
        if ($process.HasExited) {
            throw "TH08 exited during startup with code $($process.ExitCode)."
        }
        if ($process.MainWindowHandle -ne 0) {
            $windowObserved = $true
            $windowTitle = $process.MainWindowTitle
        }
    }

    if (-not $windowObserved) {
        throw 'TH08 remained alive but did not publish a top-level window during the smoke interval.'
    }

    $process.Refresh()
    Write-Host "Startup window observed: handle=$($process.MainWindowHandle) title='$windowTitle'"
    Write-Host "Process survived the $Seconds-second native Windows smoke interval."

    if (-not $process.CloseMainWindow()) {
        throw 'Unable to request a normal close through the game window.'
    }
    if (-not $process.WaitForExit(15000)) {
        throw 'TH08 did not complete normal shutdown within 15 seconds.'
    }
    if ($process.ExitCode -ne 0) {
        throw "TH08 returned nonzero exit code $($process.ExitCode) after the close request."
    }

    if (Test-Path -LiteralPath $crashPath) {
        $crashTimestampAfter = (Get-Item -LiteralPath $crashPath).LastWriteTimeUtc
        if ($crashTimestampAfter -gt $crashTimestampBefore) {
            throw "TH08 wrote a new crash report: $crashPath"
        }
    }

    $hash = (Get-FileHash -LiteralPath $executable -Algorithm SHA256).Hash.ToLowerInvariant()
    Write-Host "Native Windows smoke passed: sha256=$hash exit=0"
} finally {
    $process.Refresh()
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force
        $process.WaitForExit()
    }
}
