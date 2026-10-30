$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"
$bak8 = "$path.bak8"

if (-not (Test-Path $bak8)) {
    Write-Host "ERROR: bak8 missing" -ForegroundColor Red
    return
}

Copy-Item $bak8 $path -Force
Copy-Item $path "$path.bak12" -Force
Write-Host "Restored from bak8. Backup: $path.bak12" -ForegroundColor Cyan

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

# ===== F10a: Env struct =====
$old = @'
struct Env {
    EnvEntry e[64];
    int      n;
    Env*     parent;
    Env() : n(0), parent(nullptr) {}
    void set(const char* k, CellRef v) {
        for (int i = 0; i < n; i++) if (!strcmp(e[i].name, k)) { e[i].val = v; return; }
        if (n < 64) { e[n].name = strdup(k); e[n].val = v; n++; }
    }
'@
$new = @'
struct Env {
    EnvEntry e[64];
    int      n;
    Env*     parent;
    bool     marked;
    Env() : n(0), parent(nullptr), marked(false) {}
    bool set(const char* k, CellRef v) {
        for (int i = 0; i < n; i++) if (!strcmp(e[i].name, k)) { e[i].val = v; return true; }
        if (n < 64) {
            char* dup = strdup(k);
            if (!dup) return false;
            e[n].name = dup;
            e[n].val = v;
            n++;
            return true;
        }
        return false;
    }
'@
$content = ApplyStr $content $old $new "F10a Env marked + bool set"

# ===== F10b: markEnvChain =====
$old = @'
void markEnvChain(Env* env) {
    while (env) {
        for (int i = 0; i < env->n; i++) markCell(env->e[i].val);
        env = env->parent;
    }
}
'@
$new = @'
void markEnvChain(Env* env) {
    while (env) {
        if (env->marked) return;
        env->marked = true;
        for (int i = 0; i < env->n; i++) markCell(env->e[i].val);
        env = env->parent;
    }
}
'@
$content = ApplyStr $content $old $new "F10b markEnvChain"

# ===== F10c: sweepEnvs + gc =====
$old = @'
void gc() {
    if (gcInProgress) return;
    gcInProgress = true;
    for (int i = 0; i < poolUsed; i++) pool[i].marked = false;
    if (nilC)   markCell(nilC);
    if (trueC)  markCell(trueC);
    markEnvChain(&globalEnv);
    sweepPool();
    gcInProgress = false;
}
'@
$new = @'
void sweepEnvs() {
    int j = 0;
    for (int i = 0; i < envN; i++) {
        Env* e = envList[i];
        if (e->marked) {
            envList[j++] = e;
        } else {
            for (int k = 0; k < e->n; k++) free(e->e[k].name);
            delete e;
        }
    }
    envN = j;
}
void gc() {
    if (gcInProgress) return;
    gcInProgress = true;
    for (int i = 0; i < poolUsed; i++) pool[i].marked = false;
    for (int i = 0; i < envN; i++) envList[i]->marked = false;
    globalEnv.marked = false;
    if (nilC)   markCell(nilC);
    if (trueC)  markCell(trueC);
    markEnvChain(&globalEnv);
    sweepPool();
    sweepEnvs();
    gcInProgress = false;
}
'@
$content = ApplyStr $content $old $new "F10c sweepEnvs + gc"

# ===== F1: remove (gc) builtin =====
$old = @'
    globalEnv.set("apply", mkBuiltin(bApply));
    globalEnv.set("gc", mkBuiltin(bGCCall));
    globalEnv.set("pi", mkNum(M_PI));
'@
$new = @'
    globalEnv.set("apply", mkBuiltin(bApply));
    globalEnv.set("pi", mkNum(M_PI));
'@
$content = ApplyStr $content $old $new "F1 remove (gc) builtin"

# ===== F2: parseList EOF =====
$old = @'
CellRef parseList() {
    skipWS();
    if (*gPos == ')') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}
'@
$new = @'
CellRef parseList() {
    skipWS();
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
    if (*gPos == ')') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}
'@
$content = ApplyStr $content $old $new "F2 parseList EOF"

# ===== F3a: REPL =====
$old = @'
                    ULisp::CellRef expr = ULisp::parseStr(input);
                    ULisp::errMsg[0] = 0;
                    ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$new = @'
                    if (ULisp::envN > 100) ULisp::gc();
                    ULisp::errMsg[0] = 0;
                    ULisp::CellRef expr = ULisp::parseStr(input);
                    ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$content = ApplyStr $content $old $new "F3a REPL errMsg + envN safe"

# ===== F3b: runFile =====
$old = @'
            ULisp::CellRef expr = ULisp::parseExpr();
            ULisp::errMsg[0] = 0;
            ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$new = @'
            ULisp::errMsg[0] = 0;
            ULisp::CellRef expr = ULisp::parseExpr();
            ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$content = ApplyStr $content $old $new "F3b runFile errMsg"

# ===== F3c: examples =====
$old = @'
        ULisp::CellRef expr = ULisp::parseStr(code);
        ULisp::errMsg[0] = 0;
        ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$new = @'
        ULisp::errMsg[0] = 0;
        ULisp::CellRef expr = ULisp::parseStr(code);
        ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
'@
$content = ApplyStr $content $old $new "F3c examples errMsg"

# ===== F4: if args =====
$old = @'
            CellRef c = eval(args->cons.car, env);
            if (!isNil(c)) return eval(args->cons.cdr->cons.car, env);
            else if (!isNil(args->cons.cdr->cons.cdr)) return eval(args->cons.cdr->cons.cdr->cons.car, env);
            return nilC;
        }
        if (!strcmp(n, "cond")) {
'@
$new = @'
            CellRef c = eval(args->cons.car, env);
            CellRef rest = args->cons.cdr;
            if (!isNil(c)) {
                if (isNil(rest) || rest->type != T_CONS) { setErr("if: no then"); return nilC; }
                return eval(rest->cons.car, env);
            }
            if (isNil(rest) || rest->type != T_CONS) return nilC;
            CellRef elsePart = rest->cons.cdr;
            if (isNil(elsePart) || elsePart->type != T_CONS) return nilC;
            return eval(elsePart->cons.car, env);
        }
        if (!strcmp(n, "cond")) {
'@
$content = ApplyStr $content $old $new "F4 if args"

# ===== F5: cond clause =====
$old = @'
        if (!strcmp(n, "cond")) {
            while (!isNil(args)) {
                CellRef clause = args->cons.car;
                CellRef test = eval(clause->cons.car, env);
                if (!isNil(test)) {
                    CellRef body = clause->cons.cdr;
                    CellRef r = nilC;
                    while (!isNil(body)) { r = eval(body->cons.car, env); body = body->cons.cdr; }
                    return r;
                }
                args = args->cons.cdr;
            }
            return nilC;
        }
'@
$new = @'
        if (!strcmp(n, "cond")) {
            while (!isNil(args) && args->type == T_CONS) {
                CellRef clause = args->cons.car;
                if (isNil(clause) || clause->type != T_CONS) {
                    args = args->cons.cdr;
                    continue;
                }
                CellRef test = eval(clause->cons.car, env);
                if (!isNil(test)) {
                    CellRef body = clause->cons.cdr;
                    CellRef r = nilC;
                    while (!isNil(body) && body->type == T_CONS) { r = eval(body->cons.car, env); body = body->cons.cdr; }
                    return r;
                }
                args = args->cons.cdr;
            }
            return nilC;
        }
'@
$content = ApplyStr $content $old $new "F5 cond clause"

# ===== F6: let binding =====
$old = @'
            while (!isNil(bindings)) {
                CellRef bind = bindings->cons.car;
                CellRef v = eval(bind->cons.cdr->cons.car, env);
                newEnv->set(bind->cons.car->sym, v);
                bindings = bindings->cons.cdr;
            }
'@
$new = @'
            while (!isNil(bindings) && bindings->type == T_CONS) {
                CellRef bind = bindings->cons.car;
                if (!isNil(bind) && bind->type == T_CONS
                    && !isNil(bind->cons.cdr) && bind->cons.cdr->type == T_CONS
                    && bind->cons.car && bind->cons.car->type == T_SYM) {
                    CellRef v = eval(bind->cons.cdr->cons.car, env);
                    newEnv->set(bind->cons.car->sym, v);
                }
                bindings = bindings->cons.cdr;
            }
'@
$content = ApplyStr $content $old $new "F6 let binding"

# ===== F17: define env error =====
$old = @'
                CellRef v = eval(args->cons.cdr->cons.car, env);
                env->set(target->sym, v);
                return v;
'@
$new = @'
                CellRef v = eval(args->cons.cdr->cons.car, env);
                if (!env->set(target->sym, v)) { setErr("env table full"); return nilC; }
                return v;
'@
$content = ApplyStr $content $old $new "F17 define env error"

# ===== F8a: mkSym =====
$old = @'
inline CellRef mkSym(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_SYM;
    int n = strlen(s) + 1;
    c->sym = (char*)malloc(n);
    if (c->sym) strcpy(c->sym, s);
    return c;
}
'@
$new = @'
inline CellRef mkSym(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("sym malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_SYM; c->sym = tmp;
    return c;
}
'@
$content = ApplyStr $content $old $new "F8a mkSym"

# ===== F8b: mkStr =====
$old = @'
inline CellRef mkStr(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_STR;
    int n = strlen(s) + 1;
    c->str = (char*)malloc(n);
    if (c->str) strcpy(c->str, s);
    return c;
}
'@
$new = @'
inline CellRef mkStr(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("str malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_STR; c->str = tmp;
    return c;
}
'@
$content = ApplyStr $content $old $new "F8b mkStr"

# ===== F9a: bDiv =====
$old = @'
CellRef bDiv(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(1);
    double s = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a)) { double d = toD(a->cons.car); if (d == 0) return mkNum(0); s /= d; a = a->cons.cdr; }
    return mkNum(s);
}
'@
$new = @'
CellRef bDiv(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(1);
    double s = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a) && a->type == T_CONS) {
        double d = toD(a->cons.car);
        if (d == 0) { setErr("division by zero"); return nilC; }
        s /= d;
        a = a->cons.cdr;
    }
    return mkNum(s);
}
'@
$content = ApplyStr $content $old $new "F9a bDiv"

