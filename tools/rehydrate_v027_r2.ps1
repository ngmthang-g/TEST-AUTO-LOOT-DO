$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$controller = Join-Path $root 'generated\donor159\src\controller.cpp'
$partsDir = Join-Path $root 'vendor\v027_r2_patch_parts'
$temp = Join-Path $root 'generated\_v027_r2_patch'
$archive = Join-Path $temp 'controller_v027_r2_patch.tar.xz'

& (Join-Path $PSScriptRoot 'rehydrate_v159.ps1')

$baseHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($baseHash -ne '397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f') {
    throw "v0.2.7 clean base SHA mismatch: $baseHash"
}

# F4 is protected by user runtime feedback. Save the exact source block before applying R2.
$baseText = [IO.File]::ReadAllText($controller)
$f4Pattern = '(?ms)^    void ToggleGlobalPause\(\) \{.*?^    \}'
$baseF4 = [regex]::Match($baseText, $f4Pattern).Value
if ([string]::IsNullOrWhiteSpace($baseF4)) { throw 'Cannot locate protected ToggleGlobalPause block in v0.2.7 base' }

if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $temp | Out-Null

$parts = Get-ChildItem -LiteralPath $partsDir -Filter 'part.*' | Sort-Object Name
if ($parts.Count -eq 0) { throw 'No v0.2.7-R2 patch parts found' }
$out = [IO.File]::Create($archive)
try {
    foreach ($p in $parts) {
        $input = [IO.File]::OpenRead($p.FullName)
        try { $input.CopyTo($out) } finally { $input.Dispose() }
    }
} finally { $out.Dispose() }

$archiveHash = (Get-FileHash -Algorithm SHA256 $archive).Hash.ToLowerInvariant()
if ($archiveHash -ne '548cfaee5ca5e1426cbbe2517bd2faac9e05b0cc8a5852bc2d6ed3a2a3425952') {
    throw "v0.2.7-R2 patch archive SHA mismatch: $archiveHash"
}

& tar.exe -xJf $archive -C $temp
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.7-R2 patch archive' }
$patch = Join-Path $temp 'controller_v027_r2.patch'
$patchHash = (Get-FileHash -Algorithm SHA256 $patch).Hash.ToLowerInvariant()
if ($patchHash -ne '6854233f99cf2b54bb7c1d235ec3dfc089a95752799ec4b29ba12cc83d0309b7') { throw "v0.2.7-R2 patch SHA mismatch: $patchHash" }

Push-Location $root
try {
    & git apply --whitespace=nowarn $patch
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.7-R2 patch' }
} finally {
    Pop-Location
}

# git apply on Windows may honor checkout EOL settings; canonical source lineage is LF.
$normalized = [IO.File]::ReadAllText($controller).Replace("`r`n", "`n")
[IO.File]::WriteAllText($controller, $normalized, [Text.UTF8Encoding]::new($false))

$finalHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($finalHash -ne '2124f79119754abfa95f320481b878239ae38c12810c7c85dbe963c44c41f09b') {
    throw "v0.2.7-R2 final controller SHA mismatch: $finalHash"
}

$finalText = [IO.File]::ReadAllText($controller)
$finalF4 = [regex]::Match($finalText, $f4Pattern).Value
if ($finalF4 -cne $baseF4) { throw 'PROTECTED F4 ToggleGlobalPause block changed from clean v0.2.7' }
if (([regex]::Matches($finalText, 'RegisterHotKey\(hwnd_, kPauseHotkeyId, MOD_NOREPEAT, VK_F4\)')).Count -ne 1) {
    throw 'PROTECTED F4 RegisterHotKey line changed/missing'
}

Write-Host "REHYDRATE v0.2.7-R2 PASS base=$baseHash final=$finalHash F4=BYTE-IDENTICAL"
