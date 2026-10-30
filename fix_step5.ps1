$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak8" -Force
Write-Host "Backup: $path.bak8" -ForegroundColor Cyan

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

# ============ F10a: Env struct with marked field + bool set ============
$content = Apply $content `
'struct Env {
    EnvEntry e[64];
    int      n;
    Env*     parent;
    Env() : n(0), parent(nullptr) {}
    void set(const char* k, CellRef v) {
        for (int i = 0; i < n; i++) if (!strcmp(e[i].name, k)) { e[i].val = v; return; }
        if (n < 64) { e[n].name = strdup(k); e[n].val = v; n++; }
    }' `
'struct Env {
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
    }' `
"F10a Env marked + bool set"

# ============ F10b: markEnvChain early return ============
$content = Apply $content `
'void markEnvChain(Env* env) {
    while (env) {
        for (int i = 0; i < env->n; i++) markCell(env->e[i].val);
        env = env->parent;
    }
}' `
'void markEnvChain(Env* env) {
    while (env) {
        if (env->marked) return;
        env->marked = true;
        for (int i = 0; i < env->n; i++) markCell(env->e[i].val);
        env = env->parent;
    }
}' `
"F10b markEnvChain early-return"

# ============ F10c: sweepEnvs + gc update ============
$content = Apply $content `
'void gc() {
    if (gcInProgress) return;
    gcInProgress = true;
    for (int i = 0; i < poolUsed; i++) pool[i].marked = false;
    if (nilC)   markCell(nilC);
    if (trueC)  markCell(trueC);
    markEnvChain(&globalEnv);
    sweepPool();
    gcInProgress = false;
}' `
'void sweepEnvs() {
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
}' `
"F10c sweepEnvs + gc"

# ============ F1: remove (gc) builtin registration ============
$content = Apply $content `
'    globalEnv.set("apply", mkBuiltin(bApply));
    globalEnv.set("gc", mkBuiltin(bGCCall));
    globalEnv.set("pi", mkNum(M_PI));' `
'    globalEnv.set("apply", mkBuiltin(bApply));
    globalEnv.set("pi", mkNum(M_PI));' `
"F1 remove (gc) builtin"

# ============ F2: parseList EOF check ============
$content = Apply $content `
'CellRef parseList() {
    skipWS();
    if (*gPos == '"'"')'"'"') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}' `
'CellRef parseList() {
    skipWS();
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
    if (*gPos == '"'"')'"'"') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}' `
"F2 parseList EOF"

# ============ F3a: REPL errMsg reorder ============
$content = Apply $content `
'                    ULisp::CellRef expr = ULisp::parseStr(input);
                    ULisp::errMsg[0] = 0;
                    ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
'                    if (ULisp::envN > 100) ULisp::gc();
                    ULisp::errMsg[0] = 0;
                    ULisp::CellRef expr = ULisp::parseStr(input);
                    ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
"F3a REPL errMsg + envN safe point"

# ============ F3b: runFile errMsg reorder ============
$content = Apply $content `
'            ULisp::CellRef expr = ULisp::parseExpr();
            ULisp::errMsg[0] = 0;
            ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
'            ULisp::errMsg[0] = 0;
            ULisp::CellRef expr = ULisp::parseExpr();
            ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
"F3b runFile errMsg"

# ============ F3c: examples errMsg reorder ============
$content = Apply $content `
'        ULisp::CellRef expr = ULisp::parseStr(code);
        ULisp::errMsg[0] = 0;
        ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
'        ULisp::errMsg[0] = 0;
        ULisp::CellRef expr = ULisp::parseStr(code);
        ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);' `
"F3c examples errMsg"

# ============ F4: if args ============
$content = Apply $content `
'        if (!strcmp(n, "if")) {
            if (isNil(args) || args->type != T_CONS) { setErr("if: bad args"); return nilC; }
            CellRef c = eval(args->cons.car, env);
            if (!isNil(c)) return eval(args->cons.cdr->cons.car, env);
            else if (!isNil(args->cons.cdr->cons.cdr)) return eval(args->cons.cdr->cons.cdr->cons.car, env);
            return nilC;
        }' `
