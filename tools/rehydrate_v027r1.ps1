$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$generated = Join-Path $root 'generated\donor159'
$temp = Join-Path $root 'generated\_rehydrate_r1'
$controller = Join-Path $generated 'src\controller.cpp'

# R1 is intentionally based on the exact v0.2.7 rehydrate chain. Do not bypass or copy later-version source.
& (Join-Path $PSScriptRoot 'rehydrate_v159.ps1')

function Join-BinaryParts([string]$dir, [string]$pattern, [string]$outPath) {
    $parts = Get-ChildItem -LiteralPath $dir -Filter $pattern | Sort-Object Name
    if ($parts.Count -eq 0) { throw "No parts found: $dir / $pattern" }
    $out = [IO.File]::Create($outPath)
    try {
        foreach ($p in $parts) {
            $input = [IO.File]::OpenRead($p.FullName)
            try { $input.CopyTo($out) } finally { $input.Dispose() }
        }
    } finally { $out.Dispose() }
}

function Normalize-Lf([string]$path) {
    $text = [IO.File]::ReadAllText($path)
    $text = $text.Replace("`r`n", "`n")
    [IO.File]::WriteAllText($path, $text, [Text.UTF8Encoding]::new($false))
}

$baseHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($baseHash -ne '397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f') {
    throw "v0.2.7 base controller SHA256 mismatch before R1: $baseHash"
}

if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $temp | Out-Null

$r1Archive = Join-Path $temp 'controller_v027r1_patch.tar.xz'
Join-BinaryParts (Join-Path $root 'vendor\v027r1_patch_parts') 'part.*' $r1Archive
$r1ArchiveHash = (Get-FileHash -Algorithm SHA256 $r1Archive).Hash.ToLowerInvariant()
if ($r1ArchiveHash -ne '302426edc5bb31e260516d8b7a7596edd2dcbcd05dbec79418753b1862f08d9e') {
    throw "v0.2.7-R1 patch archive SHA256 mismatch: $r1ArchiveHash"
}

$r1Dir = Join-Path $temp 'v027r1'
New-Item -ItemType Directory -Force $r1Dir | Out-Null
& tar.exe -xJf $r1Archive -C $r1Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.7-R1 patch tar.xz' }
$r1PatchFile = Join-Path $r1Dir 'controller_v027r1.patch'
$r1PatchHash = (Get-FileHash -Algorithm SHA256 $r1PatchFile).Hash.ToLowerInvariant()
if ($r1PatchHash -ne '5184f3c15d664143ce8a04c128cc1febfd443cdf13657ad6aea200e23e397520') {
    throw "v0.2.7-R1 controller patch SHA256 mismatch: $r1PatchHash"
}

Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $r1PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.7-R1 requested-only patch' }
} finally { Pop-Location }

Normalize-Lf $controller
$r1ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($r1ControllerHash -ne '575c289a7587d8de62d91124cf2d7601816d6bf1ff028e6941ecabaf3ae8d2d4') {
    throw "v0.2.7-R1 controller SHA256 mismatch: $r1ControllerHash"
}

Write-Host "REHYDRATE v0.2.7-R1 PASS base=$baseHash patch=$r1PatchHash archive=$r1ArchiveHash controller=$r1ControllerHash"
