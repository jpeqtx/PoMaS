$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak7" -Force
Write-Host "Backup: $path.bak7" -ForegroundColor Cyan

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

# ========== M1: mk* gc removal ==========
$content = Apply $content `
'inline CellRef mkNum(double v) { CellRef c = alloc(); if (!c) { gc(); c = alloc(); } if (!c) return nilC; c->type = T_NUM; c->num = v; return c; }' `
'inline CellRef mkNum(double v) { CellRef c = alloc(); if (!c) return nilC; c->type = T_NUM; c->num = v; return c; }' `
"M1a mkNum"

$content = Apply $content `
'    CellRef c = alloc(); if (!c) { gc(); c = alloc(); } if (!c) return nilC;
    c->type = T_SYM;' `
'    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_SYM;' `
"M1b mkSym"

$content = Apply $content `
'    CellRef c = alloc(); if (!c) { gc(); c = alloc(); } if (!c) return nilC;
    c->type = T_STR;' `
'    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_STR;' `
"M1c mkStr"

$content = Apply $content `
'    CellRef c = alloc(); if (!c) { gc(); c = alloc(); } if (!c) return nilC;
    c->type = T_CONS; c->cons.car = a; c->cons.cdr = d; return c;' `
'    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_CONS; c->cons.car = a; c->cons.cdr = d; return c;' `
"M1d mkCons"

$content = Apply $content `
'    CellRef c = alloc(); if (!c) { gc(); c = alloc(); } if (!c) return nilC;
    c->type = T_BUILTIN; c->builtin = fn; return c;' `
'    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_BUILTIN; c->builtin = fn; return c;' `
"M1e mkBuiltin"

# ========== M5: eval form guards ==========
$content = Apply $content `
'        if (!strcmp(n, "quote"))  return args->cons.car;' `
'        if (!strcmp(n, "quote")) {
            if (isNil(args) || args->type != T_CONS) { setErr("quote: no arg"); return nilC; }
            return args->cons.car;
        }' `
"M5a quote"

$content = Apply $content `
'        if (!strcmp(n, "if")) {
            CellRef c = eval(args->cons.car, env);' `
'        if (!strcmp(n, "if")) {
            if (isNil(args) || args->type != T_CONS) { setErr("if: bad args"); return nilC; }
            CellRef c = eval(args->cons.car, env);' `
"M5b if"

$content = Apply $content `
'        if (!strcmp(n, "not")) { CellRef r = eval(args->cons.car, env); return isNil(r) ? trueC : nilC; }' `
'        if (!strcmp(n, "not")) {
            if (isNil(args) || args->type != T_CONS) return trueC;
            CellRef r = eval(args->cons.car, env);
            return isNil(r) ? trueC : nilC;
        }' `
"M5c not"

$content = Apply $content `
'        if (!strcmp(n, "define") || !strcmp(n, "setq")) {
            CellRef target = args->cons.car;
            if (target->type == T_SYM) {
                CellRef v = eval(args->cons.cdr->cons.car, env);
                env->set(target->sym, v);
                return v;
            }
            const char* name = target->cons.car->sym;' `
'        if (!strcmp(n, "define") || !strcmp(n, "setq")) {
            if (isNil(args) || args->type != T_CONS) { setErr("define: bad args"); return nilC; }
            CellRef target = args->cons.car;
            if (!target) { setErr("define: no target"); return nilC; }
            if (target->type == T_SYM) {
                if (isNil(args->cons.cdr)) { setErr("define: no value"); return nilC; }
                CellRef v = eval(args->cons.cdr->cons.car, env);
                env->set(target->sym, v);
                return v;
            }
            if (target->type != T_CONS || !target->cons.car || target->cons.car->type != T_SYM) {
                setErr("define: bad target"); return nilC;
            }
            const char* name = target->cons.car->sym;' `
"M5d define/setq"

$content = Apply $content `
'        if (!strcmp(n, "lambda")) {
            CellRef lam = alloc();' `
'        if (!strcmp(n, "lambda")) {
            if (isNil(args) || args->type != T_CONS) { setErr("lambda: bad args"); return nilC; }
            CellRef lam = alloc();' `
"M5e lambda"

$content = Apply $content `
'        if (!strcmp(n, "let")) {
            CellRef bindings = args->cons.car;' `
'        if (!strcmp(n, "let")) {
            if (isNil(args) || args->type != T_CONS) { setErr("let: bad args"); return nilC; }
            CellRef bindings = args->cons.car;' `
"M5f let"

$content = Apply $content `
'        if (!strcmp(n, "while")) {
            CellRef test = args->cons.car;' `
'        if (!strcmp(n, "while")) {
            if (isNil(args) || args->type != T_CONS) { setErr("while: bad args"); return nilC; }
            CellRef test = args->cons.car;' `
"M5g while"

# ========== Arithmetic builtin guards ==========
$content = Apply $content `
'CellRef bMod(CellRef a, Env* e) { return mkNum(fmod(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }' `
'CellRef bMod(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkNum(fmod(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }' `
"bMod guard"

$content = Apply $content `
'CellRef bLT(CellRef a, Env* e) { return (toD(a->cons.car) <  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
'CellRef bLT(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
"bLT guard"

