$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

# バックアップ
Copy-Item $path "$path.bak" -Force
Write-Host "Backup created: $path.bak" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

function Apply($c, $old, $new, $label) {
    $o = $old.Replace("`r`n", "`n")
    $n = $new.Replace("`r`n", "`n")
    $r = $c.Replace($o, $n)
    if ($r -eq $c) {
        Write-Host ("NG: " + $label) -ForegroundColor Red
    } else {
        Write-Host ("OK: " + $label) -ForegroundColor Green
    }
    return $r
}

# ---- 1-A: allocEnv ----
$oldA = @'
static Env globalEnv;
'@
$newA = @'
static Env* envList[128];
static int  envN = 0;

Env* allocEnv(Env* parent) {
    if (envN >= 128) return nullptr;
    Env* e = new Env();
    e->parent = parent;
    envList[envN++] = e;
    return e;
}

static Env globalEnv;
'@
$content = Apply $content $oldA $newA "1-A allocEnv"

# ---- 1-B: applyFn (open) ----
$oldB = @'
        Env* newEnv = new Env();
        newEnv->parent = fn->lambda.env;
'@
$newB = @'
        Env* newEnv = allocEnv(fn->lambda.env);
        if (!newEnv) { setErr("Env limit"); return nilC; }
'@
$content = Apply $content $oldB $newB "1-B applyFn open"

# ---- 1-B: applyFn (delete) ----
$oldB2 = @'
        while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
        delete newEnv;
        return r;
'@
$newB2 = @'
        while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
        return r;
'@
$content = Apply $content $oldB2 $newB2 "1-B applyFn delete"

# ---- 1-C: let (open) ----
$oldC = @'
            Env* newEnv = new Env();
            newEnv->parent = env;
'@
$newC = @'
            Env* newEnv = allocEnv(env);
            if (!newEnv) { setErr("Env limit"); return nilC; }
'@
$content = Apply $content $oldC $newC "1-C let open"

# ---- 1-C: let (delete) ----
$oldC2 = @'
            while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
            delete newEnv;
            return r;
'@
$newC2 = @'
            while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
            return r;
'@
$content = Apply $content $oldC2 $newC2 "1-C let delete"

# ---- 1-D: resetPool ----
$oldD = @'
    for (int i = 0; i < globalEnv.n; i++) free(globalEnv.e[i].name);
    globalEnv.n = 0;
    initEnv();
}
'@
$newD = @'
    for (int i = 0; i < globalEnv.n; i++) free(globalEnv.e[i].name);
    globalEnv.n = 0;
    for (int i = 0; i < envN; i++) {
        for (int j = 0; j < envList[i]->n; j++) free(envList[i]->e[j].name);
        delete envList[i];
    }
    envN = 0;
    initEnv();
}
'@
$content = Apply $content $oldD $newD "1-D resetPool"

# ---- 2-A: inputLine ----
$oldE = @'
            d.fillRect(6, yy + 14, w - 4, 16, C_PANEL);
            d.setTextColor(C_FG, C_PANEL);
            d.drawString(buf, 8, yy + 17);
            int cx = 8 + textWidth(buf);
            if (cx > w - 2) cx = w - 2;
            if (blink) d.fillRect(cx, yy + 16, 2, 12, C_ACC);
'@
$newE = @'
            const int fieldX = 8;
            const int fieldW = w - 12;
            const int maxChars = fieldW / 6;
            int startCh = (cur > maxChars) ? (cur - maxChars) : 0;
            const char* visible = buf + startCh;
            d.fillRect(6, yy + 14, w - 4, 16, C_PANEL);
            d.setTextColor(C_FG, C_PANEL);
            d.drawString(visible, fieldX, yy + 17);
            int cx = fieldX + textWidth(visible);
            if (cx > LCD_W - 6) cx = LCD_W - 6;
            if (blink) d.fillRect(cx, yy + 16, 2, 12, C_ACC);
'@
$content = Apply $content $oldE $newE "2-A inputLine"

# ---- 2-B: repl ----
$oldF = @'
            d.drawString("> ", 4, LCD_H - FTR_H - 14);
            d.drawString(input, 16, LCD_H - FTR_H - 14);
            int cur = strlen(input);
            d.drawFastVLine(16 + textWidth(input), LCD_H - FTR_H - 16, 13, C_ACC);
'@
$newF = @'
            d.drawString("> ", 4, LCD_H - FTR_H - 14);
            int cur = strlen(input);
            const int fieldX = 16;
            const int fieldW = LCD_W - fieldX - 4;
            const int maxChars = fieldW / 6;
            int startCh = (cur > maxChars) ? (cur - maxChars) : 0;
            const char* visible = input + startCh;
            d.drawString(visible, fieldX, LCD_H - FTR_H - 14);
            int cx = fieldX + textWidth(visible);
            if (cx > LCD_W - 4) cx = LCD_W - 4;
            d.drawFastVLine(cx, LCD_H - FTR_H - 16, 13, C_ACC);
'@
$content = Apply $content $oldF $newF "2-B repl"

# ---- 書き戻し ----
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak" -ForegroundColor Cyan