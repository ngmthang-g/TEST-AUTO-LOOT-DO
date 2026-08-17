$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$vendor = Join-Path $root 'vendor\donor159'
$v022Vendor = Join-Path $root 'vendor\v022_patch_parts'
$v023Vendor = Join-Path $root 'vendor\v023_patch_parts'
$v024Vendor = Join-Path $root 'vendor\v024_patch_parts'
$v025Vendor = Join-Path $root 'vendor\v025_patch_parts'
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
if ($donorHash -ne '316bd386c8b69d82e24aac12411449c816819e3614758f2f44a41b04a4758861') { throw "Donor archive SHA256 mismatch: $donorHash" }
& tar.exe -xJf $donorArchive -C $generated
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract donor source tar.xz' }

$patchArchive = Join-Path $temp 'controller_patch.tar.xz'
Join-BinaryParts (Join-Path $vendor 'patch_parts') 'patch.part.*' $patchArchive
$patchHash = (Get-FileHash -Algorithm SHA256 $patchArchive).Hash.ToLowerInvariant()
if ($patchHash -ne '758acb948c07dadbb010d5608da8715c001438cf8d2adbc0905ec82491a26c1f') { throw "Patch archive SHA256 mismatch: $patchHash" }
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
    & git apply --whitespace=nowarn .\tools\controller_compile_fix.patch
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for MSVC compile fix' }
} finally { Pop-Location }

Normalize-Lf $controller
$v021ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v021ControllerHash -ne '66e2fe61418405b666f3afa4d0aebc5609fb482c692407642b8d5838bdc47162') { throw "v0.2.1 controller SHA256 mismatch: $v021ControllerHash" }

$v022Patch = Join-Path $temp 'controller_v022.patch'
Join-BinaryParts $v022Vendor 'part.*' $v022Patch
$v022PatchHash = (Get-FileHash -Algorithm SHA256 $v022Patch).Hash.ToLowerInvariant()
if ($v022PatchHash -ne '65bafda2c9980f67c1202be01ca1391bd68b73685cc451e8b8130c8c80ddb32b') { throw "v0.2.2 patch SHA256 mismatch: $v022PatchHash" }
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v022Patch
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.2 REAL INPUT coordinator patch' }
} finally { Pop-Location }
Normalize-Lf $controller
$v022ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v022ControllerHash -ne '732fcfdd6ab497b1f1da442ec94b63a5f63a2d5757a8fcb8b5c9ee9efc5a1066') { throw "v0.2.2 controller SHA256 mismatch: $v022ControllerHash" }

$v023Archive = Join-Path $temp 'controller_v023_patch.tar.xz'
Join-BinaryParts $v023Vendor 'part.*' $v023Archive
$v023PatchHash = (Get-FileHash -Algorithm SHA256 $v023Archive).Hash.ToLowerInvariant()
if ($v023PatchHash -ne '16c3b976d01f6cc7e0987a22b7a4bec5e35edb198b1f61c2fdd70d013715e7ee') { throw "v0.2.3 patch archive SHA256 mismatch: $v023PatchHash" }
$v023Dir = Join-Path $temp 'v023'
New-Item -ItemType Directory -Force $v023Dir | Out-Null
& tar.exe -xJf $v023Archive -C $v023Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.3 patch tar.xz' }
$v023PatchFile = Join-Path $v023Dir 'controller_v023.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v023PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.3 central arbiter patch' }
} finally { Pop-Location }
Push-Location $root
try {
    & git apply --whitespace=nowarn .\tools\controller_v023_compile_fix.patch
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.3 MSVC const fix' }
} finally { Pop-Location }
Normalize-Lf $controller
$v023ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v023ControllerHash -ne '4f7069a0ae47b417a2a4ccf8da4bfd3d4019ae216d88e01070e51c7e0e085fe4') { throw "v0.2.3 controller SHA256 mismatch: $v023ControllerHash" }

$v024Archive = Join-Path $temp 'controller_v024_patch.tar.xz'
Join-BinaryParts $v024Vendor 'part.*' $v024Archive
$v024PatchHash = (Get-FileHash -Algorithm SHA256 $v024Archive).Hash.ToLowerInvariant()
if ($v024PatchHash -ne '8152b1e579e89da2362eb4dd72170f951e151f5da67035d3c8c9af9524a12cb3') { throw "v0.2.4 patch archive SHA256 mismatch: $v024PatchHash" }
$v024Dir = Join-Path $temp 'v024'
New-Item -ItemType Directory -Force $v024Dir | Out-Null
& tar.exe -xJf $v024Archive -C $v024Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.4 patch tar.xz' }
$v024PatchFile = Join-Path $v024Dir 'controller_v024.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v024PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.4 recorder/copy patch' }
} finally { Pop-Location }
Normalize-Lf $controller
$v024ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v024ControllerHash -ne 'f45b8969488ac5a59e773845a60f180499700f9ebd59fff905d9a4abe5d57959') { throw "v0.2.4 controller SHA256 mismatch: $v024ControllerHash" }