$content = Apply $content `
'CellRef bGT(CellRef a, Env* e) { return (toD(a->cons.car) >  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
'CellRef bGT(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
"bGT guard"

$content = Apply $content `
'CellRef bLE(CellRef a, Env* e) { return (toD(a->cons.car) <= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
'CellRef bLE(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
"bLE guard"

$content = Apply $content `
'CellRef bGE(CellRef a, Env* e) { return (toD(a->cons.car) >= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
'CellRef bGE(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }' `
"bGE guard"

$content = Apply $content `
'CellRef bEq(CellRef a, Env* e) {
    CellRef x = a->cons.car, y = a->cons.cdr->cons.car;' `
'CellRef bEq(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef x = a->cons.car, y = a->cons.cdr->cons.car;' `
"bEq guard"

$content = Apply $content `
'CellRef bCons(CellRef a, Env* e) { return mkCons(a->cons.car, a->cons.cdr->cons.car); }' `
'CellRef bCons(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkCons(a->cons.car, a->cons.cdr->cons.car); }' `
"bCons guard"

$content = Apply $content `
'CellRef bNth(CellRef a, Env* e) {
    int n = (int)toD(a->cons.car);
    CellRef l = a->cons.cdr->cons.car;' `
'CellRef bNth(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    int n = (int)toD(a->cons.car);
    CellRef l = a->cons.cdr->cons.car;' `
"bNth guard"

$content = Apply $content `
'CellRef bPow(CellRef a, Env* e)   { return mkNum(pow(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }' `
'CellRef bPow(CellRef a, Env* e)   { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkNum(pow(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }' `
"bPow guard"

# ========== Higher-order builtin guards ==========
$content = Apply $content `
'CellRef bMap(CellRef a, Env* e) {
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;' `
'CellRef bMap(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;' `
"bMap guard"

$content = Apply $content `
'CellRef bFilter(CellRef a, Env* e) {
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;' `
'CellRef bFilter(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;' `
"bFilter guard"

$content = Apply $content `
'CellRef bReduce(CellRef a, Env* e) {
    CellRef fn   = a->cons.car;
    CellRef acc  = a->cons.cdr->cons.car;
    CellRef lst  = a->cons.cdr->cons.cdr->cons.car;' `
'CellRef bReduce(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr) || isNil(a->cons.cdr->cons.cdr)) return nilC;
    CellRef fn   = a->cons.car;
    CellRef acc  = a->cons.cdr->cons.car;
    CellRef lst  = a->cons.cdr->cons.cdr->cons.car;' `
"bReduce guard"

$content = Apply $content `
'CellRef bApply(CellRef a, Env* e) {
    CellRef fn   = a->cons.car;
    CellRef args = a->cons.cdr->cons.car;' `
'CellRef bApply(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn   = a->cons.car;
    CellRef args = a->cons.cdr->cons.car;' `
"bApply guard"

# ========== bSave cdr check ==========
$content = Apply $content `
'    File f = SD.open(path, FILE_WRITE);
    if (!f) return nilC;
    CellRef val = a->cons.cdr->cons.car;' `
'    File f = SD.open(path, FILE_WRITE);
    if (!f) return nilC;
    if (isNil(a->cons.cdr) || a->cons.cdr->type != T_CONS) { f.close(); return nilC; }
    CellRef val = a->cons.cdr->cons.car;' `
"bSave cdr check"

# ========== M3: chi2 plotPDF ==========
$content = Apply $content `
'        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = Dist::chiSquareInv(atof(b), df);
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "x = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        }
    }' `
'        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = Dist::chiSquareInv(atof(b), df);
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "x = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        } else {
            static double sdf; sdf = (double)df;
            plotPDF(0.01, df * 3, [](double x, double* p){ return Dist::chiSquarePDF(x, (int)p[0]); }, &sdf);
        }
    }' `
"M3 chi2 plotPDF"

# ========== M3: fDist plotPDF ==========
$content = Apply $content `
'        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = Dist::FInv(atof(b), d1, d2);
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "F = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        }
    }' `
'        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = Dist::FInv(atof(b), d1, d2);
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "F = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        } else {
            static double sp[2]; sp[0] = (double)d1; sp[1] = (double)d2;
            plotPDF(0.01, 3, [](double x, double* p){ return Dist::FPDF(x, (int)p[0], (int)p[1]); }, sp);
        }
    }' `
"M3 fDist plotPDF"

# ========== M7: CsvStream input check ==========
$content = Apply $content `
'        char rxs[8]="0", rys[8]="0";
        DM.inputLine("Reg X col(1.. ,0=skip):", rxs, sizeof(rxs));
        DM.inputLine("Reg Y col(1..):", rys, sizeof(rys));' `
