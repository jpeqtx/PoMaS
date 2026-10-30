$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak16" -Force
Write-Host "Backup: $path.bak16" -ForegroundColor Cyan

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

# ============ L1: swap hex/strtod order ============
$old = @'
        if (isdigit((unsigned char)c) || c == '.') {
            char* end;
            double v = strtod(_src + _pos, &end);
            _pos = end - _src;
            return v;
        }
        if (c == '0' && (_src[_pos+1] == 'x' || _src[_pos+1] == 'X')) {
            _pos += 2;
            unsigned long long v = 0;
            while (isxdigit((unsigned char)_src[_pos])) {
                char d = _src[_pos++];
                int n = (d <= '9') ? d - '0' : ((d|0x20) - 'a' + 10);
                v = v * 16 + n;
            }
            return (double)v;
        }
'@
$new = @'
        if (c == '0' && (_src[_pos+1] == 'x' || _src[_pos+1] == 'X')) {
            _pos += 2;
            unsigned long long v = 0;
            while (isxdigit((unsigned char)_src[_pos])) {
                char d = _src[_pos++];
                int n = (d <= '9') ? d - '0' : ((d|0x20) - 'a' + 10);
                v = v * 16 + n;
            }
            return (double)v;
        }
        if (isdigit((unsigned char)c) || c == '.') {
            char* end;
            double v = strtod(_src + _pos, &end);
            _pos = end - _src;
            return v;
        }
'@
$content = ApplyStr $content $old $new "L1 hex before strtod"

# ============ L2: remove dead condition in plotYofX ============
$old = '            if (prevOk && abs(prevY - py) < gh) d.drawLine(prevX, prevY, gx + px, py, col);'
$new = '            if (prevOk) d.drawLine(prevX, prevY, gx + px, py, col);'
$content = ApplyStr $content $old $new "L2 plotYofX dead condition"

# ============ L5: radical limit 99 -> 200 ============
$old = '    for (int rt = 2; rt < 100; rt++) {'
$new = '    for (int rt = 2; rt < 200; rt++) {'
$content = ApplyStr $content $old $new "L5 radical limit"

# ============ L8: header badge position clamp ============
$old = @'
        int bw = textWidth(bd);
        int cx = LCD_W - bw - 8;
'@
$new = @'
        int bw = textWidth(bd);
        int cx = LCD_W - bw - 8;
        if (cx < 4) cx = 4;
'@
$content = ApplyStr $content $old $new "L8 header badge clamp"

# ============ L13: scan limit 20 -> 30 ============
$old = '        if (n > 20) n = 20;'
$new = '        if (n > 30) n = 30;'
$content = ApplyStr $content $old $new "L13 scan limit"

# ============ M4: decisionTree -1 child display ============
$old = @'
            if (tree[i].feature < 0) snprintf(l, sizeof(l), "[%d] LEAF -> %.3f", i, tree[i].prediction);
            else snprintf(l, sizeof(l), "[%d] L%d <= %.3f -> [%d/%d]",
                          i, lst[tree[i].feature]+1, tree[i].threshold, tree[i].left, tree[i].right);
'@
$new = @'
            if (tree[i].feature < 0) snprintf(l, sizeof(l), "[%d] LEAF -> %.3f", i, tree[i].prediction);
            else if (tree[i].left < 0 || tree[i].right < 0) snprintf(l, sizeof(l), "[%d] L%d <= %.3f -> [--]",
                          i, lst[tree[i].feature]+1, tree[i].threshold);
            else snprintf(l, sizeof(l), "[%d] L%d <= %.3f -> [%d/%d]",
                          i, lst[tree[i].feature]+1, tree[i].threshold, tree[i].left, tree[i].right);
'@
$content = ApplyStr $content $old $new "M4 decisionTree -1 guard"

# ============ L12: qrEig convergence early break ============
$old = @'
    for (int it = 0; it < 200; it++) {
        double* Q = (double*)malloc(n*n*sizeof(double));
'@
$new = @'
    for (int it = 0; it < 200; it++) {
        double maxOff = 0;
        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                if (r != c && fabs(A[r*n+c]) > maxOff) maxOff = fabs(A[r*n+c]);
        if (maxOff < 1e-10) break;
        double* Q = (double*)malloc(n*n*sizeof(double));
'@
$content = ApplyStr $content $old $new "L12 qrEig convergence"

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
    "L1 hex before strtod"  = ($final.IndexOf("if (c == '0' && (_src[_pos+1] == 'x'") -lt $final.IndexOf("if (isdigit((unsigned char)c) || c == '.')"))
    "L2 no dead condition"  = (-not $final.Contains('if (prevOk && abs(prevY - py) < gh)'))
    "L5 rt < 200"           = $final.Contains('for (int rt = 2; rt < 200; rt++)')
    "L8 cx clamp"           = $final.Contains('if (cx < 4) cx = 4;')
    "L13 scan 30"           = $final.Contains('if (n > 30) n = 30;')
    "M4 [--] guard"         = $final.Contains('-> [--]')
    "L12 maxOff check"      = $final.Contains('if (maxOff < 1e-10) break;')
}
foreach ($kv in $checks.GetEnumerator()) {
    $color = if ($kv.Value) { "Green" } else { "Red" }
    $status = if ($kv.Value) { "OK" } else { "FAIL" }
    Write-Host "  $($kv.Key): $status" -ForegroundColor $color
}

Write-Host ""
Write-Host "Backup: $path.bak16" -ForegroundColor Cyan