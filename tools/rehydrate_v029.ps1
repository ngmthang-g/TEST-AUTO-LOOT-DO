$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

# Rebuild the exact validated v0.2.8 source first.
& (Join-Path $PSScriptRoot 'rehydrate_v028.ps1')
if ($LASTEXITCODE -ne 0) { throw 'Base rehydrate through v0.2.8 failed' }

function Join-BinaryParts([string]$dir, [string]$pattern, [string]$outPath) {
    $parts = Get-ChildItem -LiteralPath $dir -Filter $pattern | Sort-Object Name
    if ($parts.Count -eq 0) { throw "No parts found: $dir / $pattern" }
    $out = [IO.File]::Create($outPath)
    try { foreach ($p in $parts) { $input = [IO.File]::OpenRead($p.FullName); try { $input.CopyTo($out) } finally { $input.Dispose() } } }
    finally { $out.Dispose() }
}

function Normalize-Lf([string]$path) {
    $text = [IO.File]::ReadAllText($path).Replace("`r`n", "`n")
    [IO.File]::WriteAllText($path, $text, [Text.UTF8Encoding]::new($false))
}

$generated = Join-Path $root 'generated\donor159'
$temp = Join-Path $root 'generated\_rehydrate\v029'
if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $temp | Out-Null

$archive = Join-Path $temp 'controller_v029_patch.tar.xz'
Join-BinaryParts (Join-Path $root 'vendor\v029_patch_parts') 'part.*' $archive
$patchHash = (Get-FileHash -Algorithm SHA256 $archive).Hash.ToLowerInvariant()
if ($patchHash -ne '64ace31be608efb44f8407c8e03855f73c2580ba05b07c877436e00a2d747b68') { throw "v0.2.9 patch archive SHA256 mismatch: $patchHash" }

$patchDir = Join-Path $temp 'patch'
New-Item -ItemType Directory -Force $patchDir | Out-Null
& tar.exe -xJf $archive -C $patchDir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.9 patch tar.xz' }

$controller = Join-Path $generated 'src\controller.cpp'
Normalize-Lf $controller
$patchFile = Join-Path $patchDir 'controller_v029.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $patchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.9 party/rendezvous-lock/F4 patch' }
} finally { Pop-Location }
Normalize-Lf $controller

$controllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($controllerHash -ne '5b8b4d02d7f9f12bcd49541bb32d177d3a3b01de6ed674dd66f724a5734daacc') { throw "v0.2.9 controller SHA256 mismatch: $controllerHash" }
Write-Host "REHYDRATE v0.2.9 PASS patch=$patchHash controller=$controllerHash"
