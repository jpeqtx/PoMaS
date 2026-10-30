$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak4" -Force
Write-Host "Backup created: $path.bak4" -ForegroundColor Cyan

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

# ---- C1: DistBrowser::tDist int->double ----
$oldC1 = @'
            static int sdf; sdf = df;
            plotPDF(-5, 5, [](double x, double* p){ return Dist::tPDF(x, (int)p[0]); }, (double*)&sdf);
'@
$newC1 = @'
            static double sdf; sdf = (double)df;
            plotPDF(-5, 5, [](double x, double* p){ return Dist::tPDF(x, (int)p[0]); }, &sdf);
'@
$content = Apply $content $oldC1 $newC1 "C1 tDist type-punning"

# ---- C2: VecMatCalc::matOps case7 ----
$oldC2 = @'
                    double er[10], ei[10];
                    Mat::qrEig(G.matA, G.matAr, er, ei);
'@
$newC2 = @'
                    double er[10], ei[10];
                    Mat::qrEig(A, G.matAr, er, ei);
'@
$content = Apply $content $oldC2 $newC2 "C2 qrEig packed matrix"

# ---- C3: plotImplicit Y-flip ----
$oldC3 = @'
                if ((code & 1) != ((code >> 1) & 1)) edge(0, 1, cx0, cy1, cx1, cy1, pts[np].x, pts[np].y), np++;
                if (((code >> 1) & 1) != ((code >> 2) & 1)) edge(1, 2, cx1, cy1, cx1, cy0, pts[np].x, pts[np].y), np++;
                if (((code >> 2) & 1) != ((code >> 3) & 1)) edge(2, 3, cx1, cy0, cx0, cy0, pts[np].x, pts[np].y), np++;
                if (((code >> 3) & 1) != (code & 1)) edge(3, 0, cx0, cy0, cx0, cy1, pts[np].x, pts[np].y), np++;
'@
$newC3 = @'
                if ((code & 1) != ((code >> 1) & 1)) edge(0, 1, cx0, cy0, cx1, cy0, pts[np].x, pts[np].y), np++;
                if (((code >> 1) & 1) != ((code >> 2) & 1)) edge(1, 2, cx1, cy0, cx1, cy1, pts[np].x, pts[np].y), np++;
                if (((code >> 2) & 1) != ((code >> 3) & 1)) edge(2, 3, cx1, cy1, cx0, cy1, pts[np].x, pts[np].y), np++;
                if (((code >> 3) & 1) != (code & 1)) edge(3, 0, cx0, cy1, cx0, cy0, pts[np].x, pts[np].y), np++;
'@
$content = Apply $content $oldC3 $newC3 "C3 plotImplicit Y-flip"

# ---- C4: plotSeq N<1 -> N<2 ----
$oldC4 = @'
        int N = seqEnd - seqStart + 1;
        if (N < 1) return;
'@
$newC4 = @'
        int N = seqEnd - seqStart + 1;
        if (N < 2) return;
'@
$content = Apply $content $oldC4 $newC4 "C4 plotSeq N<2"

# ---- C5a: bSave type check ----
$oldC5a = @'
CellRef bSave(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->cons.car->type != T_STR) return nilC;
'@
$newC5a = @'
CellRef bSave(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef arg0 = a->cons.car;
    if (!arg0 || arg0->type != T_STR) return nilC;
'@
$content = Apply $content $oldC5a $newC5a "C5a bSave type check"

# ---- C5b: bLoad type check ----
$oldC5b = @'
CellRef bLoad(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->cons.car->type != T_STR) return nilC;
'@
$newC5b = @'
CellRef bLoad(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef arg0 = a->cons.car;
    if (!arg0 || arg0->type != T_STR) return nilC;
'@
$content = Apply $content $oldC5b $newC5b "C5b bLoad type check"

# ---- C6: geomInv p guard ----
$oldC6 = @'
inline int geomInv(double q, double p) {
    if (q <= 0) return 1;
    if (q >= 1) return 1000;
    return (int)ceil(log(1 - q) / log(1 - p));
}
'@
$newC6 = @'
inline int geomInv(double q, double p) {
    if (q <= 0) return 1;
    if (q >= 1) return 1000;
    if (p >= 1) return 1;
    if (p <= 0) return 1000;
    return (int)ceil(log(1 - q) / log(1 - p));
}
'@
$content = Apply $content $oldC6 $newC6 "C6 geomInv p guard"

# ---- 書き戻し ----
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak4" -ForegroundColor Cyan