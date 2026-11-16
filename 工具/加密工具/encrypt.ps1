# 从最终烧录文件取出 A 区 30KB，按固件 AES-CBC 做成升级包，并保留原始文件。
$ErrorActionPreference = "Stop"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Drop = Join-Path $Here "放入"
$OutDir = Join-Path $Here "输出"
$KeyFile = Join-Path $Here "密钥.txt"
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

function Format-HexBytes([byte[]]$Bytes) {
    return (($Bytes | ForEach-Object { "{0:X2}" -f $_ }) -join "")
}

function Convert-HexBytes([string]$Text) {
    $hex = ($Text -replace '(?i)0x', '' -replace '\s', '')
    if (($hex.Length -lt 2) -or (($hex.Length % 2) -ne 0) -or ($hex -notmatch '^[0-9A-Fa-f]+$')) {
        throw "密钥.txt 里有不是十六进制的内容。"
    }
    $bytes = New-Object byte[] ($hex.Length / 2)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        $bytes[$i] = [Convert]::ToByte($hex.Substring($i * 2, 2), 16)
    }
    return $bytes
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

function Read-BinMap([System.IO.FileInfo]$File) {
    $start = [uint32]0
    if ($File.Name -match "(?i)0x([0-9A-Fa-f]{1,8})") {
        $start = [uint32]::Parse($Matches[1], [System.Globalization.NumberStyles]::AllowHexSpecifier)
    }
    $data = [System.IO.File]::ReadAllBytes($File.FullName)
    $map = @{}
    for ($i = 0; $i -lt $data.Length; $i++) {
        $addr = Convert-ToFlashAddress ([uint32]($start + [uint32]$i))
        $map[$addr] = $data[$i]
    }
    return $map
}

function Read-KeyMaterial([string]$Path) {
    if (-not (Test-Path $Path)) {
        throw "没有找到密钥.txt。"
    }
    $text = [System.IO.File]::ReadAllText($Path, [System.Text.Encoding]::UTF8)
    $key = $null
    $iv = $null
    foreach ($line in ($text -split "`r?`n")) {
        $trim = $line.Trim()
        if (($trim.Length -eq 0) -or $trim.StartsWith("#")) { continue }
        if ($trim -match '(?i)^(KEY|密钥)\s*[:=]?\s*([0-9A-Fa-fx\s]+)$') {
            $key = Convert-HexBytes $Matches[2]
        }
        elseif ($trim -match '(?i)^(IV|向量)\s*[:=]?\s*([0-9A-Fa-fx\s]+)$') {
            $iv = Convert-HexBytes $Matches[2]
        }
        else {
            throw "密钥.txt 里这行看不懂：$trim"
        }
    }
    if ($null -eq $key) { throw "密钥.txt 里没有 KEY。" }
    if ($null -eq $iv) { throw "密钥.txt 里没有 IV。" }
    if (($key.Length -ne 16) -and ($key.Length -ne 24) -and ($key.Length -ne 32)) {
        throw "密钥长度必须是 16、24 或 32 字节，现在是 $($key.Length) 字节。"
    }
    if ($iv.Length -ne 16) {
        throw "向量必须是 16 字节，现在是 $($iv.Length) 字节。"
    }
    return @{ Key = $key; IV = $iv }
}

function Protect-AesCbc([byte[]]$Plain, [byte[]]$Key, [byte[]]$Iv) {
    $pad = 0
    if (($Plain.Length % 16) -ne 0) {
        $pad = 16 - ($Plain.Length % 16)
    }
    $buf = New-Object byte[] ($Plain.Length + $pad)
    if ($Plain.Length -gt 0) {
        [Array]::Copy($Plain, $buf, $Plain.Length)
    }
    for ($i = 0; $i -lt $pad; $i++) {
        $buf[$Plain.Length + $i] = [byte]0xFF
    }
    $aes = [System.Security.Cryptography.Aes]::Create()
    try {
        $aes.Mode = [System.Security.Cryptography.CipherMode]::CBC
        $aes.Padding = [System.Security.Cryptography.PaddingMode]::None
        $aes.Key = $Key
        $aes.IV = $Iv
        $enc = $aes.CreateEncryptor()
        try {
            return $enc.TransformFinalBlock($buf, 0, $buf.Length)
        }
        finally {
            $enc.Dispose()
        }
    }
    finally {
        $aes.Dispose()
    }
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

function Get-ErasedByte([uint32]$Address) {
    # 小端读出的字是 0xE339E339，内存字节是 39 39 E3 E3
    switch ([int]($Address -band 3)) {
        0 { return [byte]0x39 }
        1 { return [byte]0x39 }
        2 { return [byte]0xE3 }
        default { return [byte]0xE3 }
    }
}

function Get-SlotPlain($Map, [uint32]$Start, [int]$Length) {
    $plain = New-Object byte[] $Length
    $filled = 0
    for ($i = 0; $i -lt $Length; $i++) {
        $addr = [uint32]($Start + [uint32]$i)
        if ($Map.ContainsKey($addr)) {
            $plain[$i] = [byte]$Map[$addr]
            $filled++
        }
        else {
            $plain[$i] = Get-ErasedByte $addr
        }
    }
    return @{ Plain = $plain; Filled = $filled }
}

function Clear-Generated {
    foreach ($name in @("原始.hex", "原始.bin", "加密.hex", "加密.bin", "说明.txt")) {
        $path = Join-Path $OutDir $name
        if (Test-Path $path) { Remove-Item $path }
    }
}

Write-Host "AES 加密"
Write-Host ""

$material = Read-KeyMaterial $KeyFile
$keyBits = $material.Key.Length * 8

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
    $candidates = @(
        (Join-Path $Here "..\合并烧录文件\输出\合并烧录.hex")
    )
    $inputFile = $null
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            $inputFile = Get-Item $candidate
            Write-Host "放入是空的，改用："
            break
        }
    }
    if ($null -eq $inputFile) {
        throw "放入里没有 hex 或 bin，也没有找到 合并烧录文件\输出\合并烧录.hex。"
    }
}

