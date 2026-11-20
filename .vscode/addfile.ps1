param(
    [Parameter(Mandatory=$true)][string]$Path,
    [string]$Group = ''
)

$root = Split-Path -Parent $PSScriptRoot
if (-not (Test-Path $Path)) { Write-Host "File not found: $Path" -ForegroundColor Red; exit 1 }

$full = (Resolve-Path $Path).Path
if (-not $full.StartsWith($root, [StringComparison]::OrdinalIgnoreCase)) {
    Write-Host "File must be inside the project folder: $root" -ForegroundColor Red; exit 1
}
$rel  = '.\' + $full.Substring($root.Length).TrimStart('\')
$sub  = Split-Path (Split-Path $rel -Parent) -Leaf

$projFile = Get-ChildItem -Path $root -Filter *.uvprojx -File -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $projFile) { Write-Host "No .uvprojx found in $root" -ForegroundColor Red; exit 1 }

$text = Get-Content $projFile.FullName -Raw -Encoding UTF8

if ($text.Contains("<FilePath>$rel</FilePath>")) {
    Write-Host "[SKIP] already in project: $rel" -ForegroundColor Yellow
    exit 0
}

# pick group: explicit -> folder name that already exists as a group -> User
if ([string]::IsNullOrWhiteSpace($Group)) {
    if ($sub -and $text -match "<GroupName>$([regex]::Escape($sub))</GroupName>") { $Group = $sub }
    else { $Group = 'User' }
}

$gPattern = '(?s)(<GroupName>' + [regex]::Escape($Group) + '</GroupName>.*?<Files>)(.*?)(</Files>)'
$m = [regex]::Match($text, $gPattern)
if (-not $m.Success) { Write-Host "Group '$Group' not found in project" -ForegroundColor Red; exit 1 }

$fileName = Split-Path $rel -Leaf
$fileType = 1
if ($rel -match '\.(s|S)$') { $fileType = 2 }
elseif ($rel -match '\.(h|H)$') { $fileType = 5 }

$entry = "`r`n            <File>`r`n              <FileName>$fileName</FileName>`r`n              <FileType>$fileType</FileType>`r`n              <FilePath>$rel</FilePath>`r`n            </File>"
$newFiles = $m.Groups[2].Value + $entry
$newText  = $text.Remove($m.Groups[2].Index, $m.Groups[2].Length).Insert($m.Groups[2].Index, $newFiles)

Copy-Item $projFile.FullName "$($projFile.FullName).bak-addfile" -Force
[System.IO.File]::WriteAllText($projFile.FullName, $newText, (New-Object System.Text.UTF8Encoding($false)))

try {
    $null = [xml](Get-Content $projFile.FullName -Raw -Encoding UTF8)
    Write-Host "[OK] $rel  ->  group '$Group'" -ForegroundColor Green
}
catch {
    Copy-Item "$($projFile.FullName).bak-addfile" $projFile.FullName -Force
    Write-Host "[FAIL] XML invalid, restored" -ForegroundColor Red
    exit 1
}