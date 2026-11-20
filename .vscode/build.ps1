param(
    [switch]$Rebuild,
    [switch]$Flash,
    [switch]$Clean
)

$root = Split-Path -Parent $PSScriptRoot
$projFile = Get-ChildItem -Path $root -Filter *.uvprojx -File -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $projFile) { Write-Host "No .uvprojx found in $root" -ForegroundColor Red; exit 1 }
$proj = $projFile.FullName
$uv4  = 'D:\Keil_v5\UV4\UV4.exe'
$buildLog = Join-Path $root 'Objects\vscode_build.log'
$flashLog = Join-Path $root 'Objects\flash_dl.log'

function Sync-ProjectFiles {
    param([string]$ProjPath, [string]$Root)

    $text = Get-Content $ProjPath -Raw -Encoding UTF8
    $orig = $text
    $added = @()
    $targets = @()

    foreach ($dirName in @('Hardware', 'System', 'user')) {
        $dir = Join-Path $Root $dirName
        if (-not (Test-Path $dir)) { continue }
        $group = 'User'
        if ($text -match "<GroupName>$([regex]::Escape($dirName))</GroupName>") { $group = $dirName }
        foreach ($f in (Get-ChildItem $dir -Filter *.c -File -ErrorAction SilentlyContinue)) {
            $targets += ,@("$dirName\$($f.Name)", $f.Name, $group)
        }
    }
    foreach ($f in (Get-ChildItem $Root -Filter *.c -File -ErrorAction SilentlyContinue)) {
        $targets += ,@($f.Name, $f.Name, 'User')
    }

    foreach ($t in $targets) {
        $rel  = '.\' + $t[0]
        $name = $t[1]
        $grp  = $t[2]
        if ($text.Contains("<FilePath>$rel</FilePath>")) { continue }

        $m = [regex]::Match($text, '(?s)(<GroupName>' + [regex]::Escape($grp) + '</GroupName>.*?<Files>)(.*?)(</Files>)')
        if (-not $m.Success) { continue }

        $entry = "`r`n            <File>`r`n              <FileName>$name</FileName>`r`n              <FileType>1</FileType>`r`n              <FilePath>$rel</FilePath>`r`n            </File>"
        $text = $text.Remove($m.Groups[2].Index, $m.Groups[2].Length).Insert($m.Groups[2].Index, ($m.Groups[2].Value + $entry))
        $added += $rel
    }

    if ($text -ne $orig) {
        Copy-Item $ProjPath "$ProjPath.bak-autosync" -Force
        [System.IO.File]::WriteAllText($ProjPath, $text, (New-Object System.Text.UTF8Encoding($false)))
        try {
            $null = [xml](Get-Content $ProjPath -Raw -Encoding UTF8)
            Write-Host "[AUTO] added to Keil project: $($added -join ', ')" -ForegroundColor Cyan
        } catch {
            Copy-Item "$ProjPath.bak-autosync" $ProjPath -Force
            Write-Host "[AUTO] failed, project restored" -ForegroundColor Red
        }
    }
}

