// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  PoMaS (Pocket Mathematics Station) ver.0.92                          ║
// ║  Target : M5Stack Cardputer-Adv (ESP32-S3FN8, 8MB Flash)              ║
// ║  Author : YNGMT                                                       ║
// ║  Part  1 / 20 ─ 共通ヘッダ・型・GState・補助関数                        ║
// ╚═══════════════════════════════════════════════════════════════════════╝

#include <M5Cardputer.h>
#include <M5GFX.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include "splash_data.h"

#define POMAS_VER  "0.92"
#define POMAS_NAME "PoMaS"
#define POMAS_CRED "Created by YNGMT"

struct LawEntry {
    const char* name;
    const char* formula;
    const char* notes;
    const char* cat;
};

#define SD_CS    12
#define SD_MOSI  14
#define SD_SCK   40
#define SD_MISO  39

struct SdGuard {
    SdGuard() { M5Cardputer.Display.endWrite(); }
};

static const int LCD_W   = 240;
static const int LCD_H   = 135;
static const int HDR_H   = 16;
static const int FTR_H   = 14;
static const int BODY_X  = 2;

// ────────────────────────────────────────────────────────────────
//  PoMaS Paper UI v2 ─ 配色パレット (RGB565)
//  ・紙のような明るい背景 + グラファイト文字 + ミントのアクセント
//  ・ST7789V2 (240x135) での視認性を最優先
// ────────────────────────────────────────────────────────────────
#define C_BG       0xF7FE   // 背景 (最淡ミント紙)
#define C_PANEL    0xFFFF   // パネル (純白)
#define C_PANEL2   0xE7FE   // 副パネル (淡ミント、メニューの交互行用)
#define C_HDR      0xD7FC   // ヘッダ/フッタ バー
#define C_HDRTX    0x18E3   // バー上の主文字 (グラファイト)
#define C_HDRDIM   0x5D55   // バー上の副文字 (ミュート)
#define C_FG       0x18E3   // 本文 (高コントラスト)
#define C_DIM      0x6D96   // 副文字
#define C_ACC      0x34C6   // アクセント (フレッシュミント)
#define C_ACC2     0x23A4   // アクセント2 (深ミント)
#define C_OK       0x2C64   // 成功 (緑)
#define C_WARN     0xDDA0   // 警告 (琥珀)
#define C_ERR      0xC986   // エラー (柔らかい赤)
#define C_SEL_BG   0xCF5D   // 選択行背景 (ミント)
#define C_SEL_BAR  0x23A4   // 選択側帯 (深ミント)
#define C_GRID     0xC638   // グリッド線
#define C_LINE     0xE71C   // 罫線 (極淡、行セパレータ)
#define C_SHADOW   0xDEFB   // 影 (通知/ダイアログ用)
#define C_CHIP     0xDFFE   // チップ背景 (バッジ用)

#define S_BG       0x10A4
#define S_HDR      0x18E3
#define S_FG       0xEF5D
#define S_DIM      0x6B6D
#define S_ACC      0x07FF
#define S_ACC2     0xFD20

#define KEY_BS    0x08
#define IS_ESC    M5Cardputer.Keyboard.isKeyPressed('`')
#define IS_UP     M5Cardputer.Keyboard.isKeyPressed(';')
#define IS_DOWN   M5Cardputer.Keyboard.isKeyPressed('.')
#define IS_LEFT   M5Cardputer.Keyboard.isKeyPressed(',')
#define IS_RIGHT  M5Cardputer.Keyboard.isKeyPressed('/')

#define LIST_MAX        999
#define HIST_MAX        100
#define UNDO_MAX        20
#define VAR_COUNT       10
#define MAT_MAX         10
#define VEC_MAX         10
#define STAT_LISTS      8
#define ANS_BANK_MAX    20
#define MULTIREG_MAX    5
#define META_MAX        50
#define SURVIVAL_MAX    500
#define OUTBREAK_MAX    100
#define HCLUST_MAX      200
#define KMEANS_MAX      500
#define FA_MAX_VAR      10
#define INTEG_MAX_N     1000
#define PRIME_LIMIT     500000
#define SS_ROWS         45
#define SS_COLS         26
#define ULISP_CELLS     2500

enum AppMode   { MODE_MENU=0, MODE_CALC, MODE_CAS, MODE_STAT, MODE_SCIREF, MODE_GRAPH };
enum AngleMode { DEG=0, RAD=1, GRAD=2 };
enum DispMode  { DISP_NORM=0, DISP_FIX, DISP_SCI, DISP_ENG };
enum BaseMode  { BASE_DEC=10, BASE_HEX=16, BASE_OCT=8, BASE_BIN=2 };
enum VarScale  { SCALE_RATIO=0, SCALE_INTERVAL, SCALE_ORDINAL, SCALE_NOMINAL };
enum InputMode { INPUT_ALG=0, INPUT_RPN };
enum TestSide  { TS_TWO=0, TS_UPPER, TS_LOWER };

struct Fraction {
    long long num;
    long long den;
    Fraction() : num(0), den(1) {}
    Fraction(long long n) : num(n), den(1) {}
    Fraction(long long n, long long d) {
        if (d == 0) { num = 0; den = 1; return; }
        if (d < 0) { n = -n; d = -d; }
        num = n; den = d;
        reduce();
    }
    static long long gcdLL(long long a, long long b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b) { long long t = a % b; a = b; b = t; }
        return a ? a : 1;
    }
    void reduce() {
        if (den < 0) { num = -num; den = -den; }
        long long g = gcdLL(num, den);
        if (g > 1) { num /= g; den /= g; }
        if (num == 0) den = 1;
    }
    Fraction operator+(const Fraction& o) const { return Fraction(num*o.den + o.num*den, den*o.den); }
    Fraction operator-(const Fraction& o) const { return Fraction(num*o.den - o.num*den, den*o.den); }
    Fraction operator*(const Fraction& o) const { return Fraction(num*o.num, den*o.den); }
    Fraction operator/(const Fraction& o) const { return Fraction(num*o.den, den*o.num); }
    bool     operator==(const Fraction& o) const { return num*o.den == o.num*den; }
    Fraction neg() const { return Fraction(-num, den); }
    double   toDouble() const { return (double)num / (double)den; }
    void toString(char* out, int n, bool mixed = false) const {
        if (den == 1) { snprintf(out, n, "%lld", num); return; }
        if (mixed && (num >= den || num <= -den)) {
            long long w = num / den;
            long long r = num % den; if (r < 0) r = -r;
            snprintf(out, n, "%lld %lld/%lld", w, r, den);
        } else {
            snprintf(out, n, "%lld/%lld", num, den);
        }
    }
    static Fraction fromDouble(double v, double tol = 1e-9, long long maxDen = 1000000LL) {
        if (isnan(v) || isinf(v)) return Fraction(0,1);
        bool neg = v < 0; if (neg) v = -v;
        long long h0=0, h1=1, k0=1, k1=0;
        double x = v;
        for (int i = 0; i < 30; i++) {
            long long a = (long long)floor(x);
            long long h2 = a*h1 + h0;
            long long k2 = a*k1 + k0;
            if (k2 > maxDen) break;
            h0 = h1; h1 = h2;
            k0 = k1; k1 = k2;
            double diff = x - a;
            if (diff < tol) break;
            x = 1.0 / diff;
            if (x > 1e15) break;
        }
        return Fraction(neg ? -h1 : h1, k1);
    }
};

struct Complex {
    double re, im;
    Complex() : re(0), im(0) {}
    Complex(double r) : re(r), im(0) {}
    Complex(double r, double i) : re(r), im(i) {}
    Complex operator+(const Complex& o) const { return Complex(re+o.re, im+o.im); }
    Complex operator-(const Complex& o) const { return Complex(re-o.re, im-o.im); }
    Complex operator*(const Complex& o) const { return Complex(re*o.re-im*o.im, re*o.im+im*o.re); }
    Complex operator/(const Complex& o) const {
        double d = o.re*o.re + o.im*o.im;
        return Complex((re*o.re+im*o.im)/d, (im*o.re-re*o.im)/d);
    }
    Complex conj() const { return Complex(re, -im); }
    double  abs()  const { return sqrt(re*re + im*im); }
    double  arg()  const { return atan2(im, re); }
    static Complex fromPolar(double r, double theta) {
        return Complex(r*cos(theta), r*sin(theta));
    }
    Complex powN(double n) const { return fromPolar(::pow(abs(), n), arg()*n); }
    Complex sqrtC()        const { return fromPolar(sqrt(abs()), arg()*0.5); }
    Complex expC()         const { return Complex(::exp(re)*cos(im), ::exp(re)*sin(im)); }
    Complex logC()         const { return Complex(::log(abs()), arg()); }
    Complex sinC() const { return Complex(::sin(re)*::cosh(im), ::cos(re)*::sinh(im)); }
    Complex cosC() const { return Complex(::cos(re)*::cosh(im), -::sin(re)*::sinh(im)); }
    void toString(char* out, int n, bool polar = false) const {
        if (polar) {
            snprintf(out, n, "%g\xE2\x88\xA0%g", abs(), arg());
        } else {
            if (fabs(im) < 1e-12)      snprintf(out, n, "%g", re);
            else if (fabs(re) < 1e-12) snprintf(out, n, "%gi", im);
            else if (im > 0)            snprintf(out, n, "%g+%gi", re, im);
            else                        snprintf(out, n, "%g%gi", re, im);
        }
    }
};

#define POLY_MAX 16
struct Polynomial {
    double c[POLY_MAX+1];
    int    deg;
    Polynomial() : deg(0) { for (int i=0;i<=POLY_MAX;i++) c[i]=0; }
    void   normalize() {
        deg = 0;
        for (int i=POLY_MAX; i>=0; i--) if (fabs(c[i]) > 1e-15) { deg=i; break; }
    }
    double  eval(double x) const {
        double r = 0;
        for (int i=deg; i>=0; i--) r = r*x + c[i];
        return r;
    }
    Complex evalC(Complex x) const {
        Complex r;
        for (int i=deg; i>=0; i--) r = r*x + Complex(c[i],0);
        return r;
    }
    Polynomial diff() const {
        Polynomial r;
        for (int i=1; i<=deg; i++) r.c[i-1] = c[i] * i;
        r.normalize();
        return r;
    }
    Polynomial integ() const {
        Polynomial r;
        for (int i=0; i<=deg && i<POLY_MAX; i++) r.c[i+1] = c[i] / (i+1.0);
        r.normalize();
        return r;
    }
    Polynomial operator+(const Polynomial& o) const {
        Polynomial r;
        int m = (deg > o.deg) ? deg : o.deg;
        for (int i=0;i<=m;i++) r.c[i] = c[i] + o.c[i];
        r.normalize();
        return r;
    }
    Polynomial operator-(const Polynomial& o) const {
        Polynomial r;
        int m = (deg > o.deg) ? deg : o.deg;
        for (int i=0;i<=m;i++) r.c[i] = c[i] - o.c[i];
        r.normalize();
        return r;
    }
    Polynomial operator*(const Polynomial& o) const {
        Polynomial r;
        for (int i=0; i<=deg; i++)
            for (int j=0; j<=o.deg && i+j<=POLY_MAX; j++)
                r.c[i+j] += c[i] * o.c[j];
        r.normalize();
        return r;
    }
    void toString(char* out, int n) const {
        out[0] = 0;
        bool first = true;
        for (int i=deg; i>=0; i--) {
            if (fabs(c[i]) < 1e-12) continue;
            char term[40];
            const char* sg = "";
            double v = c[i];
            if (first) { sg = (v<0)?"-":""; first = false; }
            else       { sg = (v<0)?" - ":" + "; }
            double a = fabs(v);
            if (i == 0)            snprintf(term,sizeof(term),"%s%g", sg, a);
            else if (i == 1) {
                if (fabs(a-1) < 1e-9) snprintf(term,sizeof(term),"%sx", sg);
                else                  snprintf(term,sizeof(term),"%s%gx", sg, a);
            } else {
                if (fabs(a-1) < 1e-9) snprintf(term,sizeof(term),"%sx^%d", sg, i);
                else                  snprintf(term,sizeof(term),"%s%gx^%d", sg, a, i);
            }
            strncat(out, term, n - strlen(out) - 1);
        }
        if (first) snprintf(out, n, "0");
    }
};

struct HistoryEntry {
    char    expr[80];
    double  value;
    bool    ok;
    bool    isComplex;
    double  imag;
    HistoryEntry() : value(0), ok(true), isComplex(false), imag(0) { expr[0]=0; }
};

struct StatColumn {
    char     name[12];
    VarScale scale;
    int      count;
    double*  data;
    double*  freq;
};

struct GState {
    AppMode    mode = MODE_MENU;
    AngleMode  angle = DEG;
    DispMode   disp = DISP_NORM;
    int        dispDigits = 4;
    int        normMode = 1;
    BaseMode   base = BASE_DEC;
    bool       rpn = false;
    bool       sdOK = false;
    bool       psramOK = false;
    bool       wifiOK = false;
    bool       fracDisplay = false;
    char       wifiSSID[33] = {0};
    char       wifiPASS[33] = {0};

    double  ans  = 0;
    Complex ansC = {0,0};
    bool    ansComplex = false;
    Fraction ansF;
    bool    ansFracValid = false;
    double  mem  = 0;
    double  vars[VAR_COUNT] = {0};

    double bank[ANS_BANK_MAX] = {0};
    char   bankLabel[ANS_BANK_MAX][16] = {{0}};
    int    bankN = 0;

    HistoryEntry hist[HIST_MAX];
    int          histN = 0;

    double rpnStk[4] = {0,0,0,0};
    int    rpnSp = 0;

    double  vecA[VEC_MAX] = {0};
    double  vecB[VEC_MAX] = {0};
    int     vecAdim = 3, vecBdim = 3;
    double  matA[MAT_MAX*MAT_MAX] = {0};
    double  matB[MAT_MAX*MAT_MAX] = {0};
    int     matAr = 0, matAc = 0;
    int     matBr = 0, matBc = 0;

    StatColumn stat[STAT_LISTS];
    int        statCapacity = 0;

    char*  ssData[SS_ROWS][SS_COLS] = {{nullptr}};
    bool   ssInitialized = false;

    char   cwd[64] = "/";

    bool allocStat(int cap = LIST_MAX) {
        freeStat();
        for (int i = 0; i < STAT_LISTS; i++) {
            stat[i].data = (double*)ps_malloc(cap * sizeof(double));
            stat[i].freq = nullptr;
            if (!stat[i].data) {
                stat[i].data = (double*)malloc(cap * sizeof(double));
            }
            if (!stat[i].data) { freeStat(); return false; }
            memset(stat[i].data, 0, cap * sizeof(double));
            snprintf(stat[i].name, sizeof(stat[i].name), "L%d", i+1);
            stat[i].scale = SCALE_RATIO;
            stat[i].count = 0;
        }
        statCapacity = cap;
        return true;
    }
    void freeStat() {
        for (int i = 0; i < STAT_LISTS; i++) {
            if (stat[i].data) { free(stat[i].data); stat[i].data = nullptr; }
            if (stat[i].freq) { free(stat[i].freq); stat[i].freq = nullptr; }
            stat[i].count = 0;
        }
        statCapacity = 0;
    }
    void freeSS() {
        for (int r=0; r<SS_ROWS; r++)
            for (int c=0; c<SS_COLS; c++)
                if (ssData[r][c]) { free(ssData[r][c]); ssData[r][c] = nullptr; }
        ssInitialized = false;
    }
    void setSS(int r, int c, const char* expr) {
        if (r<0 || r>=SS_ROWS || c<0 || c>=SS_COLS) return;
        if (ssData[r][c]) { free(ssData[r][c]); ssData[r][c] = nullptr; }
        if (expr && *expr) {
            int n = strlen(expr) + 1;
            ssData[r][c] = (char*)malloc(n);
            if (ssData[r][c]) strcpy(ssData[r][c], expr);
        }
    }
    void pushHist(const char* expr, double v, bool ok = true,
                  bool isC = false, double imag = 0) {
        int idx;
        if (histN < HIST_MAX) idx = histN++;
        else {
            for (int i=1; i<HIST_MAX; i++) hist[i-1] = hist[i];
            idx = HIST_MAX - 1;
        }
        strncpy(hist[idx].expr, expr, 79); hist[idx].expr[79] = 0;
        hist[idx].value = v;
        hist[idx].ok = ok;
        hist[idx].isComplex = isC;
        hist[idx].imag = imag;
    }
    void resetVars() { for (int i=0;i<VAR_COUNT;i++) vars[i]=0; mem=0; }
    void resetStat() { for (int i=0;i<STAT_LISTS;i++) stat[i].count=0; }
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
};
GState G;

inline double toRad(double x) {
    if (G.angle == DEG)  return x * M_PI / 180.0;
    if (G.angle == GRAD) return x * M_PI / 200.0;
    return x;
}
inline double fromRad(double x) {
    if (G.angle == DEG)  return x * 180.0 / M_PI;
    if (G.angle == GRAD) return x * 200.0 / M_PI;
    return x;
}

inline const char* siPrefix(int em) {
    static const char* P[] = {"y","z","a","f","p","n","u","m","","k","M","G","T","P","E","Z","Y"};
    int k = em / 3 + 8;
    if (k < 0) k = 0;
    if (k > 16) k = 16;
    return P[k];
}

inline void formatNum(double v, char* out, int n) {
    if (isnan(v)) { snprintf(out,n,"NaN"); return; }
    if (isinf(v)) { snprintf(out,n,v<0?"-Inf":"Inf"); return; }
    int d = G.dispDigits;
    if (d < 0) d = 0; if (d > 9) d = 9;
    switch (G.disp) {
    case DISP_FIX:
        snprintf(out, n, "%.*f", d, v);
        break;
    case DISP_SCI:
        snprintf(out, n, "%.*e", d, v);
        break;
    case DISP_ENG: {
        if (v == 0) { snprintf(out,n,"0"); break; }
        double a = fabs(v);
        int ex = (int)floor(log10(a));
        int em = (ex>=0) ? (ex/3)*3 : -(((-ex)+2)/3)*3;
        double m = v / pow(10.0, em);
        const char* sp = siPrefix(em);
        if (*sp)  snprintf(out, n, "%.*f%s",  d, m, sp);
        else      snprintf(out, n, "%.*fe%d", d, m, em);
    } break;
    default: {
        double lim_lo = (G.normMode == 2) ? 1e-9 : 1e-2;
        double lim_hi = 1e10;
        double a = fabs(v);
        if (a == 0 || (a >= lim_lo && a < lim_hi))
            snprintf(out, n, "%.*g", d+6, v);
        else
            snprintf(out, n, "%.*e", d+2, v);
    } break;
    }
}
inline String numStr(double v) { char b[40]; formatNum(v,b,40); return String(b); }

inline void formatInt(long long v, char* out, int n, BaseMode b) {
    switch (b) {
    case BASE_HEX: snprintf(out, n, "0x%llX", (unsigned long long)v); break;
    case BASE_OCT: snprintf(out, n, "0o%llo", (unsigned long long)v); break;
    case BASE_BIN: {
        unsigned long long u = (unsigned long long)v;
        char buf[68]; int p = 0;
        if (u == 0) { strncpy(out, "0b0", n); return; }
        while (u && p < 64) { buf[p++] = '0' + (int)(u & 1ULL); u >>= 1; }
        int o = 0;
        if (o < n-1) out[o++] = '0';
        if (o < n-1) out[o++] = 'b';
        while (p-- && o < n-1) out[o++] = buf[p];
        out[o] = 0;
    } break;
    default: snprintf(out, n, "%lld", v); break;
    }
}

inline bool tryDecimalToSpecial(double v, char* out, int n) {
    if (!isfinite(v) || v == 0) return false;
    double a = fabs(v);
    double rp = a / M_PI;
    if (a < 1e6 && fabs(rp - round(rp)) < 1e-9) {
        long long k = (long long)round(rp);
        if (k != 0) {
            if (v < 0)  snprintf(out, n, "-%lld\xCF\x80", k);
            else        snprintf(out, n,  "%lld\xCF\x80", k);
            return true;
        }
    }
    double rp2 = (2*a) / M_PI;
    if (a < 1e6 && fabs(rp2 - round(rp2)) < 1e-9 && (long long)round(rp2)%2!=0) {
        long long k = (long long)round(rp2);
        if (v < 0) snprintf(out, n, "-%lld\xCF\x80/2", k);
        else       snprintf(out, n,  "%lld\xCF\x80/2", k);
        return true;
    }
    for (int rt = 2; rt < 200; rt++) {
        double sq = sqrt((double)rt);
        double k = a / sq;
        if (k < 100 && fabs(k - round(k)) < 1e-9) {
            long long ki = (long long)round(k);
            if (ki != 0) {
                if (v < 0)  snprintf(out, n, "-%lld\xE2\x88\x9A%d", ki, rt);
                else        snprintf(out, n,  "%lld\xE2\x88\x9A%d", ki, rt);
                return true;
            }
        }
    }
    Fraction f = Fraction::fromDouble(v, 1e-9, 100000);
    if (fabs(f.toDouble() - v) < 1e-9 && f.den > 1) {
        f.toString(out, n);
        return true;
    }
    return false;
}

inline int textWidth(const char* s) {
    return strlen(s) * 6;
}

struct KSum {
    double s = 0.0, c = 0.0;
    inline void add(double x) {
        double t = s + x;
        if (fabs(s) >= fabs(x)) c += (s - t) + x;
        else                    c += (x - t) + s;
        s = t;
    }
    inline double get() const { return s + c; }
};

class DisplayMgr;
class ExprEval;
extern DisplayMgr DM;
extern ExprEval   gEv;
extern unsigned long gLastInputT;

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 2 / 20 ─ DisplayMgr                                             ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class DisplayMgr {
public:
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

    void begin() {
        auto& d = M5Cardputer.Display;
        d.setRotation(1);
        d.setColorDepth(16);
        d.fillScreen(C_BG);
        d.setFont(&fonts::Font0);
        d.setTextSize(1);
        d.setTextColor(C_FG, C_BG);
    }

    // ── スプラッシュ (splash_data.h 利用時は画像のまま、非利用時はPaper風) ──
    void splash() {
        auto& d = M5Cardputer.Display;
#ifdef SPLASH_RLE
  #if SPLASH_RLE == 1
        int px = 0, py = 0;
        for (int i = 0; i < SPLASH_RLE_LEN; i++) {
            uint8_t cnt = pgm_read_byte(&splash_rle_count[i]);
            uint16_t val = pgm_read_word(&splash_rle_value[i]);
            for (int k = 0; k < cnt; k++) {
                if (px < SPLASH_W && py < SPLASH_H) d.drawPixel(px, py, val);
                px++;
                if (px >= SPLASH_W) { px = 0; py++; }
            }
        }
  #else
        for (int y = 0; y < SPLASH_H; y++)
            for (int x = 0; x < SPLASH_W; x++) {
                uint16_t v = pgm_read_word(&splash_data[y * SPLASH_W + x]);
                d.drawPixel(x, y, v);
            }
  #endif
        d.setTextDatum(bottom_left);
        d.setTextSize(1);
        d.setTextColor(S_FG, S_BG);
        d.drawString("Ver. " POMAS_VER, LCD_W - 236, LCD_H - 2);
        d.setTextDatum(top_left);
#else
        // テキストスプラッシュ (Paper風)
        d.fillScreen(C_BG);
        d.fillRect(0, 0, LCD_W, HDR_H, C_HDR);
        d.fillRect(0, HDR_H, LCD_W, 2, C_ACC2);
        d.fillRect(0, LCD_H - FTR_H, LCD_W, FTR_H, C_HDR);
        d.fillRect(0, LCD_H - FTR_H - 1, LCD_W, 2, C_ACC2);

        d.setTextDatum(middle_center);
        d.setTextSize(2);
        d.setTextColor(C_HDRTX, C_BG);
        char title[32]; snprintf(title, sizeof(title), POMAS_NAME " ver." POMAS_VER);
        d.drawString(title, LCD_W/2, LCD_H/2 - 14);
        d.setTextSize(1);
        d.setTextColor(C_DIM, C_BG);
        d.drawString(POMAS_CRED, LCD_W/2, LCD_H/2 + 6);
        // アクセントバー (細)
        d.fillRect(LCD_W/2 - 34, LCD_H/2 + 20, 68, 2, C_ACC);
        d.fillRect(LCD_W/2 - 24, LCD_H/2 + 24, 48, 2, C_ACC2);
        d.setTextDatum(top_left);
#endif
        delay(2500);
        d.fillScreen(C_BG);
    }

    // ── ヘッダ: [左アクセント3px][タイトル][右にバッジチップ] ──
    void header(const char* title) {
        auto& d = M5Cardputer.Display;
        // バー背景
        d.fillRect(0, 0, LCD_W, HDR_H, C_HDR);
        // 左アクセント
        d.fillRect(0, 0, 3, HDR_H, C_SEL_BAR);
        // 下線 (2段: 深ミント + 明ミント)
        d.fillRect(0, HDR_H - 2, LCD_W, 1, C_ACC2);
        d.fillRect(0, HDR_H - 1, LCD_W, 1, C_ACC);
        // タイトル
        d.setTextColor(C_HDRTX, C_HDR);
        d.setTextDatum(top_left);
        d.drawString(title, 6, 3);
        // バッジ
        char bd[40];
        const char* ang = (G.angle==DEG)?"DEG":(G.angle==RAD?"RAD":"GRA");
        const char* dsp =
            (G.disp==DISP_FIX)?"FIX":
            (G.disp==DISP_SCI)?"SCI":
            (G.disp==DISP_ENG)?"ENG":"NRM";
        snprintf(bd, sizeof(bd), "%s|%s%s%s",
                 ang, dsp, G.rpn?"|RPN":"",
                 G.base==BASE_DEC?"":(G.base==BASE_HEX?"|HEX":(G.base==BASE_OCT?"|OCT":"|BIN")));
        int bw = textWidth(bd);
        int cx = LCD_W - bw - 8;
        if (cx < 4) cx = 4;
        // チップ背景
        d.fillRect(cx - 3, 3, bw + 6, HDR_H - 6, C_CHIP);
        d.setTextColor(C_HDRDIM, C_CHIP);
        d.drawString(bd, cx, 3);
    }

    // ── フッタ: [左ヒント][中央][右ヒント]、上下に罫線 ──
    void footer(const char* l = "", const char* c = "", const char* r = "") {
        auto& d = M5Cardputer.Display;
        int y0 = LCD_H - FTR_H;
        d.fillRect(0, y0, LCD_W, FTR_H, C_HDR);
        d.fillRect(0, y0, LCD_W, 1, C_ACC2);
        d.fillRect(0, y0 + 1, LCD_W, 1, C_ACC);
        d.setTextColor(C_HDRDIM, C_HDR);
        d.setTextDatum(top_left);
        if (l && *l) d.drawString(l, 4, y0 + 3);
        if (c && *c) {
            int cw = textWidth(c);
            d.drawString(c, (LCD_W - cw)/2, y0 + 3);
        }
        if (r && *r) {
            int rw = textWidth(r);
            d.drawString(r, LCD_W - rw - 4, y0 + 3);
        }
    }

    void clearBody() {
        auto& d = M5Cardputer.Display;
        d.fillRect(0, HDR_H, LCD_W, LCD_H - HDR_H - FTR_H, C_BG);
    }

    // ── 通知: ソフトシャドウ + 左アクセントストライプ ──
    void notify(const char* msg, uint16_t color = C_ACC, int ms = 800) {
        auto& d = M5Cardputer.Display;
        int w = textWidth(msg) + 24;
        if (w > LCD_W - 12) w = LCD_W - 12;
        int x = (LCD_W - w)/2, y = LCD_H/2 - 12;
        // 影
        d.fillRect(x + 2, y + 2, w, 24, C_SHADOW);
        // パネル
        d.fillRect(x, y, w, 24, C_PANEL);
        d.drawRect(x, y, w, 24, color);
        // 左アクセントストライプ
        d.fillRect(x + 1, y + 1, 3, 22, color);
        // 文字
        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        char clipMsg[64];
        int maxCh = (w - 24) / 6;
        if (maxCh > 63) maxCh = 63;
        if (maxCh < 1) maxCh = 1;
        strncpy(clipMsg, msg, maxCh); clipMsg[maxCh] = 0;
        d.drawString(clipMsg, LCD_W/2 + 2, y + 12);
        d.setTextDatum(top_left);
        delay(ms);
    }

    // ── 確認ダイアログ: Y/N を色分けチップで ──
    bool confirm(const char* msg) {
        auto& d = M5Cardputer.Display;
        int w = textWidth(msg) + 24;
        if (w > LCD_W - 12) w = LCD_W - 12;
        if (w < 180) w = 180;
        int h = 60;
        int x = (LCD_W - w)/2, y = LCD_H/2 - h/2;
        // 影
        d.fillRect(x + 2, y + 2, w, h, C_SHADOW);
        // パネル
        d.fillRect(x, y, w, h, C_PANEL);
        d.drawRect(x, y, w, h, C_WARN);
        // 上アクセントバー
        d.fillRect(x + 1, y + 1, w - 2, 2, C_WARN);
        // メッセージ
        d.setTextColor(C_FG, C_PANEL);
        d.setTextDatum(middle_center);
        char clipCfm[64];
        int maxCc = (w - 24) / 6;
        if (maxCc > 63) maxCc = 63;
        if (maxCc < 1) maxCc = 1;
        strncpy(clipCfm, msg, maxCc); clipCfm[maxCc] = 0;
        d.drawString(clipCfm, LCD_W/2, y + 16);
        // ボタン
        int by = y + h - 20;
        int bw = 60;
        // Yes (緑)
        d.fillRect(LCD_W/2 - bw - 8, by, bw, 16, C_OK);
        d.setTextColor(C_PANEL, C_OK);
        d.drawString("[Y] Yes", LCD_W/2 - bw/2 - 8, by + 8);
        // No (赤)
        d.fillRect(LCD_W/2 + 8, by, bw, 16, C_ERR);
        d.setTextColor(C_PANEL, C_ERR);
        d.drawString("[N] No", LCD_W/2 + bw/2 + 8, by + 8);
        d.setTextDatum(top_left);
        while (true) {
            if (pollKeys()) {
                if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                    M5Cardputer.Keyboard.isKeyPressed('Y')) return true;
                if (M5Cardputer.Keyboard.isKeyPressed('n') ||
                    M5Cardputer.Keyboard.isKeyPressed('N')) return false;
                if (IS_ESC) return false;
            }
            delay(15);
        }
    }

    void waitEsc(const char* hint = "ESC: back") {
        footer("", "", hint);
        while (true) {
            if (pollKeys() && IS_ESC) break;
            delay(15);
        }
    }

    // ── 入力行: プロンプトをチップ化 + 2px カーソル ──
    bool inputLine(const char* prompt, char* buf, int maxLen, int y = 0) {
        auto& d = M5Cardputer.Display;
        int yy = (y > 0) ? y : LCD_H/2 - 18;
        int w = LCD_W - 8;
        int h = 32;
        // 影
        d.fillRect(6, yy + 2, w, h, C_SHADOW);
        // パネル
        d.fillRect(4, yy, w, h, C_PANEL);
        d.drawRect(4, yy, w, h, C_ACC);
        // プロンプトチップ
        int pw = textWidth(prompt) + 8;
        d.fillRect(4, yy, pw + 4, 12, C_ACC);
        d.setTextColor(C_PANEL, C_ACC);
        d.setTextDatum(top_left);
        d.drawString(prompt, 8, yy + 2);
        // 入力エリア
        bool blink = false;
        unsigned long lastBlink = millis();
        int cur = strlen(buf);
        while (true) {
            // 入力エリア再描画
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
            unsigned long t0 = millis();
            while (millis() - t0 < 80) {
                if (pollKeys()) {
                    auto st = M5Cardputer.Keyboard.keysState();
                    if (st.fn && IS_ESC) return false;
                    if (st.enter) return true;
                    if (st.del && cur > 0) { buf[--cur] = 0; goto rd; }
                    for (auto c : st.word) {
                        if (cur < maxLen-1 && c >= ' ' && c < 127) {
                            buf[cur++] = c; buf[cur] = 0;
                        }
                    }
                    if (!st.word.empty()) goto rd;
                }
                delay(15);
            }
            if (millis() - lastBlink > 400) { blink = !blink; lastBlink = millis(); }
            rd:;
        }
    }

    // ── メニュー: 選択行の強調 + 交互行 + 細セパレータ ──
    int menu(const char* title, const char* items[], int n, int init = 0) {
        auto& d = M5Cardputer.Display;
        int sel = init;
        if (sel < 0) sel = 0;
        if (sel >= n) sel = n-1;
        const int rowH = 13;
        int viewH = LCD_H - HDR_H - FTR_H;
        int rows = viewH / rowH;
        int top = 0;
        while (true) {
            if (sel < top) top = sel;
            if (sel >= top + rows) top = sel - rows + 1;
            d.fillScreen(C_BG);
            header(title);
            footer("ENT:OK", "", "ESC:back");
            for (int i = 0; i < rows; i++) {
                int idx = top + i;
                if (idx >= n) break;
                int y = HDR_H + i * rowH;
                bool isSel = (idx == sel);
                // 交互背景
                uint16_t bg = isSel ? C_SEL_BG : ((i & 1) ? C_BG : C_PANEL2);
                d.fillRect(0, y, LCD_W, rowH, bg);
                // 行セパレータ
                d.drawFastHLine(0, y + rowH - 1, LCD_W, C_LINE);
                // 左選択バー
                if (isSel) d.fillRect(0, y, 3, rowH, C_SEL_BAR);
                // 番号 (アクセント色)
                char num[4]; snprintf(num, sizeof(num), "%d", idx+1);
                uint16_t numCol = isSel ? C_SEL_BAR : C_ACC;
                d.setTextColor(numCol, bg);
                d.drawString(num, 8, y + 3);
                // テキスト
                uint16_t txtCol = isSel ? C_HDRTX : C_FG;
                d.setTextColor(txtCol, bg);
                d.drawString(items[idx], 26, y + 3);
            }
            // スクロールバー
            if (n > rows) {
                int sbx = LCD_W - 3;
                d.fillRect(sbx, HDR_H, 3, viewH, C_PANEL2);
                int thumb = (rows * viewH) / n;
                if (thumb < 12) thumb = 12;
                int ty = HDR_H + (top * (viewH - thumb)) / (n - rows);
                d.fillRect(sbx, ty, 3, thumb, C_ACC);
            }
            while (true) {
                if (!pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return -1;
                if (st.enter) return sel;
                if (st.fn && IS_UP)   { if (sel > 0)   { sel--; break; } }
                if (st.fn && IS_DOWN) { if (sel < n-1) { sel++; break; } }
                for (auto c : st.word) {
                    if (c >= '1' && c <= '9') {
                        int k = c - '1';
                        if (k < n) return k;
                    }
                }
                if (!st.word.empty()) {
                    bool found = false;
                    for (auto c : st.word) if (c >= '1' && c <= '9') found = true;
                    if (found) break;
                }
            }
        }
    }

    // ── 折返しテキスト (ロジック同一、既定色は C_FG) ──
    int drawWrapped(const char* text, int x, int y, int w, uint16_t col = C_FG) {
        auto& d = M5Cardputer.Display;
        d.setTextColor(col, C_BG);
        int maxCh = w / 6;
        int len = strlen(text), p = 0, yy = y;
        char line[80];
        while (p < len && yy < LCD_H - FTR_H - 2) {
            int sz = (len - p < maxCh) ? (len - p) : maxCh;
            if (sz < len - p) {
                int sp = sz;
                while (sp > maxCh/2 && line[sp] != ' ' && p+sp < len && text[p+sp] != ' ') sp--;
                if (sp > maxCh/2) sz = sp;
            }
            memcpy(line, text + p, sz); line[sz] = 0;
            d.drawString(line, x, yy);
            yy += 11; p += sz;
            while (p < len && text[p] == ' ') p++;
        }
        return yy;
    }

    // ── BMP 保存 (ロジック同一) ──
    bool saveBMP(const char* path) {
        if (!G.sdOK) return false;
        auto& d = M5Cardputer.Display;
        SdGuard guard;
        File f = SD.open(path, FILE_WRITE);
        if (!f) return false;
        const int W = LCD_W, H = LCD_H;
        const int rowSize = ((W * 3 + 3) / 4) * 4;
        uint32_t imgSize = rowSize * H;
        uint32_t fileSize = 54 + imgSize;
        uint8_t hdr[54];
        memset(hdr, 0, sizeof(hdr));
        hdr[0]='B'; hdr[1]='M';
        hdr[2]=fileSize & 0xFF; hdr[3]=(fileSize>>8)&0xFF;
        hdr[4]=(fileSize>>16)&0xFF; hdr[5]=(fileSize>>24)&0xFF;
        hdr[10]=54; hdr[14]=40;
        hdr[18]=W & 0xFF; hdr[19]=(W>>8)&0xFF;
        hdr[22]=H & 0xFF; hdr[23]=(H>>8)&0xFF;
        hdr[26]=1; hdr[28]=24;
        hdr[34]=imgSize & 0xFF; hdr[35]=(imgSize>>8)&0xFF;
        hdr[36]=(imgSize>>16)&0xFF; hdr[37]=(imgSize>>24)&0xFF;
        f.write(hdr, 54);
        uint8_t* row = (uint8_t*)malloc(rowSize);
        if (!row) { f.close(); return false; }
        for (int y = H-1; y >= 0; y--) {
            memset(row, 0, rowSize);
            for (int x = 0; x < W; x++) {
                uint16_t px = d.readPixel(x, y);
                uint8_t r5 = (px >> 11) & 0x1F;
                uint8_t g6 = (px >> 5)  & 0x3F;
                uint8_t b5 =  px        & 0x1F;
                row[x*3 + 0] = (b5 * 255) / 31;
                row[x*3 + 1] = (g6 * 255) / 63;
                row[x*3 + 2] = (r5 * 255) / 31;
            }
            f.write(row, rowSize);
        }
        free(row);
        f.close();
        return true;
    }

    bool saveBMPAuto(const char* prefix = "pomas") {
        char path[48];
        snprintf(path, sizeof(path), "/%s_%lu.bmp", prefix, millis());
        bool ok = saveBMP(path);
        notify(ok ? path : "BMP save failed", ok ? C_OK : C_ERR, 1200);
        return ok;
    }
};
DisplayMgr DM;

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 3 / 20 ─ ExprEval                                               ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class ExprEval {
public:
    struct Subst { char var; double val; };
    struct Result { double val; bool ok; const char* err; };

    Result eval(const char* expr, const Subst* subs = nullptr, int nSubs = 0) {
        _src = expr; _pos = 0; _err = nullptr; _subs = subs; _nSubs = nSubs;
        skipSpaces();
        double v = parseExpr();
        if (_err) return {0, false, _err};
        skipSpaces();
        if (_src[_pos] != 0) return {0, false, "trailing chars"};
        return {v, true, nullptr};
    }

private:
    const char* _src; int _pos;
    const char* _err;
    const Subst* _subs; int _nSubs;

    void skipSpaces() { while (_src[_pos] == ' ' || _src[_pos] == '\t') _pos++; }
    bool match(const char* s) {
        skipSpaces();
        int n = strlen(s);
        if (strncmp(_src + _pos, s, n) == 0) { _pos += n; return true; }
        return false;
    }
    bool matchCI(const char* s) {
        skipSpaces();
        int n = strlen(s);
        if (strncasecmp(_src + _pos, s, n) == 0) { _pos += n; return true; }
        return false;
    }
    void setErr(const char* msg) { if (!_err) _err = msg; }

    double parseExpr() {
        double v = parseTerm();
        while (true) {
            skipSpaces();
            char c = _src[_pos];
            if (c == '+') { _pos++; v += parseTerm(); }
            else if (c == '-') { _pos++; v -= parseTerm(); }
            else break;
        }
        return v;
    }
    double parseTerm() {
        double v = parseFactor();
        while (true) {
            skipSpaces();
            char c = _src[_pos];
            if (c == '*') { _pos++; v *= parseFactor(); }
            else if (c == '/') { _pos++; double w = parseFactor();
                if (w == 0) { setErr("/0"); return 0; } v /= w; }
            else if (c == '%') { _pos++; double w = parseFactor();
                if (w == 0) { setErr("/0"); return 0; } v = fmod(v,w); }
            else if (matchCI("mod")) { double w = parseFactor();
                if (w == 0) { setErr("/0"); return 0; } v = fmod(v,w); }
            else break;
        }
        return v;
    }
    double parseFactor() {
        double v = parseUnary();
        while (true) {
            skipSpaces();
            if (_src[_pos] == '!') { _pos++; v = factD(v); }
            else break;
        }
        skipSpaces();
        if (_src[_pos] == '^') { _pos++; double e = parseFactor(); v = pow(v, e); }
        return v;
    }
    double parseUnary() {
        skipSpaces();
        char c = _src[_pos];
        if (c == '+') { _pos++; return parseUnary(); }
        if (c == '-') { _pos++; return -parseUnary(); }
        return parsePrimary();
    }
    double parsePrimary() {
        skipSpaces();
        char c = _src[_pos];
        if (c == '(') {
            _pos++;
            double v = parseExpr();
            skipSpaces();
            if (_src[_pos] == ')') _pos++;
            else setErr(") expected");
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
        if (isdigit((unsigned char)c) || c == '.') {
            char* end;
            double v = strtod(_src + _pos, &end);
            _pos = end - _src;
            return v;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            return parseIdent();
        }
        setErr("syntax");
        _pos++;
        return 0;
    }

    double parseIdent() {
        char name[24]; int ni = 0;
        while ((isalnum((unsigned char)_src[_pos]) || _src[_pos] == '_') && ni < 23) {
            name[ni++] = _src[_pos++];
        }
        name[ni] = 0;
        skipSpaces();
        if (_src[_pos] == '(') {
            _pos++;
            double a[8]; int na = 0;
            if (_src[_pos] != ')') {
                a[na++] = parseExpr();
                while (_src[_pos] == ',' && na < 8) {
                    _pos++; a[na++] = parseExpr();
                }
            }
            skipSpaces();
            if (_src[_pos] == ')') _pos++;
            else setErr(") expected");
            return callFn(name, a, na);
        }
        if (!strcmp(name, "pi") || !strcmp(name, "PI") || !strcmp(name, "Pi")) return M_PI;
        if (!strcmp(name, "e") || !strcasecmp(name, "euler"))                  return M_E;
        if (!strcasecmp(name, "ans"))                                          return G.ans;
        if (!strcasecmp(name, "mem"))                                          return G.mem;
        if (!strcasecmp(name, "phi"))                                          return 1.6180339887498948482;
        if (!strcasecmp(name, "gamma"))                                        return 0.5772156649015328606;
        if (!strcasecmp(name, "inf"))                                          return INFINITY;
        if (!strcasecmp(name, "nan"))                                          return NAN;
        if (ni == 1) {
            char v = name[0];
            for (int i = 0; i < _nSubs; i++)
                if (_subs[i].var == v) return _subs[i].val;
            if (v >= 'A' && v <= 'J') return G.vars[v - 'A'];
            if (v >= 'a' && v <= 'j') return G.vars[v - 'a'];
            if (v == 'x' || v == 'X') return 0;
            if (v == 't' || v == 'T') return 0;
            if (v == 'n' || v == 'N') return 0;
        }
        setErr("unknown id");
        return 0;
    }

    double callFn(const char* nm, double* a, int na) {
        if (na == 1) {
            if (!strcasecmp(nm,"sin"))   return sin(toRad(a[0]));
            if (!strcasecmp(nm,"cos"))   return cos(toRad(a[0]));
            if (!strcasecmp(nm,"tan"))   return tan(toRad(a[0]));
            if (!strcasecmp(nm,"asin"))  return fromRad(asin(a[0]));
            if (!strcasecmp(nm,"acos"))  return fromRad(acos(a[0]));
            if (!strcasecmp(nm,"atan"))  return fromRad(atan(a[0]));
            if (!strcasecmp(nm,"sinh"))  return sinh(a[0]);
            if (!strcasecmp(nm,"cosh"))  return cosh(a[0]);
            if (!strcasecmp(nm,"tanh"))  return tanh(a[0]);
            if (!strcasecmp(nm,"asinh")) return asinh(a[0]);
            if (!strcasecmp(nm,"acosh")) return acosh(a[0]);
            if (!strcasecmp(nm,"atanh")) return atanh(a[0]);
            if (!strcasecmp(nm,"csc"))   return 1.0/sin(toRad(a[0]));
            if (!strcasecmp(nm,"sec"))   return 1.0/cos(toRad(a[0]));
            if (!strcasecmp(nm,"cot"))   return 1.0/tan(toRad(a[0]));
            if (!strcasecmp(nm,"csch"))  return 1.0/sinh(a[0]);
            if (!strcasecmp(nm,"sech"))  return 1.0/cosh(a[0]);
            if (!strcasecmp(nm,"coth"))  return 1.0/tanh(a[0]);
            if (!strcasecmp(nm,"exp"))   return exp(a[0]);
            if (!strcasecmp(nm,"ln"))    return log(a[0]);
            if (!strcasecmp(nm,"log"))   return log10(a[0]);
            if (!strcasecmp(nm,"log10")) return log10(a[0]);
            if (!strcasecmp(nm,"log2"))  return log(a[0])/log(2.0);
            if (!strcasecmp(nm,"sqrt"))  return sqrt(a[0]);
            if (!strcasecmp(nm,"cbrt"))  return cbrt(a[0]);
            if (!strcasecmp(nm,"abs"))   return fabs(a[0]);
            if (!strcasecmp(nm,"sign"))  return (a[0]>0)-(a[0]<0);
            if (!strcasecmp(nm,"sgn"))   return (a[0]>0)-(a[0]<0);
            if (!strcasecmp(nm,"floor")) return floor(a[0]);
            if (!strcasecmp(nm,"ceil"))  return ceil(a[0]);
            if (!strcasecmp(nm,"round")) return round(a[0]);
            if (!strcasecmp(nm,"int"))   return trunc(a[0]);
            if (!strcasecmp(nm,"frac"))  return a[0] - trunc(a[0]);
            if (!strcasecmp(nm,"fact"))  return factD(a[0]);
            if (!strcasecmp(nm,"gamma")) return tgamma(a[0]);
            if (!strcasecmp(nm,"lgamma"))return lgamma(a[0]);
            if (!strcasecmp(nm,"erf"))   return erf(a[0]);
            if (!strcasecmp(nm,"erfc"))  return erfc(a[0]);
            if (!strcasecmp(nm,"deg"))   return a[0] * 180.0/M_PI;
            if (!strcasecmp(nm,"rad"))   return a[0] * M_PI/180.0;
            if (!strcasecmp(nm,"isPrime") || !strcasecmp(nm,"prime"))
                                          return isPrime((long long)a[0]) ? 1 : 0;
            if (!strcasecmp(nm,"nextPrime")) return nextPrime((long long)a[0]);
            if (!strcasecmp(nm,"prevPrime")) return prevPrime((long long)a[0]);
            if (!strcasecmp(nm,"random"))    return ((double)esp_random() / 4294967296.0);
            if (!strcasecmp(nm,"randInt") || !strcasecmp(nm,"rint"))
                                          return floor(((double)esp_random()/4294967296.0) * a[0]);
            if (!strcasecmp(nm,"randN")) {
                double u1 = (esp_random()+1.0) / 4294967297.0;
                double u2 = (esp_random()+1.0) / 4294967297.0;
                return sqrt(-2.0*log(u1))*cos(2*M_PI*u2) * a[0];
            }
            if (!strcasecmp(nm,"toF"))   return a[0] * 1.8 + 32.0;
            if (!strcasecmp(nm,"toC"))   return (a[0] - 32.0) / 1.8;
            if (!strcasecmp(nm,"toK"))   return a[0] + 273.15;
        }
        if (na == 2) {
            if (!strcasecmp(nm,"pow"))   return pow(a[0], a[1]);
            if (!strcasecmp(nm,"nrt"))   return pow(a[1], 1.0/a[0]);
            if (!strcasecmp(nm,"atan2")) return fromRad(atan2(a[0], a[1]));
            if (!strcasecmp(nm,"hypot")) return hypot(a[0], a[1]);
            if (!strcasecmp(nm,"min"))   return fmin(a[0], a[1]);
            if (!strcasecmp(nm,"max"))   return fmax(a[0], a[1]);
            if (!strcasecmp(nm,"mod"))   return fmod(a[0], a[1]);
            if (!strcasecmp(nm,"gcd"))   return (double)gcdLL((long long)a[0], (long long)a[1]);
            if (!strcasecmp(nm,"lcm"))   {
                long long x = (long long)a[0], y = (long long)a[1];
                if (!x || !y) return 0;
                return (double)((x/gcdLL(x,y)) * y);
            }
            if (!strcasecmp(nm,"nPr"))   return permD((long long)a[0], (long long)a[1]);
            if (!strcasecmp(nm,"nCr"))   return combD((long long)a[0], (long long)a[1]);
            if (!strcasecmp(nm,"logn"))  return log(a[1]) / log(a[0]);
            if (!strcasecmp(nm,"randR")) {
                double u = (double)esp_random() / 4294967296.0;
                return a[0] + u * (a[1] - a[0]);
            }
        }
        if (na == 3) {
            if (!strcasecmp(nm,"if"))    return a[0] ? a[1] : a[2];
            if (!strcasecmp(nm,"clamp")) return fmin(fmax(a[0], a[1]), a[2]);
            if (!strcasecmp(nm,"dms")) {
                double s = (a[0]<0||a[1]<0||a[2]<0) ? -1 : 1;
                return s * (fabs(a[0]) + fabs(a[1])/60.0 + fabs(a[2])/3600.0);
            }
        }
        setErr("unknown fn");
        return 0;
    }

    static double factD(double x) {
        if (x < 0 || x != floor(x) || x > 170) return tgamma(x + 1.0);
        double r = 1;
        for (int i = 2; i <= (int)x; i++) r *= i;
        return r;
    }
    static long long gcdLL(long long a, long long b) {
        if (a < 0) a = -a; if (b < 0) b = -b;
        while (b) { long long t = a % b; a = b; b = t; }
        return a ? a : 1;
    }
    static double permD(long long n, long long r) {
        if (r < 0 || r > n) return 0;
        double v = 1;
        for (long long i = 0; i < r; i++) v *= (n - i);
        return v;
    }
    static double combD(long long n, long long r) {
        if (r < 0 || r > n) return 0;
        if (r > n - r) r = n - r;
        double v = 1;
        for (long long i = 0; i < r; i++) v = v * (n - i) / (i + 1);
        return v;
    }
    static unsigned long long mulmod64(unsigned long long a, unsigned long long b, unsigned long long m) {
        unsigned long long result = 0;
        a %= m;
        while (b > 0) {
            if (b & 1ULL) result = (result + a) % m;
            a = (a << 1) % m;
            b >>= 1;
        }
        return result;
    }
    static unsigned long long modpow64(unsigned long long b, unsigned long long e, unsigned long long m) {
        unsigned long long r = 1 % m; b %= m;
        while (e) {
            if (e & 1) r = mulmod64(r, b, m);
            b = mulmod64(b, b, m);
            e >>= 1;
        }
        return r;
    }
    static bool isPrime(long long n) {
        if (n < 2) return false;
        for (long long p : {2,3,5,7,11,13,17,19,23,29,31,37}) {
            if (n == p) return true;
            if (n % p == 0) return false;
        }
        unsigned long long d = n - 1; int r = 0;
        while ((d & 1) == 0) { d >>= 1; r++; }
        for (unsigned long long a : {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL}) {
            if (a >= (unsigned long long)n) continue;
            unsigned long long x = modpow64(a, d, n);
            if (x == 1 || x == (unsigned long long)(n-1)) continue;
            bool composite = true;
            for (int i = 0; i < r-1; i++) {
                x = mulmod64(x, x, n);
                if (x == (unsigned long long)(n-1)) { composite = false; break; }
            }
            if (composite) return false;
        }
        return true;
    }
    static long long nextPrime(long long n) {
        if (n < 2) return 2;
        long long p = n + 1;
        while (!isPrime(p)) p++;
        return p;
    }
    static long long prevPrime(long long n) {
        if (n <= 2) return 0;
        long long p = n - 1;
        while (p > 1 && !isPrime(p)) p--;
        return (p > 1) ? p : 0;
    }
};
ExprEval gEv;

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 4 / 20 ─ NumAnalysis / Mat / Dist                               ║
// ╚═══════════════════════════════════════════════════════════════════════╝

namespace NumAnalysis {

inline double evalAt(const char* expr, char var, double x) {
    ExprEval::Subst s = {var, x};
    auto r = gEv.eval(expr, &s, 1);
    return r.ok ? r.val : NAN;
}

inline double diffN(const char* expr, char var, double x, double h = 0.0, double* errOut = nullptr) {
    const int    NTAB = 12;
    const double CON = 1.4, CON2 = CON*CON, BIG = 1e300, SAFE = 2.0;
    double hh = (h > 0) ? h : 0.01 * (fabs(x) > 1.0 ? fabs(x) : 1.0);
    if (hh == 0) hh = 1e-4;
    static double A[NTAB][NTAB];
    A[0][0] = (evalAt(expr, var, x+hh) - evalAt(expr, var, x-hh)) / (2*hh);
    double err = BIG, ans = A[0][0];
    for (int i = 1; i < NTAB; i++) {
        hh /= CON;
        A[0][i] = (evalAt(expr, var, x+hh) - evalAt(expr, var, x-hh)) / (2*hh);
        double fac = CON2;
        for (int j = 1; j <= i; j++) {
            A[j][i] = (A[j-1][i]*fac - A[j-1][i-1]) / (fac - 1.0);
            fac = CON2*fac;
            double errt = fmax(fabs(A[j][i]-A[j-1][i]), fabs(A[j][i]-A[j-1][i-1]));
            if (errt <= err) { err = errt; ans = A[j][i]; }
        }
        if (fabs(A[i][i] - A[i-1][i-1]) >= SAFE*err) break;
    }
    if (errOut) *errOut = (err >= BIG) ? NAN : err;
    return ans;
}

inline double adsimp(const char* expr, char var,
                     double a, double b, double fa, double fb, double fm,
                     double S, double eps, int depth, double* errAcc, long* evals) {
    double m  = 0.5*(a + b);
    double lm = 0.5*(a + m), rm = 0.5*(m + b);
    double flm = evalAt(expr, var, lm), frm = evalAt(expr, var, rm);
    if (evals) *evals += 2;
    double Sl = (m - a)/6.0 * (fa + 4*flm + fm);
    double Sr = (b - m)/6.0 * (fm + 4*frm + fb);
    double S2 = Sl + Sr;
    bool budget = (evals && *evals > 200000L);
    if (depth <= 0 || budget || fabs(S2 - S) <= 15.0*eps) {
        if (errAcc) *errAcc += fabs(S2 - S) / 15.0;
        return S2 + (S2 - S)/15.0;
    }
    return adsimp(expr, var, a, m, fa, fm, flm, Sl, eps*0.5, depth-1, errAcc, evals)
         + adsimp(expr, var, m, b, fm, fb, frm, Sr, eps*0.5, depth-1, errAcc, evals);
}

inline double integN(const char* expr, char var, double a, double b, int n = 0, double* errOut = nullptr) {
    (void)n;
    if (a == b) { if (errOut) *errOut = 0; return 0.0; }
    double sign = 1.0;
    if (a > b) { double t=a; a=b; b=t; sign = -1.0; }
    double fa = evalAt(expr, var, a), fb = evalAt(expr, var, b);
    double m  = 0.5*(a+b),  fm = evalAt(expr, var, m);
    double S  = (b - a)/6.0 * (fa + 4*fm + fb);
    double eps = 1e-11 * (fabs(S) + 1.0);
    double errAcc = 0.0; long evals = 3;
    double r = adsimp(expr, var, a, b, fa, fb, fm, S, eps, 22, &errAcc, &evals);
    if (errOut) *errOut = errAcc;
    return sign * r;
}

inline double sumN(const char* expr, char var, double from, double to) {
    KSum k;
    long long lo = (long long)from, hi = (long long)to;
    for (long long i = lo; i <= hi; i++) k.add(evalAt(expr, var, (double)i));
    return k.get();
}
inline double prodN(const char* expr, char var, double from, double to) {
    double p = 1;
    long long lo = (long long)from, hi = (long long)to;
    for (long long i = lo; i <= hi; i++) p *= evalAt(expr, var, (double)i);
    return p;
}

inline bool solveNR(const char* expr, char var, double x0, double& root,
                    double tol = 1e-9, int maxIter = 200) {
    double x = x0;
    for (int i = 0; i < maxIter; i++) {
        double fx = evalAt(expr, var, x);
        if (!isfinite(fx)) return false;
        if (fabs(fx) < tol) { root = x; return true; }
        double d = diffN(expr, var, x);
        if (fabs(d) < 1e-15) return false;
        double nx = x - fx / d;
        if (fabs(nx - x) < tol) { root = nx; return true; }
        x = nx;
    }
    root = x;
    return false;
}

inline bool bisect(const char* expr, char var, double a, double b,
                   double& root, double tol = 1e-10, int maxIter = 200) {
    double fa = evalAt(expr, var, a);
    double fb = evalAt(expr, var, b);
    if (!isfinite(fa) || !isfinite(fb)) return false;
    if (fa * fb > 0) return false;
    for (int i = 0; i < maxIter; i++) {
        double m = (a + b) / 2;
        double fm = evalAt(expr, var, m);
        if (!isfinite(fm)) return false;
        if (fabs(fm) < tol || (b - a) / 2 < tol) { root = m; return true; }
        if (fa * fm < 0) { b = m; fb = fm; }
        else             { a = m; fa = fm; }
    }
    root = (a + b) / 2;
    return true;
}

inline int quadratic(double a, double b, double c, double r[2], double i[2]) {
    if (fabs(a) < 1e-15) {
        if (fabs(b) < 1e-15) return 0;
        r[0] = -c / b; i[0] = 0;
        return 1;
    }
    double D = b*b - 4*a*c;
    if (D >= 0) {
        double s = sqrt(D);
        r[0] = (-b + s) / (2*a); i[0] = 0;
        r[1] = (-b - s) / (2*a); i[1] = 0;
        return 2;
    }
    double s = sqrt(-D);
    r[0] = -b / (2*a); i[0] =  s / (2*a);
    r[1] = -b / (2*a); i[1] = -s / (2*a);
    return 2;
}

inline int cubic(double a, double b, double c, double d, double r[3], double im[3]) {
    if (fabs(a) < 1e-15) {
        double rr[2], ii[2];
        int k = quadratic(b, c, d, rr, ii);
        for (int i = 0; i < k; i++) { r[i] = rr[i]; im[i] = ii[i]; }
        return k;
    }
    double A = b/a, B = c/a, C = d/a;
    double Q = (3*B - A*A) / 9.0;
    double R = (9*A*B - 27*C - 2*A*A*A) / 54.0;
    double D = Q*Q*Q + R*R;
    if (D >= 0) {
        double sD = sqrt(D);
        double S = cbrt(R + sD);
        double T = cbrt(R - sD);
        r[0]  = -A/3 + (S + T);            im[0] = 0;
        r[1]  = -A/3 - (S + T)/2;          im[1] =  sqrt(3.0)/2 * (S - T);
        r[2]  = r[1];                      im[2] = -im[1];
        return 3;
    }
    double th = acos(R / sqrt(-Q*Q*Q));
    double mq = 2 * sqrt(-Q);
    r[0] = mq * cos( th       /3) - A/3; im[0] = 0;
    r[1] = mq * cos((th+2*M_PI)/3) - A/3; im[1] = 0;
    r[2] = mq * cos((th+4*M_PI)/3) - A/3; im[2] = 0;
    return 3;
}

inline int quartic(double a, double b, double c, double d, double e,
                   double r[4], double im[4]) {
    if (fabs(a) < 1e-15) return cubic(b, c, d, e, r, im);
    b /= a; c /= a; d /= a; e /= a;
    double p = c - 3*b*b/8;
    double q = d - b*c/2 + b*b*b/8;
    double rr2 = e - b*d/4 + b*b*c/16 - 3*b*b*b*b/256;
    double rc[3], ic[3];
    int k = cubic(1, 2*p, p*p - 4*rr2, -q*q, rc, ic);
    double y = NAN;
    for (int i = 0; i < k; i++) if (fabs(ic[i]) < 1e-9 && rc[i] > 0) { y = rc[i]; break; }
    if (!isfinite(y)) {
        for (int i = 0; i < k; i++) if (fabs(ic[i]) < 1e-6) { y = rc[i]; break; }
    }
    if (!isfinite(y)) return 0;
    double sq = sqrt(y);
    double A1 =  sq, B1 = -(2*p + y - q/sq) / 2;
    double A2 = -sq, B2 = -(2*p + y + q/sq) / 2;
    double r1[2], i1[2], r2[2], i2[2];
    quadratic(1, A1, B1, r1, i1);
    quadratic(1, A2, B2, r2, i2);
    r[0]  = r1[0] - b/4; im[0] = i1[0];
    r[1]  = r1[1] - b/4; im[1] = i1[1];
    r[2]  = r2[0] - b/4; im[2] = i2[0];
    r[3]  = r2[1] - b/4; im[3] = i2[1];
    return 4;
}

inline int polyRoots(const double* coef, int deg, Complex* roots) {
    if (deg <= 0) return 0;
    if (deg == 1) { roots[0] = Complex(-coef[0]/coef[1], 0); return 1; }
    Complex z[16];
    for (int i = 0; i < deg; i++) {
        double th = 2*M_PI*i/deg + 0.4;
        z[i] = Complex(0.4*cos(th), 0.9*sin(th));
    }
    auto evalC = [&](Complex x){
        Complex r;
        for (int i = deg; i >= 0; i--) r = r*x + Complex(coef[i],0);
        return r;
    };
    for (int it = 0; it < 200; it++) {
        double maxd = 0;
        for (int i = 0; i < deg; i++) {
            Complex denom(1,0);
            for (int j = 0; j < deg; j++) if (j != i) denom = denom * (z[i] - z[j]);
            Complex fz = evalC(z[i]);
            if (denom.abs() < 1e-300) continue;
            Complex dz = fz / denom;
            z[i] = z[i] - dz;
            if (dz.abs() > maxd) maxd = dz.abs();
        }
        if (maxd < 1e-12) break;
    }
    for (int i = 0; i < deg; i++) roots[i] = z[i];
    return deg;
}

} // namespace NumAnalysis

namespace Mat {

inline void mul(const double* a, int r, int c, const double* b, int k, double* out) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < k; j++) {
            double s = 0;
            for (int l = 0; l < c; l++) s += a[i*c+l] * b[l*k+j];
            out[i*k+j] = s;
        }
}
inline void transpose(const double* a, int r, int c, double* out) {
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++) out[j*r+i] = a[i*c+j];
}
inline bool gaussSolve(double* A, int n, double* b) {
    for (int i = 0; i < n; i++) {
        int pi = i; double pv = fabs(A[i*n+i]);
        for (int k = i+1; k < n; k++) if (fabs(A[k*n+i]) > pv) { pv = fabs(A[k*n+i]); pi = k; }
        if (pv < 1e-15) return false;
        if (pi != i) {
            for (int j = 0; j < n; j++) { double t = A[i*n+j]; A[i*n+j] = A[pi*n+j]; A[pi*n+j] = t; }
            double t = b[i]; b[i] = b[pi]; b[pi] = t;
        }
        double piv = A[i*n+i];
        for (int j = i; j < n; j++) A[i*n+j] /= piv;
        b[i] /= piv;
        for (int k = 0; k < n; k++) if (k != i) {
            double f = A[k*n+i];
            for (int j = i; j < n; j++) A[k*n+j] -= f * A[i*n+j];
            b[k] -= f * b[i];
        }
    }
    return true;
}
inline double det(const double* in, int n) {
    double* A = (double*)malloc(n*n*sizeof(double));
    if (!A) return 0;
    memcpy(A, in, n*n*sizeof(double));
    double s = 1;
    for (int i = 0; i < n; i++) {
        int pi = i; double pv = fabs(A[i*n+i]);
        for (int k = i+1; k < n; k++) if (fabs(A[k*n+i]) > pv) { pv = fabs(A[k*n+i]); pi = k; }
        if (pv < 1e-15) { free(A); return 0; }
        if (pi != i) {
            for (int j = 0; j < n; j++) { double t = A[i*n+j]; A[i*n+j] = A[pi*n+j]; A[pi*n+j] = t; }
            s = -s;
        }
        for (int k = i+1; k < n; k++) {
            double f = A[k*n+i] / A[i*n+i];
            for (int j = i; j < n; j++) A[k*n+j] -= f * A[i*n+j];
        }
    }
    double d = s;
    for (int i = 0; i < n; i++) d *= A[i*n+i];
    free(A);
    return d;
}
inline bool inv(const double* in, int n, double* out) {
    double* A = (double*)malloc(n*n*sizeof(double));
    if (!A) return false;
    memcpy(A, in, n*n*sizeof(double));
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) out[i*n+j] = (i==j)?1.0:0.0;
    for (int i = 0; i < n; i++) {
        int pi = i; double pv = fabs(A[i*n+i]);
        for (int k = i+1; k < n; k++) if (fabs(A[k*n+i]) > pv) { pv = fabs(A[k*n+i]); pi = k; }
        if (pv < 1e-15) { free(A); return false; }
        if (pi != i) {
            for (int j = 0; j < n; j++) {
                double t1 = A[i*n+j];   A[i*n+j]   = A[pi*n+j];   A[pi*n+j]   = t1;
                double t2 = out[i*n+j]; out[i*n+j] = out[pi*n+j]; out[pi*n+j] = t2;
            }
        }
        double p = A[i*n+i];
        for (int j = 0; j < n; j++) { A[i*n+j] /= p; out[i*n+j] /= p; }
        for (int k = 0; k < n; k++) if (k != i) {
            double f = A[k*n+i];
            for (int j = 0; j < n; j++) {
                A[k*n+j]   -= f * A[i*n+j];
                out[k*n+j] -= f * out[i*n+j];
            }
        }
    }
    free(A);
    return true;
}
inline int rank(const double* in, int r, int c) {
    double* A = (double*)malloc(r*c*sizeof(double));
    if (!A) return 0;
    memcpy(A, in, r*c*sizeof(double));
    int rk = 0, col = 0;
    for (int row = 0; row < r && col < c; col++) {
        int pi = row; double pv = fabs(A[row*c + col]);
        for (int k = row+1; k < r; k++) if (fabs(A[k*c+col]) > pv) { pv = fabs(A[k*c+col]); pi = k; }
        if (pv < 1e-12) continue;
        if (pi != row) for (int j = 0; j < c; j++) { double t = A[row*c+j]; A[row*c+j] = A[pi*c+j]; A[pi*c+j] = t; }
        for (int k = row+1; k < r; k++) {
            double f = A[k*c + col] / A[row*c + col];
            for (int j = col; j < c; j++) A[k*c+j] -= f * A[row*c+j];
        }
        row++; rk++;
    }
    free(A);
    return rk;
}
inline bool jacobiEig(double* A, int n, double* eig, double* V = nullptr) {
    if (V) for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) V[i*n+j] = (i==j)?1.0:0.0;
    for (int it = 0; it < 100; it++) {
        int p = 0, q = 1;
        double maxv = 0;
        for (int i = 0; i < n; i++) for (int j = i+1; j < n; j++)
            if (fabs(A[i*n+j]) > maxv) { maxv = fabs(A[i*n+j]); p = i; q = j; }
        if (maxv < 1e-12) break;
        double app = A[p*n+p], aqq = A[q*n+q], apq = A[p*n+q];
        double th = 0.5 * atan2(2*apq, app - aqq);
        double c = cos(th), s = sin(th);
        for (int i = 0; i < n; i++) {
            double aip = A[i*n+p], aiq = A[i*n+q];
            A[i*n+p] = c*aip + s*aiq;
            A[i*n+q] = -s*aip + c*aiq;
        }
        for (int j = 0; j < n; j++) {
            double apj = A[p*n+j], aqj = A[q*n+j];
            A[p*n+j] = c*apj + s*aqj;
            A[q*n+j] = -s*apj + c*aqj;
        }
        if (V) for (int i = 0; i < n; i++) {
            double vip = V[i*n+p], viq = V[i*n+q];
            V[i*n+p] = c*vip + s*viq;
            V[i*n+q] = -s*vip + c*viq;
        }
    }
    for (int i = 0; i < n; i++) eig[i] = A[i*n+i];
    for (int i = 0; i < n; i++)
        for (int j = i+1; j < n; j++)
            if (eig[i] < eig[j]) {
                double t = eig[i]; eig[i] = eig[j]; eig[j] = t;
                if (V) for (int k = 0; k < n; k++) {
                    double tt = V[k*n+i]; V[k*n+i] = V[k*n+j]; V[k*n+j] = tt;
                }
            }
    return true;
}
inline bool qrEig(const double* in, int n, double* eigRe, double* eigIm) {
    double* A = (double*)malloc(n*n*sizeof(double));
    if (!A) return false;
    memcpy(A, in, n*n*sizeof(double));
    for (int it = 0; it < 200; it++) {
        double maxOff = 0;
        for (int r = 0; r < n; r++)
            for (int c = 0; c < n; c++)
                if (r != c && fabs(A[r*n+c]) > maxOff) maxOff = fabs(A[r*n+c]);
        if (maxOff < 1e-10) break;
        double* Q = (double*)malloc(n*n*sizeof(double));
        if (!Q) { free(A); return false; }
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) Q[i*n+j] = (i==j)?1.0:0.0;
        for (int i = 0; i < n-1; i++)
            for (int k = i+1; k < n; k++) {
                double a = A[i*n+i], b = A[k*n+i];
                double r = hypot(a, b);
                if (r < 1e-15) continue;
                double c = a/r, s = b/r;
                for (int j = 0; j < n; j++) {
                    double t1 = c*A[i*n+j] + s*A[k*n+j];
                    double t2 = -s*A[i*n+j] + c*A[k*n+j];
                    A[i*n+j] = t1; A[k*n+j] = t2;
                    double q1 = c*Q[j*n+i] + s*Q[j*n+k];
                    double q2 = -s*Q[j*n+i] + c*Q[j*n+k];
                    Q[j*n+i] = q1; Q[j*n+k] = q2;
                }
            }
        double* T = (double*)malloc(n*n*sizeof(double));
        if (!T) { free(Q); free(A); return false; }
        Mat::mul(A, n, n, Q, n, T);
        memcpy(A, T, n*n*sizeof(double));
        free(T); free(Q);
    }
    int i = 0;
    while (i < n) {
        if (i < n-1 && fabs(A[(i+1)*n+i]) > 1e-9) {
            double a = A[i*n+i], b = A[i*n+i+1];
            double c = A[(i+1)*n+i], d = A[(i+1)*n+i+1];
            double tr = a + d, dt = a*d - b*c;
            double D = tr*tr - 4*dt;
            if (D >= 0) {
                eigRe[i]   = (tr + sqrt(D))/2; eigIm[i]   = 0;
                eigRe[i+1] = (tr - sqrt(D))/2; eigIm[i+1] = 0;
            } else {
                eigRe[i]   = tr/2; eigIm[i]   =  sqrt(-D)/2;
                eigRe[i+1] = tr/2; eigIm[i+1] = -sqrt(-D)/2;
            }
            i += 2;
        } else {
            eigRe[i] = A[i*n+i]; eigIm[i] = 0; i++;
        }
    }
    free(A);
    return true;
}
inline bool luSolve(double* A, int n, double* b) { return gaussSolve(A, n, b); }

} // namespace Mat

namespace Dist {

inline double normalCDF(double x) {
    return 0.5 * (1.0 + erf(x / sqrt(2.0)));
}
inline double normalPDF(double x, double mu = 0, double sd = 1) {
    double z = (x - mu) / sd;
    return exp(-0.5*z*z) / (sd * sqrt(2*M_PI));
}
inline double normalInv(double p) {
    if (p <= 0 || p >= 1) return p <= 0 ? -INFINITY : INFINITY;
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02,
                                1.383577518672690e+02,-3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,-1.556989798598866e+02,
                                6.680131188771972e+01,-1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03,-3.223964580411365e-01,-2.400758277161838e+00,
                               -2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00};
    static const double dd[]= { 7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00,
                                3.754408661907416e+00};
    double pl = 0.02425, ph = 1 - pl;
    if (p < pl) {
        double q = sqrt(-2*log(p));
        return (((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
               ((((dd[0]*q+dd[1])*q+dd[2])*q+dd[3])*q+1);
    }
    if (p <= ph) {
        double q = p - 0.5, r = q*q;
        return (((((a[0]*r+a[1])*r+a[2])*r+a[3])*r+a[4])*r+a[5])*q /
               (((((b[0]*r+b[1])*r+b[2])*r+b[3])*r+b[4])*r+1);
    }
    double q = sqrt(-2*log(1-p));
    return -(((((c[0]*q+c[1])*q+c[2])*q+c[3])*q+c[4])*q+c[5]) /
            ((((dd[0]*q+dd[1])*q+dd[2])*q+dd[3])*q+1);
}

inline double gammaP(double a, double x) {
    if (x <= 0 || a <= 0) return 0;
    if (x < a + 1) {
        double s = 1.0/a, t = s;
        for (int i = 1; i < 200; i++) {
            t *= x / (a + i);
            s += t;
            if (fabs(t) < fabs(s)*1e-12) break;
        }
        return s * exp(-x + a*log(x) - lgamma(a));
    }
    double b = x + 1 - a, c = 1e30, d = 1.0/b, h = d;
    for (int i = 1; i < 200; i++) {
        double an = -i*(i - a);
        b += 2;
        d = an * d + b;       if (fabs(d) < 1e-300) d = 1e-300;
        c = b + an / c;       if (fabs(c) < 1e-300) c = 1e-300;
        d = 1.0 / d;
        double del = d * c;
        h *= del;
        if (fabs(del - 1) < 1e-12) break;
    }
    return 1.0 - h * exp(-x + a*log(x) - lgamma(a));
}
inline double chiSquareCDF(double x, double df) {
    if (x <= 0) return 0;
    return gammaP(df/2.0, x/2.0);
}
inline double chiSquarePDF(double x, double df) {
    if (x <= 0) return 0;
    return pow(x, df/2-1) * exp(-x/2) / (pow(2, df/2) * tgamma(df/2));
}
inline double chiSquareInv(double p, double df) {
    if (p <= 0) return 0;
    if (p >= 1) return INFINITY;
    double lo = 0, hi = df * 10 + 100;
    while (chiSquareCDF(hi, df) < p) hi *= 2;
    for (int i = 0; i < 100; i++) {
        double m = (lo + hi)/2;
        if (chiSquareCDF(m, df) < p) lo = m; else hi = m;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi)/2;
}

inline double betaCF(double a, double b, double x) {
    double qab = a+b, qap = a+1, qam = a-1;
    double c = 1, d = 1 - qab*x/qap;
    if (fabs(d) < 1e-300) d = 1e-300;
    d = 1/d; double h = d;
    for (int m = 1; m <= 200; m++) {
        int m2 = 2*m;
        double aa = m*(b-m)*x/((qam+m2)*(a+m2));
        d = 1 + aa*d; if (fabs(d) < 1e-300) d = 1e-300;
        c = 1 + aa/c; if (fabs(c) < 1e-300) c = 1e-300;
        d = 1/d; h *= d*c;
        aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d = 1 + aa*d; if (fabs(d) < 1e-300) d = 1e-300;
        c = 1 + aa/c; if (fabs(c) < 1e-300) c = 1e-300;
        d = 1/d; double del = d*c; h *= del;
        if (fabs(del - 1) < 1e-12) break;
    }
    return h;
}
inline double betaInc(double a, double b, double x) {
    if (x <= 0) return 0;
    if (x >= 1) return 1;
    double bt = exp(lgamma(a+b)-lgamma(a)-lgamma(b)+a*log(x)+b*log(1-x));
    if (x < (a+1)/(a+b+2)) return bt * betaCF(a, b, x) / a;
    return 1 - bt * betaCF(b, a, 1-x) / b;
}
inline double tCDF(double t, double df) {
    double x = df / (df + t*t);
    double p = 0.5 * betaInc(df/2.0, 0.5, x);
    return (t > 0) ? 1 - p : p;
}
inline double tPDF(double t, double df) {
    return exp(lgamma((df+1)/2) - lgamma(df/2)) / sqrt(df*M_PI) *
           pow(1 + t*t/df, -(df+1)/2);
}
inline double tInv(double p, double df) {
    if (p <= 0) return -INFINITY;
    if (p >= 1) return  INFINITY;
    double lo = -100, hi = 100;
    for (int i = 0; i < 100; i++) {
        double m = (lo + hi)/2;
        if (tCDF(m, df) < p) lo = m; else hi = m;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi)/2;
}
inline double FCDF(double x, double d1, double d2) {
    if (x <= 0) return 0;
    return 1 - betaInc(d2/2, d1/2, d2/(d2 + d1*x));
}
inline double FPDF(double x, double d1, double d2) {
    if (x <= 0) return 0;
    return exp(lgamma((d1+d2)/2) - lgamma(d1/2) - lgamma(d2/2) +
               (d1/2)*log((double)d1/d2) + (d1/2-1)*log(x) -
               ((d1+d2)/2)*log(1 + (double)d1*x/d2));
}
inline double FInv(double p, double d1, double d2) {
    if (p <= 0) return 0;
    if (p >= 1) return INFINITY;
    double lo = 0, hi = 100;
    while (FCDF(hi, d1, d2) < p) hi *= 2;
    for (int i = 0; i < 100; i++) {
        double m = (lo + hi)/2;
        if (FCDF(m, d1, d2) < p) lo = m; else hi = m;
        if (hi - lo < 1e-9) break;
    }
    return (lo + hi)/2;
}
inline double binomPMF(int k, int n, double p) {
    if (k < 0 || k > n) return 0;
    return exp(lgamma(n+1) - lgamma(k+1) - lgamma(n-k+1) + k*log(p) + (n-k)*log(1-p));
}
inline double binomCDF(int k, int n, double p) {
    double s = 0;
    for (int i = 0; i <= k; i++) s += binomPMF(i, n, p);
    return s;
}
inline int binomInv(double q, int n, double p) {
    double s = 0;
    for (int k = 0; k <= n; k++) {
        s += binomPMF(k, n, p);
        if (s >= q) return k;
    }
    return n;
}
inline double poisPMF(int k, double lam) {
    if (k < 0) return 0;
    return exp(-lam + k*log(lam) - lgamma(k+1));
}
inline double poisCDF(int k, double lam) {
    double s = 0;
    for (int i = 0; i <= k; i++) s += poisPMF(i, lam);
    return s;
}
inline int poisInv(double q, double lam) {
    double s = 0;
    for (int k = 0; k < 1000; k++) {
        s += poisPMF(k, lam);
        if (s >= q) return k;
    }
    return 1000;
}
inline double geomPMF(int k, double p) {
    if (k < 1) return 0;
    return p * pow(1 - p, k - 1);
}
inline double geomCDF(int k, double p) {
    if (k < 1) return 0;
    return 1 - pow(1 - p, k);
}
inline int geomInv(double q, double p) {
    if (q <= 0) return 1;
    if (q >= 1) return 1000;
    if (p >= 1) return 1;
    if (p <= 0) return 1000;
    return (int)ceil(log(1 - q) / log(1 - p));
}

} // namespace Dist                          // ← この1行を追加

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 5 / 20 ─ 科学定数データベース (386項目 実装済)                  ║
// ╚═══════════════════════════════════════════════════════════════════════╝

struct Constant {
    const char* name;
    const char* sym;
    double      value;
    const char* unit;
    const char* cat;
};

static const Constant constDB[] PROGMEM = {
    // ── 物理化学定数 (CODATA 2022) 38項目 ──
    {"Speed of light in vacuum",         "c",       299792458.0,            "m/s",          "Physical"},
    {"Planck constant",                  "h",       6.62607015e-34,         "J s",          "Physical"},
    {"Reduced Planck constant",          "hbar",    1.054571817e-34,        "J s",          "Physical"},
    {"Newtonian constant of gravitation","G",       6.67430e-11,            "m3/kg/s2",     "Physical"},
    {"Standard gravity (g0)",            "g0_p",    9.80665,                "m/s2",         "Physical"},
    {"Earth surface gravity (g)",        "g_p",     9.80665,                "m/s2",         "Physical"},
    {"Elementary charge",                "e",       1.602176634e-19,        "C",            "Physical"},
    {"Vacuum magnetic permeability",     "mu0",     1.25663706212e-6,       "H/m",          "Physical"},
    {"Vacuum electric permittivity",     "eps0",    8.8541878128e-12,       "F/m",          "Physical"},
    {"Vacuum impedance",                 "Z0",      376.730313668,          "ohm",          "Physical"},
    {"Electron mass",                    "me",      9.1093837015e-31,       "kg",           "Physical"},
    {"Proton mass",                      "mp",      1.67262192369e-27,      "kg",           "Physical"},
    {"Neutron mass",                     "mn",      1.67492749804e-27,      "kg",           "Physical"},
    {"Atomic mass constant",             "mu",      1.66053906660e-27,      "kg",           "Physical"},
    {"Avogadro constant",                "N_A",     6.02214076e23,          "1/mol",        "Physical"},
    {"Faraday constant",                 "F",       96485.33212,            "C/mol",        "Physical"},
    {"Fine-structure constant",          "alpha",   7.2973525693e-3,        "",             "Physical"},
    {"Rydberg constant",                 "R_inf",   10973731.568160,        "1/m",          "Physical"},
    {"Bohr radius",                      "a0",      5.29177210903e-11,      "m",            "Physical"},
    {"Classical electron radius",        "re",      2.8179403262e-15,       "m",            "Physical"},
    {"Boltzmann constant",               "k",       1.380649e-23,           "J/K",          "Physical"},
    {"Stefan-Boltzmann constant",        "sigma",   5.670374419e-8,         "W/m2/K4",      "Physical"},
    {"Molar gas constant",               "R",       8.314462618,            "J/mol/K",      "Physical"},
    {"Wien displacement law constant",   "b",       2.897771955e-3,         "m K",          "Physical"},
    {"Electron magnetic moment",         "mu_e",   -9.2847647043e-24,       "J/T",          "Physical"},
    {"Proton magnetic moment",           "mu_p",    1.41060679736e-26,      "J/T",          "Physical"},
    {"Nuclear magneton",                 "mu_N",    5.0507837461e-27,       "J/T",          "Physical"},
    {"Bohr magneton",                    "mu_B",    9.2740100783e-24,       "J/T",          "Physical"},
    {"Molar volume of ideal gas (STP)",  "V_m",     22.41396954e-3,         "m3/mol",       "Physical"},
    {"Loschmidt constant",               "n0",      2.686780111e25,         "1/m3",         "Physical"},
    {"Sackur-Tetrode constant",          "S0/R",   -1.15170753706,          "",             "Physical"},
    {"Triple point of water",            "Ttp",     273.16,                 "K",            "Physical"},
    {"Standard atmosphere",              "atm",     101325.0,               "Pa",           "Physical"},
    {"Electron volt",                    "eV",      1.602176634e-19,        "J",            "Physical"},
    {"Magnetic flux quantum",            "Phi0",    2.067833848e-15,        "Wb",           "Physical"},
    {"Conductance quantum",              "G0",      7.748091729e-5,         "S",            "Physical"},
    {"Josephson constant",               "K_J",     483597.8484e9,          "Hz/V",         "Physical"},
    {"von Klitzing constant",            "R_K",     25812.80745,            "ohm",          "Physical"},

    // ── 数学定数 (8) ──
    {"Pi",                               "pi",      3.14159265358979323846, "",             "Math"},
    {"Euler's number",                   "e",       2.71828182845904523536, "",             "Math"},
    {"Golden ratio",                     "phi",     1.61803398874989484820, "",             "Math"},
    {"Euler-Mascheroni constant",        "gamma",   0.57721566490153286060, "",             "Math"},
    {"Square root of 2",                 "sqrt2",   1.41421356237309504880, "",             "Math"},
    {"Square root of 3",                 "sqrt3",   1.73205080756887729352, "",             "Math"},
    {"Natural log of 2",                 "ln2",     0.69314718055994530941, "",             "Math"},
    {"Natural log of 10",                "ln10",    2.30258509299404568401, "",             "Math"},

    // ── 電気電子材料 (40項目) ──
    {"Coulomb constant",                 "k_e",     8.987551787e9,          "N m2/C2",      "EE"},
    {"Electron mobility in Si (300K)",   "mu_e Si", 1417.0,                 "cm2/V/s",      "EE"},
    {"Hole mobility in Si (300K)",       "mu_h Si", 1350.0,                 "cm2/V/s",      "EE"},
    {"Silver resistivity (20C)",         "rho Ag",  1.59e-8,                "ohm m",        "EE"},
    {"Silver conductivity",              "sig Ag",  6.29e7,                 "S/m",          "EE"},
    {"Silver temperature coefficient",   "a Ag",    0.0061,                 "1/C",          "EE"},
    {"Copper resistivity (20C)",         "rho Cu",  1.68e-8,                "ohm m",        "EE"},
    {"Copper conductivity",              "sig Cu",  5.95e7,                 "S/m",          "EE"},
    {"Copper temperature coefficient",   "a Cu",    0.0068,                 "1/C",          "EE"},
    {"Gold resistivity (20C)",           "rho Au",  2.44e-8,                "ohm m",        "EE"},
    {"Gold conductivity",                "sig Au",  4.10e7,                 "S/m",          "EE"},
    {"Gold temperature coefficient",     "a Au",    0.0034,                 "1/C",          "EE"},
    {"Aluminum resistivity (20C)",       "rho Al",  2.65e-8,                "ohm m",        "EE"},
    {"Aluminum conductivity",            "sig Al",  3.77e7,                 "S/m",          "EE"},
    {"Aluminum temperature coefficient", "a Al",    0.00429,                "1/C",          "EE"},
    {"Tungsten resistivity (20C)",       "rho W",   5.6e-8,                 "ohm m",        "EE"},
    {"Tungsten conductivity",            "sig W",   1.79e7,                 "S/m",          "EE"},
    {"Tungsten temperature coefficient", "a W",     0.0045,                 "1/C",          "EE"},
    {"Iron resistivity (20C)",           "rho Fe",  9.71e-8,                "ohm m",        "EE"},
    {"Iron conductivity",                "sig Fe",  1.03e7,                 "S/m",          "EE"},
    {"Iron temperature coefficient",     "a Fe",    0.00651,                "1/C",          "EE"},
    {"Platinum resistivity (20C)",       "rho Pt",  10.6e-8,                "ohm m",        "EE"},
    {"Platinum conductivity",            "sig Pt",  0.943e7,                "S/m",          "EE"},
    {"Platinum temperature coefficient", "a Pt",    0.003927,               "1/C",          "EE"},
    {"Lead resistivity (20C)",           "rho Pb",  22e-8,                  "ohm m",        "EE"},
    {"Lead conductivity",                "sig Pb",  0.45e7,                 "S/m",          "EE"},
    {"Silicon band gap (300K)",          "Eg Si",   1.12,                   "eV",           "EE"},
    {"Silicon relative permittivity",    "er Si",   11.7,                   "",             "EE"},
    {"Germanium band gap (300K)",        "Eg Ge",   0.66,                   "eV",           "EE"},
    {"Germanium relative permittivity",  "er Ge",   16.2,                   "",             "EE"},
    {"GaAs band gap (300K)",             "Eg GaAs", 1.42,                   "eV",           "EE"},
    {"GaAs relative permittivity",       "er GaAs", 12.9,                   "",             "EE"},
    {"4H-SiC band gap",                  "Eg 4H",   2.86,                   "eV",           "EE"},
    {"6H-SiC band gap",                  "Eg 6H",   3.26,                   "eV",           "EE"},
    {"SiO2 relative permittivity",       "er SiO2", 3.9,                    "",             "EE"},
    {"Fused quartz resistivity",         "rho qrz", 7.5e17,                 "ohm m",        "EE"},
    {"Hard rubber resistivity (low)",    "rho rb1", 1e13,                   "ohm m",        "EE"},
    {"Hard rubber resistivity (high)",   "rho rb2", 1e16,                   "ohm m",        "EE"},
    {"Glass resistivity (low)",          "rho gl1", 1e9,                    "ohm m",        "EE"},
    {"Glass resistivity (high)",         "rho gl2", 1e13,                   "ohm m",        "EE"},

    // ── 天文定数 (12) ──
    {"Astronomical unit",                "AU",      149597870700.0,         "m",            "Astronomy"},
    {"Light-year",                       "ly",      9.4607304725808e15,     "m",            "Astronomy"},
    {"Parsec",                           "pc",      3.08567758149137e16,    "m",            "Astronomy"},
    {"Solar mass",                       "M_sun",   1.9884e30,              "kg",           "Astronomy"},
    {"Solar radius",                     "R_sun",   6.957e8,                "m",            "Astronomy"},
    {"Solar constant",                   "S",       1361.0,                 "W/m2",         "Astronomy"},
    {"Earth mean albedo",                "A_E",     0.30,                   "",             "Astronomy"},
    {"Standard gravity",                 "g0",      9.80665,                "m/s2",         "Astronomy"},
    {"Earth equatorial radius",          "R_eq",    6378137.0,              "m",            "Astronomy"},
    {"Earth polar radius",               "R_pol",   6356752.0,              "m",            "Astronomy"},
    {"Earth mass",                       "M_E",     5.9722e24,              "kg",           "Astronomy"},
    {"Moon orbital semi-major axis",     "a_moon",  384400.0,               "km",           "Astronomy"},

    // ── 太陽系8惑星 (64項目) ──
    {"Mercury mass","M Hg",0.330e24,"kg","Planet"},
    {"Mercury radius","R Hg",2440.0,"km","Planet"},
    {"Mercury density","rho Hg",5427.0,"kg/m3","Planet"},
    {"Mercury surface gravity","g Hg",3.7,"m/s2","Planet"},
    {"Mercury escape velocity","v_e Hg",4.3,"km/s","Planet"},
    {"Mercury rotation period","T_r Hg",1407.6,"h","Planet"},
    {"Mercury semi-major axis","a Hg",57.9,"Gm","Planet"},
    {"Mercury orbital period","T Hg",88.0,"d","Planet"},
    {"Venus mass","M Ve",4.87e24,"kg","Planet"},
    {"Venus radius","R Ve",6052.0,"km","Planet"},
    {"Venus density","rho Ve",5243.0,"kg/m3","Planet"},
    {"Venus surface gravity","g Ve",8.9,"m/s2","Planet"},
    {"Venus escape velocity","v_e Ve",10.4,"km/s","Planet"},
    {"Venus rotation period","T_r Ve",-5832.5,"h","Planet"},
    {"Venus semi-major axis","a Ve",108.2,"Gm","Planet"},
    {"Venus orbital period","T Ve",224.7,"d","Planet"},
    {"Earth mass (planetary)","M Ea",5.97e24,"kg","Planet"},
    {"Earth radius (planetary)","R Ea",6378.0,"km","Planet"},
    {"Earth density (planetary)","rho Ea",5514.0,"kg/m3","Planet"},
    {"Earth surface gravity","g Ea",9.8,"m/s2","Planet"},
    {"Earth escape velocity","v_e Ea",11.2,"km/s","Planet"},
    {"Earth rotation period","T_r Ea",24.0,"h","Planet"},
    {"Earth semi-major axis","a Ea",149.6,"Gm","Planet"},
    {"Earth orbital period","T Ea",365.2,"d","Planet"},
    {"Mars mass","M Ma",0.642e24,"kg","Planet"},
    {"Mars radius","R Ma",3396.0,"km","Planet"},
    {"Mars density","rho Ma",3933.0,"kg/m3","Planet"},
    {"Mars surface gravity","g Ma",3.7,"m/s2","Planet"},
    {"Mars escape velocity","v_e Ma",5.0,"km/s","Planet"},
    {"Mars rotation period","T_r Ma",24.7,"h","Planet"},
    {"Mars semi-major axis","a Ma",227.9,"Gm","Planet"},
    {"Mars orbital period","T Ma",687.0,"d","Planet"},
    {"Jupiter mass","M Ju",1898e24,"kg","Planet"},
    {"Jupiter radius","R Ju",71492.0,"km","Planet"},
    {"Jupiter density","rho Ju",1326.0,"kg/m3","Planet"},
    {"Jupiter surface gravity","g Ju",23.1,"m/s2","Planet"},
    {"Jupiter escape velocity","v_e Ju",59.5,"km/s","Planet"},
    {"Jupiter rotation period","T_r Ju",9.9,"h","Planet"},
    {"Jupiter semi-major axis","a Ju",778.6,"Gm","Planet"},
    {"Jupiter orbital period","T Ju",4331.0,"d","Planet"},
    {"Saturn mass","M Sa",568e24,"kg","Planet"},
    {"Saturn radius","R Sa",60268.0,"km","Planet"},
    {"Saturn density","rho Sa",687.0,"kg/m3","Planet"},
    {"Saturn surface gravity","g Sa",9.0,"m/s2","Planet"},
    {"Saturn escape velocity","v_e Sa",35.5,"km/s","Planet"},
    {"Saturn rotation period","T_r Sa",10.7,"h","Planet"},
    {"Saturn semi-major axis","a Sa",1433.5,"Gm","Planet"},
    {"Saturn orbital period","T Sa",10747.0,"d","Planet"},
    {"Uranus mass","M Ur",86.8e24,"kg","Planet"},
    {"Uranus radius","R Ur",25559.0,"km","Planet"},
    {"Uranus density","rho Ur",1271.0,"kg/m3","Planet"},
    {"Uranus surface gravity","g Ur",8.7,"m/s2","Planet"},
    {"Uranus escape velocity","v_e Ur",21.3,"km/s","Planet"},
    {"Uranus rotation period","T_r Ur",-17.2,"h","Planet"},
    {"Uranus semi-major axis","a Ur",2872.5,"Gm","Planet"},
    {"Uranus orbital period","T Ur",30589.0,"d","Planet"},
    {"Neptune mass","M Ne",102e24,"kg","Planet"},
    {"Neptune radius","R Ne",24764.0,"km","Planet"},
    {"Neptune density","rho Ne",1638.0,"kg/m3","Planet"},
    {"Neptune surface gravity","g Ne",11.0,"m/s2","Planet"},
    {"Neptune escape velocity","v_e Ne",23.5,"km/s","Planet"},
    {"Neptune rotation period","T_r Ne",16.1,"h","Planet"},
    {"Neptune semi-major axis","a Ne",4495.1,"Gm","Planet"},
    {"Neptune orbital period","T Ne",59800.0,"d","Planet"},

    // ── 準惑星・主要衛星 (32項目) ──
    {"Pluto mass","M Pl",0.0146e24,"kg","DwarfMoon"},
    {"Pluto radius","R Pl",1188.0,"km","DwarfMoon"},
    {"Pluto density","rho Pl",1850.0,"kg/m3","DwarfMoon"},
    {"Pluto surface gravity","g Pl",0.7,"m/s2","DwarfMoon"},
    {"Pluto escape velocity","v_e Pl",1.2,"km/s","DwarfMoon"},
    {"Pluto rotation period","T_r Pl",-153.3,"h","DwarfMoon"},
    {"Pluto semi-major axis","a Pl",5906.4,"Gm","DwarfMoon"},
    {"Pluto orbital period","T Pl",90560.0,"d","DwarfMoon"},
    {"Ceres mass","M Ce",0.00094e24,"kg","DwarfMoon"},
    {"Ceres radius","R Ce",476.0,"km","DwarfMoon"},
    {"Ceres density","rho Ce",2162.0,"kg/m3","DwarfMoon"},
    {"Ceres surface gravity","g Ce",0.27,"m/s2","DwarfMoon"},
    {"Ceres escape velocity","v_e Ce",0.51,"km/s","DwarfMoon"},
    {"Ceres rotation period","T_r Ce",9.1,"h","DwarfMoon"},
    {"Ceres semi-major axis","a Ce",414.0,"Gm","DwarfMoon"},
    {"Ceres orbital period","T Ce",1680.0,"d","DwarfMoon"},
    {"Eris mass","M Er",0.0166e24,"kg","DwarfMoon"},
    {"Eris radius","R Er",1163.0,"km","DwarfMoon"},
    {"Eris density","rho Er",2520.0,"kg/m3","DwarfMoon"},
    {"Eris surface gravity","g Er",0.82,"m/s2","DwarfMoon"},
    {"Eris escape velocity","v_e Er",1.38,"km/s","DwarfMoon"},
    {"Eris rotation period","T_r Er",25.9,"h","DwarfMoon"},
    {"Eris semi-major axis","a Er",10125.0,"Gm","DwarfMoon"},
    {"Eris orbital period","T Er",203830.0,"d","DwarfMoon"},
    {"Moon mass","M Mo",0.07346e24,"kg","DwarfMoon"},
    {"Moon radius","R Mo",1737.4,"km","DwarfMoon"},
    {"Moon density","rho Mo",3344.0,"kg/m3","DwarfMoon"},
    {"Moon surface gravity","g Mo",1.62,"m/s2","DwarfMoon"},
    {"Moon escape velocity","v_e Mo",2.38,"km/s","DwarfMoon"},
    {"Moon rotation period (synch)","T_r Mo",655.7,"h","DwarfMoon"},
    {"Moon semi-major axis","a Mo",0.384,"Gm","DwarfMoon"},
    {"Moon orbital period","T Mo",27.3,"d","DwarfMoon"},

    // ── 地球物理 (23項目) ──
    {"Earth mean radius","R_m E",6371.01,"km","Geo"},
    {"Earth mean density","rho_m E",5.51,"g/cm3","Geo"},
    {"Earth flattening factor","f E",1.0/298.257223563,"","Geo"},
    {"Earth angular velocity","om E",7.2921159e-5,"rad/s","Geo"},
    {"Earth moment of inertia factor","I/MR2",0.3307,"","Geo"},
    {"Dynamic form factor (J2)","J2",0.00108262545,"","Geo"},
    {"Upper crust P-wave velocity","Vp_uc",5.97,"km/s","Geo"},
    {"Middle crust P-wave velocity","Vp_mc",6.25,"km/s","Geo"},
    {"Lower crust P-wave velocity","Vp_lc",6.40,"km/s","Geo"},
    {"Upper mantle P-wave velocity","Vp_um",8.20,"km/s","Geo"},
    {"Upper crust S-wave velocity","Vs_uc",3.44,"km/s","Geo"},
    {"Middle crust S-wave velocity","Vs_mc",3.61,"km/s","Geo"},
    {"Lower crust S-wave velocity","Vs_lc",3.70,"km/s","Geo"},
    {"Upper mantle S-wave velocity","Vs_um",4.734,"km/s","Geo"},
    {"Crust mean density","rho_cr",2830.0,"kg/m3","Geo"},
    {"Upper mantle density","rho_um",3300.0,"kg/m3","Geo"},
    {"Geomagnetic dipole moment","M_geo",7.72e22,"A m2","Geo"},
    {"Gravity at pole","g_p",9.8321863685,"m/s2","Geo"},
    {"Gravity at equator","g_eq",9.7803267715,"m/s2","Geo"},
    {"Earth surface area","A_E",5.1e14,"m2","Geo"},
    {"Earth volume","V_E",1.0832e21,"m3","Geo"},
    {"Earth core radius","R_core",3485.0,"km","Geo"},
    {"Earth axial tilt","ax",23.4392811,"deg","Geo"},

    // ── 大気科学 (11項目) ──
    {"Sea level pressure","P0",101325.0,"Pa","Atmos"},
    {"Sea level temperature","T0",288.15,"K","Atmos"},
    {"Sea level air density","rho0",1.225,"kg/m3","Atmos"},
    {"Temperature lapse rate","L",-0.0065,"K/m","Atmos"},
    {"Mean molecular weight of dry air","M0",28.9644,"g/mol","Atmos"},
    {"Gas constant for dry air","R_d",287.058,"J/kg/K","Atmos"},
    {"Gas constant for water vapor","R_v",461.495,"J/kg/K","Atmos"},
    {"Specific heat of dry air (cp)","cp_d",1004.0,"J/kg/K","Atmos"},
    {"Specific heat of water vapor (cp)","cp_v",1865.0,"J/kg/K","Atmos"},
    {"Latent heat of vaporization (0C)","L_v",2.5e6,"J/kg","Atmos"},
    {"Karman constant","kappa",0.40,"","Atmos"},

    // ── 海洋物理 (4項目) ──
    {"Mean seawater density","rho_sw",1025.0,"kg/m3","Ocean"},
    {"Specific heat of seawater","cp_sw",3993.0,"J/kg/K","Ocean"},
    {"Standard seawater salinity","S_sw",35.0,"PSU","Ocean"},
    {"Speed of sound in seawater","v_sw",1522.0,"m/s","Ocean"},

    // ── 化学元素 (118+6=124項目) ──
    {"Hydrogen atomic weight","H",1.008,"g/mol","Element"},
    {"Helium atomic weight","He",4.0026,"g/mol","Element"},
    {"Lithium atomic weight","Li",6.94,"g/mol","Element"},
    {"Beryllium atomic weight","Be",9.0122,"g/mol","Element"},
    {"Boron atomic weight","B",10.81,"g/mol","Element"},
    {"Carbon atomic weight","C",12.011,"g/mol","Element"},
    {"Nitrogen atomic weight","N",14.007,"g/mol","Element"},
    {"Oxygen atomic weight","O",15.999,"g/mol","Element"},
    {"Fluorine atomic weight","F",18.998,"g/mol","Element"},
    {"Neon atomic weight","Ne",20.180,"g/mol","Element"},
    {"Sodium atomic weight","Na",22.990,"g/mol","Element"},
    {"Magnesium atomic weight","Mg",24.305,"g/mol","Element"},
    {"Aluminum atomic weight","Al",26.982,"g/mol","Element"},
    {"Silicon atomic weight","Si",28.085,"g/mol","Element"},
    {"Phosphorus atomic weight","P",30.974,"g/mol","Element"},
    {"Sulfur atomic weight","S",32.06,"g/mol","Element"},
    {"Chlorine atomic weight","Cl",35.45,"g/mol","Element"},
    {"Argon atomic weight","Ar",39.948,"g/mol","Element"},
    {"Potassium atomic weight","K",39.098,"g/mol","Element"},
    {"Calcium atomic weight","Ca",40.078,"g/mol","Element"},
    {"Scandium atomic weight","Sc",44.956,"g/mol","Element"},
    {"Titanium atomic weight","Ti",47.867,"g/mol","Element"},
    {"Vanadium atomic weight","V",50.942,"g/mol","Element"},
    {"Chromium atomic weight","Cr",51.996,"g/mol","Element"},
    {"Manganese atomic weight","Mn",54.938,"g/mol","Element"},
    {"Iron atomic weight","Fe",55.845,"g/mol","Element"},
    {"Cobalt atomic weight","Co",58.933,"g/mol","Element"},
    {"Nickel atomic weight","Ni",58.693,"g/mol","Element"},
    {"Copper atomic weight","Cu",63.546,"g/mol","Element"},
    {"Zinc atomic weight","Zn",65.38,"g/mol","Element"},
    {"Gallium atomic weight","Ga",69.723,"g/mol","Element"},
    {"Germanium atomic weight","Ge",72.630,"g/mol","Element"},
    {"Arsenic atomic weight","As",74.922,"g/mol","Element"},
    {"Selenium atomic weight","Se",78.971,"g/mol","Element"},
    {"Bromine atomic weight","Br",79.904,"g/mol","Element"},
    {"Krypton atomic weight","Kr",83.798,"g/mol","Element"},
    {"Rubidium atomic weight","Rb",85.468,"g/mol","Element"},
    {"Strontium atomic weight","Sr",87.62,"g/mol","Element"},
    {"Yttrium atomic weight","Y",88.906,"g/mol","Element"},
    {"Zirconium atomic weight","Zr",91.224,"g/mol","Element"},
    {"Niobium atomic weight","Nb",92.906,"g/mol","Element"},
    {"Molybdenum atomic weight","Mo",95.95,"g/mol","Element"},
    {"Technetium atomic weight","Tc",98.0,"g/mol","Element"},
    {"Ruthenium atomic weight","Ru",101.07,"g/mol","Element"},
    {"Rhodium atomic weight","Rh",102.91,"g/mol","Element"},
    {"Palladium atomic weight","Pd",106.42,"g/mol","Element"},
    {"Silver atomic weight","Ag",107.87,"g/mol","Element"},
    {"Cadmium atomic weight","Cd",112.41,"g/mol","Element"},
    {"Indium atomic weight","In",114.82,"g/mol","Element"},
    {"Tin atomic weight","Sn",118.71,"g/mol","Element"},
    {"Antimony atomic weight","Sb",121.76,"g/mol","Element"},
    {"Tellurium atomic weight","Te",127.60,"g/mol","Element"},
    {"Iodine atomic weight","I",126.90,"g/mol","Element"},
    {"Xenon atomic weight","Xe",131.29,"g/mol","Element"},
    {"Cesium atomic weight","Cs",132.91,"g/mol","Element"},
    {"Barium atomic weight","Ba",137.33,"g/mol","Element"},
    {"Lanthanum atomic weight","La",138.91,"g/mol","Element"},
    {"Cerium atomic weight","Ce",140.12,"g/mol","Element"},
    {"Praseodymium atomic weight","Pr",140.91,"g/mol","Element"},
    {"Neodymium atomic weight","Nd",144.24,"g/mol","Element"},
    {"Promethium atomic weight","Pm",145.0,"g/mol","Element"},
    {"Samarium atomic weight","Sm",150.36,"g/mol","Element"},
    {"Europium atomic weight","Eu",151.96,"g/mol","Element"},
    {"Gadolinium atomic weight","Gd",157.25,"g/mol","Element"},
    {"Terbium atomic weight","Tb",158.93,"g/mol","Element"},
    {"Dysprosium atomic weight","Dy",162.50,"g/mol","Element"},
    {"Holmium atomic weight","Ho",164.93,"g/mol","Element"},
    {"Erbium atomic weight","Er",167.26,"g/mol","Element"},
    {"Thulium atomic weight","Tm",168.93,"g/mol","Element"},
    {"Ytterbium atomic weight","Yb",173.05,"g/mol","Element"},
    {"Lutetium atomic weight","Lu",174.97,"g/mol","Element"},
    {"Hafnium atomic weight","Hf",178.49,"g/mol","Element"},
    {"Tantalum atomic weight","Ta",180.95,"g/mol","Element"},
    {"Tungsten atomic weight","W",183.84,"g/mol","Element"},
    {"Rhenium atomic weight","Re",186.21,"g/mol","Element"},
    {"Osmium atomic weight","Os",190.23,"g/mol","Element"},
    {"Iridium atomic weight","Ir",192.22,"g/mol","Element"},
    {"Platinum atomic weight","Pt",195.08,"g/mol","Element"},
    {"Gold atomic weight","Au",196.97,"g/mol","Element"},
    {"Mercury(Hg) atomic weight","Hg",200.59,"g/mol","Element"},
    {"Thallium atomic weight","Tl",204.38,"g/mol","Element"},
    {"Lead atomic weight","Pb",207.2,"g/mol","Element"},
    {"Bismuth atomic weight","Bi",208.98,"g/mol","Element"},
    {"Polonium atomic weight","Po",209.0,"g/mol","Element"},
    {"Astatine atomic weight","At",210.0,"g/mol","Element"},
    {"Radon atomic weight","Rn",222.0,"g/mol","Element"},
    {"Francium atomic weight","Fr",223.0,"g/mol","Element"},
    {"Radium atomic weight","Ra",226.0,"g/mol","Element"},
    {"Actinium atomic weight","Ac",227.0,"g/mol","Element"},
    {"Thorium atomic weight","Th",232.04,"g/mol","Element"},
    {"Protactinium atomic weight","Pa",231.04,"g/mol","Element"},
    {"Uranium atomic weight","U",238.03,"g/mol","Element"},
    {"Neptunium atomic weight","Np",237.0,"g/mol","Element"},
    {"Plutonium atomic weight","Pu",244.0,"g/mol","Element"},
    {"Americium atomic weight","Am",243.0,"g/mol","Element"},
    {"Curium atomic weight","Cm",247.0,"g/mol","Element"},
    {"Berkelium atomic weight","Bk",247.0,"g/mol","Element"},
    {"Californium atomic weight","Cf",251.0,"g/mol","Element"},
    {"Einsteinium atomic weight","Es",252.0,"g/mol","Element"},
    {"Fermium atomic weight","Fm",257.0,"g/mol","Element"},
    {"Mendelevium atomic weight","Md",258.0,"g/mol","Element"},
    {"Nobelium atomic weight","No",259.0,"g/mol","Element"},
    {"Lawrencium atomic weight","Lr",262.0,"g/mol","Element"},
    {"Rutherfordium atomic weight","Rf",267.0,"g/mol","Element"},
    {"Dubnium atomic weight","Db",268.0,"g/mol","Element"},
    {"Seaborgium atomic weight","Sg",271.0,"g/mol","Element"},
    {"Bohrium atomic weight","Bh",272.0,"g/mol","Element"},
    {"Hassium atomic weight","Hs",270.0,"g/mol","Element"},
    {"Meitnerium atomic weight","Mt",276.0,"g/mol","Element"},
    {"Darmstadtium atomic weight","Ds",281.0,"g/mol","Element"},
    {"Roentgenium atomic weight","Rg",280.0,"g/mol","Element"},
    {"Copernicium atomic weight","Cn",285.0,"g/mol","Element"},
    {"Nihonium atomic weight","Nh",286.0,"g/mol","Element"},
    {"Flerovium atomic weight","Fl",289.0,"g/mol","Element"},
    {"Moscovium atomic weight","Mc",290.0,"g/mol","Element"},
    {"Livermorium atomic weight","Lv",293.0,"g/mol","Element"},
    {"Tennessine atomic weight","Ts",294.0,"g/mol","Element"},
    {"Oganesson atomic weight","Og",294.0,"g/mol","Element"},
    {"Water melting point","Tm H2O",273.15,"K","Element"},
    {"Water boiling point (1 atm)","Tb H2O",373.15,"K","Element"},
    {"Water density at 4C","rho H2O",0.999972,"g/cm3","Element"},
    {"Water specific heat (20C)","cp H2O",4.1813,"J/g/K","Element"},
    {"Water heat of vaporization","Lv H2O",2257.0,"J/g","Element"},
    {"Ice heat of fusion","Lf H2O",334.0,"J/g","Element"},

    // ── 生物学定数 (30項目) ──
    {"Alanine molar mass","Ala",89.09,"g/mol","Biology"},
    {"Arginine molar mass","Arg",174.20,"g/mol","Biology"},
    {"Asparagine molar mass","Asn",132.12,"g/mol","Biology"},
    {"Aspartic acid molar mass","Asp",133.10,"g/mol","Biology"},
    {"Cysteine molar mass","Cys",121.16,"g/mol","Biology"},
    {"Glutamine molar mass","Gln",146.15,"g/mol","Biology"},
    {"Glutamic acid molar mass","Glu",147.13,"g/mol","Biology"},
    {"Glycine molar mass","Gly",75.07,"g/mol","Biology"},
    {"Histidine molar mass","His",155.16,"g/mol","Biology"},
    {"Isoleucine molar mass","Ile",131.17,"g/mol","Biology"},
    {"Leucine molar mass","Leu",131.17,"g/mol","Biology"},
    {"Lysine molar mass","Lys",146.19,"g/mol","Biology"},
    {"Methionine molar mass","Met",149.21,"g/mol","Biology"},
    {"Phenylalanine molar mass","Phe",165.19,"g/mol","Biology"},
    {"Proline molar mass","Pro",115.13,"g/mol","Biology"},
    {"Serine molar mass","Ser",105.09,"g/mol","Biology"},
    {"Threonine molar mass","Thr",119.12,"g/mol","Biology"},
    {"Tryptophan molar mass","Trp",204.23,"g/mol","Biology"},
    {"Tyrosine molar mass","Tyr",181.19,"g/mol","Biology"},
    {"Valine molar mass","Val",117.15,"g/mol","Biology"},
    {"Adenine molar mass","A",135.13,"g/mol","Biology"},
    {"Guanine molar mass","G",151.13,"g/mol","Biology"},
    {"Cytosine molar mass","C",111.10,"g/mol","Biology"},
    {"Thymine molar mass","T",126.11,"g/mol","Biology"},
    {"Uracil molar mass","U",112.09,"g/mol","Biology"},
    {"DNA double helix pitch","p_DNA",3.4e-9,"m","Biology"},
    {"E. coli diameter","d_Ec",1e-6,"m","Biology"},
    {"Human erythrocyte diameter","d_RBC",7e-6,"m","Biology"},
    {"Human ovum diameter","d_ova",1e-4,"m","Biology"},
    {"Bacterial gen. time (E.coli)","T_gen",1200.0,"s","Biology"},
};
static const int constN = sizeof(constDB) / sizeof(Constant);

static const char* PERIODIC[118] = {
    "H","He","Li","Be","B","C","N","O","F","Ne",
    "Na","Mg","Al","Si","P","S","Cl","Ar","K","Ca",
    "Sc","Ti","V","Cr","Mn","Fe","Co","Ni","Cu","Zn",
    "Ga","Ge","As","Se","Br","Kr","Rb","Sr","Y","Zr",
    "Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn",
    "Sb","Te","I","Xe","Cs","Ba","La","Ce","Pr","Nd",
    "Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb",
    "Lu","Hf","Ta","W","Re","Os","Ir","Pt","Au","Hg",
    "Tl","Pb","Bi","Po","At","Rn","Fr","Ra","Ac","Th",
    "Pa","U","Np","Pu","Am","Cm","Bk","Cf","Es","Fm",
    "Md","No","Lr","Rf","Db","Sg","Bh","Hs","Mt","Ds",
    "Rg","Cn","Nh","Fl","Mc","Lv","Ts","Og"
};
static const double ATOMIC_W[118] = {
    1.008,4.0026,6.94,9.0122,10.81,12.011,14.007,15.999,18.998,20.180,
    22.990,24.305,26.982,28.085,30.974,32.06,35.45,39.948,39.098,40.078,
    44.956,47.867,50.942,51.996,54.938,55.845,58.933,58.693,63.546,65.38,
    69.723,72.630,74.922,78.971,79.904,83.798,85.468,87.62,88.906,91.224,
    92.906,95.95,98.0,101.07,102.91,106.42,107.87,112.41,114.82,118.71,
    121.76,127.60,126.90,131.29,132.91,137.33,138.91,140.12,140.91,144.24,
    145.0,150.36,151.96,157.25,158.93,162.50,164.93,167.26,168.93,173.05,
    174.97,178.49,180.95,183.84,186.21,190.23,192.22,195.08,196.97,200.59,
    204.38,207.2,208.98,209.0,210.0,222.0,223.0,226.0,227.0,232.04,
    231.04,238.03,237.0,244.0,243.0,247.0,247.0,251.0,252.0,257.0,
    258.0,259.0,262.0,267.0,268.0,271.0,272.0,270.0,276.0,281.0,
    280.0,285.0,286.0,289.0,290.0,293.0,294.0,294.0
};

static const char* CONST_CATS[] = {
    "Physical","Math","EE","Astronomy","Planet","DwarfMoon",
    "Geo","Atmos","Ocean","Element","Biology"
};
static const int CONST_NCAT = sizeof(CONST_CATS) / sizeof(char*);

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 6 / 20 ─ 単位換算データベース (532 単位 / 20 カテゴリ)          ║
// ╚═══════════════════════════════════════════════════════════════════════╝

struct UnitConv {
    const char* name;
    const char* sym;
    double      factor;
    double      offset;
    int         cat;
};

enum UnitCat {
    UC_LENGTH=0, UC_AREA, UC_VOLUME, UC_MASS, UC_VELOCITY,
    UC_ACCEL, UC_FORCE, UC_PRESSURE, UC_ENERGY, UC_POWER,
    UC_TEMP, UC_ANGLE, UC_TIME, UC_DIGITAL, UC_FREQ,
    UC_FLOW, UC_DENSITY, UC_VISC, UC_RADIATION, UC_MISC,
    UC_NCAT
};

static const char* UNIT_CATS[] = {
    "Length","Area","Volume","Mass","Velocity",
    "Acceleration","Force","Pressure","Energy","Power",
    "Temperature","Angle","Time","Digital Info","Frequency",
    "Volumetric Flow","Density","Viscosity","Radiation","Misc Quantity"
};
static const int UNIT_NCAT = sizeof(UNIT_CATS) / sizeof(char*);

static const UnitConv unitDB[] PROGMEM = {
    // ── Length (62) ──
    {"meter","m",1.0,0,UC_LENGTH},
    {"kilometer","km",1000.0,0,UC_LENGTH},
    {"centimeter","cm",0.01,0,UC_LENGTH},
    {"millimeter","mm",0.001,0,UC_LENGTH},
    {"micrometer","um",1e-6,0,UC_LENGTH},
    {"nanometer","nm",1e-9,0,UC_LENGTH},
    {"picometer","pm",1e-12,0,UC_LENGTH},
    {"femtometer","fm",1e-15,0,UC_LENGTH},
    {"angstrom","A",1e-10,0,UC_LENGTH},
    {"inch","in",0.0254,0,UC_LENGTH},
    {"foot","ft",0.3048,0,UC_LENGTH},
    {"yard","yd",0.9144,0,UC_LENGTH},
    {"mile","mi",1609.344,0,UC_LENGTH},
    {"nautical mile","NM",1852.0,0,UC_LENGTH},
    {"shaku","shaku",0.303030303,0,UC_LENGTH},
    {"sun","sun",0.0303030303,0,UC_LENGTH},
    {"bu","bu",0.00303030303,0,UC_LENGTH},
    {"rin","rin",0.000303030303,0,UC_LENGTH},
    {"ken","ken",1.81818181,0,UC_LENGTH},
    {"cho","cho",109.0909,0,UC_LENGTH},
    {"ri","ri",3927.27272,0,UC_LENGTH},
    {"light-year","ly",9.4607304725808e15,0,UC_LENGTH},
    {"astronomical unit","AU",149597870700.0,0,UC_LENGTH},
    {"parsec","pc",3.08567758149137e16,0,UC_LENGTH},
    {"Planck length","l_P",1.616255e-35,0,UC_LENGTH},
    {"Bohr radius","a0",5.29177210903e-11,0,UC_LENGTH},
    {"furlong","fur",201.168,0,UC_LENGTH},
    {"chain","ch",20.1168,0,UC_LENGTH},
    {"rod","rd",5.0292,0,UC_LENGTH},
    {"fathom","fath",1.8288,0,UC_LENGTH},
    {"hand","hand",0.1016,0,UC_LENGTH},
    {"span","span",0.2286,0,UC_LENGTH},
    {"cubit","cubit",0.4572,0,UC_LENGTH},
    {"league","lea",4828.032,0,UC_LENGTH},
    {"mil","mil",2.54e-5,0,UC_LENGTH},
    {"microinch","uin",2.54e-8,0,UC_LENGTH},
    {"hiro","hiro",1.51515,0,UC_LENGTH},
    {"jo","jo",3.030303,0,UC_LENGTH},
    {"mou","mou",3.0303e-5,0,UC_LENGTH},
    {"French pied","pied",0.3248406,0,UC_LENGTH},
    {"German Fuss","Fuss",0.31385,0,UC_LENGTH},
    {"Chinese chi","chi",0.3333,0,UC_LENGTH},
    {"point (DTP)","pt",3.52778e-4,0,UC_LENGTH},
    {"pica (DTP)","pica",4.23333e-3,0,UC_LENGTH},
    {"twip","twip",1.76389e-5,0,UC_LENGTH},
    {"thou (mil)","thou",2.54e-5,0,UC_LENGTH},
    {"caliber","cal",2.54e-4,0,UC_LENGTH},
    {"US survey foot","ft_us",0.304800609601,0,UC_LENGTH},
    {"US survey mile","mi_us",1609.34721869,0,UC_LENGTH},
    {"Roman mile","mi_rom",1479.0,0,UC_LENGTH},
    {"verst","verst",1066.8,0,UC_LENGTH},
    {"li (Chinese)","li_cn",500.0,0,UC_LENGTH},
    {"cun (Chinese)","cun",0.0333333,0,UC_LENGTH},
    {"zhang (Chinese)","zhang",3.33333,0,UC_LENGTH},
    {"barleycorn","barley",0.00846667,0,UC_LENGTH},
    {"nail","nail",0.05715,0,UC_LENGTH},
    {"finger","finger",0.1143,0,UC_LENGTH},
    {"palm","palm",0.0762,0,UC_LENGTH},
    {"pace","pace",0.762,0,UC_LENGTH},
    {"cable","cable",185.2,0,UC_LENGTH},
    {"link","link",0.201168,0,UC_LENGTH},
    {"nautical league","nlea",5556.0,0,UC_LENGTH},

    // ── Area (45) ──
    {"square meter","m2",1.0,0,UC_AREA},
    {"square kilometer","km2",1e6,0,UC_AREA},
    {"square centimeter","cm2",1e-4,0,UC_AREA},
    {"square millimeter","mm2",1e-6,0,UC_AREA},
    {"hectare","ha",10000.0,0,UC_AREA},
    {"are","a",100.0,0,UC_AREA},
    {"acre","ac",4046.8564224,0,UC_AREA},
    {"square inch","in2",0.00064516,0,UC_AREA},
    {"square foot","ft2",0.09290304,0,UC_AREA},
    {"square yard","yd2",0.83612736,0,UC_AREA},
    {"square mile","mi2",2589988.110336,0,UC_AREA},
    {"tsubo","tsubo",3.305785,0,UC_AREA},
    {"tatami chukyo","jo_ck",1.6562,0,UC_AREA},
    {"tatami edo","jo_ed",1.5488,0,UC_AREA},
    {"tatami kyoma","jo_ky",1.8245,0,UC_AREA},
    {"se","se",99.1736,0,UC_AREA},
    {"tan","tan",991.736,0,UC_AREA},
    {"cho area","cho_a",9917.36,0,UC_AREA},
    {"bu area","bu_a",3.305785,0,UC_AREA},
    {"rood","rood",1011.7141,0,UC_AREA},
    {"barn","b",1e-28,0,UC_AREA},
    {"section","sec_a",2589988.110336,0,UC_AREA},
    {"township","twp",93239571.972096,0,UC_AREA},
    {"homestead","hmd",647497.0275584,0,UC_AREA},
    {"square chain","ch2",404.68564224,0,UC_AREA},
    {"square rod","rd2",25.29285264,0,UC_AREA},
    {"square fathom","fath2",3.34450944,0,UC_AREA},
    {"circular mil","cmil",5.067075e-10,0,UC_AREA},
    {"dunam","dun",1000.0,0,UC_AREA},
    {"square shaku","shaku2",0.0918274,0,UC_AREA},
    {"square ken","ken2",3.305785,0,UC_AREA},
    {"square cho","cho2",11900.835,0,UC_AREA},
    {"tagwerk","tag",3407.27,0,UC_AREA},
    {"hide","hide",485622.770,0,UC_AREA},
    {"virgate","virgate",121405.69,0,UC_AREA},
    {"carucate","carucate",485622.770,0,UC_AREA},
    {"morgen","morgen",2500.0,0,UC_AREA},
    {"oxgang","oxgang",60702.85,0,UC_AREA},
    {"bovate","bovate",60702.85,0,UC_AREA},
    {"sabin","sabin",0.09290304,0,UC_AREA},
    {"darcy","darcy",9.869233e-13,0,UC_AREA},
    {"rai","rai",1600.0,0,UC_AREA},
    {"circular inch","cin",5.067075e-4,0,UC_AREA},
    {"square micrometre","um2",1e-12,0,UC_AREA},
    {"square nanometer","nm2",1e-18,0,UC_AREA},

    // ── Volume (57) ──
    {"cubic meter","m3",1.0,0,UC_VOLUME},
    {"cubic centimeter","cm3",1e-6,0,UC_VOLUME},
    {"cubic millimeter","mm3",1e-9,0,UC_VOLUME},
    {"liter","L",0.001,0,UC_VOLUME},
    {"milliliter","mL",1e-6,0,UC_VOLUME},
    {"kiloliter","kL",1.0,0,UC_VOLUME},
    {"US gallon","gal_US",0.003785411784,0,UC_VOLUME},
    {"UK gallon","gal_UK",0.00454609,0,UC_VOLUME},
    {"US quart","qt_US",0.000946352946,0,UC_VOLUME},
    {"UK quart","qt_UK",0.0011365225,0,UC_VOLUME},
    {"US pint","pt_US",0.000473176473,0,UC_VOLUME},
    {"UK pint","pt_UK",0.00056826125,0,UC_VOLUME},
    {"US cup","cup_US",0.0002365882,0,UC_VOLUME},
    {"US tablespoon","tbsp_US",1.4787e-5,0,UC_VOLUME},
    {"US teaspoon","tsp_US",4.92892e-6,0,UC_VOLUME},
    {"US fluid ounce","floz_US",2.95735e-5,0,UC_VOLUME},
    {"UK fluid ounce","floz_UK",2.84131e-5,0,UC_VOLUME},
    {"oil barrel","bbl_oil",0.158987294928,0,UC_VOLUME},
    {"US barrel","bbl_US",0.119240471196,0,UC_VOLUME},
    {"UK barrel","bbl_UK",0.16365924,0,UC_VOLUME},
    {"US bushel","bu_US",0.0352390704,0,UC_VOLUME},
    {"UK bushel","bu_UK",0.03636872,0,UC_VOLUME},
    {"sho","sho",0.001803907,0,UC_VOLUME},
    {"go","go",0.0001803907,0,UC_VOLUME},
    {"shaku vol","shaku_v",1.803907e-5,0,UC_VOLUME},
    {"koku","koku",0.1803907,0,UC_VOLUME},
    {"to","to",0.01803907,0,UC_VOLUME},
    {"cubic inch","in3",1.6387064e-5,0,UC_VOLUME},
    {"cubic foot","ft3",0.028316846592,0,UC_VOLUME},
    {"cubic yard","yd3",0.764554857984,0,UC_VOLUME},
    {"cord","cord",3.624556363776,0,UC_VOLUME},
    {"cord foot","cordft",0.453069545,0,UC_VOLUME},
    {"board foot","bdft",0.002359737,0,UC_VOLUME},
    {"acre-foot","acft",1233.48183754752,0,UC_VOLUME},
    {"US peck","peck_US",0.0088097676,0,UC_VOLUME},
    {"UK peck","peck_UK",0.00909218,0,UC_VOLUME},
    {"US gill","gill_US",0.000118294,0,UC_VOLUME},
    {"UK gill","gill_UK",0.0001420653,0,UC_VOLUME},
    {"drum","drum",0.208197649,0,UC_VOLUME},
    {"drop","drop",5e-8,0,UC_VOLUME},
    {"microliter","uL",1e-9,0,UC_VOLUME},
    {"US minim","min_US",6.16115e-8,0,UC_VOLUME},
    {"UK minim","min_UK",5.91939e-8,0,UC_VOLUME},
    {"US fluid dram","fl_dr_US",3.69669e-6,0,UC_VOLUME},
    {"UK fluid dram","fl_dr_UK",3.55163e-6,0,UC_VOLUME},
    {"hogshead","hogshead",0.238481,0,UC_VOLUME},
    {"US dry pint","dpt_US",0.000550610,0,UC_VOLUME},
    {"US dry quart","dqt_US",0.00110122,0,UC_VOLUME},
    {"firkin","firkin",0.04091481,0,UC_VOLUME},
    {"kilderkin","kilderkin",0.08182962,0,UC_VOLUME},
    {"perch","perch",0.700841953,0,UC_VOLUME},
    {"cubic rod","rd3",0.02831685,0,UC_VOLUME},
    {"cubic mile","mi3",4.1681818e9,0,UC_VOLUME},
    {"US dry gallon","dgal_US",0.00440488,0,UC_VOLUME},
    {"UK dry gallon","dgal_UK",0.00454609,0,UC_VOLUME},
    {"teaspoon (metric)","tsp_m",5e-6,0,UC_VOLUME},
    {"tablespoon (metric)","tbsp_m",15e-6,0,UC_VOLUME},

    // ── Mass (51) ──
    {"kilogram","kg",1.0,0,UC_MASS},
    {"gram","g",0.001,0,UC_MASS},
    {"milligram","mg",1e-6,0,UC_MASS},
    {"microgram","ug",1e-9,0,UC_MASS},
    {"metric ton","t",1000.0,0,UC_MASS},
    {"pound","lb",0.45359237,0,UC_MASS},
    {"ounce","oz",0.028349523125,0,UC_MASS},
    {"grain","gr",6.479891e-5,0,UC_MASS},
    {"stone","st",6.35029318,0,UC_MASS},
    {"UK hundredweight","cwt_UK",50.80234544,0,UC_MASS},
    {"US hundredweight","cwt_US",45.359237,0,UC_MASS},
    {"long ton","lt",1016.0469088,0,UC_MASS},
    {"short ton","sht",907.18474,0,UC_MASS},
    {"kan","kan",3.75,0,UC_MASS},
    {"momme","momme",0.00375,0,UC_MASS},
    {"kin","kin",0.6,0,UC_MASS},
    {"ryo","ryo",0.0375,0,UC_MASS},
    {"bu mass","bu_m",0.000375,0,UC_MASS},
    {"shu","shu",0.0015625,0,UC_MASS},
    {"carat","ct",0.0002,0,UC_MASS},
    {"dram","dr",0.001771845,0,UC_MASS},
    {"scruple","s_ap",0.001295978,0,UC_MASS},
    {"pennyweight","dwt",0.001555174,0,UC_MASS},
    {"troy ounce","oz_t",0.0311034768,0,UC_MASS},
    {"troy pound","lb_t",0.3732417216,0,UC_MASS},
    {"slug","slug",14.59390294,0,UC_MASS},
    {"atomic mass unit","u",1.66053906660e-27,0,UC_MASS},
    {"kiloton","kt",1e6,0,UC_MASS},
    {"megaton","Mt",1e9,0,UC_MASS},
    {"gamma","gamma",1e-9,0,UC_MASS},
    {"tael","tael",0.0377994,0,UC_MASS},
    {"hyakume","hyakume",0.375,0,UC_MASS},
    {"seer","seer",0.93310,0,UC_MASS},
    {"jin","jin",0.5,0,UC_MASS},
    {"liang","liang",0.05,0,UC_MASS},
    {"quintal","q",100.0,0,UC_MASS},
    {"bag (cement)","bag_c",42.6377,0,UC_MASS},
    {"bale","bale",226.796,0,UC_MASS},
    {"hundredweight","cwt",50.80234544,0,UC_MASS},
    {"metric carat","ct_m",0.0002,0,UC_MASS},
    {"assay ton","AT",0.0291667,0,UC_MASS},
    {"firkin (mass)","firkin_m",40.8233,0,UC_MASS},
    {"clove","clove",3.17515,0,UC_MASS},
    {"tod","tod",12.7006,0,UC_MASS},
    {"wey","wey",114.305,0,UC_MASS},
    {"sack","sack",114.305,0,UC_MASS},
    {"quarter","qtr",12.7006,0,UC_MASS},
    {"packen","packen",136.078,0,UC_MASS},
    {"scruple (ap)","s_ap2",0.00129598,0,UC_MASS},
    {"metric grain","gr_m",5e-5,0,UC_MASS},
    {"dalton","Da",1.66053906660e-27,0,UC_MASS},

    // ── Velocity (26) ──
    {"meter per second","m/s",1.0,0,UC_VELOCITY},
    {"kilometer per hour","km/h",0.27777777777778,0,UC_VELOCITY},
    {"miles per hour","mph",0.44704,0,UC_VELOCITY},
    {"knot","kn",0.51444444,0,UC_VELOCITY},
    {"feet per second","ft/s",0.3048,0,UC_VELOCITY},
    {"inches per second","in/s",0.0254,0,UC_VELOCITY},
    {"Mach (sea level)","Mach",340.29,0,UC_VELOCITY},
    {"speed of light","c",299792458.0,0,UC_VELOCITY},
    {"yards per second","yd/s",0.9144,0,UC_VELOCITY},
    {"cm per second","cm/s",0.01,0,UC_VELOCITY},
    {"mm per second","mm/s",0.001,0,UC_VELOCITY},
    {"km per second","km/s",1000.0,0,UC_VELOCITY},
    {"miles per second","mi/s",1609.344,0,UC_VELOCITY},
    {"feet per minute","ft/min",0.00508,0,UC_VELOCITY},
    {"meter per minute","m/min",0.0166666667,0,UC_VELOCITY},
    {"league per hour","lea/h",1.34112,0,UC_VELOCITY},
    {"fathom per second","fath/s",1.8288,0,UC_VELOCITY},
    {"furlong per fortnight","fur/ftn",1.66309e-4,0,UC_VELOCITY},
    {"speed of sound (water)","c_w",1497.0,0,UC_VELOCITY},
    {"speed of sound (steel)","c_s",5960.0,0,UC_VELOCITY},
    {"walking speed","walk",1.4,0,UC_VELOCITY},
    {"km per minute","km/min",16.6667,0,UC_VELOCITY},
    {"mile per minute","mi/min",26.8224,0,UC_VELOCITY},
    {"furlong per day","fur/d",0.00232828,0,UC_VELOCITY},
    {"inch per year","in/yr",8.04864e-10,0,UC_VELOCITY},
    {"km per day","km/d",0.0115741,0,UC_VELOCITY},

    // ── Acceleration (14) ──
    {"meter per second^2","m/s2",1.0,0,UC_ACCEL},
    {"standard gravity","g0",9.80665,0,UC_ACCEL},
    {"galileo","Gal",0.01,0,UC_ACCEL},
    {"feet per second^2","ft/s2",0.3048,0,UC_ACCEL},
    {"inches per second^2","in/s2",0.0254,0,UC_ACCEL},
    {"km/h per s","km/h/s",0.27777778,0,UC_ACCEL},
    {"mph per s","mph/s",0.44704,0,UC_ACCEL},
    {"knot per s","kn/s",0.51444444,0,UC_ACCEL},
    {"cm per second^2","cm/s2",0.01,0,UC_ACCEL},
    {"foot per hour per second","ft/h/s",8.46667e-5,0,UC_ACCEL},
    {"mile per hour per second","mph/s_2",0.44704,0,UC_ACCEL},
    {"knot per hour","kn/h",1.42873e-4,0,UC_ACCEL},
    {"m per hour per second","m/h/s",2.77778e-4,0,UC_ACCEL},
    {"gravity (Moon)","g_moon",1.62,0,UC_ACCEL},

    // ── Force (20) ──
    {"newton","N",1.0,0,UC_FORCE},
    {"dyne","dyn",1e-5,0,UC_FORCE},
    {"kilogram-force","kgf",9.80665,0,UC_FORCE},
    {"pound-force","lbf",4.4482216152605,0,UC_FORCE},
    {"poundal","pdl",0.138254954,0,UC_FORCE},
    {"ton-force (metric)","tf",9806.65,0,UC_FORCE},
    {"long ton-force","ltf",9964.0164181707,0,UC_FORCE},
    {"short ton-force","stf",8896.443230521,0,UC_FORCE},
    {"ounce-force","ozf",0.27801385095,0,UC_FORCE},
    {"grain-force","grf",0.000635460091,0,UC_FORCE},
    {"sthene","sn",1000.0,0,UC_FORCE},
    {"kip","kip",4448.2216152605,0,UC_FORCE},
    {"millinewton","mN",0.001,0,UC_FORCE},
    {"kilopound","klb",4448.2216152605,0,UC_FORCE},
    {"micronewton","uN",1e-6,0,UC_FORCE},
    {"meganewton","MN",1e6,0,UC_FORCE},
    {"kilonewton","kN",1000.0,0,UC_FORCE},
    {"decanewton","daN",10.0,0,UC_FORCE},
    {"pond","pond",0.00980665,0,UC_FORCE},
    {"grave","grave",9.80665,0,UC_FORCE},

    // ── Pressure (33) ──
    {"pascal","Pa",1.0,0,UC_PRESSURE},
    {"hectopascal","hPa",100.0,0,UC_PRESSURE},
    {"kilopascal","kPa",1000.0,0,UC_PRESSURE},
    {"megapascal","MPa",1e6,0,UC_PRESSURE},
    {"atmosphere","atm",101325.0,0,UC_PRESSURE},
    {"bar","bar",100000.0,0,UC_PRESSURE},
    {"millibar","mbar",100.0,0,UC_PRESSURE},
    {"torr","Torr",133.322368421,0,UC_PRESSURE},
    {"mmHg","mmHg",133.322387415,0,UC_PRESSURE},
    {"mmH2O","mmH2O",9.80665,0,UC_PRESSURE},
    {"psi","psi",6894.757293168,0,UC_PRESSURE},
    {"ksi","ksi",6894757.293168,0,UC_PRESSURE},
    {"kgf/cm^2","kgf/cm2",98066.5,0,UC_PRESSURE},
    {"psf","psf",47.88025898,0,UC_PRESSURE},
    {"barye","barye",0.1,0,UC_PRESSURE},
    {"pieze","pz",1000.0,0,UC_PRESSURE},
    {"inHg","inHg",3386.389,0,UC_PRESSURE},
    {"inH2O","inH2O",249.0889,0,UC_PRESSURE},
    {"ftH2O","ftH2O",2989.0668,0,UC_PRESSURE},
    {"technical atm","at",98066.5,0,UC_PRESSURE},
    {"millitorr","mTorr",0.13332237,0,UC_PRESSURE},
    {"microbar","ubar",0.1,0,UC_PRESSURE},
    {"gigapascal","GPa",1e9,0,UC_PRESSURE},
    {"kip per square inch","ksi_2",6894757.293168,0,UC_PRESSURE},
    {"kip per square foot","ksf",47880.25898,0,UC_PRESSURE},
    {"ton-force per square foot","tsf",95760.51796,0,UC_PRESSURE},
    {"ton-force per square inch","tsi",13789514.586,0,UC_PRESSURE},
    {"meter of water","mH2O",9806.65,0,UC_PRESSURE},
    {"inch of water (4C)","inH2O_4C",249.082,0,UC_PRESSURE},
    {"cm of mercury","cmHg",1333.22387415,0,UC_PRESSURE},
    {"cm of water","cmH2O",98.0665,0,UC_PRESSURE},
    {"kgf per square millimeter","kgf/mm2",9806650.0,0,UC_PRESSURE},
    {"newton per square millimeter","N/mm2",1e6,0,UC_PRESSURE},

    // ── Energy (39) ──
    {"joule","J",1.0,0,UC_ENERGY},
    {"kilojoule","kJ",1000.0,0,UC_ENERGY},
    {"calorie","cal",4.184,0,UC_ENERGY},
    {"kilocalorie","kcal",4184.0,0,UC_ENERGY},
    {"electron volt","eV",1.602176634e-19,0,UC_ENERGY},
    {"watt-second","Ws",1.0,0,UC_ENERGY},
    {"watt-hour","Wh",3600.0,0,UC_ENERGY},
    {"kilowatt-hour","kWh",3.6e6,0,UC_ENERGY},
    {"erg","erg",1e-7,0,UC_ENERGY},
    {"BTU (IT)","BTU_IT",1055.05585,0,UC_ENERGY},
    {"BTU (th)","BTU_th",1054.35026,0,UC_ENERGY},
    {"thermie","th",4.1868e6,0,UC_ENERGY},
    {"foot-pound-force","ft lbf",1.3558179483314,0,UC_ENERGY},
    {"inch-pound-force","in lbf",0.112984829,0,UC_ENERGY},
    {"megajoule","MJ",1e6,0,UC_ENERGY},
    {"gigajoule","GJ",1e9,0,UC_ENERGY},
    {"terajoule","TJ",1e12,0,UC_ENERGY},
    {"horsepower-hour","hp h",2.6845e6,0,UC_ENERGY},
    {"ton TNT","tTNT",4.184e9,0,UC_ENERGY},
    {"kiloton (energy)","kt_E",4.184e12,0,UC_ENERGY},
    {"megaerg","Merg",0.1,0,UC_ENERGY},
    {"liter-atmosphere","L atm",101.325,0,UC_ENERGY},
    {"hartree","Ha",4.3597447222071e-18,0,UC_ENERGY},
    {"rydberg","Ry",2.1798723611035e-18,0,UC_ENERGY},
    {"kilopond meter","kpm",9.80665,0,UC_ENERGY},
    {"coal energy (CCE)","CCE",29.3076e6,0,UC_ENERGY},
    {"quad","quad",1.05505585262e18,0,UC_ENERGY},
    {"foe","foe",1e44,0,UC_ENERGY},
    {"foot-poundal","ftpdl",0.042140110,0,UC_ENERGY},
    {"therm (US)","thm_US",1.054804e8,0,UC_ENERGY},
    {"therm (EU)","thm_EU",1.05506e8,0,UC_ENERGY},
    {"cubic foot of nat gas","cf_ng",1.05506e6,0,UC_ENERGY},
    {"barrel of oil equiv","boe",6.1178632e9,0,UC_ENERGY},
    {"tonne of oil equiv","toe",4.1868e10,0,UC_ENERGY},
    {"watt-day","Wd",86400.0,0,UC_ENERGY},
    {"kilocalorie (IT)","kcal_IT",4186.8,0,UC_ENERGY},
    {"kilocalorie (th)","kcal_th",4184.0,0,UC_ENERGY},
    {"electron mass energy","E_me",8.1871057769e-14,0,UC_ENERGY},
    {"proton mass energy","E_mp",1.5032776158e-10,0,UC_ENERGY},

    // ── Power (24) ──
    {"watt","W",1.0,0,UC_POWER},
    {"kilowatt","kW",1000.0,0,UC_POWER},
    {"megawatt","MW",1e6,0,UC_POWER},
    {"metric horsepower","PS",735.49875,0,UC_POWER},
    {"horsepower","hp",745.69987158227,0,UC_POWER},
    {"cv","cv",735.49875,0,UC_POWER},
    {"ft lbf per s","ft lbf/s",1.3558179483314,0,UC_POWER},
    {"ft lbf per min","ft lbf/min",0.022596966,0,UC_POWER},
    {"calorie per s","cal/s",4.184,0,UC_POWER},
    {"kcal per h","kcal/h",1.16222222,0,UC_POWER},
    {"BTU per h","BTU/h",0.293071,0,UC_POWER},
    {"ton of refrig","TR",3516.853,0,UC_POWER},
    {"erg per s","erg/s",1e-7,0,UC_POWER},
    {"kgf m per s","kgf m/s",9.80665,0,UC_POWER},
    {"volt-ampere","VA",1.0,0,UC_POWER},
    {"gigawatt","GW",1e9,0,UC_POWER},
    {"milliwatt","mW",0.001,0,UC_POWER},
    {"microwatt","uW",1e-6,0,UC_POWER},
    {"volt-ampere reactive","VAr",1.0,0,UC_POWER},
    {"boiler horsepower","bhp",9809.5,0,UC_POWER},
    {"electric horsepower","ehp",746.0,0,UC_POWER},
    {"water horsepower","whp",746.043,0,UC_POWER},
    {"air horsepower","ahp",745.7,0,UC_POWER},
    {"kilocalorie per minute","kcal/min",69.7333,0,UC_POWER},

    // ── Temperature (8) ──
    {"Celsius","C",1.0,273.15,UC_TEMP},
    {"Fahrenheit","F",5.0/9.0,459.67*5.0/9.0,UC_TEMP},
    {"kelvin","K",1.0,0,UC_TEMP},
    {"Rankine","R_temp",5.0/9.0,0,UC_TEMP},
    {"Reaumur","Re",1.25,273.15,UC_TEMP},
    {"Newton scale","N_t",100.0/33.0,273.15,UC_TEMP},
    {"Romer scale","Ro",40.0/21.0,273.15 + 7.5*40.0/21.0,UC_TEMP},
    {"Delisle scale","De",-2.0/3.0,373.15,UC_TEMP},

    // ── Angle (12) ──
    {"degree","deg",M_PI/180.0,0,UC_ANGLE},
    {"radian","rad",1.0,0,UC_ANGLE},
    {"gradian","grad",M_PI/200.0,0,UC_ANGLE},
    {"arcminute","arcmin",M_PI/10800.0,0,UC_ANGLE},
    {"arcsecond","arcsec",M_PI/648000.0,0,UC_ANGLE},
    {"mil (angle)","mil_a",M_PI/3200.0,0,UC_ANGLE},
    {"revolution","rev",2*M_PI,0,UC_ANGLE},
    {"turn","turn",2*M_PI,0,UC_ANGLE},
    {"quadrant","quad",M_PI/2.0,0,UC_ANGLE},
    {"sextant","sext",M_PI/3.0,0,UC_ANGLE},
    {"octant","oct",M_PI/4.0,0,UC_ANGLE},
    {"sign (angle)","sign_a",M_PI/6.0,0,UC_ANGLE},

    // ── Time (20) ──
    {"second","s",1.0,0,UC_TIME},
    {"minute","min",60.0,0,UC_TIME},
    {"hour","h",3600.0,0,UC_TIME},
    {"day","d",86400.0,0,UC_TIME},
    {"week","wk",604800.0,0,UC_TIME},
    {"month (avg)","mo",2629746.0,0,UC_TIME},
    {"year (365 d)","yr",31536000.0,0,UC_TIME},
    {"tropical year","yr_t",31556925.19,0,UC_TIME},
    {"millisecond","ms",0.001,0,UC_TIME},
    {"microsecond","us",1e-6,0,UC_TIME},
    {"nanosecond","ns",1e-9,0,UC_TIME},
    {"picosecond","ps_t",1e-12,0,UC_TIME},
    {"fortnight","ftn",1209600.0,0,UC_TIME},
    {"century","c_t",3155760000.0,0,UC_TIME},
    {"shake","shake",1e-8,0,UC_TIME},
    {"jiffy","jiffy",0.01,0,UC_TIME},
    {"lustrum","lustrum",1.57785e8,0,UC_TIME},
    {"decade","decade",3.15576e8,0,UC_TIME},
    {"millennium","mil_a2",3.15576e10,0,UC_TIME},
    {"sidereal day","sd",86164.0905,0,UC_TIME},

    // ── Digital (25) ──
    {"bit","bit",1.0,0,UC_DIGITAL},
    {"byte","B",8.0,0,UC_DIGITAL},
    {"kilobyte (1024)","KB",8192.0,0,UC_DIGITAL},
    {"megabyte (1024)","MB",8388608.0,0,UC_DIGITAL},
    {"gigabyte (1024)","GB",8589934592.0,0,UC_DIGITAL},
    {"terabyte (1024)","TB",8.79609302208e12,0,UC_DIGITAL},
    {"kibibyte","KiB",8192.0,0,UC_DIGITAL},
    {"mebibyte","MiB",8388608.0,0,UC_DIGITAL},
    {"gibibyte","GiB",8589934592.0,0,UC_DIGITAL},
    {"tebibyte","TiB",8.79609302208e12,0,UC_DIGITAL},
    {"petabyte (1024)","PB",9.0071992547410e15,0,UC_DIGITAL},
    {"exabyte (1024)","EB",9.2233720368548e18,0,UC_DIGITAL},
    {"pebibyte","PiB",9.0071992547410e15,0,UC_DIGITAL},
    {"exbibyte","EiB",9.2233720368548e18,0,UC_DIGITAL},
    {"SI kilobyte","kB",8000.0,0,UC_DIGITAL},
    {"SI megabyte","MB_SI",8e6,0,UC_DIGITAL},
    {"SI gigabyte","GB_SI",8e9,0,UC_DIGITAL},
    {"SI terabyte","TB_SI",8e12,0,UC_DIGITAL},
    {"word (16-bit)","word",16.0,0,UC_DIGITAL},
    {"nibble","nib",4.0,0,UC_DIGITAL},
    {"zettabyte","ZB",9.2233720368548e21,0,UC_DIGITAL},
    {"yottabyte","YB",9.2233720368548e24,0,UC_DIGITAL},
    {"zebibyte","ZiB",9.4447329657393e21,0,UC_DIGITAL},
    {"yobibyte","YiB",9.671406556917e24,0,UC_DIGITAL},
    {"floppy (3.5in)","floppy",1.47456e7,0,UC_DIGITAL},

    // ── Frequency (12) ──
    {"hertz","Hz",1.0,0,UC_FREQ},
    {"kilohertz","kHz",1000.0,0,UC_FREQ},
    {"megahertz","MHz",1e6,0,UC_FREQ},
    {"gigahertz","GHz",1e9,0,UC_FREQ},
    {"terahertz","THz",1e12,0,UC_FREQ},
    {"rpm","rpm",0.01666666667,0,UC_FREQ},
    {"rps","rps",1.0,0,UC_FREQ},
    {"rad per s","rad/s",0.15915494309,0,UC_FREQ},
    {"petahertz","PHz",1e15,0,UC_FREQ},
    {"exahertz","EHz",1e18,0,UC_FREQ},
    {"beat per minute","bpm",0.0166666667,0,UC_FREQ},
    {"cycle per second","cps",1.0,0,UC_FREQ},

    // ── Volumetric Flow (20) ──
    {"cubic meter per s","m3/s",1.0,0,UC_FLOW},
    {"cubic meter per h","m3/h",0.000277778,0,UC_FLOW},
    {"liter per s","L/s",0.001,0,UC_FLOW},
    {"liter per min","L/min",1.66667e-5,0,UC_FLOW},
    {"liter per h","L/h",2.77778e-7,0,UC_FLOW},
    {"US gallon per min","gpm",6.30902e-5,0,UC_FLOW},
    {"US gallon per h","gph",1.05150e-6,0,UC_FLOW},
    {"cusec","cusec",0.0283168466,0,UC_FLOW},
    {"cubic feet per min","CFM",0.000471947,0,UC_FLOW},
    {"cubic inch per s","in3/s",1.6387e-5,0,UC_FLOW},
    {"acre-ft per h","acft/h",0.34257851,0,UC_FLOW},
    {"barrel per day","BPD",1.84013e-6,0,UC_FLOW},
    {"mL per min","mL/min",1.66667e-8,0,UC_FLOW},
    {"uL per s","uL/s",1e-9,0,UC_FLOW},
    {"miner's inch","min_in",6.30902e-5,0,UC_FLOW},
    {"cubic yard per min","yd3/min",0.0127426,0,UC_FLOW},
    {"acre-foot per day","acft/d",0.0142764,0,UC_FLOW},
    {"Sverdrup","Sv_fl",1e6,0,UC_FLOW},
    {"lambda","lambda",1e-9,0,UC_FLOW},
    {"cubic foot per s","cfs",0.0283168,0,UC_FLOW},

    // ── Density (20) ──
    {"kg per m^3","kg/m3",1.0,0,UC_DENSITY},
    {"g per cm^3","g/cm3",1000.0,0,UC_DENSITY},
    {"lb per ft^3","lb/ft3",16.018463,0,UC_DENSITY},
    {"lb per in^3","lb/in3",27679.905,0,UC_DENSITY},
    {"lb per US gal","lb/galUS",119.826,0,UC_DENSITY},
    {"lb per UK gal","lb/galUK",99.776,0,UC_DENSITY},
    {"g per L","g/L",1.0,0,UC_DENSITY},
    {"mg per L","mg/L",0.001,0,UC_DENSITY},
    {"specific gravity","SG",1000.0,0,UC_DENSITY},
    {"kg per L","kg/L",1000.0,0,UC_DENSITY},
    {"t per m^3","t/m3",1000.0,0,UC_DENSITY},
    {"oz per in^3","oz/in3",1729.99404,0,UC_DENSITY},
    {"slug per ft^3","slug/ft3",515.378818,0,UC_DENSITY},
    {"degrees API","API",141.5,0,UC_DENSITY},
    {"degrees Baume","Be",145.0,0,UC_DENSITY},
    {"Planck density","rho_P",5.155e96,0,UC_DENSITY},
    {"water density (4C)","rho_w",999.972,0,UC_DENSITY},
    {"air density (STP)","rho_a",1.2922,0,UC_DENSITY},
    {"mercury density","rho_Hg",13534.0,0,UC_DENSITY},
    {"mean Earth density","rho_E",5514.0,0,UC_DENSITY},

    // ── Viscosity (15) ──
    {"pascal-second","Pa s",1.0,0,UC_VISC},
    {"poise","P",0.1,0,UC_VISC},
    {"centipoise","cP",0.001,0,UC_VISC},
    {"stokes","St",0.0001,0,UC_VISC},
    {"centistokes","cSt",1e-6,0,UC_VISC},
    {"kgf s per m^2","kgf s/m2",9.80665,0,UC_VISC},
    {"lb per ft per s","lb/ft/s",1.488164,0,UC_VISC},
    {"slug per ft per s","slug/ft/s",47.88026,0,UC_VISC},
    {"m^2 per s","m2/s",1.0,0,UC_VISC},
    {"ft^2 per s","ft2/s",0.092903,0,UC_VISC},
    {"in^2 per s","in2/s",0.00064516,0,UC_VISC},
    {"reyn","reyn",6894.757293168,0,UC_VISC},
    {"micropoise","uP",1e-7,0,UC_VISC},
    {"poundal-sec per ft^2","pdl s/ft2",1.48816,0,UC_VISC},
    {"pound per foot hour","lb/ft/h",0.000413379,0,UC_VISC},

    // ── Radiation (13) ──
    {"becquerel","Bq",1.0,0,UC_RADIATION},
    {"curie","Ci",3.7e10,0,UC_RADIATION},
    {"gray","Gy",1.0,0,UC_RADIATION},
    {"rad","rad_r",0.01,0,UC_RADIATION},
    {"sievert","Sv",1.0,0,UC_RADIATION},
    {"rem","rem",0.01,0,UC_RADIATION},
    {"roentgen","R",0.000258,0,UC_RADIATION},
    {"coulomb per kg","C/kg",1.0,0,UC_RADIATION},
    {"per second","s-1",1.0,0,UC_RADIATION},
    {"kerma","kerma",1.0,0,UC_RADIATION},
    {"exposure rate","exp_rate",1.0,0,UC_RADIATION},
    {"absorbed dose rate","abs_rate",1.0,0,UC_RADIATION},
    {"dose equivalent rate","dose_rate",1.0,0,UC_RADIATION},

    // ── Misc (16) ──
    {"dozen","dozen",12.0,0,UC_MISC},
    {"gross","gross",144.0,0,UC_MISC},
    {"great gross","ggross",1728.0,0,UC_MISC},
    {"percent","%",0.01,0,UC_MISC},
    {"per mille","permille",0.001,0,UC_MISC},
    {"ppm","ppm",1e-6,0,UC_MISC},
    {"ppb","ppb",1e-9,0,UC_MISC},
    {"piece","pc_q",1.0,0,UC_MISC},
    {"pair","pair",2.0,0,UC_MISC},
    {"score","score",20.0,0,UC_MISC},
    {"baker's dozen","bdoz",13.0,0,UC_MISC},
    {"ream","ream",500.0,0,UC_MISC},
    {"quire","quire",25.0,0,UC_MISC},
    {"long hundred","lhund",120.0,0,UC_MISC},
    {"gross dozen","gdoz",1728.0,0,UC_MISC},
    {"counting measure","count",1.0,0,UC_MISC},
};
static const int unitN = sizeof(unitDB) / sizeof(UnitConv);

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 7 / 20 ─ 科学法則辞書 Part 1 (物理100 + 化学70)                 ║
// ╚═══════════════════════════════════════════════════════════════════════╝

static const LawEntry lawDB[] PROGMEM = {
    {"Newton's First Law","v = const (F = 0)","External force zero -> velocity constant. Inertia.","Physics"},
    {"Newton's Second Law","F = m a","Net force = mass x acceleration. F=dp/dt.","Physics"},
    {"Newton's Third Law","F_ab = - F_ba","Action-reaction. Equal magnitude, opposite direction.","Physics"},
    {"Universal Gravitation","F = G m1 m2 / r^2","G = 6.674e-11. Inverse square law of gravity.","Physics"},
    {"Kepler 1st Law","Ellipse: r = a(1-e^2)/(1+e cos f)","Planetary orbit is an ellipse, sun at one focus.","Physics"},
    {"Kepler 2nd Law","dA/dt = const","Equal areas in equal times. Angular momentum conservation.","Physics"},
    {"Kepler 3rd Law","T^2 = (4 pi^2 / G M) a^3","Period^2 proportional to semi-major axis^3.","Physics"},
    {"Hooke's Law","F = - k x","Spring force ~ displacement (small x).","Physics"},
    {"Archimedes' Principle","F_b = rho_f V g","Buoyancy = weight of displaced fluid.","Physics"},
    {"Pascal's Principle","Delta P = transmitted equally","Pressure change in confined fluid transmitted everywhere.","Physics"},
    {"Bernoulli's Equation","P + 0.5 rho v^2 + rho g h = const","Conservation along streamline (incompressible, inviscid).","Physics"},
    {"Boyle's Law","P V = const (T fixed)","Isothermal: P proportional to 1/V.","Physics"},
    {"Charles's Law","V / T = const (P fixed)","Isobaric: V proportional to T.","Physics"},
    {"Gay-Lussac's Law","P / T = const (V fixed)","Isochoric: P proportional to T.","Physics"},
    {"Avogadro's Law","V / n = const","Equal volumes at same P,T -> equal molecules.","Physics"},
    {"Ideal Gas Law","P V = n R T","Combined Boyle/Charles/Avogadro. R = 8.3145.","Physics"},
    {"Dalton's Law of Partial Pressures","P_total = sum P_i","Total pressure = sum of partial pressures.","Physics"},
    {"Henry's Law","C = k_H P","Solubility ~ partial pressure of gas above liquid.","Physics"},
    {"Zeroth Law of Thermodynamics","A=B and B=C -> A=C","Thermal equilibrium is transitive. Defines temperature.","Physics"},
    {"First Law of Thermodynamics","dU = Q - W","Energy conservation. Internal energy change.","Physics"},
    {"Second Law of Thermodynamics","dS >= 0 (isolated)","Entropy of isolated system never decreases.","Physics"},
    {"Third Law of Thermodynamics","S -> 0 (T -> 0)","Entropy approaches a constant as T to 0 K.","Physics"},
    {"Stefan-Boltzmann Law","j = sigma T^4","Black-body radiated power per area. sigma=5.67e-8.","Physics"},
    {"Wien's Displacement Law","lambda_max T = b","Peak wavelength inversely proportional to T. b=2.898e-3 m K.","Physics"},
    {"Planck's Law","B(nu,T) = 2 h nu^3 / c^2 / (exp(h nu/kT) - 1)","Spectral radiance of black body.","Physics"},
    {"Rayleigh-Jeans Law","B(nu,T) ~= 2 nu^2 k T / c^2","Low-frequency limit of Planck's law (UV catastrophe).","Physics"},
    {"Coulomb's Law","F = k_e q1 q2 / r^2","Electrostatic force. k_e = 1/(4 pi eps0).","Physics"},
    {"Gauss's Law (Electric)","Phi_E = Q_enc / eps0","Total electric flux through closed surface.","Physics"},
    {"Gauss's Law (Magnetic)","Phi_B = 0","No magnetic monopoles. Net B-flux through closed surface = 0.","Physics"},
    {"Faraday's Law of Induction","EMF = - d Phi_B / dt","Time-varying magnetic flux induces EMF.","Physics"},
    {"Ampere's Law","circ B dl = mu0 I_enc","Magnetic field around closed loop ~ enclosed current.","Physics"},
    {"Maxwell's Equations","div E=rho/eps0; div B=0; curl E=-dB/dt; curl B=mu0(J+eps0 dE/dt)","Foundations of classical electromagnetism.","Physics"},
    {"Lorentz Force","F = q(E + v x B)","Force on charged particle in EM field.","Physics"},
    {"Ohm's Law","V = I R","Voltage = current x resistance (linear conductor).","Physics"},
    {"Kirchhoff's Current Law","sum I_in = sum I_out","Charge conservation at a node.","Physics"},
    {"Kirchhoff's Voltage Law","sum V = 0 (around loop)","Energy conservation around any closed loop.","Physics"},
    {"Joule Heating","P = I^2 R = V^2 / R","Heat dissipated by resistor = I*V.","Physics"},
    {"Biot-Savart Law","dB = mu0 I dl x r^ / (4 pi r^2)","Magnetic field by current element.","Physics"},
    {"Lenz's Law","EMF opposes change in flux","Induced current direction. Energy conservation.","Physics"},
    {"Snell's Law","n1 sin(t1) = n2 sin(t2)","Refraction at boundary.","Physics"},
    {"Law of Reflection","theta_i = theta_r","Angle of incidence equals angle of reflection.","Physics"},
    {"Huygens' Principle","Wavelet superposition","Every point on a wavefront is a source of secondary wavelets.","Physics"},
    {"Bragg's Law","n lambda = 2 d sin(theta)","Constructive X-ray diffraction by crystal planes.","Physics"},
    {"Malus's Law","I = I0 cos^2(theta)","Intensity transmitted by polarizer.","Physics"},
    {"Brewster's Angle","tan(theta_B) = n2/n1","Reflected light is fully polarized.","Physics"},
    {"Doppler Effect (Sound)","f' = f (c +/- v_o)/(c +/- v_s)","Frequency shift for moving source/observer.","Physics"},
    {"Relativistic Doppler","f' = f sqrt((1-beta)/(1+beta))","Light Doppler with special relativity.","Physics"},
    {"Compton Scattering","Delta lambda = (h/mc)(1 - cos theta)","Photon-electron scattering shifts wavelength.","Physics"},
    {"Photoelectric Effect","K_max = h nu - phi","Einstein 1905. phi = work function.","Physics"},
    {"de Broglie Wavelength","lambda = h / p","Matter has wave nature.","Physics"},
    {"Heisenberg Uncertainty","Delta x Delta p >= hbar / 2","Position-momentum simultaneous precision limit.","Physics"},
    {"Schrodinger Eq (time-dep)","i hbar dPsi/dt = H Psi","Time evolution of quantum state.","Physics"},
    {"Schrodinger Eq (time-indep)","H Psi = E Psi","Energy eigenvalue equation.","Physics"},
    {"Pauli Exclusion Principle","No 2 fermions same state","Antisymmetric wavefunction for identical fermions.","Physics"},
    {"Fermi-Dirac Distribution","f(E) = 1/(exp((E-mu)/kT)+1)","Occupation probability for fermions.","Physics"},
    {"Bose-Einstein Distribution","f(E) = 1/(exp((E-mu)/kT)-1)","Occupation probability for bosons.","Physics"},
    {"Boltzmann Distribution","f(E) ~ exp(-E/kT)","Classical statistics. Maxwell-Boltzmann.","Physics"},
    {"Equipartition Theorem","<E> = (1/2) kT per quadratic dof","Average energy per degree of freedom.","Physics"},
    {"Curie's Law","M = C B / T","Paramagnetic magnetization above Curie temp.","Physics"},
    {"Curie-Weiss Law","chi = C / (T - Tc)","Susceptibility near Curie temperature.","Physics"},
    {"Bloch's Theorem","psi = u(r) exp(i k r)","Wavefunction in periodic potential.","Physics"},
    {"E = m c^2","E = m c^2","Mass-energy equivalence.","Physics"},
    {"Relativistic Energy","E^2 = (p c)^2 + (m c^2)^2","Total energy momentum relation.","Physics"},
    {"Time Dilation","Delta t' = gamma Delta t","Moving clocks run slow. gamma = 1/sqrt(1-v^2/c^2).","Physics"},
    {"Length Contraction","L' = L / gamma","Moving objects shorten in motion direction.","Physics"},
    {"Velocity Addition (Rel)","u' = (u-v)/(1-uv/c^2)","Relativistic velocity composition.","Physics"},
    {"Newton's Law of Cooling","dT/dt = - k (T - Te)","Cooling rate proportional to temperature difference.","Physics"},
    {"Fourier's Law","q = - k grad T","Heat flux ~ negative temperature gradient.","Physics"},
    {"Fick's First Law","J = - D grad C","Diffusion flux ~ concentration gradient.","Physics"},
    {"Fick's Second Law","dC/dt = D laplace C","Time evolution of diffusion.","Physics"},
    {"Poiseuille's Law","Q = pi r^4 Delta P / (8 eta L)","Laminar flow rate in cylindrical pipe.","Physics"},
    {"Stokes' Law","F = 6 pi eta r v","Drag on sphere in viscous fluid (low Re).","Physics"},
    {"Reynolds Number","Re = rho v L / eta","Inertial / viscous force ratio.","Physics"},
    {"Mach Number","M = v / a","v / speed of sound. M < 1 subsonic, > 1 supersonic.","Physics"},
    {"Clausius-Clapeyron Eq","dP/dT = L / (T Delta V)","Phase boundary slope on P-T diagram.","Physics"},
    {"Gibbs Phase Rule","F = C - P + 2","Degrees of freedom = components - phases + 2.","Physics"},
    {"Raoult's Law","P_A = x_A P*_A","Vapor pressure of ideal solution component.","Physics"},
    {"Moseley's Law","sqrt(nu) = a (Z - b)","X-ray frequency vs atomic number.","Physics"},
    {"Hubble's Law","v = H0 d","Recession velocity of distant galaxies.","Physics"},
    {"Chandrasekhar Limit","M ~= 1.4 M_sun","Max mass for white dwarf stability.","Physics"},
    {"Schwarzschild Radius","r_s = 2 G M / c^2","Event horizon radius of non-rotating black hole.","Physics"},
    {"Hawking Radiation","T_H = hbar c^3 / (8 pi G M k)","Black hole temperature (quantum effect).","Physics"},
    {"Bekenstein-Hawking Entropy","S_BH = k A / (4 l_P^2)","Black hole entropy ~ horizon area.","Physics"},
    {"Noether's Theorem","Symmetry => Conservation","Continuous symmetry implies a conserved current.","Physics"},
    {"Principle of Least Action","delta S = 0","Path makes action stationary.","Physics"},
    {"Hamilton's Principle","delta integ L dt = 0","Lagrangian formulation of mechanics.","Physics"},
    {"Froude Number","Fr = v / sqrt(g L)","Inertial / gravity force in free-surface flow.","Physics"},
    {"Weber Number","We = rho v^2 L / sigma","Inertia / surface tension force.","Physics"},
    {"Prandtl Number","Pr = nu / alpha","Momentum / thermal diffusivity.","Physics"},
    {"Nusselt Number","Nu = h L / k","Convective / conductive heat transfer.","Physics"},
    {"Grashof Number","Gr = g beta Delta T L^3 / nu^2","Buoyancy / viscous force.","Physics"},
    {"Rayleigh Number","Ra = Gr Pr","Buoyancy-driven flow regime indicator.","Physics"},
    {"Peclet Number","Pe = L v / alpha","Advection / diffusion of heat.","Physics"},
    {"Schmidt Number","Sc = nu / D","Momentum / mass diffusivity.","Physics"},
    {"Lewis Number","Le = alpha / D","Thermal / mass diffusivity.","Physics"},
    {"Buckingham Pi Theorem","Pi groups dimensionless","Reduces dimensional equations to dimensionless.","Physics"},
    {"Bernoulli (compressible)","h + v^2/2 = const","Stagnation enthalpy along streamline.","Physics"},
    {"Casimir Effect","F/A = - pi^2 hbar c / 240 d^4","Vacuum fluctuation force between plates.","Physics"},
    {"Larmor Formula","P = q^2 a^2 / (6 pi eps0 c^3)","Power radiated by accelerated charge.","Physics"},
    {"Cherenkov Radiation","v > c/n","Charged particle exceeds light speed in medium.","Physics"},
    {"Drude Model","sigma = n e^2 tau / m","Electrical conductivity of metals.","Physics"},

    {"Law of Conservation of Mass","Sum m_react = Sum m_prod","Lavoisier 1789. No mass loss in chemical reactions.","Chemistry"},
    {"Law of Definite Proportions","fixed mass ratio","Compound has same composition by mass everywhere.","Chemistry"},
    {"Law of Multiple Proportions","Small whole numbers","Different compounds of same elements: small int ratios.","Chemistry"},
    {"Avogadro's Hypothesis","Equal V -> Equal N (P,T fixed)","Same gas volume at same conditions = same molecules.","Chemistry"},
    {"Le Chatelier's Principle","Stress -> shift to relieve","Equilibrium shifts to oppose imposed change.","Chemistry"},
    {"Hess's Law","Delta H = sum Delta H_i","Total enthalpy change is path-independent.","Chemistry"},
    {"Gibbs Free Energy","G = H - T S; dG <= 0 spontaneous","Spontaneity criterion at constant T,P.","Chemistry"},
    {"Arrhenius Equation","k = A exp(-Ea/RT)","Reaction rate temperature dependence.","Chemistry"},
    {"Mass Action Law","rate = k [A]^a [B]^b","Rate proportional to reactant concentrations.","Chemistry"},
    {"Michaelis-Menten Equation","v = V_max [S]/(K_M + [S])","Enzyme kinetics, hyperbolic.","Chemistry"},
    {"Henderson-Hasselbalch","pH = pKa + log([A-]/[HA])","Buffer solution pH.","Chemistry"},
    {"Nernst Equation","E = E0 - (RT/nF) ln Q","Electrode potential dependence on concentration.","Chemistry"},
    {"Faraday's 1st Electrolysis","m = (Q M)/(n F)","Mass deposited ~ charge passed.","Chemistry"},
    {"Faraday's 2nd Electrolysis","m1/m2 = E1/E2","Equivalent weights at same charge.","Chemistry"},
    {"Graham's Law of Diffusion","r1/r2 = sqrt(M2/M1)","Diffusion rate inverse sqrt of molar mass.","Chemistry"},
    {"van der Waals Equation","(P + an^2/V^2)(V - nb) = nRT","Real gas with intermolecular forces.","Chemistry"},
    {"Dulong-Petit Law","C_V ~= 3R per mol atoms","Solid heat capacity at high T.","Chemistry"},
    {"Born-Haber Cycle","Lattice energy decomposition","Thermodynamic cycle for ionic compound formation.","Chemistry"},
    {"Pauling Electronegativity","Bond energy difference","Atom's electron-attracting power.","Chemistry"},
    {"VSEPR Theory","Pairs repel maximally","Molecular geometry from electron pairs.","Chemistry"},
    {"Crystal Field Theory","d-orbital splitting","Color/magnetism of transition metal complexes.","Chemistry"},
    {"Jahn-Teller Effect","Distortion to lift degeneracy","Non-linear molecules with degenerate electronic states distort.","Chemistry"},
    {"Huckel's Rule","(4n+2) pi electrons -> aromatic","Aromaticity criterion.","Chemistry"},
    {"Markovnikov's Rule","H to C with more H","HX addition to asymmetric alkene.","Chemistry"},
    {"Zaitsev's Rule","More substituted alkene favored","Elimination product preference.","Chemistry"},
    {"Cahn-Ingold-Prelog Rules","R/S nomenclature priority","Stereodescriptors based on atomic number priority.","Chemistry"},
    {"Hammond Postulate","TS resembles closer-energy state","Transition state geometry.","Chemistry"},
    {"Curtin-Hammett Principle","Product ratio depends on TS","Two interconverting reactants -> kinetics matters.","Chemistry"},
    {"Fajans' Rules","Polarization in ionic bonds","Predicts covalent character.","Chemistry"},
    {"HSAB Principle","Hard-Hard, Soft-Soft preferred","Lewis acid-base reactivity.","Chemistry"},
    {"Molecular Orbital Theory","psi = c1 phi1 + c2 phi2 + ...","Electrons delocalized in molecular orbitals.","Chemistry"},
    {"Ligand Field Theory","MO theory + crystal field","Bonding in transition metal complexes.","Chemistry"},
    {"Woodward-Hoffmann Rules","Symmetry-controlled reactions","Pericyclic reaction allowed/forbidden.","Chemistry"},
    {"Baldwin's Rules","Ring closure favorability","n-exo-tet, etc.","Chemistry"},
    {"Cope Elimination","Amine oxide -> alkene","Syn-periplanar 5-membered TS.","Chemistry"},
    {"Hofmann Elimination","Bulky base -> less subst alkene","Quaternary ammonium hydroxide pyrolysis.","Chemistry"},
    {"Aldol Condensation","C=O + alpha-H -> beta-hydroxy","Two carbonyl compounds.","Chemistry"},
    {"Diels-Alder Reaction","[4+2] cycloaddition","Diene + dienophile -> cyclohexene.","Chemistry"},
    {"Friedel-Crafts Alkylation","ArH + RX -> ArR (Lewis acid)","Electrophilic aromatic substitution.","Chemistry"},
    {"Friedel-Crafts Acylation","ArH + RCOCl -> ArCOR","Aromatic ketone formation.","Chemistry"},
    {"Grignard Reaction","RMgX + carbonyl -> alcohol","C-C bond formation, organometallic.","Chemistry"},
    {"Wittig Reaction","Ph3P=CR2 + R'CHO -> alkene","Phosphorus ylide + aldehyde -> alkene.","Chemistry"},
    {"Cannizzaro Reaction","2 ArCHO -> ArCH2OH + ArCOOH","No alpha-H aldehyde disproportionation.","Chemistry"},
    {"Claisen Rearrangement","Allyl vinyl ether [3,3]","Sigmatropic rearrangement.","Chemistry"},
    {"Birch Reduction","ArH + Na/NH3 -> 1,4-dihydroarene","Partial aromatic reduction.","Chemistry"},
    {"Clemmensen Reduction","C=O -> CH2 (Zn/HCl)","Carbonyl to methylene.","Chemistry"},
    {"Wolff-Kishner Reduction","C=O -> CH2 (NH2NH2/base)","Carbonyl reduction in basic conditions.","Chemistry"},
    {"Beckmann Rearrangement","Oxime -> amide","Acid-catalyzed.","Chemistry"},
    {"Finkelstein Reaction","RX + NaI(acetone) -> RI","Halide exchange (SN2).","Chemistry"},
    {"Sandmeyer Reaction","ArN2+ + CuX -> ArX","Diazonium salt -> aryl halide.","Chemistry"},
    {"Strecker Synthesis","RCHO + NH3 + HCN -> amino acid","Synthesis of alpha-amino acids.","Chemistry"},
    {"Gabriel Synthesis","Phthalimide + RX -> R-NH2","Primary amines via phthalimide.","Chemistry"},
    {"Williamson Synthesis","RO- + R'X -> R-O-R'","Ether synthesis.","Chemistry"},
    {"Fischer Esterification","RCOOH + R'OH <-> RCOOR' + H2O","Acid + alcohol -> ester (acid cat.).","Chemistry"},
    {"Michael Addition","Nucleophile + alpha,beta-unsat C=O","1,4-addition to enone.","Chemistry"},
    {"Mannich Reaction","Amine + CH2O + alpha-H carbonyl","Beta-amino carbonyl synthesis.","Chemistry"},
    {"Perkin Reaction","ArCHO + (RCO)2O -> cinnamic acid","Acetic anhydride condensation.","Chemistry"},
    {"Reformatsky Reaction","BrCH2COOR + RCHO + Zn","Beta-hydroxy ester synthesis.","Chemistry"},
    {"Suzuki Coupling","ArX + Ar'B(OH)2 -> Ar-Ar' (Pd)","C-C bond formation, cross-coupling.","Chemistry"},
    {"Heck Reaction","ArX + CH2=CHR -> Ar-CH=CHR (Pd)","Pd-catalyzed olefination.","Chemistry"},
    {"Kolbe-Schmitt Reaction","PhONa + CO2 -> salicylic acid","Phenol carboxylation.","Chemistry"},
    {"Hell-Volhard-Zelinsky","RCH2COOH + Br2/PBr3 -> RCHBrCOOH","Alpha-halogenation of carboxylic acid.","Chemistry"},
    {"Mulliken Electronegativity","chi = (IE + EA)/2","Average of ionization energy and electron affinity.","Chemistry"},
    {"Virial Equation of State","PV/RT = 1 + B/V + C/V^2 + ...","Real gas expansion in inverse molar volume.","Chemistry"},
    {"Trouton's Rule","Delta S_vap ~= 88 J/mol K","Entropy of vaporization for many liquids.","Chemistry"},
    {"Law of Combining Volumes","Whole number ratios at fixed T,P","Gay-Lussac. Reacting gases combine in simple volume ratios.","Chemistry"},
    {"Beer-Lambert Law","A = epsilon c l","Absorbance is proportional to path length and concentration.","Chemistry"},
    {"Ostwald's Dilution Law","Ka = c alpha^2 / (1 - alpha)","Weak electrolyte dissociation degree vs concentration.","Chemistry"},
    {"Raoult's Law (Solution)","P_solv = x_solv P*_solv","Vapor pressure of solvent above ideal solution.","Chemistry"},
    {"Kohlrausch's Law","Lambda = Lambda+ + Lambda-","Independent migration of ions at infinite dilution.","Chemistry"},
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 8 / 20 ─ 科学法則辞書 Part 2 (数学50 + 地球科学30 + 天文20 +    ║
// ║                              生物20 + 電気電子11 = 131)              ║
// ║  実装方針: lawDB[] は独立配列、lawDB2[] を別途定義、lawAt() で統合   ║
// ╚═══════════════════════════════════════════════════════════════════════╝

static const LawEntry lawDB2[] PROGMEM = {
    {"Pythagorean Theorem","a^2 + b^2 = c^2","Right triangle: hypotenuse squared = sum of legs squared.","Math"},
    {"Fundamental Theorem of Arithmetic","n = prod p_i^a_i (unique)","Every integer > 1 has unique prime factorization.","Math"},
    {"Fundamental Theorem of Algebra","deg n poly has n complex roots","Polynomial of degree n has exactly n roots in C.","Math"},
    {"Fundamental Theorem of Calculus","integ_a^b f' dx = f(b) - f(a)","Differentiation and integration are inverse.","Math"},
    {"Intermediate Value Theorem","f cont, f(a)<u<f(b) -> exists c","Continuous function takes intermediate values.","Math"},
    {"Mean Value Theorem","exists c: f'(c) = (f(b)-f(a))/(b-a)","Differentiable function has tangent parallel to chord.","Math"},
    {"Rolle's Theorem","f(a)=f(b) -> exists c: f'(c)=0","Special case of MVT.","Math"},
    {"Taylor's Theorem","f(x) = sum f^(n)(a) (x-a)^n/n!","Expansion of differentiable function around a.","Math"},
    {"L'Hopital's Rule","lim f/g = lim f'/g' (0/0 or inf/inf)","Indeterminate form via derivatives.","Math"},
    {"Cauchy-Schwarz Inequality","|<u,v>| <= ||u|| ||v||","Inner product bounded by norms.","Math"},
    {"Triangle Inequality","||u+v|| <= ||u|| + ||v||","Sum of vectors bounded by sum of norms.","Math"},
    {"AM-GM Inequality","(a+b)/2 >= sqrt(a b)","Arithmetic mean >= geometric mean (positives).","Math"},
    {"Bernoulli's Inequality","(1+x)^r >= 1 + r x","For x > -1, r >= 1 (integer).","Math"},
    {"Euler's Identity","e^(i pi) + 1 = 0","Most beautiful equation. Connects e, i, pi, 1, 0.","Math"},
    {"de Moivre's Theorem","(cos x + i sin x)^n = cos n x + i sin n x","Complex number power.","Math"},
    {"Binomial Theorem","(a+b)^n = sum C(n,k) a^(n-k) b^k","Expansion of binomial power.","Math"},
    {"Multinomial Theorem","(x1+...+xk)^n = sum n!/(n1!..nk!) prod x_i^n_i","Generalization of binomial.","Math"},
    {"Quadratic Formula","x = (-b +/- sqrt(b^2-4ac))/(2a)","Roots of ax^2+bx+c=0.","Math"},
    {"Cardano's Cubic Formula","Substitution + radicals","Solves general cubic equation.","Math"},
    {"Ferrari's Quartic Formula","Resolvent cubic + radicals","Solves general quartic.","Math"},
    {"Abel-Ruffini Theorem","deg >= 5: no general radical solution","No closed-form radical formula for quintic.","Math"},
    {"Cayley-Hamilton Theorem","p_A(A) = 0","Matrix satisfies its own characteristic polynomial.","Math"},
    {"Spectral Theorem","Hermitian -> orthogonal eigvec diag","Self-adjoint operators are diagonalizable.","Math"},
    {"Singular Value Decomposition","A = U Sigma V^*","Any matrix factorization.","Math"},
    {"Rank-Nullity Theorem","rank(A) + nullity(A) = n","Linear map dimensions theorem.","Math"},
    {"Law of Large Numbers","X_bar -> mu (n -> inf)","Sample mean converges to population mean.","Math"},
    {"Central Limit Theorem","X_bar ~ N(mu, sigma^2/n) (n large)","Sum of iid random variables -> normal.","Math"},
    {"Bayes' Theorem","P(A|B) = P(B|A) P(A) / P(B)","Posterior probability formula.","Math"},
    {"Chebyshev's Inequality","P(|X-mu| >= k sigma) <= 1/k^2","Tail bound from variance.","Math"},
    {"Markov's Inequality","P(X >= a) <= E[X]/a","Tail bound for nonneg RV.","Math"},
    {"Jensen's Inequality","f convex: f(E X) <= E f(X)","Convex function and expectation.","Math"},
    {"Holder's Inequality","sum |a_i b_i| <= (sum |a_i|^p)^(1/p) (sum |b_i|^q)^(1/q)","1/p + 1/q = 1.","Math"},
    {"Minkowski Inequality","(sum |a_i+b_i|^p)^(1/p) <= ..._a + ..._b","Triangle inequality for L^p norms.","Math"},
    {"Fermat's Little Theorem","a^p == a (mod p)","If p prime, then a^p - a divisible by p.","Math"},
    {"Euler's Theorem (Number)","a^phi(n) == 1 (mod n) (gcd 1)","Generalization of Fermat's little theorem.","Math"},
    {"Chinese Remainder Theorem","Unique mod prod m_i","System of congruences solvable.","Math"},
    {"Wilson's Theorem","(p-1)! == -1 (mod p)","p prime <=> Wilson congruence.","Math"},
    {"Prime Number Theorem","pi(x) ~ x / ln x","Density of primes.","Math"},
    {"Riemann Hypothesis","All non-trivial zeros: Re=1/2","Open problem on zeta function zeros.","Math"},
    {"Goldbach Conjecture","Even n>2 = p1 + p2","Every even integer > 2 is sum of two primes (open).","Math"},
    {"Twin Prime Conjecture","infinite p,p+2 prime","Infinitely many twin primes (open).","Math"},
    {"Four Color Theorem","Plane map: 4 colors suffice","Any planar map colorable with 4 colors. Proved 1976.","Math"},
    {"Godel Incompleteness","Consistent system has undecidable","Any sufficient formal system is incomplete.","Math"},
    {"Church-Turing Thesis","Computable = Turing-machine","All effective computation captured by Turing machines.","Math"},
    {"P vs NP","P = NP ?","Open: polynomial-time = polynomial-verifiable.","Math"},
    {"Zorn's Lemma","Chain has upper bound -> max elt","Equivalent to axiom of choice.","Math"},
    {"Axiom of Choice","Choose from any family of sets","ZFC axiom. Allows non-constructive proofs.","Math"},
    {"Banach-Tarski Paradox","Sphere split + reconstructed = 2","Counterintuitive consequence of AC.","Math"},
    {"Cantor's Diagonal Argument","|R| > |N|","Real numbers uncountable.","Math"},
    {"Stokes' Theorem","integ_S curl F dA = circ_dS F dl","Generalization of Green's, Gauss's, classical Stokes.","Math"},

    {"Plate Tectonics","Lithosphere plates move","Earth's surface composed of moving rigid plates.","GeoSci"},
    {"Continental Drift","Continents migrate (Wegener)","Pangea broke up; continents move slowly.","GeoSci"},
    {"Sea-Floor Spreading","Mid-ocean ridge: new crust","Ocean floor expands at ridges.","GeoSci"},
    {"Mohorovicic Discontinuity","Crust-mantle boundary","Seismic velocity jump at ~30 km depth.","GeoSci"},
    {"Gutenberg Discontinuity","Mantle-outer core boundary","Discontinuity at ~2900 km depth.","GeoSci"},
    {"Lehmann Discontinuity","Outer core - inner core boundary","Discontinuity at ~5150 km. Inner core solid.","GeoSci"},
    {"Seismic Shadow Zone","104deg-140deg from epicenter","P-wave shadow due to outer core refraction.","GeoSci"},
    {"Snell's Law (Seismic)","sin(t1)/v1 = sin(t2)/v2","Seismic ray refraction at interfaces.","GeoSci"},
    {"Elastic Rebound Theory","Strain release at fault","Reid 1910. Earthquake mechanism.","GeoSci"},
    {"Reid's Elastic Rebound","Stored strain -> rapid slip","Same as elastic rebound, ref'd by Reid.","GeoSci"},
    {"Gutenberg-Richter Law","log N = a - b M","Earthquake frequency-magnitude relation.","GeoSci"},
    {"Omori's Law","n(t) = K/(c+t)^p","Aftershock decay.","GeoSci"},
    {"Bath's Law","M_main - M_aftermax ~= 1.2","Largest aftershock magnitude difference.","GeoSci"},
    {"Coriolis Effect","F_C = -2 m Omega x v","Apparent force in rotating frame. Drives weather.","GeoSci"},
    {"Geostrophic Balance","Coriolis ~ pressure gradient","Large-scale wind balance.","GeoSci"},
    {"Thermal Wind Equation","dV/dz ~ grad T","Vertical shear by horizontal temperature gradient.","GeoSci"},
    {"Milankovitch Cycles","100k/41k/26k yr","Orbital variations cause climate cycles.","GeoSci"},
    {"Albedo Feedback","Reflectivity-temp coupling","Snow/ice surface reflectivity feedback.","GeoSci"},
    {"Ice-Albedo Feedback","Ice cover -> high albedo","Positive feedback in cooling/warming.","GeoSci"},
    {"Clausius-Clapeyron (Atmos)","es ~ exp(L/RT)","Saturation vapor pressure of water vs T.","GeoSci"},
    {"Coriolis Parameter","f = 2 Omega sin(lat)","Vertical Coriolis component.","GeoSci"},
    {"Rossby Number","Ro = U / (f L)","Inertial / Coriolis. Ro<<1: rotation dominant.","GeoSci"},
    {"Richardson Number","Ri = N^2 / (du/dz)^2","Static / dynamic stability of stratified flow.","GeoSci"},
    {"Bowen Ratio","B = sensible / latent heat flux","Surface energy partition indicator.","GeoSci"},
    {"Monin-Obukhov Similarity","Surface layer profiles","Theoretical framework for boundary layer.","GeoSci"},
    {"Darcy's Law","Q = - K A dH/dL","Groundwater flow through porous media.","GeoSci"},
    {"Ekman Number","Ek = nu / (Omega L^2)","Viscous / Coriolis force ratio.","GeoSci"},
    {"Modified Mercalli Scale","I-XII intensity","Earthquake felt intensity.","GeoSci"},
    {"Richter Magnitude","M_L = log10(A) + corr","Seismic wave amplitude logarithm.","GeoSci"},
    {"Moment Magnitude","M_w = (2/3) log10(M0) - 10.7","Modern seismic moment based scale.","GeoSci"},

    {"Hubble's Law (Astro)","v = H0 d","Galaxies recede with speed proportional to distance.","Astronomy"},
    {"Kepler's First Law (Astro)","Elliptical orbits","Planets move on ellipses with sun at focus.","Astronomy"},
    {"Kepler's Second Law (Astro)","Equal areas in equal times","Conservation of angular momentum.","Astronomy"},
    {"Kepler's Third Law (Astro)","T^2 / a^3 = const","Period-distance relation.","Astronomy"},
    {"Titius-Bode Law","a = 0.4 + 0.3 * 2^n","Empirical planetary distance pattern.","Astronomy"},
    {"Stefan-Boltzmann (Star)","L = 4 pi R^2 sigma T^4","Stellar luminosity from radius and temperature.","Astronomy"},
    {"Wien's Law (Star)","lambda_max = b / T","Peak emission of stellar black body.","Astronomy"},
    {"Black-Body Radiation","B(nu, T) Planck","Idealized emitter spectrum.","Astronomy"},
    {"Cosmological Doppler","z = Delta lambda / lambda","Redshift / blueshift of cosmic sources.","Astronomy"},
    {"Einstein's Field Eq","G_mu_nu = 8 pi G T_mu_nu / c^4","GR field equations.","Astronomy"},
    {"Schwarzschild Metric","ds^2 = -(1-r_s/r) c^2 dt^2 + ...","Spherically symmetric vacuum solution.","Astronomy"},
    {"Chandrasekhar Limit (Astro)","M_C ~= 1.4 M_sun","Maximum WD mass before collapse.","Astronomy"},
    {"Jeans Instability","M > M_J = (5kT/Gm)^1.5 (1/rho)^0.5","Gas cloud collapse criterion.","Astronomy"},
    {"Roche Limit","d ~ R (2 rho_M / rho_m)^(1/3)","Tidal disruption distance.","Astronomy"},
    {"Hill Sphere","r_H ~= a (m / 3M)^(1/3)","Gravitational dominance radius.","Astronomy"},
    {"Eddington Luminosity","L_Edd ~ 4 pi G M m_p c / sigma_T","Maximum stable luminosity.","Astronomy"},
    {"Virial Theorem (Astro)","2 T + U = 0 (gravitational)","Time-average kinetic and potential energy.","Astronomy"},
    {"Saha Equation","n_i+1 n_e / n_i = ...","Ionization in stellar atmospheres.","Astronomy"},
    {"Hayashi Track","Pre-main-sequence vertical track","Convective protostar evolution on HR.","Astronomy"},
    {"Henyey Track","Pre-MS horizontal track","Radiative protostar evolution on HR.","Astronomy"},

    {"Mendel's Law of Segregation","Allele pairs separate in gametes","1st law: each allele segregates independently.","Biology"},
    {"Mendel's Law of Independent Assortment","Genes assort independently","2nd law: different genes inherit independently.","Biology"},
    {"Mendel's Law of Dominance","Dominant masks recessive","3rd law: in F1, dominant trait expressed.","Biology"},
    {"Hardy-Weinberg","p^2 + 2pq + q^2 = 1","Population genetics equilibrium.","Biology"},
    {"Chargaff's Rules","%A=%T, %G=%C","DNA base ratios.","Biology"},
    {"Central Dogma","DNA -> RNA -> Protein","Genetic information flow.","Biology"},
    {"Watson-Crick Base Pairing","A-T (2H), G-C (3H)","DNA double helix base complementarity.","Biology"},
    {"DNA Semiconservative Replication","Each daughter: 1 old + 1 new strand","Meselson-Stahl 1958.","Biology"},
    {"Genetic Code Universality","Codon table same in all life","61 sense + 3 stop codons.","Biology"},
    {"Lac Operon Model","Lactose induces gene expression","Jacob-Monod 1961 prokaryote regulation.","Biology"},
    {"All-or-None Action Potential","Threshold or no firing","Neuron firing law.","Biology"},
    {"Nernst Equation (Cell)","V = (RT/zF) ln([out]/[in])","Membrane equilibrium potential.","Biology"},
    {"Goldman-Hodgkin-Katz","V_m = RT/F ln(weighted ions)","Membrane potential with multiple ions.","Biology"},
    {"Hodgkin-Huxley Model","Ion currents -> action potential","Mathematical model of neuron firing.","Biology"},
    {"Michaelis-Menten (Enzyme)","v = Vmax [S]/(Km + [S])","Enzyme kinetics rate equation.","Biology"},
    {"Monod Equation","mu = mu_max S/(K_S + S)","Bacterial growth kinetics.","Biology"},
    {"Lineweaver-Burk Plot","1/v = Km/Vmax 1/[S] + 1/Vmax","Enzyme kinetics linearization.","Biology"},
    {"Bergmann's Rule","Cold climate -> larger body","Endotherm body size vs latitude.","Biology"},
    {"Allen's Rule","Cold climate -> shorter limbs","Body extremity length vs climate.","Biology"},
    {"Restriction Enzyme Recognition","Palindromic 4-8 bp sequences","EcoRI: GAATTC, etc.","Biology"},

    {"Ohm's Law (EE)","V = I R","Linear conductor: voltage = current x resistance.","EE"},
    {"Kirchhoff's Current Law (EE)","Sum I = 0 at node","Charge conservation.","EE"},
    {"Kirchhoff's Voltage Law (EE)","Sum V = 0 around loop","Energy conservation.","EE"},
    {"Thevenin's Theorem","Linear net = V_th + R_th","Equivalent voltage source + series resistor.","EE"},
    {"Norton's Theorem","Linear net = I_n parallel R_n","Equivalent current source + parallel resistor.","EE"},
    {"Superposition Theorem","Linear net: sum responses","Each independent source contributes independently.","EE"},
    {"Maximum Power Transfer","R_load = R_source","Maximum power when load matches source impedance.","EE"},
    {"Nyquist-Shannon Sampling","f_s >= 2 f_max","Sampling rate >= twice highest frequency.","EE"},
    {"Shannon-Hartley Theorem","C = B log2(1 + S/N)","Channel capacity in noise.","EE"},
    {"Friis Transmission Eq","Pr = Pt Gt Gr (lambda/(4 pi d))^2","Wireless link budget.","EE"},
    {"Johnson-Nyquist Noise","v_n^2 = 4 k T R B","Thermal noise in resistor.","EE"},
};

static const int lawN  = sizeof(lawDB)  / sizeof(LawEntry);
static const int lawN2 = sizeof(lawDB2) / sizeof(LawEntry);

static const char* LAW_CATS[] = {
    "Physics", "Chemistry", "Math", "GeoSci", "Astronomy", "Biology", "EE"
};
static const int LAW_NCAT = sizeof(LAW_CATS) / sizeof(char*);

inline const LawEntry* lawAt(int idx) {
    if (idx < lawN)            return &lawDB[idx];
    if (idx < lawN + lawN2)    return &lawDB2[idx - lawN];
    return nullptr;
}
static const int LAW_TOTAL = lawN + lawN2;

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 9 / 20 ─ SciRef モード                                          ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class SciRef {
public:
    void run() {
        while (true) {
            const char* items[] = {
                "Constants (386)",
                "Scientific Laws (300)",
                "Unit Converter (532)",
                "Periodic Table (118)",
                "Search by Symbol",
                "Back"
            };
            int s = DM.menu("SciRef", items, 6);
            if (s < 0 || s == 5) return;
            switch (s) {
                case 0: browseConstants(); break;
                case 1: browseLaws();      break;
                case 2: unitConvert();     break;
                case 3: periodicTable();   break;
                case 4: searchSymbol();    break;
            }
        }
    }

private:
    void browseConstants() {
        while (true) {
            int cat = DM.menu("Const Category", CONST_CATS, CONST_NCAT);
            if (cat < 0) return;
            int idx[400], n = 0;
            for (int i = 0; i < constN; i++) {
                Constant c;
                memcpy_P(&c, &constDB[i], sizeof(Constant));
                if (!strcmp(c.cat, CONST_CATS[cat])) idx[n++] = i;
            }
            if (n == 0) { DM.notify("Empty category", C_WARN); continue; }
            char** labels = (char**)malloc(n * sizeof(char*));
            for (int i = 0; i < n; i++) {
                Constant c; memcpy_P(&c, &constDB[idx[i]], sizeof(Constant));
                labels[i] = (char*)malloc(48);
                int symLen = strlen(c.sym);
                if (symLen > 7) symLen = 7;
                char tmp[10] = "       ";
                memcpy(tmp, c.sym, symLen);
                tmp[7] = 0;
                snprintf(labels[i], 48, "%s %s", tmp, c.name);
            }
            const char** clabels = (const char**)labels;
            int sel = 0;
            while (true) {
                sel = DM.menu(CONST_CATS[cat], clabels, n, sel);
                if (sel < 0) break;
                showConstantDetail(idx[sel]);
            }
            for (int i = 0; i < n; i++) free(labels[i]);
            free(labels);
        }
    }

    void showConstantDetail(int i) {
        Constant c; memcpy_P(&c, &constDB[i], sizeof(Constant));
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header(c.sym);
        DM.footer("Y:Ans  B:Bank+", "S:Save", "ESC:back");
        d.setTextColor(C_FG, C_BG);
        DM.drawWrapped(c.name, 4, HDR_H + 2, LCD_W - 8, C_FG);
        char val[40]; formatNum(c.value, val, sizeof(val));
        d.setTextSize(2);
        d.setTextColor(C_ACC, C_BG);
        d.drawString(val, 4, HDR_H + 26);
        d.setTextSize(1);
        d.setTextColor(C_DIM, C_BG);
        d.drawString(c.unit, 4, HDR_H + 54);
        char cl[24]; snprintf(cl, sizeof(cl), "[%s]", c.cat);
        d.setTextColor(C_ACC2, C_BG);
        d.drawString(cl, 4, HDR_H + 68);
        d.setTextDatum(top_left);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = c.value;
                G.ansComplex = false;
                DM.notify("-> Ans", C_OK);
                return;
            }
            if (M5Cardputer.Keyboard.isKeyPressed('b') ||
                M5Cardputer.Keyboard.isKeyPressed('B')) {
                if (G.bankN < ANS_BANK_MAX) {
                    G.bank[G.bankN] = c.value;
                    strncpy(G.bankLabel[G.bankN], c.sym, 15);
                    G.bankLabel[G.bankN][15] = 0;
                    G.bankN++;
                    char m[40]; snprintf(m, sizeof(m), "Bank+ (%d/%d)", G.bankN, ANS_BANK_MAX);
                    DM.notify(m, C_OK);
                } else {
                    DM.notify("Bank full!", C_ERR);
                }
            }
            if (M5Cardputer.Keyboard.isKeyPressed('s') ||
                M5Cardputer.Keyboard.isKeyPressed('S')) {
                DM.saveBMPAuto("const");
                DM.header(c.sym);
                DM.footer("Y:Ans  B:Bank+", "S:Save", "ESC:back");
            }
        }
    }

    void browseLaws() {
        while (true) {
            int cat = DM.menu("Law Category", LAW_CATS, LAW_NCAT);
            if (cat < 0) return;
            int idx[400], n = 0;
            for (int i = 0; i < LAW_TOTAL; i++) {
                const LawEntry* L = lawAt(i);
                LawEntry tmp; memcpy_P(&tmp, L, sizeof(LawEntry));
                if (!strcmp(tmp.cat, LAW_CATS[cat])) idx[n++] = i;
            }
            if (n == 0) { DM.notify("Empty", C_WARN); continue; }
            char** labels = (char**)malloc(n * sizeof(char*));
            for (int i = 0; i < n; i++) {
                LawEntry tmp; memcpy_P(&tmp, lawAt(idx[i]), sizeof(LawEntry));
                labels[i] = (char*)malloc(48);
                snprintf(labels[i], 48, "%s", tmp.name);
            }
            const char** clabels = (const char**)labels;
            int sel = 0;
            while (true) {
                sel = DM.menu(LAW_CATS[cat], clabels, n, sel);
                if (sel < 0) break;
                showLawDetail(idx[sel]);
            }
            for (int i = 0; i < n; i++) free(labels[i]);
            free(labels);
        }
    }

    void showLawDetail(int i) {
        LawEntry L; memcpy_P(&L, lawAt(i), sizeof(LawEntry));
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header(L.name);
        DM.footer("S:Save", "", "ESC:back");
        d.setTextColor(C_ACC, C_BG);
        d.drawString("Formula:", 4, HDR_H + 2);
        DM.drawWrapped(L.formula, 4, HDR_H + 14, LCD_W - 8, C_FG);
        d.setTextColor(C_ACC2, C_BG);
        d.drawString("Note:", 4, HDR_H + 50);
        DM.drawWrapped(L.notes, 4, HDR_H + 62, LCD_W - 8, C_DIM);
        d.setTextDatum(top_left);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') ||
                M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("law");
        }
    }

    void unitConvert() {
        int cat = DM.menu("Unit Category", UNIT_CATS, UNIT_NCAT);
        if (cat < 0) return;
        int idx[80], n = 0;
        for (int i = 0; i < unitN; i++) {
            UnitConv u; memcpy_P(&u, &unitDB[i], sizeof(UnitConv));
            if (u.cat == cat) idx[n++] = i;
        }
        if (n == 0) { DM.notify("Empty", C_WARN); return; }
        char** labels = (char**)malloc(n * sizeof(char*));
        for (int i = 0; i < n; i++) {
            UnitConv u; memcpy_P(&u, &unitDB[idx[i]], sizeof(UnitConv));
            labels[i] = (char*)malloc(40);
            snprintf(labels[i], 40, "%s (%s)", u.sym, u.name);
        }
        const char** clabels = (const char**)labels;
        int from = DM.menu("From", clabels, n, 0);
        if (from < 0) goto cleanup;
        {
            int to = DM.menu("To", clabels, n, 0);
            if (to < 0) goto cleanup;
            char buf[24] = "1";
            if (!DM.inputLine("Value:", buf, sizeof(buf))) goto cleanup;
            double v = atof(buf);
            UnitConv uf, ut;
            memcpy_P(&uf, &unitDB[idx[from]], sizeof(UnitConv));
            memcpy_P(&ut, &unitDB[idx[to]],   sizeof(UnitConv));
            double base = v * uf.factor + uf.offset;
            double out  = (base - ut.offset) / ut.factor;

            auto& d = M5Cardputer.Display;
            d.fillScreen(C_BG);
            DM.header("Result");
            DM.footer("Y:Ans", "S:Save", "ESC:back");

            char l1[40], l2[40], l3[40];
            char vs[24]; formatNum(v, vs, sizeof(vs));
            char os[24]; formatNum(out, os, sizeof(os));
            snprintf(l1, sizeof(l1), "%s %s", vs, uf.sym);
            snprintf(l2, sizeof(l2), "= %s %s", os, ut.sym);
            d.setTextColor(C_FG, C_BG);  d.drawString(l1, 4, HDR_H + 8);
            d.setTextColor(C_OK, C_BG);
            d.setTextSize(2);
            d.drawString(l2, 4, HDR_H + 28);
            d.setTextSize(1);
            d.setTextColor(C_DIM, C_BG);
            snprintf(l3, sizeof(l3), "[%s]", UNIT_CATS[cat]);
            d.drawString(l3, 4, LCD_H - FTR_H - 14);

            G.ans = out; G.ansComplex = false;
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) break;
                if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                    M5Cardputer.Keyboard.isKeyPressed('Y')) {
                    DM.notify("-> Ans", C_OK); break;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('s') ||
                    M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("unit");
            }
        }
cleanup:
        for (int i = 0; i < n; i++) free(labels[i]);
        free(labels);
    }

    void periodicTable() {
        int sel = 0;
        auto& d = M5Cardputer.Display;
        static const int8_t POS[118][2] = {
            {1,1},{18,1},
            {1,2},{2,2},{13,2},{14,2},{15,2},{16,2},{17,2},{18,2},
            {1,3},{2,3},{13,3},{14,3},{15,3},{16,3},{17,3},{18,3},
            {1,4},{2,4},{3,4},{4,4},{5,4},{6,4},{7,4},{8,4},{9,4},{10,4},{11,4},{12,4},{13,4},{14,4},{15,4},{16,4},{17,4},{18,4},
            {1,5},{2,5},{3,5},{4,5},{5,5},{6,5},{7,5},{8,5},{9,5},{10,5},{11,5},{12,5},{13,5},{14,5},{15,5},{16,5},{17,5},{18,5},
            {1,6},{2,6},{3,6},
            {4,9},{5,9},{6,9},{7,9},{8,9},{9,9},{10,9},{11,9},{12,9},{13,9},{14,9},{15,9},{16,9},{17,9},
            {4,6},{5,6},{6,6},{7,6},{8,6},{9,6},{10,6},{11,6},{12,6},{13,6},{14,6},{15,6},{16,6},{17,6},{18,6},
            {1,7},{2,7},{3,7},
            {4,10},{5,10},{6,10},{7,10},{8,10},{9,10},{10,10},{11,10},{12,10},{13,10},{14,10},{15,10},{16,10},{17,10},
            {4,7},{5,7},{6,7},{7,7},{8,7},{9,7},{10,7},{11,7},{12,7},{13,7},{14,7},{15,7},{16,7},{17,7},{18,7},
        };
        while (true) {
            d.fillScreen(C_BG);
            DM.header("Periodic Table");
            DM.footer("Arrows:Move", "ENT:detail", "ESC:back");
            int cw = 13, ch = 9;
            int oy = HDR_H;
            for (int i = 0; i < 118; i++) {
                int g = POS[i][0], p = POS[i][1];
                int x = (g - 1) * cw;
                int y = oy + (p - 1) * ch;
                bool isSel = (i == sel);
                uint16_t col;
                if (i < 2)             col = 0xCE7F;   // s-block  (淡青緑)
                else if (i < 4 || i==10 || i==11 || i==18 || i==19 || i==36 || i==37 || i==54 || i==55 || i==86 || i==87)
                                    col = 0xFD3F;   // group 1-2 (淡橙)
                else if (i >= 56 && i <= 70)  col = 0xFE9F;   // ランタノイド (淡黄)
                else if (i >= 88 && i <= 102) col = 0xFDBF;   // アクチノイド (淡桃)
                else if (g >= 3 && g <= 12) col = 0xCEFF;     // d-block  (淡水色)
                else                  col = 0xD7BE;           // p-block  (淡灰)
                d.fillRect(x, y, cw-1, ch-1, isSel ? C_ACC : col);
                d.setTextColor(isSel ? C_PANEL : C_FG, isSel ? C_ACC : col);
            }
            d.setTextDatum(top_left);
            char info[64];
            snprintf(info, sizeof(info), "Z=%d  %s  %.4g g/mol",
                     sel+1, PERIODIC[sel], ATOMIC_W[sel]);
            d.setTextColor(C_FG, C_BG);
            d.drawString(info, 4, LCD_H - FTR_H - 13);

            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                int curG = POS[sel][0], curP = POS[sel][1];
                if (st.fn && IS_LEFT)  {
                    int best = -1, bestG = -1;
                    for (int i = 0; i < 118; i++) {
                        if (POS[i][1] == curP && POS[i][0] < curG && POS[i][0] > bestG) {
                            best = i; bestG = POS[i][0];
                        }
                    }
                    if (best < 0) { if (sel > 0) sel--; } else sel = best;
                    break;
                }
                if (st.fn && IS_RIGHT) {
                    int best = -1, bestG = 99;
                    for (int i = 0; i < 118; i++) {
                        if (POS[i][1] == curP && POS[i][0] > curG && POS[i][0] < bestG) {
                            best = i; bestG = POS[i][0];
                        }
                    }
                    if (best < 0) { if (sel < 117) sel++; } else sel = best;
                    break;
                }
                if (st.fn && IS_UP)    {
                    int best = -1, bestP = -1;
                    for (int i = 0; i < 118; i++) {
                        if (POS[i][0] == curG && POS[i][1] < curP && POS[i][1] > bestP) {
                            best = i; bestP = POS[i][1];
                        }
                    }
                    if (best >= 0) sel = best;
                    break;
                }
                if (st.fn && IS_DOWN) {
                    int best = -1, bestP = 99;
                    for (int i = 0; i < 118; i++) {
                        if (POS[i][0] == curG && POS[i][1] > curP && POS[i][1] < bestP) {
                            best = i; bestP = POS[i][1];
                        }
                    }
                    if (best >= 0) sel = best;
                    break;
                }
                if (st.enter) { showElementDetail(sel); break; }
            }
        }
    }

    void showElementDetail(int z) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        char title[16]; snprintf(title, sizeof(title), "Element %d", z+1);
        DM.header(title);
        DM.footer("Y:weight->Ans", "", "ESC:back");
        d.setTextSize(4);
        d.setTextColor(C_ACC, C_BG);
        d.drawString(PERIODIC[z], 8, HDR_H + 8);
        d.setTextSize(1);
        char zs[16]; snprintf(zs, sizeof(zs), "Z=%d", z+1);
        d.setTextColor(C_DIM, C_BG);
        d.drawString(zs, 8, HDR_H + 56);
        char ws[24]; snprintf(ws, sizeof(ws), "%.4g g/mol", ATOMIC_W[z]);
        d.setTextColor(C_OK, C_BG);
        d.drawString(ws, 80, HDR_H + 16);
        const char* group = "";
        if (z+1 == 1)                  group = "Hydrogen";
        else if (z+1 == 2)             group = "Noble gas";
        else if (z >= 2 && z <= 3)     group = "Alkali/Earth";
        else if (z >= 56 && z <= 70)   group = "Lanthanide";
        else if (z >= 88 && z <= 102)  group = "Actinide";
        else                           group = "Other";
        d.setTextColor(C_FG, C_BG);
        d.drawString(group, 80, HDR_H + 32);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = ATOMIC_W[z]; G.ansComplex = false;
                DM.notify("-> Ans", C_OK);
                return;
            }
        }
    }

    void searchSymbol() {
        char q[24] = "";
        if (!DM.inputLine("Symbol/name:", q, sizeof(q))) return;
        if (q[0] == 0) return;
        int hits[32], n = 0;
        for (int i = 0; i < constN && n < 32; i++) {
            Constant c; memcpy_P(&c, &constDB[i], sizeof(Constant));
            if (strcasestr(c.sym, q) || strcasestr(c.name, q)) hits[n++] = i;
        }
        if (n == 0) { DM.notify("Not found", C_ERR); return; }
        char** labels = (char**)malloc(n * sizeof(char*));
        for (int i = 0; i < n; i++) {
            Constant c; memcpy_P(&c, &constDB[hits[i]], sizeof(Constant));
            labels[i] = (char*)malloc(40);
            snprintf(labels[i], 40, "%s: %s", c.sym, c.name);
        }
        const char** clabels = (const char**)labels;
        int sel = 0;
        while (true) {
            sel = DM.menu("Search Result", clabels, n, sel);
            if (sel < 0) break;
            showConstantDetail(hits[sel]);
        }
        for (int i = 0; i < n; i++) free(labels[i]);
        free(labels);
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 10 / 20 ─ Calculator モード コア部                              ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class CalculatorCore {
public:
    void run() {
        char input[80] = "";
        int  cur = 0;
        bool needRedraw = true;
        while (true) {
            if (needRedraw) { drawScreen(input, cur); needRedraw = false; }
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (st.fn) {
                if (M5Cardputer.Keyboard.isKeyPressed('1')) { settingsMenu();              needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('2')) { historyMenu(input, cur);     needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('3')) { memoryMenu();                needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('4')) { varsMenu();                  needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('5')) { baseConvert();               needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('6')) { bitOpsMenu();                needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('7')) { complexMenu();               needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('8')) { extToolMenu(input, cur);     needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('9')) { G.rpn = !G.rpn;
                    DM.notify(G.rpn ? "RPN ON" : "Algebraic", C_ACC); needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('0')) { helpScreen();                needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('=')) { G.fracDisplay = !G.fracDisplay;
                    DM.notify(G.fracDisplay ? "Fraction mode" : "Decimal mode", C_ACC); needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('a')) {
                    char ab[40]; formatNum(G.ans, ab, sizeof(ab));
                    int al = strlen(ab);
                    for (int i = 0; i < al && cur < 79; i++) input[cur++] = ab[i];
                    input[cur] = 0; needRedraw = true; continue;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('b')) { ansBankMenu(input, cur); needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed(',')) { G.dispDigits = (G.dispDigits + 1) % 10; needRedraw = true; continue; }
                if (M5Cardputer.Keyboard.isKeyPressed('/')) { G.dispDigits = (G.dispDigits + 9) % 10; needRedraw = true; continue; }
            }
            if (st.del) { if (cur > 0) input[--cur] = 0; needRedraw = true; continue; }
            if (st.enter) {
                if (input[0] == 0) { needRedraw = true; continue; }
                if (G.rpn) processRPN(input);
                else        processAlgebraic(input);
                input[0] = 0; cur = 0;
                needRedraw = true;
                continue;
            }
            for (auto c : st.word) {
                if (G.rpn && (c=='+'||c=='-'||c=='*'||c=='/'||c=='^')) {
                    bool asSign = ((c=='+'||c=='-') &&
                                   (cur==0 || input[cur-1]=='e' || input[cur-1]=='E'));
                    if (!asSign) {
                        char line[96];
                        snprintf(line, sizeof(line), "%s %c", input, c);
                        processRPN(line);
                        input[0] = 0; cur = 0;
                        needRedraw = true;
                        continue;
                    }
                }
                if (cur < 79 && c >= ' ' && c < 127) { input[cur++] = c; input[cur] = 0; }
            }
            if (!st.word.empty()) needRedraw = true;
        }
    }

private:
    void drawScreen(const char* input, int cur) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header("Calculator");
        DM.footer("Fn+0:Help", "Fn+1:Set", "Fn+9:RPN");
        int y = HDR_H + 2;
        if (G.rpn) {
            d.setTextColor(C_DIM, C_BG);
            for (int i = 3; i >= 0; i--) {
                char b[40]; formatNum(G.rpnStk[i], b, sizeof(b));
                char l[48]; snprintf(l, sizeof(l), "%c: %s", "TZYX"[i], b);
                d.drawString(l, 4, y); y += 11;
            }
        } else {
            d.setTextColor(C_DIM, C_BG);
            int start = (G.histN > 3) ? G.histN - 3 : 0;
            for (int i = start; i < G.histN; i++) {
                char l[80];
                if (G.hist[i].ok) {
                    char vs[40]; formatNum(G.hist[i].value, vs, sizeof(vs));
                    snprintf(l, sizeof(l), "%s = %s", G.hist[i].expr, vs);
                } else {
                    snprintf(l, sizeof(l), "%s = ERR", G.hist[i].expr);
                }
                if (textWidth(l) > LCD_W - 8) l[(LCD_W - 8) / 6 - 2] = 0;
                d.drawString(l, 4, y); y += 11;
            }
        }
        char ansLine[64];
        char ansStr[40];
        if (G.ansComplex) G.ansC.toString(ansStr, sizeof(ansStr));
        else if (G.fracDisplay) {
            char tmp[40];
            if (tryDecimalToSpecial(G.ans, tmp, sizeof(tmp))) strcpy(ansStr, tmp);
            else formatNum(G.ans, ansStr, sizeof(ansStr));
        } else                  formatNum(G.ans, ansStr, sizeof(ansStr));
        snprintf(ansLine, sizeof(ansLine), "Ans = %s", ansStr);
        d.setTextColor(C_OK, C_BG);
        d.drawString(ansLine, 4, LCD_H - FTR_H - 30);
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
    }

    void processAlgebraic(const char* expr) {
        auto r = gEv.eval(expr);
        if (!r.ok) {
            DM.notify(r.err ? r.err : "ERR", C_ERR, 1000);
            G.pushHist(expr, 0, false);
            return;
        }
        G.ans = r.val; G.ansComplex = false;
        G.pushHist(expr, r.val, true);
    }

    void rpnPush(double v) {
        for (int i = 3; i > 0; i--) G.rpnStk[i] = G.rpnStk[i-1];
        G.rpnStk[0] = v;
        G.rpnSp = (G.rpnSp < 4) ? G.rpnSp + 1 : 4;
    }
    double rpnPop() {
        double v = G.rpnStk[0];
        for (int i = 0; i < 3; i++) G.rpnStk[i] = G.rpnStk[i+1];
        G.rpnStk[3] = 0;
        G.rpnSp = (G.rpnSp > 0) ? G.rpnSp - 1 : 0;
        return v;
    }
    void processRPN(const char* expr) {
        char buf[80]; strncpy(buf, expr, 79); buf[79] = 0;
        char* tok = strtok(buf, " \t");
        while (tok) {
            char* end;
            double v = strtod(tok, &end);
            if (end != tok && *end == 0) {
                rpnPush(v);
            } else {
                if      (!strcmp(tok, "+"))      { double b=rpnPop(),a=rpnPop(); rpnPush(a+b); }
                else if (!strcmp(tok, "-"))      { double b=rpnPop(),a=rpnPop(); rpnPush(a-b); }
                else if (!strcmp(tok, "*"))      { double b=rpnPop(),a=rpnPop(); rpnPush(a*b); }
                else if (!strcmp(tok, "/"))      { double b=rpnPop(),a=rpnPop(); rpnPush(a/b); }
                else if (!strcmp(tok, "^"))      { double b=rpnPop(),a=rpnPop(); rpnPush(pow(a,b)); }
                else if (!strcmp(tok, "neg"))    { double a=rpnPop(); rpnPush(-a); }
                else if (!strcmp(tok, "sqrt"))   { double a=rpnPop(); rpnPush(sqrt(a)); }
                else if (!strcmp(tok, "sin"))    { double a=rpnPop(); rpnPush(sin(toRad(a))); }
                else if (!strcmp(tok, "cos"))    { double a=rpnPop(); rpnPush(cos(toRad(a))); }
                else if (!strcmp(tok, "tan"))    { double a=rpnPop(); rpnPush(tan(toRad(a))); }
                else if (!strcmp(tok, "ln"))     { double a=rpnPop(); rpnPush(log(a)); }
                else if (!strcmp(tok, "log"))    { double a=rpnPop(); rpnPush(log10(a)); }
                else if (!strcmp(tok, "exp"))    { double a=rpnPop(); rpnPush(exp(a)); }
                else if (!strcmp(tok, "inv"))    { double a=rpnPop(); rpnPush(1.0/a); }
                else if (!strcmp(tok, "swap")) {
                    double a = G.rpnStk[0]; G.rpnStk[0] = G.rpnStk[1]; G.rpnStk[1] = a;
                }
                else if (!strcmp(tok, "dup"))    rpnPush(G.rpnStk[0]);
                else if (!strcmp(tok, "drop"))   rpnPop();
                else if (!strcmp(tok, "clr"))    {
                    for (int i=0;i<4;i++) G.rpnStk[i] = 0;
                    G.rpnSp = 0;
                }
                else {
                    auto r = gEv.eval(tok);
                    if (r.ok) rpnPush(r.val);
                    else { DM.notify("RPN ERR", C_ERR, 800); return; }
                }
            }
            tok = strtok(nullptr, " \t");
        }
        G.ans = G.rpnStk[0]; G.ansComplex = false;
        G.pushHist(expr, G.ans, true);
    }

    void settingsMenu() {
        while (true) {
            const char* items[] = {
                "Angle: DEG/RAD/GRAD","Display: Norm/Fix/Sci/Eng","Display digits",
                "Norm 1/2 threshold","Reset all","Back"
            };
            int s = DM.menu("Settings", items, 6);
            if (s < 0 || s == 5) return;
            if (s == 0) {
                const char* a[] = {"DEG","RAD","GRAD"};
                int x = DM.menu("Angle", a, 3, G.angle);
                if (x >= 0) G.angle = (AngleMode)x;
            }
            else if (s == 1) {
                const char* d[] = {"Norm","Fix","Sci","Eng"};
                int x = DM.menu("Display", d, 4, G.disp);
                if (x >= 0) G.disp = (DispMode)x;
            }
            else if (s == 2) {
                char b[8]; snprintf(b, sizeof(b), "%d", G.dispDigits);
                if (DM.inputLine("digits 0-9:", b, sizeof(b))) {
                    int v = atoi(b);
                    if (v >= 0 && v <= 9) G.dispDigits = v;
                }
            }
            else if (s == 3) {
                const char* n[] = {"Norm 1","Norm 2"};
                int x = DM.menu("Norm", n, 2, G.normMode - 1);
                if (x >= 0) G.normMode = x + 1;
            }
            else if (s == 4) {
                if (DM.confirm("Reset all?")) { G.resetAll(); DM.notify("Reset done", C_OK); }
            }
        }
    }

    void historyMenu(char* input, int& cur) {
        if (G.histN == 0) { DM.notify("Empty", C_DIM); return; }
        char** labels = (char**)malloc(G.histN * sizeof(char*));
        for (int i = 0; i < G.histN; i++) {
            labels[i] = (char*)malloc(80);
            char vs[40]; formatNum(G.hist[i].value, vs, sizeof(vs));
            snprintf(labels[i], 80, "%s=%s", G.hist[i].expr, vs);
        }
        const char** clabels = (const char**)labels;
        int s = DM.menu("History (recall)", clabels, G.histN, G.histN - 1);
        if (s >= 0) { strncpy(input, G.hist[s].expr, 79); input[79] = 0; cur = strlen(input); }
        for (int i = 0; i < G.histN; i++) free(labels[i]);
        free(labels);
    }

    void memoryMenu() {
        while (true) {
            char ms[40]; formatNum(G.mem, ms, sizeof(ms));
            char title[48]; snprintf(title, sizeof(title), "Memory M=%s", ms);
            const char* items[] = {
                "M+ (M += Ans)","M- (M -= Ans)","MR (Ans <- M)","MC (M = 0)","Set M = ...","Back"
            };
            int s = DM.menu(title, items, 6);
            if (s < 0 || s == 5) return;
            switch (s) {
                case 0: G.mem += G.ans; break;
                case 1: G.mem -= G.ans; break;
                case 2: G.ans = G.mem; G.ansComplex = false; DM.notify("MR -> Ans", C_OK); break;
                case 3: G.mem = 0; DM.notify("Cleared", C_OK); break;
                case 4: {
                    char b[24] = "0"; formatNum(G.mem, b, sizeof(b));
                    if (DM.inputLine("M = ", b, sizeof(b))) {
                        auto r = gEv.eval(b);
                        if (r.ok) G.mem = r.val;
                    }
                } break;
            }
        }
    }

    void varsMenu() {
        while (true) {
            char** labels = (char**)malloc(VAR_COUNT * sizeof(char*));
            for (int i = 0; i < VAR_COUNT; i++) {
                labels[i] = (char*)malloc(40);
                char vs[24]; formatNum(G.vars[i], vs, sizeof(vs));
                snprintf(labels[i], 40, "%c = %s", 'A'+i, vs);
            }
            const char** clabels = (const char**)labels;
            int s = DM.menu("Variables A..J", clabels, VAR_COUNT);
            for (int i = 0; i < VAR_COUNT; i++) free(labels[i]);
            free(labels);
            if (s < 0) return;
            char b[40]; formatNum(G.vars[s], b, sizeof(b));
            char prompt[16]; snprintf(prompt, sizeof(prompt), "%c = ", 'A'+s);
            if (DM.inputLine(prompt, b, sizeof(b))) {
                auto r = gEv.eval(b);
                if (r.ok) G.vars[s] = r.val;
            }
        }
    }

    void ansBankMenu(char* input, int& cur) {
        while (true) {
            int nItems = G.bankN + 4;
            char** labels = (char**)malloc(nItems * sizeof(char*));
            for (int i = 0; i < G.bankN; i++) {
                labels[i] = (char*)malloc(48);
                char vs[24]; formatNum(G.bank[i], vs, sizeof(vs));
                if (G.bankLabel[i][0]) snprintf(labels[i], 48, "%-8s = %s", G.bankLabel[i], vs);
                else                   snprintf(labels[i], 48, "[%d]      = %s", i, vs);
            }
            int op = G.bankN;
            labels[op] = (char*)malloc(48); snprintf(labels[op], 48, "[+] Add current Ans");
            labels[op+1] = (char*)malloc(48); snprintf(labels[op+1], 48, "[V] Pop to variable A..J");
            labels[op+2] = (char*)malloc(48); snprintf(labels[op+2], 48, "[C] Clear all bank");
            labels[op+3] = (char*)malloc(48); snprintf(labels[op+3], 48, "[<-] Back");
            const char** clabels = (const char**)labels;
            char title[32]; snprintf(title, sizeof(title), "Ans Bank (%d/%d)", G.bankN, ANS_BANK_MAX);
            int s = DM.menu(title, clabels, nItems);
            for (int i = 0; i < nItems; i++) free(labels[i]);
            free(labels);
            if (s < 0 || s == op + 3) return;
            if (s < G.bankN) {
                const char* sub[] = {
                    "Insert into expression","-> Ans (replace)","-> Variable A..J",
                    "Delete this entry","Back"
                };
                int ss = DM.menu(G.bankLabel[s][0] ? G.bankLabel[s] : "Entry", sub, 5);
                if (ss < 0 || ss == 4) continue;
                if (ss == 0) {
                    char ab[40]; formatNum(G.bank[s], ab, sizeof(ab));
                    int al = strlen(ab);
                    for (int i = 0; i < al && cur < 79; i++) input[cur++] = ab[i];
                    input[cur] = 0;
                    DM.notify("Inserted", C_OK);
                    return;
                } else if (ss == 1) {
                    G.ans = G.bank[s]; G.ansComplex = false;
                    DM.notify("-> Ans", C_OK);
                } else if (ss == 2) {
                    const char* va[] = {"A","B","C","D","E","F","G","H","I","J"};
                    int v = DM.menu("Save to var", va, 10);
                    if (v >= 0) {
                        G.vars[v] = G.bank[s];
                        char m[24]; snprintf(m, sizeof(m), "-> %c", 'A'+v);
                        DM.notify(m, C_OK);
                    }
                } else if (ss == 3) {
                    for (int k = s; k < G.bankN - 1; k++) {
                        G.bank[k] = G.bank[k+1];
                        memcpy(G.bankLabel[k], G.bankLabel[k+1], 16);
                    }
                    G.bankN--;
                    DM.notify("Deleted", C_OK);
                }
            } else if (s == op) {
                if (G.bankN < ANS_BANK_MAX) {
                    G.bank[G.bankN] = G.ans;
                    snprintf(G.bankLabel[G.bankN], 16, "Ans");
                    G.bankN++;
                    DM.notify("Added", C_OK);
                } else {
                    DM.notify("Bank full", C_ERR);
                }
            } else if (s == op + 1) {
                if (G.bankN == 0) { DM.notify("Bank empty", C_WARN); continue; }
                const char* va[] = {"A","B","C","D","E","F","G","H","I","J"};
                int v = DM.menu("Pop top -> var", va, 10);
                if (v >= 0) {
                    G.vars[v] = G.bank[G.bankN - 1];
                    G.bankN--;
                    DM.notify("Popped", C_OK);
                }
            } else if (s == op + 2) {
                if (DM.confirm("Clear all bank?")) { G.bankN = 0; DM.notify("Cleared", C_OK); }
            }
        }
    }

    void baseConvert() {
        char b[40]; formatNum(G.ans, b, sizeof(b));
        if (!DM.inputLine("Number:", b, sizeof(b))) return;
        long long v;
        if (b[0] == '0' && (b[1] == 'x' || b[1] == 'X')) v = strtoll(b+2, nullptr, 16);
        else if (b[0] == '0' && (b[1] == 'b' || b[1] == 'B')) v = strtoll(b+2, nullptr, 2);
        else if (b[0] == '0' && (b[1] == 'o' || b[1] == 'O')) v = strtoll(b+2, nullptr, 8);
        else if (b[0] == '0' && b[1] >= '0' && b[1] <= '7')   v = strtoll(b+1, nullptr, 8);
        else                                                  v = (long long)atof(b);

        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header("Base Convert");
        DM.footer("Y:Ans","","ESC:back");
        char dec[40], hex[40], oct[40], bin[80];
        formatInt(v, dec, sizeof(dec), BASE_DEC);
        formatInt(v, hex, sizeof(hex), BASE_HEX);
        formatInt(v, oct, sizeof(oct), BASE_OCT);
        formatInt(v, bin, sizeof(bin), BASE_BIN);
        d.setTextColor(C_FG, C_BG);
        d.drawString("DEC:", 4, HDR_H + 4); d.drawString(dec, 50, HDR_H + 4);
        d.drawString("HEX:", 4, HDR_H + 18); d.drawString(hex, 50, HDR_H + 18);
        d.drawString("OCT:", 4, HDR_H + 32); d.drawString(oct, 50, HDR_H + 32);
        d.drawString("BIN:", 4, HDR_H + 46);
        int bl = strlen(bin), maxCh = (LCD_W - 50)/6;
        int yy = HDR_H + 46;
        for (int i = 0; i < bl && yy < LCD_H - FTR_H; i += maxCh) {
            char tmp[64]; int sz = (bl-i < maxCh) ? bl-i : maxCh;
            memcpy(tmp, bin+i, sz); tmp[sz] = 0;
            d.drawString(tmp, 50, yy);
            yy += 11;
        }
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = (double)v; G.ansComplex = false;
                DM.notify("-> Ans", C_OK); return;
            }
        }
    }

    void bitOpsMenu() {
        const char* items[] = {"AND","OR","XOR","NOT","SHL","SHR","Back"};
        int s = DM.menu("Bit Op", items, 7);
        if (s < 0 || s == 6) return;
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
        long long r = 0;
        switch (s) {
            case 0: r = a & bb;  break;
            case 1: r = a | bb;  break;
            case 2: r = a ^ bb;  break;
            case 3: r = ~a;      break;
            case 4: r = a << bb; break;
            case 5: r = a >> bb; break;
        }
        char dec[40], hex[40], bin[80];
        formatInt(r, dec, sizeof(dec), BASE_DEC);
        formatInt(r, hex, sizeof(hex), BASE_HEX);
        formatInt(r, bin, sizeof(bin), BASE_BIN);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Bit Result"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        d.drawString(dec, 4, HDR_H + 8);
        d.setTextColor(C_FG, C_BG);
        d.drawString(hex, 4, HDR_H + 24);
        DM.drawWrapped(bin, 4, HDR_H + 40, LCD_W - 8, C_DIM);
        G.ans = (double)r;
        DM.waitEsc();
    }

    void complexMenu() {
        const char* items[] = {
            "z = a+bi","z + w","z - w","z * w","z / w","|z|, arg z, conj z",
            "z polar form","z^n","z^(1/n)","exp/sin/cos/log","Back"
        };
        int s = DM.menu("Complex", items, 11);
        if (s < 0 || s == 10) return;
        Complex z, w;
        char b[24];
        strcpy(b, "0"); if (!DM.inputLine("Re(z):", b, sizeof(b))) return; z.re = atof(b);
        strcpy(b, "0"); if (!DM.inputLine("Im(z):", b, sizeof(b))) return; z.im = atof(b);
        if (s >= 1 && s <= 4) {
            strcpy(b, "0"); if (!DM.inputLine("Re(w):", b, sizeof(b))) return; w.re = atof(b);
            strcpy(b, "0"); if (!DM.inputLine("Im(w):", b, sizeof(b))) return; w.im = atof(b);
        }
        Complex r;
        switch (s) {
            case 0: r = z; break;
            case 1: r = z + w; break;
            case 2: r = z - w; break;
            case 3: r = z * w; break;
            case 4: r = z / w; break;
            case 5: {
                auto& d = M5Cardputer.Display;
                d.fillScreen(C_BG); DM.header("|z|, arg, conj"); DM.footer("","","ESC:back");
                d.setTextColor(C_FG, C_BG);
                char l[48];
                snprintf(l, sizeof(l), "|z|  = %g", z.abs());     d.drawString(l, 4, HDR_H+8);
                snprintf(l, sizeof(l), "arg  = %g", fromRad(z.arg())); d.drawString(l, 4, HDR_H+22);
                Complex c = z.conj(); c.toString(l, sizeof(l));
                d.drawString("conj = ", 4, HDR_H+36);
                d.drawString(l, 50, HDR_H+36);
                G.ans = z.abs(); G.ansComplex = false;
                DM.waitEsc();
                return;
            }
            case 6: {
                auto& d = M5Cardputer.Display;
                d.fillScreen(C_BG); DM.header("Polar Form"); DM.footer("","","ESC:back");
                d.setTextColor(C_FG, C_BG);
                char l[48];
                z.toString(l, sizeof(l), false); d.drawString("Cart: ", 4, HDR_H+8); d.drawString(l, 50, HDR_H+8);
                z.toString(l, sizeof(l), true);  d.drawString("Polar:", 4, HDR_H+22); d.drawString(l, 50, HDR_H+22);
                DM.waitEsc();
                return;
            }
            case 7: {
                strcpy(b,"2"); if (!DM.inputLine("n:", b, sizeof(b))) return;
                r = z.powN(atof(b));
            } break;
            case 8: {
                strcpy(b,"2"); if (!DM.inputLine("n:", b, sizeof(b))) return;
                r = z.powN(1.0/atof(b));
            } break;
            case 9: {
                const char* fns[] = {"exp(z)","sin(z)","cos(z)","ln(z)","Back"};
                int x = DM.menu("Function", fns, 5);
                if (x < 0 || x == 4) return;
                if (x == 0) r = z.expC();
                if (x == 1) r = z.sinC();
                if (x == 2) r = z.cosC();
                if (x == 3) r = z.logC();
            } break;
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Complex Result"); DM.footer("Y:Ans","","ESC:back");
        char rs[64]; r.toString(rs, sizeof(rs), false);
        d.setTextColor(C_OK, C_BG);
        d.setTextSize(2);
        d.drawString(rs, 4, HDR_H + 12);
        d.setTextSize(1);
        char rs2[64]; r.toString(rs2, sizeof(rs2), true);
        d.setTextColor(C_DIM, C_BG);
        d.drawString(rs2, 4, HDR_H + 40);
        G.ansC = r; G.ansComplex = true; G.ans = r.re;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') ||
                M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
        }
    }

    void extToolMenu(char* input, int& cur);

    void helpScreen() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Calculator Help"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        const char* lines[] = {
            "Fn+1 Settings   Fn+2 History",
            "Fn+3 Memory M   Fn+4 Vars A..J",
            "Fn+5 BaseConv   Fn+6 BitOps",
            "Fn+7 Complex    Fn+8 Tools",
            "Fn+9 RPN/Alg    Fn+0 Help",
            "Fn+= S<->D Frac mode",
            "Fn+a Insert Ans",
            "Fn+b Ans Bank (multi values)",
            "ENT eval   DEL backspace"
        };
        int n = sizeof(lines)/sizeof(char*);
        for (int i = 0; i < n; i++) d.drawString(lines[i], 4, HDR_H + i*11);
        DM.waitEsc();
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 11 / 20 ─ 方程式ソルバー / 数値解析 / ベクトル・行列 / 幾何     ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class EqSolver {
public:
    void run() {
        const char* items[] = {
            "Newton-Raphson f(x)=0","Bisection f(x)=0","Quadratic ax^2+bx+c",
            "Cubic ax^3+...","Quartic ax^4+...","Linear System (n<=5)",
            "General Polynomial","Back"
        };
        int s = DM.menu("Equation Solver", items, 8);
        if (s < 0 || s == 7) return;
        char b[80];
        switch (s) {
            case 0: {
                strcpy(b, "x^2 - 2"); if (!DM.inputLine("f(x)=", b, sizeof(b))) return;
                char xb[24] = "1"; if (!DM.inputLine("x0:", xb, sizeof(xb))) return;
                double r;
                if (NumAnalysis::solveNR(b, 'x', atof(xb), r)) showResult("Root", r);
                else                                          DM.notify("No conv", C_ERR);
            } break;
            case 1: {
                strcpy(b, "x^2 - 2"); if (!DM.inputLine("f(x)=", b, sizeof(b))) return;
                char ab[24] = "0"; if (!DM.inputLine("a:", ab, sizeof(ab))) return;
                char bb[24] = "2"; if (!DM.inputLine("b:", bb, sizeof(bb))) return;
                double r;
                if (NumAnalysis::bisect(b, 'x', atof(ab), atof(bb), r)) showResult("Root", r);
                else                                                    DM.notify("No sign change", C_ERR);
            } break;
            case 2: {
                double a, c2, c3;
                strcpy(b, "1"); if (!DM.inputLine("a:", b, sizeof(b))) return; a = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("b:", b, sizeof(b))) return; c2 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("c:", b, sizeof(b))) return; c3 = atof(b);
                double rr[2], ii[2];
                int n = NumAnalysis::quadratic(a, c2, c3, rr, ii);
                showRoots("Quadratic", rr, ii, n);
            } break;
            case 3: {
                double a, c2, c3, c4;
                strcpy(b, "1"); if (!DM.inputLine("a:", b, sizeof(b))) return; a = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("b:", b, sizeof(b))) return; c2 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("c:", b, sizeof(b))) return; c3 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("d:", b, sizeof(b))) return; c4 = atof(b);
                double rr[3], ii[3];
                int n = NumAnalysis::cubic(a, c2, c3, c4, rr, ii);
                showRoots("Cubic", rr, ii, n);
            } break;
            case 4: {
                double a, c2, c3, c4, c5;
                strcpy(b, "1"); if (!DM.inputLine("a:", b, sizeof(b))) return; a = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("b:", b, sizeof(b))) return; c2 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("c:", b, sizeof(b))) return; c3 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("d:", b, sizeof(b))) return; c4 = atof(b);
                strcpy(b, "0"); if (!DM.inputLine("e:", b, sizeof(b))) return; c5 = atof(b);
                double rr[4], ii[4];
                int n = NumAnalysis::quartic(a, c2, c3, c4, c5, rr, ii);
                showRoots("Quartic", rr, ii, n);
            } break;
            case 5: {
                strcpy(b, "3"); if (!DM.inputLine("Size n (2-5):", b, sizeof(b))) return;
                int n = atoi(b);
                if (n < 2 || n > 5) { DM.notify("2..5", C_ERR); return; }
                double A[25], rhs[5];
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        char p[16]; snprintf(p, sizeof(p), "A[%d][%d]:", i+1, j+1);
                        char x[16] = "0";
                        if (!DM.inputLine(p, x, sizeof(x))) return;
                        A[i*n+j] = atof(x);
                    }
                    char p[16]; snprintf(p, sizeof(p), "b[%d]:", i+1);
                    char x[16] = "0";
                    if (!DM.inputLine(p, x, sizeof(x))) return;
                    rhs[i] = atof(x);
                }
                if (Mat::gaussSolve(A, n, rhs)) {
                    auto& d = M5Cardputer.Display;
                    d.fillScreen(C_BG); DM.header("Linear Solution"); DM.footer("","","ESC:back");
                    d.setTextColor(C_OK, C_BG);
                    for (int i = 0; i < n; i++) {
                        char l[32]; char vs[24]; formatNum(rhs[i], vs, sizeof(vs));
                        snprintf(l, sizeof(l), "x%d = %s", i+1, vs);
                        d.drawString(l, 4, HDR_H + 4 + i*12);
                    }
                    G.ans = rhs[0]; G.ansComplex = false;
                    DM.waitEsc();
                } else {
                    DM.notify("Singular", C_ERR);
                }
            } break;
            case 6: {
                strcpy(b, "5"); if (!DM.inputLine("Degree (1-8):", b, sizeof(b))) return;
                int deg = atoi(b);
                if (deg < 1 || deg > 8) { DM.notify("1..8", C_ERR); return; }
                double coef[9];
                for (int i = deg; i >= 0; i--) {
                    char p[16]; snprintf(p, sizeof(p), "x^%d:", i);
                    char x[16] = "0";
                    if (!DM.inputLine(p, x, sizeof(x))) return;
                    coef[i] = atof(x);
                }
                Complex roots[16];
                int n = NumAnalysis::polyRoots(coef, deg, roots);
                auto& d = M5Cardputer.Display;
                d.fillScreen(C_BG); DM.header("Polynomial Roots"); DM.footer("","","ESC:back");
                d.setTextColor(C_OK, C_BG);
                for (int i = 0; i < n && i < 8; i++) {
                    char rs[32]; roots[i].toString(rs, sizeof(rs));
                    char l[40]; snprintf(l, sizeof(l), "r%d = %s", i+1, rs);
                    d.drawString(l, 4, HDR_H + 4 + i*12);
                }
                G.ansC = roots[0]; G.ansComplex = true; G.ans = roots[0].re;
                DM.waitEsc();
            } break;
        }
    }

private:
    void showResult(const char* name, double v) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(name); DM.footer("Y:Ans","","ESC:back");
        char vs[40]; formatNum(v, vs, sizeof(vs));
        d.setTextSize(2);
        d.setTextColor(C_OK, C_BG);
        d.drawString(vs, 4, HDR_H + 12);
        d.setTextSize(1);
        G.ans = v; G.ansComplex = false;
        DM.waitEsc();
    }
    void showRoots(const char* title, double* re, double* im, int n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("Y:r1->Ans","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        for (int i = 0; i < n; i++) {
            char l[40];
            if (fabs(im[i]) < 1e-9) {
                char vs[24]; formatNum(re[i], vs, sizeof(vs));
                snprintf(l, sizeof(l), "x%d = %s", i+1, vs);
            } else if (im[i] > 0) {
                snprintf(l, sizeof(l), "x%d = %g+%gi", i+1, re[i], im[i]);
            } else {
                snprintf(l, sizeof(l), "x%d = %g%gi", i+1, re[i], im[i]);
            }
            d.drawString(l, 4, HDR_H + 4 + i*12);
        }
        G.ans = re[0]; G.ansComplex = (fabs(im[0]) > 1e-9);
        if (G.ansComplex) G.ansC = Complex(re[0], im[0]);
        DM.waitEsc();
    }
};

class NumMenu {
public:
    void run() {
        const char* items[] = {
            "d/dx f(x) at a","Integral f(x) [a,b]","Sum f(i) [a,b]",
            "Product f(i) [a,b]","Back"
        };
        int s = DM.menu("Numerical Analysis", items, 5);
        if (s < 0 || s == 4) return;
        char e[80] = "x^2";
        if (!DM.inputLine("f(x)=", e, sizeof(e))) return;
        char b[24];
        if (s == 0) {
            strcpy(b, "1"); if (!DM.inputLine("at x=", b, sizeof(b))) return;
            double err = 0;
            double r = NumAnalysis::diffN(e, 'x', atof(b), 0.0, &err);
            showResult("df/dx", r, err);
        } else if (s == 1) {
            char a[24] = "0", c[24] = "1";
            if (!DM.inputLine("a:", a, sizeof(a))) return;
            if (!DM.inputLine("b:", c, sizeof(c))) return;
            double err = 0;
            double r = NumAnalysis::integN(e, 'x', atof(a), atof(c), 0, &err);
            showResult("Integral", r, err);
        } else if (s == 2) {
            char a[24] = "1", c[24] = "10";
            if (!DM.inputLine("a:", a, sizeof(a))) return;
            if (!DM.inputLine("b:", c, sizeof(c))) return;
            double r = NumAnalysis::sumN(e, 'x', atof(a), atof(c));
            showResult("Sum", r);
        } else if (s == 3) {
            char a[24] = "1", c[24] = "5";
            if (!DM.inputLine("a:", a, sizeof(a))) return;
            if (!DM.inputLine("b:", c, sizeof(c))) return;
            double r = NumAnalysis::prodN(e, 'x', atof(a), atof(c));
            showResult("Product", r);
        }
    }
private:
    void showResult(const char* nm, double v, double err = -1) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(nm); DM.footer("Y:Ans","","ESC:back");
        char vs[40]; formatNum(v, vs, sizeof(vs));
        d.setTextSize(2);
        d.setTextColor(C_OK, C_BG);
        d.drawString(vs, 4, HDR_H + 12);
        d.setTextSize(1);
        if (err >= 0) {
            char es[48]; snprintf(es, sizeof(es), "est. err ~ %.2e", err);
            d.setTextColor(C_DIM, C_BG);
            d.drawString(es, 4, HDR_H + 40);
        }
        G.ans = v; G.ansComplex = false;
        DM.waitEsc();
    }
};

class VecMatCalc {
public:
    void run() {
        const char* items[] = {
            "Edit Vector A","Edit Vector B","Vector ops (A.B, AxB, |A|, A+B, A-B)",
            "Edit Matrix A","Edit Matrix B","Matrix ops (mul, det, inv, T, rank, eig)","Back"
        };
        int s = DM.menu("Vec / Mat", items, 7);
        if (s < 0 || s == 6) return;
        switch (s) {
            case 0: editVec(0); break;
            case 1: editVec(1); break;
            case 2: vecOps();   break;
            case 3: editMat(0); break;
            case 4: editMat(1); break;
            case 5: matOps();   break;
        }
    }
private:
    void editVec(int sel) {
        double* v = (sel == 0) ? G.vecA : G.vecB;
        int* dim  = (sel == 0) ? &G.vecAdim : &G.vecBdim;
        char b[8]; snprintf(b, sizeof(b), "%d", *dim);
        if (!DM.inputLine("Dim 2-10:", b, sizeof(b))) return;
        int d = atoi(b); if (d < 2 || d > 10) { DM.notify("2..10", C_ERR); return; }
        *dim = d;
        for (int i = 0; i < d; i++) {
            char p[12]; snprintf(p, sizeof(p), "%c[%d]:", 'A'+sel, i+1);
            char x[24]; formatNum(v[i], x, sizeof(x));
            if (!DM.inputLine(p, x, sizeof(x))) return;
            v[i] = atof(x);
        }
        DM.notify("Saved", C_OK);
    }
    void vecOps() {
        const char* items[] = {"A . B (dot)","A x B (cross 3D)","|A|","|B|","A + B","A - B","Back"};
        int s = DM.menu("Vector Op", items, 7);
        if (s < 0 || s == 6) return;
        if (G.vecAdim != G.vecBdim && (s == 0 || s == 4 || s == 5)) {
            DM.notify("Dim mismatch", C_ERR); return;
        }
        double r = 0; double rv[10];
        bool isVec = false;
        if (s == 0) { for (int i = 0; i < G.vecAdim; i++) r += G.vecA[i] * G.vecB[i]; }
        else if (s == 1) {
            if (G.vecAdim != 3 || G.vecBdim != 3) { DM.notify("3D only", C_ERR); return; }
            rv[0] = G.vecA[1]*G.vecB[2] - G.vecA[2]*G.vecB[1];
            rv[1] = G.vecA[2]*G.vecB[0] - G.vecA[0]*G.vecB[2];
            rv[2] = G.vecA[0]*G.vecB[1] - G.vecA[1]*G.vecB[0];
            isVec = true;
        }
        else if (s == 2) { for (int i = 0; i < G.vecAdim; i++) r += G.vecA[i]*G.vecA[i]; r = sqrt(r); }
        else if (s == 3) { for (int i = 0; i < G.vecBdim; i++) r += G.vecB[i]*G.vecB[i]; r = sqrt(r); }
        else if (s == 4) { for (int i = 0; i < G.vecAdim; i++) rv[i] = G.vecA[i] + G.vecB[i]; isVec = true; }
        else if (s == 5) { for (int i = 0; i < G.vecAdim; i++) rv[i] = G.vecA[i] - G.vecB[i]; isVec = true; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Vec Result"); DM.footer("Y:Ans","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        if (isVec) {
            int dim = (s == 1) ? 3 : G.vecAdim;
            char l[40]; l[0] = '('; l[1] = 0;
            for (int i = 0; i < dim; i++) {
                char vs[16]; formatNum(rv[i], vs, sizeof(vs));
                strncat(l, vs, sizeof(l) - strlen(l) - 2);
                if (i < dim-1) strcat(l, ", ");
            }
            strcat(l, ")");
            d.drawString(l, 4, HDR_H + 12);
            G.ans = rv[0];
        } else {
            char vs[24]; formatNum(r, vs, sizeof(vs));
            d.setTextSize(2); d.drawString(vs, 4, HDR_H + 12); d.setTextSize(1);
            G.ans = r;
        }
        G.ansComplex = false;
        DM.waitEsc();
    }
    void editMat(int sel) {
        double* M = (sel == 0) ? G.matA : G.matB;
        int* nr = (sel == 0) ? &G.matAr : &G.matBr;
        int* nc = (sel == 0) ? &G.matAc : &G.matBc;
        char b[8] = "3";
        if (!DM.inputLine("Rows (1-10):", b, sizeof(b))) return;
        int r = atoi(b); if (r < 1 || r > MAT_MAX) { DM.notify("Range", C_ERR); return; }
        strcpy(b, "3");
        if (!DM.inputLine("Cols (1-10):", b, sizeof(b))) return;
        int c = atoi(b); if (c < 1 || c > MAT_MAX) { DM.notify("Range", C_ERR); return; }
        *nr = r; *nc = c;
        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++) {
                char p[16]; snprintf(p, sizeof(p), "%c[%d][%d]:", 'A'+sel, i+1, j+1);
                char x[24]; formatNum(M[i*MAT_MAX+j], x, sizeof(x));
                if (!DM.inputLine(p, x, sizeof(x))) return;
                M[i*MAT_MAX+j] = atof(x);
            }
        DM.notify("Saved", C_OK);
    }
    void matOps() {
        const char* items[] = {
            "A * B","A + B","A - B","Transpose A","det A","inv A","rank A",
            "Eigenvalues A","Show A","Show B","Back"
        };
        int s = DM.menu("Matrix Op", items, 11);
        if (s < 0 || s == 10) return;
        if (G.matAr == 0 || G.matAc == 0) {
            DM.notify("Edit Matrix A first", C_WARN);
            return;
        }
        auto pack = [](double* dst, double* src, int r, int c){
            for (int i = 0; i < r; i++) for (int j = 0; j < c; j++) dst[i*c+j] = src[i*MAT_MAX+j];
        };
        double A[100], B[100], R[100];
        switch (s) {
            case 0: {
                if (G.matAc != G.matBr) { DM.notify("Dim mismatch", C_ERR); return; }
                pack(A, G.matA, G.matAr, G.matAc);
                pack(B, G.matB, G.matBr, G.matBc);
                Mat::mul(A, G.matAr, G.matAc, B, G.matBc, R);
                showMat("A*B", R, G.matAr, G.matBc);
            } break;
            case 1: case 2: {
                if (G.matAr != G.matBr || G.matAc != G.matBc) { DM.notify("Dim mismatch", C_ERR); return; }
                pack(A, G.matA, G.matAr, G.matAc);
                pack(B, G.matB, G.matBr, G.matBc);
                int sz = G.matAr * G.matAc;
                if (s == 1) for (int i = 0; i < sz; i++) R[i] = A[i] + B[i];
                else        for (int i = 0; i < sz; i++) R[i] = A[i] - B[i];
                showMat(s == 1 ? "A+B" : "A-B", R, G.matAr, G.matAc);
            } break;
            case 3: {
                pack(A, G.matA, G.matAr, G.matAc);
                Mat::transpose(A, G.matAr, G.matAc, R);
                showMat("A^T", R, G.matAc, G.matAr);
            } break;
            case 4: {
                if (G.matAr != G.matAc) { DM.notify("Square only", C_ERR); return; }
                pack(A, G.matA, G.matAr, G.matAc);
                double v = Mat::det(A, G.matAr);
                showScalar("det A", v);
            } break;
            case 5: {
                if (G.matAr != G.matAc) { DM.notify("Square only", C_ERR); return; }
                pack(A, G.matA, G.matAr, G.matAc);
                if (Mat::inv(A, G.matAr, R)) showMat("A^-1", R, G.matAr, G.matAr);
                else                          DM.notify("Singular", C_ERR);
            } break;
            case 6: {
                pack(A, G.matA, G.matAr, G.matAc);
                int rk = Mat::rank(A, G.matAr, G.matAc);
                showScalar("rank A", rk);
            } break;
            case 7: {
                if (G.matAr != G.matAc) { DM.notify("Square only", C_ERR); return; }
                pack(A, G.matA, G.matAr, G.matAc);
                double eig[10], V[100];
                bool sym = true;
                for (int i = 0; i < G.matAr; i++)
                    for (int j = i+1; j < G.matAr; j++)
                        if (fabs(A[i*G.matAr+j] - A[j*G.matAr+i]) > 1e-9) { sym = false; break; }
                if (sym) {
                    Mat::jacobiEig(A, G.matAr, eig, V);
                    showVec("Eigenvalues", eig, G.matAr);
                } else {
                    double er[10] = {0}, ei[10] = {0};
                    Mat::qrEig(A, G.matAr, er, ei);
                    auto& d = M5Cardputer.Display;
                    d.fillScreen(C_BG); DM.header("Eigenvalues (general)"); DM.footer("","","ESC:back");
                    d.setTextColor(C_OK, C_BG);
                    for (int i = 0; i < G.matAr; i++) {
                        char l[40];
                        if (fabs(ei[i]) < 1e-9)      snprintf(l, sizeof(l), "lambda%d = %g", i+1, er[i]);
                        else if (ei[i] > 0)          snprintf(l, sizeof(l), "lambda%d = %g+%gi", i+1, er[i], ei[i]);
                        else                         snprintf(l, sizeof(l), "lambda%d = %g%gi",  i+1, er[i], ei[i]);
                        d.drawString(l, 4, HDR_H + 4 + i*11);
                    }
                    DM.waitEsc();
                }
            } break;
            case 8: { pack(A, G.matA, G.matAr, G.matAc); showMat("Matrix A", A, G.matAr, G.matAc); } break;
            case 9: { pack(B, G.matB, G.matBr, G.matBc); showMat("Matrix B", B, G.matBr, G.matBc); } break;
        }
    }
    void showMat(const char* title, double* M, int r, int c) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        int cellW = (LCD_W - 8) / c;
        if (cellW < 24) cellW = 24;
        for (int i = 0; i < r && HDR_H + i*12 < LCD_H - FTR_H - 4; i++) {
            for (int j = 0; j < c; j++) {
                char vs[16]; formatNum(M[i*c+j], vs, sizeof(vs));
                d.drawString(vs, 4 + j*cellW, HDR_H + 4 + i*12);
            }
        }
        DM.waitEsc();
    }
    void showVec(const char* title, double* V, int n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        for (int i = 0; i < n && HDR_H + i*12 < LCD_H - FTR_H - 4; i++) {
            char l[32]; char vs[24]; formatNum(V[i], vs, sizeof(vs));
            snprintf(l, sizeof(l), "%d: %s", i+1, vs);
            d.drawString(l, 4, HDR_H + 4 + i*12);
        }
        DM.waitEsc();
    }
    void showScalar(const char* title, double v) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("Y:Ans","","ESC:back");
        char vs[40]; formatNum(v, vs, sizeof(vs));
        d.setTextSize(2);
        d.setTextColor(C_OK, C_BG);
        d.drawString(vs, 4, HDR_H + 12);
        d.setTextSize(1);
        G.ans = v; G.ansComplex = false;
        DM.waitEsc();
    }
};

class GeomCalc {
public:
    void run() {
        const char* items[] = {
            "Triangle (3 sides)","Triangle (b,h)","Triangle (SAS)","Rectangle","Square",
            "Parallelogram","Trapezoid","Rhombus","Circle","Annulus","Ellipse","Sector",
            "Polygon (n,r)","Regular polygon","Cube","Rect Prism","Sphere","Hemisphere",
            "Cylinder","Cone","Frustum","Pyramid","Tetrahedron","Octahedron",
            "Dodecahedron","Icosahedron","Torus","Back"
        };
        int s = DM.menu("Geometry (27)", items, 28);
        if (s < 0 || s == 27) return;
        switch (s) {
            case 0: triangleSSS();    break;
            case 1: triangleBH();     break;
            case 2: triangleSAS();    break;
            case 3: rectangle();      break;
            case 4: square();         break;
            case 5: parallelogram();  break;
            case 6: trapezoid();      break;
            case 7: rhombus();        break;
            case 8: circle();         break;
            case 9: annulus();        break;
            case 10: ellipse();       break;
            case 11: sector();        break;
            case 12: polygon();       break;
            case 13: regularPoly();   break;
            case 14: cube();          break;
            case 15: rectPrism();     break;
            case 16: sphere();        break;
            case 17: hemisphere();    break;
            case 18: cylinder();      break;
            case 19: cone();          break;
            case 20: frustum();       break;
            case 21: pyramid();       break;
            case 22: tetrahedron();   break;
            case 23: octahedron();    break;
            case 24: dodecahedron();  break;
            case 25: icosahedron();   break;
            case 26: torus();         break;
        }
    }
private:
    double askD(const char* p, const char* def) {
        char b[24]; strncpy(b, def, sizeof(b)); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }
    void show2(const char* title, const char* l1, double v1, const char* l2, double v2) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("Y:Ans","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char vs[24]; char l[40];
        formatNum(v1, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l1, vs);
        d.drawString(l, 4, HDR_H + 8);
        formatNum(v2, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l2, vs);
        d.drawString(l, 4, HDR_H + 24);
        G.ans = v1; G.ansComplex = false;
        DM.waitEsc();
    }
    void show3(const char* t, const char* l1, double v1, const char* l2, double v2,
                              const char* l3, double v3) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char vs[24]; char l[40];
        formatNum(v1, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 4);
        formatNum(v2, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l2, vs); d.drawString(l, 4, HDR_H + 18);
        formatNum(v3, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l3, vs); d.drawString(l, 4, HDR_H + 32);
        G.ans = v1; G.ansComplex = false;
        DM.waitEsc();
    }
    void triangleSSS() { double a=askD("a:","3"),b=askD("b:","4"),c=askD("c:","5"); double s=(a+b+c)/2;
        show2("Triangle SSS","Area",sqrt(s*(s-a)*(s-b)*(s-c)),"Perim",a+b+c); }
    void triangleBH() { double b=askD("base:","10"),h=askD("height:","5"); show2("Triangle BH","Area",0.5*b*h,"(b*h/2)",0); }
    void triangleSAS() { double a=askD("a:","5"),b=askD("b:","7"),C=askD("angle C:","60");
        show2("Triangle SAS","Area",0.5*a*b*sin(toRad(C)),"(0.5ab sinC)",0); }
    void rectangle() { double w=askD("w:","4"),h=askD("h:","3"); show2("Rectangle","Area",w*h,"Perim",2*(w+h)); }
    void square() { double a=askD("side:","5"); show3("Square","Area",a*a,"Perim",4*a,"Diag",a*sqrt(2)); }
    void parallelogram() { double b=askD("base:","6"),h=askD("h:","4"); show2("Parallelogram","Area",b*h,"(b*h)",0); }
    void trapezoid() { double a=askD("a (top):","3"),b=askD("b (bot):","7"),h=askD("h:","4");
        show2("Trapezoid","Area",0.5*(a+b)*h,"(a+b)/2*h",0); }
    void rhombus() { double d1=askD("d1:","6"),d2=askD("d2:","8"); show2("Rhombus","Area",0.5*d1*d2,"(d1*d2/2)",0); }
    void circle() { double r=askD("r:","5"); show3("Circle","Area",M_PI*r*r,"Circ",2*M_PI*r,"Diam",2*r); }
    void annulus() { double R=askD("R:","5"),r=askD("r:","3"); show2("Annulus","Area",M_PI*(R*R-r*r),"(R^2-r^2)pi",0); }
    void ellipse() { double a=askD("semi a:","5"),b=askD("semi b:","3");
        double h=(a-b)*(a-b)/((a+b)*(a+b)); double per=M_PI*(a+b)*(1+3*h/(10+sqrt(4-3*h)));
        show2("Ellipse","Area",M_PI*a*b,"Perim(approx)",per); }
    void sector() { double r=askD("r:","5"),th=askD("angle:","60"); double rd=toRad(th);
        show3("Sector","Area",0.5*r*r*rd,"Arc",r*rd,"Chord",2*r*sin(rd/2)); }
    void polygon() {
        char b[8]="5"; if (!DM.inputLine("# of vertices:", b, sizeof(b))) return;
        int n=atoi(b); if (n<3||n>20){DM.notify("3..20",C_ERR);return;}
        double X[20],Y[20];
        for (int i=0;i<n;i++){
            char p[12]; snprintf(p,sizeof(p),"x%d:",i+1); char v[16]="0";
            if (!DM.inputLine(p,v,sizeof(v))) return; X[i]=atof(v);
            snprintf(p,sizeof(p),"y%d:",i+1); strcpy(v,"0");
            if (!DM.inputLine(p,v,sizeof(v))) return; Y[i]=atof(v);
        }
        double A=0,P=0;
        for (int i=0;i<n;i++){int j=(i+1)%n; A+=X[i]*Y[j]-X[j]*Y[i]; P+=hypot(X[j]-X[i],Y[j]-Y[i]);}
        show2("Polygon","Area",fabs(A)/2,"Perim",P);
    }
    void regularPoly() { double n=askD("n sides:","6"),r=askD("apothem:","3"); double s=2*r*tan(M_PI/n);
        show3("Regular polygon","Area",0.5*n*s*r,"Side",s,"Perim",n*s); }
    void cube() { double a=askD("side:","3"); show3("Cube","Vol",a*a*a,"Surf",6*a*a,"Diag",a*sqrt(3)); }
    void rectPrism() { double a=askD("a:","2"),b=askD("b:","3"),c=askD("c:","4");
        show3("Rect Prism","Vol",a*b*c,"Surf",2*(a*b+b*c+a*c),"Diag",sqrt(a*a+b*b+c*c)); }
    void sphere() { double r=askD("r:","5"); show2("Sphere","Vol",4.0/3*M_PI*r*r*r,"Surf",4*M_PI*r*r); }
    void hemisphere() { double r=askD("r:","5"); show2("Hemisphere","Vol",2.0/3*M_PI*r*r*r,"Surf",3*M_PI*r*r); }
    void cylinder() { double r=askD("r:","3"),h=askD("h:","5"); show2("Cylinder","Vol",M_PI*r*r*h,"Surf",2*M_PI*r*(r+h)); }
    void cone() { double r=askD("r:","3"),h=askD("h:","5"); double s=sqrt(r*r+h*h);
        show3("Cone","Vol",M_PI*r*r*h/3,"Surf",M_PI*r*(r+s),"Slant",s); }
    void frustum() { double R=askD("R:","5"),r=askD("r:","3"),h=askD("h:","4"); double s=sqrt((R-r)*(R-r)+h*h);
        show3("Frustum","Vol",M_PI*h/3*(R*R+r*r+R*r),"Surf",M_PI*((R+r)*s+R*R+r*r),"Slant",s); }
    void pyramid() { double a=askD("base side:","3"),h=askD("h:","5"); double s=sqrt(h*h+a*a/4);
        show3("Pyramid","Vol",a*a*h/3,"Surf",a*a+2*a*s,"Slant",s); }
    void tetrahedron() { double a=askD("edge:","3");
        show2("Tetrahedron","Vol",a*a*a/(6*sqrt(2)),"Surf",sqrt(3)*a*a); }
    void octahedron() { double a=askD("edge:","3");
        show2("Octahedron","Vol",sqrt(2)/3*a*a*a,"Surf",2*sqrt(3)*a*a); }
    void dodecahedron() { double a=askD("edge:","3");
        show2("Dodecahedron","Vol",(15+7*sqrt(5))/4*a*a*a,"Surf",3*sqrt(25+10*sqrt(5))*a*a); }
    void icosahedron() { double a=askD("edge:","3");
        show2("Icosahedron","Vol",5*(3+sqrt(5))/12*a*a*a,"Surf",5*sqrt(3)*a*a); }
    void torus() { double R=askD("R (major):","5"),r=askD("r (minor):","2");
        show2("Torus","Vol",2*M_PI*M_PI*R*r*r,"Surf",4*M_PI*M_PI*R*r); }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 12 / 20 ─ 確率シミュレーション / 素因数分解 / 拡張ツール統合    ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class ProbSim {
public:
    void run() {
        const char* items[] = {
            "Coin flip","Dice roll","Gacha (n out of m)","Random integer",
            "Random real","Normal random","Exponential","Birthday paradox","Back"
        };
        int s = DM.menu("Probability Sim", items, 9);
        if (s < 0 || s == 8) return;
        switch (s) {
            case 0: coin();      break;
            case 1: dice();      break;
            case 2: gacha();     break;
            case 3: randInt();   break;
            case 4: randReal();  break;
            case 5: randNorm();  break;
            case 6: randExp();   break;
            case 7: birthday();  break;
        }
    }
private:
    double rnd() { return ((double)esp_random() / 4294967296.0); }
    double rndN(double mu = 0, double sd = 1) {
        double u1 = (esp_random() + 1.0) / 4294967297.0;
        double u2 = (esp_random() + 1.0) / 4294967297.0;
        return mu + sd * sqrt(-2*log(u1)) * cos(2*M_PI*u2);
    }
    void coin() {
        char b[8]="1000"; if (!DM.inputLine("trials:",b,sizeof(b))) return;
        int n=atoi(b); int h=0;
        for (int i=0;i<n;i++) if (rnd()<0.5) h++;
        if (G.statCapacity>0 && G.stat[0].data){ G.stat[0].data[0]=h; G.stat[0].data[1]=n-h; G.stat[0].count=2; }
        auto& d=M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Coin Flip"); DM.footer("","","ESC:back");
        char l[40];
        snprintf(l,sizeof(l),"Trials: %d",n);  d.setTextColor(C_FG,C_BG);  d.drawString(l,4,HDR_H+8);
        snprintf(l,sizeof(l),"Heads:  %d",h);   d.setTextColor(C_OK,C_BG);  d.drawString(l,4,HDR_H+22);
        snprintf(l,sizeof(l),"Tails:  %d",n-h); d.setTextColor(C_ACC2,C_BG);d.drawString(l,4,HDR_H+36);
        snprintf(l,sizeof(l),"Ratio:  %.4f",(double)h/n);
        d.setTextColor(C_DIM,C_BG); d.drawString(l,4,HDR_H+52);
        int barH=60; int hx=(int)((double)h/n*barH);
        d.fillRect(120,HDR_H+22,hx,8,C_OK);
        d.fillRect(120,HDR_H+36,barH-hx,8,C_ACC2);
        DM.waitEsc();
    }
    void dice() {
        char b[8]="1000"; if (!DM.inputLine("trials:",b,sizeof(b))) return;
        char fb[8]="6";   if (!DM.inputLine("faces:",fb,sizeof(fb))) return;
        int n=atoi(b), faces=atoi(fb);
        if (faces<2||faces>100){DM.notify("2..100",C_ERR);return;}
        int* hist=(int*)calloc(faces,sizeof(int));
        for (int i=0;i<n;i++) hist[(int)(rnd()*faces)]++;
        if (G.statCapacity>=faces){ for (int i=0;i<faces;i++) G.stat[0].data[i]=hist[i]; G.stat[0].count=faces; }
        auto& d=M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Dice Roll"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG,C_BG);
        char l[40]; snprintf(l,sizeof(l),"n=%d  faces=%d",n,faces);
        d.drawString(l,4,HDR_H+2);
        int barW=(LCD_W-8)/faces; if (barW<4) barW=4;
        int maxV=0; for (int i=0;i<faces;i++) if (hist[i]>maxV) maxV=hist[i];
        for (int i=0;i<faces && 4+i*barW<LCD_W-4;i++){
            int h=(int)((double)hist[i]/maxV*70);
            d.fillRect(4+i*barW,LCD_H-FTR_H-4-h,barW-1,h,C_ACC);
        }
        free(hist); DM.waitEsc();
    }
    void gacha() {
        char b[16]="0.05"; if (!DM.inputLine("p (rare):",b,sizeof(b))) return;
        double p=atof(b);
        char tb[16]="100"; if (!DM.inputLine("trials:",tb,sizeof(tb))) return;
        int n=atoi(tb);
        int total_pulls=0,hit=0;
        for (int t=0;t<n;t++){ int pulls=0; while (true){ pulls++; if (rnd()<p){ total_pulls+=pulls; hit++; break;} if (pulls>100000) break; } }
        double avg=(double)total_pulls/hit;
        auto& d=M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Gacha"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG,C_BG);
        char l[40];
        snprintf(l,sizeof(l),"p = %g",p);                    d.drawString(l,4,HDR_H+4);
        snprintf(l,sizeof(l),"trials = %d",n);                d.drawString(l,4,HDR_H+18);
        snprintf(l,sizeof(l),"Avg pulls (sim) = %.2f",avg);   d.setTextColor(C_OK,C_BG);  d.drawString(l,4,HDR_H+34);
        snprintf(l,sizeof(l),"Theory 1/p = %.2f",1.0/p);      d.setTextColor(C_DIM,C_BG); d.drawString(l,4,HDR_H+48);
        DM.waitEsc();
    }
    void randInt() {
        char a[16]="1",c[16]="100";
        if (!DM.inputLine("min:",a,sizeof(a))) return;
        if (!DM.inputLine("max:",c,sizeof(c))) return;
        long lo=atol(a),hi=atol(c);
        long v=lo+(long)(rnd()*(hi-lo+1));
        char vs[24]; snprintf(vs,sizeof(vs),"%ld",v);
        DM.notify(vs,C_OK,1500);
        G.ans=(double)v; G.ansComplex=false;
    }
    void randReal() {
        char a[16]="0",c[16]="1";
        if (!DM.inputLine("min:",a,sizeof(a))) return;
        if (!DM.inputLine("max:",c,sizeof(c))) return;
        double v=atof(a)+rnd()*(atof(c)-atof(a));
        char vs[24]; formatNum(v,vs,sizeof(vs));
        DM.notify(vs,C_OK,1500);
        G.ans=v; G.ansComplex=false;
    }
    void randNorm() {
        char m[16]="0",s[16]="1";
        if (!DM.inputLine("mu:",m,sizeof(m))) return;
        if (!DM.inputLine("sigma:",s,sizeof(s))) return;
        double v=rndN(atof(m),atof(s));
        char vs[24]; formatNum(v,vs,sizeof(vs));
        DM.notify(vs,C_OK,1500);
        G.ans=v; G.ansComplex=false;
    }
    void randExp() {
        char b[16]="1"; if (!DM.inputLine("lambda:",b,sizeof(b))) return;
        double l=atof(b);
        double v=-log(1-rnd())/l;
        char vs[24]; formatNum(v,vs,sizeof(vs));
        DM.notify(vs,C_OK,1500);
        G.ans=v; G.ansComplex=false;
    }
    void birthday() {
        char b[8]="23"; if (!DM.inputLine("# people:",b,sizeof(b))) return;
        int n=atoi(b);
        char tb[8]="10000"; if (!DM.inputLine("trials:",tb,sizeof(tb))) return;
        int trials=atoi(tb);
        int collisions=0; bool used[366];
        for (int t=0;t<trials;t++){ for (int i=0;i<366;i++) used[i]=false; bool col=false;
            for (int i=0;i<n;i++){ int day=(int)(rnd()*365); if (used[day]){col=true;break;} used[day]=true; }
            if (col) collisions++;
        }
        double sim=(double)collisions/trials;
        double th=1.0; for (int i=0;i<n;i++) th*=(double)(365-i)/365.0; th=1.0-th;
        auto& d=M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Birthday Paradox"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG,C_BG);
        char l[40];
        snprintf(l,sizeof(l),"n=%d trials=%d",n,trials); d.drawString(l,4,HDR_H+4);
        snprintf(l,sizeof(l),"Sim Prob = %.5f",sim);      d.setTextColor(C_OK,C_BG); d.drawString(l,4,HDR_H+22);
        snprintf(l,sizeof(l),"Theory   = %.5f",th);       d.setTextColor(C_ACC,C_BG); d.drawString(l,4,HDR_H+38);
        DM.waitEsc();
    }
};

class PrimeFactor {
public:
    void run() {
        char b[24]="360"; if (!DM.inputLine("Number:",b,sizeof(b))) return;
        long long n=(long long)atof(b);
        if (n<2){DM.notify("Need n>=2",C_ERR);return;}
        long long N=(n<0)?-n:n;
        long long factors[64]; int counts[64]; int nf=0;
        if (N%2==0){ factors[nf]=2; counts[nf]=0; while(N%2==0){N/=2;counts[nf]++;} nf++; }
        for (long long p=3;p*p<=N && nf<64;p+=2){ if (N%p==0){ factors[nf]=p;counts[nf]=0;
            while(N%p==0){N/=p;counts[nf]++;} nf++; } }
        if (N>1 && nf<64){ factors[nf]=N; counts[nf]=1; nf++; }
        auto& d=M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Prime Factorize"); DM.footer("","","ESC:back");
        char title[40]; snprintf(title,sizeof(title),"%lld =",(long long)n);
        d.setTextColor(C_FG,C_BG); d.drawString(title,4,HDR_H+4);
        char buf[120]; buf[0]=0;
        for (int i=0;i<nf;i++){
            char p[24];
            if (counts[i]==1) snprintf(p,sizeof(p),"%lld",factors[i]);
            else              snprintf(p,sizeof(p),"%lld^%d",factors[i],counts[i]);
            if (i>0) strncat(buf," x ",sizeof(buf)-strlen(buf)-1);
            strncat(buf,p,sizeof(buf)-strlen(buf)-1);
        }
        d.setTextColor(C_OK,C_BG);
        DM.drawWrapped(buf,4,HDR_H+22,LCD_W-8,C_OK);
        DM.waitEsc();
    }
};

EqSolver     gEqSolver;
NumMenu      gNumMenu;
VecMatCalc   gVecMat;
GeomCalc     gGeom;
ProbSim      gProb;
PrimeFactor  gPrime;

void CalculatorCore::extToolMenu(char* input, int& cur) {
    while (true) {
        const char* items[] = {
            "Equation Solver","Numerical Analysis (d/dx, integ)","Vector / Matrix",
            "Geometry (27 shapes)","Probability Sim","Prime Factorize","Back"
        };
        int s = DM.menu("Tools", items, 7);
        if (s < 0 || s == 6) return;
        switch (s) {
            case 0: gEqSolver.run(); break;
            case 1: gNumMenu.run();  break;
            case 2: gVecMat.run();   break;
            case 3: gGeom.run();     break;
            case 4: gProb.run();     break;
            case 5: gPrime.run();    break;
        }
    }
}

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 13 / 20 ─ Stat モード コア (記述統計・CSV I/O)                  ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class StatCore {
public:
    void run() {
        if (G.statCapacity == 0) {
            if (!G.allocStat(LIST_MAX)) { DM.notify("PSRAM alloc failed", C_ERR, 1500); return; }
        }
        while (true) {
            const char* items[] = {
                "List Editor","Variable Scale","Descriptive Statistics",
                "Weighted/Geo/Harm/Trim Mean","Frequency Distribution","Z-Scores",
                "CSV Load (SD)","CSV Save (SD)","Clear All Lists","List -> next module","Back"
            };
            int s = DM.menu("Statistics", items, 11);
            if (s < 0 || s == 10) return;
            switch (s) {
                case 0: editList();        break;
                case 1: scaleMenu();       break;
                case 2: descriptive();     break;
                case 3: specialMeans();    break;
                case 4: freqDist();        break;
                case 5: zScores();         break;
                case 6: csvLoad();         break;
                case 7: csvSave();         break;
                case 8: clearAll();        break;
                case 9: passToNextHint();  break;
            }
        }
    }
private:
    int pickList(const char* title) {
        char** labels = (char**)malloc(STAT_LISTS * sizeof(char*));
        for (int i = 0; i < STAT_LISTS; i++) {
            labels[i] = (char*)malloc(20);
            snprintf(labels[i], 20, "%s (n=%d)", G.stat[i].name, G.stat[i].count);
        }
        const char** clabels = (const char**)labels;
        int s = DM.menu(title, clabels, STAT_LISTS);
        for (int i = 0; i < STAT_LISTS; i++) free(labels[i]);
        free(labels);
        return s;
    }
    void editList() {
        int li = pickList("Edit which list?");
        if (li < 0) return;
        editListIdx(li);
    }
    void editListIdx(int li) {
        auto& d = M5Cardputer.Display;
        int sel = 0, top = 0;
        const int rowH = 11;
        const int viewH = LCD_H - HDR_H - FTR_H;
        const int rows = viewH / rowH;
        char buf[24];
        while (true) {
            int n = G.stat[li].count;
            if (sel < top) top = sel;
            if (sel >= top + rows) top = sel - rows + 1;
            d.fillScreen(C_BG);
            char th[24]; snprintf(th, sizeof(th), "%s (n=%d)", G.stat[li].name, n);
            DM.header(th);
            DM.footer("ENT:edit DEL:rm", "S:Sort A:add", "ESC:back");
            for (int i = 0; i < rows; i++) {
                int idx = top + i;
                if (idx > n) break;
                int y = HDR_H + i * rowH;
                bool isSel = (idx == sel);
                if (isSel) { d.fillRect(0, y, LCD_W - 4, rowH, C_SEL_BG); d.fillRect(0, y, 3, rowH, C_SEL_BAR); }
                d.setTextColor(isSel ? C_FG : C_DIM, isSel ? C_SEL_BG : C_BG);
                char line[40];
                if (idx < n) {
                    char vs[20]; formatNum(G.stat[li].data[idx], vs, sizeof(vs));
                    snprintf(line, sizeof(line), "%4d: %s", idx+1, vs);
                } else {
                    snprintf(line, sizeof(line), "%4d: <new>", idx+1);
                }
                d.drawString(line, 6, y + 1);
            }
            if (n + 1 > rows) {
                int sbx = LCD_W - 4;
                d.fillRect(sbx, HDR_H, 4, viewH, C_PANEL);
                int total = n + 1;
                int thumb = (rows * viewH) / total;
                if (thumb < 8) thumb = 8;
                int ty = HDR_H + (top * (viewH - thumb)) / (total - rows);
                d.fillRect(sbx, ty, 4, thumb, C_ACC);
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.fn && IS_UP)   { if (sel > 0) sel--; break; }
                if (st.fn && IS_DOWN) { if (sel <= n) sel++; break; }
                if (st.enter) {
                    snprintf(buf, sizeof(buf), "%g", (sel < n) ? G.stat[li].data[sel] : 0.0);
                    if (DM.inputLine("Value:", buf, sizeof(buf))) {
                        char* end; double v = strtod(buf, &end);
                        if (end != buf) {
                            if (sel < n) G.stat[li].data[sel] = v;
                            else if (n < G.statCapacity) { G.stat[li].data[n] = v; G.stat[li].count = n + 1; }
                        }
                    }
                    break;
                }
                if (st.del) {
                    if (sel < n) {
                        for (int i = sel; i < n-1; i++) G.stat[li].data[i] = G.stat[li].data[i+1];
                        G.stat[li].count = n - 1;
                        if (sel >= G.stat[li].count && sel > 0) sel--;
                    }
                    break;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) {
                    sortList(li); DM.notify("Sorted", C_OK); break;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('a') || M5Cardputer.Keyboard.isKeyPressed('A')) {
                    char in[40] = "";
                    if (DM.inputLine("Add (comma OK):", in, sizeof(in))) {
                        char* tok = strtok(in, ", \t");
                        while (tok && G.stat[li].count < G.statCapacity) {
                            char* e; double v = strtod(tok, &e);
                            if (e != tok) G.stat[li].data[G.stat[li].count++] = v;
                            tok = strtok(nullptr, ", \t");
                        }
                    }
                    break;
                }
            }
        }
    }
    void sortList(int li) {
        int n = G.stat[li].count;
        double* a = G.stat[li].data;
        for (int i = 1; i < n; i++) { double k = a[i]; int j = i - 1;
            while (j >= 0 && a[j] > k) { a[j+1] = a[j]; j--; } a[j+1] = k; }
    }
    void scaleMenu() {
        int li = pickList("Scale: which list?"); if (li < 0) return;
        const char* items[] = {"Ratio (比例)","Interval (間隔)","Ordinal (順序)","Nominal (名義)","Back"};
        int s = DM.menu("Variable Scale", items, 5, G.stat[li].scale);
        if (s < 0 || s == 4) return;
        G.stat[li].scale = (VarScale)s;
        DM.notify("Set", C_OK);
    }
    void descriptive() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 1) { DM.notify("Empty", C_ERR); return; }
        double* x = G.stat[li].data;
        KSum ksum, ksq;
        double mn = x[0], mx = x[0];
        for (int i = 0; i < n; i++) {
            ksum.add(x[i]); ksq.add(x[i]*x[i]);
            if (x[i] < mn) mn = x[i];
            if (x[i] > mx) mx = x[i];
        }
        double sum = ksum.get(), sum2 = ksq.get();
        double mean = sum / n;
        KSum kss, km3, km4;
        for (int i = 0; i < n; i++) {
            double d = x[i] - mean;
            kss.add(d*d); km3.add(d*d*d); km4.add(d*d*d*d);
        }
        double SS = kss.get();
        double varS = (n > 1) ? SS / (n-1) : 0;
        double varP = SS / n;
        double sdS = sqrt(varS), sdP = sqrt(varP);
        double SE = sdS / sqrt((double)n);
        double CV = (mean != 0) ? sdS / fabs(mean) * 100 : 0;
        double m3 = km3.get() / n, m4 = km4.get() / n;
        double skew = (sdP > 0) ? m3 / (sdP*sdP*sdP) : 0;
        double kurt = (sdP > 0) ? m4 / (sdP*sdP*sdP*sdP) - 3.0 : 0;
        double* tmp = (double*)malloc(n * sizeof(double));
        if (!tmp) { DM.notify("malloc fail", C_ERR); return; }
        memcpy(tmp, x, n * sizeof(double));
        for (int i = 1; i < n; i++) { double k = tmp[i]; int j = i - 1;
            while (j >= 0 && tmp[j] > k) { tmp[j+1] = tmp[j]; j--; } tmp[j+1] = k; }
        auto pct = [&](double p) -> double {
            if (n == 1) return tmp[0];
            double idx = p * (n - 1);
            int lo = (int)floor(idx);
            double f = idx - lo;
            return tmp[lo] + f * (tmp[(lo+1<n)?lo+1:lo] - tmp[lo]);
        };
        double q1 = pct(0.25), med = pct(0.5), q3 = pct(0.75);
        double iqr = q3 - q1;
        double mode = tmp[0]; int modeCount = 1, curCount = 1;
        for (int i = 1; i < n; i++) {
            if (tmp[i] == tmp[i-1]) { curCount++; if (curCount > modeCount) { modeCount = curCount; mode = tmp[i]; } }
            else curCount = 1;
        }
        free(tmp);
        showStatPage(li, n, sum, sum2, mean, varS, sdS, varP, sdP, SE, CV,
                     mn, mx, q1, med, q3, iqr, skew, kurt, mode, modeCount);
    }
    void showStatPage(int li, int n, double sum, double sum2, double mean,
                      double varS, double sdS, double varP, double sdP,
                      double SE, double CV, double mn, double mx,
                      double q1, double med, double q3, double iqr,
                      double skew, double kurt, double mode, int modeCount) {
        int page = 0;
        auto& d = M5Cardputer.Display;
        while (true) {
            d.fillScreen(C_BG);
            char th[24]; snprintf(th, sizeof(th), "Stat %s (p%d/2)", G.stat[li].name, page+1);
            DM.header(th);
            DM.footer("L/R:page", "Y:mean->Ans", "ESC:back");
            d.setTextColor(C_FG, C_BG);
            char l[60];
            int y = HDR_H + 2;
            if (page == 0) {
                snprintf(l,sizeof(l),"n = %d", n);                    d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"sum  = %.6g", sum);             d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"sum2 = %.6g", sum2);            d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"mean = %.6g", mean);            d.setTextColor(C_OK,C_BG); d.drawString(l,4,y); y+=11; d.setTextColor(C_FG,C_BG);
                snprintf(l,sizeof(l),"sd  (samp) = %.6g", sdS);       d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"sd  (pop)  = %.6g", sdP);       d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"var (samp) = %.5g", varS);      d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"SE  = %.5g  CV = %.3f%%", SE, CV); d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"min = %.5g  max = %.5g", mn, mx); d.drawString(l,4,y); y+=11;
            } else {
                snprintf(l,sizeof(l),"Q1  = %.5g", q1);               d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"med = %.5g", med);              d.setTextColor(C_OK,C_BG); d.drawString(l,4,y); y+=11; d.setTextColor(C_FG,C_BG);
                snprintf(l,sizeof(l),"Q3  = %.5g", q3);               d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"IQR = %.5g  range=%.5g", iqr, mx-mn); d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"skew = %.4g", skew);            d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"kurt = %.4g (excess)", kurt);   d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"mode = %.5g (x%d)", mode, modeCount); d.drawString(l,4,y); y+=11;
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 1) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                    G.ans = mean; G.ansComplex = false; DM.notify("mean -> Ans", C_OK); return;
                }
            }
        }
    }
    void specialMeans() {
        const char* items[] = {
            "Weighted mean (L1=val, L2=weight)","Geometric mean (L?)","Harmonic mean (L?)",
            "Trimmed mean (L?, alpha)","Back"
        };
        int s = DM.menu("Special Means", items, 5);
        if (s < 0 || s == 4) return;
        if (s == 0) {
            int n = G.stat[0].count;
            if (n < 1 || G.stat[1].count != n) { DM.notify("L1,L2 size mismatch", C_ERR); return; }
            double sw = 0, sxw = 0;
            for (int i = 0; i < n; i++) { sw += G.stat[1].data[i]; sxw += G.stat[0].data[i] * G.stat[1].data[i]; }
            if (sw == 0) { DM.notify("Sum weights = 0", C_ERR); return; }
            showOne("Weighted mean", sxw / sw);
        } else if (s == 1) {
            int li = pickList("Geo mean: list?"); if (li < 0) return;
            int n = G.stat[li].count;
            for (int i = 0; i < n; i++) if (G.stat[li].data[i] <= 0) { DM.notify("All > 0 required", C_ERR); return; }
            double sl = 0;
            for (int i = 0; i < n; i++) sl += log(G.stat[li].data[i]);
            showOne("Geometric mean", exp(sl / n));
        } else if (s == 2) {
            int li = pickList("Harm mean: list?"); if (li < 0) return;
            int n = G.stat[li].count;
            for (int i = 0; i < n; i++) if (G.stat[li].data[i] == 0) { DM.notify("Non-zero only", C_ERR); return; }
            double s2 = 0;
            for (int i = 0; i < n; i++) s2 += 1.0 / G.stat[li].data[i];
            showOne("Harmonic mean", n / s2);
        } else if (s == 3) {
            int li = pickList("Trim mean: list?"); if (li < 0) return;
            char b[16] = "0.1";
            if (!DM.inputLine("alpha (0..0.5):", b, sizeof(b))) return;
            double a = atof(b);
            if (a < 0 || a >= 0.5) { DM.notify("0..0.5", C_ERR); return; }
            int n = G.stat[li].count;
            double* tmp = (double*)malloc(n * sizeof(double));
            memcpy(tmp, G.stat[li].data, n * sizeof(double));
            for (int i = 1; i < n; i++) { double k = tmp[i]; int j = i-1;
                while (j >= 0 && tmp[j] > k) { tmp[j+1] = tmp[j]; j--; } tmp[j+1] = k; }
            int trim = (int)(n * a);
            double s = 0;
            for (int i = trim; i < n - trim; i++) s += tmp[i];
            free(tmp);
            int eff = n - 2*trim;
            showOne("Trimmed mean", eff > 0 ? s / eff : 0);
        }
    }
    void showOne(const char* nm, double v) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(nm); DM.footer("Y:Ans","","ESC:back");
        char vs[40]; formatNum(v, vs, sizeof(vs));
        d.setTextSize(2); d.setTextColor(C_OK, C_BG);
        d.drawString(vs, 4, HDR_H + 14);
        d.setTextSize(1);
        G.ans = v; G.ansComplex = false;
        DM.waitEsc();
    }
    void freqDist() {
        int li = pickList("Freq dist: list?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 1) { DM.notify("Empty", C_ERR); return; }
        char bb[8] = "10";
        if (!DM.inputLine("# bins:", bb, sizeof(bb))) return;
        int nb = atoi(bb);
        if (nb < 2 || nb > 50) { DM.notify("2..50", C_ERR); return; }
        double mn = G.stat[li].data[0], mx = mn;
        for (int i = 0; i < n; i++) {
            if (G.stat[li].data[i] < mn) mn = G.stat[li].data[i];
            if (G.stat[li].data[i] > mx) mx = G.stat[li].data[i];
        }
        double w = (mx - mn) / nb;
        if (w == 0) w = 1;
        int* hist = (int*)calloc(nb, sizeof(int));
        for (int i = 0; i < n; i++) {
            int b = (int)((G.stat[li].data[i] - mn) / w);
            if (b == nb) b = nb - 1;
            if (b < 0)   b = 0;
            hist[b]++;
        }
        int maxV = 0; for (int i = 0; i < nb; i++) if (hist[i] > maxV) maxV = hist[i];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Freq Dist"); DM.footer("S:save BMP","","ESC:back");
        d.setTextColor(C_DIM, C_BG);
        char info[40]; snprintf(info, sizeof(info), "n=%d bins=%d w=%.3g", n, nb, w);
        d.drawString(info, 4, HDR_H);
        int bw = (LCD_W - 8) / nb;
        if (bw < 2) bw = 2;
        int gy0 = HDR_H + 14;
        int gh  = LCD_H - HDR_H - FTR_H - 18;
        for (int i = 0; i < nb; i++) {
            int h = (maxV > 0) ? (int)((double)hist[i] / maxV * gh) : 0;
            d.fillRect(4 + i*bw, gy0 + gh - h, bw - 1, h, C_ACC);
        }
        d.drawLine(4, gy0 + gh, 4 + nb*bw, gy0 + gh, C_DIM);
        free(hist);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("hist");
        }
    }
    void zScores() {
        int li = pickList("Source list?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 2) { DM.notify("n>=2 required", C_ERR); return; }
        double sum = 0, sum2 = 0;
        for (int i = 0; i < n; i++) { sum += G.stat[li].data[i]; sum2 += G.stat[li].data[i]*G.stat[li].data[i]; }
        double m = sum / n;
        double sd = sqrt((sum2 - n*m*m)/(n-1));
        if (sd == 0) { DM.notify("sd = 0", C_ERR); return; }
        int dst = pickList("Destination?");
        if (dst < 0) return;
        if (dst == li) { DM.notify("Same list", C_ERR); return; }
        for (int i = 0; i < n; i++) G.stat[dst].data[i] = (G.stat[li].data[i] - m) / sd;
        G.stat[dst].count = n;
        DM.notify("Z-scores saved", C_OK);
    }
    void csvLoad() {
        if (!G.sdOK) { DM.notify("No SD card", C_ERR); return; }
        char path[48] = "/data.csv";
        if (!DM.inputLine("Path:", path, sizeof(path))) return;
        SdGuard guard;
        File f = SD.open(path);
        if (!f) { DM.notify("File not found", C_ERR); return; }
        for (int i = 0; i < STAT_LISTS; i++) G.stat[i].count = 0;
        bool firstLine = true;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            if (line.length() == 0) continue;
            if (firstLine) {
                firstLine = false;
                bool hasAlpha = false;
                for (size_t k = 0; k < line.length(); k++)
                    if (isalpha(line[k])) { hasAlpha = true; break; }
                if (hasAlpha) continue;
            }
            int col = 0, from = 0;
            while (col < STAT_LISTS) {
                int comma = line.indexOf(',', from);
                String tok = (comma < 0) ? line.substring(from) : line.substring(from, comma);
                tok.trim();
                if (tok.length() > 0 && G.stat[col].count < G.statCapacity) {
                    G.stat[col].data[G.stat[col].count++] = tok.toDouble();
                }
                if (comma < 0) break;
                from = comma + 1; col++;
            }
        }
        f.close();
        DM.notify("Loaded", C_OK);
    }
    void csvSave() {
        if (!G.sdOK) { DM.notify("No SD card", C_ERR); return; }
        char path[48] = "/stat.csv";
        if (!DM.inputLine("Path:", path, sizeof(path))) return;
        SdGuard guard;
        File f = SD.open(path, FILE_WRITE);
        if (!f) { DM.notify("Open failed", C_ERR); return; }
        for (int i = 0; i < STAT_LISTS; i++) {
            f.print(G.stat[i].name);
            if (i < STAT_LISTS-1) f.print(',');
        }
        f.println();
        int maxN = 0;
        for (int i = 0; i < STAT_LISTS; i++) if (G.stat[i].count > maxN) maxN = G.stat[i].count;
        char b[24];
        for (int r = 0; r < maxN; r++) {
            for (int c = 0; c < STAT_LISTS; c++) {
                if (r < G.stat[c].count) { snprintf(b, sizeof(b), "%g", G.stat[c].data[r]); f.print(b); }
                if (c < STAT_LISTS-1) f.print(',');
            }
            f.println();
        }
        f.close();
        DM.notify("Saved", C_OK);
    }
    void clearAll() {
        if (DM.confirm("Clear all 6 lists?")) {
            for (int i = 0; i < STAT_LISTS; i++) G.stat[i].count = 0;
            DM.notify("Cleared", C_OK);
        }
    }
    void passToNextHint() { DM.notify("Use Regression / Tests menu", C_ACC, 1500); }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 14 / 20 ─ 回帰 / 検定統計                                       ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class StatRegress {
public:
    void run() {
        const char* items[] = {
            "Linear (LSM)","Quadratic","Power","Exponential","Logarithmic",
            "Logistic (2-par logit)","Med-Med","Multiple Regression",
            "Stepwise Selection","Predict y from x","Save residuals to L?","Back"
        };
        int s = DM.menu("Regression", items, 12);
        if (s < 0 || s == 11) return;
        switch (s) {
            case 0: simple(0); break;
            case 1: simple(1); break;
            case 2: simple(2); break;
            case 3: simple(3); break;
            case 4: simple(4); break;
            case 5: simple(5); break;
            case 6: simple(6); break;
            case 7: multiple();   break;
            case 8: stepwise();   break;
            case 9: predict();    break;
            case 10:saveResid();  break;
        }
    }
private:
    int pickList(const char* title) {
        const char* items[] = {"L1","L2","L3","L4","L5","L6","L7","L8"};
        return DM.menu(title, items, STAT_LISTS);
    }
    int lastType = 0;
    double lastA = 0, lastB = 0, lastC = 0;
    int lastXi = -1, lastYi = -1;
    void linfit(double* x, double* y, int n, double& a, double& b, double& r2) {
        if (n < 2) { a = NAN; b = NAN; r2 = 0; return; }
        KSum kx, ky;
        for (int i = 0; i < n; i++) { kx.add(x[i]); ky.add(y[i]); }
        double mx = kx.get()/n, my = ky.get()/n;
        KSum kxx, kxy, kyy;
        for (int i = 0; i < n; i++) {
            double dx = x[i]-mx, dy = y[i]-my;
            kxx.add(dx*dx); kxy.add(dx*dy); kyy.add(dy*dy);
        }
        double Sxx = kxx.get(), Sxy = kxy.get(), Syy = kyy.get();
        if (fabs(Sxx) < 1e-300) { a = NAN; b = NAN; r2 = 0; return; }
        b  = Sxy / Sxx;
        a  = my - b*mx;
        r2 = (Syy > 0) ? (Sxy*Sxy)/(Sxx*Syy) : 0;
    }
    void simple(int type) {
        int xi = pickList("X list?"); if (xi < 0) return;
        int yi = pickList("Y list?"); if (yi < 0) return;
        int n = G.stat[xi].count;
        if (n < 2 || G.stat[yi].count != n) { DM.notify("Mismatch / n<2", C_ERR); return; }
        double* X = G.stat[xi].data;
        double* Y = G.stat[yi].data;
        char r1[40] = "", r2[40] = "", r3[40] = "", r4[40] = "", r5[40] = "";
        double aa = 0, bb = 0, cc = 0, r2v = 0;
        switch (type) {
        case 0: {
            linfit(X, Y, n, aa, bb, r2v);
            snprintf(r1,sizeof(r1),"y = a + b x");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.6g", r2v);
            snprintf(r5,sizeof(r5),"r   = %.6g", sqrt(r2v) * (bb<0?-1:1));
        } break;
        case 1: {
            double M[9] = {0}, v[3] = {0};
            double sx=0,sx2=0,sx3=0,sx4=0,sy=0,sxy=0,sx2y=0;
            for (int i = 0; i < n; i++) {
                double xi = X[i], yi = Y[i];
                sx+=xi; sx2+=xi*xi; sx3+=xi*xi*xi; sx4+=xi*xi*xi*xi;
                sy+=yi; sxy+=xi*yi; sx2y+=xi*xi*yi;
            }
            M[0]=n; M[1]=sx; M[2]=sx2;
            M[3]=sx; M[4]=sx2; M[5]=sx3;
            M[6]=sx2; M[7]=sx3; M[8]=sx4;
            v[0]=sy; v[1]=sxy; v[2]=sx2y;
            if (Mat::gaussSolve(M, 3, v)) {
                aa = v[0]; bb = v[1]; cc = v[2];
                snprintf(r1,sizeof(r1),"y = a + b x + c x^2");
                snprintf(r2,sizeof(r2),"a = %.5g", aa);
                snprintf(r3,sizeof(r3),"b = %.5g", bb);
                snprintf(r4,sizeof(r4),"c = %.5g", cc);
                double yMean = sy/n, sst = 0, sse = 0;
                for (int i = 0; i < n; i++) {
                    double yhat = aa + bb*X[i] + cc*X[i]*X[i];
                    sst += (Y[i]-yMean)*(Y[i]-yMean);
                    sse += (Y[i]-yhat)*(Y[i]-yhat);
                }
                r2v = (sst > 0) ? 1 - sse/sst : 0;
                snprintf(r5,sizeof(r5),"r^2 = %.5g", r2v);
            } else snprintf(r1,sizeof(r1),"Singular");
        } break;
        case 2: {
            for (int i = 0; i < n; i++) if (X[i] <= 0 || Y[i] <= 0) { DM.notify("X,Y > 0", C_ERR); return; }
            double *lx = (double*)malloc(n*8), *ly = (double*)malloc(n*8);
            if (!lx || !ly) { free(lx); free(ly); DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) { lx[i] = log(X[i]); ly[i] = log(Y[i]); }
            double la, lb;
            linfit(lx, ly, n, la, lb, r2v);
            free(lx); free(ly);
            aa = exp(la); bb = lb;
            snprintf(r1,sizeof(r1),"y = a * x^b");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.6g", r2v);
        } break;
        case 3: {
            for (int i = 0; i < n; i++) if (Y[i] <= 0) { DM.notify("Y > 0", C_ERR); return; }
            double *ly = (double*)malloc(n*8);
            if (!ly) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]);
            double la, lb;
            linfit(X, ly, n, la, lb, r2v);
            free(ly);
            aa = exp(la); bb = lb;
            snprintf(r1,sizeof(r1),"y = a * e^(b x)");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.6g", r2v);
        } break;
        case 4: {
            for (int i = 0; i < n; i++) if (X[i] <= 0) { DM.notify("X > 0", C_ERR); return; }
            double *lx = (double*)malloc(n*8);
            if (!lx) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) lx[i] = log(X[i]);
            linfit(lx, Y, n, aa, bb, r2v);
            free(lx);
            snprintf(r1,sizeof(r1),"y = a + b ln x");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.6g", r2v);
        } break;
        case 5: {
            for (int i = 0; i < n; i++) if (Y[i] <= 0 || Y[i] >= 1) { DM.notify("0<Y<1", C_ERR); return; }
            double *ly = (double*)malloc(n*8);
            if (!ly) { DM.notify("malloc fail", C_ERR); return; }
            for (int i = 0; i < n; i++) ly[i] = log(Y[i]/(1-Y[i]));
            linfit(X, ly, n, aa, bb, r2v);
            free(ly);
            snprintf(r1,sizeof(r1),"logit(y) = a + b x");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.6g (logit)", r2v);
        } break;
        case 6: {
            int* idx = (int*)malloc(n * sizeof(int));
            for (int i = 0; i < n; i++) idx[i] = i;
            for (int i = 1; i < n; i++) {
                int k = idx[i]; int j = i - 1;
                while (j >= 0 && X[idx[j]] > X[k]) { idx[j+1] = idx[j]; j--; }
                idx[j+1] = k;
            }
            int g1 = n/3, g3s = n - g1;
            auto med = [&](double* arr, int* ids, int from, int to) {
                int m = to - from;
                double* tmp = (double*)malloc(m*sizeof(double));
                for (int i = 0; i < m; i++) tmp[i] = arr[ids[from+i]];
                for (int i = 1; i < m; i++) { double k=tmp[i]; int j=i-1;
                    while (j>=0 && tmp[j]>k){ tmp[j+1]=tmp[j]; j--; } tmp[j+1]=k; }
                double r = (m%2) ? tmp[m/2] : 0.5*(tmp[m/2-1]+tmp[m/2]);
                free(tmp); return r;
            };
            double mx1 = med(X, idx, 0, g1);
            double my1 = med(Y, idx, 0, g1);
            double mx3 = med(X, idx, g3s, n);
            double my3 = med(Y, idx, g3s, n);
            double mx2 = med(X, idx, g1, g3s);
            double my2 = med(Y, idx, g1, g3s);
            free(idx);
            bb = (my3 - my1) / (mx3 - mx1);
            aa = ((my1 - bb*mx1) + (my2 - bb*mx2) + (my3 - bb*mx3)) / 3.0;
            double sy = 0, sst = 0, sse = 0;
            for (int i = 0; i < n; i++) sy += Y[i];
            double yMean = sy / n;
            for (int i = 0; i < n; i++) {
                double yhat = aa + bb*X[i];
                sst += (Y[i]-yMean)*(Y[i]-yMean);
                sse += (Y[i]-yhat)*(Y[i]-yhat);
            }
            r2v = (sst > 0) ? 1 - sse/sst : 0;
            snprintf(r1,sizeof(r1),"Med-Med y = a + b x");
            snprintf(r2,sizeof(r2),"a = %.6g", aa);
            snprintf(r3,sizeof(r3),"b = %.6g", bb);
            snprintf(r4,sizeof(r4),"r^2 = %.5g", r2v);
        } break;
        }
        lastType = type; lastA = aa; lastB = bb; lastC = cc; lastXi = xi; lastYi = yi;
        showRegResult(r1, r2, r3, r4, r5);
    }
    void showRegResult(const char* r1, const char* r2, const char* r3, const char* r4, const char* r5) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Regression"); DM.footer("Y:a->Ans","S:save","ESC:back");
        d.setTextColor(C_ACC, C_BG); d.drawString(r1, 4, HDR_H + 4);
        d.setTextColor(C_FG,  C_BG);
        d.drawString(r2, 4, HDR_H + 20);
        d.drawString(r3, 4, HDR_H + 32);
        d.drawString(r4, 4, HDR_H + 44);
        d.drawString(r5, 4, HDR_H + 56);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = lastA; G.ansComplex = false; DM.notify("a -> Ans", C_OK); return;
            }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("reg");
        }
    }
    void multiple() {
        char b[8] = "2";
        if (!DM.inputLine("# predictors (1-5):", b, sizeof(b))) return;
        int k = atoi(b);
        if (k < 1 || k > 5) { DM.notify("1..5", C_ERR); return; }
        int xL[5], yi;
        for (int i = 0; i < k; i++) {
            char t[24]; snprintf(t, sizeof(t), "X%d list?", i+1);
            xL[i] = pickList(t); if (xL[i] < 0) return;
        }
        yi = pickList("Y list?"); if (yi < 0) return;
        int n = G.stat[yi].count;
        if (n < k+2) { DM.notify("n must be >= k+2", C_ERR); return; }
        for (int i = 0; i < k; i++) if (G.stat[xL[i]].count != n) { DM.notify("Mismatch", C_ERR); return; }
        int p = k + 1;
        double XtX[36] = {0}, XtY[6] = {0};
        for (int s = 0; s < n; s++) {
            double row[6]; row[0] = 1.0;
            for (int j = 0; j < k; j++) row[j+1] = G.stat[xL[j]].data[s];
            for (int i = 0; i < p; i++) {
                XtY[i] += row[i] * G.stat[yi].data[s];
                for (int j = 0; j < p; j++) XtX[i*p + j] += row[i] * row[j];
            }
        }
        double Inv[36];
        if (!Mat::inv(XtX, p, Inv)) { DM.notify("Singular", C_ERR); return; }
        double beta[6] = {0};
        for (int i = 0; i < p; i++) for (int j = 0; j < p; j++) beta[i] += Inv[i*p+j] * XtY[j];
        double yMean = 0;
        for (int s = 0; s < n; s++) yMean += G.stat[yi].data[s];
        yMean /= n;
        double sst = 0, sse = 0;
        for (int s = 0; s < n; s++) {
            double yhat = beta[0];
            for (int j = 0; j < k; j++) yhat += beta[j+1] * G.stat[xL[j]].data[s];
            sst += (G.stat[yi].data[s] - yMean) * (G.stat[yi].data[s] - yMean);
            sse += (G.stat[yi].data[s] - yhat)  * (G.stat[yi].data[s] - yhat);
        }
        double r2v = (sst > 0) ? 1.0 - sse/sst : 0;
        double r2adj = 1.0 - (1.0 - r2v) * (n - 1.0) / (n - p);
        double mse = sse / (n - p);
        double SE[6], tval[6], pval[6];
        for (int i = 0; i < p; i++) {
            SE[i]   = sqrt(mse * Inv[i*p+i]);
            tval[i] = (SE[i] > 0) ? beta[i] / SE[i] : 0;
            pval[i] = 2.0 * (1.0 - Dist::tCDF(fabs(tval[i]), n - p));
        }
        double VIF[5];
        for (int i = 0; i < k; i++) {
            double yyM = 0; for (int s = 0; s < n; s++) yyM += G.stat[xL[i]].data[s];
            yyM /= n;
            double XX[36] = {0}, XY[6] = {0};
            int kk = k - 1;
            int pp = kk + 1;
            for (int s = 0; s < n; s++) {
                double rr[6]; rr[0] = 1; int t = 1;
                for (int j = 0; j < k; j++) if (j != i) rr[t++] = G.stat[xL[j]].data[s];
                for (int a = 0; a < pp; a++) {
                    XY[a] += rr[a] * G.stat[xL[i]].data[s];
                    for (int bj = 0; bj < pp; bj++) XX[a*pp+bj] += rr[a]*rr[bj];
                }
            }
            if (!Mat::gaussSolve(XX, pp, XY)) { VIF[i] = INFINITY; continue; }
            double sst2 = 0, sse2 = 0;
            for (int s = 0; s < n; s++) {
                double yh = XY[0]; int t = 1;
                for (int j = 0; j < k; j++) if (j != i) yh += XY[t++] * G.stat[xL[j]].data[s];
                sst2 += (G.stat[xL[i]].data[s] - yyM)*(G.stat[xL[i]].data[s] - yyM);
                sse2 += (G.stat[xL[i]].data[s] - yh)*(G.stat[xL[i]].data[s] - yh);
            }
            double r2_i = (sst2 > 0) ? 1 - sse2/sst2 : 0;
            VIF[i] = (1 - r2_i > 1e-12) ? 1.0 / (1 - r2_i) : INFINITY;
        }
        int page = 0;
        auto& d = M5Cardputer.Display;
        while (true) {
            d.fillScreen(C_BG);
            char th[40]; snprintf(th, sizeof(th), "MultiReg p%d/3", page+1);
            DM.header(th);
            DM.footer("L/R:page", "S:BMP", "ESC:back");
            d.setTextColor(C_FG, C_BG);
            char l[60];
            int y = HDR_H + 2;
            if (page == 0) {
                snprintf(l,sizeof(l),"n=%d k=%d", n, k);                          d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"R^2     = %.5g", r2v);    d.setTextColor(C_OK,C_BG);  d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"R^2 adj = %.5g", r2adj);                    d.drawString(l,4,y); y+=11;
                d.setTextColor(C_FG,C_BG);
                snprintf(l,sizeof(l),"SSE = %.4g  MSE = %.4g", sse, mse);         d.drawString(l,4,y); y+=11;
                double F = (r2v / k) / ((1 - r2v) / (n - p));
                double pF = 1.0 - Dist::FCDF(F, k, n - p);
                snprintf(l,sizeof(l),"F = %.4g (df %d,%d)", F, k, n-p);           d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"p(F) = %.5g", pF);                          d.drawString(l,4,y); y+=11;
            } else if (page == 1) {
                d.drawString("    coef       SE     t      p", 4, y); y += 11;
                for (int i = 0; i < p; i++) {
                    snprintf(l, sizeof(l), "b%d %8.3g %8.3g %5.2f %.4f",
                             i, beta[i], SE[i], tval[i], pval[i]);
                    d.drawString(l, 4, y); y += 11;
                }
            } else {
                d.drawString("VIF (multicoll.):", 4, y); y += 11;
                for (int i = 0; i < k; i++) {
                    snprintf(l, sizeof(l), "VIF X%d = %.3g %s", i+1, VIF[i], VIF[i] > 10 ? "(HIGH!)" : "");
                    if (VIF[i] > 10) d.setTextColor(C_ERR, C_BG);
                    else             d.setTextColor(C_FG, C_BG);
                    d.drawString(l, 4, y); y += 11;
                }
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 2) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("mreg");
            }
        }
    }
    void stepwise() {
        int yi = pickList("Y list?"); if (yi < 0) return;
        int n = G.stat[yi].count;
        int cand[STAT_LISTS], nc = 0;
        for (int i = 0; i < STAT_LISTS; i++)
            if (i != yi && G.stat[i].count == n && n > 2) cand[nc++] = i;
        if (nc < 1) { DM.notify("No candidates", C_ERR); return; }
        bool included[STAT_LISTS] = {false};
        int order[STAT_LISTS]; int nin = 0;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Stepwise (forward)"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        int y = HDR_H + 2;
        while (nin < nc) {
            int best = -1;
            double bestR2 = -1;
            for (int c = 0; c < nc; c++) {
                if (included[c]) continue;
                int test[STAT_LISTS];
                for (int j = 0; j < nin; j++) test[j] = order[j];
                test[nin] = cand[c];
                int k = nin + 1, p = k + 1;
                double XtX[36] = {0}, XtY[6] = {0};
                for (int s = 0; s < n; s++) {
                    double row[6]; row[0] = 1;
                    for (int j = 0; j < k; j++) row[j+1] = G.stat[test[j]].data[s];
                    for (int i = 0; i < p; i++) {
                        XtY[i] += row[i] * G.stat[yi].data[s];
                        for (int jj = 0; jj < p; jj++) XtX[i*p+jj] += row[i]*row[jj];
                    }
                }
                if (!Mat::gaussSolve(XtX, p, XtY)) continue;
                double yMean = 0;
                for (int s = 0; s < n; s++) yMean += G.stat[yi].data[s]; yMean /= n;
                double sst = 0, sse = 0;
                for (int s = 0; s < n; s++) {
                    double yh = XtY[0];
                    for (int j = 0; j < k; j++) yh += XtY[j+1] * G.stat[test[j]].data[s];
                    sst += (G.stat[yi].data[s] - yMean)*(G.stat[yi].data[s] - yMean);
                    sse += (G.stat[yi].data[s] - yh)*(G.stat[yi].data[s] - yh);
                }
                double r2 = (sst > 0) ? 1 - sse/sst : 0;
                if (r2 > bestR2) { bestR2 = r2; best = c; }
            }
            if (best < 0) break;
            included[best] = true;
            order[nin++] = cand[best];
            char l[40]; snprintf(l, sizeof(l), "+ L%d  R^2=%.4f", cand[best]+1, bestR2);
            d.drawString(l, 4, y); y += 11;
            if (y > LCD_H - FTR_H - 14) break;
            if (nin >= 5) break;
        }
        DM.waitEsc();
    }
    void predict() {
        if (lastXi < 0) { DM.notify("No model yet", C_WARN); return; }
        char b[24] = "1";
        if (!DM.inputLine("x =", b, sizeof(b))) return;
        double x = atof(b);
        double y = 0;
        switch (lastType) {
            case 0: case 6: y = lastA + lastB*x; break;
            case 1: y = lastA + lastB*x + lastC*x*x; break;
            case 2: y = lastA * pow(x, lastB); break;
            case 3: y = lastA * exp(lastB*x); break;
            case 4: y = lastA + lastB * log(x); break;
            case 5: { double e = lastA + lastB*x; y = 1.0 / (1.0 + exp(-e)); } break;
        }
        char vs[40]; formatNum(y, vs, sizeof(vs));
        char m[40]; snprintf(m, sizeof(m), "y = %s", vs);
        DM.notify(m, C_OK, 1500);
        G.ans = y; G.ansComplex = false;
    }
    void saveResid() {
        if (lastXi < 0) { DM.notify("No model yet", C_WARN); return; }
        int dst = pickList("Save to which?");
        if (dst < 0) return;
        if (dst == lastYi || dst == lastXi) { DM.notify("Same list", C_ERR); return; }
        int n = G.stat[lastXi].count;
        for (int i = 0; i < n; i++) {
            double x = G.stat[lastXi].data[i];
            double y = G.stat[lastYi].data[i];
            double yh = 0;
            switch (lastType) {
                case 0: case 6: yh = lastA + lastB*x; break;
                case 1: yh = lastA + lastB*x + lastC*x*x; break;
                case 2: yh = lastA * pow(x, lastB); break;
                case 3: yh = lastA * exp(lastB*x); break;
                case 4: yh = lastA + lastB * log(x); break;
                case 5: { double e = lastA + lastB*x; yh = 1.0/(1.0+exp(-e)); } break;
            }
            G.stat[dst].data[i] = y - yh;
        }
        G.stat[dst].count = n;
        DM.notify("Residuals saved", C_OK);
    }
};

class StatTests {
public:
    void run() {
        const char* items[] = {
            "z-test (1-sample)","t-test (1-sample)","t-test (2-sample, eq. var)",
            "Welch's t-test","Paired t-test","F-test (variance)","One-way ANOVA",
            "Two-way ANOVA","Chi-Square GOF","Chi-Square Independence (rxc)",
            "Fisher's exact (2x2)","Back"
        };
        int s = DM.menu("Tests", items, 12);
        if (s < 0 || s == 11) return;
        switch (s) {
            case 0: zTest1();        break;
            case 1: tTest1();        break;
            case 2: tTest2(false);   break;
            case 3: tTest2(true);    break;
            case 4: tTestPaired();   break;
            case 5: fTest();         break;
            case 6: anova1way();     break;
            case 7: anova2way();     break;
            case 8: chiGOF();        break;
            case 9: chiIndep();      break;
            case 10:fisherExact();   break;
        }
    }
private:
    int pickList(const char* title) {
        const char* items[] = {"L1","L2","L3","L4","L5","L6","L7","L8"};
        return DM.menu(title, items, STAT_LISTS);
    }
    int pickSide() {
        const char* items[] = {"Two-sided", "Upper (right)", "Lower (left)"};
        return DM.menu("Side", items, 3);
    }
    void showTest(const char* nm, double stat, int df, double p, int side = 0) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(nm); DM.footer("S:BMP", "Y:Ans", "ESC:back");
        char l[40];
        d.setTextColor(C_FG, C_BG);
        snprintf(l, sizeof(l), "stat = %.5g", stat);  d.drawString(l, 4, HDR_H+8);
        if (df > 0) { snprintf(l, sizeof(l), "df   = %d", df);     d.drawString(l, 4, HDR_H+22); }
        snprintf(l, sizeof(l), "p    = %.5g", p);     d.setTextColor(C_OK, C_BG); d.drawString(l, 4, HDR_H+36);
        d.setTextColor(p<0.05 ? C_OK : C_DIM, C_BG);
        d.drawString(p<0.05 ? "* significant (a=0.05)" : "n.s.", 4, HDR_H+52);
        const char* sd[] = {"two-sided", "upper", "lower"};
        d.setTextColor(C_DIM, C_BG); d.drawString(sd[side], 4, HDR_H+66);
        G.ans = p; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("p->Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("test");
        }
    }
    void zTest1() {
        int li = pickList("Sample list?"); if (li < 0) return;
        char b[16]="0"; if (!DM.inputLine("mu0:", b, sizeof(b))) return;
        double mu0 = atof(b);
        strcpy(b, "1"); if (!DM.inputLine("sigma (known):", b, sizeof(b))) return;
        double sd = atof(b);
        int side = pickSide(); if (side < 0) return;
        int n = G.stat[li].count;
        double sum = 0;
        for (int i = 0; i < n; i++) sum += G.stat[li].data[i];
        double m = sum / n;
        double z = (m - mu0) / (sd / sqrt((double)n));
        double p;
        if (side == 0)      p = 2.0 * (1.0 - Dist::normalCDF(fabs(z)));
        else if (side == 1) p = 1.0 - Dist::normalCDF(z);
        else                p = Dist::normalCDF(z);
        showTest("z-test", z, 0, p, side);
    }
    void tTest1() {
        int li = pickList("Sample list?"); if (li < 0) return;
        char b[16]="0"; if (!DM.inputLine("mu0:", b, sizeof(b))) return;
        double mu0 = atof(b);
        int side = pickSide(); if (side < 0) return;
        int n = G.stat[li].count;
        if (n < 2) { DM.notify("n<2", C_ERR); return; }
        double sum=0, sum2=0;
        for (int i=0;i<n;i++){ sum+=G.stat[li].data[i]; sum2+=G.stat[li].data[i]*G.stat[li].data[i]; }
        double m = sum/n, sd = sqrt((sum2 - n*m*m)/(n-1));
        double t = (m - mu0) / (sd / sqrt((double)n));
        int df = n - 1;
        double p;
        if (side == 0)      p = 2.0 * (1.0 - Dist::tCDF(fabs(t), df));
        else if (side == 1) p = 1.0 - Dist::tCDF(t, df);
        else                p = Dist::tCDF(t, df);
        showTest("1-sample t", t, df, p, side);
    }
    void tTest2(bool welch) {
        int la = pickList("Group A?"); if (la < 0) return;
        int lb = pickList("Group B?"); if (lb < 0) return;
        int side = pickSide(); if (side < 0) return;
        int na=G.stat[la].count, nb=G.stat[lb].count;
        if (na<2 || nb<2) { DM.notify("n<2", C_ERR); return; }
        double sa=0,sa2=0,sb=0,sb2=0;
        for (int i=0;i<na;i++){ sa+=G.stat[la].data[i]; sa2+=G.stat[la].data[i]*G.stat[la].data[i]; }
        for (int i=0;i<nb;i++){ sb+=G.stat[lb].data[i]; sb2+=G.stat[lb].data[i]*G.stat[lb].data[i]; }
        double ma=sa/na, mb=sb/nb;
        double va=(sa2-na*ma*ma)/(na-1), vb=(sb2-nb*mb*mb)/(nb-1);
        double t; int df;
        if (!welch) {
            double sp = sqrt(((na-1)*va + (nb-1)*vb) / (na+nb-2));
            t = (ma - mb) / (sp * sqrt(1.0/na + 1.0/nb));
            df = na + nb - 2;
        } else {
            double seA = va/na, seB = vb/nb;
            t = (ma - mb) / sqrt(seA + seB);
            double num = (seA + seB) * (seA + seB);
            double den = (seA*seA)/(na-1) + (seB*seB)/(nb-1);
            df = (int)(num/den + 0.5);
        }
        double p;
        if (side == 0)      p = 2.0 * (1.0 - Dist::tCDF(fabs(t), df));
        else if (side == 1) p = 1.0 - Dist::tCDF(t, df);
        else                p = Dist::tCDF(t, df);
        showTest(welch ? "Welch t" : "2-samp t (eq)", t, df, p, side);
    }
    void tTestPaired() {
        int la = pickList("Pair A?"); if (la < 0) return;
        int lb = pickList("Pair B?"); if (lb < 0) return;
        int side = pickSide(); if (side < 0) return;
        int n = G.stat[la].count;
        if (G.stat[lb].count != n || n < 2) { DM.notify("Mismatch / n<2", C_ERR); return; }
        double sd1=0, sd2=0;
        for (int i=0;i<n;i++){ double dd=G.stat[la].data[i]-G.stat[lb].data[i]; sd1+=dd; sd2+=dd*dd; }
        double m = sd1/n;
        double v = (sd2 - n*m*m)/(n-1);
        double t = m / (sqrt(v)/sqrt((double)n));
        int df = n - 1;
        double p;
        if (side == 0)      p = 2.0 * (1.0 - Dist::tCDF(fabs(t), df));
        else if (side == 1) p = 1.0 - Dist::tCDF(t, df);
        else                p = Dist::tCDF(t, df);
        showTest("Paired t", t, df, p, side);
    }
    void fTest() {
        int la = pickList("List A?"); if (la < 0) return;
        int lb = pickList("List B?"); if (lb < 0) return;
        int na=G.stat[la].count, nb=G.stat[lb].count;
        if (na<2 || nb<2) { DM.notify("n<2", C_ERR); return; }
        double sa=0,sa2=0,sb=0,sb2=0;
        for (int i=0;i<na;i++){ sa+=G.stat[la].data[i]; sa2+=G.stat[la].data[i]*G.stat[la].data[i]; }
        for (int i=0;i<nb;i++){ sb+=G.stat[lb].data[i]; sb2+=G.stat[lb].data[i]*G.stat[lb].data[i]; }
        double ma=sa/na, mb=sb/nb;
        double va=(sa2-na*ma*ma)/(na-1), vb=(sb2-nb*mb*mb)/(nb-1);
        double F = va/vb;
        int df1 = na-1, df2 = nb-1;
        double p = (F > 1) ? 2*(1 - Dist::FCDF(F, df1, df2)) : 2*Dist::FCDF(F, df1, df2);
        showTest("F-test", F, df1, p, 0);
    }
    void anova1way() {
        int g[STAT_LISTS], gN = 0, total = 0;
        for (int i = 0; i < STAT_LISTS; i++) if (G.stat[i].count > 1) g[gN++] = i;
        if (gN < 2) { DM.notify("Need >=2 lists", C_ERR); return; }
        double grand = 0;
        for (int i = 0; i < gN; i++) {
            int n = G.stat[g[i]].count;
            for (int j = 0; j < n; j++) grand += G.stat[g[i]].data[j];
            total += n;
        }
        double grandM = grand/total;
        double SSB=0, SSW=0;
        for (int i = 0; i < gN; i++) {
            int n = G.stat[g[i]].count;
            double s = 0;
            for (int j = 0; j < n; j++) s += G.stat[g[i]].data[j];
            double m = s/n;
            SSB += n*(m-grandM)*(m-grandM);
            for (int j = 0; j < n; j++) {
                double d = G.stat[g[i]].data[j]-m;
                SSW += d*d;
            }
        }
        int df1 = gN-1, df2 = total-gN;
        double F = (SSB/df1) / (SSW/df2);
        double p = 1.0 - Dist::FCDF(F, df1, df2);
        showTest("ANOVA 1way", F, df1, p, 0);
    }
    void anova2way() {
        int n = G.stat[0].count;
        if (n < 4 || G.stat[1].count != n || G.stat[2].count != n) {
            DM.notify("L1=Y, L2=A, L3=B", C_ERR); return;
        }
        int Alev[20], nA = 0, Blev[20], nB = 0;
        for (int i = 0; i < n; i++) {
            int a = (int)round(G.stat[1].data[i]);
            int b = (int)round(G.stat[2].data[i]);
            bool found = false;
            for (int k = 0; k < nA; k++) if (Alev[k] == a) { found = true; break; }
            if (!found && nA < 20) Alev[nA++] = a;
            found = false;
            for (int k = 0; k < nB; k++) if (Blev[k] == b) { found = true; break; }
            if (!found && nB < 20) Blev[nB++] = b;
        }
        double sumAll = 0;
        for (int i = 0; i < n; i++) sumAll += G.stat[0].data[i];
        double mAll = sumAll / n;
        double sumA[20] = {0}, cntA[20] = {0};
        double sumB[20] = {0}, cntB[20] = {0};
        for (int i = 0; i < n; i++) {
            int a = (int)round(G.stat[1].data[i]);
            int b = (int)round(G.stat[2].data[i]);
            for (int k = 0; k < nA; k++) if (Alev[k] == a) { sumA[k] += G.stat[0].data[i]; cntA[k]++; break; }
            for (int k = 0; k < nB; k++) if (Blev[k] == b) { sumB[k] += G.stat[0].data[i]; cntB[k]++; break; }
        }
        double SSA = 0;
        for (int k = 0; k < nA; k++) {
            double m = sumA[k]/cntA[k];
            SSA += cntA[k] * (m - mAll) * (m - mAll);
        }
        double SSB = 0;
        for (int k = 0; k < nB; k++) {
            double m = sumB[k]/cntB[k];
            SSB += cntB[k] * (m - mAll) * (m - mAll);
        }
        double SST = 0;
        for (int i = 0; i < n; i++) SST += (G.stat[0].data[i] - mAll) * (G.stat[0].data[i] - mAll);
        double SSE = SST - SSA - SSB;
        int dfA = nA-1, dfB = nB-1, dfE = n - nA - nB + 1;
        double FA = (SSA/dfA) / (SSE/dfE);
        double FB = (SSB/dfB) / (SSE/dfE);
        double pA = 1 - Dist::FCDF(FA, dfA, dfE);
        double pB = 1 - Dist::FCDF(FB, dfB, dfE);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("ANOVA 2-way"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40]; int y = HDR_H + 4;
        snprintf(l,sizeof(l),"Factor A: F=%.3g p=%.4f", FA, pA); d.drawString(l, 4, y); y += 11;
        snprintf(l,sizeof(l),"Factor B: F=%.3g p=%.4f", FB, pB); d.drawString(l, 4, y); y += 11;
        snprintf(l,sizeof(l),"SSA=%.3g SSB=%.3g", SSA, SSB);     d.drawString(l, 4, y); y += 11;
        snprintf(l,sizeof(l),"SSE=%.3g SST=%.3g", SSE, SST);     d.drawString(l, 4, y); y += 11;
        snprintf(l,sizeof(l),"dfA=%d dfB=%d dfE=%d", dfA, dfB, dfE); d.drawString(l, 4, y); y += 11;
        DM.waitEsc();
    }
    void chiGOF() {
        int oL = pickList("Observed?"); if (oL < 0) return;
        int eL = pickList("Expected?"); if (eL < 0) return;
        int n = G.stat[oL].count;
        if (G.stat[eL].count != n || n < 2) { DM.notify("Mismatch", C_ERR); return; }
        double chi = 0;
        for (int i = 0; i < n; i++) {
            double o = G.stat[oL].data[i], e = G.stat[eL].data[i];
            if (e <= 0) { DM.notify("E > 0 required", C_ERR); return; }
            chi += (o-e)*(o-e)/e;
        }
        int df = n - 1;
        double p = 1.0 - Dist::chiSquareCDF(chi, df);
        showTest("Chi^2 GOF", chi, df, p, 0);
    }
    void chiIndep() {
        char b[8] = "2";
        if (!DM.inputLine("Rows (2-6):", b, sizeof(b))) return;
        int r = atoi(b);
        strcpy(b, "2");
        if (!DM.inputLine("Cols (2-6):", b, sizeof(b))) return;
        int c = atoi(b);
        if (r < 2 || c < 2 || r > 6 || c > 6) { DM.notify("2..6", C_ERR); return; }
        double O[36];
        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++) {
                char p[16]; snprintf(p, sizeof(p), "O[%d][%d]:", i+1, j+1);
                char x[16] = "0";
                if (!DM.inputLine(p, x, sizeof(x))) return;
                O[i*c+j] = atof(x);
            }
        double rowS[6] = {0}, colS[6] = {0}, total = 0;
        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++) {
                rowS[i] += O[i*c+j];
                colS[j] += O[i*c+j];
                total  += O[i*c+j];
            }
        if (total <= 0) { DM.notify("Total = 0", C_ERR); return; }
        double chi = 0;
        for (int i = 0; i < r; i++)
            for (int j = 0; j < c; j++) {
                double e = rowS[i]*colS[j]/total;
                if (e > 0) chi += (O[i*c+j]-e)*(O[i*c+j]-e)/e;
            }
        int df = (r-1)*(c-1);
        double p = 1.0 - Dist::chiSquareCDF(chi, df);
        showTest("Chi^2 indep", chi, df, p, 0);
    }
    void fisherExact() {
        char b[8];
        long long a, bb, cc, dd;
        strcpy(b, "0"); if (!DM.inputLine("a:", b, sizeof(b))) return; a  = atol(b);
        strcpy(b, "0"); if (!DM.inputLine("b:", b, sizeof(b))) return; bb = atol(b);
        strcpy(b, "0"); if (!DM.inputLine("c:", b, sizeof(b))) return; cc = atol(b);
        strcpy(b, "0"); if (!DM.inputLine("d:", b, sizeof(b))) return; dd = atol(b);
        long long n = a + bb + cc + dd;
        long long r1 = a + bb, r2 = cc + dd, c1 = a + cc, c2 = bb + dd;
        auto logFact = [](long long n) { return lgamma((double)(n+1)); };
        auto logHyper = [&](long long aa) {
            return logFact(r1) + logFact(r2) + logFact(c1) + logFact(c2) -
                   logFact(n) - logFact(aa) - logFact(r1-aa) - logFact(c1-aa) - logFact(r2 - (c1-aa));
        };
        double lp0 = logHyper(a);
        long long mn = (r1 < c1) ? r1 : c1;
        double pSum = 0;
        for (long long k = 0; k <= mn; k++) {
            long long bk = r1 - k;
            long long ck = c1 - k;
            long long dk = r2 - ck;
            if (bk < 0 || ck < 0 || dk < 0) continue;
            double lp = logHyper(k);
            if (lp <= lp0 + 1e-9) pSum += exp(lp);
        }
        showTest("Fisher 2x2", a, 0, pSum, 0);
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 15 / 20 ─ ノンパラ / リスク / 分布ブラウザ                      ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class StatNonparOdds {
public:
    void run() {
        const char* items[] = {
            "Mann-Whitney U","Wilcoxon signed rank","Kruskal-Wallis","Friedman",
            "Sign test","Spearman correlation","Kendall tau","Odds Ratio + 95% CI",
            "Risk Ratio + 95% CI","Risk Difference + 95% CI","NNT (Number Needed to Treat)",
            "Confidence Interval (mean)","CI (proportion)","Back"
        };
        int s = DM.menu("Nonpar/Risk", items, 14);
        if (s < 0 || s == 13) return;
        switch (s) {
            case 0: mannWhitney();    break;
            case 1: wilcoxon();       break;
            case 2: kruskalWallis();  break;
            case 3: friedman();       break;
            case 4: signTest();       break;
            case 5: spearman();       break;
            case 6: kendallTau();     break;
            case 7: oddsRatio();      break;
            case 8: riskRatio();      break;
            case 9: riskDiff();       break;
            case 10: nnt();           break;
            case 11: ciMean();        break;
            case 12: ciProp();        break;
        }
    }
private:
    int pickList(const char* title) {
        const char* items[] = {"L1","L2","L3","L4","L5","L6","L7","L8"};
        return DM.menu(title, items, STAT_LISTS);
    }
    void showTest(const char* nm, double stat, int df, double p, const char* extra = "") {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(nm); DM.footer("Y:p->Ans","S:BMP","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l, sizeof(l), "stat = %.5g", stat);  d.drawString(l, 4, HDR_H+8);
        if (df > 0) { snprintf(l, sizeof(l), "df   = %d", df); d.drawString(l, 4, HDR_H+22); }
        d.setTextColor(C_OK, C_BG);
        snprintf(l, sizeof(l), "p    = %.5g", p);     d.drawString(l, 4, HDR_H+36);
        if (extra && *extra) { d.setTextColor(C_DIM, C_BG); d.drawString(extra, 4, HDR_H+52); }
        d.setTextColor(p < 0.05 ? C_OK : C_DIM, C_BG);
        d.drawString(p < 0.05 ? "* significant" : "n.s.", 4, HDR_H+66);
        G.ans = p; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("p->Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("test");
        }
    }
    void mannWhitney() {
        int la = pickList("Group A?"); if (la < 0) return;
        int lb = pickList("Group B?"); if (lb < 0) return;
        int na = G.stat[la].count, nb = G.stat[lb].count;
        if (na < 2 || nb < 2) { DM.notify("n<2", C_ERR); return; }
        int N = na + nb;
        double* v = (double*)malloc(N * sizeof(double));
        int*    g = (int*)   malloc(N * sizeof(int));
        for (int i = 0; i < na; i++) { v[i] = G.stat[la].data[i]; g[i] = 0; }
        for (int i = 0; i < nb; i++) { v[na+i] = G.stat[lb].data[i]; g[na+i] = 1; }
        for (int i = 1; i < N; i++) {
            double k = v[i]; int gk = g[i]; int j = i - 1;
            while (j >= 0 && v[j] > k) { v[j+1]=v[j]; g[j+1]=g[j]; j--; }
            v[j+1] = k; g[j+1] = gk;
        }
        double* rk = (double*)malloc(N * sizeof(double));
        int i = 0;
        while (i < N) {
            int j = i; while (j+1 < N && v[j+1] == v[i]) j++;
            double avg = (i+1 + j+1) / 2.0;
            for (int t = i; t <= j; t++) rk[t] = avg;
            i = j + 1;
        }
        double Ra = 0;
        for (int k = 0; k < N; k++) if (g[k] == 0) Ra += rk[k];
        double U1 = Ra - na*(na+1)/2.0;
        double U2 = (double)na*nb - U1;
        double U = (U1 < U2) ? U1 : U2;
        double mu = na*nb/2.0;
        double sd = sqrt((double)na*nb*(na+nb+1)/12.0);
        double z = (U - mu) / sd;
        double p = 2.0 * (1.0 - Dist::normalCDF(fabs(z)));
        free(v); free(g); free(rk);
        char extra[32]; snprintf(extra, sizeof(extra), "z=%.3f", z);
        showTest("Mann-Whitney U", U, 0, p, extra);
    }
    void wilcoxon() {
        int la = pickList("Pair A?"); if (la < 0) return;
        int lb = pickList("Pair B?"); if (lb < 0) return;
        int n = G.stat[la].count;
        if (G.stat[lb].count != n || n < 2) { DM.notify("Mismatch", C_ERR); return; }
        double* d = (double*)malloc(n * sizeof(double));
        int m = 0;
        for (int i = 0; i < n; i++) {
            double t = G.stat[la].data[i] - G.stat[lb].data[i];
            if (t != 0) d[m++] = t;
        }
        if (m < 2) { free(d); DM.notify("All ties", C_ERR); return; }
        double* ad = (double*)malloc(m * sizeof(double));
        int*    sg = (int*)   malloc(m * sizeof(int));
        for (int i = 0; i < m; i++) { ad[i] = fabs(d[i]); sg[i] = (d[i] > 0) ? 1 : -1; }
        for (int i = 1; i < m; i++) {
            double k = ad[i]; int s = sg[i]; int j = i - 1;
            while (j >= 0 && ad[j] > k) { ad[j+1] = ad[j]; sg[j+1] = sg[j]; j--; }
            ad[j+1] = k; sg[j+1] = s;
        }
        double Wp = 0, Wn = 0;
        int i = 0;
        while (i < m) {
            int j = i; while (j+1 < m && ad[j+1] == ad[i]) j++;
            double avg = (i+1 + j+1) / 2.0;
            for (int t = i; t <= j; t++) {
                if (sg[t] > 0) Wp += avg; else Wn += avg;
            }
            i = j + 1;
        }
        double W = (Wp < Wn) ? Wp : Wn;
        double mu = m*(m+1)/4.0;
        double sd = sqrt(m*(m+1)*(2*m+1)/24.0);
        double z = (W - mu) / sd;
        double p = 2.0 * (1.0 - Dist::normalCDF(fabs(z)));
        free(d); free(ad); free(sg);
        char extra[32]; snprintf(extra, sizeof(extra), "z=%.3f", z);
        showTest("Wilcoxon", W, 0, p, extra);
    }
    void kruskalWallis() {
        int g[STAT_LISTS], gN = 0, total = 0;
        for (int i = 0; i < STAT_LISTS; i++) if (G.stat[i].count >= 2) g[gN++] = i;
        if (gN < 2) { DM.notify("Need >=2 lists", C_ERR); return; }
        for (int i = 0; i < gN; i++) total += G.stat[g[i]].count;
        double* v = (double*)malloc(total * sizeof(double));
        int*    grp = (int*) malloc(total * sizeof(int));
        int p = 0;
        for (int i = 0; i < gN; i++) {
            for (int j = 0; j < G.stat[g[i]].count; j++) {
                v[p] = G.stat[g[i]].data[j]; grp[p] = i; p++;
            }
        }
        for (int i = 1; i < total; i++) {
            double k = v[i]; int gg = grp[i]; int j = i - 1;
            while (j >= 0 && v[j] > k) { v[j+1] = v[j]; grp[j+1] = grp[j]; j--; }
            v[j+1] = k; grp[j+1] = gg;
        }
        double* rk = (double*)malloc(total * sizeof(double));
        int i = 0;
        while (i < total) {
            int j = i; while (j+1 < total && v[j+1] == v[i]) j++;
            double avg = (i+1 + j+1)/2.0;
            for (int t = i; t <= j; t++) rk[t] = avg;
            i = j + 1;
        }
        double H = 0;
        for (int k = 0; k < gN; k++) {
            double Rk = 0; int nk = 0;
            for (int t = 0; t < total; t++) if (grp[t] == k) { Rk += rk[t]; nk++; }
            if (nk > 0) H += Rk*Rk/nk;
        }
        H = 12.0/(total*(total+1)) * H - 3*(total+1);
        int df = gN - 1;
        double pp = 1.0 - Dist::chiSquareCDF(H, df);
        free(v); free(grp); free(rk);
        showTest("Kruskal-Wallis", H, df, pp);
    }
    void friedman() {
        int g[STAT_LISTS], gN = 0;
        for (int i = 0; i < STAT_LISTS; i++) if (G.stat[i].count > 0) g[gN++] = i;
        if (gN < 3) { DM.notify("Need >=3 lists", C_ERR); return; }
        int n = G.stat[g[0]].count;
        for (int i = 1; i < gN; i++)
            if (G.stat[g[i]].count != n) { DM.notify("Same length req", C_ERR); return; }
        double R[STAT_LISTS] = {0};
        double row[STAT_LISTS], rk[STAT_LISTS];
        for (int r = 0; r < n; r++) {
            for (int j = 0; j < gN; j++) row[j] = G.stat[g[j]].data[r];
            int idx[STAT_LISTS];
            for (int j = 0; j < gN; j++) idx[j] = j;
            for (int i = 1; i < gN; i++) {
                int k = idx[i]; int j = i - 1;
                while (j >= 0 && row[idx[j]] > row[k]) { idx[j+1] = idx[j]; j--; }
                idx[j+1] = k;
            }
            for (int j = 0; j < gN; j++) rk[idx[j]] = j + 1;
            for (int j = 0; j < gN; j++) R[j] += rk[j];
        }
        double sumR2 = 0;
        for (int j = 0; j < gN; j++) sumR2 += R[j]*R[j];
        double Q = 12.0/(n*gN*(gN+1)) * sumR2 - 3.0*n*(gN+1);
        int df = gN - 1;
        double p = 1.0 - Dist::chiSquareCDF(Q, df);
        showTest("Friedman", Q, df, p);
    }
    void signTest() {
        int la = pickList("Pair A?"); if (la < 0) return;
        int lb = pickList("Pair B?"); if (lb < 0) return;
        int n = G.stat[la].count;
        if (G.stat[lb].count != n || n < 2) { DM.notify("Mismatch", C_ERR); return; }
        int pos = 0, neg = 0;
        for (int i = 0; i < n; i++) {
            double d = G.stat[la].data[i] - G.stat[lb].data[i];
            if (d > 0) pos++;
            else if (d < 0) neg++;
        }
        int N = pos + neg;
        int k = (pos < neg) ? pos : neg;
        double p = 2.0 * Dist::binomCDF(k, N, 0.5);
        if (p > 1) p = 1;
        char extra[32]; snprintf(extra, sizeof(extra), "+%d -%d", pos, neg);
        showTest("Sign test", k, 0, p, extra);
    }
    void spearman() {
        int la = pickList("X?"); if (la < 0) return;
        int lb = pickList("Y?"); if (lb < 0) return;
        int n = G.stat[la].count;
        if (G.stat[lb].count != n || n < 3) { DM.notify("n>=3", C_ERR); return; }
        double* xr = rankList(G.stat[la].data, n);
        double* yr = rankList(G.stat[lb].data, n);
        double sx=0, sy=0, sxx=0, syy=0, sxy=0;
        for (int i = 0; i < n; i++) {
            sx += xr[i]; sy += yr[i];
            sxx += xr[i]*xr[i]; syy += yr[i]*yr[i]; sxy += xr[i]*yr[i];
        }
        double rho = (n*sxy - sx*sy) / sqrt((n*sxx - sx*sx)*(n*syy - sy*sy));
        double t = rho * sqrt((double)(n-2)/(1-rho*rho));
        double p = 2.0 * (1.0 - Dist::tCDF(fabs(t), n-2));
        free(xr); free(yr);
        char extra[32]; snprintf(extra, sizeof(extra), "rho=%.4f", rho);
        showTest("Spearman", t, n-2, p, extra);
    }
    double* rankList(double* a, int n) {
        double* tmp = (double*)malloc(n*sizeof(double));
        int*    idx = (int*)   malloc(n*sizeof(int));
        for (int i = 0; i < n; i++) { tmp[i] = a[i]; idx[i] = i; }
        for (int i = 1; i < n; i++) {
            double k = tmp[i]; int ki = idx[i]; int j = i-1;
            while (j>=0 && tmp[j]>k) { tmp[j+1]=tmp[j]; idx[j+1]=idx[j]; j--; }
            tmp[j+1]=k; idx[j+1]=ki;
        }
        double* rk = (double*)malloc(n*sizeof(double));
        int i = 0;
        while (i < n) {
            int j = i; while (j+1 < n && tmp[j+1]==tmp[i]) j++;
            double avg = (i+1 + j+1)/2.0;
            for (int t = i; t <= j; t++) rk[idx[t]] = avg;
            i = j + 1;
        }
        free(tmp); free(idx);
        return rk;
    }
    void kendallTau() {
        int la = pickList("X?"); if (la < 0) return;
        int lb = pickList("Y?"); if (lb < 0) return;
        int n = G.stat[la].count;
        if (G.stat[lb].count != n || n < 3) { DM.notify("n>=3", C_ERR); return; }
        long conc = 0, disc = 0, tieX = 0, tieY = 0;
        for (int i = 0; i < n; i++)
            for (int j = i+1; j < n; j++) {
                double dx = G.stat[la].data[j] - G.stat[la].data[i];
                double dy = G.stat[lb].data[j] - G.stat[lb].data[i];
                if (dx == 0 && dy == 0)      ;
                else if (dx == 0)            tieX++;
                else if (dy == 0)            tieY++;
                else if (dx*dy > 0)          conc++;
                else                          disc++;
            }
        long N = (long)n * (n-1) / 2;
        double tau = (double)(conc - disc) / sqrt((double)(N - tieX)*(N - tieY));
        double z = 3 * tau * sqrt((double)n*(n-1)) / sqrt((double)2*(2*n+5));
        double p = 2.0 * (1.0 - Dist::normalCDF(fabs(z)));
        char extra[32]; snprintf(extra, sizeof(extra), "tau=%.4f", tau);
        showTest("Kendall tau", z, 0, p, extra);
    }
    void oddsRatio() { run2x2("Odds Ratio", 0); }
    void riskRatio() { run2x2("Risk Ratio", 1); }
    void riskDiff()  { run2x2("Risk Diff",  2); }
    void run2x2(const char* title, int kind) {
        char b[10];
        double a, bb, c, d;
        strcpy(b, "0"); if (!DM.inputLine("a (E+ D+):", b, sizeof(b))) return; a  = atof(b);
        strcpy(b, "0"); if (!DM.inputLine("b (E+ D-):", b, sizeof(b))) return; bb = atof(b);
        strcpy(b, "0"); if (!DM.inputLine("c (E- D+):", b, sizeof(b))) return; c  = atof(b);
        strcpy(b, "0"); if (!DM.inputLine("d (E- D-):", b, sizeof(b))) return; d  = atof(b);
        if (a <= 0 || bb <= 0 || c <= 0 || d <= 0) { a += 0.5; bb += 0.5; c += 0.5; d += 0.5; }
        double r1 = a/(a+bb), r2 = c/(c+d);
        double val, lo, hi;
        if (kind == 0) {
            double OR = (a*d)/(bb*c);
            double seL = sqrt(1.0/a + 1.0/bb + 1.0/c + 1.0/d);
            double lnOR = log(OR);
            val = OR; lo = exp(lnOR - 1.96*seL); hi = exp(lnOR + 1.96*seL);
        } else if (kind == 1) {
            double RR = r1/r2;
            double seL = sqrt(bb/(a*(a+bb)) + d/(c*(c+d)));
            double lnRR = log(RR);
            val = RR; lo = exp(lnRR - 1.96*seL); hi = exp(lnRR + 1.96*seL);
        } else {
            double RD = r1 - r2;
            double seR = sqrt(r1*(1-r1)/(a+bb) + r2*(1-r2)/(c+d));
            val = RD; lo = RD - 1.96*seR; hi = RD + 1.96*seR;
        }
        auto& d2 = M5Cardputer.Display;
        d2.fillScreen(C_BG); DM.header(title); DM.footer("Y:Ans","S:BMP","ESC:back");
        d2.setTextColor(C_OK, C_BG);
        char l[50];
        snprintf(l,sizeof(l),"%s = %.5g", title, val);     d2.drawString(l, 4, HDR_H + 8);
        snprintf(l,sizeof(l),"95%% CI [%.4g, %.4g]", lo, hi); d2.drawString(l, 4, HDR_H + 24);
        d2.setTextColor(C_FG, C_BG);
        snprintf(l,sizeof(l),"r1 = %.4g  r2 = %.4g", r1, r2); d2.drawString(l, 4, HDR_H + 40);
        if (kind <= 1) {
            int xC = LCD_W/2, w = LCD_W - 20;
            double L = log(lo), H = log(hi), V = log(val);
            int x0 = xC - w/2, x1 = xC + w/2;
            int sxL = x0 + (int)((L + 2)/4.0 * w);
            int sxH = x0 + (int)((H + 2)/4.0 * w);
            int sxV = x0 + (int)((V + 2)/4.0 * w);
            d2.drawLine(xC, HDR_H+58, xC, HDR_H+72, C_DIM);
            d2.drawLine(sxL, HDR_H+65, sxH, HDR_H+65, C_ACC);
            d2.fillRect(sxV-2, HDR_H+62, 4, 8, C_OK);
        }
        G.ans = val; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("->Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("ratio");
        }
    }
    void nnt() {
        char b[16] = "0.05";
        if (!DM.inputLine("Risk diff:", b, sizeof(b))) return;
        double rd = atof(b);
        if (rd == 0) { DM.notify("RD = 0", C_ERR); return; }
        double v = 1.0 / fabs(rd);
        char vs[24]; formatNum(v, vs, sizeof(vs));
        char m[40]; snprintf(m, sizeof(m), "NNT = %s", vs);
        DM.notify(m, C_OK, 1500);
        G.ans = v; G.ansComplex = false;
    }
    void ciMean() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 2) { DM.notify("n<2", C_ERR); return; }
        char b[16] = "0.95";
        if (!DM.inputLine("Confidence:", b, sizeof(b))) return;
        double c = atof(b);
        double sum=0, sum2=0;
        for (int i=0;i<n;i++){ sum+=G.stat[li].data[i]; sum2+=G.stat[li].data[i]*G.stat[li].data[i]; }
        double m = sum/n;
        double sd = sqrt((sum2 - n*m*m)/(n-1));
        double SE = sd/sqrt((double)n);
        double t = Dist::tInv(0.5 + c/2, n-1);
        double lo = m - t*SE, hi = m + t*SE;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("CI (mean)"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"mean = %.5g", m);             d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"%g%% CI = [%.4g, %.4g]", c*100, lo, hi); d.drawString(l, 4, HDR_H+24);
        snprintf(l,sizeof(l),"SE = %.4g  t = %.3f", SE, t);  d.drawString(l, 4, HDR_H+40);
        DM.waitEsc();
    }
    void ciProp() {
        char b[16];
        long x, n;
        strcpy(b,"5"); if (!DM.inputLine("x:", b, sizeof(b))) return; x = atol(b);
        strcpy(b,"10"); if (!DM.inputLine("n:", b, sizeof(b))) return; n = atol(b);
        strcpy(b,"0.95"); if (!DM.inputLine("Conf:", b, sizeof(b))) return;
        double c = atof(b);
        double p = (double)x/n;
        double z = Dist::normalInv(0.5 + c/2);
        double denom = 1 + z*z/n;
        double center = (p + z*z/(2*n)) / denom;
        double half = z * sqrt(p*(1-p)/n + z*z/(4*n*n)) / denom;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("CI (prop, Wilson)"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"p = %.5g", p); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"%g%% CI = [%.4g, %.4g]", c*100, center-half, center+half);
        d.drawString(l, 4, HDR_H+24);
        DM.waitEsc();
    }
};

class DistBrowser {
public:
    void run() {
        const char* items[] = {
            "Normal N(mu, sigma)","Student t (df)","Chi-Square (df)","F (d1, d2)",
            "Binomial (n, p)","Poisson (lambda)","Geometric (p)","Back"
        };
        int s = DM.menu("Distribution", items, 8);
        if (s < 0 || s == 7) return;
        switch (s) {
            case 0: normal();   break;
            case 1: tDist();    break;
            case 2: chi2();     break;
            case 3: fDist();    break;
            case 4: binomial(); break;
            case 5: poisson();  break;
            case 6: geometric();break;
        }
    }
private:
    int dirMenu() {
        const char* d[] = {"PDF/PMF f(x)","CDF P(X<=x)","Inverse: x s.t. P=q","Plot PDF","Back"};
        return DM.menu("Action", d, 5);
    }
    void show3(const char* t, const char* l1, double v1, const char* l2, double v2,
                              const char* l3, double v3) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans","S:BMP","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40], vs[24];
        formatNum(v1, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H+8);
        formatNum(v2, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l2, vs); d.drawString(l, 4, HDR_H+24);
        formatNum(v3, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l3, vs); d.drawString(l, 4, HDR_H+40);
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("->Ans",C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("dist");
        }
    }
    void plotPDF(double xmin, double xmax, double (*pdf)(double, double*), double* params) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("PDF Plot"); DM.footer("S:BMP","","ESC:back");
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        double maxV = 0;
        for (int px = 0; px < gw; px++) {
            double x = xmin + (xmax-xmin) * px / gw;
            double y = pdf(x, params);
            if (y > maxV) maxV = y;
        }
        if (maxV <= 0) maxV = 1;
        int prevX = -1, prevY = -1;
        for (int px = 0; px < gw; px++) {
            double x = xmin + (xmax-xmin) * px / gw;
            double y = pdf(x, params);
            int sy = gy + gh - 1 - (int)(y/maxV * (gh-2));
            if (prevX >= 0) d.drawLine(prevX, prevY, gx + px, sy, C_ACC);
            prevX = gx + px; prevY = sy;
        }
        DM.waitEsc();
    }
    void normal() {
        char b[24];
        strcpy(b, "0"); if (!DM.inputLine("mu:", b, sizeof(b))) return; double mu = atof(b);
        strcpy(b, "1"); if (!DM.inputLine("sigma:", b, sizeof(b))) return; double sd = atof(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "0"); if (!DM.inputLine("x:", b, sizeof(b))) return; double x = atof(b);
            double pdf = Dist::normalPDF(x, mu, sd);
            double cdf = Dist::normalCDF((x-mu)/sd);
            show3("Normal", "PDF", pdf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = mu + sd * Dist::normalInv(atof(b));
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "x = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        } else {
            double prm[2] = {mu, sd};
            plotPDF(mu - 4*sd, mu + 4*sd, [](double x, double* p){ return Dist::normalPDF(x, p[0], p[1]); }, prm);
        }
    }
    void tDist() {
        char b[24];
        strcpy(b, "10"); if (!DM.inputLine("df:", b, sizeof(b))) return; int df = atoi(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "0"); if (!DM.inputLine("x:", b, sizeof(b))) return; double x = atof(b);
            double pdf = Dist::tPDF(x, df);
            double cdf = Dist::tCDF(x, df);
            show3("Student t", "PDF", pdf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            double x = Dist::tInv(atof(b), df);
            char vs[24]; formatNum(x, vs, sizeof(vs));
            char m[40]; snprintf(m, sizeof(m), "t = %s", vs);
            DM.notify(m, C_OK, 1500);
            G.ans = x; G.ansComplex = false;
        } else {
            static double sdf; sdf = (double)df;
            plotPDF(-5, 5, [](double x, double* p){ return Dist::tPDF(x, (int)p[0]); }, &sdf);
        }
    }
    void chi2() {
        char b[24];
        strcpy(b, "5"); if (!DM.inputLine("df:", b, sizeof(b))) return; int df = atoi(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "0"); if (!DM.inputLine("x:", b, sizeof(b))) return; double x = atof(b);
            double pdf = Dist::chiSquarePDF(x, df);
            double cdf = Dist::chiSquareCDF(x, df);
            show3("Chi^2", "PDF", pdf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
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
    }
    void fDist() {
        char b[24];
        strcpy(b, "5"); if (!DM.inputLine("d1:", b, sizeof(b))) return; int d1 = atoi(b);
        strcpy(b, "5"); if (!DM.inputLine("d2:", b, sizeof(b))) return; int d2 = atoi(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "1"); if (!DM.inputLine("x:", b, sizeof(b))) return; double x = atof(b);
            double pdf = Dist::FPDF(x, d1, d2);
            double cdf = Dist::FCDF(x, d1, d2);
            show3("F", "PDF", pdf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
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
    }
    void binomial() {
        char b[24];
        strcpy(b, "10"); if (!DM.inputLine("n:", b, sizeof(b))) return; int n = atoi(b);
        strcpy(b, "0.5"); if (!DM.inputLine("p:", b, sizeof(b))) return; double p = atof(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "0"); if (!DM.inputLine("k:", b, sizeof(b))) return; int k = atoi(b);
            double pmf = Dist::binomPMF(k, n, p);
            double cdf = Dist::binomCDF(k, n, p);
            show3("Binomial", "PMF", pmf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            int k = Dist::binomInv(atof(b), n, p);
            char m[40]; snprintf(m, sizeof(m), "k = %d", k);
            DM.notify(m, C_OK, 1500);
            G.ans = k; G.ansComplex = false;
        }
    }
    void poisson() {
        char b[24];
        strcpy(b, "3"); if (!DM.inputLine("lambda:", b, sizeof(b))) return; double l = atof(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "0"); if (!DM.inputLine("k:", b, sizeof(b))) return; int k = atoi(b);
            double pmf = Dist::poisPMF(k, l);
            double cdf = Dist::poisCDF(k, l);
            show3("Poisson", "PMF", pmf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            int k = Dist::poisInv(atof(b), l);
            char m[40]; snprintf(m, sizeof(m), "k = %d", k);
            DM.notify(m, C_OK, 1500);
            G.ans = k; G.ansComplex = false;
        }
    }
    void geometric() {
        char b[24];
        strcpy(b, "0.5"); if (!DM.inputLine("p:", b, sizeof(b))) return; double p = atof(b);
        int act = dirMenu(); if (act < 0 || act == 4) return;
        if (act == 0 || act == 1) {
            strcpy(b, "1"); if (!DM.inputLine("k:", b, sizeof(b))) return; int k = atoi(b);
            double pmf = Dist::geomPMF(k, p);
            double cdf = Dist::geomCDF(k, p);
            show3("Geometric", "PMF", pmf, "CDF", cdf, "1-CDF", 1-cdf);
        } else if (act == 2) {
            strcpy(b, "0.95"); if (!DM.inputLine("q:", b, sizeof(b))) return;
            int k = Dist::geomInv(atof(b), p);
            char m[40]; snprintf(m, sizeof(m), "k = %d", k);
            DM.notify(m, C_OK, 1500);
            G.ans = k; G.ansComplex = false;
        }
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 16 / 20 ─ 多変量解析                                            ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class StatMultivar {
public:
    void run() {
        const char* items[] = {
            "PCA (Principal Component Analysis)","Factor Analysis (with Varimax)",
            "k-means Clustering","Hierarchical (Ward)","Decision Tree (CART regress)","Back"
        };
        int s = DM.menu("Multivariate", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: pca();           break;
            case 1: factorAnalysis();break;
            case 2: kmeans();        break;
            case 3: hierarchical();  break;
            case 4: decisionTree();  break;
        }
    }
private:
    int pickList(const char* title) {
        const char* it[] = {"L1","L2","L3","L4","L5","L6","L7","L8"};
        return DM.menu(title, it, STAT_LISTS);
    }
    int collectVarLists(int* out) {
        int n = 0;
        for (int i = 0; i < STAT_LISTS; i++) if (G.stat[i].count >= 2) out[n++] = i;
        if (n < 2) return 0;
        int len = G.stat[out[0]].count;
        int n2 = 0;
        for (int i = 0; i < n; i++) if (G.stat[out[i]].count == len) out[n2++] = out[i];
        return n2;
    }
    void standardize(double* X, int n, int p) {
        for (int j = 0; j < p; j++) {
            double s = 0, s2 = 0;
            for (int i = 0; i < n; i++) { s += X[i*p+j]; s2 += X[i*p+j]*X[i*p+j]; }
            double m = s / n;
            double sd = sqrt((s2 - n*m*m) / (n-1));
            if (sd < 1e-12) sd = 1;
            for (int i = 0; i < n; i++) X[i*p+j] = (X[i*p+j] - m) / sd;
        }
    }
    void covMatrix(double* X, int n, int p, double* C) {
        for (int i = 0; i < p; i++) for (int j = 0; j < p; j++) C[i*p+j] = 0;
        double mean[FA_MAX_VAR];
        for (int j = 0; j < p; j++) {
            double s = 0;
            for (int i = 0; i < n; i++) s += X[i*p+j];
            mean[j] = s / n;
        }
        for (int i = 0; i < n; i++)
            for (int a = 0; a < p; a++)
                for (int b = 0; b < p; b++)
                    C[a*p+b] += (X[i*p+a] - mean[a]) * (X[i*p+b] - mean[b]);
        for (int i = 0; i < p; i++) for (int j = 0; j < p; j++) C[i*p+j] /= (n - 1);
    }
    void corMatrix(double* X, int n, int p, double* R) {
        covMatrix(X, n, p, R);
        double sd[FA_MAX_VAR];
        for (int i = 0; i < p; i++) sd[i] = sqrt(R[i*p+i]);
        for (int i = 0; i < p; i++)
            for (int j = 0; j < p; j++)
                R[i*p+j] = R[i*p+j] / (sd[i] * sd[j]);
    }
    void pca() {
        int lst[FA_MAX_VAR];
        int p = collectVarLists(lst);
        if (p < 2) { DM.notify("Need >=2 lists same len", C_ERR); return; }
        if (p > FA_MAX_VAR) p = FA_MAX_VAR;
        int n = G.stat[lst[0]].count;
        if (n < p+1) { DM.notify("n must be > p", C_ERR); return; }
        double* X = (double*)malloc(n*p*sizeof(double));
        if (!X) { DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        standardize(X, n, p);
        double R[FA_MAX_VAR*FA_MAX_VAR];
        covMatrix(X, n, p, R);
        double eig[FA_MAX_VAR], V[FA_MAX_VAR*FA_MAX_VAR];
        Mat::jacobiEig(R, p, eig, V);
        double total = 0; for (int i = 0; i < p; i++) total += eig[i];
        auto& d = M5Cardputer.Display;
        int page = 0;
        while (true) {
            d.fillScreen(C_BG);
            char th[24]; snprintf(th, sizeof(th), "PCA p%d/2", page+1);
            DM.header(th);
            DM.footer("L/R:page", "S:BMP", "ESC:back");
            d.setTextColor(C_FG, C_BG);
            char l[40];
            int yy = HDR_H + 4;
            if (page == 0) {
                d.drawString("PC  eig    var%   cumul%", 4, yy); yy += 11;
                double cum = 0;
                for (int i = 0; i < p && yy < LCD_H - FTR_H - 4; i++) {
                    cum += eig[i];
                    snprintf(l, sizeof(l), "%2d %6.3f %5.1f%% %5.1f%%",
                             i+1, eig[i], 100.0*eig[i]/total, 100.0*cum/total);
                    d.drawString(l, 4, yy); yy += 11;
                }
            } else {
                d.drawString("Loadings (1st 2 PCs):", 4, yy); yy += 11;
                for (int j = 0; j < p && yy < LCD_H - FTR_H - 4; j++) {
                    snprintf(l, sizeof(l), "L%d  %+.3f  %+.3f",
                             lst[j]+1, V[j*p+0]*sqrt(eig[0]), V[j*p+1]*sqrt(eig[1]));
                    d.drawString(l, 4, yy); yy += 11;
                }
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) { free(X); return; }
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 1) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("pca");
            }
        }
    }
    void varimax(double* L, int p, int k) {
        const double eps = 1e-7;
        for (int iter = 0; iter < 100; iter++) {
            double maxChg = 0;
            for (int i = 0; i < k - 1; i++) {
                for (int j = i + 1; j < k; j++) {
                    double sumU = 0, sumV = 0, sumU2V2 = 0, sumUV = 0;
                    for (int v = 0; v < p; v++) {
                        double a = L[v*k+i], b = L[v*k+j];
                        double u = a*a - b*b;
                        double w = 2*a*b;
                        sumU += u; sumV += w;
                        sumU2V2 += u*u - w*w;
                        sumUV += 2*u*w;
                    }
                    double num = sumUV - 2*sumU*sumV/p;
                    double den = sumU2V2 - (sumU*sumU - sumV*sumV)/p;
                    double th = 0.25 * atan2(num, den);
                    double c = cos(th), s = sin(th);
                    if (fabs(th) > eps) {
                        for (int v = 0; v < p; v++) {
                            double a = L[v*k+i], b = L[v*k+j];
                            L[v*k+i] = c*a + s*b;
                            L[v*k+j] = -s*a + c*b;
                        }
                        if (fabs(th) > maxChg) maxChg = fabs(th);
                    }
                }
            }
            if (maxChg < eps) break;
        }
    }
    void factorAnalysis() {
        int lst[FA_MAX_VAR];
        int p = collectVarLists(lst);
        if (p < 2) { DM.notify("Need >=2 lists", C_ERR); return; }
        if (p > FA_MAX_VAR) p = FA_MAX_VAR;
        int n = G.stat[lst[0]].count;
        char b[8] = "2";
        if (!DM.inputLine("# factors:", b, sizeof(b))) return;
        int k = atoi(b);
        if (k < 1 || k > p) { DM.notify("Invalid k", C_ERR); return; }
        double* X = (double*)malloc(n*p*sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        standardize(X, n, p);
        double R[FA_MAX_VAR*FA_MAX_VAR];
        corMatrix(X, n, p, R);
        double eig[FA_MAX_VAR], V[FA_MAX_VAR*FA_MAX_VAR];
        Mat::jacobiEig(R, p, eig, V);
        double* L = (double*)malloc(p*k*sizeof(double));
        for (int i = 0; i < p; i++)
            for (int f = 0; f < k; f++) L[i*k+f] = V[i*p+f] * sqrt(eig[f] > 0 ? eig[f] : 0);
        varimax(L, p, k);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Factor (Varimax)"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        d.drawString("Var       F1     F2     F3", 4, HDR_H + 2);
        for (int i = 0; i < p && HDR_H + 14 + i*11 < LCD_H - FTR_H - 4; i++) {
            char buf[60]; snprintf(buf, sizeof(buf), "L%-2d  ", lst[i]+1);
            for (int f = 0; f < k && f < 3; f++) {
                char t[16]; snprintf(t, sizeof(t), " %+.3f", L[i*k+f]);
                strncat(buf, t, sizeof(buf) - strlen(buf) - 1);
            }
            d.drawString(buf, 4, HDR_H + 14 + i*11);
        }
        free(X); free(L);
        DM.waitEsc();
    }
    void kmeans() {
        int lst[FA_MAX_VAR];
        int p = collectVarLists(lst);
        if (p < 1) { DM.notify("Need lists", C_ERR); return; }
        if (p > FA_MAX_VAR) p = FA_MAX_VAR;
        int n = G.stat[lst[0]].count;
        if (n > KMEANS_MAX) { DM.notify("n too large", C_ERR); return; }
        char b[8] = "3";
        if (!DM.inputLine("k clusters:", b, sizeof(b))) return;
        int k = atoi(b);
        if (k < 2 || k > 10) { DM.notify("2..10", C_ERR); return; }
        double* X = (double*)malloc(n*p*sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        double* cent = (double*)malloc(k*p*sizeof(double));
        int* assign = (int*)malloc(n*sizeof(int));
        for (int c = 0; c < k; c++) {
            int idx = (int)((double)esp_random()/4294967296.0 * n);
            for (int j = 0; j < p; j++) cent[c*p+j] = X[idx*p+j];
        }
        for (int it = 0; it < 100; it++) {
            bool changed = false;
            for (int i = 0; i < n; i++) {
                double bestD = INFINITY; int bestC = 0;
                for (int c = 0; c < k; c++) {
                    double d2 = 0;
                    for (int j = 0; j < p; j++) {
                        double dd = X[i*p+j] - cent[c*p+j];
                        d2 += dd*dd;
                    }
                    if (d2 < bestD) { bestD = d2; bestC = c; }
                }
                if (it == 0 || assign[i] != bestC) { assign[i] = bestC; changed = true; }
            }
            int* cnt = (int*)calloc(k, sizeof(int));
            for (int c = 0; c < k; c++) for (int j = 0; j < p; j++) cent[c*p+j] = 0;
            for (int i = 0; i < n; i++) {
                cnt[assign[i]]++;
                for (int j = 0; j < p; j++) cent[assign[i]*p+j] += X[i*p+j];
            }
            for (int c = 0; c < k; c++)
                if (cnt[c] > 0) for (int j = 0; j < p; j++) cent[c*p+j] /= cnt[c];
            free(cnt);
            if (!changed) break;
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("k-means"); DM.footer("S:BMP","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"n=%d p=%d k=%d", n, p, k); d.drawString(l, 4, HDR_H + 2);
        d.drawString("Centroids:", 4, HDR_H + 14);
        for (int c = 0; c < k && HDR_H + 26 + c*11 < LCD_H - FTR_H - 4; c++) {
            char buf[40]; snprintf(buf, sizeof(buf), "C%d:", c+1);
            for (int j = 0; j < p && j < 3; j++) {
                char t[16]; snprintf(t, sizeof(t), " %.2f", cent[c*p+j]);
                strncat(buf, t, sizeof(buf) - strlen(buf) - 1);
            }
            d.drawString(buf, 4, HDR_H + 26 + c*11);
        }
        if (p >= 2) {
            double xmin = X[0], xmax = X[0], ymin = X[1], ymax = X[1];
            for (int i = 0; i < n; i++) {
                if (X[i*p+0] < xmin) xmin = X[i*p+0]; if (X[i*p+0] > xmax) xmax = X[i*p+0];
                if (X[i*p+1] < ymin) ymin = X[i*p+1]; if (X[i*p+1] > ymax) ymax = X[i*p+1];
            }
            int gx = LCD_W - 80, gy = HDR_H + 4, gw = 75, gh = LCD_H - HDR_H - FTR_H - 8;
            d.drawRect(gx, gy, gw, gh, C_DIM);
            uint16_t cols[10] = {C_ACC, C_OK, C_ACC2, C_ERR, C_WARN,
                                 0xF7BE, 0x07FF, 0xF81F, 0xFFE0, 0x07E0};
            for (int i = 0; i < n; i++) {
                int px = gx + (int)((X[i*p+0] - xmin)/(xmax - xmin + 1e-12) * (gw-2)) + 1;
                int py = gy + gh - 2 - (int)((X[i*p+1] - ymin)/(ymax - ymin + 1e-12) * (gh-2));
                d.fillCircle(px, py, 1, cols[assign[i] % 10]);
            }
        }
        DM.waitEsc();
        free(X); free(cent); free(assign);
    }
    void hierarchical() {
        int lst[FA_MAX_VAR];
        int p = collectVarLists(lst);
        if (p < 1) { DM.notify("Need lists", C_ERR); return; }
        int n = G.stat[lst[0]].count;
        if (n > HCLUST_MAX) { DM.notify("n too large", C_ERR); return; }
        char b[8] = "3";
        if (!DM.inputLine("k clusters:", b, sizeof(b))) return;
        int k = atoi(b);
        if (k < 2 || k >= n) { DM.notify("Invalid k", C_ERR); return; }
        double* X = (double*)malloc(n*p*sizeof(double));
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        int* clust = (int*)malloc(n*sizeof(int));
        int* size  = (int*)calloc(n, sizeof(int));
        double* centroid = (double*)malloc(n*p*sizeof(double));
        for (int i = 0; i < n; i++) {
            clust[i] = i; size[i] = 1;
            for (int j = 0; j < p; j++) centroid[i*p+j] = X[i*p+j];
        }
        int active = n;
        while (active > k) {
            double bestD = INFINITY;
            int bestI = -1, bestJ = -1;
            for (int i = 0; i < n; i++) if (size[i] > 0)
                for (int j = i + 1; j < n; j++) if (size[j] > 0) {
                    double d2 = 0;
                    for (int q = 0; q < p; q++) {
                        double dd = centroid[i*p+q] - centroid[j*p+q];
                        d2 += dd*dd;
                    }
                    double w = ((double)size[i] * size[j]) / (size[i] + size[j]) * d2;
                    if (w < bestD) { bestD = w; bestI = i; bestJ = j; }
                }
            for (int q = 0; q < p; q++)
                centroid[bestI*p+q] = (size[bestI]*centroid[bestI*p+q] +
                                        size[bestJ]*centroid[bestJ*p+q]) /
                                       (size[bestI] + size[bestJ]);
            size[bestI] += size[bestJ];
            for (int i = 0; i < n; i++) if (clust[i] == bestJ) clust[i] = bestI;
            size[bestJ] = 0;
            active--;
        }
        int map[KMEANS_MAX] = {0};
        int nextId = 1;
        for (int i = 0; i < n; i++) if (size[i] > 0) map[i] = nextId++;
        for (int i = 0; i < n; i++) clust[i] = map[clust[i]];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Hierarchical (Ward)"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l, sizeof(l), "n=%d p=%d k=%d", n, p, k); d.drawString(l, 4, HDR_H + 2);
        for (int c = 1; c <= k && HDR_H + 16 + (c-1)*11 < LCD_H - FTR_H - 4; c++) {
            int cnt = 0;
            for (int i = 0; i < n; i++) if (clust[i] == c) cnt++;
            snprintf(l, sizeof(l), "Cluster %d: n=%d", c, cnt);
            d.drawString(l, 4, HDR_H + 16 + (c-1)*11);
        }
        free(X); free(clust); free(size); free(centroid);
        DM.waitEsc();
    }
    struct TreeNode {
        int feature;
        double threshold;
        double prediction;
        int left, right;
    };
    void decisionTree() {
        int lst[FA_MAX_VAR];
        int p = collectVarLists(lst);
        if (p < 2) { DM.notify("Need >=2 lists (last=Y)", C_ERR); return; }
        int n = G.stat[lst[0]].count;
        if (n > 200) { DM.notify("n too large (max 200)", C_ERR); return; }
        int yi = lst[p-1];
        int xCount = p - 1;
        char b[8] = "3";
        if (!DM.inputLine("max depth:", b, sizeof(b))) return;
        int maxDepth = atoi(b);
        if (maxDepth < 1 || maxDepth > 8) { DM.notify("1..8", C_ERR); return; }
        TreeNode* tree = (TreeNode*)malloc(64*sizeof(TreeNode));
        int treeN = 0;
        int* idxArr = (int*)malloc(n*sizeof(int));
        for (int i = 0; i < n; i++) idxArr[i] = i;
        int root = buildTree(tree, treeN, idxArr, n, lst, xCount, yi, 0, maxDepth);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Decision Tree"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[60];
        snprintf(l, sizeof(l), "Nodes: %d  Depth: %d", treeN, maxDepth);
        d.drawString(l, 4, HDR_H + 2);
        for (int i = 0; i < treeN && HDR_H + 14 + i*11 < LCD_H - FTR_H - 4 && i < 8; i++) {
            if (tree[i].feature < 0) snprintf(l, sizeof(l), "[%d] LEAF -> %.3f", i, tree[i].prediction);
            else if (tree[i].left < 0 || tree[i].right < 0) snprintf(l, sizeof(l), "[%d] L%d <= %.3f -> [--]",
                          i, lst[tree[i].feature]+1, tree[i].threshold);
            else snprintf(l, sizeof(l), "[%d] L%d <= %.3f -> [%d/%d]",
                          i, lst[tree[i].feature]+1, tree[i].threshold, tree[i].left, tree[i].right);
            d.drawString(l, 4, HDR_H + 14 + i*11);
        }
        free(idxArr); free(tree);
        DM.waitEsc();
    }
    int buildTree(TreeNode* tree, int& treeN, int* idx, int nIdx,
                  int* lst, int xCount, int yi, int depth, int maxDepth) {
        if (treeN >= 60) return -1;
        int self = treeN++;
        tree[self].feature = -1; tree[self].left = -1; tree[self].right = -1;
        double sum = 0, sum2 = 0;
        for (int i = 0; i < nIdx; i++) {
            double y = G.stat[yi].data[idx[i]];
            sum += y; sum2 += y*y;
        }
        double mean = sum / nIdx;
        tree[self].prediction = mean;
        if (depth >= maxDepth || nIdx < 4) return self;
        double bestVar = (sum2 - nIdx*mean*mean);
        double bestGain = 0;
        int bestF = -1; double bestT = 0;
        for (int f = 0; f < xCount; f++) {
            for (int t = 0; t < nIdx; t++) {
                double th = G.stat[lst[f]].data[idx[t]];
                double sL = 0, sL2 = 0; int nL = 0;
                double sR = 0, sR2 = 0; int nR = 0;
                for (int i = 0; i < nIdx; i++) {
                    double y = G.stat[yi].data[idx[i]];
                    if (G.stat[lst[f]].data[idx[i]] <= th) { sL += y; sL2 += y*y; nL++; }
                    else                                    { sR += y; sR2 += y*y; nR++; }
                }
                if (nL < 2 || nR < 2) continue;
                double mL = sL/nL, mR = sR/nR;
                double varL = sL2 - nL*mL*mL;
                double varR = sR2 - nR*mR*mR;
                double gain = bestVar - (varL + varR);
                if (gain > bestGain) { bestGain = gain; bestF = f; bestT = th; }
            }
        }
        if (bestF < 0 || bestGain < 1e-6) return self;
        tree[self].feature = bestF;
        tree[self].threshold = bestT;
        int* leftIdx  = (int*)malloc(nIdx*sizeof(int));
        int* rightIdx = (int*)malloc(nIdx*sizeof(int));
        int nL = 0, nR = 0;
        for (int i = 0; i < nIdx; i++) {
            if (G.stat[lst[bestF]].data[idx[i]] <= bestT) leftIdx[nL++] = idx[i];
            else                                          rightIdx[nR++] = idx[i];
        }
        tree[self].left  = buildTree(tree, treeN, leftIdx,  nL, lst, xCount, yi, depth+1, maxDepth);
        tree[self].right = buildTree(tree, treeN, rightIdx, nR, lst, xCount, yi, depth+1, maxDepth);
        free(leftIdx); free(rightIdx);
        return self;
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 17 / 20 ─ 層別解析 / 生存分析 / メタアナリシス / 17統計グラフ   ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class StatStrata {
public:
    void run() {
        const char* items[] = {
            "MH OR + per-stratum + Forest","MH RR + per-stratum + Forest",
            "Food Poisoning (auto detect)","Cochran's Q (heterogeneity)","Breslow-Day test","Back"
        };
        int s = DM.menu("Stratified", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: mhOR();          break;
            case 1: mhRR();          break;
            case 2: foodPoisoning(); break;
            case 3: cochranQ();      break;
            case 4: breslowDay();    break;
        }
    }
    static double a_[10], b_[10], c_[10], d_[10];
    static double sOR_[10], sLO_[10], sHI_[10];
    static double sRR_[10], sRRLO_[10], sRRHI_[10];
    static char strataLabel[10][16];
    static int    K_;
    static double commonVal, commonLO, commonHI;
    static int    plotKind;
private:
    int strataInput(bool askLabel = false) {
        char buf[16] = "3";
        if (!DM.inputLine("# strata (1-10):", buf, sizeof(buf))) return 0;
        int K = atoi(buf);
        if (K < 1 || K > 10) { DM.notify("1..10", C_ERR); return 0; }
        K_ = K;
        for (int k = 0; k < K; k++) {
            char p[20];
            if (askLabel) {
                snprintf(p, sizeof(p), "S%d label:", k+1);
                strncpy(strataLabel[k], "", 16);
                snprintf(strataLabel[k], 16, "S%d", k+1);
                if (!DM.inputLine(p, strataLabel[k], 16)) return 0;
            } else {
                snprintf(strataLabel[k], 16, "S%d", k+1);
            }
            snprintf(p, sizeof(p), "%s a (E+D+):", strataLabel[k]); strcpy(buf, "0");
            if (!DM.inputLine(p, buf, sizeof(buf))) return 0; a_[k] = atof(buf);
            snprintf(p, sizeof(p), "%s b (E+D-):", strataLabel[k]); strcpy(buf, "0");
            if (!DM.inputLine(p, buf, sizeof(buf))) return 0; b_[k] = atof(buf);
            snprintf(p, sizeof(p), "%s c (E-D+):", strataLabel[k]); strcpy(buf, "0");
            if (!DM.inputLine(p, buf, sizeof(buf))) return 0; c_[k] = atof(buf);
            snprintf(p, sizeof(p), "%s d (E-D-):", strataLabel[k]); strcpy(buf, "0");
            if (!DM.inputLine(p, buf, sizeof(buf))) return 0; d_[k] = atof(buf);
        }
        return K;
    }
    void computePerStratumOR() {
        for (int k = 0; k < K_; k++) {
            double a = a_[k], b = b_[k], c = c_[k], d = d_[k];
            if (a==0 || b==0 || c==0 || d==0) { a+=0.5; b+=0.5; c+=0.5; d+=0.5; }
            sOR_[k] = (a*d)/(b*c);
            double seL = sqrt(1.0/a + 1.0/b + 1.0/c + 1.0/d);
            double lnOR = log(sOR_[k]);
            sLO_[k] = exp(lnOR - 1.96*seL);
            sHI_[k] = exp(lnOR + 1.96*seL);
        }
    }
    void computePerStratumRR() {
        for (int k = 0; k < K_; k++) {
            double a = a_[k], b = b_[k], c = c_[k], d = d_[k];
            if (a==0 || b==0 || c==0 || d==0) { a+=0.5; b+=0.5; c+=0.5; d+=0.5; }
            double r1 = a/(a+b), r2 = c/(c+d);
            sRR_[k] = r1/r2;
            double seL = sqrt(b/(a*(a+b)) + d/(c*(c+d)));
            double lnRR = log(sRR_[k]);
            sRRLO_[k] = exp(lnRR - 1.96*seL);
            sRRHI_[k] = exp(lnRR + 1.96*seL);
        }
    }
    double computeMHOR(double& mhLO, double& mhHI, double& chiMH, double& pMH) {
        double num = 0, den = 0;
        for (int k = 0; k < K_; k++) {
            double n = a_[k] + b_[k] + c_[k] + d_[k];
            if (n == 0) continue;
            num += a_[k]*d_[k]/n;
            den += b_[k]*c_[k]/n;
        }
        double OR = num / den;
        double sumP = 0, sumQ = 0, sumR = 0, sumS = 0, sumPR = 0;
        for (int k = 0; k < K_; k++) {
            double n = a_[k] + b_[k] + c_[k] + d_[k];
            if (n == 0) continue;
            double Pk = (a_[k]+d_[k])/n, Qk = (b_[k]+c_[k])/n;
            double Rk = a_[k]*d_[k]/n,   Sk = b_[k]*c_[k]/n;
            sumP += Pk*Rk; sumQ += Qk*Sk; sumR += Rk; sumS += Sk;
            sumPR += Pk*Sk + Qk*Rk;
        }
        double V = sumP/(2*sumR*sumR) + sumPR/(2*sumR*sumS) + sumQ/(2*sumS*sumS);
        double seL = sqrt(V);
        double lnOR = log(OR);
        mhLO = exp(lnOR - 1.96*seL);
        mhHI = exp(lnOR + 1.96*seL);
        double obsA = 0, expA = 0, varA = 0;
        for (int k = 0; k < K_; k++) {
            double n = a_[k]+b_[k]+c_[k]+d_[k];
            if (n <= 1) continue;
            double n1 = a_[k]+b_[k], M = a_[k]+c_[k], n0 = c_[k]+d_[k];
            obsA += a_[k];
            expA += (n1*M)/n;
            varA += (n1*n0*M*(n-M)) / (n*n*(n-1));
        }
        double diff = fabs(obsA - expA) - 0.5;
        if (diff < 0) diff = 0;
        chiMH = (diff*diff) / varA;
        pMH = 1.0 - Dist::chiSquareCDF(chiMH, 1);
        return OR;
    }
    double computeMHRR(double& mhLO, double& mhHI) {
        double num = 0, den = 0;
        for (int k = 0; k < K_; k++) {
            double n = a_[k]+b_[k]+c_[k]+d_[k];
            if (n == 0) continue;
            num += a_[k]*(c_[k]+d_[k])/n;
            den += c_[k]*(a_[k]+b_[k])/n;
        }
        double RR = num / den;
        double V = 0, sumNum = 0, sumDen = 0;
        for (int k = 0; k < K_; k++) {
            double Tk = a_[k]+b_[k]+c_[k]+d_[k];
            if (Tk == 0) continue;
            double n1 = a_[k]+b_[k], n0 = c_[k]+d_[k];
            double M = a_[k]+c_[k];
            V += (M*n1*n0/Tk - a_[k]*c_[k])/Tk;
            sumNum += a_[k]*n0/Tk;
            sumDen += c_[k]*n1/Tk;
        }
        double var = V / (sumNum * sumDen);
        double seL = sqrt(var);
        double lo = exp(log(RR) - 1.96*seL);
        double hi = exp(log(RR) + 1.96*seL);
        mhLO = lo; mhHI = hi;
        return RR;
    }
    void computeBreslowDay(double commonOR, double& X2, double& pBD) {
        X2 = 0;
        for (int k = 0; k < K_; k++) {
            double n1 = a_[k]+b_[k], n0 = c_[k]+d_[k];
            double M  = a_[k]+c_[k], N = a_[k]+b_[k]+c_[k]+d_[k];
            double A = commonOR - 1, B = N - (commonOR-1)*(M + n1), C = -commonOR*M*n1;
            double disc = B*B - 4*A*C;
            if (disc < 0) disc = 0;
            double aE = (fabs(A) < 1e-12) ? -C/B : (-B + sqrt(disc))/(2*A);
            double bE = n1 - aE;
            double cE = M - aE;
            double dE = N - aE - bE - cE;
            if (aE <= 0 || bE <= 0 || cE <= 0 || dE <= 0) continue;
            double var = 1.0/(1.0/aE + 1.0/bE + 1.0/cE + 1.0/dE);
            X2 += (a_[k] - aE)*(a_[k] - aE) / var;
        }
        pBD = 1.0 - Dist::chiSquareCDF(X2, K_ - 1);
    }
    void mhOR() {
        if (strataInput(true) == 0) return;
        computePerStratumOR();
        double mhLO, mhHI, chiMH, pMH;
        commonVal = computeMHOR(mhLO, mhHI, chiMH, pMH);
        commonLO = mhLO; commonHI = mhHI;
        plotKind = 0;
        double X2_BD, p_BD;
        computeBreslowDay(commonVal, X2_BD, p_BD);
        showStratifiedResult("Mantel-Haenszel OR", "OR", chiMH, pMH, X2_BD, p_BD);
    }
    void mhRR() {
        if (strataInput(true) == 0) return;
        computePerStratumRR();
        for (int k = 0; k < K_; k++) {
            sOR_[k] = sRR_[k];
            sLO_[k] = sRRLO_[k];
            sHI_[k] = sRRHI_[k];
        }
        double mhLO, mhHI;
        commonVal = computeMHRR(mhLO, mhHI);
        commonLO = mhLO; commonHI = mhHI;
        plotKind = 1;
        double X2_BD = 0, p_BD = 1, chiMH = 0, pMH = 1;
        double obsA = 0, expA = 0, varA = 0;
        for (int k = 0; k < K_; k++) {
            double n = a_[k]+b_[k]+c_[k]+d_[k];
            if (n <= 1) continue;
            double n1 = a_[k]+b_[k], M = a_[k]+c_[k], n0 = c_[k]+d_[k];
            obsA += a_[k];
            expA += (n1*M)/n;
            varA += (n1*n0*M*(n-M)) / (n*n*(n-1));
        }
        double diff = fabs(obsA - expA) - 0.5;
        if (diff < 0) diff = 0;
        chiMH = (diff*diff) / varA;
        pMH = 1.0 - Dist::chiSquareCDF(chiMH, 1);
        computeBreslowDay(commonVal, X2_BD, p_BD);
        showStratifiedResult("Mantel-Haenszel RR", "RR", chiMH, pMH, X2_BD, p_BD);
    }
    void foodPoisoning() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Food Poisoning Trace"); DM.footer("","","ESC:next");
        d.setTextColor(C_FG, C_BG);
        d.drawString("Setup:", 4, HDR_H + 4);
        d.setTextColor(C_DIM, C_BG);
        d.drawString("L1..L5: foods (0/1)", 4, HDR_H + 16);
        d.drawString("L6: illness (0/1)", 4, HDR_H + 28);
        d.drawString("All same length needed", 4, HDR_H + 40);
        d.setTextColor(C_FG, C_BG);
        d.drawString("Press any key...", 4, HDR_H + 56);
        DM.waitEsc();
        int illL = 5;
        int N = G.stat[illL].count;
        if (N < 4) { DM.notify("L6 too small", C_ERR); return; }
        struct FoodResult {
            int li;
            double RR, lo, hi;
            int a, b, c, dCount;
            double pVal;
        };
        FoodResult res[5];
        int nFood = 0;
        for (int li = 0; li < 5; li++) {
            if (G.stat[li].count != N) continue;
            int aa = 0, bb = 0, cc = 0, dd = 0;
            for (int i = 0; i < N; i++) {
                bool ate = (G.stat[li].data[i] != 0);
                bool ill = (G.stat[illL].data[i] != 0);
                if (ate && ill)       aa++;
                else if (ate && !ill) bb++;
                else if (!ate && ill) cc++;
                else                  dd++;
            }
            double a2 = aa, b2 = bb, c2 = cc, d2 = dd;
            if (a2==0 || b2==0 || c2==0 || d2==0) { a2+=0.5; b2+=0.5; c2+=0.5; d2+=0.5; }
            double r1 = a2/(a2+b2), r2 = c2/(c2+d2);
            double RR = (r2 > 0) ? r1/r2 : 0;
            double seL = sqrt(b2/(a2*(a2+b2)) + d2/(c2*(c2+d2)));
            double lnRR = log(RR);
            double n_total = aa + bb + cc + dd;
            double chi = 0;
            if (n_total > 0) {
                double E11 = (double)(aa+bb)*(aa+cc)/n_total;
                double E12 = (double)(aa+bb)*(bb+dd)/n_total;
                double E21 = (double)(cc+dd)*(aa+cc)/n_total;
                double E22 = (double)(cc+dd)*(bb+dd)/n_total;
                if (E11>0) chi += pow(fabs(aa-E11)-0.5, 2)/E11;
                if (E12>0) chi += pow(fabs(bb-E12)-0.5, 2)/E12;
                if (E21>0) chi += pow(fabs(cc-E21)-0.5, 2)/E21;
                if (E22>0) chi += pow(fabs(dd-E22)-0.5, 2)/E22;
            }
            double pp = 1.0 - Dist::chiSquareCDF(chi, 1);
            res[nFood] = {li, RR, exp(lnRR-1.96*seL), exp(lnRR+1.96*seL), aa, bb, cc, dd, pp};
            nFood++;
        }
        if (nFood == 0) { DM.notify("No food data", C_ERR); return; }
        for (int i = 0; i < nFood-1; i++)
            for (int j = i+1; j < nFood; j++)
                if (res[j].RR > res[i].RR) { FoodResult t = res[i]; res[i] = res[j]; res[j] = t; }
        d.fillScreen(C_BG);
        DM.header("Food Poisoning - Ranked");
        DM.footer("F:Forest", "Y:Top RR->Ans", "ESC:back");
        d.setTextColor(C_DIM, C_BG);
        d.drawString("Food RR    [95%CI]   p", 4, HDR_H + 2);
        for (int i = 0; i < nFood && HDR_H + 14 + i*11 < LCD_H - FTR_H - 4; i++) {
            char l[60];
            uint16_t col;
            if (res[i].RR > 2.0 && res[i].pVal < 0.05) col = C_ERR;
            else if (res[i].RR > 1.5)                  col = C_ACC2;
            else                                       col = C_FG;
            d.setTextColor(col, C_BG);
            snprintf(l, sizeof(l), "L%d %5.2f [%4.2f,%4.2f] %.3f",
                     res[i].li+1, res[i].RR, res[i].lo, res[i].hi, res[i].pVal);
            d.drawString(l, 4, HDR_H + 14 + i*11);
        }
        d.setTextColor(C_ERR, C_BG);
        d.drawString("Red: RR>2 & p<0.05 SUSPECT", 4, LCD_H - FTR_H - 14);
        K_ = nFood;
        for (int i = 0; i < nFood; i++) {
            sOR_[i] = res[i].RR;
            sLO_[i] = res[i].lo;
            sHI_[i] = res[i].hi;
            snprintf(strataLabel[i], 16, "L%d", res[i].li+1);
        }
        commonVal = 1.0; commonLO = 1.0; commonHI = 1.0;
        plotKind = 1;
        if (res[0].RR > 0) G.ans = res[0].RR;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                DM.notify("Top RR -> Ans", C_OK); return;
            }
            if (M5Cardputer.Keyboard.isKeyPressed('f') || M5Cardputer.Keyboard.isKeyPressed('F')) {
                drawForestPlot("Food Forest (RR)", true);
                d.fillScreen(C_BG);
                DM.header("Food Poisoning - Ranked");
                DM.footer("F:Forest", "Y:Top RR->Ans", "ESC:back");
                d.setTextColor(C_DIM, C_BG);
                d.drawString("Food RR    [95%CI]   p", 4, HDR_H + 2);
                for (int i = 0; i < nFood && HDR_H + 14 + i*11 < LCD_H - FTR_H - 4; i++) {
                    char l[60];
                    uint16_t col;
                    if (res[i].RR > 2.0 && res[i].pVal < 0.05) col = C_ERR;
                    else if (res[i].RR > 1.5)                  col = C_ACC2;
                    else                                       col = C_FG;
                    d.setTextColor(col, C_BG);
                    snprintf(l, sizeof(l), "L%d %5.2f [%4.2f,%4.2f] %.3f",
                             res[i].li+1, res[i].RR, res[i].lo, res[i].hi, res[i].pVal);
                    d.drawString(l, 4, HDR_H + 14 + i*11);
                }
                d.setTextColor(C_ERR, C_BG);
                d.drawString("Red: RR>2 & p<0.05 SUSPECT", 4, LCD_H - FTR_H - 14);
            }
        }
    }
    void showStratifiedResult(const char* title, const char* effLabel,
                               double chiMH, double pMH, double X2_BD, double p_BD) {
        int page = 0;
        auto& d = M5Cardputer.Display;
        while (true) {
            d.fillScreen(C_BG);
            char th[32]; snprintf(th, sizeof(th), "%s p%d/3", title, page+1);
            DM.header(th);
            DM.footer("L/R:page F:Forest", "Y:->Ans", "ESC:back");
            d.setTextColor(C_FG, C_BG);
            char l[60];
            if (page == 0) {
                d.setTextColor(C_ACC, C_BG);
                d.drawString("Per-stratum estimates:", 4, HDR_H + 2);
                d.setTextColor(C_FG, C_BG);
                d.drawString("Lbl   Est   [LO,  HI]", 4, HDR_H + 14);
                for (int k = 0; k < K_ && HDR_H + 26 + k*10 < LCD_H - FTR_H - 4; k++) {
                    bool sig = (sLO_[k] > 1.0);
                    bool high = (sOR_[k] > 2.0);
                    if (sig && high)      d.setTextColor(C_ERR, C_BG);
                    else if (sig)         d.setTextColor(C_ACC2, C_BG);
                    else if (sOR_[k] > 1) d.setTextColor(C_FG, C_BG);
                    else                  d.setTextColor(C_DIM, C_BG);
                    snprintf(l, sizeof(l), "%-4s %5.2f [%5.2f,%5.2f]",
                             strataLabel[k], sOR_[k], sLO_[k], sHI_[k]);
                    d.drawString(l, 4, HDR_H + 26 + k*10);
                }
            } else if (page == 1) {
                d.setTextColor(C_ACC, C_BG);
                d.drawString("Common (MH):", 4, HDR_H + 2);
                d.setTextColor(C_OK, C_BG);
                snprintf(l, sizeof(l), "%s = %.4g", effLabel, commonVal);
                d.drawString(l, 4, HDR_H + 18);
                snprintf(l, sizeof(l), "95%% CI [%.3g, %.3g]", commonLO, commonHI);
                d.drawString(l, 4, HDR_H + 32);
                d.setTextColor(C_FG, C_BG);
                snprintf(l, sizeof(l), "MH chi^2 = %.3g", chiMH);
                d.drawString(l, 4, HDR_H + 50);
                snprintf(l, sizeof(l), "p = %.5g", pMH);
                d.drawString(l, 4, HDR_H + 62);
                if (pMH < 0.05) { d.setTextColor(C_OK, C_BG); d.drawString("* significant", 4, HDR_H + 74); }
            } else {
                d.setTextColor(C_ACC, C_BG);
                d.drawString("Heterogeneity (Breslow-Day):", 4, HDR_H + 2);
                d.setTextColor(C_FG, C_BG);
                snprintf(l, sizeof(l), "X^2 = %.3g  df = %d", X2_BD, K_ - 1);
                d.drawString(l, 4, HDR_H + 18);
                snprintf(l, sizeof(l), "p = %.5g", p_BD);
                d.drawString(l, 4, HDR_H + 32);
                if (p_BD < 0.05) {
                    d.setTextColor(C_ERR, C_BG);
                    d.drawString("HETEROGENEITY DETECTED!", 4, HDR_H + 50);
                    d.drawString("Effects differ across", 4, HDR_H + 62);
                    d.drawString("strata.", 4, HDR_H + 74);
                } else {
                    d.setTextColor(C_OK, C_BG);
                    d.drawString("Homogeneous (OK to pool)", 4, HDR_H + 50);
                }
            }
            G.ans = commonVal; G.ansComplex = false;
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 2) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                    DM.notify("Common -> Ans", C_OK); return;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('f') || M5Cardputer.Keyboard.isKeyPressed('F')) {
                    drawForestPlot(title, true);
                    break;
                }
            }
        }
    }
    void drawForestPlot(const char* title, bool showCommon) {
        auto& d = M5Cardputer.Display;
        double logMin = 1e30, logMax = -1e30;
        for (int k = 0; k < K_; k++) {
            if (sLO_[k] > 0) { double l = log(sLO_[k]); if (l < logMin) logMin = l; }
            if (sHI_[k] > 0) { double h = log(sHI_[k]); if (h > logMax) logMax = h; }
        }
        if (showCommon && commonLO > 0 && commonHI > 0) {
            double l = log(commonLO), h = log(commonHI);
            if (l < logMin) logMin = l;
            if (h > logMax) logMax = h;
        }
        if (0 < logMin) logMin = 0;
        if (0 > logMax) logMax = 0;
        double margin = (logMax - logMin) * 0.1;
        if (margin < 0.1) margin = 0.1;
        logMin -= margin; logMax += margin;
        d.fillScreen(C_BG);
        DM.header(title);
        DM.footer("S:BMP", "", "ESC:back");
        int xLab = 4;
        int xPlot0 = 36;
        int xPlot1 = LCD_W - 4;
        int gw = xPlot1 - xPlot0;
        int yTop = HDR_H + 2;
        int yBot = LCD_H - FTR_H - 14;
        int gh = yBot - yTop;
        int rows = K_ + (showCommon ? 1 : 0);
        int rh = gh / (rows + 1);
        if (rh < 6) rh = 6;
        if (rh > 14) rh = 14;
        int xRef = xPlot0 + (int)((0 - logMin)/(logMax - logMin) * gw);
        for (int yy = yTop; yy < yBot; yy += 4) {
            d.drawPixel(xRef, yy,     C_DIM);
            d.drawPixel(xRef, yy + 1, C_DIM);
        }
        for (int k = 0; k < K_; k++) {
            int y = yTop + (k + 1) * rh - rh/2;
            d.setTextColor(C_FG, C_BG);
            d.drawString(strataLabel[k], xLab, y - 3);
            if (sLO_[k] > 0 && sHI_[k] > 0) {
                int xL = xPlot0 + (int)((log(sLO_[k]) - logMin)/(logMax - logMin) * gw);
                int xH = xPlot0 + (int)((log(sHI_[k]) - logMin)/(logMax - logMin) * gw);
                if (xL < xPlot0) xL = xPlot0;
                if (xH > xPlot1) xH = xPlot1;
                d.drawLine(xL, y, xH, y, C_ACC);
                d.drawLine(xL, y - 2, xL, y + 2, C_ACC);
                d.drawLine(xH, y - 2, xH, y + 2, C_ACC);
                int xC = xPlot0 + (int)((log(sOR_[k]) - logMin)/(logMax - logMin) * gw);
                d.fillRect(xC - 2, y - 2, 5, 5, C_OK);
            }
        }
        if (showCommon && commonLO > 0 && commonHI > 0) {
            int y = yTop + (K_ + 1) * rh - rh/2;
            d.setTextColor(C_ACC2, C_BG);
            d.drawString("Pool", xLab, y - 3);
            int xL = xPlot0 + (int)((log(commonLO) - logMin)/(logMax - logMin) * gw);
            int xH = xPlot0 + (int)((log(commonHI) - logMin)/(logMax - logMin) * gw);
            int xC = xPlot0 + (int)((log(commonVal) - logMin)/(logMax - logMin) * gw);
            d.fillTriangle(xL, y, xC, y - 4, xC, y, C_ACC2);
            d.fillTriangle(xH, y, xC, y - 4, xC, y, C_ACC2);
            d.fillTriangle(xL, y, xC, y + 4, xC, y, C_ACC2);
            d.fillTriangle(xH, y, xC, y + 4, xC, y, C_ACC2);
        }
        d.setTextColor(C_DIM, C_BG);
        char ll[16];
        snprintf(ll, sizeof(ll), "%.2g", exp(logMin));
        d.drawString(ll, xPlot0 - 12, yBot + 1);
        d.drawString("1", xRef - 2, yBot + 1);
        snprintf(ll, sizeof(ll), "%.2g", exp(logMax));
        d.drawString(ll, xPlot1 - 18, yBot + 1);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("forest");
        }
    }
    void cochranQ() {
        char buf[8] = "5";
        if (!DM.inputLine("# studies:", buf, sizeof(buf))) return;
        int K = atoi(buf);
        if (K < 2 || K > 50) { DM.notify("2..50", C_ERR); return; }
        double y[50], v[50];
        for (int i = 0; i < K; i++) {
            char p[16]; snprintf(p, sizeof(p), "y[%d] (effect):", i+1);
            char b[16] = "0"; if (!DM.inputLine(p, b, sizeof(b))) return; y[i] = atof(b);
            snprintf(p, sizeof(p), "var[%d]:", i+1);
            strcpy(b, "1"); if (!DM.inputLine(p, b, sizeof(b))) return; v[i] = atof(b);
            if (v[i] <= 0) { DM.notify("var > 0 required", C_ERR); return; }
        }
        double sw = 0, swy = 0;
        for (int i = 0; i < K; i++) { sw += 1.0/v[i]; swy += y[i]/v[i]; }
        double mu = swy / sw;
        double Q = 0;
        for (int i = 0; i < K; i++) Q += (y[i] - mu)*(y[i] - mu)/v[i];
        int df = K - 1;
        double pp = 1.0 - Dist::chiSquareCDF(Q, df);
        double I2 = (Q > df) ? 100.0*(Q - df)/Q : 0;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Cochran Q"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"Q  = %.4g  df=%d", Q, df); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"p  = %.5g", pp);            d.drawString(l, 4, HDR_H+22);
        snprintf(l,sizeof(l),"I2 = %.2f%%", I2);          d.setTextColor(C_OK, C_BG); d.drawString(l, 4, HDR_H+38);
        DM.waitEsc();
    }
    void breslowDay() {
        if (strataInput(true) == 0) return;
        double mhLO, mhHI, chiMH, pMH;
        commonVal = computeMHOR(mhLO, mhHI, chiMH, pMH);
        commonLO = mhLO; commonHI = mhHI;
        double X2, pBD;
        computeBreslowDay(commonVal, X2, pBD);
        auto& dsp = M5Cardputer.Display;
        dsp.fillScreen(C_BG); DM.header("Breslow-Day"); DM.footer("","","ESC:back");
        dsp.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"Common OR = %.4g", commonVal); dsp.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"X^2 = %.4g  df=%d", X2, K_-1); dsp.drawString(l, 4, HDR_H+22);
        snprintf(l,sizeof(l),"p   = %.5g", pBD);
        if (pBD < 0.05) dsp.setTextColor(C_ERR, C_BG); else dsp.setTextColor(C_OK, C_BG);
        dsp.drawString(l, 4, HDR_H+36);
        dsp.setTextColor(C_DIM, C_BG);
        if (pBD < 0.05) dsp.drawString("Heterogeneous!", 4, HDR_H + 52);
        else            dsp.drawString("Homogeneous (OK pool)", 4, HDR_H + 52);
        DM.waitEsc();
    }
};

double StatStrata::a_[10], StatStrata::b_[10], StatStrata::c_[10], StatStrata::d_[10];
double StatStrata::sOR_[10], StatStrata::sLO_[10], StatStrata::sHI_[10];
double StatStrata::sRR_[10], StatStrata::sRRLO_[10], StatStrata::sRRHI_[10];
char   StatStrata::strataLabel[10][16];
int    StatStrata::K_ = 0;
double StatStrata::commonVal, StatStrata::commonLO, StatStrata::commonHI;
int    StatStrata::plotKind = 0;

class SurvivalAnalysis {
public:
    void run() {
        const char* items[] = {
            "Kaplan-Meier (L1=time L2=event)","Log-rank test (2 groups)",
            "Cox PH (1 covariate)","Median survival","Back"
        };
        int s = DM.menu("Survival", items, 5);
        if (s < 0 || s == 4) return;
        switch (s) {
            case 0: kaplanMeier(); break;
            case 1: logRank();     break;
            case 2: coxPH();       break;
            case 3: medianSurv();  break;
        }
    }
private:
    void kaplanMeier() {
        int n = G.stat[0].count;
        if (n < 2 || G.stat[1].count != n) { DM.notify("L1=time L2=event(0/1)", C_ERR); return; }
        double* T = (double*)malloc(n*sizeof(double));
        int*    E = (int*)   malloc(n*sizeof(int));
        for (int i = 0; i < n; i++) { T[i] = G.stat[0].data[i]; E[i] = (G.stat[1].data[i] != 0) ? 1 : 0; }
        for (int i = 1; i < n; i++) {
            double k = T[i]; int ek = E[i]; int j = i - 1;
            while (j >= 0 && T[j] > k) { T[j+1]=T[j]; E[j+1]=E[j]; j--; }
            T[j+1] = k; E[j+1] = ek;
        }
        double S = 1.0;
        int risk = n;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Kaplan-Meier"); DM.footer("S:BMP","","ESC:back");
        int gx = 4, gy = HDR_H + 10, gw = LCD_W - 10, gh = LCD_H - HDR_H - FTR_H - 14;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        double tMax = T[n-1];
        int prevX = gx, prevY = gy;
        int i = 0;
        while (i < n) {
            int j = i; while (j+1 < n && T[j+1] == T[i]) j++;
            int events = 0, censor = 0;
            for (int k = i; k <= j; k++) { if (E[k]) events++; else censor++; }
            if (events > 0) S *= (double)(risk - events) / risk;
            int sx = gx + (int)(T[i]/tMax * (gw - 4));
            int sy = gy + gh - 2 - (int)(S * (gh - 4));
            d.drawLine(prevX, prevY, sx, prevY, C_ACC);
            d.drawLine(sx,    prevY, sx, sy,    C_ACC);
            for (int k = i; k <= j; k++) if (!E[k]) d.drawFastVLine(sx, sy-2, 4, C_ACC2);
            prevX = sx; prevY = sy;
            risk -= events + censor;
            i = j + 1;
        }
        d.drawLine(prevX, prevY, gx + gw - 2, prevY, C_ACC);
        char lab[24];
        d.setTextColor(C_DIM, C_BG);
        snprintf(lab, sizeof(lab), "T_max=%.3g", tMax); d.drawString(lab, 4, HDR_H);
        snprintf(lab, sizeof(lab), "S_min=%.3f", S);    d.drawString(lab, LCD_W - 80, HDR_H);
        free(T); free(E);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("km");
        }
    }
    void logRank() {
        int n = G.stat[0].count;
        if (n < 2 || G.stat[1].count != n || G.stat[2].count != n) { DM.notify("L1=t L2=ev L3=grp", C_ERR); return; }
        int O1 = 0; double E1 = 0, V = 0;
        double times[SURVIVAL_MAX]; int nT = 0;
        for (int i = 0; i < n; i++) if (G.stat[1].data[i] != 0) {
            bool dup = false;
            for (int k = 0; k < nT; k++) if (times[k] == G.stat[0].data[i]) { dup = true; break; }
            if (!dup && nT < SURVIVAL_MAX) times[nT++] = G.stat[0].data[i];
        }
        for (int i = 1; i < nT; i++) {
            double k = times[i]; int j = i-1;
            while (j >= 0 && times[j] > k) { times[j+1]=times[j]; j--; }
            times[j+1] = k;
        }
        for (int t = 0; t < nT; t++) {
            double tau = times[t];
            int n1 = 0, n2 = 0, d1 = 0, d2 = 0;
            for (int i = 0; i < n; i++) {
                if (G.stat[0].data[i] >= tau) {
                    if (G.stat[2].data[i] == 0) n1++; else n2++;
                }
                if (G.stat[0].data[i] == tau && G.stat[1].data[i] != 0) {
                    if (G.stat[2].data[i] == 0) d1++; else d2++;
                }
            }
            int dT = d1 + d2, nT_ = n1 + n2;
            if (nT_ <= 1) continue;
            double e1 = (double)dT * n1 / nT_;
            double v  = (double)dT * (nT_ - dT) * n1 * n2 / ((double)nT_ * nT_ * (nT_ - 1));
            O1 += d1; E1 += e1; V += v;
        }
        double chi = (O1 - E1) * (O1 - E1) / V;
        double pp = 1.0 - Dist::chiSquareCDF(chi, 1);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Log-rank Test"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"Chi^2 = %.4g", chi); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"df = 1");            d.drawString(l, 4, HDR_H+22);
        snprintf(l,sizeof(l),"p  = %.5g", pp);     d.setTextColor(C_OK, C_BG); d.drawString(l, 4, HDR_H+36);
        DM.waitEsc();
    }
    void coxPH() {
        int n = G.stat[0].count;
        if (n < 3 || G.stat[1].count != n || G.stat[2].count != n) { DM.notify("L1=t L2=ev L3=x", C_ERR); return; }
        double beta = 0;
        for (int it = 0; it < 50; it++) {
            double U = 0, I = 0;
            for (int i = 0; i < n; i++) {
                if (G.stat[1].data[i] == 0) continue;
                double ti = G.stat[0].data[i];
                double sum1 = 0, sumX = 0, sumXX = 0;
                for (int j = 0; j < n; j++)
                    if (G.stat[0].data[j] >= ti) {
                        double e = exp(beta * G.stat[2].data[j]);
                        sum1  += e; sumX  += G.stat[2].data[j] * e;
                        sumXX += G.stat[2].data[j] * G.stat[2].data[j] * e;
                    }
                double meanX = sumX / sum1;
                U += G.stat[2].data[i] - meanX;
                I += sumXX/sum1 - meanX*meanX;
            }
            if (fabs(I) < 1e-12) break;
            double db = U / I;
            beta += db;
            if (fabs(db) < 1e-8) break;
        }
        double I = 0;
        for (int i = 0; i < n; i++) {
            if (G.stat[1].data[i] == 0) continue;
            double ti = G.stat[0].data[i];
            double sum1 = 0, sumX = 0, sumXX = 0;
            for (int j = 0; j < n; j++)
                if (G.stat[0].data[j] >= ti) {
                    double e = exp(beta * G.stat[2].data[j]);
                    sum1  += e; sumX  += G.stat[2].data[j] * e;
                    sumXX += G.stat[2].data[j] * G.stat[2].data[j] * e;
                }
            double mX = sumX/sum1;
            I += sumXX/sum1 - mX*mX;
        }
        double SE = (I > 0) ? 1.0/sqrt(I) : 0;
        double HR = exp(beta);
        double lo = exp(beta - 1.96*SE), hi = exp(beta + 1.96*SE);
        double z = beta / SE;
        double p = 2.0 * (1.0 - Dist::normalCDF(fabs(z)));
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Cox PH"); DM.footer("Y:HR->Ans","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"beta = %.5g  SE=%.4g", beta, SE); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"HR   = %.5g", HR);                d.setTextColor(C_OK, C_BG); d.drawString(l, 4, HDR_H+22);
        snprintf(l,sizeof(l),"95%% CI [%.4g, %.4g]", lo, hi);   d.drawString(l, 4, HDR_H+36);
        d.setTextColor(C_FG, C_BG);
        snprintf(l,sizeof(l),"z = %.3g  p = %.5g", z, p);        d.drawString(l, 4, HDR_H+52);
        G.ans = HR;
        DM.waitEsc();
    }
    void medianSurv() {
        int n = G.stat[0].count;
        if (n < 2 || G.stat[1].count != n) { DM.notify("L1=t L2=ev", C_ERR); return; }
        double* T = (double*)malloc(n*sizeof(double));
        int*    E = (int*)   malloc(n*sizeof(int));
        for (int i = 0; i < n; i++) { T[i] = G.stat[0].data[i]; E[i] = (G.stat[1].data[i] != 0) ? 1 : 0; }
        for (int i = 1; i < n; i++) {
            double k = T[i]; int ek = E[i]; int j = i - 1;
            while (j >= 0 && T[j] > k) { T[j+1]=T[j]; E[j+1]=E[j]; j--; }
            T[j+1] = k; E[j+1] = ek;
        }
        double S = 1; int risk = n;
        double medT = NAN;
        int i = 0;
        while (i < n) {
            int j = i; while (j+1 < n && T[j+1] == T[i]) j++;
            int events = 0, cens = 0;
            for (int k = i; k <= j; k++) { if (E[k]) events++; else cens++; }
            if (events > 0) S *= (double)(risk - events) / risk;
            if (S <= 0.5 && isnan(medT)) medT = T[i];
            risk -= events + cens;
            i = j + 1;
        }
        free(T); free(E);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Median Survival"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40];
        if (isnan(medT)) d.drawString("Not reached (S>0.5 throughout)", 4, HDR_H+12);
        else { snprintf(l,sizeof(l),"Median time = %.4g", medT); d.drawString(l, 4, HDR_H+12); }
        DM.waitEsc();
    }
};

class MetaAnalysis {
public:
    void run() {
        const char* items[] = {
            "Inverse variance (FE/RE)","DerSimonian-Laird (RE)","Forest plot",
            "Funnel plot","Egger's test","Back"
        };
        int s = DM.menu("Meta-Analysis", items, 6);
        if (s < 0 || s == 5) return;
        int K = inputData();
        if (K < 2) return;
        switch (s) {
            case 0: invVar(K); break;
            case 1: dlRE(K);   break;
            case 2: forest(K); break;
            case 3: funnel(K); break;
            case 4: egger(K);  break;
        }
    }
private:
    double y[META_MAX], v[META_MAX], se[META_MAX];
    char   label[META_MAX][16];
    int inputData() {
        char buf[8] = "5";
        if (!DM.inputLine("# studies (2-50):", buf, sizeof(buf))) return 0;
        int K = atoi(buf);
        if (K < 2 || K > META_MAX) { DM.notify("2..50", C_ERR); return 0; }
        for (int i = 0; i < K; i++) {
            char p[16]; snprintf(p, sizeof(p), "y[%d]:", i+1);
            char b[16] = "0";
            if (!DM.inputLine(p, b, sizeof(b))) return 0; y[i] = atof(b);
            snprintf(p, sizeof(p), "var[%d]:", i+1); strcpy(b, "1");
            if (!DM.inputLine(p, b, sizeof(b))) return 0; v[i] = atof(b);
            if (v[i] <= 0) { DM.notify("var > 0 required", C_ERR); return 0; }
            se[i] = sqrt(v[i]);
            snprintf(label[i], sizeof(label[i]), "S%d", i+1);
        }
        return K;
    }
    double sumW2(int K) {
        double s = 0;
        for (int i = 0; i < K; i++) s += (1.0/v[i])*(1.0/v[i]);
        return s;
    }
    void invVar(int K) {
        double sw = 0, swy = 0;
        for (int i = 0; i < K; i++) { sw += 1.0/v[i]; swy += y[i]/v[i]; }
        double mu = swy / sw;
        double seFE = sqrt(1.0/sw);
        double Q = 0;
        for (int i = 0; i < K; i++) Q += (y[i] - mu)*(y[i] - mu)/v[i];
        int df = K - 1;
        double tau2 = (Q > df) ? (Q - df) / (sw - (sumW2(K) / sw)) : 0;
        double I2 = (Q > df) ? 100.0*(Q - df)/Q : 0;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Meta IV (FE)"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"mu = %.5g", mu); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"95%% CI [%.4g, %.4g]", mu - 1.96*seFE, mu + 1.96*seFE); d.drawString(l, 4, HDR_H+22);
        d.setTextColor(C_FG, C_BG);
        snprintf(l,sizeof(l),"Q = %.4g  df=%d", Q, df); d.drawString(l, 4, HDR_H+38);
        snprintf(l,sizeof(l),"I^2 = %.2f%%", I2);        d.drawString(l, 4, HDR_H+52);
        snprintf(l,sizeof(l),"tau^2 = %.4g", tau2);      d.drawString(l, 4, HDR_H+66);
        DM.waitEsc();
    }
    void dlRE(int K) {
        double sw = 0, swy = 0;
        for (int i = 0; i < K; i++) { sw += 1.0/v[i]; swy += y[i]/v[i]; }
        double muFE = swy / sw;
        double Q = 0;
        for (int i = 0; i < K; i++) Q += (y[i] - muFE)*(y[i] - muFE)/v[i];
        int df = K - 1;
        double tau2 = (Q > df) ? (Q - df) / (sw - sumW2(K)/sw) : 0;
        double swR = 0, swyR = 0;
        for (int i = 0; i < K; i++) {
            double w = 1.0/(v[i] + tau2);
            swR += w; swyR += w*y[i];
        }
        double muRE = swyR / swR;
        double seRE = sqrt(1.0/swR);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Meta DL (RE)"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"mu_RE = %.5g", muRE); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"95%% CI [%.4g, %.4g]", muRE - 1.96*seRE, muRE + 1.96*seRE); d.drawString(l, 4, HDR_H+22);
        d.setTextColor(C_FG, C_BG);
        snprintf(l,sizeof(l),"tau^2 = %.4g  Q=%.3g", tau2, Q); d.drawString(l, 4, HDR_H+38);
        DM.waitEsc();
    }
    void forest(int K) {
        double sw = 0, swy = 0;
        for (int i = 0; i < K; i++) { sw += 1.0/v[i]; swy += y[i]/v[i]; }
        double mu = swy / sw;
        double mn = y[0]-1.96*se[0], mx = y[0]+1.96*se[0];
        for (int i = 0; i < K; i++) {
            double lo = y[i]-1.96*se[i], hi = y[i]+1.96*se[i];
            if (lo < mn) mn = lo; if (hi > mx) mx = hi;
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Forest Plot"); DM.footer("S:BMP","","ESC:back");
        int x0 = 30, x1 = LCD_W - 6, gw = x1 - x0;
        d.drawFastVLine(x0 + (int)((0 - mn)/(mx - mn) * gw), HDR_H, LCD_H - HDR_H - FTR_H, C_DIM);
        for (int i = 0; i < K; i++) {
            int yy = HDR_H + 4 + i * ((LCD_H - HDR_H - FTR_H - 16) / K);
            int xC = x0 + (int)((y[i] - mn)/(mx - mn) * gw);
            int xL = x0 + (int)((y[i]-1.96*se[i] - mn)/(mx - mn) * gw);
            int xH = x0 + (int)((y[i]+1.96*se[i] - mn)/(mx - mn) * gw);
            d.drawLine(xL, yy, xH, yy, C_ACC);
            d.fillRect(xC-2, yy-2, 4, 4, C_OK);
            d.setTextColor(C_DIM, C_BG);
            d.drawString(label[i], 2, yy - 3);
        }
        int yy = LCD_H - FTR_H - 8;
        double seM = sqrt(1.0/sw);
        int xC = x0 + (int)((mu - mn)/(mx - mn) * gw);
        int xL = x0 + (int)((mu - 1.96*seM - mn)/(mx - mn) * gw);
        int xH = x0 + (int)((mu + 1.96*seM - mn)/(mx - mn) * gw);
        d.fillTriangle(xL, yy, xC, yy-3, xC, yy+3, C_ACC2);
        d.fillTriangle(xH, yy, xC, yy-3, xC, yy+3, C_ACC2);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("forest");
        }
    }
    void funnel(int K) {
        double xmin = y[0], xmax = y[0], smax = se[0];
        for (int i = 0; i < K; i++) {
            if (y[i] < xmin) xmin = y[i]; if (y[i] > xmax) xmax = y[i];
            if (se[i] > smax) smax = se[i];
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Funnel Plot"); DM.footer("S:BMP","","ESC:back");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        for (int i = 0; i < K; i++) {
            int px = gx + (int)((y[i] - xmin)/(xmax - xmin + 1e-12) * (gw - 4)) + 2;
            int py = gy + (int)(se[i] / smax * (gh - 4)) + 2;
            d.fillCircle(px, py, 2, C_ACC);
        }
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("funnel");
        }
    }
    void egger(int K) {
        if (K < 3) { DM.notify("Egger needs K>=3", C_ERR); return; }
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (int i = 0; i < K; i++) {
            double xx = 1.0/se[i], yy = y[i]/se[i];
            sx += xx; sy += yy; sxx += xx*xx; sxy += xx*yy;
        }
        double dn = K*sxx - sx*sx;
        double slope = (K*sxy - sx*sy) / dn;
        double intercept = (sy - slope*sx)/K;
        double yMean = sy/K;
        double sst = 0, sse = 0;
        for (int i = 0; i < K; i++) {
            double xx = 1.0/se[i], yy = y[i]/se[i];
            double yh = intercept + slope*xx;
            sst += (yy - yMean)*(yy - yMean);
            sse += (yy - yh)*(yy - yh);
        }
        double mse = sse/(K-2);
        double seB = sqrt(mse / dn * K);
        double t = intercept / seB;
        double p = 2.0 * (1.0 - Dist::tCDF(fabs(t), K-2));
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Egger's Test"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"intercept = %.4g", intercept); d.drawString(l, 4, HDR_H+8);
        snprintf(l,sizeof(l),"t = %.4g  df=%d", t, K-2);     d.drawString(l, 4, HDR_H+22);
        snprintf(l,sizeof(l),"p = %.5g", p);                  d.setTextColor(C_OK, C_BG); d.drawString(l, 4, HDR_H+36);
        d.setTextColor(C_DIM, C_BG);
        d.drawString("(p<0.05 -> publication bias)", 4, HDR_H+52);
        DM.waitEsc();
    }
};

class StatGraphs {
public:
    void run() {
        const char* items[] = {
            "Histogram","Box-and-Whisker","Scatter plot","Q-Q plot (vs Normal)",
            "Bar chart","Pie chart","Line chart","Stem-and-leaf","Pareto chart",
            "ECDF plot","Density (KDE) approx","Strip chart","Cumulative line",
            "Mean +/- SD plot","Residuals plot","P-P plot","Multi-line overlay","Back"
        };
        int s = DM.menu("Statistical Graphs", items, 18);
        if (s < 0 || s == 17) return;
        switch (s) {
            case 0:  histogram();        break;
            case 1:  boxplot();          break;
            case 2:  scatter();          break;
            case 3:  qqPlot();           break;
            case 4:  bar();              break;
            case 5:  pie();              break;
            case 6:  line();             break;
            case 7:  stemLeaf();         break;
            case 8:  pareto();           break;
            case 9:  ecdf();             break;
            case 10: kde();              break;
            case 11: strip();            break;
            case 12: cumulative();       break;
            case 13: meanSDPlot();       break;
            case 14: residualPlot();     break;
            case 15: ppPlot();           break;
            case 16: multiLine();        break;
        }
    }
private:
    int pickList(const char* title) {
        const char* it[] = {"L1","L2","L3","L4","L5","L6","L7","L8"};
        return DM.menu(title, it, STAT_LISTS);
    }
    void afterPlot(const char* prefix) {
        DM.footer("S:BMP","","ESC:back");
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (st.fn && IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto(prefix);
        }
    }
    void getRange(double* a, int n, double& mn, double& mx) {
        mn = a[0]; mx = a[0];
        for (int i = 1; i < n; i++) { if (a[i] < mn) mn = a[i]; if (a[i] > mx) mx = a[i]; }
    }
    void histogram() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count; if (n < 2) { DM.notify("n<2", C_ERR); return; }
        char b[8] = "10"; if (!DM.inputLine("bins:", b, sizeof(b))) return;
        int nb = atoi(b); if (nb < 2 || nb > 50) return;
        double mn, mx; getRange(G.stat[li].data, n, mn, mx);
        double w = (mx - mn) / nb; if (w == 0) w = 1;
        int* h = (int*)calloc(nb, sizeof(int));
        for (int i = 0; i < n; i++) {
            int bin = (int)((G.stat[li].data[i] - mn)/w);
            if (bin == nb) bin = nb - 1;
            if (bin >= 0 && bin < nb) h[bin]++;
        }
        int maxV = 0; for (int i = 0; i < nb; i++) if (h[i] > maxV) maxV = h[i];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Histogram");
        int gy0 = HDR_H + 4, gh = LCD_H - HDR_H - FTR_H - 8;
        int bw = (LCD_W - 8) / nb;
        for (int i = 0; i < nb; i++) {
            int hh = (maxV>0) ? (int)((double)h[i]/maxV * gh) : 0;
            d.fillRect(4 + i*bw, gy0 + gh - hh, bw - 1, hh, C_ACC);
        }
        free(h);
        afterPlot("hist");
    }
    void boxplot() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count; if (n < 4) { DM.notify("n<4", C_ERR); return; }
        double* tmp = (double*)malloc(n*sizeof(double));
        memcpy(tmp, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=tmp[i]; int j=i-1;
            while (j>=0 && tmp[j]>k){tmp[j+1]=tmp[j];j--;} tmp[j+1]=k; }
        auto pct = [&](double p) { double idx = p*(n-1); int lo=(int)floor(idx); double f=idx-lo;
            return tmp[lo] + f*(tmp[(lo+1<n)?lo+1:lo] - tmp[lo]); };
        double q1 = pct(0.25), med = pct(0.5), q3 = pct(0.75);
        double iqr = q3 - q1, w_l = q1 - 1.5*iqr, w_h = q3 + 1.5*iqr;
        double lw = tmp[0], uw = tmp[n-1];
        for (int i = 0; i < n; i++) if (tmp[i] >= w_l) { lw = tmp[i]; break; }
        for (int i = n-1; i >= 0; i--) if (tmp[i] <= w_h) { uw = tmp[i]; break; }
        double mn = tmp[0], mx = tmp[n-1];
        free(tmp);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Boxplot");
        int gx = 10, gw = LCD_W - 20, gy = LCD_H/2;
        auto sx = [&](double v) { return gx + (int)((v - mn)/(mx - mn) * gw); };
        d.drawLine(sx(lw), gy, sx(q1), gy, C_DIM);
        d.drawLine(sx(q3), gy, sx(uw), gy, C_DIM);
        d.drawRect(sx(q1), gy-12, sx(q3)-sx(q1), 24, C_ACC);
        d.drawLine(sx(med), gy-12, sx(med), gy+12, C_ACC2);
        d.drawLine(sx(lw), gy-6, sx(lw), gy+6, C_DIM);
        d.drawLine(sx(uw), gy-6, sx(uw), gy+6, C_DIM);
        afterPlot("box");
    }
    void scatter() {
        int xi = pickList("X?"); if (xi < 0) return;
        int yi = pickList("Y?"); if (yi < 0) return;
        int n = G.stat[xi].count;
        if (G.stat[yi].count != n) { DM.notify("Mismatch", C_ERR); return; }
        double xmn, xmx, ymn, ymx;
        getRange(G.stat[xi].data, n, xmn, xmx);
        getRange(G.stat[yi].data, n, ymn, ymx);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Scatter");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        for (int i = 0; i < n; i++) {
            int px = gx + (int)((G.stat[xi].data[i] - xmn)/(xmx - xmn + 1e-12) * (gw - 4)) + 2;
            int py = gy + gh - 2 - (int)((G.stat[yi].data[i] - ymn)/(ymx - ymn + 1e-12) * (gh - 4));
            d.fillCircle(px, py, 1, C_ACC);
        }
        afterPlot("scatter");
    }
    void qqPlot() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count; if (n < 4) { DM.notify("n<4", C_ERR); return; }
        double* tmp = (double*)malloc(n*sizeof(double));
        memcpy(tmp, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=tmp[i]; int j=i-1;
            while (j>=0 && tmp[j]>k){tmp[j+1]=tmp[j];j--;} tmp[j+1]=k; }
        double mn = tmp[0], mx = tmp[n-1];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Q-Q (Normal)");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        double zmn = -3, zmx = 3;
        for (int i = 0; i < n; i++) {
            double q = (i + 0.5) / n;
            double z = Dist::normalInv(q);
            int px = gx + (int)((z - zmn)/(zmx - zmn) * (gw - 4)) + 2;
            int py = gy + gh - 2 - (int)((tmp[i] - mn)/(mx - mn + 1e-12) * (gh - 4));
            d.fillCircle(px, py, 1, C_ACC);
        }
        d.drawLine(gx+2, gy+gh-2, gx+gw-2, gy+2, C_DIM);
        free(tmp);
        afterPlot("qq");
    }
    void bar() {
        int li = pickList("Values?"); if (li < 0) return;
        int n = G.stat[li].count;
        double mx = 0;
        for (int i = 0; i < n; i++) if (fabs(G.stat[li].data[i]) > mx) mx = fabs(G.stat[li].data[i]);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Bar Chart");
        int bw = (LCD_W - 8) / n;
        if (bw < 2) bw = 2;
        int gh = LCD_H - HDR_H - FTR_H - 8;
        for (int i = 0; i < n; i++) {
            int h = (int)((double)G.stat[li].data[i]/mx * gh);
            d.fillRect(4 + i*bw, HDR_H + 4 + gh - h, bw - 1, h, C_ACC);
        }
        afterPlot("bar");
    }
    void pie() {
        int li = pickList("Values?"); if (li < 0) return;
        int n = G.stat[li].count;
        double sum = 0;
        for (int i = 0; i < n; i++) sum += fabs(G.stat[li].data[i]);
        if (sum == 0) { DM.notify("All zero", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Pie Chart");
        int cx = LCD_W/2, cy = LCD_H/2, r = 50;
        double a0 = -M_PI/2;
        uint16_t cols[10] = {C_ACC, C_OK, C_ACC2, C_ERR, C_WARN, 0xF7BE, 0x07FF, 0xF81F, 0xFFE0, 0x07E0};
        for (int i = 0; i < n; i++) {
            double a1 = a0 + 2*M_PI*fabs(G.stat[li].data[i])/sum;
            for (double a = a0; a < a1; a += 0.05) {
                d.drawLine(cx, cy, cx + (int)(r*cos(a)), cy + (int)(r*sin(a)), cols[i % 10]);
            }
            a0 = a1;
        }
        afterPlot("pie");
    }
    void line() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double mn, mx; getRange(G.stat[li].data, n, mn, mx);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Line Chart");
        int gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        int prevX = -1, prevY = -1;
        for (int i = 0; i < n; i++) {
            int px = 4 + (int)((double)i/(n-1) * (gw - 1));
            int py = HDR_H + 4 + gh - 1 - (int)((G.stat[li].data[i] - mn)/(mx - mn + 1e-12) * (gh - 1));
            if (prevX >= 0) d.drawLine(prevX, prevY, px, py, C_ACC);
            prevX = px; prevY = py;
        }
        afterPlot("line");
    }
    void stemLeaf() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Stem-and-Leaf"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        for (int row = 0; row < (LCD_H - HDR_H - FTR_H)/11 - 1; row++) {
            int stem = row;
            char line[40] = "";
            int p = 0;
            char tmp[8]; snprintf(tmp, sizeof(tmp), "%2d|", stem);
            strcat(line, tmp);
            for (int i = 0; i < n; i++) {
                int v = (int)round(G.stat[li].data[i]);
                if (v/10 == stem) {
                    snprintf(tmp, sizeof(tmp), "%d", v % 10);
                    strncat(line, tmp, sizeof(line)-strlen(line)-1);
                    p++;
                }
            }
            if (p > 0) d.drawString(line, 4, HDR_H + 4 + row*11);
        }
        DM.waitEsc();
    }
    void pareto() {
        int li = pickList("Frequencies?"); if (li < 0) return;
        int n = G.stat[li].count;
        double* a = (double*)malloc(n*sizeof(double));
        memcpy(a, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=a[i]; int j=i-1;
            while (j>=0 && a[j]<k){a[j+1]=a[j];j--;} a[j+1]=k; }
        double sum = 0; for (int i = 0; i < n; i++) sum += a[i];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Pareto Chart");
        int bw = (LCD_W - 8) / n; if (bw < 2) bw = 2;
        int gh = LCD_H - HDR_H - FTR_H - 8;
        double cum = 0;
        int prevX = -1, prevY = -1;
        for (int i = 0; i < n; i++) {
            int h = (int)((double)a[i]/a[0] * gh);
            d.fillRect(4 + i*bw, HDR_H + 4 + gh - h, bw - 1, h, C_ACC);
            cum += a[i];
            int cy = HDR_H + 4 + gh - 1 - (int)(cum/sum * (gh - 1));
            int cx = 4 + i*bw + bw/2;
            if (prevX >= 0) d.drawLine(prevX, prevY, cx, cy, C_OK);
            prevX = cx; prevY = cy;
        }
        free(a);
        afterPlot("pareto");
    }
    void ecdf() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double* a = (double*)malloc(n*sizeof(double));
        memcpy(a, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=a[i]; int j=i-1;
            while (j>=0 && a[j]>k){a[j+1]=a[j];j--;} a[j+1]=k; }
        double mn = a[0], mx = a[n-1];
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("ECDF");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        int prevX = gx, prevY = gy + gh - 2;
        for (int i = 0; i < n; i++) {
            int px = gx + (int)((a[i] - mn)/(mx - mn + 1e-12) * (gw - 2));
            int py = gy + gh - 2 - (int)((double)(i+1)/n * (gh - 4));
            d.drawLine(prevX, prevY, px, prevY, C_ACC);
            d.drawLine(px, prevY, px, py, C_ACC);
            prevX = px; prevY = py;
        }
        free(a);
        afterPlot("ecdf");
    }
    void kde() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double mn, mx; getRange(G.stat[li].data, n, mn, mx);
        double sum = 0, sum2 = 0;
        for (int i = 0; i < n; i++) { sum += G.stat[li].data[i]; sum2 += G.stat[li].data[i]*G.stat[li].data[i]; }
        double mean = sum/n;
        double sd = sqrt((sum2 - n*mean*mean)/(n-1));
        double h = 1.06 * sd * pow((double)n, -0.2);
        if (h < 1e-6) h = 1;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("KDE");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        double maxF = 0;
        double* fv = (double*)malloc(gw*sizeof(double));
        for (int p = 0; p < gw; p++) {
            double x = mn + (mx - mn) * p / gw;
            double f = 0;
            for (int i = 0; i < n; i++) {
                double u = (x - G.stat[li].data[i]) / h;
                f += exp(-0.5*u*u);
            }
            f /= (n * h * sqrt(2*M_PI));
            fv[p] = f;
            if (f > maxF) maxF = f;
        }
        int prevX = -1, prevY = -1;
        for (int p = 0; p < gw; p++) {
            int py = gy + gh - 2 - (int)(fv[p]/maxF * (gh - 4));
            int px = gx + p;
            if (prevX >= 0) d.drawLine(prevX, prevY, px, py, C_ACC);
            prevX = px; prevY = py;
        }
        free(fv);
        afterPlot("kde");
    }
    void strip() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double mn, mx; getRange(G.stat[li].data, n, mn, mx);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Strip Chart");
        int gx = 4, gy = LCD_H/2, gw = LCD_W - 8;
        d.drawLine(gx, gy, gx+gw, gy, C_DIM);
        for (int i = 0; i < n; i++) {
            int px = gx + (int)((G.stat[li].data[i] - mn)/(mx - mn + 1e-12) * (gw - 2));
            int jit = (int)(((esp_random() % 1000)/1000.0 - 0.5) * 20);
            d.fillCircle(px, gy + jit, 1, C_ACC);
        }
        afterPlot("strip");
    }
    void cumulative() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double sum = 0; double mx = 0;
        for (int i = 0; i < n; i++) { sum += G.stat[li].data[i]; if (sum > mx) mx = sum; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Cumulative");
        int gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        sum = 0;
        int prevX = 4, prevY = HDR_H + 4 + gh - 1;
        for (int i = 0; i < n; i++) {
            sum += G.stat[li].data[i];
            int px = 4 + (int)((double)i/(n-1) * (gw - 1));
            int py = HDR_H + 4 + gh - 1 - (int)(sum/mx * (gh - 1));
            d.drawLine(prevX, prevY, px, py, C_ACC);
            prevX = px; prevY = py;
        }
        afterPlot("cum");
    }
    void meanSDPlot() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Mean +/- SD");
        double mns[STAT_LISTS], sds[STAT_LISTS];
        int valid = 0;
        for (int i = 0; i < STAT_LISTS; i++) {
            int n = G.stat[i].count;
            if (n < 2) continue;
            double sum = 0, sum2 = 0;
            for (int j = 0; j < n; j++) { sum += G.stat[i].data[j]; sum2 += G.stat[i].data[j]*G.stat[i].data[j]; }
            mns[valid] = sum/n;
            sds[valid] = sqrt((sum2 - n*mns[valid]*mns[valid])/(n-1));
            valid++;
        }
        if (valid == 0) { DM.notify("No data", C_ERR); return; }
        double mn = mns[0]-sds[0], mx = mns[0]+sds[0];
        for (int i = 0; i < valid; i++) {
            if (mns[i]-sds[i] < mn) mn = mns[i]-sds[i];
            if (mns[i]+sds[i] > mx) mx = mns[i]+sds[i];
        }
        int gh = LCD_H - HDR_H - FTR_H - 8;
        for (int i = 0; i < valid; i++) {
            int px = 20 + i * 30;
            int yC = HDR_H + 4 + gh - 1 - (int)((mns[i] - mn)/(mx - mn) * (gh - 1));
            int yL = HDR_H + 4 + gh - 1 - (int)((mns[i]-sds[i] - mn)/(mx - mn) * (gh - 1));
            int yH = HDR_H + 4 + gh - 1 - (int)((mns[i]+sds[i] - mn)/(mx - mn) * (gh - 1));
            d.drawLine(px, yL, px, yH, C_ACC);
            d.fillCircle(px, yC, 2, C_OK);
            d.drawLine(px-3, yL, px+3, yL, C_ACC);
            d.drawLine(px-3, yH, px+3, yH, C_ACC);
        }
        afterPlot("msd");
    }
    void residualPlot() {
        if (G.stat[0].count < 2 || G.stat[1].count != G.stat[0].count) { DM.notify("Need L1,L2", C_ERR); return; }
        scatter();
    }
    void ppPlot() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double* a = (double*)malloc(n*sizeof(double));
        memcpy(a, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=a[i]; int j=i-1;
            while (j>=0 && a[j]>k){a[j+1]=a[j];j--;} a[j+1]=k; }
        double sum = 0, sum2 = 0;
        for (int i = 0; i < n; i++) { sum += a[i]; sum2 += a[i]*a[i]; }
        double m = sum/n, sd = sqrt((sum2 - n*m*m)/(n-1));
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("P-P Plot");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        d.drawLine(gx, gy + gh, gx + gw, gy, C_DIM);
        for (int i = 0; i < n; i++) {
            double obs = (i + 0.5) / n;
            double exp = Dist::normalCDF((a[i] - m)/sd);
            int px = gx + (int)(exp * (gw - 2)) + 1;
            int py = gy + gh - 2 - (int)(obs * (gh - 4));
            d.fillCircle(px, py, 1, C_ACC);
        }
        free(a);
        afterPlot("pp");
    }
    void multiLine() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Multi-line");
        double mn = INFINITY, mx = -INFINITY;
        int maxN = 0;
        for (int i = 0; i < STAT_LISTS; i++) {
            int n = G.stat[i].count;
            if (n > maxN) maxN = n;
            for (int j = 0; j < n; j++) {
                if (G.stat[i].data[j] < mn) mn = G.stat[i].data[j];
                if (G.stat[i].data[j] > mx) mx = G.stat[i].data[j];
            }
        }
        if (maxN < 2) { DM.notify("No data", C_ERR); return; }
        uint16_t cols[6] = {C_ACC, C_OK, C_ACC2, C_ERR, C_WARN, 0xF81F};
        int gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        for (int li = 0; li < STAT_LISTS; li++) {
            int n = G.stat[li].count;
            if (n < 2) continue;
            int prevX = -1, prevY = -1;
            for (int i = 0; i < n; i++) {
                int px = 4 + (int)((double)i/(maxN-1) * (gw - 1));
                int py = HDR_H + 4 + gh - 1 - (int)((G.stat[li].data[i] - mn)/(mx - mn + 1e-12) * (gh - 1));
                if (prevX >= 0) d.drawLine(prevX, prevY, px, py, cols[li]);
                prevX = px; prevY = py;
            }
        }
        afterPlot("multi");
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 18 / 20 ─ μLisp (GC + map/filter/reduce/apply 追加)             ║
// ╚═══════════════════════════════════════════════════════════════════════╝

namespace ULisp {

enum CellType : uint8_t {
    T_NIL = 0, T_NUM, T_SYM, T_STR, T_CONS, T_LAMBDA, T_BUILTIN, T_BOOL_T
};

struct Cell;
typedef Cell* CellRef;
struct Env;

struct Cell {
    CellType type;
    bool     marked;
    Cell*    next;
    union {
        double  num;
        char*   sym;
        char*   str;
        struct { CellRef car, cdr; }      cons;
        struct { CellRef params, body; Env* env; } lambda;
        CellRef (*builtin)(CellRef args, Env* env);
    };
};

static Cell pool[ULISP_CELLS];
static int  poolUsed = 0;
static Cell* freeList = nullptr;
static bool  gcInProgress = false;
static CellRef nilC = nullptr;
static CellRef trueC = nullptr;

void gc();   // ← この1行を追加

inline CellRef alloc() {
    if (freeList) {
        CellRef c = freeList;
        freeList = c->next;
        c->next  = nullptr;
        c->marked = false;
        return c;
    }
    if (poolUsed >= ULISP_CELLS) return nullptr;
    return &pool[poolUsed++];
}
static char errMsg[64] = "";
inline void setErr(const char* m) { strncpy(errMsg, m, 63); errMsg[63] = 0; }

inline CellRef mkNum(double v) { CellRef c = alloc(); if (!c) return nilC; c->type = T_NUM; c->num = v; return c; }
inline CellRef mkSym(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("sym malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_SYM; c->sym = tmp;
    return c;
}
inline CellRef mkStr(const char* s) {
    CellRef c = alloc(); if (!c) return nilC;
    int n = strlen(s) + 1;
    char* tmp = (char*)malloc(n);
    if (!tmp) { c->type = T_NIL; setErr("str malloc fail"); return nilC; }
    strcpy(tmp, s);
    c->type = T_STR; c->str = tmp;
    return c;
}
inline CellRef mkCons(CellRef a, CellRef d) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_CONS; c->cons.car = a; c->cons.cdr = d; return c;
}
inline CellRef mkBuiltin(CellRef (*fn)(CellRef, Env*)) {
    CellRef c = alloc(); if (!c) return nilC;
    c->type = T_BUILTIN; c->builtin = fn; return c;
}
inline bool isNil(CellRef c) { return !c || c == nilC || c->type == T_NIL; }

struct EnvEntry { char* name; CellRef val; };
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
    CellRef lookup(const char* k) {
        for (int i = 0; i < n; i++) if (!strcmp(e[i].name, k)) return e[i].val;
        return parent ? parent->lookup(k) : nullptr;
    }
};

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
static const char* gPos;
static int parseDepth = 0;
struct DepthGuard {
    int* d;
    DepthGuard(int* p) : d(p) { (*d)++; }
    ~DepthGuard() { (*d)--; }
};

inline void skipWS() {
    while (*gPos) {
        if (*gPos == ';') { while (*gPos && *gPos != '\n') gPos++; }
        else if (isspace((unsigned char)*gPos)) gPos++;
        else break;
    }
}

CellRef parseExpr();
CellRef eval(CellRef expr, Env* env);
CellRef applyFn(CellRef fn, CellRef args, Env* env);
void gc();
CellRef mkNum(double v);

void markCell(CellRef c);
void markEnvChain(Env* env);

void markCell(CellRef c) {
    if (!c || c->marked) return;
    c->marked = true;
    switch (c->type) {
        case T_CONS:   markCell(c->cons.car); markCell(c->cons.cdr); break;
        case T_LAMBDA: markCell(c->lambda.params);
                       markCell(c->lambda.body);
                       markEnvChain(c->lambda.env);
                       break;
        default: break;
    }
}
void markEnvChain(Env* env) {
    while (env) {
        if (env->marked) return;
        env->marked = true;
        for (int i = 0; i < env->n; i++) markCell(env->e[i].val);
        env = env->parent;
    }
}
void sweepPool() {
    freeList = nullptr;
    for (int i = poolUsed - 1; i >= 0; i--) {
        if (!pool[i].marked) {
            if (pool[i].type == T_SYM && pool[i].sym) { free(pool[i].sym); pool[i].sym = nullptr; }
            if (pool[i].type == T_STR && pool[i].str) { free(pool[i].str); pool[i].str = nullptr; }
            pool[i].type = T_NIL;
            pool[i].next = freeList;
            freeList = &pool[i];
        }
    }
}
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

CellRef parseAtom() {
    skipWS();
    char buf[64]; int n = 0;
    while (*gPos && !isspace((unsigned char)*gPos) && *gPos != '(' && *gPos != ')' && n < 63) {
        buf[n++] = *gPos++;
    }
    buf[n] = 0;
    if (n == 0) return nilC;
    char* end;
    double v = strtod(buf, &end);
    if (end == buf + n) return mkNum(v);
    if (!strcmp(buf, "nil")) return nilC;
    if (!strcmp(buf, "#t") || !strcmp(buf, "t")) return trueC;
    return mkSym(buf);
}

CellRef parseList() {
    if (parseDepth > 200) { setErr("parse too deep"); return nilC; }
    DepthGuard guard(&parseDepth);
    skipWS();
    if (!*gPos) { setErr("unexpected EOF"); return nilC; }
    if (*gPos == ')') { gPos++; return nilC; }
    CellRef car = parseExpr();
    CellRef cdr = parseList();
    return mkCons(car, cdr);
}

CellRef parseString() {
    char buf[256]; int n = 0;
    gPos++;
    while (*gPos && *gPos != '"' && n < 255) {
        if (*gPos == '\\' && *(gPos+1)) { gPos++; buf[n++] = *gPos++; }
        else buf[n++] = *gPos++;
    }
    if (*gPos == '"') gPos++;
    buf[n] = 0;
    return mkStr(buf);
}

CellRef parseExpr() {
    if (parseDepth > 200) { setErr("parse too deep"); return nilC; }
    DepthGuard guard(&parseDepth);
    skipWS();
    if (!*gPos) return nilC;
    if (*gPos == '(') { gPos++; return parseList(); }
    if (*gPos == '\'') { gPos++; return mkCons(mkSym("quote"), mkCons(parseExpr(), nilC)); }
    if (*gPos == '"') return parseString();
    return parseAtom();
}

CellRef parseStr(const char* s) { gPos = s; return parseExpr(); }

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
double toD(CellRef c) { if (c && c->type == T_NUM) return c->num; return 0; }

CellRef bAdd(CellRef a, Env* e) { double s = 0; while (!isNil(a)) { s += toD(a->cons.car); a = a->cons.cdr; } return mkNum(s); }
CellRef bSub(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(0);
    double s = toD(a->cons.car); a = a->cons.cdr;
    if (isNil(a)) return mkNum(-s);
    while (!isNil(a)) { s -= toD(a->cons.car); a = a->cons.cdr; }
    return mkNum(s);
}
CellRef bMul(CellRef a, Env* e) { double s = 1; while (!isNil(a)) { s *= toD(a->cons.car); a = a->cons.cdr; } return mkNum(s); }
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
CellRef bMod(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    double d = toD(a->cons.cdr->cons.car);
    if (d == 0) { setErr("mod by zero"); return nilC; }
    return mkNum(fmod(toD(a->cons.car), d));
}
CellRef bLT(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bGT(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bLE(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bGE(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bEq(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef x = a->cons.car, y = a->cons.cdr->cons.car;
    if (!x || !y) return nilC;
    if (x->type != y->type) return nilC;
    if (x->type == T_NUM)  return (x->num == y->num) ? trueC : nilC;
    if (x->type == T_SYM)  return (!strcmp(x->sym, y->sym)) ? trueC : nilC;
    if (x->type == T_STR)  return (!strcmp(x->str, y->str)) ? trueC : nilC;
    return (x == y) ? trueC : nilC;
}
CellRef bCar(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC; }
CellRef bCdr(CellRef a, Env* e)  { if (isNil(a) || a->type != T_CONS) return nilC; CellRef l = a->cons.car; return (!isNil(l) && l->type == T_CONS) ? l->cons.cdr : nilC; }
CellRef bCons(CellRef a, Env* e) { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkCons(a->cons.car, a->cons.cdr->cons.car); }
CellRef bList(CellRef a, Env* e) { return a; }
CellRef bNullP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; return isNil(a->cons.car) ? trueC : nilC; }
CellRef bAtomP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }
CellRef bNumP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }
CellRef bSymP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }

CellRef bLength(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(0);
    int n = 0; CellRef l = a->cons.car;
    while (!isNil(l) && l->type == T_CONS) { n++; l = l->cons.cdr; }
    return mkNum(n);
}
CellRef bAppend(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef result = nilC, tail = nilC;
    CellRef cur = a;
    while (!isNil(cur) && cur->type == T_CONS) {
        CellRef item = cur->cons.car;
        bool isLast = isNil(cur->cons.cdr);
        if (isLast) {
            if (isNil(result)) return item;
            tail->cons.cdr = item;
            return result;
        }
        CellRef l = item;
        while (!isNil(l) && l->type == T_CONS) {
            CellRef nc = mkCons(l->cons.car, nilC);
            if (isNil(result)) { result = tail = nc; }
            else { tail->cons.cdr = nc; tail = nc; }
            l = l->cons.cdr;
        }
        cur = cur->cons.cdr;
    }
    return result;
}
CellRef bReverse(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef l = a->cons.car; CellRef r = nilC;
    while (!isNil(l) && l->type == T_CONS) { r = mkCons(l->cons.car, r); l = l->cons.cdr; }
    return r;
}
CellRef bNth(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    int n = (int)toD(a->cons.car);
    CellRef l = a->cons.cdr->cons.car;
    while (n > 0 && !isNil(l) && l->type == T_CONS) { l = l->cons.cdr; n--; }
    return (!isNil(l) && l->type == T_CONS) ? l->cons.car : nilC;
}

CellRef bSin(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(sin(toD(a->cons.car))); }
CellRef bCos(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(cos(toD(a->cons.car))); }
CellRef bTan(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(tan(toD(a->cons.car))); }
CellRef bExp(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(exp(toD(a->cons.car))); }
CellRef bLog(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(log(toD(a->cons.car))); }
CellRef bSqrt(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(sqrt(toD(a->cons.car))); }
CellRef bAbs(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(fabs(toD(a->cons.car))); }
CellRef bFloor(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(floor(toD(a->cons.car))); }
CellRef bCeil(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(ceil(toD(a->cons.car))); }
CellRef bRound(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } return mkNum(round(toD(a->cons.car))); }
CellRef bPow(CellRef a, Env* e)   { if (isNil(a) || isNil(a->cons.cdr)) return nilC; return mkNum(pow(toD(a->cons.car), toD(a->cons.cdr->cons.car))); }
CellRef bMin(CellRef a, Env* e)   {
    if (isNil(a)) return mkNum(0);
    double m = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a)) { double v = toD(a->cons.car); if (v < m) m = v; a = a->cons.cdr; }
    return mkNum(m);
}
CellRef bMax(CellRef a, Env* e) {
    if (isNil(a)) return mkNum(0);
    double m = toD(a->cons.car); a = a->cons.cdr;
    while (!isNil(a)) { double v = toD(a->cons.car); if (v > m) m = v; a = a->cons.cdr; }
    return mkNum(m);
}
CellRef bRand(CellRef a, Env* e)  { return mkNum((double)esp_random() / 4294967296.0); }

CellRef bMean(CellRef a, Env* e) {
    CellRef l = a->cons.car;
    double s = 0; int n = 0;
    while (!isNil(l) && l->type == T_CONS) { s += toD(l->cons.car); n++; l = l->cons.cdr; }
    return (n > 0) ? mkNum(s/n) : mkNum(0);
}
CellRef bSum(CellRef a, Env* e) {
    CellRef l = a->cons.car; double s = 0;
    while (!isNil(l) && l->type == T_CONS) { s += toD(l->cons.car); l = l->cons.cdr; }
    return mkNum(s);
}
CellRef bSD(CellRef a, Env* e) {
    CellRef l = a->cons.car;
    double s = 0, s2 = 0; int n = 0;
    while (!isNil(l) && l->type == T_CONS) { double v = toD(l->cons.car); s += v; s2 += v*v; n++; l = l->cons.cdr; }
    if (n < 2) return mkNum(0);
    double m = s/n;
    return mkNum(sqrt((s2 - n*m*m)/(n-1)));
}

void printCell(CellRef c, char* buf, int& p, int max);
CellRef bPrint(CellRef a, Env* e) {
    char buf[256]; int p = 0;
    printCell(a->cons.car, buf, p, 255);
    Serial.println(buf);
    return a->cons.car;
}

CellRef bSave(CellRef a, Env* e);
CellRef bLoad(CellRef a, Env* e);

void printCell(CellRef c, char* buf, int& p, int max) {
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
        putc1('(');
        bool first = true;
        while (!isNil(c) && c->type == T_CONS) {
            if (!first) putc1(' ');
            printCell(c->cons.car, buf, p, max);
            first = false;
            c = c->cons.cdr;
        }
        if (!isNil(c)) { put(" . "); printCell(c, buf, p, max); }
        putc1(')');
    }
}

CellRef applyFn(CellRef fn, CellRef args, Env* env) {
    if (!fn) return nilC;
    if (fn->type == T_BUILTIN) return fn->builtin(args, env);
    if (fn->type == T_LAMBDA) {
        Env* newEnv = allocEnv(fn->lambda.env);
        if (!newEnv) { setErr("Env limit"); return nilC; }
        CellRef p = fn->lambda.params;
        CellRef a = args;
        while (!isNil(p) && p->type == T_CONS) {
            if (isNil(a)) break;
            newEnv->set(p->cons.car->sym, a->cons.car);
            p = p->cons.cdr; a = a->cons.cdr;
        }
        CellRef body = fn->lambda.body;
        CellRef r = nilC;
        while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
        return r;
    }
    setErr("not callable");
    return nilC;
}

CellRef bMap(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;
    CellRef head = nilC, tail = nilC;
    while (!isNil(lst) && lst->type == T_CONS) {
        CellRef v = applyFn(fn, mkCons(lst->cons.car, nilC), e);
        CellRef nc = mkCons(v, nilC);
        if (isNil(head)) { head = tail = nc; }
        else { tail->cons.cdr = nc; tail = nc; }
        lst = lst->cons.cdr;
    }
    return head;
}
CellRef bFilter(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn  = a->cons.car;
    CellRef lst = a->cons.cdr->cons.car;
    CellRef head = nilC, tail = nilC;
    while (!isNil(lst) && lst->type == T_CONS) {
        CellRef v = applyFn(fn, mkCons(lst->cons.car, nilC), e);
        if (!isNil(v)) {
            CellRef nc = mkCons(lst->cons.car, nilC);
            if (isNil(head)) { head = tail = nc; }
            else { tail->cons.cdr = nc; tail = nc; }
        }
        lst = lst->cons.cdr;
    }
    return head;
}
CellRef bReduce(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr) || isNil(a->cons.cdr->cons.cdr)) return nilC;
    CellRef fn   = a->cons.car;
    CellRef acc  = a->cons.cdr->cons.car;
    CellRef lst  = a->cons.cdr->cons.cdr->cons.car;
    while (!isNil(lst) && lst->type == T_CONS) {
        acc = applyFn(fn, mkCons(acc, mkCons(lst->cons.car, nilC)), e);
        lst = lst->cons.cdr;
    }
    return acc;
}
CellRef bApply(CellRef a, Env* e) {
    if (isNil(a) || isNil(a->cons.cdr)) return nilC;
    CellRef fn   = a->cons.car;
    CellRef args = a->cons.cdr->cons.car;
    return applyFn(fn, args, e);
}
CellRef bGCCall(CellRef a, Env* e) { gc(); return mkNum((double)poolUsed); }

CellRef eval(CellRef expr, Env* env) {
    if (isNil(expr)) return nilC;
    if (expr->type == T_NUM)     return expr;
    if (expr->type == T_STR)     return expr;
    if (expr->type == T_BOOL_T)  return expr;
    if (expr->type == T_SYM) {
        CellRef v = env->lookup(expr->sym);
        if (!v) { setErr("unbound symbol"); return nilC; }
        return v;
    }
    if (expr->type != T_CONS) return expr;
    CellRef op = expr->cons.car;
    CellRef args = expr->cons.cdr;
    if (op && op->type == T_SYM) {
        const char* n = op->sym;
        if (!strcmp(n, "quote")) {
            if (isNil(args) || args->type != T_CONS) { setErr("quote: no arg"); return nilC; }
            return args->cons.car;
        }
        if (!strcmp(n, "if")) {
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
        }
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
        if (!strcmp(n, "and")) {
            CellRef r = trueC;
            while (!isNil(args)) { r = eval(args->cons.car, env); if (isNil(r)) return nilC; args = args->cons.cdr; }
            return r;
        }
        if (!strcmp(n, "or")) {
            while (!isNil(args)) { CellRef r = eval(args->cons.car, env); if (!isNil(r)) return r; args = args->cons.cdr; }
            return nilC;
        }
        if (!strcmp(n, "not")) {
            if (isNil(args) || args->type != T_CONS) return trueC;
            CellRef r = eval(args->cons.car, env);
            return isNil(r) ? trueC : nilC;
        }
        if (!strcmp(n, "define") || !strcmp(n, "setq")) {
            if (isNil(args) || args->type != T_CONS) { setErr("define: bad args"); return nilC; }
            CellRef target = args->cons.car;
            if (!target) { setErr("define: no target"); return nilC; }
            if (target->type == T_SYM) {
                if (isNil(args->cons.cdr)) { setErr("define: no value"); return nilC; }
                CellRef v = eval(args->cons.cdr->cons.car, env);
                if (!env->set(target->sym, v)) { setErr("env table full"); return nilC; }
                return v;
            }
            if (target->type != T_CONS || !target->cons.car || target->cons.car->type != T_SYM) {
                setErr("define: bad target"); return nilC;
            }
            const char* name = target->cons.car->sym;
            CellRef params = target->cons.cdr;
            CellRef body = args->cons.cdr;
            CellRef lam = alloc();
            if (!lam) return nilC;
            lam->type = T_LAMBDA;
            lam->lambda.params = params;
            lam->lambda.body = body;
            lam->lambda.env = env;
            env->set(name, lam);
            return lam;
        }
        if (!strcmp(n, "lambda")) {
            if (isNil(args) || args->type != T_CONS) { setErr("lambda: bad args"); return nilC; }
            CellRef lam = alloc();
            if (!lam) return nilC;
            lam->type = T_LAMBDA;
            lam->lambda.params = args->cons.car;
            lam->lambda.body = args->cons.cdr;
            lam->lambda.env = env;
            return lam;
        }
        if (!strcmp(n, "let")) {
            if (isNil(args) || args->type != T_CONS) { setErr("let: bad args"); return nilC; }
            CellRef bindings = args->cons.car;
            Env* newEnv = allocEnv(env);
            if (!newEnv) { setErr("Env limit"); return nilC; }
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
            CellRef body = args->cons.cdr;
            CellRef r = nilC;
            while (!isNil(body)) { r = eval(body->cons.car, newEnv); body = body->cons.cdr; }
            return r;
        }
        if (!strcmp(n, "while")) {
            if (isNil(args) || args->type != T_CONS) { setErr("while: bad args"); return nilC; }
            CellRef test = args->cons.car;
            CellRef body = args->cons.cdr;
            CellRef r = nilC;
            int safety = 100000;
            while (!isNil(eval(test, env))) {
                CellRef b = body;
                while (!isNil(b)) { r = eval(b->cons.car, env); b = b->cons.cdr; }
                if (--safety <= 0) { setErr("loop limit"); break; }
            }
            return r;
        }
        if (!strcmp(n, "begin") || !strcmp(n, "progn")) {
            CellRef r = nilC;
            while (!isNil(args)) { r = eval(args->cons.car, env); args = args->cons.cdr; }
            return r;
        }
    }
    CellRef fn = eval(op, env);
    CellRef ev_args = evalArgs(args, env);
    return applyFn(fn, ev_args, env);
}

CellRef bSave(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef arg0 = a->cons.car;
    if (!arg0 || arg0->type != T_STR) return nilC;
    char path[64] = "/";
    strncat(path, a->cons.car->str, 60);
    SdGuard guard;
    File f = SD.open(path, FILE_WRITE);
    if (!f) return nilC;
    if (isNil(a->cons.cdr) || a->cons.cdr->type != T_CONS) { f.close(); return nilC; }
    CellRef val = a->cons.cdr->cons.car;
    char buf[1024]; int p = 0;
    printCell(val, buf, p, 1023);
    f.print(buf);
    f.close();
    return trueC;
}
CellRef bLoad(CellRef a, Env* env) {
    if (!G.sdOK) return nilC;
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef arg0 = a->cons.car;
    if (!arg0 || arg0->type != T_STR) return nilC;
    char path[64] = "/";
    strncat(path, a->cons.car->str, 60);
    SdGuard guard;
    File f = SD.open(path);
    if (!f) return nilC;
    String content = "";
    while (f.available()) content += (char)f.read();
    f.close();
    return parseStr(content.c_str());
}

void initEnv() {
    nilC = &pool[poolUsed++]; nilC->type = T_NIL; nilC->marked = false; nilC->next = nullptr;
    trueC = &pool[poolUsed++]; trueC->type = T_BOOL_T; trueC->marked = false; trueC->next = nullptr;
    globalEnv.set("+", mkBuiltin(bAdd));
    globalEnv.set("-", mkBuiltin(bSub));
    globalEnv.set("*", mkBuiltin(bMul));
    globalEnv.set("/", mkBuiltin(bDiv));
    globalEnv.set("mod", mkBuiltin(bMod));
    globalEnv.set("<", mkBuiltin(bLT));
    globalEnv.set(">", mkBuiltin(bGT));
    globalEnv.set("<=", mkBuiltin(bLE));
    globalEnv.set(">=", mkBuiltin(bGE));
    globalEnv.set("=", mkBuiltin(bEq));
    globalEnv.set("equal?", mkBuiltin(bEq));
    globalEnv.set("car", mkBuiltin(bCar));
    globalEnv.set("cdr", mkBuiltin(bCdr));
    globalEnv.set("cons", mkBuiltin(bCons));
    globalEnv.set("list", mkBuiltin(bList));
    globalEnv.set("null?", mkBuiltin(bNullP));
    globalEnv.set("atom?", mkBuiltin(bAtomP));
    globalEnv.set("number?", mkBuiltin(bNumP));
    globalEnv.set("symbol?", mkBuiltin(bSymP));
    globalEnv.set("length", mkBuiltin(bLength));
    globalEnv.set("append", mkBuiltin(bAppend));
    globalEnv.set("reverse", mkBuiltin(bReverse));
    globalEnv.set("nth", mkBuiltin(bNth));
    globalEnv.set("sin", mkBuiltin(bSin));
    globalEnv.set("cos", mkBuiltin(bCos));
    globalEnv.set("tan", mkBuiltin(bTan));
    globalEnv.set("exp", mkBuiltin(bExp));
    globalEnv.set("log", mkBuiltin(bLog));
    globalEnv.set("sqrt", mkBuiltin(bSqrt));
    globalEnv.set("abs", mkBuiltin(bAbs));
    globalEnv.set("floor", mkBuiltin(bFloor));
    globalEnv.set("ceil", mkBuiltin(bCeil));
    globalEnv.set("round", mkBuiltin(bRound));
    globalEnv.set("expt", mkBuiltin(bPow));
    globalEnv.set("pow", mkBuiltin(bPow));
    globalEnv.set("min", mkBuiltin(bMin));
    globalEnv.set("max", mkBuiltin(bMax));
    globalEnv.set("random", mkBuiltin(bRand));
    globalEnv.set("mean", mkBuiltin(bMean));
    globalEnv.set("sum", mkBuiltin(bSum));
    globalEnv.set("sd", mkBuiltin(bSD));
    globalEnv.set("print", mkBuiltin(bPrint));
    globalEnv.set("save", mkBuiltin(bSave));
    globalEnv.set("load", mkBuiltin(bLoad));
    globalEnv.set("map", mkBuiltin(bMap));
    globalEnv.set("filter", mkBuiltin(bFilter));
    globalEnv.set("reduce", mkBuiltin(bReduce));
    globalEnv.set("apply", mkBuiltin(bApply));
    globalEnv.set("pi", mkNum(M_PI));
    globalEnv.set("e", mkNum(M_E));
}

void resetPool() {
    for (int i = 0; i < poolUsed; i++) {
        if (pool[i].type == T_SYM && pool[i].sym) { free(pool[i].sym); pool[i].sym = nullptr; }
        if (pool[i].type == T_STR && pool[i].str) { free(pool[i].str); pool[i].str = nullptr; }
    }
    poolUsed = 0;
    freeList = nullptr;
    for (int i = 0; i < globalEnv.n; i++) free(globalEnv.e[i].name);
    globalEnv.n = 0;
    for (int i = 0; i < envN; i++) {
        for (int j = 0; j < envList[i]->n; j++) free(envList[i]->e[j].name);
        delete envList[i];
    }
    envN = 0;
    initEnv();
}

} // namespace ULisp

class ULispMode {
public:
    void run() {
        if (ULisp::poolUsed == 0) ULisp::initEnv();
        const char* items[] = {
            "REPL (interactive)","Run script from SD","Run GC now",
            "Reset interpreter","Examples","Back"
        };
        int s = DM.menu("uLisp", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: repl();      break;
            case 1: runFile();   break;
            case 2: {
                int used = ULisp::poolUsed;
                ULisp::gc();
                char m[32]; snprintf(m, sizeof(m), "GC done (%d->%d)", used, ULisp::poolUsed);
                DM.notify(m, C_OK, 1200);
            } break;
            case 3:
                if (DM.confirm("Reset uLisp?")) { ULisp::resetPool(); DM.notify("Reset done", C_OK); }
                break;
            case 4: examples();  break;
        }
    }
private:
    void repl() {
        char input[80] = "";
        char history[6][80] = {{0}};
        int  histN = 0;
        auto& d = M5Cardputer.Display;
        while (true) {
            d.fillScreen(C_BG);
            DM.header("uLisp REPL");
            DM.footer("ENT:eval", "Fn+E:exit", "ESC:back");
            d.setTextColor(C_DIM, C_BG);
            int y = HDR_H + 2;
            for (int i = 0; i < histN && y < LCD_H - FTR_H - 24; i++) {
                d.drawString(history[i], 4, y); y += 11;
            }
            d.drawFastHLine(0, LCD_H - FTR_H - 17, LCD_W, C_ACC);
            d.fillRect(0, LCD_H - FTR_H - 16, LCD_W, 14, C_PANEL);
            d.setTextColor(C_FG, C_PANEL);
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
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.del) { if (cur > 0) input[--cur] = 0; break; }
                if (st.enter) {
                    if (input[0] == 0) break;
                    if (ULisp::envN > 100) ULisp::gc();
                    ULisp::errMsg[0] = 0;
                    ULisp::CellRef expr = ULisp::parseStr(input);
                    ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
                    char buf[80];
                    if (ULisp::errMsg[0]) {
                        snprintf(buf, sizeof(buf), "ERR: %s", ULisp::errMsg);
                    } else {
                        char tmp[78]; int p = 0;
                        ULisp::printCell(r, tmp, p, 77); tmp[p] = 0;
                        snprintf(buf, sizeof(buf), "%.30s -> %s", input, tmp);
                    }
                    if (histN < 6) strncpy(history[histN++], buf, 79);
                    else { for (int i = 1; i < 6; i++) strcpy(history[i-1], history[i]); strncpy(history[5], buf, 79); }
                    input[0] = 0;
                    break;
                }
                for (auto c : st.word) {
                    if (cur < 79 && c >= ' ' && c < 127) { input[cur++] = c; input[cur] = 0; }
                }
                if (!st.word.empty()) break;
            }
        }
    }
    void runFile() {
        if (!G.sdOK) { DM.notify("No SD", C_ERR); return; }
        char path[48] = "/script.lsp";
        if (!DM.inputLine("Path:", path, sizeof(path))) return;
        SdGuard guard;
        File f = SD.open(path);
        if (!f) { DM.notify("Not found", C_ERR); return; }
        String src = "";
        while (f.available()) src += (char)f.read();
        f.close();
        const char* p = src.c_str();
        ULisp::gPos = p;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Run Script"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        int y = HDR_H + 2;
        int evals = 0;
        while (true) {
            ULisp::skipWS();
            if (!*ULisp::gPos) break;
            ULisp::errMsg[0] = 0;
            ULisp::CellRef expr = ULisp::parseExpr();
            ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
            evals++;
            if (ULisp::errMsg[0]) {
                d.setTextColor(C_ERR, C_BG);
                char b[40]; snprintf(b, sizeof(b), "ERR: %s", ULisp::errMsg);
                d.drawString(b, 4, y); y += 11;
                d.setTextColor(C_FG, C_BG);
                if (y > LCD_H - FTR_H - 12) break;
            }
        }
        char b[40]; snprintf(b, sizeof(b), "%d expressions evaluated", evals);
        d.setTextColor(C_OK, C_BG);
        d.drawString(b, 4, y);
        DM.waitEsc();
    }
    void examples() {
        const char* items[] = {
            "Hello (+ 1 2 3)","Define square","Factorial recursion",
            "Map (squares)","Statistics demo","Back"
        };
        int s = DM.menu("Examples", items, 6);
        if (s < 0 || s == 5) return;
        const char* code = "";
        switch (s) {
            case 0: code = "(+ 1 2 3)"; break;
            case 1: code = "(define (sq x) (* x x))"; break;
            case 2: code = "(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))"; break;
            case 3: code = "(map (lambda (x) (* x x)) '(1 2 3 4 5))"; break;
            case 4: code = "(mean (list 1 2 3 4 5))"; break;
        }
        ULisp::errMsg[0] = 0;
        ULisp::CellRef expr = ULisp::parseStr(code);
        ULisp::CellRef r = ULisp::eval(expr, &ULisp::globalEnv);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Example"); DM.footer("","","ESC:back");
        d.setTextColor(C_DIM, C_BG); d.drawString(code, 4, HDR_H + 4);
        char tmp[80]; int p = 0;
        ULisp::printCell(r, tmp, p, 79); tmp[p] = 0;
        d.setTextColor(C_OK, C_BG);
        d.drawString("=>", 4, HDR_H + 22);
        d.drawString(tmp, 24, HDR_H + 22);
        if (ULisp::errMsg[0]) { d.setTextColor(C_ERR, C_BG); d.drawString(ULisp::errMsg, 4, HDR_H + 38); }
        DM.waitEsc();
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 19 / 20 ─ グラフエンジン + スプレッドシート                     ║
// ╚═══════════════════════════════════════════════════════════════════════╝

class GraphEngine {
public:
    static const int MAXF = 4;
    void run() {
        while (true) {
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
        }
    }
    char  expr[MAXF][48] = {"sin(x)", "", "", ""};
    bool  active[MAXF]   = {true, false, false, false};
    int   modeT = 0;
    double xmin = -6.28, xmax = 6.28, ymin = -2.5, ymax = 2.5;
    double zmin = -2, zmax = 2;
    double tmin = 0, tmax = 6.28, tstep = 0.05;
    int    seqStart = 1, seqEnd = 30;
private:
    int  pickType() {
        const char* items[] = {
            "y = f(x)","Parametric x(t),y(t)","Polar r=f(t)",
            "Implicit f(x,y)=0","Inequality f(x,y)>0","Sequence a_n","3D z=f(x,y)","Back"
        };
        int s = DM.menu("Plot Type", items, 8);
        if (s >= 0 && s < 7) modeT = s;
        return s;
    }
    void editFns() {
        const char* labels[MAXF];
        char buf[MAXF][56];
        for (int i = 0; i < MAXF; i++) {
            snprintf(buf[i], sizeof(buf[i]), "%c%d: %s",
                     active[i] ? '*' : ' ', i+1, expr[i][0] ? expr[i] : "<empty>");
            labels[i] = buf[i];
        }
        int s = DM.menu("Edit Fns", labels, MAXF);
        if (s < 0) return;
        char e[48];
        strncpy(e, expr[s], sizeof(e)); e[47] = 0;
        if (DM.inputLine("y =", e, sizeof(e))) {
            strncpy(expr[s], e, sizeof(expr[s])); expr[s][47] = 0;
            active[s] = (e[0] != 0);
        }
    }
    void editRange() {
        char b[24];
        snprintf(b, sizeof(b), "%g", xmin); if (!DM.inputLine("xmin:", b, sizeof(b))) return; xmin = atof(b);
        snprintf(b, sizeof(b), "%g", xmax); if (!DM.inputLine("xmax:", b, sizeof(b))) return; xmax = atof(b);
        snprintf(b, sizeof(b), "%g", ymin); if (!DM.inputLine("ymin:", b, sizeof(b))) return; ymin = atof(b);
        snprintf(b, sizeof(b), "%g", ymax); if (!DM.inputLine("ymax:", b, sizeof(b))) return; ymax = atof(b);
        if (modeT == 1 || modeT == 2) {
            snprintf(b, sizeof(b), "%g", tmin); if (!DM.inputLine("tmin:", b, sizeof(b))) return; tmin = atof(b);
            snprintf(b, sizeof(b), "%g", tmax); if (!DM.inputLine("tmax:", b, sizeof(b))) return; tmax = atof(b);
            snprintf(b, sizeof(b), "%g", tstep);if (!DM.inputLine("tstep:", b, sizeof(b))) return; tstep = atof(b);
            if (tstep <= 0) tstep = 0.05;
        }
        if (modeT == 5) {
            snprintf(b, sizeof(b), "%d", seqStart); if (!DM.inputLine("n start:", b, sizeof(b))) return; seqStart = atoi(b);
            snprintf(b, sizeof(b), "%d", seqEnd);   if (!DM.inputLine("n end:",   b, sizeof(b))) return; seqEnd   = atoi(b);
        }
        if (modeT == 6) {
            snprintf(b, sizeof(b), "%g", zmin); if (!DM.inputLine("zmin:", b, sizeof(b))) return; zmin = atof(b);
            snprintf(b, sizeof(b), "%g", zmax); if (!DM.inputLine("zmax:", b, sizeof(b))) return; zmax = atof(b);
        }
    }
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
    void trigWindow() { xmin=-2*M_PI; xmax=2*M_PI; ymin=-1.5; ymax=1.5; DM.notify("Trig", C_OK); }
    void plot(bool interactive = true) {
        bool any = false;
        for (int i = 0; i < MAXF; i++) if (active[i] && expr[i][0]) { any = true; break; }
        if (!any) { DM.notify("No active fn", C_ERR); return; }
        bool traceOn = false;
        int traceFn = 0;
        for (int i = 0; i < MAXF; i++) if (active[i]) { traceFn = i; break; }
        double tcur = (xmin + xmax) / 2;
        while (true) {
            drawAll();
            if (traceOn && modeT == 0) drawTraceMarker(traceFn, tcur);
            if (!interactive) return;
            DM.footer("Z:in X:out R:reset T:trace", "B:box +:next", "ESC:back");
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (!st.word.empty()) {
                    char c = st.word[0];
                    if (c == 'z' || c == 'Z') { zoomIn();  break; }
                    if (c == 'x' || c == 'X') { zoomOut(); break; }
                    if (c == 'r' || c == 'R') { stdWindow(); break; }
                    if (c == 't' || c == 'T') { traceOn = !traceOn; break; }
                    if (c == 'b' || c == 'B') { boxZoom(); break; }
                    if (c == '+' || c == 'n') {
                        for (int k = 1; k <= MAXF; k++) {
                            int q = (traceFn + k) % MAXF;
                            if (active[q]) { traceFn = q; break; }
                        }
                        break;
                    }
                    if (c == 's' || c == 'S') { DM.saveBMPAuto("graph"); break; }
                }
                if (traceOn) {
                    double dx = (xmax - xmin) / 80.0;
                    if (st.fn && IS_LEFT)  { tcur -= dx; break; }
                    if (st.fn && IS_RIGHT) { tcur += dx; break; }
                }
            }
        }
    }
    void drawAll() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        const char* names[] = {"y=f(x)", "Param", "Polar", "Implicit", "Ineq", "Sequence", "3D"};
        DM.header(names[modeT]);
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        if (xmin < 0 && xmax > 0) {
            int sx = gx + (int)((-xmin)/(xmax-xmin)*gw);
            d.drawLine(sx, gy, sx, gy+gh-1, C_GRID);
        }
        if (ymin < 0 && ymax > 0) {
            int sy = gy + gh - 1 - (int)((-ymin)/(ymax-ymin)*gh);
            d.drawLine(gx, sy, gx+gw-1, sy, C_GRID);
        }
        uint16_t cols[MAXF] = {C_ACC, C_OK, C_ACC2, C_ERR};
        for (int f = 0; f < MAXF; f++) {
            if (!active[f] || !expr[f][0]) continue;
            switch (modeT) {
                case 0: plotYofX(f, cols[f]); break;
                case 1: plotParam(f, cols[f]); break;
                case 2: plotPolar(f, cols[f]); break;
                case 3: plotImplicit(f, cols[f]); break;
                case 4: plotIneq(f, cols[f]); break;
                case 5: plotSeq(f, cols[f]); break;
                case 6: plot3D(f, cols[f]); break;
            }
        }
    }
    void plotYofX(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst sx = {'x', 0};
        int prevX = -1, prevY = -1; bool prevOk = false;
        for (int px = 0; px < gw; px++) {
            double x = xmin + (xmax - xmin) * px / gw;
            sx.val = x;
            auto r = gEv.eval(expr[f], &sx, 1);
            if (!r.ok || isnan(r.val) || isinf(r.val)) { prevOk = false; continue; }
            double y = r.val;
            if (y < ymin || y > ymax) { prevOk = false; continue; }
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin)*(gh-1));
            if (prevOk) d.drawLine(prevX, prevY, gx + px, py, col);
            else d.drawPixel(gx + px, py, col);
            prevX = gx + px; prevY = py; prevOk = true;
        }
    }
    void plotParam(int f, uint16_t col) {
        char* sep = strchr(expr[f], ';');
        if (!sep) return;
        char xe[48], ye[48];
        int n = sep - expr[f]; if (n >= 47) n = 47;
        memcpy(xe, expr[f], n); xe[n] = 0;
        strncpy(ye, sep + 1, 47); ye[47] = 0;
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst st = {'t', 0};
        int prevX = -1, prevY = -1; bool prevOk = false;
        for (double t = tmin; t <= tmax; t += tstep) {
            st.val = t;
            auto rx = gEv.eval(xe, &st, 1);
            auto ry = gEv.eval(ye, &st, 1);
            if (!rx.ok || !ry.ok) { prevOk = false; continue; }
            double x = rx.val, y = ry.val;
            if (x < xmin || x > xmax || y < ymin || y > ymax) { prevOk = false; continue; }
            int px = gx + (int)((x - xmin)/(xmax - xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin) * gh);
            if (prevOk) d.drawLine(prevX, prevY, px, py, col);
            prevX = px; prevY = py; prevOk = true;
        }
    }
    void plotPolar(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst st = {'t', 0};
        int prevX = -1, prevY = -1; bool prevOk = false;
        for (double t = tmin; t <= tmax; t += tstep) {
            st.val = t;
            auto r = gEv.eval(expr[f], &st, 1);
            if (!r.ok) { prevOk = false; continue; }
            double rad = r.val;
            double x = rad * cos(t), y = rad * sin(t);
            if (x < xmin || x > xmax || y < ymin || y > ymax) { prevOk = false; continue; }
            int px = gx + (int)((x - xmin)/(xmax - xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin) * gh);
            if (prevOk) d.drawLine(prevX, prevY, px, py, col);
            prevX = px; prevY = py; prevOk = true;
        }
    }
    void plotImplicit(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        const int RES = 40;
        ExprEval::Subst sub[2] = {{'x', 0}, {'y', 0}};
        double cellX = (xmax - xmin)/RES;
        double cellY = (ymax - ymin)/RES;
        double prev[42];
        for (int j = 0; j <= RES; j++) {
            double xj = xmin + j * cellX;
            sub[0].val = xj; sub[1].val = ymin;
            auto r = gEv.eval(expr[f], sub, 2);
            prev[j] = r.ok ? r.val : NAN;
        }
        for (int i = 0; i < RES; i++) {
            double yL = ymin + (i+1) * cellY;
            double cur[42];
            for (int j = 0; j <= RES; j++) {
                double xj = xmin + j * cellX;
                sub[0].val = xj; sub[1].val = yL;
                auto r = gEv.eval(expr[f], sub, 2);
                cur[j] = r.ok ? r.val : NAN;
            }
            for (int j = 0; j < RES; j++) {
                double v[4] = {prev[j], prev[j+1], cur[j+1], cur[j]};
                if (isnan(v[0])||isnan(v[1])||isnan(v[2])||isnan(v[3])) continue;
                int code = 0;
                for (int k = 0; k < 4; k++) if (v[k] > 0) code |= (1 << k);
                if (code == 0 || code == 15) continue;
                double cx0 = xmin + j*cellX, cy0 = ymin + i*cellY;
                double cx1 = cx0 + cellX,     cy1 = cy0 + cellY;
                auto edge = [&](int a, int b, double xa, double ya, double xb, double yb,
                                double& ex, double& ey) {
                    double t = v[a] / (v[a] - v[b]);
                    ex = xa + t * (xb - xa);
                    ey = ya + t * (yb - ya);
                };
                struct P { double x, y; };
                P pts[4]; int np = 0;
                if ((code & 1) != ((code >> 1) & 1)) edge(0, 1, cx0, cy0, cx1, cy0, pts[np].x, pts[np].y), np++;
                if (((code >> 1) & 1) != ((code >> 2) & 1)) edge(1, 2, cx1, cy0, cx1, cy1, pts[np].x, pts[np].y), np++;
                if (((code >> 2) & 1) != ((code >> 3) & 1)) edge(2, 3, cx1, cy1, cx0, cy1, pts[np].x, pts[np].y), np++;
                if (((code >> 3) & 1) != (code & 1)) edge(3, 0, cx0, cy1, cx0, cy0, pts[np].x, pts[np].y), np++;
                if (np >= 2) {
                    int x1 = gx + (int)((pts[0].x - xmin)/(xmax - xmin) * gw);
                    int y1 = gy + gh - 1 - (int)((pts[0].y - ymin)/(ymax - ymin) * gh);
                    int x2 = gx + (int)((pts[1].x - xmin)/(xmax - xmin) * gw);
                    int y2 = gy + gh - 1 - (int)((pts[1].y - ymin)/(ymax - ymin) * gh);
                    d.drawLine(x1, y1, x2, y2, col);
                }
                if (np == 4) {
                    int x1 = gx + (int)((pts[2].x - xmin)/(xmax - xmin) * gw);
                    int y1 = gy + gh - 1 - (int)((pts[2].y - ymin)/(ymax - ymin) * gh);
                    int x2 = gx + (int)((pts[3].x - xmin)/(xmax - xmin) * gw);
                    int y2 = gy + gh - 1 - (int)((pts[3].y - ymin)/(ymax - ymin) * gh);
                    d.drawLine(x1, y1, x2, y2, col);
                }
            }
            for (int j = 0; j <= RES; j++) prev[j] = cur[j];
        }
    }
    void plotIneq(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst sub[2] = {{'x', 0}, {'y', 0}};
        for (int py = 0; py < gh; py += 2) {
            double y = ymin + (gh - 1 - py) * (ymax - ymin) / gh;
            sub[1].val = y;
            for (int px = 0; px < gw; px += 2) {
                double x = xmin + px * (xmax - xmin) / gw;
                sub[0].val = x;
                auto r = gEv.eval(expr[f], sub, 2);
                if (r.ok && r.val > 0) d.drawPixel(gx + px, gy + py, col);
            }
        }
    }
    void plotSeq(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst sn = {'n', 0};
        int N = seqEnd - seqStart + 1;
        if (N < 2) return;
        for (int i = 0; i < N; i++) {
            sn.val = seqStart + i;
            auto r = gEv.eval(expr[f], &sn, 1);
            if (!r.ok) continue;
            if (r.val < ymin || r.val > ymax) continue;
            int px = gx + (int)((double)i / (N - 1) * (gw - 4)) + 2;
            int py = gy + gh - 1 - (int)((r.val - ymin)/(ymax - ymin) * gh);
            d.drawLine(px, py, px, gy + gh - 1, col);
            d.fillCircle(px, py, 2, col);
        }
    }
    void plot3D(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        const int N = 16;
        double dx = (xmax - xmin)/N, dy = (ymax - ymin)/N;
        ExprEval::Subst sub[2] = {{'x', 0}, {'y', 0}};
        double phi = M_PI/6, theta = M_PI/8;
        double cp = cos(phi), sp = sin(phi);
        double ct = cos(theta), st_ = sin(theta);
        double cx0 = (xmin + xmax)/2, cy0 = (ymin + ymax)/2, cz0 = (zmin + zmax)/2;
        double scale = (gw < gh ? gw : gh) / (2.5 * (xmax - xmin));
        auto project = [&](double x, double y, double z, int& px, int& py){
            double xr = (x - cx0)*cp - (y - cy0)*sp;
            double yr = (x - cx0)*sp + (y - cy0)*cp;
            double zr = (z - cz0);
            px = gx + gw/2 + (int)(xr * scale);
            py = gy + gh/2 - (int)((-yr*st_ + zr*ct) * scale);
        };
        for (int i = 0; i <= N; i++) {
            int prevX = -1, prevY = -1; bool ok = false;
            for (int j = 0; j <= N; j++) {
                double x = xmin + i*dx, y = ymin + j*dy;
                sub[0].val = x; sub[1].val = y;
                auto r = gEv.eval(expr[f], sub, 2);
                if (!r.ok) { ok = false; continue; }
                double z = r.val;
                if (z < zmin) z = zmin; if (z > zmax) z = zmax;
                int px, py; project(x, y, z, px, py);
                if (ok) d.drawLine(prevX, prevY, px, py, col);
                prevX = px; prevY = py; ok = true;
            }
        }
        for (int j = 0; j <= N; j++) {
            int prevX = -1, prevY = -1; bool ok = false;
            for (int i = 0; i <= N; i++) {
                double x = xmin + i*dx, y = ymin + j*dy;
                sub[0].val = x; sub[1].val = y;
                auto r = gEv.eval(expr[f], sub, 2);
                if (!r.ok) { ok = false; continue; }
                double z = r.val;
                if (z < zmin) z = zmin; if (z > zmax) z = zmax;
                int px, py; project(x, y, z, px, py);
                if (ok) d.drawLine(prevX, prevY, px, py, col);
                prevX = px; prevY = py; ok = true;
            }
        }
    }
    void drawTraceMarker(int f, double xv) {
        if (modeT != 0) return;
        if (!active[f]) return;
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        ExprEval::Subst sx = {'x', xv};
        auto r = gEv.eval(expr[f], &sx, 1);
        if (!r.ok) return;
        if (xv < xmin || xv > xmax || r.val < ymin || r.val > ymax) return;
        int px = gx + (int)((xv - xmin)/(xmax - xmin) * gw);
        int py = gy + gh - 1 - (int)((r.val - ymin)/(ymax - ymin) * gh);
        d.drawLine(px, gy, px, gy + gh - 1, C_ACC2);
        d.fillCircle(px, py, 3, C_OK);
        char l[40]; snprintf(l, sizeof(l), "x=%.4g y=%.4g F%d", xv, r.val, f+1);
        int tw = textWidth(l) + 6;
        if (tw > LCD_W) tw = LCD_W;
        d.fillRect(LCD_W - tw, gy, tw, 11, C_HDR);
        d.setTextColor(C_HDRTX, C_HDR);
        d.drawString(l, LCD_W - tw + 3, gy + 1);
    }
    void zoomIn() {
        double cx = (xmin+xmax)/2, cy = (ymin+ymax)/2;
        double dx = (xmax-xmin)/4, dy = (ymax-ymin)/4;
        xmin = cx - dx; xmax = cx + dx;
        ymin = cy - dy; ymax = cy + dy;
    }
    void zoomOut() {
        double cx = (xmin+xmax)/2, cy = (ymin+ymax)/2;
        double dx = xmax-xmin, dy = ymax-ymin;
        xmin = cx - dx; xmax = cx + dx;
        ymin = cy - dy; ymax = cy + dy;
    }
    void boxZoom() {
        char b[16];
        strcpy(b,"-1"); if (!DM.inputLine("zoom xmin:", b, sizeof(b))) return; xmin = atof(b);
        strcpy(b,"1");  if (!DM.inputLine("zoom xmax:", b, sizeof(b))) return; xmax = atof(b);
        strcpy(b,"-1"); if (!DM.inputLine("zoom ymin:", b, sizeof(b))) return; ymin = atof(b);
        strcpy(b,"1");  if (!DM.inputLine("zoom ymax:", b, sizeof(b))) return; ymax = atof(b);
    }
    void gsolveMenu() {
        const char* items[] = {
            "Root (sign change scan)","Maximum (gradient ascent)","Minimum (gradient descent)",
            "Intersection f1 = f2","y at x","x at y (root of f-y)","Definite integral","Back"
        };
        int s = DM.menu("G-Solve", items, 8);
        if (s < 0 || s == 7) return;
        int idx = -1;
        for (int i = 0; i < MAXF; i++) if (active[i] && expr[i][0]) { idx = i; break; }
        if (idx < 0) { DM.notify("No fn", C_ERR); return; }
        switch (s) {
            case 0: gsolveRoot(idx); break;
            case 1: gsolveExtremum(idx, true);  break;
            case 2: gsolveExtremum(idx, false); break;
            case 3: gsolveIntersect(); break;
            case 4: yAtX(idx); break;
            case 5: xAtY(idx); break;
            case 6: gsolveIntegral(idx); break;
        }
    }
    void gsolveRoot(int idx) {
        const int N = 200;
        double dx = (xmax-xmin)/N;
        ExprEval::Subst sx = {'x', 0};
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Roots"); DM.footer("","","ESC:back");
        int found = 0, y = HDR_H + 4;
        sx.val = xmin;
        auto r0 = gEv.eval(expr[idx], &sx, 1);
        double prev = r0.ok ? r0.val : NAN;
        for (int i = 1; i <= N && found < 8; i++) {
            sx.val = xmin + i*dx;
            auto r = gEv.eval(expr[idx], &sx, 1);
            if (!r.ok || isnan(prev)) { prev = r.ok ? r.val : NAN; continue; }
            if (prev * r.val < 0) {
                double a = sx.val - dx, b = sx.val, root;
                if (NumAnalysis::bisect(expr[idx], 'x', a, b, root, 1e-12, 60)) {
                    char l[40]; snprintf(l, sizeof(l), "x = %.6g", root);
                    d.setTextColor(C_OK, C_BG);
                    d.drawString(l, 4, y); y += 11; found++;
                }
            }
            prev = r.val;
        }
        if (found == 0) { d.setTextColor(C_DIM, C_BG); d.drawString("No sign change.", 4, HDR_H + 8); }
        DM.waitEsc();
    }
    void gsolveExtremum(int idx, bool isMax) {
        const int N = 200;
        double dx = (xmax-xmin)/N;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(isMax ? "Maximum" : "Minimum"); DM.footer("","","ESC:back");
        int found = 0, yy = HDR_H + 4;
        double prev = NumAnalysis::diffN(expr[idx], 'x', xmin);
        for (int i = 1; i <= N && found < 8; i++) {
            double x = xmin + i*dx;
            double cur = NumAnalysis::diffN(expr[idx], 'x', x);
            if (prev * cur < 0) {
                double lo = x - dx, hi = x;
                for (int k = 0; k < 60; k++) {
                    double m = (lo + hi)/2;
                    double fm = NumAnalysis::diffN(expr[idx], 'x', m);
                    if (prev * fm < 0) hi = m; else { lo = m; prev = fm; }
                }
                double xm = (lo + hi)/2;
                ExprEval::Subst sx = {'x', xm};
                auto r = gEv.eval(expr[idx], &sx, 1);
                if (r.ok) {
                    double d2 = NumAnalysis::diffN(expr[idx], 'x', xm + 0.001) -
                                NumAnalysis::diffN(expr[idx], 'x', xm - 0.001);
                    bool isLocalMax = (d2 < 0);
                    if (isLocalMax == isMax) {
                        char l[40]; snprintf(l, sizeof(l), "x=%.5g y=%.5g", xm, r.val);
                        d.setTextColor(C_OK, C_BG);
                        d.drawString(l, 4, yy); yy += 11; found++;
                    }
                }
            }
            prev = cur;
        }
        if (found == 0) { d.setTextColor(C_DIM, C_BG); d.drawString("Not found", 4, HDR_H + 8); }
        DM.waitEsc();
    }
    void gsolveIntersect() {
        int a = -1, b = -1;
        for (int i = 0; i < MAXF; i++) if (active[i] && expr[i][0]) {
            if (a < 0) a = i; else if (b < 0) { b = i; break; }
        }
        if (a < 0 || b < 0) { DM.notify("Need 2 fns", C_ERR); return; }
        char F[100]; snprintf(F, sizeof(F), "(%s)-(%s)", expr[a], expr[b]);
        const int N = 200;
        double dx = (xmax-xmin)/N;
        ExprEval::Subst sx = {'x', 0};
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Intersect"); DM.footer("","","ESC:back");
        int found = 0, yy = HDR_H + 4;
        sx.val = xmin;
        auto r0 = gEv.eval(F, &sx, 1);
        double prev = r0.ok ? r0.val : NAN;
        for (int i = 1; i <= N && found < 8; i++) {
            sx.val = xmin + i*dx;
            auto r = gEv.eval(F, &sx, 1);
            if (!r.ok || isnan(prev)) { prev = r.ok ? r.val : NAN; continue; }
            if (prev * r.val < 0) {
                double aa = sx.val - dx, bb = sx.val, root;
                if (NumAnalysis::bisect(F, 'x', aa, bb, root, 1e-12, 60)) {
                    sx.val = root;
                    auto ry = gEv.eval(expr[a], &sx, 1);
                    char l[40]; snprintf(l, sizeof(l), "x=%.5g y=%.5g", root, ry.val);
                    d.setTextColor(C_OK, C_BG);
                    d.drawString(l, 4, yy); yy += 11; found++;
                }
            }
            prev = r.val;
        }
        if (found == 0) { d.setTextColor(C_DIM, C_BG); d.drawString("No intersect", 4, HDR_H + 8); }
        DM.waitEsc();
    }
    void yAtX(int idx) {
        char b[16] = "0";
        if (!DM.inputLine("x:", b, sizeof(b))) return;
        ExprEval::Subst sx = {'x', atof(b)};
        auto r = gEv.eval(expr[idx], &sx, 1);
        char m[40]; snprintf(m, sizeof(m), "y = %g", r.ok ? r.val : NAN);
        DM.notify(m, C_OK, 1500);
    }
    void xAtY(int idx) {
        char b[16] = "0";
        if (!DM.inputLine("y:", b, sizeof(b))) return;
        char F[100]; snprintf(F, sizeof(F), "(%s)-(%s)", expr[idx], b);
        double root;
        const int N = 200;
        double dx = (xmax-xmin)/N;
        ExprEval::Subst sx = {'x', 0};
        sx.val = xmin;
        auto r0 = gEv.eval(F, &sx, 1);
        double prev = r0.ok ? r0.val : NAN;
        for (int i = 1; i <= N; i++) {
            sx.val = xmin + i*dx;
            auto r = gEv.eval(F, &sx, 1);
            if (!r.ok || isnan(prev)) { prev = r.ok ? r.val : NAN; continue; }
            if (prev * r.val < 0) {
                if (NumAnalysis::bisect(F, 'x', sx.val - dx, sx.val, root)) {
                    char m[40]; snprintf(m, sizeof(m), "x = %g", root);
                    DM.notify(m, C_OK, 1500);
                    return;
                }
            }
            prev = r.val;
        }
        DM.notify("Not found", C_ERR);
    }
    void gsolveIntegral(int idx) {
        char b[16];
        strcpy(b, "0"); if (!DM.inputLine("from a:", b, sizeof(b))) return; double a = atof(b);
        strcpy(b, "1"); if (!DM.inputLine("to b:",   b, sizeof(b))) return; double bb = atof(b);
        double v = NumAnalysis::integN(expr[idx], 'x', a, bb, 200);
        char m[40]; snprintf(m, sizeof(m), "Integral = %g", v);
        DM.notify(m, C_OK, 2000);
        G.ans = v; G.ansComplex = false;
    }
    void tanNormal() {
        int idx = -1;
        for (int i = 0; i < MAXF; i++) if (active[i] && expr[i][0]) { idx = i; break; }
        if (idx < 0) { DM.notify("No fn", C_ERR); return; }
        char b[16] = "1";
        if (!DM.inputLine("at x:", b, sizeof(b))) return;
        double x = atof(b);
        ExprEval::Subst sx = {'x', x};
        auto r = gEv.eval(expr[idx], &sx, 1);
        if (!r.ok) { DM.notify("eval err", C_ERR); return; }
        double y = r.val;
        double slope = NumAnalysis::diffN(expr[idx], 'x', x);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Tangent / Normal"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[60];
        snprintf(l,sizeof(l),"At (x, y) = (%g, %g)", x, y);     d.drawString(l, 4, HDR_H+4);
        snprintf(l,sizeof(l),"slope = %g", slope);              d.drawString(l, 4, HDR_H+18);
        snprintf(l,sizeof(l),"Tan: y = %g(X - %g) + %g", slope, x, y); d.drawString(l, 4, HDR_H+34);
        if (slope != 0) {
            snprintf(l,sizeof(l),"Norm: y = %g(X - %g) + %g", -1/slope, x, y);
            d.drawString(l, 4, HDR_H+50);
        } else {
            d.drawString("Norm: X = const", 4, HDR_H+50);
        }
        DM.waitEsc();
    }
};

class Spreadsheet {
public:
    void run() {
        if (!G.ssInitialized) {
            for (int r = 0; r < SS_ROWS; r++) for (int c = 0; c < SS_COLS; c++) G.ssData[r][c] = nullptr;
            G.ssInitialized = true;
        }
        int curR = 0, curC = 0;
        int topR = 0, leftC = 0;
        const int cellW = 38, cellH = 11;
        const int viewC = (LCD_W - 22) / cellW;
        const int viewR = (LCD_H - HDR_H - FTR_H - cellH) / cellH;
        while (true) {
            auto& d = M5Cardputer.Display;
            d.fillScreen(C_BG);
            char th[24]; snprintf(th, sizeof(th), "Sheet %c%d", 'A' + curC, curR + 1);
            DM.header(th);
            DM.footer("ENT:edit T:Lists", "S:save L:load", "ESC:back");
            if (curR < topR) topR = curR;
            if (curR >= topR + viewR) topR = curR - viewR + 1;
            if (curC < leftC) leftC = curC;
            if (curC >= leftC + viewC) leftC = curC - viewC + 1;
            d.fillRect(0, HDR_H, LCD_W, cellH, C_PANEL);
            d.setTextColor(C_DIM, C_PANEL);
            for (int c = 0; c < viewC; c++) {
                int cc = leftC + c;
                if (cc >= SS_COLS) break;
                char lab[4]; snprintf(lab, sizeof(lab), "%c", 'A' + cc);
                d.drawString(lab, 22 + c*cellW + cellW/2 - 4, HDR_H + 1);
            }
            for (int r = 0; r < viewR; r++) {
                int rr = topR + r;
                if (rr >= SS_ROWS) break;
                int yy = HDR_H + cellH + r*cellH;
                d.fillRect(0, yy, 22, cellH, C_PANEL);
                d.setTextColor(C_DIM, C_PANEL);
                char rs[4]; snprintf(rs, sizeof(rs), "%d", rr+1);
                d.drawString(rs, 4, yy + 1);
                for (int c = 0; c < viewC; c++) {
                    int cc = leftC + c;
                    if (cc >= SS_COLS) break;
                    int xx = 22 + c*cellW;
                    bool isSel = (rr == curR && cc == curC);
                    if (isSel) d.fillRect(xx, yy, cellW-1, cellH, C_SEL_BG);
                    d.setTextColor(isSel ? C_FG : C_DIM, isSel ? C_SEL_BG : C_BG);
                    if (G.ssData[rr][cc]) {
                        double v = evalCell(rr, cc);
                        if (!isnan(v)) {
                            char vs[12]; snprintf(vs, sizeof(vs), "%.4g", v);
                            d.drawString(vs, xx+1, yy+1);
                        } else {
                            d.drawString(G.ssData[rr][cc], xx+1, yy+1);
                        }
                    }
                    d.drawRect(xx, yy, cellW, cellH, C_GRID);
                }
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (st.fn && IS_UP)    { if (curR > 0) curR--; break; }
                if (st.fn && IS_DOWN)  { if (curR < SS_ROWS - 1) curR++; break; }
                if (st.fn && IS_LEFT)  { if (curC > 0) curC--; break; }
                if (st.fn && IS_RIGHT) { if (curC < SS_COLS - 1) curC++; break; }
                if (st.enter) {
                    char buf[64];
                    if (G.ssData[curR][curC]) strncpy(buf, G.ssData[curR][curC], 63);
                    else buf[0] = 0;
                    char title[24]; snprintf(title, sizeof(title), "%c%d =", 'A'+curC, curR+1);
                    if (DM.inputLine(title, buf, sizeof(buf))) G.setSS(curR, curC, buf);
                    break;
                }
                if (st.del) { G.setSS(curR, curC, ""); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { saveCSV(); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('l') || M5Cardputer.Keyboard.isKeyPressed('L')) { loadCSV(); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('t') || M5Cardputer.Keyboard.isKeyPressed('T')) { transferMenu(); break; }
            }
        }
    }
private:
    void transferMenu() {
        const char* it[] = { "Sheet A..H -> L1..L8", "L1..L8 -> Sheet A..H", "Cancel" };
        int s = DM.menu("Sheet <-> Lists", it, 3);
        if (s < 0 || s == 2) return;
        if (!G.stat[0].data) { if (!G.allocStat()) { DM.notify("alloc fail", C_ERR); return; } }
        if (s == 0) {
            for (int c = 0; c < STAT_LISTS; c++) {
                G.stat[c].count = 0;
                for (int r = 0; r < SS_ROWS; r++) {
                    double v = evalCell(r, c);
                    if (!isnan(v) && G.stat[c].count < G.statCapacity)
                        G.stat[c].data[G.stat[c].count++] = v;
                }
            }
            DM.notify("Imported to L1..L8", C_OK);
        } else {
            char b[24];
            for (int c = 0; c < STAT_LISTS; c++)
                for (int r = 0; r < G.stat[c].count && r < SS_ROWS; r++) {
                    snprintf(b, sizeof(b), "%.10g", G.stat[c].data[r]);
                    G.setSS(r, c, b);
                }
            DM.notify("Exported to sheet", C_OK);
        }
    }
    double evalCell(int row, int col, int depth = 0) {
        if (depth > 20) return NAN;
        if (!G.ssData[row][col]) return NAN;
        const char* src = G.ssData[row][col];
        char* end;
        double v = strtod(src, &end);
        if (end != src && *end == 0) return v;
        const char* p = src;
        if (*p == '=') p++;
        char expanded[256] = "";
        int outP = 0;
        while (*p && outP < 250) {
            if (isalpha((unsigned char)*p) && isdigit((unsigned char)*(p+1))) {
                int c = (*p >= 'a') ? (*p - 'a') : (*p - 'A');
                p++;
                int r = 0;
                while (isdigit((unsigned char)*p)) { r = r*10 + (*p - '0'); p++; }
                r--;
                if (c >= 0 && c < SS_COLS && r >= 0 && r < SS_ROWS) {
                    double cellV = evalCell(r, c, depth+1);
                    outP += snprintf(expanded + outP, 250 - outP, "(%g)", cellV);
                }
            } else {
                expanded[outP++] = *p++;
            }
        }
        expanded[outP] = 0;
        auto r = gEv.eval(expanded);
        return r.ok ? r.val : NAN;
    }
    void saveCSV() {
        if (!G.sdOK) { DM.notify("No SD", C_ERR); return; }
        char path[48] = "/sheet.csv";
        if (!DM.inputLine("Path:", path, sizeof(path))) return;
        SdGuard guard;
        File f = SD.open(path, FILE_WRITE);
        if (!f) return;
        for (int r = 0; r < SS_ROWS; r++) {
            for (int c = 0; c < SS_COLS; c++) {
                if (G.ssData[r][c]) f.print(G.ssData[r][c]);
                if (c < SS_COLS - 1) f.print(',');
            }
            f.println();
        }
        f.close();
        DM.notify("Saved", C_OK);
    }
    void loadCSV() {
        if (!G.sdOK) { DM.notify("No SD", C_ERR); return; }
        char path[48] = "/sheet.csv";
        if (!DM.inputLine("Path:", path, sizeof(path))) return;
        SdGuard guard;
        File f = SD.open(path);
        if (!f) { DM.notify("Not found", C_ERR); return; }
        G.freeSS();
        G.ssInitialized = true;
        int row = 0;
        while (f.available() && row < SS_ROWS) {
            String line = f.readStringUntil('\n');
            int col = 0, from = 0;
            while (col < SS_COLS) {
                int comma = line.indexOf(',', from);
                String tok = (comma < 0) ? line.substring(from) : line.substring(from, comma);
                if (tok.length() > 0) G.setSS(row, col, tok.c_str());
                if (comma < 0) break;
                from = comma + 1;
                col++;
            }
            row++;
        }
        f.close();
        DM.notify("Loaded", C_OK);
    }
};

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 20 / 20 ─ setup() / loop() / メインメニュー / NVS / Wi-Fi       ║
// ╚═══════════════════════════════════════════════════════════════════════╝

CalculatorCore   gCalc;
SciRef           gSciRef;
StatCore         gStatCore;
StatRegress      gStatRegress;
StatTests        gStatTests;
StatNonparOdds   gStatNonpar;
DistBrowser      gDistBrowser;
StatMultivar     gStatMV;
StatStrata       gStatStrata;
SurvivalAnalysis gSurvival;
MetaAnalysis     gMeta;
StatGraphs       gStatGraphs;
GraphEngine      gGraph;
ULispMode        gULisp;
Spreadsheet      gSheet;

class CASEngine {
public:
    void run() {
        while (true) {
            const char* items[] = {
                "Polynomial: Eval at x","Polynomial: Differentiate","Polynomial: Integrate",
                "Polynomial: Add","Polynomial: Multiply","Polynomial: Roots",
                "Numerical Taylor (any f)","Numerical Limit (any f)","Substitute symbol","Back"
            };
            int s = DM.menu("CAS", items, 10);
            if (s < 0 || s == 9) return;
            switch (s) {
                case 0: polyEval(); break;
                case 1: polyDiff(); break;
                case 2: polyInteg(); break;
                case 3: polyOp(false); break;
                case 4: polyOp(true);  break;
                case 5: polyRoots(); break;
                case 6: taylor();    break;
                case 7: limitN();    break;
                case 8: substitute(); break;
            }
        }
    }
private:
    bool parsePoly(const char* s, Polynomial& p) {
        for (int i = 0; i <= POLY_MAX; i++) p.c[i] = 0;
        const char* q = s;
        int sign = 1;
        while (*q == ' ') q++;
        if (*q == '+') q++;
        else if (*q == '-') { sign = -1; q++; }
        while (*q) {
            while (*q == ' ') q++;
            if (!*q) break;
            double coef = 1.0;
            bool sawCoef = false;
            if (isdigit((unsigned char)*q) || *q == '.') {
                char* end;
                coef = strtod(q, &end); q = end; sawCoef = true;
            }
            int power = 0;
            if (*q == 'x' || *q == 'X') {
                q++; power = 1;
                if (*q == '^') {
                    q++;
                    char* end; long e = strtol(q, &end, 10);
                    if (end == q) return false;
                    q = end; power = (int)e;
                }
            } else {
                if (!sawCoef) return false;
                power = 0;
            }
            if (power < 0 || power > POLY_MAX) return false;
            p.c[power] += sign * coef;
            while (*q == ' ') q++;
            if (*q == '+') { sign = 1; q++; }
            else if (*q == '-') { sign = -1; q++; }
            else if (*q == 0) break;
            else return false;
        }
        p.normalize();
        return true;
    }
    void polyEval() {
        char b[64] = "1+2x-x^2";
        if (!DM.inputLine("p(x):", b, sizeof(b))) return;
        Polynomial p;
        if (!parsePoly(b, p)) { DM.notify("Parse err", C_ERR); return; }
        char xb[24] = "0";
        if (!DM.inputLine("x =", xb, sizeof(xb))) return;
        double x = atof(xb);
        double v = p.eval(x);
        char vs[40]; formatNum(v, vs, sizeof(vs));
        char m[40]; snprintf(m, sizeof(m), "p(%g) = %s", x, vs);
        DM.notify(m, C_OK, 1500);
        G.ans = v;
    }
    void polyDiff() {
        char b[64] = "x^3-2x+1";
        if (!DM.inputLine("p(x):", b, sizeof(b))) return;
        Polynomial p;
        if (!parsePoly(b, p)) { DM.notify("Parse err", C_ERR); return; }
        Polynomial d = p.diff();
        char r[120]; d.toString(r, sizeof(r));
        showResult("p'(x)", r);
    }
    void polyInteg() {
        char b[64] = "3x^2-2x+1";
        if (!DM.inputLine("p(x):", b, sizeof(b))) return;
        Polynomial p;
        if (!parsePoly(b, p)) { DM.notify("Parse err", C_ERR); return; }
        Polynomial i = p.integ();
        char r[120]; i.toString(r, sizeof(r));
        char r2[140]; snprintf(r2, sizeof(r2), "%s + C", r);
        showResult("Integral", r2);
    }
    void polyOp(bool isMul) {
        char a[64] = "x+1", b[64] = "x-1";
        if (!DM.inputLine("p(x):", a, sizeof(a))) return;
        if (!DM.inputLine("q(x):", b, sizeof(b))) return;
        Polynomial P, Q;
        if (!parsePoly(a, P) || !parsePoly(b, Q)) { DM.notify("Parse err", C_ERR); return; }
        Polynomial R = isMul ? (P * Q) : (P + Q);
        char r[140]; R.toString(r, sizeof(r));
        showResult(isMul ? "p*q" : "p+q", r);
    }
    void polyRoots() {
        char b[64] = "x^2-1";
        if (!DM.inputLine("p(x):", b, sizeof(b))) return;
        Polynomial p;
        if (!parsePoly(b, p)) { DM.notify("Parse err", C_ERR); return; }
        Complex roots[16];
        int n = NumAnalysis::polyRoots(p.c, p.deg, roots);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Roots"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        for (int i = 0; i < n && HDR_H + 4 + i*12 < LCD_H - FTR_H; i++) {
            char rs[32]; roots[i].toString(rs, sizeof(rs));
            char l[40]; snprintf(l, sizeof(l), "r%d = %s", i+1, rs);
            d.drawString(l, 4, HDR_H + 4 + i*12);
        }
        DM.waitEsc();
    }
    void taylor() {
        char b[80] = "sin(x)";
        if (!DM.inputLine("f(x):", b, sizeof(b))) return;
        char x0b[16] = "0";
        if (!DM.inputLine("center x0:", x0b, sizeof(x0b))) return;
        char nb[8] = "5";
        if (!DM.inputLine("order (1-7):", nb, sizeof(nb))) return;
        double x0 = atof(x0b);
        int n = atoi(nb);
        if (n < 1 || n > 7) { DM.notify("1..7", C_ERR); return; }
        double coef[8];
        ExprEval::Subst s = {'x', x0};
        auto fv = gEv.eval(b, &s, 1);
        if (!fv.ok) { DM.notify("eval err", C_ERR); return; }
        coef[0] = fv.val;
        double h = 0.01;
        for (int k = 1; k <= n; k++) {
            double sum = 0;
            for (int j = 0; j <= k; j++) {
                double cb = 1.0;
                for (int t = 1; t <= j; t++) cb = cb * (k - t + 1) / t;
                if (j & 1) cb = -cb;
                ExprEval::Subst ss = {'x', x0 + (k - 2.0*j)*h/2.0};
                auto rr = gEv.eval(b, &ss, 1);
                if (!rr.ok) { DM.notify("eval err", C_ERR); return; }
                sum += cb * rr.val;
            }
            double dk = sum / pow(h, k);
            double fact = 1; for (int t = 2; t <= k; t++) fact *= t;
            coef[k] = dk / fact;
        }
        char r[200]; r[0] = 0;
        bool first = true;
        for (int i = 0; i <= n; i++) {
            if (fabs(coef[i]) < 1e-9) continue;
            char term[40];
            const char* sg = "";
            double a = coef[i];
            if (first) { sg = (a < 0) ? "-" : ""; first = false; }
            else       { sg = (a < 0) ? " - " : " + "; }
            double aa = fabs(a);
            if (i == 0)      snprintf(term,sizeof(term),"%s%.5g", sg, aa);
            else if (i == 1) snprintf(term,sizeof(term),"%s%.5g(x-%.3g)", sg, aa, x0);
            else             snprintf(term,sizeof(term),"%s%.5g(x-%.3g)^%d", sg, aa, x0, i);
            strncat(r, term, sizeof(r) - strlen(r) - 1);
        }
        if (first) snprintf(r, sizeof(r), "0");
        showResult("Taylor", r);
    }
    void limitN() {
        char b[80] = "sin(x)/x";
        if (!DM.inputLine("f(x):", b, sizeof(b))) return;
        char xb[16] = "0";
        if (!DM.inputLine("x ->", xb, sizeof(xb))) return;
        double x0 = atof(xb);
        double h = 1e-4;
        ExprEval::Subst sL = {'x', x0 - h}, sR = {'x', x0 + h};
        auto rL = gEv.eval(b, &sL, 1);
        auto rR = gEv.eval(b, &sR, 1);
        char m[40];
        if (rL.ok && rR.ok && fabs(rL.val - rR.val) < 1e-3) {
            snprintf(m, sizeof(m), "Limit = %g", (rL.val + rR.val)/2);
            G.ans = (rL.val + rR.val)/2;
        } else {
            snprintf(m, sizeof(m), "L=%g R=%g (diff)", rL.val, rR.val);
        }
        DM.notify(m, C_OK, 2000);
    }
    void substitute() {
        char b[80] = "x^2+3x+2";
        if (!DM.inputLine("f(x):", b, sizeof(b))) return;
        char vb[24] = "y+1";
        if (!DM.inputLine("Replace x =", vb, sizeof(vb))) return;
        char out[160] = "";
        int outP = 0;
        const char* p = b;
        while (*p && outP < 150) {
            if (*p == 'x' && (p == b || !isalnum((unsigned char)*(p-1))) && !isalnum((unsigned char)*(p+1))) {
                outP += snprintf(out + outP, 150 - outP, "(%s)", vb);
                p++;
            } else {
                out[outP++] = *p++;
            }
        }
        out[outP] = 0;
        showResult("Substituted", out);
    }
    void showResult(const char* title, const char* r) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(title); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        DM.drawWrapped(r, 4, HDR_H + 8, LCD_W - 8, C_OK);
        DM.waitEsc();
    }
};
CASEngine gCAS;

class SettingsNVS {
public:
    void load() {
        Preferences p;
        p.begin("pomas", true);
        G.angle      = (AngleMode)p.getInt("angle",      DEG);
        G.disp       = (DispMode) p.getInt("disp",       DISP_NORM);
        G.dispDigits = p.getInt("digits",     4);
        G.normMode   = p.getInt("norm",       1);
        G.rpn        = p.getBool("rpn",       false);
        G.fracDisplay= p.getBool("frac",      false);
        String ssid  = p.getString("wifissid", "");
        String pass  = p.getString("wifipass", "");
        strncpy(G.wifiSSID, ssid.c_str(), 32);
        strncpy(G.wifiPASS, pass.c_str(), 32);
        p.end();
    }
    void save() {
        Preferences p;
        p.begin("pomas", false);
        p.putInt("angle",  G.angle);
        p.putInt("disp",   G.disp);
        p.putInt("digits", G.dispDigits);
        p.putInt("norm",   G.normMode);
        p.putBool("rpn",   G.rpn);
        p.putBool("frac",  G.fracDisplay);
        p.putString("wifissid", G.wifiSSID);
        p.putString("wifipass", G.wifiPASS);
        p.end();
    }
};
SettingsNVS gNVS;

class WiFiMaxima {
public:
    void run() {
        while (true) {
            const char* items[] = {
                "Set SSID/Pass","Scan networks","Connect","Disconnect",
                "Status","Send to Maxima (srv req.)","Back"
            };
            int s = DM.menu("Wi-Fi/Maxima (srv)", items, 7);
            if (s < 0 || s == 6) return;
            switch (s) {
                case 0: setCred();    break;
                case 1: scanNetworks(); break;
                case 2: connect();    break;
                case 3: disconnect(); break;
                case 4: status();     break;
                case 5: sendMaxima(); break;
            }
        }
    }
private:
    void scanNetworks() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Scanning..."); DM.footer("","","please wait");
        d.setTextColor(C_FG, C_BG);
        d.drawString("Searching for APs...", 4, HDR_H + 8);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(true, true);
        delay(100);
        int n = WiFi.scanNetworks();
        if (n <= 0) { DM.notify("No networks", C_WARN); return; }
        if (n > 30) n = 30;
        char** labels = (char**)malloc(n * sizeof(char*));
        for (int i = 0; i < n; i++) {
            labels[i] = (char*)malloc(48);
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            wifi_auth_mode_t auth = WiFi.encryptionType(i);
            const char* lock = (auth == WIFI_AUTH_OPEN) ? " " : "*";
            snprintf(labels[i], 48, "%s%-20s %d", lock, ssid.c_str(), rssi);
        }
        const char** clabels = (const char**)labels;
        int sel = DM.menu("Networks (* secure)", clabels, n);
        if (sel >= 0) {
            String ssid = WiFi.SSID(sel);
            strncpy(G.wifiSSID, ssid.c_str(), 32);
            G.wifiSSID[32] = 0;
            DM.notify("SSID set, set Pass", C_OK, 1500);
            char pass[33]; strncpy(pass, G.wifiPASS, 32); pass[32]=0;
            if (DM.inputLine("Pass:", pass, sizeof(pass))) {
                strncpy(G.wifiPASS, pass, 32);
                gNVS.save();
                DM.notify("Saved", C_OK);
            }
        }
        for (int i = 0; i < n; i++) free(labels[i]);
        free(labels);
        WiFi.scanDelete();
    }
    void setCred() {
        char ssid[33]; strncpy(ssid, G.wifiSSID, 32); ssid[32]=0;
        if (!DM.inputLine("SSID:", ssid, sizeof(ssid))) return;
        char pass[33]; strncpy(pass, G.wifiPASS, 32); pass[32]=0;
        if (!DM.inputLine("Pass:", pass, sizeof(pass))) return;
        strncpy(G.wifiSSID, ssid, 32);
        strncpy(G.wifiPASS, pass, 32);
        gNVS.save();
        DM.notify("Saved", C_OK);
    }
    void connect() {
        if (G.wifiSSID[0] == 0) { DM.notify("No SSID set", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Connecting..."); DM.footer("","","ESC:abort");
        d.setTextColor(C_FG, C_BG);
        char l[64];
        snprintf(l, sizeof(l), "SSID: %s", G.wifiSSID);
        d.drawString(l, 4, HDR_H + 8);
        WiFi.disconnect(true, true);
        delay(200);
        WiFi.mode(WIFI_OFF);
        delay(100);
        WiFi.mode(WIFI_STA);
        delay(100);
        WiFi.setAutoReconnect(false);
        if (G.wifiPASS[0] == 0) WiFi.begin(G.wifiSSID);
        else                    WiFi.begin(G.wifiSSID, G.wifiPASS);
        unsigned long t0 = millis();
        int dots = 0;
        wl_status_t lastStatus = WL_IDLE_STATUS;
        while (millis() - t0 < 20000) {
            wl_status_t s = WiFi.status();
            if (s == WL_CONNECTED) break;
            if (s == WL_CONNECT_FAILED) break;
            if (s == WL_NO_SSID_AVAIL) break;
            if (s != lastStatus) {
                d.fillRect(0, HDR_H + 22, LCD_W, 30, C_BG);
                d.setTextColor(C_DIM, C_BG);
                const char* msg = "...";
                switch (s) {
                    case WL_IDLE_STATUS:     msg = "Idle..."; break;
                    case WL_NO_SSID_AVAIL:   msg = "SSID not found"; break;
                    case WL_SCAN_COMPLETED:  msg = "Scan done"; break;
                    case WL_CONNECT_FAILED:  msg = "Auth failed"; break;
                    case WL_CONNECTION_LOST: msg = "Connection lost"; break;
                    case WL_DISCONNECTED:    msg = "Disconnected"; break;
                    default:                 msg = "Connecting..."; break;
                }
                d.drawString(msg, 4, HDR_H + 22);
                lastStatus = s;
            }
            char dotStr[16] = "";
            for (int i = 0; i < (dots % 4); i++) dotStr[i] = '.';
            dotStr[dots % 4] = 0;
            d.fillRect(4, HDR_H + 36, 60, 12, C_BG);
            d.setTextColor(C_ACC, C_BG);
            d.drawString(dotStr, 4, HDR_H + 36);
            dots++;
            delay(250);
            if (DM.pollKeys()) {
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) {
                    WiFi.disconnect(true, true);
                    DM.notify("Aborted", C_WARN);
                    return;
                }
            }
        }
        if (WiFi.status() == WL_CONNECTED) {
            G.wifiOK = true;
            d.fillScreen(C_BG); DM.header("Connected"); DM.footer("","","ESC:back");
            d.setTextColor(C_OK, C_BG);
            snprintf(l, sizeof(l), "IP: %s", WiFi.localIP().toString().c_str());
            d.drawString(l, 4, HDR_H + 8);
            snprintf(l, sizeof(l), "RSSI: %d dBm", WiFi.RSSI());
            d.setTextColor(C_FG, C_BG);
            d.drawString(l, 4, HDR_H + 22);
            snprintf(l, sizeof(l), "MAC: %s", WiFi.macAddress().c_str());
            d.setTextColor(C_DIM, C_BG);
            d.drawString(l, 4, HDR_H + 36);
            DM.waitEsc();
        } else {
            G.wifiOK = false;
            const char* reason = "Unknown";
            wl_status_t s = WiFi.status();
            switch (s) {
                case WL_NO_SSID_AVAIL:   reason = "SSID not found"; break;
                case WL_CONNECT_FAILED:  reason = "Auth failed (pass?)"; break;
                case WL_CONNECTION_LOST: reason = "Connection lost"; break;
                case WL_DISCONNECTED:    reason = "Timeout"; break;
                default:                 reason = "Failed"; break;
            }
            d.fillScreen(C_BG); DM.header("Connect Failed"); DM.footer("","","ESC:back");
            d.setTextColor(C_ERR, C_BG);
            d.drawString(reason, 4, HDR_H + 8);
            d.setTextColor(C_DIM, C_BG);
            d.drawString("Check SSID/Pass", 4, HDR_H + 22);
            d.drawString("2.4GHz only (no 5G)", 4, HDR_H + 34);
            d.drawString("Press ESC to back", 4, HDR_H + 56);
            DM.waitEsc();
        }
    }
    void disconnect() {
        WiFi.disconnect();
        G.wifiOK = false;
        DM.notify("Disconnected", C_OK);
    }
    void status() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Wi-Fi Status"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        char l[40];
        snprintf(l,sizeof(l),"SSID: %s", G.wifiSSID); d.drawString(l, 4, HDR_H + 4);
        if (WiFi.status() == WL_CONNECTED) {
            d.setTextColor(C_OK, C_BG);
            snprintf(l,sizeof(l),"IP: %s", WiFi.localIP().toString().c_str()); d.drawString(l, 4, HDR_H + 18);
            snprintf(l,sizeof(l),"RSSI: %d dBm", WiFi.RSSI()); d.drawString(l, 4, HDR_H + 32);
        } else {
            d.setTextColor(C_ERR, C_BG);
            d.drawString("Not connected", 4, HDR_H + 18);
        }
        DM.waitEsc();
    }
    void sendMaxima() {
        if (!G.wifiOK || WiFi.status() != WL_CONNECTED) { DM.notify("Not connected", C_ERR); return; }
        char url[128] = "http://YOUR-PROXY-HOST/maxima?expr=";
        if (!DM.inputLine("URL prefix:", url, sizeof(url))) return;
        if (strstr(url, "YOUR-PROXY-HOST")) { DM.notify("Set proxy URL (see manual)", C_WARN); return; }
        char expr[80] = "diff(sin(x),x)";
        if (!DM.inputLine("Maxima expr:", expr, sizeof(expr))) return;
        char full[256]; int p = strlen(url);
        strncpy(full, url, sizeof(full));
        for (int i = 0; expr[i] && p < 250; i++) {
            char c = expr[i];
            if (isalnum(c) || c == '.' || c == '-' || c == '_') full[p++] = c;
            else { p += snprintf(full + p, 256 - p, "%%%02X", (unsigned char)c); }
        }
        full[p] = 0;
        HTTPClient http;
        http.begin(full);
        int code = http.GET();
        if (code == 200) {
            String resp = http.getString();
            auto& d = M5Cardputer.Display;
            d.fillScreen(C_BG); DM.header("Maxima Result"); DM.footer("","","ESC:back");
            DM.drawWrapped(resp.c_str(), 4, HDR_H + 4, LCD_W - 8, C_OK);
            DM.waitEsc();
        } else {
            char m[32]; snprintf(m, sizeof(m), "HTTP %d", code);
            DM.notify(m, C_ERR);
        }
        http.end();
    }
};
WiFiMaxima gWiFi;

class PowerInfo {
public:
    void show() {
        auto& d = M5Cardputer.Display;
        while (true) {
            d.fillScreen(C_BG); DM.header("Power / About"); DM.footer("R:retry SD", "T:SD test", "ESC:back");
            int level = -1;
            bool charging = false;
#ifdef ARDUINO_M5STACK_CARDPUTER
            level = M5Cardputer.Power.getBatteryLevel();
            charging = M5Cardputer.Power.isCharging();
#endif
            d.setTextColor(C_FG, C_BG);
            char l[60];
            snprintf(l,sizeof(l),"Battery: %d%% %s", level, charging ? "(charging)" : "");
            d.drawString(l, 4, HDR_H + 4);
            snprintf(l,sizeof(l),"PSRAM: %s", G.psramOK ? "available" : "none (normal)");
            d.drawString(l, 4, HDR_H + 16);
            if (G.sdOK) {
                d.setTextColor(C_OK, C_BG);
                uint64_t cardSize = SD.cardSize() / (1024 * 1024);
                snprintf(l, sizeof(l), "SD: OK (%lluMB)", cardSize);
            } else {
                d.setTextColor(C_ERR, C_BG);
                snprintf(l, sizeof(l), "SD: NOT detected");
            }
            d.drawString(l, 4, HDR_H + 28);
            if (G.wifiOK && WiFi.status() == WL_CONNECTED) {
                d.setTextColor(C_OK, C_BG);
                snprintf(l, sizeof(l), "Wi-Fi: %s", WiFi.SSID().c_str());
            } else {
                d.setTextColor(C_DIM, C_BG);
                snprintf(l, sizeof(l), "Wi-Fi: off");
            }
            d.drawString(l, 4, HDR_H + 40);
            d.setTextColor(C_FG, C_BG);
            snprintf(l, sizeof(l), "Free heap: %u KB", ESP.getFreeHeap() / 1024);
            d.drawString(l, 4, HDR_H + 52);
            d.setTextColor(C_ACC, C_BG);
            d.drawString(POMAS_NAME " ver." POMAS_VER, 4, HDR_H + 70);
            d.setTextColor(C_DIM, C_BG);
            d.drawString(POMAS_CRED, 4, HDR_H + 82);
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) { retrySD(); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('t') || M5Cardputer.Keyboard.isKeyPressed('T')) { sdTest(); break; }
            }
        }
    }
private:
    void retrySD() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Retry SD"); DM.footer("","","please wait");
        d.setTextColor(C_FG, C_BG);
        d.drawString("Re-initializing SD...", 4, HDR_H + 8);
        SD.end();
        delay(100);
        M5Cardputer.Display.endWrite();
        SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
        delay(100);
        G.sdOK = false;
        uint32_t sdFreqs[] = {25000000, 10000000, 4000000, 1000000};
        int succFreq = 0;
        for (int retry = 0; retry < 4 && !G.sdOK; retry++) {
            M5Cardputer.Display.endWrite();
            if (SD.begin(SD_CS, SPI, sdFreqs[retry])) {
                G.sdOK = true; succFreq = sdFreqs[retry]; break;
            }
            SD.end();
            delay(200);
        }
        d.fillScreen(C_BG); DM.header("Retry SD"); DM.footer("","","ESC:back");
        if (G.sdOK) {
            d.setTextColor(C_OK, C_BG);
            d.drawString("SD card detected!", 4, HDR_H + 8);
            char l[40];
            uint8_t cardType = SD.cardType();
            const char* tStr = "?";
            if (cardType == CARD_MMC)       tStr = "MMC";
            else if (cardType == CARD_SD)   tStr = "SDSC";
            else if (cardType == CARD_SDHC) tStr = "SDHC";
            else if (cardType == CARD_NONE) tStr = "NONE";
            uint64_t cardSize = SD.cardSize() / (1024 * 1024);
            snprintf(l, sizeof(l), "Type: %s", tStr); d.drawString(l, 4, HDR_H + 22);
            snprintf(l, sizeof(l), "Size: %lluMB", cardSize); d.drawString(l, 4, HDR_H + 34);
            snprintf(l, sizeof(l), "Speed: %u Hz", succFreq); d.drawString(l, 4, HDR_H + 46);
        } else {
            d.setTextColor(C_ERR, C_BG);
            d.drawString("SD init failed", 4, HDR_H + 8);
            d.setTextColor(C_DIM, C_BG);
            d.drawString("Check:", 4, HDR_H + 22);
            d.drawString("- Card inserted fully", 4, HDR_H + 34);
            d.drawString("- Contacts AWAY screen", 4, HDR_H + 46);
            d.drawString("- Format: FAT32", 4, HDR_H + 58);
            d.drawString("- Size: 32GB or less", 4, HDR_H + 70);
        }
        DM.waitEsc();
    }
    void sdTest() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("SD Test"); DM.footer("","","ESC:back");
        d.setTextColor(C_FG, C_BG);
        int y = HDR_H + 4;
        if (!G.sdOK) { d.setTextColor(C_ERR, C_BG); d.drawString("SD not mounted", 4, y); DM.waitEsc(); return; }
        d.drawString("Writing /pomas_test.txt", 4, y); y += 12;
        File f = SD.open("/pomas_test.txt", FILE_WRITE);
        if (!f) { d.setTextColor(C_ERR, C_BG); d.drawString("Write open FAILED", 4, y); DM.waitEsc(); return; }
        f.println("PoMaS SD test"); f.println("Hello, world!"); f.close();
        d.setTextColor(C_OK, C_BG); d.drawString("Write OK", 4, y); y += 12;
        d.setTextColor(C_FG, C_BG); d.drawString("Reading back...", 4, y); y += 12;
        File f2 = SD.open("/pomas_test.txt");
        if (!f2) { d.setTextColor(C_ERR, C_BG); d.drawString("Read open FAILED", 4, y); DM.waitEsc(); return; }
        String content = "";
        while (f2.available()) content += (char)f2.read();
        f2.close();
        d.setTextColor(C_OK, C_BG); d.drawString("Read OK", 4, y); y += 12;
        SD.remove("/pomas_test.txt");
        d.setTextColor(C_DIM, C_BG); d.drawString("Cleanup OK", 4, y); y += 12;
        d.setTextColor(C_OK, C_BG); d.drawString("ALL TESTS PASSED", 4, y);
        DM.waitEsc();
    }
};
PowerInfo gPwr;

// ── CSVストリーミング統計 ──
class CsvStream {
    static const int MAXC = 32;
    int    nc = 0;
    long   cnt[MAXC];
    double mean[MAXC], M2[MAXC], M3[MAXC], M4[MAXC], mn[MAXC], mx[MAXC];
    char   cname[MAXC][14];
    int    rx = -1, ry = -1;
    long   pN = 0;
    double pmx = 0, pmy = 0, pM2x = 0, pM2y = 0, pC = 0;
    double lastMean = 0;
    void resetCols() {
        nc = 0;
        for (int i=0;i<MAXC;i++){ cnt[i]=0; mean[i]=M2[i]=M3[i]=M4[i]=0; mn[i]=mx[i]=0; cname[i][0]=0; }
        pN=0; pmx=pmy=pM2x=pM2y=pC=0;
    }
    void update(int c, double x) {
        if (c<0||c>=MAXC) return;
        long n0 = cnt[c], n = n0 + 1;
        double delta = x - mean[c];
        double dn = delta / n, dn2 = dn*dn;
        double t1 = delta*dn*(double)n0;
        mean[c] += dn;
        M4[c] += t1*dn2*(double)(n*n - 3*n + 3) + 6.0*dn2*M2[c] - 4.0*dn*M3[c];
        M3[c] += t1*dn*(double)(n-2) - 3.0*dn*M2[c];
        M2[c] += t1;
        if (n0==0){ mn[c]=mx[c]=x; } else { if(x<mn[c])mn[c]=x; if(x>mx[c])mx[c]=x; }
        cnt[c]=n;
    }
    void updatePair(double x, double y) {
        long n = pN + 1;
        double dx = x - pmx, dy = y - pmy;
        pmx += dx/n;  pmy += dy/n;
        pM2x += dx*(x-pmx);
        pM2y += dy*(y-pmy);
        pC   += dx*(y-pmy);
        pN = n;
    }
    void captureHeader(const String& line) {
        int col=0, from=0;
        while (col < MAXC) {
            int comma = line.indexOf(',', from);
            String tok = (comma<0)? line.substring(from) : line.substring(from, comma);
            tok.trim();
            strncpy(cname[col], tok.c_str(), 13); cname[col][13]=0;
            if (col+1 > nc) nc = col+1;
            if (comma<0) break;
            from = comma+1; col++;
        }
    }
    void drawKV(const char* k, double v, int y) {
        auto& d = M5Cardputer.Display;
        char vs[32]; formatNum(v, vs, sizeof(vs));
        char ln[56]; snprintf(ln,sizeof(ln),"%-10s %s", k, vs);
        d.drawString(ln, 8, y);
    }
    void showResults(long rows) {
        auto& d = M5Cardputer.Display;
        int page = 0;
        int pages = nc + ((rx>=0 && ry>=0) ? 1 : 0);
        if (pages < 1) pages = 1;
        while (true) {
            d.fillScreen(C_BG);
            char h[40]; snprintf(h,sizeof(h),"CSV %d/%d (rows %ld)", page+1, pages, rows);
            DM.header(h);
            DM.footer("L/R:page","Y:->Ans","ESC:back");
            int y = HDR_H + 4;
            if (page < nc) {
                int c = page; long n = cnt[c];
                const char* nm = cname[c][0]? cname[c] : "(col)";
                char ln[48]; snprintf(ln,sizeof(ln),"Col %d: %s", c+1, nm);
                d.setTextColor(C_ACC,C_BG); d.drawString(ln,4,y); y+=12;
                d.setTextColor(C_FG,C_BG);
                double varS=(n>1)?M2[c]/(n-1):0, sdS=sqrt(varS);
                double varP=(n>0)?M2[c]/n:0,     sdP=sqrt(varP);
                double skew=(n>2 && M2[c]>0)? sqrt((double)n)*M3[c]/pow(M2[c],1.5):0;
                double kurt=(n>3 && M2[c]>0)? (double)n*M4[c]/(M2[c]*M2[c])-3.0:0;
                drawKV("n",        (double)n, y); y+=11;
                drawKV("mean",     mean[c],   y); y+=11;
                drawKV("sd(samp)", sdS,       y); y+=11;
                drawKV("sd(pop)",  sdP,       y); y+=11;
                drawKV("min",      mn[c],     y); y+=11;
                drawKV("max",      mx[c],     y); y+=11;
                drawKV("skew",     skew,      y); y+=11;
                drawKV("kurt",     kurt,      y); y+=11;
                lastMean = mean[c];
            } else {
                char ln[48]; snprintf(ln,sizeof(ln),"Reg  X=col%d  Y=col%d", rx+1, ry+1);
                d.setTextColor(C_ACC,C_BG); d.drawString(ln,4,y); y+=12;
                d.setTextColor(C_FG,C_BG);
                if (pN>1 && pM2x>0) {
                    double slope=pC/pM2x, intercept=pmy-slope*pmx;
                    double r=(pM2y>0)? pC/sqrt(pM2x*pM2y):0;
                    drawKV("n pairs",(double)pN, y); y+=11;
                    drawKV("slope b", slope,     y); y+=11;
                    drawKV("intcpt a",intercept, y); y+=11;
                    drawKV("r",       r,         y); y+=11;
                    drawKV("r^2",     r*r,       y); y+=11;
                    lastMean = slope;
                } else {
                    d.drawString("insufficient pair data", 4, y);
                }
            }
            while (true) {
                if (!DM.pollKeys()){ delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (st.fn && IS_ESC) return;
                if (IS_RIGHT){ page=(page+1)%pages; break; }
                if (IS_LEFT) { page=(page-1+pages)%pages; break; }
                if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')){
                    G.ans=lastMean; G.ansComplex=false; DM.notify("-> Ans", C_OK); break; }
            }
        }
    }
public:
    void run() {
        if (!G.sdOK) { DM.notify("No SD card", C_ERR); return; }
        char path[48] = "/data.csv";
        if (!DM.inputLine("CSV path:", path, sizeof(path))) return;
        char rxs[8]="0", rys[8]="0";
        if (!DM.inputLine("Reg X col(1.. ,0=skip):", rxs, sizeof(rxs))) return;
        if (!DM.inputLine("Reg Y col(1..):", rys, sizeof(rys))) return;
        resetCols();
        rx = atoi(rxs)-1; ry = atoi(rys)-1;
        SdGuard guard;
        File f = SD.open(path);
        if (!f) { DM.notify("File not found", C_ERR); return; }
        bool first = true; long rows = 0;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length()==0) continue;
            if (first) {
                first = false;
                bool hasAlpha=false;
                for (size_t k=0;k<line.length();k++)
                    if (isalpha((unsigned char)line[k])) { hasAlpha=true; break; }
                if (hasAlpha) { captureHeader(line); continue; }
            }
            int col=0, from=0;
            double rowx=0, rowy=0; bool hx=false, hy=false;
            while (col < MAXC) {
                int comma = line.indexOf(',', from);
                String tok = (comma<0)? line.substring(from) : line.substring(from, comma);
                tok.trim();
                if (col+1 > nc) nc = col+1;
                if (tok.length()>0) {
                    char* end; double v = strtod(tok.c_str(), &end);
                    if (end != tok.c_str() && *end==0) {
                        update(col, v);
                        if (col==rx){ rowx=v; hx=true; }
                        if (col==ry){ rowy=v; hy=true; }
                    }
                }
                if (comma<0) break;
                from = comma+1; col++;
            }
            if (rx>=0 && ry>=0 && hx && hy) updatePair(rowx, rowy);
            rows++;
        }
        f.close();
        if (nc==0) { DM.notify("No data", C_ERR); return; }
        showResults(rows);
    }
};
CsvStream gCsvStream;

unsigned long gLastInputT = 0;

void aboutScreen() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(C_BG); DM.header("About"); DM.footer("","","ESC:back");
    int y = HDR_H + 4;
    d.setTextSize(2);
    d.setTextColor(C_ACC, C_BG);
    d.drawString(POMAS_NAME " " POMAS_VER, 4, y); y += 22;
    d.setTextSize(1);
    d.setTextColor(C_FG, C_BG);
    d.drawString("Pocket Mathematics Station", 4, y); y += 11;
    d.setTextColor(C_DIM, C_BG);
    d.drawString("for M5 Cardputer-Adv", 4, y); y += 11;
    d.drawString("Modes: Calc/CAS/Stat", 4, y); y += 10;
    d.drawString("       SciRef/Graph/uLisp", 4, y); y += 10;
    d.drawString("       Sheet/WiFi", 4, y); y += 10;
    d.setTextColor(C_OK, C_BG);
    d.drawString(POMAS_CRED, 4, y);
    DM.waitEsc();
}

void statSubMenu() {
    while (true) {
        const char* items[] = {
            "Lists & Descriptive","Regression / Multi","Tests (z/t/F/ANOVA/chi)",
            "Non-parametric / Risk","Distributions","Multivariate (PCA/k-means/etc)",
            "Stratified (M-H)","Survival (KM/Cox)","Meta-analysis",
            "17 Statistical Graphs","CSV Stream Stats (SD)","Back"
        };
        int s = DM.menu("Statistics", items, 12);
        if (s < 0 || s == 11) return;
        switch (s) {
            case 0: gStatCore.run();    break;
            case 1: gStatRegress.run(); break;
            case 2: gStatTests.run();   break;
            case 3: gStatNonpar.run();  break;
            case 4: gDistBrowser.run(); break;
            case 5: gStatMV.run();      break;
            case 6: gStatStrata.run();  break;
            case 7: gSurvival.run();    break;
            case 8: gMeta.run();        break;
            case 9: gStatGraphs.run();  break;
            case 10: gCsvStream.run();  break;
        }
    }
}

void settingsScreen();

void toolsMenu() {
    while (true) {
        const char* items[] = {
            "Spreadsheet (45x26)","uLisp Interpreter","Wi-Fi/Maxima (srv)",
            "Power / About","Settings (NVS persistent)","Save BMP screenshot now","Back"
        };
        int s = DM.menu("Tools", items, 7);
        if (s < 0 || s == 6) return;
        switch (s) {
            case 0: gSheet.run();      break;
            case 1: gULisp.run();      break;
            case 2: gWiFi.run();       break;
            case 3: gPwr.show();       break;
            case 4: settingsScreen();  break;
            case 5: DM.saveBMPAuto("screen"); break;
        }
    }
}

void settingsScreen() {
    while (true) {
        const char* items[] = {
            "Angle: DEG/RAD/GRAD","Display: Norm/Fix/Sci/Eng","Display digits",
            "Save settings now","Reset all (clear NVS)","Back"
        };
        int s = DM.menu("Settings", items, 6);
        if (s < 0 || s == 5) return;
        if (s == 0) {
            const char* a[] = {"DEG","RAD","GRAD"};
            int x = DM.menu("Angle", a, 3, G.angle);
            if (x >= 0) G.angle = (AngleMode)x;
        }
        else if (s == 1) {
            const char* dd[] = {"Norm","Fix","Sci","Eng"};
            int x = DM.menu("Display", dd, 4, G.disp);
            if (x >= 0) G.disp = (DispMode)x;
        }
        else if (s == 2) {
            char b[8]; snprintf(b, sizeof(b), "%d", G.dispDigits);
            if (DM.inputLine("digits 0-9:", b, sizeof(b))) {
                int v = atoi(b);
                if (v >= 0 && v <= 9) G.dispDigits = v;
            }
        }
        else if (s == 3) { gNVS.save(); DM.notify("Saved to NVS", C_OK); }
        else if (s == 4) {
            if (DM.confirm("Clear NVS settings?")) {
                Preferences p; p.begin("pomas", false); p.clear(); p.end();
                DM.notify("NVS cleared", C_OK);
            }
        }
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    Serial.begin(115200);
    delay(100);
    M5Cardputer.Display.setRotation(1);
    Serial.println("[SD] Init starting...");
    G.sdOK = false;
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    uint32_t sdFreqs[] = {4000000, 10000000, 25000000};
    for (int retry = 0; retry < 3 && !G.sdOK; retry++) {
        Serial.printf("[SD] Try %d at %u Hz\n", retry+1, sdFreqs[retry]);
        if (SD.begin(SD_CS, SPI, sdFreqs[retry])) {
            G.sdOK = true;
            uint8_t cardType = SD.cardType();
            uint64_t cardSize = SD.cardSize() / (1024 * 1024);
            const char* tStr = (cardType == CARD_MMC) ? "MMC" :
                               (cardType == CARD_SD)  ? "SDSC" :
                               (cardType == CARD_SDHC)? "SDHC" : "UNKNOWN";
            Serial.printf("[SD] OK at %u Hz (Type: %s, Size: %lluMB)\n",
                          sdFreqs[retry], tStr, cardSize);
            break;
        }
        Serial.printf("[SD] Failed at %u Hz, retrying...\n", sdFreqs[retry]);
        SD.end();
        delay(200);
    }
    if (!G.sdOK) {
        Serial.println("[SD] All attempts failed");
        Serial.println("  - Card not inserted or not seated");
        Serial.println("  - Card not FAT32 format (try reformat)");
        Serial.println("  - Card > 32GB (use 32GB or smaller)");
        Serial.println("  - Card defective");
    }
    DM.begin();
    DM.splash();
    G.psramOK = psramFound();
    if (G.psramOK) G.allocStat(LIST_MAX);
    else           G.allocStat(200);
    gNVS.load();
    ULisp::initEnv();
    WiFi.mode(WIFI_OFF);
    gLastInputT = millis();
    delay(200);
}

void loop() {
    const char* items[] = {
        "Calculator","CAS","Statistics","Scientific Reference",
        "Graph","Tools (Sheet/uLisp/WiFi)","About"
    };
    int s = DM.menu(POMAS_NAME " " POMAS_VER, items, 7);
    gLastInputT = millis();
    switch (s) {
        case 0: gCalc.run();       break;
        case 1: gCAS.run();        break;
        case 2: statSubMenu();     break;
        case 3: gSciRef.run();     break;
        case 4: gGraph.run();      break;
        case 5: toolsMenu();       break;
        case 6: aboutScreen();     break;
        default: break;
    }
    if (millis() - gLastInputT > 600000UL) {
#ifdef ARDUINO_M5STACK_CARDPUTER
        M5Cardputer.Power.powerOff();
#endif
    }
    static unsigned long lastSave = 0;
    if (millis() - lastSave > 60000UL) {
        gNVS.save();
        lastSave = millis();
    }
}