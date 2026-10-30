$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak9" -Force
Write-Host "Backup: $path.bak9" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

function Apply($c, $old, $new, $label) {
    $o = $old.Replace("`r`n", "`n")
    $n = $new.Replace("`r`n", "`n")
    if ($o.StartsWith("`n")) { $o = $o.Substring(1) }
    if ($o.EndsWith("`n"))   { $o = $o.Substring(0, $o.Length-1) }
    if ($n.StartsWith("`n")) { $n = $n.Substring(1) }
    if ($n.EndsWith("`n"))   { $n = $n.Substring(0, $n.Length-1) }
    $r = $c.Replace($o, $n)
    if ($r -eq $c) {
        Write-Host ("NG: " + $label) -ForegroundColor Red
    } else {
        Write-Host ("OK: " + $label) -ForegroundColor Green
    }
    return $r
}

# ===== F12 4件リトライ（スペース数を正確に）=====

# bSqrt: 2スペース
$content = Apply $content `
'CellRef bSqrt(CellRef a, Env* e)  { return mkNum(sqrt(toD(a->cons.car))); }' `
'CellRef bSqrt(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } return mkNum(sqrt(toD(a->cons.car))); }' `
"F12 bSqrt retry"

# bFloor: 1スペース
$content = Apply $content `
'CellRef bFloor(CellRef a, Env* e) { return mkNum(floor(toD(a->cons.car))); }' `
'CellRef bFloor(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } return mkNum(floor(toD(a->cons.car))); }' `
"F12 bFloor retry"

# bCeil: 2スペース
$content = Apply $content `
'CellRef bCeil(CellRef a, Env* e)  { return mkNum(ceil(toD(a->cons.car))); }' `
'CellRef bCeil(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } return mkNum(ceil(toD(a->cons.car))); }' `
"F12 bCeil retry"

# bRound: 1スペース
$content = Apply $content `
'CellRef bRound(CellRef a, Env* e) { return mkNum(round(toD(a->cons.car))); }' `
'CellRef bRound(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } return mkNum(round(toD(a->cons.car))); }' `
"F12 bRound retry"

# ===== F2, F7 の適用状態を検証 =====
Write-Host ""
$hasF2  = $content.Contains('setErr("unexpected EOF")')
$hasF7a = $content.Contains('auto put = [&](const char* s)')
$hasF7b = $content.Contains('auto putc1 = [&](char ch)')

if ($hasF2) {
    Write-Host "VERIFY F2 (parseList EOF): APPLIED" -ForegroundColor Green
} else {
    Write-Host "VERIFY F2 (parseList EOF): MISSING - will apply now" -ForegroundColor Yellow
    $content = Apply $content `
"CellRef parseList() {
    skipWS();
    if (*gPos == ')') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}" `
"CellRef parseList() {
    skipWS();
    if (!*gPos) { setErr(`"unexpected EOF`"); return nilC; }
    if (*gPos == ')') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}" `
    "F2 parseList EOF fallback"
}

if ($hasF7a -and $hasF7b) {
    Write-Host "VERIFY F7 (printCell rewrite): APPLIED" -ForegroundColor Green
} else {
    Write-Host "VERIFY F7 (printCell rewrite): MISSING" -ForegroundColor Yellow
    Write-Host "  -> printCell rewrite did not take effect; will leave as-is for now" -ForegroundColor Yellow
    Write-Host "  (Old printCell still in place, but F7 is a Low priority bug)" -ForegroundColor Yellow
}

if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak9" -ForegroundColor Cyan