Write-Host ("  " + $inputFile.FullName)
Write-Host ""

$A0 = [uint32]0x08000800
$B0 = [uint32]0x08008000
$SlotLen = 30720

if ($inputFile.Extension -match '(?i)\.bin$') {
    $map = Read-BinMap $inputFile
}
else {
    $map = Read-IntelHex $inputFile.FullName
}

$slotA = Get-SlotPlain $map $A0 $SlotLen
$slotB = Get-SlotPlain $map $B0 $SlotLen
if ($slotA.Filled -eq 0 -or $slotB.Filled -eq 0) {
    Clear-Generated
    Write-Host "没有生成文件。升级包要同时有 A 面和 B 面，Boot 不加密。"
    if ($slotA.Filled -eq 0) { Write-Host "  A 区 0x08000800 .. 0x08007FFF 里没有数据。" }
    if ($slotB.Filled -eq 0) { Write-Host "  B 区 0x08008000 .. 0x0800F7FF 里没有数据。" }
    exit 1
}

$cipherA = Protect-AesCbc $slotA.Plain $material.Key $material.IV
$cipherB = Protect-AesCbc $slotB.Plain $material.Key $material.IV
if (($cipherA.Length -ne $SlotLen) -or ($cipherB.Length -ne $SlotLen)) {
    throw "升级包每一面都应该是 30720 字节。"
}

$encrypted = @{}
for ($i = 0; $i -lt $SlotLen; $i++) {
    $encrypted[[uint32]($A0 + [uint32]$i)] = $cipherA[$i]
    $encrypted[[uint32]($B0 + [uint32]$i)] = $cipherB[$i]
}

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}
Clear-Generated

$ext = ".hex"
if ($inputFile.Extension -match '(?i)\.bin$') { $ext = ".bin" }
$outPlain = Join-Path $OutDir ("原始" + $ext)
$outCipher = Join-Path $OutDir "加密.hex"
$outTxt = Join-Path $OutDir "说明.txt"
Copy-Item $inputFile.FullName $outPlain
Write-IntelHex $encrypted $outCipher

$report = New-Object System.Collections.Generic.List[string]
$report.Add("算法 AES-$keyBits-CBC")
$report.Add("密钥 " + (Format-HexBytes $material.Key))
$report.Add("向量 " + (Format-HexBytes $material.IV))
$report.Add("每一面单独从这一个向量开始")
$report.Add("空洞按擦除值 E339E339")
$report.Add("Boot 不在升级包里")
$report.Add("A 0x08000800 .. 0x08007FFF  原有 $($slotA.Filled) 字节  密文 $SlotLen 字节")
$report.Add("B 0x08008000 .. 0x0800F7FF  原有 $($slotB.Filled) 字节  密文 $SlotLen 字节")
$report.Add("")
$report.Add($inputFile.FullName)
$utf8 = New-Object System.Text.UTF8Encoding $true
[System.IO.File]::WriteAllLines($outTxt, $report.ToArray(), $utf8)

Write-Host "已生成："
Write-Host ("  " + $outPlain)
Write-Host ("  " + $outCipher)
Write-Host ("算法 AES-$keyBits-CBC。A、B 各 $SlotLen 字节，地址还在原来的位置。Boot 没有放进升级包。")
Write-Host ("A 区原有 $($slotA.Filled) 字节，B 区原有 $($slotB.Filled) 字节，空洞按擦除值 E339E339。")
Write-Host ""
Write-Host "烧录用原始文件。加密.hex 是升级包，上位机按地址整份发出去。"
exit 0