# ===== F9b: bMod =====
$old = 'CellRef bMod(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkNum(fmod(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }'
$new = @'
CellRef bMod(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    double d = toD(a->cons.cdr->cons.car);
    if (d == 0) { setErr("mod by zero"); return nilC; }
    return mkNum(fmod(toD(a->cons.car), d));
}
'@
$content = ApplyStr $content $old $new "F9b bMod"

# ===== F11a-h: 8 guards =====
$content = ApplyStr $content 'CellRef bCar(CellRef a, Env* e)  { CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC; }' 'CellRef bCar(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC; }' "F11a bCar"

$content = ApplyStr $content 'CellRef bCdr(CellRef a, Env* e)  { CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.cdr : nilC; }' 'CellRef bCdr(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.cdr : nilC; }' "F11b bCdr"

$content = ApplyStr $content 'CellRef bNullP(CellRef a, Env* e){ return isNil(a->cons.car) ? trueC : nilC; }' 'CellRef bNullP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; return isNil(a->cons.car) ? trueC : nilC; }' "F11c bNullP"

$content = ApplyStr $content 'CellRef bAtomP(CellRef a, Env* e){ CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }' 'CellRef bAtomP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }' "F11d bAtomP"

$content = ApplyStr $content 'CellRef bNumP(CellRef a, Env* e) { CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }' 'CellRef bNumP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }' "F11e bNumP"

