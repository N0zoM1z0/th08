param(
    [string]$Executable = ""
)

$ErrorActionPreference = "Stop"

# This is a test-only, process-local patch for one VC7 reconstruction.  The
# original Japanese executable must never be patched, and every native rebuild
# can move the instruction or change the image hash.  Refuse unknown images.
$ExpectedSha256 = "db11de130f007bdcb793550c2a4b937f30968d17787e5acf511fe89b80bf9a20"
$InstructionRva = 0x0003E9E9
$ImmediateOffset = 1
$ExpectedInstruction = [byte[]]@(
    0x6A, 0xFF,                         # push -1
    0xB9, 0xB0, 0xD7, 0x61, 0x01,     # mov ecx, g_GameManager
    0xE8, 0x6F, 0xF2, 0xFE, 0xFF      # call GameManager::AddLives
)
$PatchedInstruction = [byte[]]$ExpectedInstruction.Clone()
$PatchedInstruction[$ImmediateOffset] = 0x00
$ZeroLifeDelta = [byte[]]@(0x00)

if ([string]::IsNullOrWhiteSpace($Executable)) {
    $DeployedExecutable = Join-Path $PSScriptRoot "th08-reconstructed.exe"
    $RepositoryExecutable = Join-Path $PSScriptRoot "..\build\th08.exe"
    if (Test-Path -LiteralPath $DeployedExecutable -PathType Leaf) {
        $Executable = $DeployedExecutable
    }
    else {
        $Executable = $RepositoryExecutable
    }
}
$Executable = [System.IO.Path]::GetFullPath($Executable)

$NativeSource = @'
using System;
using System.Runtime.InteropServices;

public static class Th08PreserveLivesNative
{
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool ReadProcessMemory(
        IntPtr process, IntPtr address, [Out] byte[] buffer,
        UIntPtr size, out UIntPtr bytesRead);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool WriteProcessMemory(
        IntPtr process, IntPtr address, byte[] buffer,
        UIntPtr size, out UIntPtr bytesWritten);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool VirtualProtectEx(
        IntPtr process, IntPtr address, UIntPtr size,
        uint newProtection, out uint oldProtection);

    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool FlushInstructionCache(
        IntPtr process, IntPtr address, UIntPtr size);
}
'@

function Throw-LastWin32Error([string]$Operation) {
    $Code = [Runtime.InteropServices.Marshal]::GetLastWin32Error()
    throw "$Operation failed with Win32 error $Code"
}
function Format-Bytes([byte[]]$Bytes) {
    return [BitConverter]::ToString($Bytes)
}