'        if (!strcmp(n, "if")) {
            if (isNil(args) || args->type != T_CONS) { setErr("if: bad args"); return nilC; }
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
        }' `
"F4 if args"

# ============ F5: cond clause ============
$content = Apply $content `
'        if (!strcmp(n, "cond")) {
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
        }' `
'        if (!strcmp(n, "cond")) {
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
        }' `
"F5 cond clause"

# ============ F6: let binding ============
$content = Apply $content `
'            while (!isNil(bindings)) {
                CellRef bind = bindings->cons.car;
                CellRef v = eval(bind->cons.cdr->cons.car, env);
                newEnv->set(bind->cons.car->sym, v);
                bindings = bindings->cons.cdr;
            }' `
'            while (!isNil(bindings) && bindings->type == T_CONS) {
                CellRef bind = bindings->cons.car;
                if (!isNil(bind) && bind->type == T_CONS
                    && !isNil(bind->cons.cdr) && bind->cons.cdr->type == T_CONS
                    && bind->cons.car && bind->cons.car->type == T_SYM) {
                    CellRef v = eval(bind->cons.cdr->cons.car, env);
                    newEnv->set(bind->cons.car->sym, v);
                }
                bindings = bindings->cons.cdr;
            }' `
"F6 let binding"

# ============ F17: define Env::set error detection ============
$content = Apply $content `
'            if (target->type == T_SYM) {
                if (isNil(args->cons.cdr)) { setErr("define: no value"); return nilC; }
                CellRef v = eval(args->cons.cdr->cons.car, env);
                env->set(target->sym, v);
                return v;
            }' `
'            if (target->type == T_SYM) {
                if (isNil(args->cons.cdr)) { setErr("define: no value"); return nilC; }
                CellRef v = eval(args->cons.cdr->cons.car, env);
                if (!env->set(target->sym, v)) { setErr("env table full"); return nilC; }
                return v;
            }' `
"F17 define env error"

# ============ F8a: mkSym malloc ============
$content = Apply $content `
'inline CellRef mkSym(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_SYM;
    int n = strlen(s) + 1;
    c->sym = (char*)malloc(n);
    if (c->sym) strcpy(c->sym, s);
    return c;
}' `
'inline CellRef mkSym(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("sym malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_SYM; c->sym = tmp;
    return c;
}' `
"F8a mkSym malloc"

# ============ F8b: mkStr malloc ============
$content = Apply $content `
'inline CellRef mkStr(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_STR;
    int n = strlen(s) + 1;
    c->str = (char*)malloc(n);
    if (c->str) strcpy(c->str, s);
    return c;
}' `
'inline CellRef mkStr(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("str malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_STR; c->str = tmp;
    return c;
}' `
"F8b mkStr malloc"

# ============ F9a: bDiv zero check ============
$content = Apply $content `
'CellRef bDiv(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(1);
    double s = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a)) { double d = toD(a->cons.car); if (d == 0) return mkNum(0); s /= d; a = a->cons.cdr; }
    return mkNum(s);
}' `
'CellRef bDiv(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(1);
    double s = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a) && a->type == T_CONS) {
        double d = toD(a->cons.car);
        if (d == 0) { setErr("division by zero"); return nilC; }
        s /= d;
        a = a->cons.cdr;
    }
    return mkNum(s);
}' `
"F9a bDiv zero"

# ============ F9b: bMod zero check ============
$content = Apply $content `
'CellRef bMod(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkNum(fmod(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }' `
'CellRef bMod(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    double d = toD(a->cons.cdr->cons.car);
    if (d == 0) { setErr("mod by zero"); return nilC; }
    return mkNum(fmod(toD(a->cons.car), d));
}' `
"F9b bMod zero"