$v025Archive = Join-Path $temp 'controller_v025_patch.tar.xz'
Join-BinaryParts $v025Vendor 'part.*' $v025Archive
$v025PatchHash = (Get-FileHash -Algorithm SHA256 $v025Archive).Hash.ToLowerInvariant()
if ($v025PatchHash -ne '6d8b5cbed8ec129312a4a6c895af7ccc6c7fd7ce7d3e3ab979ca49a78057a5b5') { throw "v0.2.5 patch archive SHA256 mismatch: $v025PatchHash" }
$v025Dir = Join-Path $temp 'v025'
New-Item -ItemType Directory -Force $v025Dir | Out-Null
& tar.exe -xJf $v025Archive -C $v025Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.5 patch tar.xz' }
$v025PatchFile = Join-Path $v025Dir 'controller_v025.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v025PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.5 sell-freeze/editor/REC patch' }
} finally { Pop-Location }
Normalize-Lf $controller
$v025ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v025ControllerHash -ne '1771cb5ca6b4d1421c18cdb9d666c2f2bb1ac31db87c9671b691dd5f17f2daf1') { throw "v0.2.5 controller SHA256 mismatch: $v025ControllerHash" }


$v026Archive = Join-Path $temp 'controller_v026_patch.tar.xz'
Join-BinaryParts (Join-Path $root 'vendor\v026_patch_parts') 'part.*' $v026Archive
$v026PatchHash = (Get-FileHash -Algorithm SHA256 $v026Archive).Hash.ToLowerInvariant()
if ($v026PatchHash -ne '27b3fa34800cd89fbd32b0b5d6976d6ec1acda7cd9e863c20fecb9f114239024') { throw "v0.2.6 patch archive SHA256 mismatch: $v026PatchHash" }
$v026Dir = Join-Path $temp 'v026'
New-Item -ItemType Directory -Force $v026Dir | Out-Null
& tar.exe -xJf $v026Archive -C $v026Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.6 patch tar.xz' }
$v026PatchFile = Join-Path $v026Dir 'controller_v026.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v026PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.6 consolidation-toggle/sell-copy patch' }
} finally { Pop-Location }
Normalize-Lf $controller
$v026ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v026ControllerHash -ne '266d86aeaca97a0a6f63ce33ed2f543de1fd531a2be4d6d2eb9b7c940c320f5d') { throw "v0.2.6 controller SHA256 mismatch: $v026ControllerHash" }

$v027Archive = Join-Path $temp 'controller_v027_patch.tar.xz'
Join-BinaryParts (Join-Path $root 'vendor\v027_patch_parts') 'part.*' $v027Archive
$v027PatchHash = (Get-FileHash -Algorithm SHA256 $v027Archive).Hash.ToLowerInvariant()
if ($v027PatchHash -ne '9a4e5737678bd9c951f318732fbafb3f2f08f7b07e9b8812424836029f7c37bb') { throw "v0.2.7 patch archive SHA256 mismatch: $v027PatchHash" }
$v027Dir = Join-Path $temp 'v027'
New-Item -ItemType Directory -Force $v027Dir | Out-Null
& tar.exe -xJf $v027Archive -C $v027Dir
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract v0.2.7 patch tar.xz' }
$v027PatchFile = Join-Path $v027Dir 'controller_v027.patch'
Push-Location $root
try {
    & git apply --whitespace=nowarn --directory=generated/donor159 $v027PatchFile
    if ($LASTEXITCODE -ne 0) { throw 'git apply failed for v0.2.7 shared child trade workflow patch' }
} finally { Pop-Location }
Normalize-Lf $controller
$v027ControllerHash = (Get-FileHash -Algorithm SHA256 $controller).Hash.ToLowerInvariant()
if ($v027ControllerHash -ne '397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f') { throw "v0.2.7 controller SHA256 mismatch: $v027ControllerHash" }

Write-Host "REHYDRATE PASS donor=$donorHash v021=$v021ControllerHash v022=$v022ControllerHash v023=$v023ControllerHash v024=$v024ControllerHash v025=$v025ControllerHash v026=$v026ControllerHash v027patch=$v027PatchHash controller=$v027ControllerHash"
