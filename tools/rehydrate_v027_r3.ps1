$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$controller = Join-Path $root 'generated\donor159\src\controller.cpp'
$patch = Join-Path $root 'patches\v027_r3_multidelete.patch'
$canonicalPatch = Join-Path $root 'generated\_v027_r3_multidelete.patch'

& (Join-Path $PSScriptRoot 'rehydrate_v027_r2.ps1')

$r2Hash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($r2Hash -ne 'de141e34f07903c3e490d9684410309f4e0d3a49d7e36438b76a9e941e8cd6e2') {
    throw "v0.2.7-R2 base SHA mismatch before R3: $r2Hash"
}

# GitHub checkout on Windows may materialize text patches as CRLF. Canonicalize the transport only;
# runtime source logic/patch content is unchanged.
$patchText = [IO.File]::ReadAllText($patch).Replace("`r`n", "`n")
[IO.File]::WriteAllText($canonicalPatch, $patchText, [Text.UTF8Encoding]::new($false))
$patchHash = (Get-FileHash -Algorithm SHA256 $canonicalPatch).Hash.ToLowerInvariant()
if ($patchHash -ne '3b013821934c882cce8dc755894f66ab835feec394d3433015127a8792fc2136') {
    throw "v0.2.7-R3 multi-delete patch SHA mismatch: $patchHash"
}

$beforeText = [IO.File]::ReadAllText($controller)
$f4Pattern = '(?ms)^    void ToggleGlobalPause\(\) \{.*?^    \}'
$beforeF4 = [regex]::Match($beforeText, $f4Pattern).Value
if ([string]::IsNullOrWhiteSpace($beforeF4)) { throw 'Cannot locate protected F4 block before R3 patch' }

Push-Location $root
try {
    & git apply --whitespace=nowarn $canonicalPatch
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.7-R3 multi-delete patch' }
} finally { Pop-Location }

$normalized = [IO.File]::ReadAllText($controller).Replace("`r`n", "`n")
[IO.File]::WriteAllText($controller, $normalized, [Text.UTF8Encoding]::new($false))

$finalHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($finalHash -ne 'a69fa0df4932e4020aed6e61b4109bd2c558db5c407afedb46c03456fb575abf') {
    throw "v0.2.7-R3 final controller SHA mismatch: $finalHash"
}

$afterText = [IO.File]::ReadAllText($controller)
$afterF4 = [regex]::Match($afterText, $f4Pattern).Value
if ($afterF4 -cne $beforeF4) { throw 'PROTECTED F4 block changed by R3 multi-delete patch' }
if (([regex]::Matches($afterText, 'RegisterHotKey\(hwnd_, kPauseHotkeyId, MOD_NOREPEAT, VK_F4\)')).Count -ne 1) {
    throw 'PROTECTED F4 RegisterHotKey line changed/missing'
}

Write-Host "REHYDRATE v0.2.7-R3 PASS baseR2=$r2Hash final=$finalHash F4=UNCHANGED"