$GameProcess = $null
try {
    if (-not (Test-Path -LiteralPath $Executable -PathType Leaf)) {
        throw "Missing reconstructed executable: $Executable"
    }
    $ActualSha256 = (Get-FileHash -LiteralPath $Executable -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($ActualSha256 -ne $ExpectedSha256) {
        throw (
            "Unsupported executable hash: $ActualSha256. " +
            "This launcher accepts only reconstructed build $ExpectedSha256."
        )
    }

    Add-Type -TypeDefinition $NativeSource -Language CSharp
    $StartOptions = @{
        FilePath = $Executable
        WorkingDirectory = [System.IO.Path]::GetDirectoryName($Executable)
        PassThru = $true
    }
    $GameProcess = Start-Process @StartOptions

    $Deadline = [DateTime]::UtcNow.AddSeconds(10)
    $ModuleBase = [IntPtr]::Zero
    while ([DateTime]::UtcNow -lt $Deadline) {
        if ($GameProcess.HasExited) {
            throw "The reconstructed game exited before the preserve-lives patch was installed."
        }
        try {
            $GameProcess.Refresh()
            $ModuleBase = $GameProcess.MainModule.BaseAddress
            if ($ModuleBase -ne [IntPtr]::Zero) {
                break
            }
        }
        catch {
            # MainModule may be briefly unavailable while the PE loader starts.
        }
        Start-Sleep -Milliseconds 50
    }
    if ($ModuleBase -eq [IntPtr]::Zero) {
        throw "Timed out while locating the reconstructed executable in memory."
    }

    $InstructionAddress = [IntPtr]($ModuleBase.ToInt64() + $InstructionRva)
    $InstructionSize = [UIntPtr]::new([uint64]$ExpectedInstruction.Length)
    $Before = New-Object byte[] $ExpectedInstruction.Length
    $BytesRead = [UIntPtr]::Zero
    if (-not [Th08PreserveLivesNative]::ReadProcessMemory(
        $GameProcess.Handle,
        $InstructionAddress,
        $Before,
        $InstructionSize,
        [ref]$BytesRead
    )) {
        Throw-LastWin32Error "ReadProcessMemory"
    }
    if ($BytesRead.ToUInt64() -ne $ExpectedInstruction.Length) {
        throw "ReadProcessMemory returned an incomplete AddLives instruction sequence."
    }
    if ((Format-Bytes $Before) -ne (Format-Bytes $ExpectedInstruction)) {
        throw (
            "Unexpected bytes at Player::UpdateDeathAndRespawn + 0x509: " +
            (Format-Bytes $Before)
        )
    }

    $PatchAddress = [IntPtr]($InstructionAddress.ToInt64() + $ImmediateOffset)
    $PatchSize = [UIntPtr]::new([uint64]1)
    $OldProtection = 0
    if (-not [Th08PreserveLivesNative]::VirtualProtectEx(
        $GameProcess.Handle,
        $PatchAddress,
        $PatchSize,
        0x40,
        [ref]$OldProtection
    )) {
        Throw-LastWin32Error "VirtualProtectEx(enable write)"
    }
    try {
        $BytesWritten = [UIntPtr]::Zero
        if (-not [Th08PreserveLivesNative]::WriteProcessMemory(
            $GameProcess.Handle,
            $PatchAddress,
            $ZeroLifeDelta,
            $PatchSize,
            [ref]$BytesWritten
        )) {
            Throw-LastWin32Error "WriteProcessMemory"
        }
        if ($BytesWritten.ToUInt64() -ne 1) {
            throw "WriteProcessMemory did not replace the life-delta immediate."
        }
        if (-not [Th08PreserveLivesNative]::FlushInstructionCache(
            $GameProcess.Handle,
            $InstructionAddress,
            $InstructionSize
        )) {
            Throw-LastWin32Error "FlushInstructionCache"
        }

        $After = New-Object byte[] $ExpectedInstruction.Length
        $BytesRead = [UIntPtr]::Zero
        if (-not [Th08PreserveLivesNative]::ReadProcessMemory(
            $GameProcess.Handle,
            $InstructionAddress,
            $After,
            $InstructionSize,
            [ref]$BytesRead
        )) {
            Throw-LastWin32Error "ReadProcessMemory(verify patch)"
        }
        if (
            $BytesRead.ToUInt64() -ne $PatchedInstruction.Length -or
            (Format-Bytes $After) -ne (Format-Bytes $PatchedInstruction)
        ) {
            throw "The preserve-lives patch failed its full instruction read-back check."
        }
    }
    finally {
        $IgnoredProtection = 0
        if (-not [Th08PreserveLivesNative]::VirtualProtectEx(
            $GameProcess.Handle,
            $PatchAddress,
            $PatchSize,
            $OldProtection,
            [ref]$IgnoredProtection
        )) {
            Throw-LastWin32Error "VirtualProtectEx(restore protection)"
        }
    }

    Write-Host "TH08 preserve-lives patch installed in process $($GameProcess.Id)." -ForegroundColor Green
    Write-Host "Deaths, effects, drops, respawn, and UI bookkeeping remain active."
    Write-Host "Only AddLives(-1) became AddLives(0); the executable on disk was not modified."
    exit 0
}
catch {
    if ($null -ne $GameProcess -and -not $GameProcess.HasExited) {
        Stop-Process -Id $GameProcess.Id -Force -ErrorAction SilentlyContinue
    }
    Write-Error $_ -ErrorAction Continue
    exit 1
}
