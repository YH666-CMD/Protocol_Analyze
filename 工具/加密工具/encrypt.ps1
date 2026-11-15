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
        (Join-Path $Here "..\A区拷贝B区工具\输出\AB烧录.hex"),
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
        throw "放入里没有 hex 或 bin，也没有找到 AB烧录.hex 或 合并烧录.hex。"
    }
}

Write-Host ("  " + $inputFile.FullName)
Write-Host ""

$A0 = [uint32]0x08000800
$SlotLen = 30720

if ($inputFile.Extension -match '(?i)\.bin$') {
    $map = Read-BinMap $inputFile
}
else {
    $map = Read-IntelHex $inputFile.FullName
}

$slot = Get-SlotPlain $map $A0 $SlotLen
if ($slot.Filled -eq 0) {
    Clear-Generated
    Write-Host "没有生成文件。A 区 0x08000800 .. 0x08007FFF 里没有数据。"
    exit 1
}

$plain = $slot.Plain
$cipher = Protect-AesCbc $plain $material.Key $material.IV
if ($cipher.Length -ne $SlotLen) {
    throw "升级包长度不是 30720 字节。"
}

if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}
Clear-Generated

$ext = ".hex"
if ($inputFile.Extension -match '(?i)\.bin$') { $ext = ".bin" }
$outPlain = Join-Path $OutDir ("原始" + $ext)
$outCipher = Join-Path $OutDir "加密.bin"
$outTxt = Join-Path $OutDir "说明.txt"
Copy-Item $inputFile.FullName $outPlain
[System.IO.File]::WriteAllBytes($outCipher, $cipher)

$report = New-Object System.Collections.Generic.List[string]
$report.Add("算法 AES-$keyBits-CBC")
$report.Add("密钥 " + (Format-HexBytes $material.Key))
$report.Add("向量 " + (Format-HexBytes $material.IV))
$report.Add("范围 0x08000800 .. 0x08007FFF")
$report.Add("A区已有数据 $($slot.Filled) 字节")
$report.Add("空洞按擦除值 E339E339")
$report.Add("明文 $SlotLen 字节")
$report.Add("密文 $SlotLen 字节")
$report.Add("")
$report.Add($inputFile.FullName)
$utf8 = New-Object System.Text.UTF8Encoding $true
[System.IO.File]::WriteAllLines($outTxt, $report.ToArray(), $utf8)

Write-Host "已生成："
Write-Host ("  " + $outPlain)
Write-Host ("  " + $outCipher)
Write-Host ("算法 AES-$keyBits-CBC，升级包 $SlotLen 字节。A 区原有 $($slot.Filled) 字节，其余按擦除值 E339E339。")
Write-Host ""
Write-Host "烧录用原始文件。加密.bin 是升级包。"
exit 0
