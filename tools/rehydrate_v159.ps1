$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$vendor = Join-Path $root 'vendor\donor159'
$generated = Join-Path $root 'generated\donor159'
$temp = Join-Path $root 'generated\_rehydrate'

if (Test-Path $generated) { Remove-Item $generated -Recurse -Force }
if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $generated | Out-Null
New-Item -ItemType Directory -Force $temp | Out-Null

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

$donorArchive = Join-Path $temp 'donor159_source.tar.xz'
Join-BinaryParts (Join-Path $vendor 'source_parts') 'donor.part.*' $donorArchive
$donorHash = (Get-FileHash -Algorithm SHA256 $donorArchive).Hash.ToLowerInvariant()
if ($donorHash -ne '316bd386c8b69d82e24aac12411449c816819e3614758f2f44a41b04a4758861') {
    throw "Donor archive SHA256 mismatch: $donorHash"
}
& tar.exe -xJf $donorArchive -C $generated
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract donor source tar.xz' }

$patchArchive = Join-Path $temp 'controller_patch.tar.xz'
Join-BinaryParts (Join-Path $vendor 'patch_parts') 'patch.part.*' $patchArchive
$patchHash = (Get-FileHash -Algorithm SHA256 $patchArchive).Hash.ToLowerInvariant()
if ($patchHash -ne '758acb948c07dadbb010d5608da8715c001438cf8d2adbc0905ec82491a26c1f') {
    throw "Patch archive SHA256 mismatch: $patchHash"
}
$patchDir = Join-Path $temp 'patch'
New-Item -ItemType Directory -Force $patchDir | Out-Null
& tar.exe -xJf $patchArchive -C $patchDir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract controller patch tar.xz' }

$controller = Join-Path $generated 'src\controller.cpp'
Normalize-Lf $controller
$patchFile = Join-Path $patchDir 'controller.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $patchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.1 controller patch' }
} finally { Pop-Location }

# Git for Windows may apply checkout attributes/autocrlf while writing a patched file.
# Normalize once more so verification is byte-stable across Windows/Linux.
Normalize-Lf $controller
$controllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($controllerHash -ne 'ddc2d55043fddd0525f2087d85afae9245364bded5f87cd06987dfed05515583') {
    throw "Patched controller SHA256 mismatch: $controllerHash"
}

Write-Host "REHYDRATE PASS donor=$donorHash patch=$patchHash controller=$controllerHash"
