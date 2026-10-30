$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak6" -Force
Write-Host "Backup created: $path.bak6" -ForegroundColor Cyan

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

# ---- ESC cascade fix (pollKeys cooldown) ----
$oldESC = @'
    inline bool pollKeys() {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            gLastInputT = millis();
            return true;
        }
        return false;
    }
'@
$newESC = @'
    inline bool pollKeys() {
        M5Cardputer.update();
        static bool escCooldown = false;
        if (escCooldown) {
            if (!M5Cardputer.Keyboard.isKeyPressed('`')) {
                escCooldown = false;
                delay(50);
            }
            return false;
        }
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                escCooldown = true;
            }
            gLastInputT = millis();
            return true;
        }
        return false;
    }
'@
$content = Apply $content $oldESC $newESC "ESC cascade fix"

# ---- H2: E becomes variable, e stays Euler, add euler alias ----
$oldH2 = @'
        if (!strcmp(name, "e") || !strcmp(name, "E"))                          return M_E;
'@
$newH2 = @'
        if (!strcmp(name, "e") || !strcasecmp(name, "euler"))                  return M_E;
'@
$content = Apply $content $oldH2 $newH2 "H2 E as variable + euler alias"

# ---- H9a: Graph menu items (add Square aspect) ----
$oldH9a = @'
            const char* items[] = {
                "Plot current functions","Edit functions",
                "Plot type (y/parametric/polar/implicit/ineq/seq/3D)",
                "Range / Window","Standard window","Trig window",
                "G-Solve (root/extrema/intersect)","Tangent / Normal at x",
                "Save plot as BMP","Back"
            };
            int s = DM.menu("Graph", items, 10);
            if (s < 0 || s == 9) return;
            switch (s) {
                case 0: plot(); break;
                case 1: editFns(); break;
                case 2: pickType(); break;
                case 3: editRange(); break;
                case 4: stdWindow(); break;
                case 5: trigWindow(); break;
                case 6: gsolveMenu(); break;
                case 7: tanNormal(); break;
                case 8: { plot(false); DM.saveBMPAuto("graph"); } break;
            }
'@
$newH9a = @'
            const char* items[] = {
                "Plot current functions","Edit functions",
                "Plot type (y/parametric/polar/implicit/ineq/seq/3D)",
                "Range / Window","Standard window","Trig window",
                "Square aspect (1:1)",
                "G-Solve (root/extrema/intersect)","Tangent / Normal at x",
                "Save plot as BMP","Back"
            };
            int s = DM.menu("Graph", items, 11);
            if (s < 0 || s == 10) return;
            switch (s) {
                case 0: plot(); break;
                case 1: editFns(); break;
                case 2: pickType(); break;
                case 3: editRange(); break;
                case 4: stdWindow(); break;
                case 5: trigWindow(); break;
                case 6: squareAspectFn(); break;
                case 7: gsolveMenu(); break;
                case 8: tanNormal(); break;
                case 9: { plot(false); DM.saveBMPAuto("graph"); } break;
            }
'@
$content = Apply $content $oldH9a $newH9a "H9a Graph menu"

# ---- H9b: add squareAspectFn method ----
$oldH9b = @'
    void stdWindow()  { xmin=-10; xmax=10; ymin=-10; ymax=10; tmin=0; tmax=6.28; tstep=0.05; DM.notify("Std", C_OK); }
'@
$newH9b = @'
    void stdWindow()  { xmin=-10; xmax=10; ymin=-10; ymax=10; tmin=0; tmax=6.28; tstep=0.05; DM.notify("Std", C_OK); }
    void squareAspectFn() {
        double cx = (xmin + xmax) / 2;
        double cy = (ymin + ymax) / 2;
        double xSpan = xmax - xmin;
        double gh = LCD_H - HDR_H - FTR_H;
        double ySpan = xSpan * gh / LCD_W;
        xmin = cx - xSpan/2;
        xmax = cx + xSpan/2;
        ymin = cy - ySpan/2;
        ymax = cy + ySpan/2;
        DM.notify("Square aspect set", C_OK);
    }
'@
$content = Apply $content $oldH9b $newH9b "H9b squareAspectFn"

# ---- 書き戻し ----
if ($hasCRLF) { $content = $content.Replace("`n", "`r`n") }
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host ""
Write-Host "Done. Backup: $path.bak6" -ForegroundColor Cyan