$content = ApplyStr $content 'CellRef bSymP(CellRef a, Env* e) { CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }' 'CellRef bSymP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }' "F11f bSymP"

$old = @'
CellRef bLength(CellRef a, Env* e) {
    int n = 0; CellRef l = a->cons.car;
'@
$new = @'
CellRef bLength(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(0);
    int n = 0; CellRef l = a->cons.car;
'@
$content = ApplyStr $content $old $new "F11g bLength"

$old = @'
CellRef bReverse(CellRef a, Env* e) {
    CellRef l = a->cons.car; CellRef r = nilC;
'@
$new = @'
CellRef bReverse(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef l = a->cons.car; CellRef r = nilC;
'@
$content = ApplyStr $content $old $new "F11h bReverse"

# ===== F12: 10 single-arg math =====
$mathFns = @(
    @("bSin", "sin", "3"), @("bCos", "cos", "3"), @("bTan", "tan", "3"),
    @("bExp", "exp", "3"), @("bLog", "log", "3"), @("bSqrt", "sqrt", "2"),
    @("bAbs", "fabs", "3"), @("bFloor", "floor", "1"), @("bCeil", "ceil", "2"),
    @("bRound", "round", "1")
)
foreach ($p in $mathFns) {
    $fn = $p[0]; $op = $p[1]; $sp = ' ' * [int]$p[2]
    $old = "CellRef $fn(CellRef a, Env* e)$sp{ return mkNum($op(toD(a->cons.car))); }"
    $new = "CellRef $fn(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr(`"missing arg`"); return nilC; } return mkNum($op(toD(a->cons.car))); }"
    $content = ApplyStr $content $old $new "F12 $fn"
}

# ===== F13: evalArgs loop =====
$old = @'
CellRef evalArgs(CellRef args, Env* env) {
    if (isNil(args)) return nilC;
    return mkCons(eval(args->cons.car, env), evalArgs(args->cons.cdr, env));
}
'@
$new = @'
CellRef evalArgs(CellRef args, Env* env) {
    if (isNil(args)) return nilC;
    CellRef head = nilC, tail = nilC;
    while (!isNil(args) && args->type == T_CONS) {
        CellRef v = eval(args->cons.car, env);
        CellRef nc = mkCons(v, nilC);
        if (isNil(head)) { head = tail = nc; }
        else { tail->cons.cdr = nc; tail = nc; }
        args = args->cons.cdr;
    }
    return head;
}
'@
$content = ApplyStr $content $old $new "F13 evalArgs loop"

# ===== F15: Maxima URL =====
$content = ApplyStr $content 'char url[128] = "http://example.com/maxima?expr=";' 'char url[128] = "http://YOUR-PROXY-HOST/maxima?expr=";' "F15 Maxima URL"

# ===== write back =====
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

# ===== summary =====
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Cyan
Write-Host "  OK: $global:okCount"  -ForegroundColor Green
Write-Host "  NG: $global:ngCount"  -ForegroundColor $(if ($global:ngCount -eq 0) { "Green" } else { "Red" })

Write-Host ""
Write-Host "=== Critical checks (unique markers) ===" -ForegroundColor Cyan
$final = [System.IO.File]::ReadAllText($path)
$checks = [ordered]@{
    "F1 gc removed"  = (-not $final.Contains('globalEnv.set("gc"'))
    "F2 EOF guard"   = $final.Contains('setErr("unexpected EOF")')
    "F10a marked"    = $final.Contains('bool     marked;')
    "F10b markEnv"   = $final.Contains('if (env->marked) return;')
    "F10c sweepEnv"  = $final.Contains('void sweepEnvs()')
    "F13 loop"       = (-not $final.Contains('return mkCons(eval(args->cons.car, env), evalArgs(args->cons.cdr, env));'))
    "F15 URL"        = $final.Contains('YOUR-PROXY-HOST')
}
foreach ($kv in $checks.GetEnumerator()) {
    $color = if ($kv.Value) { "Green" } else { "Red" }
    $status = if ($kv.Value) { "OK" } else { "FAIL" }
    Write-Host "  $($kv.Key): $status" -ForegroundColor $color
}

Write-Host ""
Write-Host "Backup: $path.bak12" -ForegroundColor Cyan