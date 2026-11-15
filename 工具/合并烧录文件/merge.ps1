# 把 Boot 和 App 的 Intel HEX / BIN 按地址合成一份可烧录的 HEX。
$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = (Resolve-Path (Join-Path $Here "..\..")).Path
$Drop = Join-Path $Here "放入"
$OutDir = Join-Path $Here "输出"
$FlashBase = [uint32]0x08000000

function Convert-ToFlashAddress([uint32]$Address) {
    if ($Address -lt [uint32]0x10000) {
        return [uint32]($FlashBase + $Address)
    }
    return $Address
}

function Format-Address([uint32]$Address) {
    return ("0x{0:X8}" -f $Address)
}

function Get-RecordChecksum([byte[]]$Bytes) {
    $sum = 0
    foreach ($b in $Bytes) {
        $sum = ($sum + $b) -band 255
    }
    return ((-$sum) -band 255)
}

function Read-IntelHex([string]$Path) {
    $upper = [uint32]0
    $map = @{}
    $lineNo = 0
    foreach ($raw in [System.IO.File]::ReadAllLines($Path)) {
        $lineNo++
        $line = $raw.Trim()
        if ($line.Length -eq 0) { continue }
        if ($line[0] -ne ":") {
            throw "$Path 第 $lineNo 行不是 HEX 记录"
        }
        $hex = $line.Substring(1)
        if (($hex.Length -lt 10) -or (($hex.Length % 2) -ne 0)) {
            throw "$Path 第 $lineNo 行长度不对"
        }
        $bytes = New-Object byte[] ($hex.Length / 2)
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            $bytes[$i] = [Convert]::ToByte($hex.Substring($i * 2, 2), 16)
        }
        $expect = Get-RecordChecksum $bytes[0..($bytes.Length - 2)]
        if ($expect -ne $bytes[$bytes.Length - 1]) {
            throw "$Path 第 $lineNo 行校验和错误"
        }
        $count = [int]$bytes[0]
        $offset = [uint32](([uint32]$bytes[1] -shl 8) -bor [uint32]$bytes[2])
        $type = [int]$bytes[3]
        if ($type -eq 0) {
            $base = [uint32]($upper + $offset)
            for ($i = 0; $i -lt $count; $i++) {
                $addr = Convert-ToFlashAddress ([uint32]($base + [uint32]$i))
                $map[$addr] = $bytes[4 + $i]
            }
        }
        elseif ($type -eq 1) {
            break
        }
        elseif ($type -eq 2) {
            $seg = [uint32](([uint32]$bytes[4] -shl 8) -bor [uint32]$bytes[5])
            $upper = [uint32]($seg -shl 4)
        }
        elseif ($type -eq 4) {
            $hi = [uint32](([uint32]$bytes[4] -shl 8) -bor [uint32]$bytes[5])
            $upper = [uint32]($hi -shl 16)
        }
    }
    return $map
}

function Read-BinFile([string]$Path, [uint32]$Start) {
    $data = [System.IO.File]::ReadAllBytes($Path)
    $map = @{}
    for ($i = 0; $i -lt $data.Length; $i++) {
        $addr = Convert-ToFlashAddress ([uint32]($Start + [uint32]$i))
        $map[$addr] = $data[$i]
    }
    return $map
}

function Get-BinStart([System.IO.FileInfo]$File) {
    if ($File.Name -match "(?i)0x([0-9A-Fa-f]{1,8})") {
        return [uint32]::Parse($Matches[1], [System.Globalization.NumberStyles]::AllowHexSpecifier)
    }
    $parent = $File.Directory.Name
    if ($parent -eq "boot") { return [uint32]0 }
    if ($parent -eq "app") { return [uint32]0x800 }
    if ($parent -eq "B") { return [uint32]0x8000 }
    throw "BIN 没有起始地址：$($File.FullName)。请把地址写进文件名，例如 app_0x00000800.bin，或放进 放入\boot、放入\app、放入\B。"
}

function Get-RangeText($Map) {
    $keys = @($Map.Keys | Sort-Object)
    if ($keys.Count -eq 0) { return "空文件" }
    $parts = New-Object System.Collections.Generic.List[string]
    $start = [uint32]$keys[0]
    $prev = $start
    for ($i = 1; $i -le $keys.Count; $i++) {
        $cut = $true
        if ($i -lt $keys.Count) {
            $cur = [uint32]$keys[$i]
            if ($cur -eq [uint32]($prev + [uint32]1)) {
                $prev = $cur
                $cut = $false
            }
        }
        if ($cut) {
            $len = [uint32]($prev - $start) + [uint32]1
            $parts.Add(("{0} .. {1}  {2} 字节" -f (Format-Address $start), (Format-Address $prev), $len))
            if ($i -lt $keys.Count) {
                $start = [uint32]$keys[$i]
                $prev = $start
            }
        }
    }
    return ($parts -join [Environment]::NewLine)
}