'        char rxs[8]="0", rys[8]="0";
        if (!DM.inputLine("Reg X col(1.. ,0=skip):", rxs, sizeof(rxs))) return;
        if (!DM.inputLine("Reg Y col(1..):", rys, sizeof(rys))) return;' `
"M7 CsvStream input check"

# ========== M9: chiIndep total=0 ==========
$content = Apply $content `
'                total  += O[i*c+j];
            }
        double chi = 0;' `
'                total  += O[i*c+j];
            }
        if (total <= 0) { DM.notify("Total = 0", C_ERR); return; }
        double chi = 0;' `
"M9 chiIndep total guard"

# ========== L3: plotYofX off-by-one ==========
$content = Apply $content `
'            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin)*gh);
            if (prevOk && abs(prevY - py) < gh) d.drawLine(prevX, prevY, gx + px, py, col);' `
'            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin)*(gh-1));
            if (prevOk && abs(prevY - py) < gh) d.drawLine(prevX, prevY, gx + px, py, col);' `
"L3 plotYofX off-by-one"

# ========== L6: DM::notify clip ==========
$content = Apply $content `
'        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        d.drawString(msg, LCD_W/2 + 2, y + 12);
        d.setTextDatum(top_left);' `
'        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        char clipMsg[64];
        int maxCh = (w - 24) / 6;
        if (maxCh > 63) maxCh = 63;
        if (maxCh < 1) maxCh = 1;
        strncpy(clipMsg, msg, maxCh); clipMsg[maxCh] = 0;
        d.drawString(clipMsg, LCD_W/2 + 2, y + 12);
        d.setTextDatum(top_left);' `
"L6 notify clip"

# ========== L7: DM::confirm clip ==========
$content = Apply $content `
'        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        d.drawString(msg, LCD_W/2, y + 16);' `
'        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        char clipCfm[64];
        int maxCc = (w - 24) / 6;
        if (maxCc > 63) maxCc = 63;
        if (maxCc < 1) maxCc = 1;
        strncpy(clipCfm, msg, maxCc); clipCfm[maxCc] = 0;
        d.drawString(clipCfm, LCD_W/2, y + 16);' `
"L7 confirm clip"

# ========== editRange tstep guard ==========
$content = Apply $content `
'            snprintf(b, sizeof(b), "%g", tstep);if (!DM.inputLine("tstep:", b, sizeof(b))) return; tstep = atof(b);' `
'            snprintf(b, sizeof(b), "%g", tstep);if (!DM.inputLine("tstep:", b, sizeof(b))) return; tstep = atof(b);
            if (tstep <= 0) tstep = 0.05;' `
"editRange tstep guard"

# ========== MetaAnalysis v>0 ==========
$content = Apply $content `
'            if (!DM.inputLine(p, b, sizeof(b))) return 0; v[i] = atof(b);
            se[i] = sqrt(v[i]);' `
'            if (!DM.inputLine(p, b, sizeof(b))) return 0; v[i] = atof(b);
            if (v[i] <= 0) { DM.notify("var > 0 required", C_ERR); return 0; }
            se[i] = sqrt(v[i]);' `
"MetaAnalysis v>0"

# ========== MetaAnalysis egger K<3 ==========
$content = Apply $content `
'    void egger(int K) {
        double sx = 0, sy = 0, sxx = 0, sxy = 0;' `
'    void egger(int K) {
        if (K < 3) { DM.notify("Egger needs K>=3", C_ERR); return; }
        double sx = 0, sy = 0, sxx = 0, sxy = 0;' `
"egger K<3"

# ========== StatStrata cochranQ v>0 ==========
$content = Apply $content `
'            strcpy(b, "1"); if (!DM.inputLine(p, b, sizeof(b))) return; v[i] = atof(b);
        }
        double sw = 0, swy = 0;' `
'            strcpy(b, "1"); if (!DM.inputLine(p, b, sizeof(b))) return; v[i] = atof(b);
            if (v[i] <= 0) { DM.notify("var > 0 required", C_ERR); return; }
        }
        double sw = 0, swy = 0;' `
"cochranQ v>0"

# ========== anova1way g[6] -> g[STAT_LISTS] ==========
$content = Apply $content `
'    void anova1way() {
        int g[6], gN = 0, total = 0;' `
'    void anova1way() {
        int g[STAT_LISTS], gN = 0, total = 0;' `
"anova1way buffer"

# ========== kruskalWallis g[6] -> g[STAT_LISTS] ==========
$content = Apply $content `
'    void kruskalWallis() {
        int g[6], gN = 0, total = 0;' `
'    void kruskalWallis() {
        int g[STAT_LISTS], gN = 0, total = 0;' `
"kruskalWallis buffer"

# ========== matOps case7 er/ei init ==========
$content = Apply $content `
'                    double er[10], ei[10];
                    Mat::qrEig(A, G.matAr, er, ei);' `
'                    double er[10] = {0}, ei[10] = {0};
                    Mat::qrEig(A, G.matAr, er, ei);' `
"matOps case7 er/ei init"

# ========== write back ==========
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak7" -ForegroundColor Cyan