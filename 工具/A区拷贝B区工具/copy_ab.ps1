# 把烧录文件里 A 区的字节复制到 B 区，生成两面内容相同的 hex。
$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Drop = Join-Path $Here "放入"
$OutDir = Join-Path $Here "输出"
$FlashBase = [uint32]0x08000000
$A0 = [uint32]0x08000800
$SlotLen = [uint32]0x7800
$B0 = [uint32]($A0 + $SlotLen)

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
    return [uint32]0
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

function Get-WindowMap($Map, [uint32]$Start, [uint32]$Length) {
    $end = [uint32]($Start + $Length)
    $part = @{}
    foreach ($key in @($Map.Keys)) {
        $addr = [uint32]$key
        if (($addr -ge $Start) -and ($addr -lt $end)) {
            $part[$addr] = $Map[$addr]
        }
    }
    return $part
}

Write-Host "A 区复制到 B 区"
Write-Host ""

$dropped = @(Get-ChildItem -Path $Drop -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -match '(?i)^\.(hex|ihx|bin)$' } |
    Sort-Object LastWriteTime -Descending)

if ($dropped.Count -gt 1) {
    Write-Host "放入里有多份，用最新的一份。"
}

if ($dropped.Count -gt 0) {
    $inputFile = $dropped[0]
}
else {
    $auto = Join-Path $Here "..\合并烧录文件\输出\合并烧录.hex"
    if (Test-Path $auto) {
        $inputFile = Get-Item $auto
        Write-Host "放入是空的，改用合并结果："
    }
    else {
        throw "放入里没有 hex 或 bin，也没有找到 合并烧录文件\输出\合并烧录.hex。"
    }
}

Write-Host ("  " + $inputFile.FullName)
Write-Host ""

if ($inputFile.Extension -match '(?i)\.bin$') {
    $map = Read-BinFile $inputFile.FullName (Get-BinStart $inputFile)
}
else {
    $map = Read-IntelHex $inputFile.FullName
}

$aPart = Get-WindowMap $map $A0 $SlotLen
if ($aPart.Count -eq 0) {
    Write-Host "没有生成文件。A 区 0x08000800 .. 0x08007FFF 里没有数据。"
    Write-Host "请先合并一份链接在 0x0800 的 App，再放到「放入」。"
    exit 1
}

$removed = 0
$bEnd = [uint32]($B0 + $SlotLen)
foreach ($key in @($map.Keys)) {
    $addr = [uint32]$key
    if (($addr -ge $B0) -and ($addr -lt $bEnd)) {
        $map.Remove($addr)
        $removed++
    }
}

foreach ($key in $aPart.Keys) {
    $src = [uint32]$key
    $dst = [uint32]($src + $SlotLen)
    $map[$dst] = $aPart[$src]
}

$bPart = Get-WindowMap $map $B0 $SlotLen
$same = $true
foreach ($key in $aPart.Keys) {
    $src = [uint32]$key
    $dst = [uint32]($src + $SlotLen)
    if ([byte]$map[$dst] -ne [byte]$aPart[$src]) {
        $same = $false
        break
    }
}
if (-not $same -or ($aPart.Count -ne $bPart.Count)) {
    throw "复制后 A 区和 B 区不一致。"
}

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}
$outHex = Join-Path $OutDir "AB烧录.hex"
$outTxt = Join-Path $OutDir "AB烧录.txt"
Write-IntelHex $map $outHex

$range = Get-RangeText $map
$aRange = Get-RangeText $aPart
$bRange = Get-RangeText $bPart
$report = New-Object System.Collections.Generic.List[string]
$report.Add($inputFile.FullName)
$report.Add("")
$report.Add("A 区")
$report.Add($aRange)
$report.Add("")
$report.Add("B 区")
$report.Add($bRange)
$report.Add("")
$report.Add("覆盖掉 B 区原有 $removed 字节")
$report.Add("")
$report.Add("整份")
$report.Add($range)
$utf8 = New-Object System.Text.UTF8Encoding $true
[System.IO.File]::WriteAllLines($outTxt, $report.ToArray(), $utf8)

Write-Host "已生成："
Write-Host ("  " + $outHex)
Write-Host ("A 区复制了 {0} 字节，覆盖掉 B 区原有 {1} 字节。" -f $aPart.Count, $removed)
Write-Host ""
Write-Host "A 区"
Write-Host $aRange
Write-Host ""
Write-Host "B 区"
Write-Host $bRange
Write-Host ""
Write-Host "用 WCH-LinkUtility 烧录这个 hex。"
exit 0