function Write-IntelHex([hashtable]$Map, [string]$Path) {
    $keys = @($Map.Keys | Sort-Object)
    $lines = New-Object System.Collections.Generic.List[string]
    $currentUpper = [uint32]::MaxValue
    $index = 0
    while ($index -lt $keys.Count) {
        $start = [uint32]$keys[$index]
        $upper = [uint32]($start -shr 16)
        if ($upper -ne $currentUpper) {
            $hi = [byte](($upper -shr 8) -band 255)
            $lo = [byte]($upper -band 255)
            $body = [byte[]](2, 0, 0, 4, $hi, $lo)
            $sum = Get-RecordChecksum $body
            $lines.Add((":02000004{0:X2}{1:X2}{2:X2}" -f $hi, $lo, $sum))
            $currentUpper = $upper
        }
        $chunk = New-Object System.Collections.Generic.List[byte]
        $addr = $start
        while (($chunk.Count -lt 16) -and ($index -lt $keys.Count)) {
            $at = [uint32]$keys[$index]
            if ($at -ne $addr) { break }
            if ([uint32]($addr -shr 16) -ne $currentUpper) { break }
            $chunk.Add([byte]$Map[$at])
            $index++
            $addr = [uint32]($addr + [uint32]1)
        }
        $low = [uint16]($start -band 0xFFFF)
        $bodyList = New-Object System.Collections.Generic.List[byte]
        $bodyList.Add([byte]$chunk.Count)
        $bodyList.Add([byte](($low -shr 8) -band 255))
        $bodyList.Add([byte]($low -band 255))
        $bodyList.Add([byte]0)
        $bodyList.AddRange($chunk)
        $bodyBytes = $bodyList.ToArray()
        $sum = Get-RecordChecksum $bodyBytes
        $hex = ($bodyBytes | ForEach-Object { "{0:X2}" -f $_ }) -join ""
        $lines.Add((":{0}{1:X2}" -f $hex, $sum))
    }
    $lines.Add(":00000001FF")
    $utf8 = New-Object System.Text.UTF8Encoding $false
    [System.IO.File]::WriteAllLines($Path, $lines.ToArray(), $utf8)
}

function Add-Image($Target, $Incoming, [System.Collections.Generic.List[uint32]]$Overlap) {
    foreach ($key in $Incoming.Keys) {
        $addr = [uint32]$key
        if ($Target.ContainsKey($addr)) {
            $Overlap.Add($addr)
        }
        else {
            $Target[$addr] = $Incoming[$addr]
        }
    }
}

function Get-SlotFiles([string]$Dir) {
    if (-not (Test-Path $Dir)) { return @() }
    return @(Get-ChildItem -Path $Dir -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Extension -match '(?i)^\.(hex|ihx|bin)$' } |
        Sort-Object Name)
}

function Get-NewestHex([string]$Dir) {
    if (-not (Test-Path $Dir)) { return $null }
    return @(Get-ChildItem -Path $Dir -Filter *.hex -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending) | Select-Object -First 1
}

Write-Host "合并烧录文件"
Write-Host ""

$bootFiles = @(Get-SlotFiles (Join-Path $Drop "boot"))
$appFiles = @(Get-SlotFiles (Join-Path $Drop "app"))
$bFiles = @(Get-SlotFiles (Join-Path $Drop "B"))
$loose = @(Get-ChildItem -Path $Drop -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -match '(?i)^\.(hex|ihx|bin)$' })

if ($bootFiles.Count -eq 0) {
    $autoBoot = Get-NewestHex (Join-Path $Root "boot引导软件\obj")
    if ($null -ne $autoBoot) { $bootFiles = @($autoBoot) }
}
if ($appFiles.Count -eq 0) {
    $autoApp = Get-NewestHex (Join-Path $Root "软件\obj")
    if ($null -ne $autoApp) { $appFiles = @($autoApp) }
}

$jobs = New-Object System.Collections.Generic.List[object]
foreach ($file in $bootFiles) { $jobs.Add($file) | Out-Null }
foreach ($file in $appFiles) { $jobs.Add($file) | Out-Null }
foreach ($file in $bFiles) { $jobs.Add($file) | Out-Null }
foreach ($file in $loose) { $jobs.Add($file) | Out-Null }

if ($bootFiles.Count -eq 0) {
    throw "没有找到 Boot。请先编译 boot引导软件，或把 hex 放进 放入\boot。"
}
if (($appFiles.Count + $bFiles.Count + $loose.Count) -eq 0) {
    throw "没有找到 App。请先编译 软件，或把 hex 放进 放入\app。"
}

$merged = @{}
$overlap = New-Object System.Collections.Generic.List[uint32]
$report = New-Object System.Collections.Generic.List[string]

foreach ($file in $jobs) {
    Write-Host ("  " + $file.FullName)
    if ($file.Extension -match '(?i)\.bin$') {
        $start = Get-BinStart $file
        $image = Read-BinFile $file.FullName $start
    }
    else {
        $image = Read-IntelHex $file.FullName
    }
    $range = Get-RangeText $image
    Write-Host $range
    Write-Host ""
    $report.Add($file.FullName)
    $report.Add($range)
    $report.Add("")
    Add-Image $merged $image $overlap
}

if ($overlap.Count -gt 0) {
    $overlapMap = @{}
    foreach ($addr in $overlap) { $overlapMap[$addr] = [byte]0 }
    Write-Host "没有生成文件。两份内容盖住了同一段："
    Write-Host (Get-RangeText $overlapMap)
    Write-Host ""
    Write-Host "Boot 应放在 0x08000000，App 应放在 0x08000800。两边都从 0 开始时会重叠。"
    exit 1
}

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}
$outHex = Join-Path $OutDir "合并烧录.hex"
$outTxt = Join-Path $OutDir "合并烧录.txt"
Write-IntelHex $merged $outHex
$summary = New-Object System.Collections.Generic.List[string]
$summary.Add("烧录用这一份：合并烧录.hex")
$summary.Add(("共 {0} 字节" -f $merged.Count))
$summary.Add("")
$summary.Add((Get-RangeText $merged))
$summary.Add("")
$summary.AddRange($report)
$utf8 = New-Object System.Text.UTF8Encoding $false
[System.IO.File]::WriteAllLines($outTxt, $summary.ToArray(), $utf8)

Write-Host "已生成："
Write-Host ("  " + $outHex)
Write-Host ("共 {0} 字节" -f $merged.Count)
Write-Host (Get-RangeText $merged)
Write-Host ""
Write-Host "用 WCH-LinkUtility 烧录这个 hex。"
exit 0
