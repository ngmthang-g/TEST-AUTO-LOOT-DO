$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

# Rebuild the exact validated v0.2.7 source first.
& (Join-Path $PSScriptRoot 'rehydrate_v159.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Base rehydrate through v0.2.7 failed' }

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

$generated = Join-Path $root 'generated\donor159'
$temp = Join-Path $root 'generated\_rehydrate\v028'
if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $temp | Out-Null

$archive = Join-Path $temp 'controller_v028_patch.tar.xz'
Join-BinaryParts (Join-Path $root 'vendor\v028_patch_parts') 'part.*' $archive
$patchHash = (Get-FileHash -Algorithm SHA256 $archive).Hash.ToLowerInvariant()
if ($patchHash -ne '209c7a5a4998055e04d3e9ad33da03016e5e815345802cd7d06dbf54b9ac9b01') {
    throw "v0.2.8 patch archive SHA256 mismatch: $patchHash"
}

$patchDir = Join-Path $temp 'patch'
New-Item -ItemType Directory -Force $patchDir | Out-Null
& tar.exe -xJf $archive -C $patchDir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.8 patch tar.xz' }

$controller = Join-Path $generated 'src\controller.cpp'
Normalize-Lf $controller
$patchFile = Join-Path $patchDir 'controller_v028.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $patchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.8 rendezvous/group/drain patch' }
} finally { Pop-Location }
Normalize-Lf $controller

$controllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($controllerHash -ne '5e869b54dcd3df12557887b1f14f4b79971a406852d6061490d3860185516a1f') {
    throw "v0.2.8 controller SHA256 mismatch: $controllerHash"
}
Write-Host "REHYDRATE v0.2.8 PASS patch=$patchHash controller=$controllerHash"
