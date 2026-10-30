$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak5" -Force
Write-Host "Backup created: $path.bak5" -ForegroundColor Cyan

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

# ---- H1: GState::resetAll 拡張 ----
$oldH1 = @'
    void resetAll()  { resetVars(); resetStat(); histN=0; rpnSp=0; ans=0; }
'@
$newH1 = @'
    void resetAll() {
        resetVars();
        resetStat();
        histN = 0; rpnSp = 0; ans = 0;
        ansComplex = false; ansC = Complex(0,0);
        ansFracValid = false;
        bankN = 0;
        for (int i = 0; i < ANS_BANK_MAX; i++) { bank[i] = 0; bankLabel[i][0] = 0; }
        for (int i = 0; i < VEC_MAX; i++) { vecA[i] = 0; vecB[i] = 0; }
        for (int i = 0; i < MAT_MAX*MAT_MAX; i++) { matA[i] = 0; matB[i] = 0; }
        matAr = matAc = matBr = matBc = 0;
        for (int i = 0; i < 4; i++) rpnStk[i] = 0;
    }
'@
$content = Apply $content $oldH1 $newH1 "H1 resetAll extended"

# ---- H3a: simple case 2 malloc ----
$oldH3a = @'
            double *lx = (double*)malloc(n*8), *ly = (double*)malloc(n*8);
            for (int i = 0; i < n; i++) { lx[i] = log(X[i]); ly[i] = log(Y[i]); }
'@
$newH3a = @'
            double *lx = (double*)malloc(n*8), *ly = (double*)malloc(n*8);
            if (!lx || !ly) { free(lx); free(ly); DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) { lx[i] = log(X[i]); ly[i] = log(Y[i]); }
'@
$content = Apply $content $oldH3a $newH3a "H3a case2 malloc"

# ---- H3b: simple case 3 malloc ----
$oldH3b = @'
            double *ly = (double*)malloc(n*8);
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]);
            double la, lb;
'@
$newH3b = @'
            double *ly = (double*)malloc(n*8);
            if (!ly) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]);
            double la, lb;
'@
$content = Apply $content $oldH3b $newH3b "H3b case3 malloc"

# ---- H3c: simple case 4 malloc ----
$oldH3c = @'
            double *lx = (double*)malloc(n*8);
            for (int i = 0; i < n; i++) lx[i] = log(X[i]);
            linfit(lx, Y, n, aa, bb, r2v);
'@
$newH3c = @'
            double *lx = (double*)malloc(n*8);
            if (!lx) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) lx[i] = log(X[i]);
            linfit(lx, Y, n, aa, bb, r2v);
'@
$content = Apply $content $oldH3c $newH3c "H3c case4 malloc"

# ---- H3d: simple case 5 malloc ----
$oldH3d = @'
            double *ly = (double*)malloc(n*8);
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]/(1-Y[i]));
            linfit(X, ly, n, aa, bb, r2v);
'@
$newH3d = @'
            double *ly = (double*)malloc(n*8);
            if (!ly) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]/(1-Y[i]));
            linfit(X, ly, n, aa, bb, r2v);
'@
$content = Apply $content $oldH3d $newH3d "H3d case5 malloc"

# ---- H4: multiple VIF solve failure ----
$oldH4 = @'
            if (Mat::gaussSolve(XX, pp, XY)) { /* XY now holds beta_i */ }
            double sst2 = 0, sse2 = 0;
'@
$newH4 = @'
            if (!Mat::gaussSolve(XX, pp, XY)) { VIF[i] = INFINITY; continue; }
            double sst2 = 0, sse2 = 0;
'@
$content = Apply $content $oldH4 $newH4 "H4 VIF solve failure"

# ---- H5: drawTraceMarker right-aligned ----
$oldH5 = @'
        d.fillRect(0, gy, LCD_W, 11, C_HDR);
        d.setTextColor(C_HDRTX, C_HDR);
        char l[40]; snprintf(l, sizeof(l), " x=%.4g  y=%.4g  F%d", xv, r.val, f+1);
        d.drawString(l, 2, gy + 1);
'@
$newH5 = @'
        char l[40]; snprintf(l, sizeof(l), "x=%.4g y=%.4g F%d", xv, r.val, f+1);
        int tw = textWidth(l) + 6;
        if (tw > LCD_W) tw = LCD_W;
        d.fillRect(LCD_W - tw, gy, tw, 11, C_HDR);
        d.setTextColor(C_HDRTX, C_HDR);
        d.drawString(l, LCD_W - tw + 3, gy + 1);
'@
$content = Apply $content $oldH5 $newH5 "H5 trace marker right"

# ---- H6: CsvStream h[24] -> h[40] ----
$oldH6 = @'
            char h[24]; snprintf(h,sizeof(h),"CSV %d/%d (rows %ld)", page+1, pages, rows);
'@
$newH6 = @'
            char h[40]; snprintf(h,sizeof(h),"CSV %d/%d (rows %ld)", page+1, pages, rows);
'@
$content = Apply $content $oldH6 $newH6 "H6 CsvStream h[40]"

# ---- H7: bitOpsMenu 16-base support ----
$oldH7 = @'
        char b[40] = "0";
        if (!DM.inputLine("a:", b, sizeof(b))) return;
        long long a = (long long)atof(b);
        long long bb = 0;
        if (s != 3) {
            strcpy(b, "0");
            if (!DM.inputLine("b:", b, sizeof(b))) return;
            bb = (long long)atof(b);
        }
'@
$newH7 = @'
        char b[40] = "0";
        if (!DM.inputLine("a:", b, sizeof(b))) return;
        long long a;
        if (b[0]=='0' && (b[1]=='x'||b[1]=='X')) a = strtoll(b+2, nullptr, 16);
        else if (b[0]=='0' && (b[1]=='b'||b[1]=='B')) a = strtoll(b+2, nullptr, 2);
        else if (b[0]=='0' && (b[1]=='o'||b[1]=='O')) a = strtoll(b+2, nullptr, 8);
        else a = (long long)atof(b);
        long long bb = 0;
        if (s != 3) {
            strcpy(b, "0");
            if (!DM.inputLine("b:", b, sizeof(b))) return;
            if (b[0]=='0' && (b[1]=='x'||b[1]=='X')) bb = strtoll(b+2, nullptr, 16);
            else if (b[0]=='0' && (b[1]=='b'||b[1]=='B')) bb = strtoll(b+2, nullptr, 2);
            else if (b[0]=='0' && (b[1]=='o'||b[1]=='O')) bb = strtoll(b+2, nullptr, 8);
            else bb = (long long)atof(b);
        }
'@
$content = Apply $content $oldH7 $newH7 "H7 bitOps 16base"

# ---- H8: matOps empty matrix guard ----
$oldH8 = @'
        int s = DM.menu("Matrix Op", items, 11);
        if (s < 0 || s == 10) return;
        auto pack = [](double* dst, double* src, int r, int c){
'@
$newH8 = @'
        int s = DM.menu("Matrix Op", items, 11);
        if (s < 0 || s == 10) return;
        if (G.matAr == 0 || G.matAc == 0) {
            DM.notify("Edit Matrix A first", C_WARN);
            return;
        }
        auto pack = [](double* dst, double* src, int r, int c){
'@
$content = Apply $content $oldH8 $newH8 "H8 matOps empty matrix guard"

# ---- 書き戻し ----
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak5" -ForegroundColor Cyan