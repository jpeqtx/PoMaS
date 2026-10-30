$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak14" -Force
Write-Host "Backup: $path.bak14" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

$global:okCount = 0
$global:ngCount = 0

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
        $global:okCount++
        return $r
    }
    Write-Host "NG: $label" -ForegroundColor Red
    $global:ngCount++
    return $c
}

# ============ F7: printCell complete rewrite ============
$old = @'
void printCell(CellRef c, char* buf, int& p, int max) {
    if (!c || c == nilC || c->type == T_NIL) { strncpy(buf+p, "nil", max-p); p += 3; return; }
    if (c->type == T_BOOL_T) { strncpy(buf+p, "#t", max-p); p += 2; return; }
    if (c->type == T_NUM)  { p += snprintf(buf+p, max-p, "%g", c->num); return; }
    if (c->type == T_SYM)  { p += snprintf(buf+p, max-p, "%s", c->sym); return; }
    if (c->type == T_STR)  { p += snprintf(buf+p, max-p, "\"%s\"", c->str); return; }
    if (c->type == T_LAMBDA){ p += snprintf(buf+p, max-p, "<lambda>"); return; }
    if (c->type == T_BUILTIN){p += snprintf(buf+p, max-p, "<builtin>"); return; }
    if (c->type == T_CONS) {
        if (p < max-1) buf[p++] = '"'"'('"'"';
        bool first = true;
        while (!isNil(c) && c->type == T_CONS) {
            if (!first && p < max-1) buf[p++] = '"'"' '"'"';
            printCell(c->cons.car, buf, p, max);
            first = false; c = c->cons.cdr;
        }
        if (!isNil(c)) { strncpy(buf+p, " . ", max-p); p += 3; printCell(c, buf, p, max); }
        if (p < max-1) buf[p++] = '"'"')'"'"';
    }
    buf[p < max ? p : max-1] = 0;
}
'@
$new = @'
void printCell(CellRef c, char* buf, int& p, int max) {
    if (!buf || max < 2) { if (buf && max >= 1) buf[0] = 0; p = 0; return; }
    auto put = [&](const char* s) {
        if (p >= max - 1 || !s) return;
        int n = (int)strlen(s);
        if (n > max - p - 1) n = max - p - 1;
        if (n > 0) memcpy(buf + p, s, n);
        p += n;
        buf[p] = 0;
    };
    auto putc1 = [&](char ch) { if (p < max - 1) { buf[p++] = ch; buf[p] = 0; } };
    if (!c || c == nilC || c->type == T_NIL) { put("nil"); return; }
    if (c->type == T_BOOL_T) { put("#t"); return; }
    if (c->type == T_NUM) {
        if (p >= max - 1) return;
        int w = snprintf(buf + p, max - p, "%g", c->num);
        if (w < 0) return;
        if (p + w >= max) p = max - 1;
        else { p += w; buf[p] = 0; }
        return;
    }
    if (c->type == T_SYM) { put(c->sym ? c->sym : "?"); return; }
    if (c->type == T_STR) { put("\""); put(c->str ? c->str : "?"); put("\""); return; }
    if (c->type == T_LAMBDA) { put("<lambda>"); return; }
    if (c->type == T_BUILTIN) { put("<builtin>"); return; }
    if (c->type == T_CONS) {
        putc1('(');
        bool first = true;
        while (!isNil(c) && c->type == T_CONS) {
            if (!first) putc1(' ');
            printCell(c->cons.car, buf, p, max);
            first = false;
            c = c->cons.cdr;
        }
        if (!isNil(c)) { put(" . "); printCell(c, buf, p, max); }
        putc1(')');
    }
}
'@
$content = ApplyStr $content $old $new "F7 printCell rewrite"

# ============ F12ext: 10 single-arg funcs add 'too many args' check ============
$mathFns = @("bSin","bCos","bTan","bExp","bLog","bSqrt","bAbs","bFloor","bCeil","bRound")
foreach ($fn in $mathFns) {
    # Each function now looks like: CellRef FN(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } return mkNum(OP(toD(a->cons.car))); }
    # Extract the OP by finding the pattern
    $pattern = "CellRef $fn(CellRef a, Env* e) { if \(isNil\(a\) \|\| a->type != T_CONS\) \{ setErr\(`"missing arg`"\); return nilC; \} return mkNum\((\w+)\(toD\(a->cons.car\)\)\); \}"
    if ($content -match $pattern) {
        $op = $matches[1]
        $oldLine = $matches[0]
        $newLine = "CellRef $fn(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr(`"missing arg`"); return nilC; } if (!isNil(a->cons.cdr)) { setErr(`"too many args`"); return nilC; } return mkNum($op(toD(a->cons.car))); }"
        $content = ApplyStr $content $oldLine $newLine "F12ext $fn ($op)"
    } else {
        Write-Host "NG: F12ext $fn (pattern not found)" -ForegroundColor Red
        $global:ngCount++
    }
}

