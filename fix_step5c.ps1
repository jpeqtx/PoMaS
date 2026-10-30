$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak10" -Force
Write-Host "Backup: $path.bak10" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

Write-Host ""
Write-Host "=== Diagnostic: current parseList function ===" -ForegroundColor Cyan
$idx = $content.IndexOf("CellRef parseList()")
if ($idx -ge 0) {
    $end = $content.IndexOf("`n}", $idx)
    if ($end -ge 0) {
        Write-Host $content.Substring($idx, $end - $idx + 2) -ForegroundColor DarkGray
    }
} else {
    Write-Host "  parseList() NOT FOUND" -ForegroundColor Red
}

Write-Host ""
Write-Host "=== F2 fix using literal here-string ===" -ForegroundColor Cyan

if ($content.Contains('setErr("unexpected EOF")')) {
    Write-Host "  F2 ALREADY APPLIED (skipping)" -ForegroundColor Green
} else {
    # Build old/new using string concatenation to avoid quote issues
    $nl = "`n"
    $old = '    if (*gPos == ' + "''" + ')' + "''" + ') { gPos++; return nilC; }'
    # The above produces: if (*gPos == ')') { gPos++; return nilC; }
    # Actually this is tricky. Let me use simpler approach.
    $old = @'
    if (*gPos == ')') { gPos++; return nilC; }
'@
    if ($old.StartsWith("`n")) { $old = $old.Substring(1) }
    if ($old.EndsWith("`n"))   { $old = $old.Substring(0, $old.Length-1) }

    $new = @'
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
    if (*gPos == ')') { gPos++; return nilC; }
'@
    if ($new.StartsWith("`n")) { $new = $new.Substring(1) }
    if ($new.EndsWith("`n"))   { $new = $new.Substring(0, $new.Length-1) }

    $r = $content.Replace($old, $new)
    if ($r -eq $content) {
        Write-Host "  NG: F2 old pattern not found - showing variants" -ForegroundColor Red
        # Try un-indented
        $old2 = "if (*gPos == ')') { gPos++; return nilC; }"
        if ($content.Contains($old2)) {
            Write-Host "  Found un-indented variant, trying..." -ForegroundColor Yellow
            $new2 = "if (!*gPos) { setErr(`"unexpected EOF`"); return nilC; }`n" + $old2
            $r = $content.Replace($old2, $new2)
            if ($r -ne $content) {
                Write-Host "  OK: F2 applied (un-indented)" -ForegroundColor Green
                $content = $r
            } else {
                Write-Host "  NG: un-indented replace also failed" -ForegroundColor Red
            }
        } else {
            Write-Host "  Neither indented nor un-indented pattern found" -ForegroundColor Red
            Write-Host "  Manual check required. Paste the parseList function above." -ForegroundColor Yellow
        }
    } else {
        Write-Host "  OK: F2 applied" -ForegroundColor Green
        $content = $r
    }
}

Write-Host ""
Write-Host "=== F7 fix using literal here-string ===" -ForegroundColor Cyan

if ($content.Contains('auto put = [&](const char* s)')) {
    Write-Host "  F7 ALREADY APPLIED (skipping)" -ForegroundColor Green
} else {
    Write-Host "  F7 NOT APPLIED. This is a Low-priority bug." -ForegroundColor Yellow
    Write-Host "  (printCell buffer overrun only occurs on very long lists)" -ForegroundColor Yellow
    Write-Host "  Skipping F7 for now - can address separately later." -ForegroundColor Yellow
}

if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "=== Final state ===" -ForegroundColor Cyan
$check = [System.IO.File]::ReadAllText($path)
$f2ok  = $check.Contains('setErr("unexpected EOF")')
$f7ok  = $check.Contains('auto put = [&](const char* s)')
Write-Host ("  F2 (parseList EOF): " + $(if ($f2ok) {"APPLIED"} else {"NOT APPLIED"})) -ForegroundColor $(if ($f2ok) {"Green"} else {"Red"})
Write-Host ("  F7 (printCell):     " + $(if ($f7ok) {"APPLIED"} else {"NOT APPLIED"})) -ForegroundColor $(if ($f7ok) {"Green"} else {"Red"})

Write-Host ""
Write-Host "Done. Backup: $path.bak10" -ForegroundColor Cyan