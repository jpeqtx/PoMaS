$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak13" -Force
Write-Host "Backup: $path.bak13" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

function ApplyStr($c, $old, $new, $label) {
    $o = $old.Replace("`r`n", "`n")
    $n = $new.Replace("`r`n", "`n")
    if ($o.StartsWith("`n")) { $o = $o.Substring(1) }
    if ($o.EndsWith("`n"))   { $o = $o.Substring(0, $o.Length-1) }
    if ($n.StartsWith("`n")) { $n = $n.Substring(1) }
    if ($n.EndsWith("`n"))   { $n = $n.Substring(0, $n.Length-1) }

    if ($c.Contains($o)) {
        $r = $c.Replace($o, $n)
        Write-Host "OK: $label" -ForegroundColor Green
        return $r
    }
    Write-Host "NG: $label" -ForegroundColor Red
    return $c
}

# ============ Fix 1: Remove old setErr definition (after globalEnv) ============
$old = @'
static Env globalEnv;
static char errMsg[64] = "";
inline void setErr(const char* m) { strncpy(errMsg, m, 63); errMsg[63] = 0; }
static const char* gPos;
'@
$new = @'
static Env globalEnv;
static const char* gPos;
'@
$content = ApplyStr $content $old $new "Remove old setErr definition"

# ============ Fix 2: Add errMsg + setErr before mkSym ============
# mkSym is defined right after mkNum; insert before it
$old = @'
inline CellRef mkNum(double v) { CellRef c = alloc(); if (!c) return nilC; c->type = T_NUM; c->num = v; return c; }
inline CellRef mkSym(const char* s) {
'@
$new = @'
static char errMsg[64] = "";
inline void setErr(const char* m) { strncpy(errMsg, m, 63); errMsg[63] = 0; }

inline CellRef mkNum(double v) { CellRef c = alloc(); if (!c) return nilC; c->type = T_NUM; c->num = v; return c; }
inline CellRef mkSym(const char* s) {
'@
$content = ApplyStr $content $old $new "Add errMsg+setErr before mkSym"

# ============ Verify ============
Write-Host ""
Write-Host "=== Verification ===" -ForegroundColor Cyan
$final = $content

# Count occurrences of setErr definition
$defCount = ([regex]::Matches($final, 'inline void setErr\(const char\* m\)')).Count
if ($defCount -eq 1) {
    Write-Host "  setErr definition: exactly 1 (OK)" -ForegroundColor Green
} else {
    Write-Host "  setErr definition count: $defCount (expected 1)" -ForegroundColor Red
}

# errMsg definition count
$emCount = ([regex]::Matches($final, 'static char errMsg\[64\] = "";')).Count
if ($emCount -eq 1) {
    Write-Host "  errMsg definition: exactly 1 (OK)" -ForegroundColor Green
} else {
    Write-Host "  errMsg definition count: $emCount (expected 1)" -ForegroundColor Red
}

# Position check: setErr before mkSym
$posSetErr = $final.IndexOf('inline void setErr')
$posMkSym = $final.IndexOf('inline CellRef mkSym')
if ($posSetErr -ge 0 -and $posMkSym -ge 0 -and $posSetErr -lt $posMkSym) {
    Write-Host "  setErr is before mkSym (OK)" -ForegroundColor Green
} else {
    Write-Host "  setErr position INVALID" -ForegroundColor Red
}

if ($hasCRLF) { $final = $final.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $final, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak13" -ForegroundColor Cyan