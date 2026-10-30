$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\pomas_0_92.ino"

Copy-Item $path "$path.bak2" -Force
Write-Host "Backup created: $path.bak2" -ForegroundColor Cyan

$content = [System.IO.File]::ReadAllText($path)
$hasCRLF = $content.Contains("`r`n")
$content = $content.Replace("`r`n", "`n")

$old = @'
        d.fillRect(0, LCD_H - FTR_H - 17, LCD_W, 16, C_PANEL);
        d.drawFastHLine(0, LCD_H - FTR_H - 17, LCD_W, C_ACC);
        d.setTextColor(C_FG, C_PANEL);
        d.drawString(input, 4, LCD_H - FTR_H - 14);
        int cx = 4 + textWidth(input);
        if (cx > LCD_W - 6) cx = LCD_W - 6;
        d.drawFastVLine(cx, LCD_H - FTR_H - 16, 14, C_ACC);
'@

$new = @'
        d.fillRect(0, LCD_H - FTR_H - 17, LCD_W, 16, C_PANEL);
        d.drawFastHLine(0, LCD_H - FTR_H - 17, LCD_W, C_ACC);
        d.setTextColor(C_FG, C_PANEL);
        const int fieldX = 4;
        const int fieldW = LCD_W - 8;
        const int maxChars = fieldW / 6;
        int len = strlen(input);
        int startCh = (len > maxChars) ? (len - maxChars) : 0;
        const char* visible = input + startCh;
        d.drawString(visible, fieldX, LCD_H - FTR_H - 14);
        int cx = fieldX + textWidth(visible);
        if (cx > LCD_W - 6) cx = LCD_W - 6;
        d.drawFastVLine(cx, LCD_H - FTR_H - 16, 14, C_ACC);
'@

$oldN = $old.Replace("`r`n", "`n")
$newN = $new.Replace("`r`n", "`n")
$result = $content.Replace($oldN, $newN)

if ($result -eq $content) {
    Write-Host "NG: drawScreen patch failed (text not found)" -ForegroundColor Red
} else {
    Write-Host "OK: drawScreen patch applied" -ForegroundColor Green
    if ($hasCRLF) { $result = $result.Replace("`n", "`r`n") }
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($path, $result, $utf8NoBom)
}