# ============ F13add: DepthGuard struct + parseDepth ============
$old = @'
static const char* gPos;

inline void skipWS() {
'@
$new = @'
static const char* gPos;
static int parseDepth = 0;
struct DepthGuard {
    int* d;
    DepthGuard(int* p) : d(p) { (*d)++; }
    ~DepthGuard() { (*d)--; }
};

inline void skipWS() {
'@
$content = ApplyStr $content $old $new "F13add DepthGuard struct"

# ============ F13add: parseExpr depth guard ============
$old = @'
CellRef parseExpr() {
    skipWS();
    if (!*gPos) return nilC;
'@
$new = @'
CellRef parseExpr() {
    if (parseDepth > 200) { setErr("parse too deep"); return nilC; }
    DepthGuard guard(&parseDepth);
    skipWS();
    if (!*gPos) return nilC;
'@
$content = ApplyStr $content $old $new "F13add parseExpr guard"

# ============ F13add: parseList depth guard ============
$old = @'
CellRef parseList() {
    skipWS();
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
'@
$new = @'
CellRef parseList() {
    if (parseDepth > 200) { setErr("parse too deep"); return nilC; }
    DepthGuard guard(&parseDepth);
    skipWS();
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
'@
$content = ApplyStr $content $old $new "F13add parseList guard"

# ============ M2: callFn 4 -> 8 args ============
$old = @'
            double a[4]; int na = 0;
            if (_src[_pos] != ')') {
                a[na++] = parseExpr();
                while (_src[_pos] == ',' && na < 4) {
                    _pos++; a[na++] = parseExpr();
                }
            }
'@
$new = @'
            double a[8]; int na = 0;
            if (_src[_pos] != ')') {
                a[na++] = parseExpr();
                while (_src[_pos] == ',' && na < 8) {
                    _pos++; a[na++] = parseExpr();
                }
            }
'@
$content = ApplyStr $content $old $new "M2 callFn 8 args"

# ============ M6: bAppend loop ============
$old = @'
CellRef bAppend(CellRef a, Env* e) {
    if (isNil(a)) return nilC;
    if (isNil(a->cons.cdr)) return a->cons.car;
    CellRef l1 = a->cons.car;
    CellRef rest = bAppend(a->cons.cdr, e);
    if (isNil(l1)) return rest;
    return mkCons(l1->cons.car, bAppend(mkCons(l1->cons.cdr, mkCons(rest, nilC)), e));
}
'@
$new = @'
CellRef bAppend(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef result = nilC, tail = nilC;
    CellRef cur = a;
    while (!isNil(cur) && cur->type == T_CONS) {
        CellRef item = cur->cons.car;
        bool isLast = isNil(cur->cons.cdr);
        if (isLast) {
            if (isNil(result)) return item;
            tail->cons.cdr = item;
            return result;
        }
        CellRef l = item;
        while (!isNil(l) && l->type == T_CONS) {
            CellRef nc = mkCons(l->cons.car, nilC);
            if (isNil(result)) { result = tail = nc; }
            else { tail->cons.cdr = nc; tail = nc; }
            l = l->cons.cdr;
        }
        cur = cur->cons.cdr;
    }
    return result;
}
'@
$content = ApplyStr $content $old $new "M6 bAppend loop"

# ============ write back ============
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

# ============ summary ============
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "  OK: $global:okCount" -ForegroundColor Green
Write-Host "  NG: $global:ngCount" -ForegroundColor $(if ($global:ngCount -eq 0) { "Green" } else { "Red" })

Write-Host ""
Write-Host "=== Verification ===" -ForegroundColor Cyan
$final = [System.IO.File]::ReadAllText($path)
$checks = [ordered]@{
    "F7 put lambda"      = $final.Contains('auto put = [&](const char* s)')
    "F7 putc1 lambda"    = $final.Contains('auto putc1 = [&](char ch)')
    "F13 DepthGuard"     = $final.Contains('struct DepthGuard')
    "F13 parseExpr"      = $final.Contains('if (parseDepth > 200) { setErr("parse too deep"); return nilC; }')
    "M2 8 args"          = $final.Contains('double a[8]; int na = 0;')
    "M6 bAppend loop"    = $final.Contains('bool isLast = isNil(cur->cons.cdr);')
    "F12ext sin"         = $final.Contains('if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(sin(')
    "F12ext sqrt"        = $final.Contains('if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(sqrt(')
}
foreach ($kv in $checks.GetEnumerator()) {
    $color = if ($kv.Value) { "Green" } else { "Red" }
    $status = if ($kv.Value) { "OK" } else { "FAIL" }
    Write-Host "  $($kv.Key): $status" -ForegroundColor $color
}

Write-Host ""
Write-Host "Backup: $path.bak14" -ForegroundColor Cyan