# ============ F11a: bCar ============
$content = Apply $content `
'CellRef bCar(CellRef a, Env* e)  { CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC; }' `
'CellRef bCar(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC; }' `
"F11a bCar"

# ============ F11b: bCdr ============
$content = Apply $content `
'CellRef bCdr(CellRef a, Env* e)  { CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.cdr : nilC; }' `
'CellRef bCdr(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.cdr : nilC; }' `
"F11b bCdr"

# ============ F11c: bNullP ============
$content = Apply $content `
'CellRef bNullP(CellRef a, Env* e){ return isNil(a->cons.car) ? trueC : nilC; }' `
'CellRef bNullP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; return isNil(a->cons.car) ? trueC : nilC; }' `
"F11c bNullP"

# ============ F11d: bAtomP ============
$content = Apply $content `
'CellRef bAtomP(CellRef a, Env* e){ CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }' `
'CellRef bAtomP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }' `
"F11d bAtomP"

# ============ F11e: bNumP ============
$content = Apply $content `
'CellRef bNumP(CellRef a, Env* e) { CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }' `
'CellRef bNumP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }' `
"F11e bNumP"

# ============ F11f: bSymP ============
$content = Apply $content `
'CellRef bSymP(CellRef a, Env* e) { CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }' `
'CellRef bSymP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }' `
"F11f bSymP"

# ============ F11g: bLength ============
$content = Apply $content `
'CellRef bLength(CellRef a, Env* e) {
    int n = 0; CellRef l = a->cons.car;' `
'CellRef bLength(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(0);
    int n = 0; CellRef l = a->cons.car;' `
"F11g bLength"

# ============ F11h: bReverse ============
$content = Apply $content `
'CellRef bReverse(CellRef a, Env* e) {
    CellRef l = a->cons.car; CellRef r = nilC;' `
'CellRef bReverse(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef l = a->cons.car; CellRef r = nilC;' `
"F11h bReverse"

# ============ F12: single-arg math builtins ============
$math_fns = @(
    @("bSin", "sin"), @("bCos", "cos"), @("bTan", "tan"),
    @("bExp", "exp"), @("bLog", "log"), @("bSqrt", "sqrt"),
    @("bAbs", "fabs"), @("bFloor", "floor"), @("bCeil", "ceil"),
    @("bRound", "round")
)
foreach ($pair in $math_fns) {
    $fn = $pair[0]
    $op = $pair[1]
    $old = "CellRef $fn(CellRef a, Env* e)   { return mkNum($op(toD(a->cons.car))); }"
    $new = "CellRef $fn(CellRef a, Env* e)   { if (isNil(a) || a->type != T_CONS) { setErr(`"missing arg`"); return nilC; } return mkNum($op(toD(a->cons.car))); }"
    $content = Apply $content $old $new "F12 $fn"
}

# ============ F13: evalArgs loop ============
$content = Apply $content `
'CellRef evalArgs(CellRef args, Env* env) {
    if (isNil(args)) return nilC;
    return mkCons(eval(args->cons.car, env), evalArgs(args->cons.cdr, env));
}' `
'CellRef evalArgs(CellRef args, Env* env) {
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
}' `
"F13 evalArgs loop"

# ============ F15: Maxima URL ============
$content = Apply $content `
'        char url[128] = "http://example.com/maxima?expr=";' `
'        char url[128] = "http://YOUR-PROXY-HOST/maxima?expr=";' `
"F15 Maxima URL"

# ============ F7: printCell rewrite ============
$content = Apply $content `
'void printCell(CellRef c, char* buf, int& p, int max) {
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
}' `
'void printCell(CellRef c, char* buf, int& p, int max) {
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
        putc1('"'"'('"'"');
        bool first = true;
        while (!isNil(c) && c->type == T_CONS) {
            if (!first) putc1('"'"' '"'"');
            printCell(c->cons.car, buf, p, max);
            first = false;
            c = c->cons.cdr;
        }
        if (!isNil(c)) { put(" . "); printCell(c, buf, p, max); }
        putc1('"'"')'"'"');
    }
}' `
"F7 printCell rewrite"

# ============ write back ============
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak8" -ForegroundColor Cyan