function Invoke-Uv4 {
    param([string]$Mode, [string]$LogFile)
    if (Test-Path $LogFile) { Remove-Item $LogFile -Force -ErrorAction SilentlyContinue }
    $p = Start-Process -FilePath $uv4 -ArgumentList @($Mode, "`"$proj`"", '-j0', '-o', "`"$LogFile`"") -Wait -PassThru
    if (Test-Path $LogFile) { Get-Content $LogFile -Encoding UTF8 | ForEach-Object { Write-Host $_ } }
    return $p
}

function Show-Diag {
    param([string]$LogFile)
    if (-not (Test-Path $LogFile)) { return }
    $diag = Get-Content $LogFile -Encoding UTF8 | Where-Object { $_ -match '\(\d+\)\s*:\s*(error|fatal error)' }
    if ($diag) {
        Write-Host ""
        Write-Host "======== ERRORS ========" -ForegroundColor Red
        $diag | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        Write-Host "========================" -ForegroundColor Red
    }
}

function Get-Counts {
    param([string]$LogFile)
    $e = 0; $w = 0
    if (Test-Path $LogFile) {
        $m = Select-String -Path $LogFile -Pattern '(\d+)\s+Error\(s\),\s+(\d+)\s+Warning\(s\)' -ErrorAction SilentlyContinue
        if ($m) { $e = [int]$m.Matches[0].Groups[1].Value; $w = [int]$m.Matches[0].Groups[2].Value }
    }
    return @($e, $w)
}

if (-not $Clean) { Sync-ProjectFiles -ProjPath $proj -Root $root }

if ($Flash) {
    Write-Host "=== step 1/2 : build ===" -ForegroundColor Cyan
    $pb = Invoke-Uv4 '-b' $buildLog
    $c = Get-Counts $buildLog
    Show-Diag $buildLog
    if ($c[0] -gt 0) {
        Write-Host ""
        Write-Host "[STOP] $($c[0]) error(s). Nothing was flashed." -ForegroundColor Red
        exit 2
    }

    Write-Host ""
    Write-Host "=== step 2/2 : download to board ===" -ForegroundColor Cyan

    $stale = Get-Process -Name openocd, gdb-multiarch -ErrorAction SilentlyContinue
    if ($stale) {
        Write-Host "[INFO] freeing ST-Link (stale debug processes: $(($stale | ForEach-Object { $_.ProcessName }) -join ', '))" -ForegroundColor Yellow
        $stale | Stop-Process -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 1
    }

    # ---- 用 OpenOCD 烧录 ----
    # Keil 的 ST-Link DLL 有时认不到设备 (No ST-LINK detected)，
    # 但 OpenOCD 用独立实现，通常照样能烧。所以这里不再用 UV4 -f。
    $ocdExe     = 'D:\msys64\ucrt64\bin\openocd.exe'
    $ocdScripts = 'D:\msys64\ucrt64\share\openocd\scripts'

    $axf = Get-ChildItem (Join-Path $root 'Objects') -Filter *.axf -ErrorAction SilentlyContinue |
           Sort-Object LastWriteTime -Descending | Select-Object -First 1

    if (-not (Test-Path $ocdExe)) {
        Write-Host "[FAIL] openocd not found: $ocdExe" -ForegroundColor Red
        exit 3
    }
    if (-not $axf) {
        Write-Host "[FAIL] no .axf found (build failed?)" -ForegroundColor Red
        exit 3
    }

    $axfPath = $axf.FullName -replace '\\', '/'
    Write-Host "firmware: $($axf.Name)  $([math]::Round($axf.Length/1KB,1)) KB" -ForegroundColor DarkGray

    $ocdOut = & $ocdExe -s $ocdScripts -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program {$axfPath} verify reset exit" 2>&1

    if ($ocdOut -match 'Verified OK') {
        $ocdOut | Where-Object { $_ -match 'Programming|Verified|Resetting|Error' } |
            ForEach-Object { Write-Host "  $_" -ForegroundColor DarkGray }
        Write-Host ""
        Write-Host "[OK] flashed. Press RESET on the board if it does not start." -ForegroundColor Green
        exit 0
    } else {
        Write-Host ""
        $ocdOut | Select-Object -Last 10 | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
        Write-Host ""
        Write-Host "[FAIL] flash failed (see messages above)" -ForegroundColor Red
        exit 1
    }
}

if ($Rebuild) { $mode = '-r'; $what = 'rebuild' }
elseif ($Clean) { $mode = '-c'; $what = 'clean' }
else { $mode = '-b'; $what = 'build' }

Write-Host "=== Keil $what ===" -ForegroundColor Cyan
$p = Invoke-Uv4 $mode $buildLog
Show-Diag $buildLog
$c = Get-Counts $buildLog

Write-Host ""
if ($c[0] -eq 0) {
    if ($c[1] -eq 0) { Write-Host "[OK] Build succeeded (0 errors, 0 warnings)" -ForegroundColor Green }
    else { Write-Host "[OK] Build succeeded ($($c[1]) warning(s), 0 errors)" -ForegroundColor Yellow }
    exit 0
} else {
    Write-Host "[FAIL] $($c[0]) error(s)" -ForegroundColor Red
    exit 2
}