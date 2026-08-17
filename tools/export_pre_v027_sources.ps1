$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$generated = Join-Path $root 'generated\donor159'
$controller = Join-Path $generated 'src\controller.cpp'
$outRoot = Join-Path $root 'export\pre_v027_sources'
$temp = Join-Path $root 'generated\_reverse_lineage'

if (Test-Path $outRoot) { Remove-Item $outRoot -Recurse -Force }
if (Test-Path $temp) { Remove-Item $temp -Recurse -Force }
New-Item -ItemType Directory -Force $outRoot | Out-Null
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

function Assert-Controller([string]$version, [string]$expectedHash) {
    Normalize-Lf $controller
    $actual = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
    if ($actual -ne $expectedHash) { throw "$version controller SHA256 mismatch: $actual" }
    $dir = Join-Path $outRoot $version
    New-Item -ItemType Directory -Force $dir | Out-Null
    Copy-Item $controller (Join-Path $dir 'controller.cpp') -Force
    [IO.File]::WriteAllText((Join-Path $dir 'SHA256.txt'), "$actual  controller.cpp`n", [Text.UTF8Encoding]::new($false))
    Write-Host "$version SOURCE SNAPSHOT PASS $actual"
}

function Reverse-ArchivePatch([string]$label, [string]$partsDir, [string]$archiveHash, [string]$patchName) {
    $archive = Join-Path $temp "$label.tar.xz"
    Join-BinaryParts $partsDir 'part.*' $archive
    $actualArchive = (Get-FileHash -Algorithm SHA256 $archive).Hash.ToLowerInvariant()
    if ($actualArchive -ne $archiveHash) { throw "$label archive SHA mismatch: $actualArchive" }
    $dir = Join-Path $temp $label
    New-Item -ItemType Directory -Force $dir | Out-Null
    & tar.exe -xJf $archive -C $dir
    if ($LASTEXITCODE -ne 0) { throw "Failed to extract $label" }
    $patch = Join-Path $dir $patchName
    Push-Location $root
    try {
        & git apply -R --whitespace=nowarn --directory=generated/donor159 $patch
        if ($LASTEXITCODE -ne 0) { throw "git apply -R failed for $label" }
    } finally { Pop-Location }
    Normalize-Lf $controller
}

& (Join-Path $PSScriptRoot 'rehydrate_v159.ps1')
Assert-Controller 'v0.2.7' '397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f'
Reverse-ArchivePatch 'v027' (Join-Path $root 'vendor\v027_patch_parts') '9a4e5737678bd9c951f318732fbafb3f2f08f7b07e9b8812424836029f7c37bb' 'controller_v027.patch'
Assert-Controller 'v0.2.6' '266d86aeaca97a0a6f63ce33ed2f543de1fd531a2be4d6d2eb9b7c940c320f5d'
Reverse-ArchivePatch 'v026' (Join-Path $root 'vendor\v026_patch_parts') '27b3fa34800cd89fbd32b0b5d6976d6ec1acda7cd9e863c20fecb9f114239024' 'controller_v026.patch'
Assert-Controller 'v0.2.5' '1771cb5ca6b4d1421c18cdb9d666c2f2bb1ac31db87c9671b691dd5f17f2daf1'
Reverse-ArchivePatch 'v025' (Join-Path $root 'vendor\v025_patch_parts') '6d8b5cbed8ec129312a4a6c895af7ccc6c7fd7ce7d3e3ab979ca49a78057a5b5' 'controller_v025.patch'
Assert-Controller 'v0.2.4' 'f45b8969488ac5a59e773845a60f180499700f9ebd59fff905d9a4abe5d57959'
Reverse-ArchivePatch 'v024' (Join-Path $root 'vendor\v024_patch_parts') '8152b1e579e89da2362eb4dd72170f951e151f5da67035d3c8c9af9524a12cb3' 'controller_v024.patch'
Assert-Controller 'v0.2.3' '4f7069a0ae47b417a2a4ccf8da4bfd3d4019ae216d88e01070e51c7e0e085fe4'
$text023 = [IO.File]::ReadAllText($controller)
$from023 = 'std::wstring TradeStepTargetLabel(const TradeSequenceStep& step) {'
$to023 = 'std::wstring TradeStepTargetLabel(const TradeSequenceStep& step) const {'
if (-not $text023.Contains($from023)) { throw 'v0.2.3 compile-fix signature not found for reverse' }
$text023 = $text023.Replace($from023, $to023)
[IO.File]::WriteAllText($controller, $text023, [Text.UTF8Encoding]::new($false))
Normalize-Lf $controller
Reverse-ArchivePatch 'v023' (Join-Path $root 'vendor\v023_patch_parts') '16c3b976d01f6cc7e0987a22b7a4bec5e35edb198b1f61c2fdd70d013715e7ee' 'controller_v023.patch'
Assert-Controller 'v0.2.2' '732fcfdd6ab497b1f1da442ec94b63a5f63a2d5757a8fcb8b5c9ee9efc5a1066'
$v022Patch = Join-Path $temp 'controller_v022.patch'
Join-BinaryParts (Join-Path $root 'vendor\v022_patch_parts') 'part.*' $v022Patch
$v022PatchHash = (Get-FileHash -Algorithm SHA256 $v022Patch).Hash.ToLowerInvariant()
if ($v022PatchHash -ne '65bafda2c9980f67c1202be01ca1391bd68b73685cc451e8b8130c8c80ddb32b') { throw "v022 patch SHA mismatch: $v022PatchHash" }
Push-Location $root
try {
    & git apply -R --whitespace=nowarn --directory=generated/donor159 $v022Patch
    if ($LASTEXITCODE -ne 0) { throw 'git apply -R failed for v0.2.2 patch' }
} finally { Pop-Location }
Normalize-Lf $controller
Assert-Controller 'v0.2.1' '66e2fe61418405b666f3afa4d0aebc5609fb482c692407642b8d5838bdc47162'
$manifest = @"
SOURCE LINEAGE EXPORT — PRE-v0.2.7
Generated from exact v0.2.7 head 1308b28bd38fb044b9fceed3671820e45fb2cd23 by reversing the repository's checksum-verified version patches.
v0.2.6  266d86aeaca97a0a6f63ce33ed2f543de1fd531a2be4d6d2eb9b7c940c320f5d
v0.2.5  1771cb5ca6b4d1421c18cdb9d666c2f2bb1ac31db87c9671b691dd5f17f2daf1
v0.2.4  f45b8969488ac5a59e773845a60f180499700f9ebd59fff905d9a4abe5d57959
v0.2.3  4f7069a0ae47b417a2a4ccf8da4bfd3d4019ae216d88e01070e51c7e0e085fe4
v0.2.2  732fcfdd6ab497b1f1da442ec94b63a5f63a2d5757a8fcb8b5c9ee9efc5a1066
v0.2.1  66e2fe61418405b666f3afa4d0aebc5609fb482c692407642b8d5838bdc47162
"@
[IO.File]::WriteAllText((Join-Path $outRoot 'LINEAGE_MANIFEST.txt'), $manifest, [Text.UTF8Encoding]::new($false))
Write-Host 'PRE-v0.2.7 SOURCE LINEAGE EXPORT PASS'
