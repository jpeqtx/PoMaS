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
// --- Forward decls for ctags-generated prototypes ---
enum BaseMode : int;
struct LawEntry;
// ----------------------------------------------------
#if defined(SET_LOOP_TASK_STACK_SIZE)
  SET_LOOP_TASK_STACK_SIZE(32768);
  #define POMAS_ULISP_MAX_DEPTH 400
#else
  #define POMAS_ULISP_MAX_DEPTH 100
#endif
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
#define LIST_MAX        300
#define HIST_MAX        100
#define UNDO_MAX        20
#define VAR_COUNT       10
#define MAT_MAX         10
#define VEC_MAX         10
#define STAT_LISTS      32
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
enum BaseMode : int { BASE_DEC=10, BASE_HEX=16, BASE_OCT=8, BASE_BIN=2 };
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
        if (d < 1e-300) return Complex(NAN, NAN);
        return Complex((re*o.re+im*o.im)/d, (im*o.re-re*o.im)/d);
    }
    Complex conj() const { return Complex(re, -im); }
    double  abs()  const { return sqrt(re*re + im*im); }
    double  arg()  const { return atan2(im, re); }
    static Complex fromPolar(double r, double theta) {
        return Complex(r*cos(theta), r*sin(theta));
    }
    Complex powN(double n) const { return fromPolar(::pow(abs(), n), arg()*n); }
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
    int        autoOffMin = 10;
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
struct UserFunc {
    char name[12];
    char body[64];
    char param;
    bool active;
};

#define USERFN_MAX 16
struct UserFuncBank {
    UserFunc f[USERFN_MAX];
    int n = 0;
    int find(const char* nm) const {
        for (int i = 0; i < n; i++)
            if (f[i].active && !strcmp(f[i].name, nm)) return i;
        return -1;
    }

    bool add(const char* nm, char prm, const char* bd) {
        if (n >= USERFN_MAX) return false;
        strncpy(f[n].name, nm, 11); f[n].name[11] = 0;
        strncpy(f[n].body, bd, 63); f[n].body[63] = 0;
        f[n].param = prm;
        f[n].active = true;
        n++;
        return true;
    }

    bool remove(const char* nm) {
        int i = find(nm);
        if (i < 0) return false;
        for (int k = i; k < n-1; k++) f[k] = f[k+1];
        n--;
        return true;
    }

    void clear() { n = 0; }
};

UserFuncBank gUserFns;
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

static inline void formatInt(long long v, char* out, int n, BaseMode b) {
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

inline const char* statLbl(int i) {
    static char buf[STAT_LISTS][4];
    static bool _init = false;
    if (!_init) {
        for (int k = 0; k < STAT_LISTS; k++) snprintf(buf[k], 4, "L%d", k+1);
        _init = true;
    }
    if (i < 0 || i >= STAT_LISTS) return "?";
    return buf[i];
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
class StatGraphs;
extern DisplayMgr DM;
extern ExprEval   gEv;
extern StatGraphs gStatGraphs;
void statGraphShow(int kind);
void statGraphMenu();
extern unsigned long gLastInputT;
// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 2 / 20 ─ DisplayMgr                                             ║
// ╚═══════════════════════════════════════════════════════════════════════╝
class DisplayMgr {
    static unsigned long _holdT, _repT;
public:
    void resetKeyRepeat() { _holdT = _repT = millis(); }
    inline bool pollKeys() {
        M5Cardputer.update();
        static bool escCooldown = false;
        static bool dimmed = false;
        static uint8_t savedBr = 128;
        if (escCooldown) {
            if (!M5Cardputer.Keyboard.isKeyPressed('`')) {
                escCooldown = false;
                delay(50);
            }
            return false;
        }
        unsigned long now = millis();
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isKeyPressed('`')) {
                escCooldown = true;
            }
            gLastInputT = now;
            _holdT = _repT = now;
            if (dimmed) {                       // 減光中の最初のキーは復帰のみに使う
                M5Cardputer.Display.setBrightness(savedBr);
                dimmed = false;
                return false;
            }
            return true;
        }
        if (M5Cardputer.Keyboard.isPressed()) {
            gLastInputT = now;
            // fn+矢印 / DEL の押しっぱなしは自動リピート (400ms 後に 70ms 間隔)
            auto st = M5Cardputer.Keyboard.keysState();
            bool nav = st.del || (st.fn && (IS_UP || IS_DOWN || IS_LEFT || IS_RIGHT));
            if (nav && now - _holdT > 400 && now - _repT > 70) { _repT = now; return true; }
            return false;
        }
        unsigned long idle = now - gLastInputT;
        if (!dimmed && idle > 60000UL) {        // 60秒無操作で減光
            savedBr = M5Cardputer.Display.getBrightness();
            if (savedBr < 32) savedBr = 128;
            M5Cardputer.Display.setBrightness(12);
            dimmed = true;
        }
#ifdef ARDUINO_M5STACK_CARDPUTER
        if (G.autoOffMin > 0 && idle > (unsigned long)G.autoOffMin * 60000UL) M5Cardputer.Power.powerOff();   // 600秒無操作で電源OFF (モジュール内でも有効)
#endif
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
        unsigned long nt0 = millis();
        while (millis() - nt0 < (unsigned long)ms) {      // 任意キーでスキップ
            M5Cardputer.update();
            if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isKeyPressed('`')) { gLastInputT = millis(); resetKeyRepeat(); break; }
            delay(10);
        }
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

    // ── 入力行: プロンプトをチップ化 + 2px カーソル + 行内編集 (fn+←→:移動, fn+↑↓:先頭/末尾, DEL:削除, fn+DEL:全消去) ──
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
        int len = strlen(buf), pos = len;
        const int fieldX = 8;
        const int fieldW = w - 12;
        const int maxChars = fieldW / 6;
        int start = 0;
        while (true) {
            if (pos < start) start = pos;
            if (pos > start + maxChars - 1) start = pos - maxChars + 1;
            if (start < 0) start = 0;
            char win[64];
            int wl = len - start;
            if (wl > maxChars) wl = maxChars;
            if (wl > 62) wl = 62;
            if (wl < 0) wl = 0;
            memcpy(win, buf + start, wl); win[wl] = 0;
            d.fillRect(6, yy + 14, w - 4, 16, C_PANEL);
            d.setTextColor(C_FG, C_PANEL);
            d.drawString(win, fieldX, yy + 17);
            int cx = fieldX + (pos - start) * 6;
            if (cx > LCD_W - 6) cx = LCD_W - 6;
            if (blink) d.fillRect(cx, yy + 16, 2, 12, C_ACC);
            unsigned long t0 = millis();
            while (millis() - t0 < 80) {
                if (pollKeys()) {
                    auto st = M5Cardputer.Keyboard.keysState();
                    if (IS_ESC) return false;
                    if (st.enter) return true;
                    if (st.fn && IS_LEFT)  { if (pos > 0) pos--;   blink = true; goto rd; }
                    if (st.fn && IS_RIGHT) { if (pos < len) pos++; blink = true; goto rd; }
                    if (st.fn && IS_UP)    { pos = 0;   blink = true; goto rd; }
                    if (st.fn && IS_DOWN)  { pos = len; blink = true; goto rd; }
                    if (st.fn && st.del)   { buf[0] = 0; len = pos = 0; goto rd; }
                    if (st.del) {
                        if (pos > 0) { memmove(buf + pos - 1, buf + pos, len - pos + 1); pos--; len--; }
                        goto rd;
                    }
                    if (!st.fn) {
                        for (auto c : st.word) {
                            if (len < maxLen - 1 && c >= ' ' && c < 127) {
                                char cc = (c == '_') ? '-' : ((c == '-') ? '_' : c);
                                memmove(buf + pos + 1, buf + pos, len - pos + 1);
                                buf[pos++] = cc; len++;
                            }
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

    // ── メニュー: 部分再描画 / 端で循環 / fn+←→ でページ送り / 1-9・a-z ショートカット / 選択位置の記憶 ──
    static int& menuMemo(const char* title) {
        static uint32_t keys[128];
        static int vals[128];
        static int cnt = 0;
        uint32_t h = 5381;
        for (const char* p = title; *p; p++) h = h * 33 + (uint8_t)*p;
        for (int i = 0; i < cnt; i++) if (keys[i] == h) return vals[i];
        int i = (cnt < 128) ? cnt++ : 127;
        keys[i] = h; vals[i] = 0;
        return vals[i];
    }

    int menu(const char* title, const char* items[], int n, int init = -1) {
        auto& d = M5Cardputer.Display;
        int& memo = menuMemo(title);
        int sel = (init >= 0) ? init : memo;
        if (sel < 0) sel = 0;
        if (sel >= n) sel = n - 1;
        const int rowH = 13;
        const int viewH = LCD_H - HDR_H - FTR_H;
        const int rows = viewH / rowH;
        int top = 0;
        if (sel >= rows) top = sel - rows + 1;
        auto drawRow = [&](int idx) {
            int i = idx - top;
            if (i < 0 || i >= rows || idx >= n) return;
            int y = HDR_H + i * rowH;
            bool isSel = (idx == sel);
            uint16_t bg = isSel ? C_SEL_BG : ((i & 1) ? C_BG : C_PANEL2);
            d.fillRect(0, y, LCD_W, rowH, bg);
            d.drawFastHLine(0, y + rowH - 1, LCD_W, C_LINE);
            if (isSel) d.fillRect(0, y, 3, rowH, C_SEL_BAR);
            char num[2] = { (char)(idx < 9 ? '1' + idx : (idx < 35 ? 'a' + (idx - 9) : '*')), 0 };
            d.setTextColor(isSel ? C_SEL_BAR : C_ACC, bg);
            d.drawString(num, 8, y + 3);
            d.setTextColor(isSel ? C_HDRTX : C_FG, bg);
            d.drawString(items[idx], 26, y + 3);
        };
        auto drawBar = [&]() {
            if (n <= rows) return;
            int sbx = LCD_W - 3;
            d.fillRect(sbx, HDR_H, 3, viewH, C_PANEL2);
            int thumb = (rows * viewH) / n;
            if (thumb < 12) thumb = 12;
            int ty = HDR_H + (top * (viewH - thumb)) / (n - rows);
            d.fillRect(sbx, ty, 3, thumb, C_ACC);
        };
        auto drawAll = [&]() {
            for (int i = 0; i < rows && top + i < n; i++) drawRow(top + i);
            drawBar();
        };
        d.fillScreen(C_BG);
        header(title);
        footer("ENT:OK", n > 9 ? "1-9 a-z" : "1-9", "ESC:back");
        drawAll();
        while (true) {
            if (!pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return -1;
            if (st.enter) { memo = (strcmp(items[sel], "Back") != 0) ? sel : 0; return sel; }
            int ns = sel;
            if (st.fn && IS_UP)         ns = (sel > 0) ? sel - 1 : n - 1;
            else if (st.fn && IS_DOWN)  ns = (sel < n - 1) ? sel + 1 : 0;
            else if (st.fn && IS_LEFT)  ns = (sel - rows > 0) ? sel - rows : 0;
            else if (st.fn && IS_RIGHT) ns = (sel + rows < n - 1) ? sel + rows : n - 1;
            else {
                for (auto c : st.word) {
                    int k = -1;
                    if (c >= '1' && c <= '9') k = c - '1';
                    else if (!st.fn && c >= 'a' && c <= 'z') k = 9 + (c - 'a');
                    else if (!st.fn && c >= 'A' && c <= 'Z') k = 9 + (c - 'A');
                    if (k >= 0 && k < n) { memo = (strcmp(items[k], "Back") != 0) ? k : 0; return k; }
                }
            }
            if (ns != sel) {
                int old = sel, ot = top;
                sel = ns;
                if (sel < top) top = sel;
                if (sel >= top + rows) top = sel - rows + 1;
                if (top != ot) drawAll();
                else { drawRow(old); drawRow(sel); drawBar(); }
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
                while (sp > maxCh/2 && p+sp < len && text[p+sp] != ' ') sp--;
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
        if (f.write(hdr, 54) != 54) { f.close(); return false; }
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
            if (f.write(row, rowSize) != (size_t)rowSize) { free(row); f.close(); return false; }
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

unsigned long DisplayMgr::_holdT = 0;
unsigned long DisplayMgr::_repT = 0;
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
            else if (c == '?') {
                _pos++;
                double tv = parseExpr();
                skipSpaces();
                if (_src[_pos] == ':') _pos++;
                else { setErr(": expected"); return 0; }
                double fv = parseExpr();
                return v != 0 ? tv : fv;
            }
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
            skipSpaces();
            if (_src[_pos] == '[') {
                _pos++;
                double arr[64]; int an = 0;
                skipSpaces();
                if (_src[_pos] != ']') {
                    arr[an++] = parseExpr();
                    while (_src[_pos] == ',' && an < 64) {
                        _pos++; skipSpaces();
                        arr[an++] = parseExpr();
                    }
                }
                skipSpaces();
                if (_src[_pos] == ']') _pos++;
                else setErr("] expected");
                skipSpaces();
                if (_src[_pos] == ')') _pos++;
                else setErr(") expected");
                if (!strcasecmp(name,"sum")) { double s=0; for (int i=0;i<an;i++) s+=arr[i]; return s; }
                if (!strcasecmp(name,"mean")||!strcasecmp(name,"avg")) { if (an<1) return 0; double s=0; for (int i=0;i<an;i++) s+=arr[i]; return s/an; }
                if (!strcasecmp(name,"prod")) { double p=1; for (int i=0;i<an;i++) p*=arr[i]; return p; }
                if (!strcasecmp(name,"count")) return (double)an;
                if (!strcasecmp(name,"min")) { if (an<1) return 0; double m=arr[0]; for (int i=1;i<an;i++) if (arr[i]<m) m=arr[i]; return m; }
                if (!strcasecmp(name,"max")) { if (an<1) return 0; double m=arr[0]; for (int i=1;i<an;i++) if (arr[i]>m) m=arr[i]; return m; }
                setErr("array fn unsupported");
                return 0;
            }
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
            int ufi = gUserFns.find(name);
            if (ufi >= 0) {
                if (na != 1) { setErr("user fn needs 1 arg"); return 0; }
                static int depth = 0;
                if (depth >= 32) { setErr("user fn recursion"); return 0; }
                depth++;
                ExprEval sub;
                ExprEval::Subst us = { gUserFns.f[ufi].param, a[0] };
                auto ur = sub.eval(gUserFns.f[ufi].body, &us, 1);
                depth--;
                if (!ur.ok) { setErr(ur.err ? ur.err : "user fn err"); return 0; }
                return ur.val;
            }
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
        if (na >= 1) {
            if (!strcasecmp(nm,"sum")) { double s=0; for (int i=0;i<na;i++) s+=a[i]; return s; }
            if (!strcasecmp(nm,"mean")||!strcasecmp(nm,"avg")) { double s=0; for (int i=0;i<na;i++) s+=a[i]; return s/na; }
            if (!strcasecmp(nm,"prod")) { double p=1; for (int i=0;i<na;i++) p*=a[i]; return p; }
            if (!strcasecmp(nm,"count")) return (double)na;
        }
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
            if (!strcasecmp(nm,"nrt"))   { if (a[0] == 0) return NAN; return pow(a[1], 1.0/a[0]); }
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
    if (fabs(fa) < tol) { root = a; return true; }
    if (fabs(fb) < tol) { root = b; return true; }
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

    double argAcos = R / sqrt(-Q*Q*Q);
    if (argAcos > 1.0) argAcos = 1.0;
    if (argAcos < -1.0) argAcos = -1.0;
    double th = acos(argAcos);
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
    if (y < 0) y = 0;
    double sq = sqrt(y);
    if (sq < 1e-9) { for (int i = 0; i < 4; i++) { r[i] = NAN; im[i] = NAN; } return 4; }
    double A1 =  sq, B1 = (p + y - q/sq) / 2;
    double A2 = -sq, B2 = (p + y + q/sq) / 2;
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

inline double trace(const double* a, int n) {
    double t = 0; for (int i = 0; i < n; i++) t += a[i*n+i]; return t;
}

// A^p (整数 p。負は逆行列経由)。out は n*n。
inline bool matPow(const double* a, int n, int p, double* out) {
    double base[MAT_MAX*MAT_MAX], tmp[MAT_MAX*MAT_MAX];
    if (p < 0) { if (!inv(a, n, base)) return false; p = -p; }
    else memcpy(base, a, n*n*sizeof(double));
    for (int i = 0; i < n*n; i++) out[i] = 0;
    for (int i = 0; i < n; i++) out[i*n+i] = 1;
    while (p > 0) {
        if (p & 1) { mul(out, n, n, base, n, tmp); memcpy(out, tmp, n*n*sizeof(double)); }
        p >>= 1;
        if (p) { mul(base, n, n, base, n, tmp); memcpy(base, tmp, n*n*sizeof(double)); }
    }
    return true;
}

// PA = LU (部分ピボット)。perm[i] = 元の行番号。
inline bool luDecomp(const double* a, int n, double* L, double* U, int* perm) {
    double W[MAT_MAX*MAT_MAX];
    memcpy(W, a, n*n*sizeof(double));
    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = 0; i < n*n; i++) { L[i] = 0; U[i] = 0; }
    for (int k = 0; k < n; k++) {
        int pi = k; double pv = fabs(W[k*n+k]);
        for (int r = k+1; r < n; r++) if (fabs(W[r*n+k]) > pv) { pv = fabs(W[r*n+k]); pi = r; }
        if (pv < 1e-15) return false;
        if (pi != k) {
            for (int j = 0; j < n; j++) { double t = W[k*n+j]; W[k*n+j] = W[pi*n+j]; W[pi*n+j] = t; }
            int t = perm[k]; perm[k] = perm[pi]; perm[pi] = t;
        }
        for (int r = k+1; r < n; r++) {
            W[r*n+k] /= W[k*n+k];
            for (int j = k+1; j < n; j++) W[r*n+j] -= W[r*n+k] * W[k*n+j];
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (j < i) L[i*n+j] = W[i*n+j];
            else       U[i*n+j] = W[i*n+j];
        }
        L[i*n+i] = 1;
    }
    return true;
}

// A = QR (修正グラム・シュミット、正方・列フルランク)
inline bool qrDecomp(const double* a, int n, double* Q, double* R) {
    for (int i = 0; i < n*n; i++) { Q[i] = a[i]; R[i] = 0; }
    for (int j = 0; j < n; j++) {
        for (int k = 0; k < j; k++) {
            double d = 0;
            for (int i = 0; i < n; i++) d += Q[i*n+k] * Q[i*n+j];
            R[k*n+j] = d;
            for (int i = 0; i < n; i++) Q[i*n+j] -= d * Q[i*n+k];
        }
        double nm = 0;
        for (int i = 0; i < n; i++) nm += Q[i*n+j] * Q[i*n+j];
        nm = sqrt(nm);
        if (nm < 1e-12) return false;
        R[j*n+j] = nm;
        for (int i = 0; i < n; i++) Q[i*n+j] /= nm;
    }
    return true;
}

// 特性多項式 (Faddeev-LeVerrier)。c[k] = λ^k の係数、c[n] = 1。c は n+1 要素。
inline void charPoly(const double* a, int n, double* c) {
    double M[MAT_MAX*MAT_MAX], AM[MAT_MAX*MAT_MAX];
    for (int i = 0; i < n*n; i++) M[i] = 0;
    c[n] = 1;
    for (int k = 1; k <= n; k++) {
        mul(a, n, n, M, n, AM);
        for (int i = 0; i < n; i++) AM[i*n+i] += c[n-k+1];
        memcpy(M, AM, n*n*sizeof(double));
        mul(a, n, n, M, n, AM);
        c[n-k] = -trace(AM, n) / k;
    }
}

// 実固有値 lam に対する固有ベクトル (逆反復法、正規化済み)
inline bool eigVec(const double* a, int n, double lam, double* v) {
    double B[MAT_MAX*MAT_MAX], x[MAT_MAX];
    double shift = lam + 1e-7 * (1.0 + fabs(lam));
    for (int i = 0; i < n; i++) x[i] = 1.0 / sqrt((double)n) + 0.01 * i;
    for (int it = 0; it < 8; it++) {
        memcpy(B, a, n*n*sizeof(double));
        for (int i = 0; i < n; i++) B[i*n+i] -= shift;
        if (!gaussSolve(B, n, x)) return false;
        double nm = 0;
        for (int i = 0; i < n; i++) nm += x[i]*x[i];
        nm = sqrt(nm);
        if (!(nm > 1e-300 && nm < 1e300)) return false;
        for (int i = 0; i < n; i++) x[i] /= nm;
    }
    for (int i = 0; i < n; i++) v[i] = x[i];
    return true;
}

// 複素固有値 lamR+lamI*i に対する固有ベクトル (実2n系逆反復)。vR/vI は正規化済み。
inline bool eigVecC(const double* a, int n, double lamR, double lamI, double* vR, double* vI) {
    int N = 2*n;
    double* M = (double*)malloc(N*N*sizeof(double));
    double rhs[2*MAT_MAX];
    if (!M) return false;
    double shift = 1e-7 * (1.0 + fabs(lamR));
    for (int i = 0; i < n; i++) { vR[i] = 1.0/sqrt((double)n) + 0.01*i; vI[i] = 0.01*i; }
    for (int it = 0; it < 8; it++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                double ad = a[i*n+j] - (i==j ? (lamR + shift) : 0.0);
                M[i*N+j]       = ad;
                M[i*N+n+j]     = (i==j ? lamI : 0.0);
                M[(n+i)*N+j]   = (i==j ? -lamI : 0.0);
                M[(n+i)*N+n+j] = ad;
            }
            rhs[i] = vR[i];
            rhs[n+i] = vI[i];
        }
        if (!gaussSolve(M, N, rhs)) { free(M); return false; }
        double nm = 0;
        for (int i = 0; i < N; i++) nm += rhs[i]*rhs[i];
        nm = sqrt(nm);
        if (!(nm > 1e-300 && nm < 1e300)) { free(M); return false; }
        for (int i = 0; i < N; i++) rhs[i] /= nm;
        for (int i = 0; i < n; i++) { vR[i] = rhs[i]; vI[i] = rhs[n+i]; }
    }
    free(M);
    return true;
}
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
    double lo = -1e9, hi = 1e9;
    for (int i = 0; i < 200; i++) {
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
    // ── 数学定数（追加20項目） ──
    {"Apéry constant zeta(3)",            "zeta3",   1.20205690315959428540, "",             "Math"},
    {"Catalan constant",                  "G",       0.91596559417721901505, "",             "Math"},
    {"Glaisher-Kinkelin constant",        "A",       1.28242712910062263688, "",             "Math"},
    {"Khinchin constant",                 "K0",      2.68545200106530644530, "",             "Math"},
    {"Twin prime constant",               "C2",      0.66016181584686957393, "",             "Math"},
    {"Meissel-Mertens constant",          "M",       0.26149721284764278375, "",             "Math"},
    {"Brun constant (twin primes)",       "B2",      1.902160583104,         "",             "Math"},
    {"Feigenbaum delta",                  "delta",   4.66920160910299067185, "",             "Math"},
    {"Feigenbaum alpha",                  "alphaF",  2.50290787509589282228, "",             "Math"},
    {"Conway constant",                   "lambda",  1.30357726903429639125, "",             "Math"},
    {"Lemniscate constant",               "varpi",   2.62205755429211981046, "",             "Math"},
    {"Silver ratio",                      "deltaS",  2.41421356237309504880, "",             "Math"},
    {"Plastic number",                    "rho",     1.32471795724474602596, "",             "Math"},
    {"Tribonacci constant",               "T",       1.83928675521416113255, "",             "Math"},
    {"Supergolden ratio",                 "psi",     1.46557123187676802665, "",             "Math"},
    {"Somos quadratic recurrence",        "sigma",   1.66168794963359412129, "",             "Math"},
    {"Mills constant",                    "A_M",     1.30637788386308069046, "",             "Math"},
    {"MRB constant",                      "MRB",     0.18785964246206712024, "",             "Math"},
    {"Omega constant (W(1))",             "Omega",   0.56714329040978387299, "",             "Math"},
    {"Dottie number",                     "D",       0.73908513321516064165, "",             "Math"},
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

struct ElementInfo {
    const char* nameEn;
    const char* config;
    uint8_t     group;
    uint8_t     period;
};

static const ElementInfo ELEM_INFO[118] PROGMEM = {
    {"Hydrogen","1s1",1,1},
    {"Helium","1s2",18,1},
    {"Lithium","[He]2s1",1,2},
    {"Beryllium","[He]2s2",2,2},
    {"Boron","[He]2s2 2p1",13,2},
    {"Carbon","[He]2s2 2p2",14,2},
    {"Nitrogen","[He]2s2 2p3",15,2},
    {"Oxygen","[He]2s2 2p4",16,2},
    {"Fluorine","[He]2s2 2p5",17,2},
    {"Neon","[He]2s2 2p6",18,2},
    {"Sodium","[Ne]3s1",1,3},
    {"Magnesium","[Ne]3s2",2,3},
    {"Aluminum","[Ne]3s2 3p1",13,3},
    {"Silicon","[Ne]3s2 3p2",14,3},
    {"Phosphorus","[Ne]3s2 3p3",15,3},
    {"Sulfur","[Ne]3s2 3p4",16,3},
    {"Chlorine","[Ne]3s2 3p5",17,3},
    {"Argon","[Ne]3s2 3p6",18,3},
    {"Potassium","[Ar]4s1",1,4},
    {"Calcium","[Ar]4s2",2,4},
    {"Scandium","[Ar]3d1 4s2",3,4},
    {"Titanium","[Ar]3d2 4s2",4,4},
    {"Vanadium","[Ar]3d3 4s2",5,4},
    {"Chromium","[Ar]3d5 4s1",6,4},
    {"Manganese","[Ar]3d5 4s2",7,4},
    {"Iron","[Ar]3d6 4s2",8,4},
    {"Cobalt","[Ar]3d7 4s2",9,4},
    {"Nickel","[Ar]3d8 4s2",10,4},
    {"Copper","[Ar]3d10 4s1",11,4},
    {"Zinc","[Ar]3d10 4s2",12,4},
    {"Gallium","[Ar]3d10 4s2 4p1",13,4},
    {"Germanium","[Ar]3d10 4s2 4p2",14,4},
    {"Arsenic","[Ar]3d10 4s2 4p3",15,4},
    {"Selenium","[Ar]3d10 4s2 4p4",16,4},
    {"Bromine","[Ar]3d10 4s2 4p5",17,4},
    {"Krypton","[Ar]3d10 4s2 4p6",18,4},
    {"Rubidium","[Kr]5s1",1,5},
    {"Strontium","[Kr]5s2",2,5},
    {"Yttrium","[Kr]4d1 5s2",3,5},
    {"Zirconium","[Kr]4d2 5s2",4,5},
    {"Niobium","[Kr]4d4 5s1",5,5},
    {"Molybdenum","[Kr]4d5 5s1",6,5},
    {"Technetium","[Kr]4d5 5s2",7,5},
    {"Ruthenium","[Kr]4d7 5s1",8,5},
    {"Rhodium","[Kr]4d8 5s1",9,5},
    {"Palladium","[Kr]4d10",10,5},
    {"Silver","[Kr]4d10 5s1",11,5},
    {"Cadmium","[Kr]4d10 5s2",12,5},
    {"Indium","[Kr]4d10 5s2 5p1",13,5},
    {"Tin","[Kr]4d10 5s2 5p2",14,5},
    {"Antimony","[Kr]4d10 5s2 5p3",15,5},
    {"Tellurium","[Kr]4d10 5s2 5p4",16,5},
    {"Iodine","[Kr]4d10 5s2 5p5",17,5},
    {"Xenon","[Kr]4d10 5s2 5p6",18,5},
    {"Cesium","[Xe]6s1",1,6},
    {"Barium","[Xe]6s2",2,6},
    {"Lanthanum","[Xe]5d1 6s2",3,6},
    {"Cerium","[Xe]4f1 5d1 6s2",3,6},
    {"Praseodymium","[Xe]4f3 6s2",3,6},
    {"Neodymium","[Xe]4f4 6s2",3,6},
    {"Promethium","[Xe]4f5 6s2",3,6},
    {"Samarium","[Xe]4f6 6s2",3,6},
    {"Europium","[Xe]4f7 6s2",3,6},
    {"Gadolinium","[Xe]4f7 5d1 6s2",3,6},
    {"Terbium","[Xe]4f9 6s2",3,6},
    {"Dysprosium","[Xe]4f10 6s2",3,6},
    {"Holmium","[Xe]4f11 6s2",3,6},
    {"Erbium","[Xe]4f12 6s2",3,6},
    {"Thulium","[Xe]4f13 6s2",3,6},
    {"Ytterbium","[Xe]4f14 6s2",3,6},
    {"Lutetium","[Xe]4f14 5d1 6s2",3,6},
    {"Hafnium","[Xe]4f14 5d2 6s2",4,6},
    {"Tantalum","[Xe]4f14 5d3 6s2",5,6},
    {"Tungsten","[Xe]4f14 5d4 6s2",6,6},
    {"Rhenium","[Xe]4f14 5d5 6s2",7,6},
    {"Osmium","[Xe]4f14 5d6 6s2",8,6},
    {"Iridium","[Xe]4f14 5d7 6s2",9,6},
    {"Platinum","[Xe]4f14 5d9 6s1",10,6},
    {"Gold","[Xe]4f14 5d10 6s1",11,6},
    {"Mercury","[Xe]4f14 5d10 6s2",12,6},
    {"Thallium","[Xe]4f14 5d10 6s2 6p1",13,6},
    {"Lead","[Xe]4f14 5d10 6s2 6p2",14,6},
    {"Bismuth","[Xe]4f14 5d10 6s2 6p3",15,6},
    {"Polonium","[Xe]4f14 5d10 6s2 6p4",16,6},
    {"Astatine","[Xe]4f14 5d10 6s2 6p5",17,6},
    {"Radon","[Xe]4f14 5d10 6s2 6p6",18,6},
    {"Francium","[Rn]7s1",1,7},
    {"Radium","[Rn]7s2",2,7},
    {"Actinium","[Rn]6d1 7s2",3,7},
    {"Thorium","[Rn]6d2 7s2",3,7},
    {"Protactinium","[Rn]5f2 6d1 7s2",3,7},
    {"Uranium","[Rn]5f3 6d1 7s2",3,7},
    {"Neptunium","[Rn]5f4 6d1 7s2",3,7},
    {"Plutonium","[Rn]5f6 7s2",3,7},
    {"Americium","[Rn]5f7 7s2",3,7},
    {"Curium","[Rn]5f7 6d1 7s2",3,7},
    {"Berkelium","[Rn]5f9 7s2",3,7},
    {"Californium","[Rn]5f10 7s2",3,7},
    {"Einsteinium","[Rn]5f11 7s2",3,7},
    {"Fermium","[Rn]5f12 7s2",3,7},
    {"Mendelevium","[Rn]5f13 7s2",3,7},
    {"Nobelium","[Rn]5f14 7s2",3,7},
    {"Lawrencium","[Rn]5f14 7s2 7p1",3,7},
    {"Rutherfordium","[Rn]5f14 6d2 7s2",4,7},
    {"Dubnium","[Rn]5f14 6d3 7s2",5,7},
    {"Seaborgium","[Rn]5f14 6d4 7s2",6,7},
    {"Bohrium","[Rn]5f14 6d5 7s2",7,7},
    {"Hassium","[Rn]5f14 6d6 7s2",8,7},
    {"Meitnerium","[Rn]5f14 6d7 7s2",9,7},
    {"Darmstadtium","[Rn]5f14 6d8 7s2",10,7},
    {"Roentgenium","[Rn]5f14 6d9 7s2",11,7},
    {"Copernicium","[Rn]5f14 6d10 7s2",12,7},
    {"Nihonium","[Rn]5f14 6d10 7s2 7p1",13,7},
    {"Flerovium","[Rn]5f14 6d10 7s2 7p2",14,7},
    {"Moscovium","[Rn]5f14 6d10 7s2 7p3",15,7},
    {"Livermorium","[Rn]5f14 6d10 7s2 7p4",16,7},
    {"Tennessine","[Rn]5f14 6d10 7s2 7p5",17,7},
    {"Oganesson","[Rn]5f14 6d10 7s2 7p6",18,7}
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
static inline const LawEntry* lawAt(int idx) {
    if (idx < lawN)            return &lawDB[idx];
    if (idx < lawN + lawN2)    return &lawDB2[idx - lawN];
    return nullptr;
}

static const int LAW_TOTAL = lawN + lawN2;
struct LawCalc {
    const char* name;
    const char* v;
    const char* f[4];
};

static const LawCalc lawCalcDB[] = {
    {"Newton 2nd: F = m*a", "Fma", {"m*a", "F/a", "F/m", ""}},
    {"Ohm: V = I*R", "VIR", {"I*R", "V/R", "V/I", ""}},
    {"Kinetic: E = m*v^2/2", "Emv", {"m*v^2/2", "2*E/v^2", "sqrt(2*E/m)", ""}},
    {"Elec Power: P = V*I", "PVI", {"V*I", "P/I", "P/V", ""}},
    {"Density: rho = m/V", "rmV", {"m/V", "r*V", "m/r", ""}},
    {"Power: P = I^2*R", "PIR", {"I^2*R", "sqrt(P/R)", "P/I^2", ""}},
    {"Wave: v = f*L", "vfL", {"f*L", "v/L", "v/f", ""}},
    {"Freq: f = 1/T", "fT", {"1/T", "1/f", "", ""}},
    {"Stefan: j = sigma*T^4", "jT", {"5.670374419e-8*T^4", "(j/5.670374419e-8)^0.25", "", ""}},
    {"Wien: L = b/T", "LT", {"2.897771955e-3/T", "2.897771955e-3/L", "", ""}},
    {"Einstein: E = m*c^2", "Em", {"m*89875517873681764", "E/89875517873681764", "", ""}},
    {"Coulomb: F = k*q^2/r^2", "Fqr", {"8.987551787e9*q^2/r^2", "sqrt(F*r^2/8.987551787e9)", "sqrt(8.987551787e9*q^2/F)", ""}},
    {"Photon: E = h*f", "Ef", {"6.62607015e-34*f", "E/6.62607015e-34", "", ""}},
    {"de Broglie: L = h/p", "Lp", {"6.62607015e-34/p", "6.62607015e-34/L", "", ""}},
    {"Half-life: t = ln2/k", "tk", {"0.693147180559945/k", "0.693147180559945/t", "", ""}}
};

static const int lawCalcN = sizeof(lawCalcDB) / sizeof(LawCalc);
// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 9 / 20 ─ SciRef モード                                          ║
// ╚═══════════════════════════════════════════════════════════════════════╝
    struct DimVec {
        int8_t d[7];
        DimVec() { memset(d, 0, 7); }
        DimVec(int8_t L, int8_t M, int8_t T, int8_t I=0, int8_t Th=0, int8_t N=0, int8_t J=0) {
            d[0]=L; d[1]=M; d[2]=T; d[3]=I; d[4]=Th; d[5]=N; d[6]=J;
        }
        bool operator==(const DimVec& o) const { for(int i=0;i<7;i++) if(d[i]!=o.d[i]) return false; return true; }
        bool operator!=(const DimVec& o) const { return !(*this==o); }
        DimVec operator+(const DimVec& o) const { DimVec r; for(int i=0;i<7;i++) r.d[i]=d[i]+o.d[i]; return r; }
        DimVec operator-(const DimVec& o) const { DimVec r; for(int i=0;i<7;i++) r.d[i]=d[i]-o.d[i]; return r; }
        DimVec scale(int n) const { DimVec r; for(int i=0;i<7;i++) r.d[i]=d[i]*n; return r; }
        bool isDimless() const { for(int i=0;i<7;i++) if(d[i]) return false; return true; }
        void toString(char* out, int len) const {
            static const char* nm[7] = {"L","M","T","I","Th","N","J"};
            out[0]=0; bool first=true;
            for (int i=0;i<7;i++) {
                if (!d[i]) continue;
                if (!first) strncat(out, "*", len-strlen(out)-1);
                char tk[12];
                if (d[i]==1) snprintf(tk, sizeof(tk), "%s", nm[i]);
                else         snprintf(tk, sizeof(tk), "%s^%d", nm[i], (int)d[i]);
                strncat(out, tk, len-strlen(out)-1);
                first = false;
            }
            if (first) snprintf(out, len, "1");
        }
    };
    struct Quantity {
        double v;
        DimVec dim;
        Quantity() : v(0) {}
        Quantity(double _v, DimVec _d) : v(_v), dim(_d) {}
    };
    struct UnitDim { const char* name; double scale; DimVec dim; };
    static const UnitDim unitDimDB[] = {
        {"m",1.0,{1,0,0}}, {"km",1000.0,{1,0,0}}, {"cm",0.01,{1,0,0}}, {"mm",0.001,{1,0,0}},
        {"in",0.0254,{1,0,0}}, {"ft",0.3048,{1,0,0}}, {"mi",1609.344,{1,0,0}},
        {"kg",1.0,{0,1,0}}, {"g",0.001,{0,1,0}}, {"mg",1e-6,{0,1,0}}, {"t",1000.0,{0,1,0}},
        {"lb",0.45359237,{0,1,0}}, {"oz",0.028349523125,{0,1,0}},
        {"s",1.0,{0,0,1}}, {"ms",0.001,{0,0,1}}, {"min",60.0,{0,0,1}}, {"h",3600.0,{0,0,1}},
        {"A",1.0,{0,0,0,1}}, {"mA",0.001,{0,0,0,1}},
        {"K",1.0,{0,0,0,0,1}}, {"mol",1.0,{0,0,0,0,0,1}}, {"cd",1.0,{0,0,0,0,0,0,1}},
        {"N",1.0,{1,1,-2}}, {"J",1.0,{2,1,-2}}, {"kJ",1000.0,{2,1,-2}},
        {"cal",4.184,{2,1,-2}}, {"kcal",4184.0,{2,1,-2}}, {"eV",1.602176634e-19,{2,1,-2}},
        {"W",1.0,{2,1,-3}}, {"kW",1000.0,{2,1,-3}}, {"hp",745.7,{2,1,-3}},
        {"Pa",1.0,{-1,1,-2}}, {"bar",100000.0,{-1,1,-2}}, {"atm",101325.0,{-1,1,-2}},
        {"Hz",1.0,{0,0,-1}}, {"kHz",1000.0,{0,0,-1}},
        {"C",1.0,{0,0,1,1}}, {"V",1.0,{2,1,-3,-1}}, {"ohm",1.0,{2,1,-3,-2}},
        {"L",0.001,{3,0,0}}, {"mL",1e-6,{3,0,0}},
        {"m2",1.0,{2,0,0}}, {"kph",0.277777777,{1,0,-1}}, {"mph",0.44704,{1,0,-1}},
        {"kn",0.514444444,{1,0,-1}}
    };
    static const int unitDimN = sizeof(unitDimDB) / sizeof(UnitDim);
    // 10-1: 次元解析用ベクトル (L, M, T, I, Th, N, J)
class SciRef {
public:
    void run() {
        while (true) {
            const char* items[] = {
                "Constants (386)",
                "Scientific Laws (300)",
                "Unit Converter (532)",
                "Periodic Table (118)",
                "Law Auto-Calc (15)",
                "Dim Analysis (units)",
                "Search by Symbol",
                "Back"
            };
            int s = DM.menu("SciRef", items, 8);
            if (s < 0 || s == 7) return;
            switch (s) {
                case 0: browseConstants(); break;
                case 1: browseLaws();      break;
                case 2: unitConvert();     break;
                case 3: periodicTable();   break;
                case 4: lawAutoCalcMenu(); break;
                case 5: dimAnalysisMenu(); break;
                case 6: searchSymbol();    break;
            }
        }
    }
private:
    void lawAutoCalcMenu() {
        const char* L[32];
        for (int i = 0; i < lawCalcN && i < 32; i++) L[i] = lawCalcDB[i].name;
        int s = DM.menu("Law Auto-Calc", L, lawCalcN);
        if (s < 0) return;
        lawAutoCalc(s);
    }

    void lawAutoCalc(int idx) {
        const char* v = lawCalcDB[idx].v;
        int nv = strlen(v);
        char lb[4][10]; const char* L[4];
        for (int i = 0; i < nv; i++) { snprintf(lb[i], sizeof(lb[i]), "solve %c", v[i]); L[i] = lb[i]; }
        int tg = DM.menu("Solve for", L, nv);
        if (tg < 0) return;
        ExprEval::Subst s[4]; int ns = 0;
        char b[20];
        for (int i = 0; i < nv; i++) {
            if (i == tg) continue;
            strcpy(b, "0");
            char p[8]; snprintf(p, sizeof(p), "%c =", v[i]);
            if (!DM.inputLine(p, b, sizeof(b))) return;
            s[ns].var = v[i];
            s[ns].val = atof(b);
            ns++;
        }
        auto r = gEv.eval(lawCalcDB[idx].f[tg], s, ns);
        if (!r.ok) { DM.notify("eval fail", C_ERR); return; }
        char vs[24]; formatNum(r.val, vs, sizeof(vs));
        char m[48]; snprintf(m, sizeof(m), "%c = %s", v[tg], vs);
        DM.notify(m, C_OK, 2000);
        G.ans = r.val;
    }

    static void dSkip(const char*& p) { while (*p == ' ' || *p == '\t') p++; }
    static int findUnitAt(const char* p, int& ulen, double& prefixScale) {
        int best = -1, bestLen = 0;
        prefixScale = 1.0;
        for (int i = 0; i < unitDimN; i++) {
            int n = strlen(unitDimDB[i].name);
            if (n <= bestLen) continue;
            if (strncmp(p, unitDimDB[i].name, n) != 0) continue;
            char nc = p[n];
            if (nc && (isalnum((unsigned char)nc) || nc == '_')) continue;
            best = i; bestLen = n;
        }
        if (best >= 0) { ulen = bestLen; return best; }
        static const char* baseUnits[] = {"m","g","s","A","K","mol","cd","N","J","W","Pa","Hz","C","V","ohm","L",0};
        auto tryPfx = [&](int prefLen, double pf) -> int {
            for (int i = 0; baseUnits[i]; i++) {
                const char* bu = baseUnits[i];
                int bl = strlen(bu);
                if (strncmp(p + prefLen, bu, bl) != 0) continue;
                char nc = p[prefLen + bl];
                if (nc && (isalnum((unsigned char)nc) || nc == '_')) continue;
                for (int k = 0; k < unitDimN; k++) {
                    if (strcmp(unitDimDB[k].name, bu) == 0) {
                        ulen = prefLen + bl;
                        prefixScale = pf;
                        return k;
                    }
                }
            }
            return -1;
        };
        if (p[0] == 'd' && p[1] == 'a') { int r = tryPfx(2, 1e1); if (r >= 0) return r; }
        static const char* pfx = "YZEPTGMkhdcmunpfazy";
        static const double fac[] = {1e24,1e21,1e18,1e15,1e12,1e9,1e6,1e3,1e2,1e-1,1e-2,1e-3,1e-6,1e-9,1e-12,1e-15,1e-18,1e-21,1e-24};
        if (p[0] && p[1]) {
            for (int pi = 0; pfx[pi]; pi++) {
                if (pfx[pi] == p[0]) {
                    int r = tryPfx(1, fac[pi]);
                    if (r >= 0) return r;
                    break;
                }
            }
        }
        ulen = 0;
        return -1;
    }

    static Quantity dParseExpr(const char*& p, bool& err);
    static Quantity dParseAtom(const char*& p, bool& err) {
        dSkip(p);
        if (*p == '(') {
            p++;
            Quantity q = dParseExpr(p, err);
            dSkip(p);
            if (*p == ')') p++; else err = true;
            return q;
        }
        if (*p == '+' || *p == '-') {
            bool neg = (*p == '-'); p++;
            Quantity q = dParseAtom(p, err);
            if (neg) q.v = -q.v;
            return q;
        }
        char* end;
        double v = strtod(p, &end);
        if (end == p) { err = true; return Quantity(0, DimVec()); }
        p = end;
        dSkip(p);
        int ulen = 0;
        double prefixScale = 1.0;
        int ui = findUnitAt(p, ulen, prefixScale);
        if (ui < 0) return Quantity(v, DimVec());
        p += ulen;
        double unitScale = unitDimDB[ui].scale * prefixScale;
        DimVec unitDim = unitDimDB[ui].dim;
        dSkip(p);
        if (*p == '^') {
            p++;
            char* e2;
            long n = strtol(p, &e2, 10);
            if (e2 == p) { err = true; return Quantity(0, DimVec()); }
            p = e2;
            if (n < -9 || n > 9) { err = true; return Quantity(0, DimVec()); }
            unitScale = pow(unitScale, (double)n);
            unitDim = unitDim.scale((int)n);
        }
        return Quantity(v * unitScale, unitDim);
    }

    static Quantity dParsePower(const char*& p, bool& err) {
        Quantity a = dParseAtom(p, err);
        if (err) return a;
        dSkip(p);
        if (*p == '^') {
            p++;
            char* end;
            long n = strtol(p, &end, 10);
            if (end == p) { err = true; return a; }
            p = end;
            if (n < -9 || n > 9) { err = true; return a; }
            a.v = pow(a.v, (double)n);
            a.dim = a.dim.scale((int)n);
        }
        return a;
    }

    static Quantity dParseTerm(const char*& p, bool& err) {
        Quantity a = dParsePower(p, err);
        while (!err) {
            dSkip(p);
            if (*p == '*') {
                p++;
                Quantity b = dParsePower(p, err);
                if (err) return a;
                a.v *= b.v;
                a.dim = a.dim + b.dim;
            } else if (*p == '/') {
                p++;
                Quantity b = dParsePower(p, err);
                if (err) return a;
                if (b.v == 0) { err = true; return a; }
                a.v /= b.v;
                a.dim = a.dim - b.dim;
            } else break;
        }
        return a;
    }

    void doDimEval(const char* s) {
        bool err = false;
        const char* p = s;
        Quantity q = dParseExpr(p, err);
        dSkip(p);
        if (err || *p) { DM.notify(err ? "dim/syntax mismatch" : "trailing chars", C_ERR, 1200); return; }
        char buf[64];
        if (q.dim.isDimless()) {
            snprintf(buf, sizeof(buf), "= %.6g (dimensionless)", q.v);
        } else {
            char ds[32]; q.dim.toString(ds, sizeof(ds));
            snprintf(buf, sizeof(buf), "= %.6g [%s]", q.v, ds);
        }
        DM.notify(buf, C_OK, 2500);
        G.ans = q.v; G.ansComplex = false;
    }

    void dimAnalysisMenu() {
        char b[60] = "3 m + 2 ft";
        if (!DM.inputLine("expr (e.g. 3 m + 2 ft):", b, sizeof(b))) return;
        doDimEval(b);
    }

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
            if (!labels) return;
            for (int i = 0; i < n; i++) {
                Constant c; memcpy_P(&c, &constDB[idx[i]], sizeof(Constant));
                labels[i] = (char*)malloc(48); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
                if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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
            if (IS_ESC) return;
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
            if (!labels) return;
            for (int i = 0; i < n; i++) {
                LawEntry tmp; memcpy_P(&tmp, lawAt(idx[i]), sizeof(LawEntry));
                labels[i] = (char*)malloc(48); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
                if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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
            if (IS_ESC) return;
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
            labels[i] = (char*)malloc(40); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); DM.notify("malloc fail", C_ERR); return; }
            if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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
                if (IS_ESC) break;
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
                if (IS_ESC) return;
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
        ElementInfo ei; memcpy_P(&ei, &ELEM_INFO[z], sizeof(ElementInfo));
        const char* cls;
        if (z == 0)                    cls = "Nonmetal";
        else if (z >= 56 && z <= 70)   cls = "Lanthanide";
        else if (z >= 88 && z <= 102)  cls = "Actinide";
        else if (ei.group == 1)        cls = "Alkali metal";
        else if (ei.group == 2)        cls = "Alkaline earth";
        else if (ei.group >= 3 && ei.group <= 12) cls = "Transition metal";
        else if (ei.group == 17)       cls = "Halogen";
        else if (ei.group == 18)       cls = "Noble gas";
        else if (ei.group >= 13)       cls = "Post-transition";
        else                           cls = "Other";
        d.fillScreen(C_BG);
        char title[28]; snprintf(title, sizeof(title), "Z=%d %s", z+1, PERIODIC[z]);
        DM.header(title);
        DM.footer("Y:weight->Ans", "", "ESC:back");
        d.setTextSize(3);
        d.setTextColor(C_ACC, C_BG);
        d.drawString(PERIODIC[z], 4, HDR_H + 2);
        d.setTextSize(1);
        d.setTextColor(C_FG, C_BG);
        d.drawString(ei.nameEn, 4, HDR_H + 26);
        char ws[36]; snprintf(ws, sizeof(ws), "Mass: %.5g g/mol", ATOMIC_W[z]);
        d.setTextColor(C_OK, C_BG);
        d.drawString(ws, 4, HDR_H + 40);
        d.setTextColor(C_DIM, C_BG);
        char cfg[44]; snprintf(cfg, sizeof(cfg), "Conf: %s", ei.config);
        d.drawString(cfg, 4, HDR_H + 52);
        char gp[48]; snprintf(gp, sizeof(gp), "Group %d  Period %d", ei.group, ei.period);
        d.drawString(gp, 4, HDR_H + 64);
        d.setTextColor(C_ACC2, C_BG);
        d.drawString(cls, 4, HDR_H + 76);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
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
            labels[i] = (char*)malloc(40); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
            if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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

Quantity SciRef::dParseExpr(const char*& p, bool& err) {
    Quantity a = dParseTerm(p, err);
    while (!err) {
        dSkip(p);
        if (*p == '+') {
            p++;
            Quantity b = dParseTerm(p, err);
            if (err) return a;
            if (a.dim != b.dim) { err = true; return a; }
            a.v += b.v;
        } else if (*p == '-') {
            p++;
            Quantity b = dParseTerm(p, err);
            if (err) return a;
            if (a.dim != b.dim) { err = true; return a; }
            a.v -= b.v;
        } else break;
    }
    return a;
}

// ╔═══════════════════════════════════════════════════════════════════════╗
// ║  Part 10 / 20 ─ Calculator モード コア部                              ║
// ╚═══════════════════════════════════════════════════════════════════════╝
class CasCommands {
public:
    static bool eqIC(const char* a, const char* b) {
        while (*a && *b) { if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false; a++; b++; }
        return *a == 0 && *b == 0;
    }

    bool run(const char* cmd, const char* arg, char* out, int outLen) {
        if (eqIC(cmd, "Factor"))         return doFactorRational(arg, out, outLen);
        if (eqIC(cmd, "Expand"))         return doExpand(arg, out, outLen);
        if (eqIC(cmd, "completeSquare")) return doCompleteSquare(arg, out, outLen);
        if (eqIC(cmd, "Derivative"))     return doDerivative(arg, out, outLen);
        if (eqIC(cmd, "cFactor"))        return doCFactor(arg, out, outLen);
        if (eqIC(cmd, "tExpand"))        return doTExpand(arg, out, outLen);
        if (eqIC(cmd, "tCollect"))       return doTCollect(arg, out, outLen);
        if (eqIC(cmd, "getNum"))         return doGetNum(arg, out, outLen);
        if (eqIC(cmd, "getDenom"))       return doGetDenom(arg, out, outLen);
        if (eqIC(cmd, "propFrac"))       return doPropFrac(arg, out, outLen);
        if (eqIC(cmd, "comDenom"))       return doComDenom(arg, out, outLen);
        if (eqIC(cmd, "Left"))           return doLeft(arg, out, outLen);
        if (eqIC(cmd, "Right"))          return doRight(arg, out, outLen);
        if (eqIC(cmd, "cSolve"))         return doCSolve(arg, out, outLen);
        if (eqIC(cmd, "cZeros"))         return doCSolve(arg, out, outLen);
        if (eqIC(cmd, "toLn"))           return doToLn(arg, out, outLen);
        if (eqIC(cmd, "toExp"))          return doToExp(arg, out, outLen);
        if (eqIC(cmd, "toSin"))          return doToSin(arg, out, outLen);
        if (eqIC(cmd, "toCos"))          return doToCos(arg, out, outLen);
        if (eqIC(cmd, "toLogbase"))      return doToLogbase(arg, out, outLen);
        if (eqIC(cmd, "deSolve"))        return doDeSolve(arg, out, outLen);
        if (eqIC(cmd, "deSolve2"))       return doDeSolve2(arg, out, outLen);
        if (eqIC(cmd, "RK4"))            return doRK4(arg, out, outLen);
        if (eqIC(cmd, "ImpDiff"))        return doImpDiff(arg, out, outLen);
        if (eqIC(cmd, "implicitDiff"))   return doImpDiff(arg, out, outLen);
        if (eqIC(cmd, "Taylor"))         return doTaylor(arg, out, outLen);
        if (eqIC(cmd, "Tangent"))        return doTangent(arg, out, outLen);
        if (eqIC(cmd, "Normal"))         return doNormal(arg, out, outLen);
        if (eqIC(cmd, "ArcLength"))      return doArcLength(arg, out, outLen);
        if (eqIC(cmd, "fMin"))           return doFMin(arg, out, outLen);
        if (eqIC(cmd, "fMax"))           return doFMax(arg, out, outLen);
        if (eqIC(cmd, "DiffQ"))          return doDiffQ(arg, out, outLen);
        if (eqIC(cmd, "Series"))         return doSeries(arg, out, outLen);
        if (eqIC(cmd, "LeadingTerm"))    return doLeadingTerm(arg, out, outLen);
        return false;
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
                char* end; coef = strtod(q, &end); q = end; sawCoef = true;
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

        bool doExpand(const char* s, char* out, int outLen) {
        int len = strlen(s);
        int i = 0; while (i < len && s[i]==' ') i++;
        if (i >= len || s[i] != '(') {
            Polynomial p;
            if (parsePoly(s, p)) { p.toString(out, outLen); return true; }
            return false;
        }
        Polynomial acc; acc.c[0] = 1; acc.deg = 0;
        bool any = false;
        while (i < len) {
            while (i < len && (s[i]==' ' || s[i]=='*')) i++;
            if (i >= len) break;
            if (s[i] != '(') return false;
            int depth = 0, close = -1;
            for (int k = i; k < len; k++) {
                if (s[k]=='(') depth++;
                else if (s[k]==')') { depth--; if (depth==0) { close = k; break; } }
            }
            if (close < 0) return false;
            char inner[80]; int il = close - i - 1;
            if (il < 0 || il >= 80) return false;
            memcpy(inner, s+i+1, il); inner[il] = 0;
            Polynomial p;
            if (!parsePoly(inner, p)) return false;
            int n = 1, j = close + 1;
            while (j < len && s[j]==' ') j++;
            if (j < len && s[j]=='^') { j++; n = atoi(s+j); if (n < 0 || n > 8) return false; while (j < len && (isdigit((unsigned char)s[j]) || s[j]==' ')) j++; }
            for (int k = 0; k < n; k++) acc = acc * p;
            any = true;
            i = j;
        }
        if (!any) return false;
        acc.toString(out, outLen);
        return true;
    }

    bool doCompleteSquare(const char* s, char* out, int outLen) {
        Polynomial p;
        if (!parsePoly(s, p)) return false;
        if (p.deg != 2) return false;
        double a = p.c[2], b = p.c[1], c = p.c[0];
        if (fabs(a) < 1e-15) return false;
        double pv = -b / (2*a);
        double qv = c - b*b / (4*a);
        char buf[120];
        if (fabs(a-1) < 1e-9) {
            if (pv >= 0) snprintf(buf, sizeof(buf), "(x-%.6g)^2+%.6g", pv, qv);
            else         snprintf(buf, sizeof(buf), "(x+%.6g)^2+%.6g", -pv, qv);
        } else {
            if (pv >= 0) snprintf(buf, sizeof(buf), "%.6g(x-%.6g)^2+%.6g", a, pv, qv);
            else         snprintf(buf, sizeof(buf), "%.6g(x+%.6g)^2+%.6g", a, -pv, qv);
        }
        strncpy(out, buf, outLen-1); out[outLen-1] = 0;
        return true;
    }

    // Patch 2-1 derivative engine
    static const int DNODE_MAX = 200;
    struct DNode { char kind; char fn; double val; int a, b; };
    DNode dn[DNODE_MAX];
    int dnUsed = 0;
    const char* dpp = nullptr;
    char dVar = 'x';
    bool dErr = false;
    int dAlloc() { if (dnUsed >= DNODE_MAX) { dErr = true; return -1; } return dnUsed++; }
    int dNewNum(double v) { int i = dAlloc(); if (i<0) return -1; dn[i].kind='n'; dn[i].fn=0; dn[i].val=v; dn[i].a=dn[i].b=-1; return i; }
    int dNewVar(char c)   { int i = dAlloc(); if (i<0) return -1; dn[i].kind='v'; dn[i].fn=c; dn[i].val=0; dn[i].a=dn[i].b=-1; return i; }
    int dNewOp(char k, int a, int b) { int i = dAlloc(); if (i<0) return -1; dn[i].kind=k; dn[i].fn=0; dn[i].val=0; dn[i].a=a; dn[i].b=b; return i; }
    int dNewFn(char fn, int a) { int i = dAlloc(); if (i<0) return -1; dn[i].kind='f'; dn[i].fn=fn; dn[i].val=0; dn[i].a=a; dn[i].b=-1; return i; }
    void dSkip() { while (*dpp == ' ' || *dpp == '\t') dpp++; }
    int dParseExpr();
    int dParseAtom() {
        dSkip();
        if (!*dpp) { dErr = true; return -1; }
        if (*dpp == '(') { dpp++; int e = dParseExpr(); dSkip(); if (*dpp == ')') dpp++; else dErr = true; return e; }
        if (isdigit((unsigned char)*dpp) || *dpp == '.') { char* end; double v = strtod(dpp, &end); dpp = end; return dNewNum(v); }
        if (isalpha((unsigned char)*dpp)) {
            char nm[12]; int nn = 0;
            while (isalnum((unsigned char)*dpp) && nn < 11) nm[nn++] = *dpp++;
            nm[nn] = 0; dSkip();
            if (*dpp == '(') {
                dpp++; int arg = dParseExpr(); dSkip(); if (*dpp == ')') dpp++; else dErr = true;
                char fn = 0;
                if      (!strcasecmp(nm, "sin"))  fn = 's';
                else if (!strcasecmp(nm, "cos"))  fn = 'c';
                else if (!strcasecmp(nm, "tan"))  fn = 't';
                else if (!strcasecmp(nm, "exp"))  fn = 'e';
                else if (!strcasecmp(nm, "ln"))   fn = 'l';
                else if (!strcasecmp(nm, "sqrt")) fn = 'q';
                else { dErr = true; return -1; }
                return dNewFn(fn, arg);
            }
            if (nn == 1) { char ch = nm[0]; if ((ch == 'e' || ch == 'E') && ch != dVar) return dNewNum(M_E); return dNewVar(ch); }
            if (!strcasecmp(nm, "pi")) return dNewNum(M_PI);
            dErr = true; return -1;
        }
        dErr = true; return -1;
    }

    int dParseUnary() {
        dSkip();
        if (*dpp == '-') { dpp++; int e = dParseUnary(); return dNewOp('N', e, -1); }
        if (*dpp == '+') { dpp++; return dParseUnary(); }
        return dParseAtom();
    }

    int dParsePow() {
        int base = dParseUnary(); dSkip();
        if (*dpp == '^') { dpp++; int ex = dParsePow(); return dNewOp('^', base, ex); }
        return base;
    }

    int dParseTerm() {
        int left = dParsePow();
        while (true) { dSkip();
            if      (*dpp == '*') { dpp++; int r = dParsePow(); left = dNewOp('*', left, r); }
            else if (*dpp == '/') { dpp++; int r = dParsePow(); left = dNewOp('/', left, r); }
            else break;
        }
        return left;
    }

    int dDiff(int n) {
        if (n < 0 || dErr) return -1;
        static int dDepth = 0;
        if (dDepth > 120) { dErr = true; return -1; }
        struct DG { int* p; DG(int* q):p(q){++(*p);} ~DG(){--(*p);} };
        DG _g(&dDepth);
        DNode& N = dn[n];
        switch (N.kind) {
            case 'n': return dNewNum(0);
            case 'v': return dNewNum(N.fn == dVar ? 1 : 0);
            case 'N': return dNewOp('N', dDiff(N.a), -1);
            case '+': case '-': return dNewOp(N.kind, dDiff(N.a), dDiff(N.b));
            case '*': {
                int fp = dDiff(N.a), gp = dDiff(N.b);
                int t1 = dNewOp('*', fp, N.b);
                int t2 = dNewOp('*', N.a, gp);
                return dNewOp('+', t1, t2);
            }
            case '/': {
                int fp = dDiff(N.a), gp = dDiff(N.b);
                int t1 = dNewOp('*', fp, N.b);
                int t2 = dNewOp('*', N.a, gp);
                int nm = dNewOp('-', t1, t2);
                int dn2 = dNewOp('^', N.b, dNewNum(2));
                return dNewOp('/', nm, dn2);
            }
            case '^': {
                if (dn[N.b].kind == 'n') {
                    double nv = dn[N.b].val;
                    int fp = dDiff(N.a);
                    int t1 = dNewOp('*', dNewNum(nv), dNewOp('^', N.a, dNewNum(nv - 1)));
                    return dNewOp('*', t1, fp);
                }
                dErr = true; return -1;
            }
            case 'f': {
                int arg = N.a, argp = dDiff(arg);
                switch (N.fn) {
                    case 's': return dNewOp('*', dNewFn('c', arg), argp);
                    case 'c': return dNewOp('*', dNewOp('N', dNewFn('s', arg), -1), argp);
                    case 't': { int c = dNewFn('c', arg); return dNewOp('/', argp, dNewOp('^', c, dNewNum(2))); }
                    case 'e': return dNewOp('*', dNewFn('e', arg), argp);
                    case 'l': return dNewOp('/', argp, arg);
                    case 'q': return dNewOp('/', argp, dNewOp('*', dNewNum(2), dNewFn('q', arg)));
                }
                dErr = true; return -1;
            }
        }
        dErr = true; return -1;
    }

    int dSimplify(int n) {
        if (n < 0) return n;
        static int sDepth = 0;
        if (sDepth > 200) return -1;
        sDepth++;
        DNode& N = dn[n];
        switch (N.kind) {
            case 'n': case 'v': { sDepth--; return n; }
            case 'N': { int a = dSimplify(N.a); if (a < 0) { sDepth--; return -1; } if (dn[a].kind == 'n') { sDepth--; return dNewNum(-dn[a].val); } { int rv = dNewOp('N', a, -1); sDepth--; return rv; } }
            case '+': {
                int a = dSimplify(N.a), b = dSimplify(N.b);
                if (a < 0 || b < 0) { sDepth--; return -1; }
                if (dn[a].kind=='n' && dn[a].val==0) { sDepth--; return b; }
                if (dn[b].kind=='n' && dn[b].val==0) { sDepth--; return a; }
                if (dn[a].kind=='n' && dn[b].kind=='n') { sDepth--; return dNewNum(dn[a].val + dn[b].val); }
                { int rv = dNewOp('+', a, b); sDepth--; return rv; }
            }
            case '-': {
                int a = dSimplify(N.a), b = dSimplify(N.b);
                if (a < 0 || b < 0) { sDepth--; return -1; }
                if (dn[b].kind=='N') { int rv = dNewOp('+', a, dn[b].a); sDepth--; return rv; }
                if (dn[b].kind=='n' && dn[b].val==0) { sDepth--; return a; }
                if (dn[a].kind=='n' && dn[a].val==0) { int rv = dNewOp('N', b, -1); sDepth--; return rv; }
                if (dn[a].kind=='n' && dn[b].kind=='n') { sDepth--; return dNewNum(dn[a].val - dn[b].val); }
                { int rv = dNewOp('-', a, b); sDepth--; return rv; }
            }
            case '*': {
                int a = dSimplify(N.a), b = dSimplify(N.b);
                if (a < 0 || b < 0) { sDepth--; return -1; }
                if ((dn[a].kind=='n' && dn[a].val==0) || (dn[b].kind=='n' && dn[b].val==0)) { sDepth--; return dNewNum(0); }
                if (dn[a].kind=='n' && dn[a].val==1) { sDepth--; return b; }
                if (dn[b].kind=='n' && dn[b].val==1) { sDepth--; return a; }
                if (dn[a].kind=='n' && dn[b].kind=='n') { sDepth--; return dNewNum(dn[a].val * dn[b].val); }
                { int rv = dNewOp('*', a, b); sDepth--; return rv; }
            }
            case '/': {
                int a = dSimplify(N.a), b = dSimplify(N.b);
                if (a < 0 || b < 0) { sDepth--; return -1; }
                if (dn[a].kind=='n' && dn[a].val==0) { sDepth--; return dNewNum(0); }
                if (dn[b].kind=='n' && dn[b].val==1) { sDepth--; return a; }
                if (dn[a].kind=='n' && dn[b].kind=='n' && dn[b].val!=0) { sDepth--; return dNewNum(dn[a].val / dn[b].val); }
                { int rv = dNewOp('/', a, b); sDepth--; return rv; }
            }
            case '^': {
                int a = dSimplify(N.a), b = dSimplify(N.b);
                if (a < 0 || b < 0) { sDepth--; return -1; }
                if (dn[b].kind=='n' && dn[b].val==0) { sDepth--; return dNewNum(1); }
                if (dn[b].kind=='n' && dn[b].val==1) { sDepth--; return a; }
                if (dn[a].kind=='n' && dn[b].kind=='n') { sDepth--; return dNewNum(pow(dn[a].val, dn[b].val)); }
                { int rv = dNewOp('^', a, b); sDepth--; return rv; }
            }
            case 'f': { int a = dSimplify(N.a); if (a < 0) { sDepth--; return -1; } int rv = dNewFn(N.fn, a); sDepth--; return rv; }
        }
        sDepth--;
        return n;
    }

    void dPrint(int n, char* out, int outLen, int& pos, int parentPrec) {
        if (n < 0 || pos >= outLen - 1) return;
        DNode& N = dn[n];
        int prec;
        switch (N.kind) {
            case '+': case '-': prec = 1; break;
            case '*': case '/': prec = 2; break;
            case 'N':           prec = 3; break;
            case '^':           prec = 4; break;
            case 'f':           prec = 5; break;
            default:            prec = 6; break;
        }
        bool needParen = (prec < parentPrec);
        if (needParen && pos < outLen - 1) out[pos++] = '(';
        switch (N.kind) {
            case 'n': {
                double v = N.val; char b[24];
                if (v == (long long)v && fabs(v) < 1e15) snprintf(b, sizeof(b), "%lld", (long long)v);
                else snprintf(b, sizeof(b), "%g", v);
                for (int i = 0; b[i] && pos < outLen - 1; i++) out[pos++] = b[i];
            } break;
            case 'v': if (pos < outLen - 1) out[pos++] = N.fn; break;
            case 'N': if (pos < outLen - 1) out[pos++] = '-'; dPrint(N.a, out, outLen, pos, prec); break;
            case '+': case '-': case '*': case '/': case '^': {
                int lp = (N.kind == '^') ? (prec + 1) : prec;
                dPrint(N.a, out, outLen, pos, lp);
                if (pos < outLen - 1) out[pos++] = N.kind;
                int rp = (N.kind == '^') ? prec : (prec + 1);
                dPrint(N.b, out, outLen, pos, rp);
            } break;
            case 'f': {
                const char* fname = "?(";
                switch (N.fn) {
                    case 's': fname = "sin(";  break;
                    case 'c': fname = "cos(";  break;
                    case 't': fname = "tan(";  break;
                    case 'e': fname = "exp(";  break;
                    case 'l': fname = "ln(";   break;
                    case 'q': fname = "sqrt("; break;
                }
                for (int i = 0; fname[i] && pos < outLen - 1; i++) out[pos++] = fname[i];
                dPrint(N.a, out, outLen, pos, 0);
                if (pos < outLen - 1) out[pos++] = ')';
            } break;
        }
        if (needParen && pos < outLen - 1) out[pos++] = ')';
    }

    bool doDerivativeImpl(const char* expr, char var, char* out, int outLen) {
        Polynomial p;
        if (parsePoly(expr, p)) { Polynomial d = p.diff(); d.toString(out, outLen); return true; }
        dnUsed = 0; dErr = false; dVar = var; dpp = expr;
        int root = dParseExpr();
        dSkip();
        if (dErr || *dpp != 0 || root < 0) return false;
        int d = dDiff(root);
        if (dErr || d < 0) return false;
        d = dSimplify(d);
        if (dErr || d < 0) return false;
        int pos = 0;
        dPrint(d, out, outLen, pos, 0);
        out[pos] = 0;
        return true;
    }

    bool doDerivative(const char* s, char* out, int outLen) {
        int len = strlen(s), depth = 0, cp = -1;
        for (int i = 0; i < len; i++) {
            char c = s[i];
            if      (c == '(') depth++;
            else if (c == ')') depth--;
            else if (c == ',' && depth == 0) cp = i;
        }
        if (cp < 0) return false;
        int es = 0; while (es < cp && s[es] == ' ') es++;
        int ee = cp; while (ee > es && s[ee-1] == ' ') ee--;
        int elen = ee - es;
        if (elen <= 0 || elen >= 80) return false;
        char expr[80]; memcpy(expr, s + es, elen); expr[elen] = 0;
        int vs = cp + 1; while (vs < len && s[vs] == ' ') vs++;
        int ve = len; while (ve > vs && s[ve-1] == ' ') ve--;
        if (ve - vs != 1) return false;
        return doDerivativeImpl(expr, s[vs], out, outLen);
    }

    bool doCFactor(const char* s, char* out, int outLen) {
        Polynomial p; if (!parsePoly(s, p)) return false;
        if (p.deg < 2) { p.toString(out, outLen); return true; }
        Complex roots[16];
        int n = NumAnalysis::polyRoots(p.c, p.deg, roots);
        char buf[200]; int bp = 0;
        double leading = p.c[p.deg];
        if (fabs(leading - 1) > 1e-9 && bp < (int)sizeof(buf)-20) bp += snprintf(buf+bp, sizeof(buf)-bp, "%.4g", leading);
        for (int i = 0; i < n && bp < (int)sizeof(buf)-40; i++) {
            char t2[48];
            if (fabs(roots[i].im) < 1e-6) {
                double r = roots[i].re;
                snprintf(t2, sizeof(t2), r>=0 ? "(x-%.4g)" : "(x+%.4g)", fabs(r));
            } else {
                snprintf(t2, sizeof(t2), "(x-(%.4g%+.4gi))", roots[i].re, roots[i].im);
            }
            bp += snprintf(buf+bp, sizeof(buf)-bp, "%s", t2);
        }
        strncpy(out, buf, outLen-1); out[outLen-1]=0;
        return true;
    }

    bool doFactorRational(const char* s, char* out, int outLen) {
        Polynomial p; if (!parsePoly(s, p)) return false;
        if (p.deg < 2) { p.toString(out, outLen); return true; }
        long long lcm = 1;
        for (int i = 0; i <= p.deg; i++) { Fraction f = Fraction::fromDouble(p.c[i], 1e-9, 100000LL); lcm = dfL(lcm, f.den); if (lcm > 10000000LL) { lcm = 1; break; } }
        double cc[POLY_MAX+1]; int rd = p.deg;
        for (int i = 0; i <= rd; i++) { Fraction f = Fraction::fromDouble(p.c[i], 1e-9, 100000LL); cc[i] = (double)(f.num * (lcm / f.den)); }
        char fac[12][40]; int nf = 0;
        while (rd >= 1 && nf < 10) {
            if (fabs(cc[0]) < 1e-9) { snprintf(fac[nf++],40,"x"); for (int k=1;k<=rd;k++) cc[k-1]=cc[k]; rd--; continue; }
            long long pd[64], qd[64];
            int np = dfDiv((long long)round(fabs(cc[0])), pd, 64);
            int nq = dfDiv((long long)round(fabs(cc[rd])), qd, 64);
            if (!np || !nq) break;
            bool found = false;
            for (int i = 0; i < np && !found; i++)
                for (int j = 0; j < nq && !found; j++)
                    for (int sg = -1; sg <= 1 && !found; sg += 2) {
                        double r = sg * (double)pd[i] / qd[j];
                        double rm[POLY_MAX+1]; rm[0] = cc[rd];
                        for (int k = 1; k <= rd; k++) rm[k] = cc[rd-k] + rm[k-1]*r;
                        if (fabs(rm[rd]) < 1e-6) {
                            snprintf(fac[nf++], 40, r>=0 ? "(x-%.4g)" : "(x+%.4g)", fabs(r));
                            for (int k = 0; k < rd; k++) cc[k] = rm[rd-1-k];
                            rd--; found = true;
                        }
                    }
            if (!found) break;
        }
        char buf[200]; int bp = 0;
        double leading = p.c[p.deg];
        if (fabs(leading - 1) > 1e-9 && bp < (int)sizeof(buf)-20) bp += snprintf(buf+bp, sizeof(buf)-bp, "%.4g", leading);
        for (int i = 0; i < nf && bp < (int)sizeof(buf)-40; i++) bp += snprintf(buf+bp, sizeof(buf)-bp, "%s", fac[i]);
        if (rd > 0 && bp < (int)sizeof(buf)-90) { Polynomial rp; for (int i=0;i<=rd;i++) rp.c[i]=cc[i]; rp.normalize(); char rt[80]; rp.toString(rt,sizeof(rt)); bp += snprintf(buf+bp, sizeof(buf)-bp, "(%s)", rt); }
        if (bp == 0) snprintf(buf, sizeof(buf), "1");
        strncpy(out, buf, outLen-1); out[outLen-1]=0;
        return true;
    }

    static long long dfG(long long a, long long b) { if(a<0)a=-a; if(b<0)b=-b; while(b){long long t=a%b;a=b;b=t;} return a?a:1; }
    static long long dfL(long long a, long long b) { return a/dfG(a,b)*b; }
    static int dfDiv(long long n, long long* out, int max) {
        if (n < 0) n = -n;
        if (n == 0) return 0;
        int c = 0;
        for (long long d = 1; d*d <= n && c < max; d++)
            if (n % d == 0) { out[c++]=d; if (d*d != n && c < max) out[c++]=n/d; }
        return c;
    }

    bool doTExpand(const char* s, char* out, int outLen) {
        int len = strlen(s), i = 0;
        while (i < len && s[i]==' ') i++;
        char fk = 0;
        if (i+3 <= len && !strncasecmp(s+i,"sin",3)) { fk='s'; i+=3; }
        else if (i+3 <= len && !strncasecmp(s+i,"cos",3)) { fk='c'; i+=3; }
        else return false;
        while (i < len && s[i]==' ') i++;
        if (i >= len || s[i] != '(') return false;
        int start = i+1, depth = 1, close = -1;
        for (int k = start; k < len; k++) {
            if (s[k]=='(') depth++;
            else if (s[k]==')') { depth--; if (depth==0) { close = k; break; } }
        }
        if (close < 0) return false;
        int op = -1, d2 = 0;
        for (int k = start; k < close; k++) {
            if (s[k]=='(') d2++;
            else if (s[k]==')') d2--;
            else if ((s[k]=='+'||s[k]=='-') && d2==0 && k>start) { op = k; break; }
        }
        if (op < 0) return false;
        char A[40], B[40]; int al=op-start, bl=close-op-1;
        if (al<=0 || al>=40 || bl<=0 || bl>=40) return false;
        memcpy(A, s+start, al); A[al]=0;
        memcpy(B, s+op+1, bl); B[bl]=0;
        char *Ap=A, *Bp=B;
        while(*Ap==' ')Ap++;
        while(*Bp==' ')Bp++;
        char sg = s[op];
        if (fk=='s') {
            if (sg=='+') snprintf(out,outLen,"sin(%s)*cos(%s)+cos(%s)*sin(%s)",Ap,Bp,Ap,Bp);
            else         snprintf(out,outLen,"sin(%s)*cos(%s)-cos(%s)*sin(%s)",Ap,Bp,Ap,Bp);
        } else {
            if (sg=='+') snprintf(out,outLen,"cos(%s)*cos(%s)-sin(%s)*sin(%s)",Ap,Bp,Ap,Bp);
            else         snprintf(out,outLen,"cos(%s)*cos(%s)+sin(%s)*sin(%s)",Ap,Bp,Ap,Bp);
        }
        return true;
    }

    bool doTCollect(const char* s, char* out, int outLen) {
        int len = strlen(s), a = 0, b = len;
        while (a < len && s[a]==' ') a++;
        while (b > a && s[b-1]==' ') b--;
        auto ext = [&](int pos, char* fnO, char* argO, int& aL, int& eP) -> bool {
            int i = 0;
            if (pos+i+3 > b) return false;
            if (!strncasecmp(s+pos+i,"sin",3)) strcpy(fnO,"sin");
            else if (!strncasecmp(s+pos+i,"cos",3)) strcpy(fnO,"cos");
            else return false;
            i += 3;
            while (pos+i < b && s[pos+i]==' ') i++;
            if (s[pos+i] != '(') return false;
            int st = pos+i+1, dp = 1, cl = -1;
            for (int k = st; k < b; k++) {
                if (s[k]=='(') dp++;
                else if (s[k]==')') { dp--; if (dp==0) { cl=k; break; } }
            }
            if (cl < 0) return false;
            aL = cl - st;
            if (aL<=0 || aL>=40) return false;
            memcpy(argO, s+st, aL); argO[aL]=0;
            eP = cl+1;
            return true;
        };
        char fn[4]={0}, arg[40]; int aL=0, eP=0;
        if (!ext(a, fn, arg, aL, eP)) return false;
        int p2 = a + eP;
        while (p2 < b && s[p2]==' ') p2++;
        if (p2+1 < b && s[p2]=='^' && s[p2+1]=='2') {
            if (!strcmp(fn,"sin")) snprintf(out,outLen,"(1-cos(2*%s))/2",arg);
            else                   snprintf(out,outLen,"(1+cos(2*%s))/2",arg);
            return true;
        }
        if (p2 < b && s[p2]=='*') {
            p2++;
            while (p2 < b && s[p2]==' ') p2++;
            char fn2[4]={0}, arg2[40]; int aL2=0, eP2=0;
            if (!ext(p2, fn2, arg2, aL2, eP2)) return false;
            if (strcmp(arg,arg2)!=0) return false;
            if ((!strcmp(fn,"sin")&&!strcmp(fn2,"cos")) || (!strcmp(fn,"cos")&&!strcmp(fn2,"sin"))) {
                snprintf(out,outLen,"sin(2*%s)/2",arg);
                return true;
            }
        }
        return false;
    }

    static bool dpSplit(const char* s, char* num, int numLen, char* den, int denLen) {
        int len = strlen(s), i = 0, depth = 0;
        while (i < len && s[i]==' ') i++;
        int slash = -1;
        for (int k = i; k < len; k++) {
            char c = s[k];
            if (c=='(') depth++;
            else if (c==')') depth--;
            else if (c=='/' && depth==0) { slash = k; break; }
        }
        if (slash < 0) {
            int n = len - i;
            if (n <= 0 || n >= numLen) return false;
            memcpy(num, s+i, n); num[n]=0;
            if (denLen < 2) return false;
            strcpy(den, "1");
            return true;
        }
        int a = slash - i;
        if (a <= 0 || a >= numLen) return false;
        memcpy(num, s+i, a); num[a]=0;
        int j = slash + 1;
        while (j < len && s[j]==' ') j++;
        int b2 = len - j;
        if (b2 <= 0 || b2 >= denLen) return false;
        memcpy(den, s+j, b2); den[b2]=0;
        return true;
    }

    static bool dpDivPoly(const Polynomial& P, const Polynomial& Q, Polynomial& quot, Polynomial& rem) {
        quot = Polynomial(); rem = P;
        if (Q.deg == 0) {
            if (fabs(Q.c[0]) < 1e-15) return false;
            for (int i = 0; i <= P.deg; i++) quot.c[i] = P.c[i] / Q.c[0];
            quot.normalize();
            rem = Polynomial();
            return true;
        }
        int guard = 0;
        while (rem.deg >= Q.deg && rem.deg >= 1 && guard < POLY_MAX * 2) {
            int shift = rem.deg - Q.deg;
            double coef = rem.c[rem.deg] / Q.c[Q.deg];
            quot.c[shift] += coef;
            for (int k = 0; k <= Q.deg; k++) rem.c[k + shift] -= coef * Q.c[k];
            rem.normalize();
            guard++;
        }
        quot.normalize();
        return true;
    }

    bool doGetNum(const char* s, char* out, int outLen) {
        char nb[80], db[40];
        if (!dpSplit(s, nb, 80, db, 40)) return false;
        strncpy(out, nb, outLen-1); out[outLen-1]=0;
        return true;
    }

    bool doGetDenom(const char* s, char* out, int outLen) {
        char nb[80], db[40];
        if (!dpSplit(s, nb, 80, db, 40)) return false;
        Polynomial p;
        if (parsePoly(db, p)) { p.toString(out, outLen); return true; }
        strncpy(out, db, outLen-1); out[outLen-1]=0;
        return true;
    }

    bool doPropFrac(const char* s, char* out, int outLen) {
        char nb[80], db[40];
        if (!dpSplit(s, nb, 80, db, 40)) return false;
        Polynomial P, Q;
        if (!parsePoly(nb, P) || !parsePoly(db, Q)) return false;
        if (Q.deg == 0) { P.toString(out, outLen); return true; }
        Polynomial quot, rem;
        if (!dpDivPoly(P, Q, quot, rem)) return false;
        char qs[80]="0", rs[80]="0", ds[80]="0";
        quot.toString(qs, sizeof(qs));
        rem.toString(rs, sizeof(rs));
        Q.toString(ds, sizeof(ds));
        if (rem.deg == 0 && fabs(rem.c[0]) < 1e-12) {
            strncpy(out, qs, outLen-1); out[outLen-1]=0;
        } else if (quot.deg == 0 && fabs(quot.c[0]) < 1e-12) {
            snprintf(out, outLen, "(%s)/%s", rs, ds);
        } else {
            snprintf(out, outLen, "%s + (%s)/%s", qs, rs, ds);
        }
        return true;
    }

    bool doComDenom(const char* s, char* out, int outLen) {
        Polynomial nums[4], dens[4]; int signs[4]; int nt = 0;
        int len = strlen(s), i = 0, sgn = 1;
        while (i < len && s[i]==' ') i++;
        if (i < len && (s[i]=='+' || s[i]=='-')) { if (s[i]=='-') sgn=-1; i++; }
        while (i < len && nt < 4) {
            int start = i, d2 = 0;
            while (i < len) {
                char c = s[i];
                if (c=='(') d2++;
                else if (c==')') d2--;
                else if ((c=='+'||c=='-') && d2==0) break;
                i++;
            }
            int al = i - start;
            while (al > 0 && s[start+al-1]==' ') al--;
            if (al <= 0 || al >= 80) return false;
            char tb[80]; memcpy(tb, s+start, al); tb[al]=0;
            char nb[60], db[40];
            if (!dpSplit(tb, nb, 60, db, 40)) return false;
            if (!parsePoly(nb, nums[nt]) || !parsePoly(db, dens[nt])) return false;
            signs[nt] = sgn; nt++;
            if (i >= len) break;
            sgn = (s[i]=='+') ? 1 : -1;
            i++;
            while (i < len && s[i]==' ') i++;
        }
        if (nt == 0) return false;
        if (nt >= 4 && i < len) return false;
        Polynomial cden; cden.c[0] = 1;
        for (int m = 0; m < nt; m++) cden = cden * dens[m];
        Polynomial cnum;
        for (int m = 0; m < nt; m++) {
            Polynomial tm = nums[m];
            for (int k = 0; k < nt; k++) if (k != m) tm = tm * dens[k];
            if (signs[m] < 0) for (int k = 0; k <= POLY_MAX; k++) tm.c[k] = -tm.c[k];
            cnum = cnum + tm;
        }
        cnum.normalize();
        char ns[80], ds[80];
        cnum.toString(ns, sizeof(ns));
        cden.toString(ds, sizeof(ds));
        if (cden.deg == 0 && fabs(cden.c[0] - 1.0) < 1e-12) {
            strncpy(out, ns, outLen-1); out[outLen-1]=0;
        } else {
            snprintf(out, outLen, "(%s)/(%s)", ns, ds);
        }
        return true;
    }

    bool doLeft(const char* s, char* out, int outLen) {int L=strlen(s),d=0,eq=-1;for(int i=0;i<L;i++){if(s[i]=='(')d++;else if(s[i]==')')d--;else if(s[i]=='='&&d==0){if(i+1<L&&s[i+1]=='='){i++;continue;}eq=i;break;}}if(eq<0)return false;int en=eq;while(en>0&&s[en-1]==' ')en--;if(en<=0||en>=outLen)return false;memcpy(out,s,en);out[en]=0;return true;}
    bool doRight(const char* s, char* out, int outLen) {int L=strlen(s),d=0,eq=-1;for(int i=0;i<L;i++){if(s[i]=='(')d++;else if(s[i]==')')d--;else if(s[i]=='='&&d==0){if(i+1<L&&s[i+1]=='='){i++;continue;}eq=i;break;}}if(eq<0)return false;int st=eq+1;while(st<L&&s[st]==' ')st++;int el=L-st;if(el<=0||el>=outLen)return false;memcpy(out,s+st,el);out[el]=0;return true;}
    bool doCSolve(const char* s, char* out, int outLen) {int L=strlen(s),d=0,eq=-1;for(int i=0;i<L;i++){if(s[i]=='(')d++;else if(s[i]==')')d--;else if(s[i]=='='&&d==0){if(i+1<L&&s[i+1]=='='){i++;continue;}eq=i;break;}}char ex[80];if(eq<0){if(L>=80)return false;strncpy(ex,s,79);ex[79]=0;}else{char lh[40],rh[40];int ll=eq;while(ll>0&&s[ll-1]==' ')ll--;if(ll<=0||ll>=40)return false;memcpy(lh,s,ll);lh[ll]=0;int rs=eq+1;while(rs<L&&s[rs]==' ')rs++;int el=L-rs;if(el<=0||el>=40)return false;memcpy(rh,s+rs,el);rh[el]=0;snprintf(ex,sizeof(ex),"(%s)-(%s)",lh,rh);}Polynomial p;if(!parsePoly(ex,p))return false;if(p.deg<1)return false;Complex roots[16];int n=NumAnalysis::polyRoots(p.c,p.deg,roots);char buf[200];int bp=0;for(int i=0;i<n;i++){char rs[32];roots[i].toString(rs,sizeof(rs));bp+=snprintf(buf+bp,sizeof(buf)-bp,"%sx=%s",i?" ; ":"",rs);}strncpy(out,buf,outLen-1);out[outLen-1]=0;return true;}
    bool doToLn(const char* s, char* out, int outLen) {int L=strlen(s),d=0,ct=-1;for(int i=0;i<L;i++){if(s[i]=='(')d++;else if(s[i]==')')d--;else if(s[i]=='^'&&d==0){ct=i;break;}}if(ct<=0)return false;char base[20],exp[60];int bl=ct;while(bl>0&&s[bl-1]==' ')bl--;if(bl<=0||bl>=20)return false;memcpy(base,s,bl);base[bl]=0;int es=ct+1;while(es<L&&s[es]==' ')es++;int el=L-es;while(el>0&&s[es+el-1]==' ')el--;if(el<=0||el>=60)return false;memcpy(exp,s+es,el);exp[el]=0;char*e;double bv=strtod(base,&e);if(e==base||*e)return false;snprintf(out,outLen,"e^(%s*ln(%g))",exp,bv);return true;}
    bool doToExp(const char* s, char* out, int outLen) {int L=strlen(s),i=0;while(i<L&&s[i]==' ')i++;if(i+3<L&&!strncasecmp(s+i,"exp",3)){int p=i+3;while(p<L&&s[p]==' ')p++;if(p<L&&s[p]=='('){int d=1,cl=-1;for(int k=p+1;k<L;k++){if(s[k]=='(')d++;else if(s[k]==')'){d--;if(d==0){cl=k;break;}}}if(cl>0){int il=cl-p-1;if(il>0&&il<60){char in[60];memcpy(in,s+p+1,il);in[il]=0;snprintf(out,outLen,"e^(%s)",in);return true;}}}}return false;}
    bool doToLogbase(const char* s, char* out, int outLen) {int L=strlen(s),d=0,cp=-1;for(int i=0;i<L;i++){if(s[i]=='(')d++;else if(s[i]==')')d--;else if(s[i]==','&&d==0){cp=i;break;}}if(cp<0)return false;char bb[20],eb[60];int bl=cp;while(bl>0&&s[bl-1]==' ')bl--;if(bl<=0||bl>=20)return false;memcpy(bb,s,bl);bb[bl]=0;int es=cp+1;while(es<L&&s[es]==' ')es++;int el=L-es;while(el>0&&s[es+el-1]==' ')el--;if(el<=0||el>=60)return false;memcpy(eb,s+es,el);eb[el]=0;snprintf(out,outLen,"ln(%s)/ln(%s)",eb,bb);return true;}
    bool doToSin(const char* s, char* out, int outLen) {int L=strlen(s),i=0;while(i<L&&s[i]==' ')i++;if(i+3>=L||strncasecmp(s+i,"cos",3))return false;int p=i+3;while(p<L&&s[p]==' ')p++;if(p>=L||s[p]!='(')return false;int d=1,cl=-1;for(int k=p+1;k<L;k++){if(s[k]=='(')d++;else if(s[k]==')'){d--;if(d==0){cl=k;break;}}}if(cl<0)return false;int il=cl-p-1;if(il<=0||il>=60)return false;char in[60];memcpy(in,s+p+1,il);in[il]=0;snprintf(out,outLen,"sin(pi/2-(%s))",in);return true;}
    bool doToCos(const char* s, char* out, int outLen) {int L=strlen(s),i=0;while(i<L&&s[i]==' ')i++;if(i+3>=L||strncasecmp(s+i,"sin",3))return false;int p=i+3;while(p<L&&s[p]==' ')p++;if(p>=L||s[p]!='(')return false;int d=1,cl=-1;for(int k=p+1;k<L;k++){if(s[k]=='(')d++;else if(s[k]==')'){d--;if(d==0){cl=k;break;}}}if(cl<0)return false;int il=cl-p-1;if(il<=0||il>=60)return false;char in[60];memcpy(in,s+p+1,il);in[il]=0;snprintf(out,outLen,"cos(pi/2-(%s))",in);return true;}
    bool doDeSolve(const char* s, char* out, int outLen) {
        bool hasX, hasY;
        dfVarScan(s, hasX, hasY);
        if (!hasY) {
            Polynomial p;
            if (parsePoly(s, p)) {
                Polynomial I = p.integ();
                char tmp[80]; I.toString(tmp, sizeof(tmp));
                snprintf(out, outLen, "y = %s + C", tmp);
                return true;
            }
            return false;
        }
        if (!hasX) {
            char fb[80]; dfReplaceXtoY(s, fb, sizeof(fb));
            ExprEval::Subst s0 = {'y', 0}, s1 = {'y', 1}, s2 = {'y', 2};
            auto r0 = gEv.eval(fb, &s0, 1);
            auto r1 = gEv.eval(fb, &s1, 1);
            auto r2 = gEv.eval(fb, &s2, 1);
            if (!r0.ok || !r1.ok || !r2.ok) return false;
            double a = r1.val - r0.val;
            double b = r0.val;
            if (fabs(r2.val - (2*a + b)) > 1e-6) return false;
            if (fabs(a) < 1e-12 && fabs(b) < 1e-12) { snprintf(out, outLen, "y = C"); return true; }
            if (fabs(a) < 1e-12) { snprintf(out, outLen, "y = %.4g*x + C", b); return true; }
            if (fabs(b) < 1e-9) { snprintf(out, outLen, "y = C*e^(%.4g*x)", a); return true; }
            snprintf(out, outLen, "y = %.4g + C*e^(%.4g*x)", -b/a, a);
            return true;
        }
        if (hasX && hasY) {
            double xS[3] = {0.3, 1.3, 2.3};
            double Pv[3], Qv[3];
            bool lin = true;
            for (int i = 0; i < 3; i++) {
                ExprEval::Subst sb[2];
                sb[0].var = 'x'; sb[0].val = xS[i];
                sb[1].var = 'y'; sb[1].val = 0;
                auto r0 = gEv.eval(s, sb, 2);
                sb[1].val = 1; auto r1 = gEv.eval(s, sb, 2);
                sb[1].val = 2; auto r2 = gEv.eval(s, sb, 2);
                if (!r0.ok || !r1.ok || !r2.ok) { lin = false; break; }
                double d1 = r1.val - r0.val, d2 = r2.val - r1.val;
                if (fabs(d2 - d1) > 1e-6 * (1 + fabs(d1))) { lin = false; break; }
                Pv[i] = d1; Qv[i] = r0.val;
            }
            if (lin) {
                bool pC = fabs(Pv[0]-Pv[1]) < 1e-6 && fabs(Pv[1]-Pv[2]) < 1e-6;
                bool qC = fabs(Qv[0]-Qv[1]) < 1e-6 && fabs(Qv[1]-Qv[2]) < 1e-6;
                if (pC && qC) {
                    double p = Pv[0], q = Qv[0];
                    if (fabs(p) < 1e-12 && fabs(q) < 1e-12) { snprintf(out, outLen, "y = C"); return true; }
                    if (fabs(p) < 1e-12) { snprintf(out, outLen, "y = %.4g*x + C", q); return true; }
                    snprintf(out, outLen, "y = %.4g + C*e^(%.4g*x)", -q/p, p);
                    return true;
                }
                if (pC) {
                    snprintf(out, outLen, "y = C*e^(%.4g*x) + e^(%.4g*x)*int Q(x)*e^(-%.4g*x) dx", Pv[0], Pv[0], Pv[0]);
                    return true;
                }
                snprintf(out, outLen, "y = e^(int P dx)*(C + int Q*e^(-int P dx) dx)");
                return true;
            }
        }
        return false;
    }

    static void dfVarScan(const char* s, bool& hasX, bool& hasY) {
        hasX = hasY = false;
        for (const char* p = s; *p; p++) {
            bool prevOk = (p == s) || !isalnum((unsigned char)p[-1]);
            bool nextOk = !isalnum((unsigned char)p[1]);
            if (prevOk && nextOk) {
                if (*p == 'x' || *p == 'X') hasX = true;
                if (*p == 'y' || *p == 'Y') hasY = true;
            }
        }
    }

    static void dfReplaceXtoY(const char* s, char* out, int outLen) {
        int bi = 0;
        for (const char* p = s; *p && bi < outLen - 1; p++) {
            bool prevOk = (p == s) || !isalnum((unsigned char)p[-1]);
            bool nextOk = !isalnum((unsigned char)p[1]);
            if (prevOk && nextOk && (*p == 'x' || *p == 'X')) out[bi++] = 'y';
            else out[bi++] = *p;
        }
        out[bi] = 0;
    }

    bool doRK4(const char* s, char* out, int outLen) {
        char parts[5][40]; int pi = 0, pj = 0, d = 0;
        for (int i = 0; s[i] && pi < 5; i++) {
            char c = s[i];
            if (c == '(') d++;
            else if (c == ')') d--;
            if (c == ',' && d == 0) { parts[pi][pj]=0; pi++; pj=0; continue; }
            if (pj < 39) parts[pi][pj++] = c;
        }
        if (pi < 5) { parts[pi][pj]=0; pi++; }
        if (pi < 5) return false;
        double x0 = atof(parts[1]), y0 = atof(parts[2]);
        double xe = atof(parts[3]), h = atof(parts[4]);
        if (h <= 0) return false;
        double rawSteps = ceil((xe - x0) / h);
        if (rawSteps < 1 || rawSteps > 5000) return false;
        int steps = (int)rawSteps;
        double x = x0, y = y0;
        ExprEval::Subst sb[2] = {{'x',0},{'y',0}};
        for (int i = 0; i < steps; i++) {
            sb[0].val = x; sb[1].val = y;
            auto k1r = gEv.eval(parts[0], sb, 2); if (!k1r.ok) return false; double k1 = k1r.val;
            sb[0].val = x + h/2; sb[1].val = y + h*k1/2;
            auto k2r = gEv.eval(parts[0], sb, 2); if (!k2r.ok) return false; double k2 = k2r.val;
            sb[1].val = y + h*k2/2;
            auto k3r = gEv.eval(parts[0], sb, 2); if (!k3r.ok) return false; double k3 = k3r.val;
            sb[0].val = x + h; sb[1].val = y + h*k3;
            auto k4r = gEv.eval(parts[0], sb, 2); if (!k4r.ok) return false; double k4 = k4r.val;
            y += h * (k1 + 2*k2 + 2*k3 + k4) / 6.0;
            x += h;
            if (isnan(y) || isinf(y)) { snprintf(out, outLen, "diverged at x=%.4g", x); return true; }
        }
        snprintf(out, outLen, "y(%.4g) = %.6g  (steps=%d, h=%.4g)", x, y, steps, h);
        return true;
    }

    bool doDeSolve2(const char* s, char* out, int outLen) {
        double ab[2]; int an = 0;
        const char* p = s; char tmp[24]; int tn = 0;
        while (*p && an < 2) {
            if (*p == ',') { tmp[tn]=0; ab[an++] = atof(tmp); tn=0; p++; continue; }
            if (tn < 23) tmp[tn++] = *p;
            p++;
        }
        if (tn > 0 && an < 2) { tmp[tn]=0; ab[an++] = atof(tmp); }
        if (an < 2) return false;
        double A = ab[0], B = ab[1];
        double D = A*A - 4*B;
        if (D > 1e-9) {
            double r1 = (-A + sqrt(D)) / 2, r2 = (-A - sqrt(D)) / 2;
            snprintf(out, outLen, "y = C1*e^(%.4g*x) + C2*e^(%.4g*x)", r1, r2);
        } else if (D < -1e-9) {
            double al = -A / 2, be = sqrt(-D) / 2;
            snprintf(out, outLen, "y = e^(%.4g*x)*(C1*cos(%.4g*x)+C2*sin(%.4g*x))", al, be, be);
        } else {
            double r = -A / 2;
            snprintf(out, outLen, "y = (C1+C2*x)*e^(%.4g*x)", r);
        }
        return true;
    }

    bool doImpDiff(const char* s, char* out, int outLen) {
        char fx[100] = "", fy[100] = "";
        if (!doDerivativeImpl(s, 'x', fx, sizeof(fx))) return false;
        if (!doDerivativeImpl(s, 'y', fy, sizeof(fy))) return false;
        char fytr[100]; int bi = 0;
        while (fy[bi] && bi < 99) { fytr[bi] = fy[bi]; bi++; }
        fytr[bi] = 0;
        bool zeroFx = !strcmp(fx, "0");
        bool zeroFy = !strcmp(fy, "0");
        if (zeroFy) { snprintf(out, outLen, "dy/dx undefined (Fy=0: vertical tangent)"); return true; }
        if (zeroFx) { snprintf(out, outLen, "dy/dx = 0  (horizontal tangent)"); return true; }
        snprintf(out, outLen, "dy/dx = -(%s)/(%s)", fx, fytr);
        return true;
    }

    bool doTaylor(const char* s, char* out, int outLen) {
        char parts[3][64]; int pi = 0, pj = 0, d = 0;
        for (int i = 0; s[i] && pi < 3; i++) {
            char c = s[i];
            if (c == '(') d++;
            else if (c == ')') d--;
            if (c == ',' && d == 0) { parts[pi][pj]=0; pi++; pj=0; continue; }
            if (pj < 63) parts[pi][pj++] = c;
        }
        if (pi < 3) { parts[pi][pj]=0; pi++; }
        if (pi < 3) return false;
        char expr[80]; int j = 0;
        for (int i = 0; parts[0][i] && j < 79; i++) if (parts[0][i] != ' ') expr[j++] = parts[0][i];
        expr[j] = 0;
        double a = atof(parts[1]);
        int n = atoi(parts[2]);
        if (n < 1 || n > 10) return false;
        char cur[80]; strncpy(cur, expr, 79); cur[79] = 0;
        double coef[11];
        for (int k = 0; k <= n; k++) {
            ExprEval::Subst sx = {'x', a};
            auto r = gEv.eval(cur, &sx, 1);
            if (!r.ok) return false;
            double fact = 1; for (int t = 2; t <= k; t++) fact *= t;
            coef[k] = r.val / fact;
            if (k < n) {
                char next[80];
                if (!doDerivativeImpl(cur, 'x', next, sizeof(next))) return false;
                strncpy(cur, next, 79); cur[79] = 0;
            }
        }
        char buf[200]; int bp = 0;
        bool first = true;
        for (int k = 0; k <= n && bp < (int)sizeof(buf)-50; k++) {
            if (fabs(coef[k]) < 1e-9) continue;
            char term[40]; const char* sg;
            double v = coef[k];
            if (first) { sg = (v<0)?"-":""; first = false; }
            else       { sg = (v<0)?" - ":" + "; }
            double av = fabs(v);
            if (k == 0)      snprintf(term, sizeof(term), "%s%.6g", sg, av);
            else if (k == 1) {
                if (fabs(a) < 1e-12) snprintf(term, sizeof(term), "%s%.6g*x", sg, av);
                else if (a > 0)      snprintf(term, sizeof(term), "%s%.6g*(x-%.4g)", sg, av, a);
                else                 snprintf(term, sizeof(term), "%s%.6g*(x+%.4g)", sg, av, -a);
            } else {
                if (fabs(a) < 1e-12) snprintf(term, sizeof(term), "%s%.6g*x^%d", sg, av, k);
                else if (a > 0)      snprintf(term, sizeof(term), "%s%.6g*(x-%.4g)^%d", sg, av, a, k);
                else                 snprintf(term, sizeof(term), "%s%.6g*(x+%.4g)^%d", sg, av, -a, k);
            }
            bp += snprintf(buf+bp, sizeof(buf)-bp, "%s", term);
        }
        if (first) snprintf(buf, sizeof(buf), "0");
        bp = strlen(buf);
        if (bp < (int)sizeof(buf) - 20) snprintf(buf+bp, sizeof(buf)-bp, " + O((x-a)^%d)", n+1);
        strncpy(out, buf, outLen-1); out[outLen-1] = 0;
        return true;
    }

    bool doTangent(const char* s, char* out, int outLen) {int L=strlen(s),d=0,cp=-1;for(int i=0;i<L;i++){char c=s[i];if(c=='(')d++;else if(c==')')d--;else if(c==','&&d==0)cp=i;}if(cp<0)return false;int el=cp;while(el>0&&s[el-1]==' ')el--;if(el<=0||el>=80)return false;char expr[80];memcpy(expr,s,el);expr[el]=0;int as=cp+1;while(as<L&&s[as]==' ')as++;int ae=L;while(ae>as&&s[ae-1]==' ')ae--;if(ae-as<1||ae-as>=24)return false;char ab[24];memcpy(ab,s+as,ae-as);ab[ae-as]=0;double a=atof(ab);char fp[80];if(!doDerivativeImpl(expr,'x',fp,sizeof(fp)))return false;ExprEval::Subst sx={'x',a};auto rf=gEv.eval(expr,&sx,1);auto rf1=gEv.eval(fp,&sx,1);if(!rf.ok||!rf1.ok)return false;double m=rf1.val,b=rf.val;double c0=b-m*a;char s0[40],s1[40];snprintf(s0,sizeof(s0),"%.6g",m);snprintf(s1,sizeof(s1),"%.6g",c0);if(fabs(m)<1e-12)snprintf(out,outLen,"y = %s",s1);else if(fabs(c0)<1e-12)snprintf(out,outLen,"y = %s*x",s0);else if(c0>0)snprintf(out,outLen,"y = %s*x + %s",s0,s1);else snprintf(out,outLen,"y = %s*x - %.6g",s0,-c0);return true;}
    bool doNormal(const char* s, char* out, int outLen) {int L=strlen(s),d=0,cp=-1;for(int i=0;i<L;i++){char c=s[i];if(c=='(')d++;else if(c==')')d--;else if(c==','&&d==0)cp=i;}if(cp<0)return false;int el=cp;while(el>0&&s[el-1]==' ')el--;if(el<=0||el>=80)return false;char expr[80];memcpy(expr,s,el);expr[el]=0;int as=cp+1;while(as<L&&s[as]==' ')as++;int ae=L;while(ae>as&&s[ae-1]==' ')ae--;if(ae-as<1||ae-as>=24)return false;char ab[24];memcpy(ab,s+as,ae-as);ab[ae-as]=0;double a=atof(ab);char fp[80];if(!doDerivativeImpl(expr,'x',fp,sizeof(fp)))return false;ExprEval::Subst sx={'x',a};auto rf=gEv.eval(expr,&sx,1);auto rf1=gEv.eval(fp,&sx,1);if(!rf.ok||!rf1.ok)return false;double m=rf1.val,b=rf.val;if(fabs(m)<1e-12){snprintf(out,outLen,"x = %.6g",a);return true;}double mn=-1.0/m;double c0=b-mn*a;char s0[40],s1[40];snprintf(s0,sizeof(s0),"%.6g",mn);snprintf(s1,sizeof(s1),"%.6g",c0);if(fabs(c0)<1e-12)snprintf(out,outLen,"y = %s*x",s0);else if(c0>0)snprintf(out,outLen,"y = %s*x + %s",s0,s1);else snprintf(out,outLen,"y = %s*x - %.6g",s0,-c0);return true;}
    bool doFMinMax(const char* s, bool wantMax, char* out, int outLen) {char p[3][64];int pi=0,pj=0,d=0;for(int i=0;s[i]&&pi<3;i++){char c=s[i];if(c=='(')d++;else if(c==')')d--;if(c==','&&d==0){p[pi][pj]=0;pi++;pj=0;continue;}if(pj<63)p[pi][pj++]=c;}if(pi<3){p[pi][pj]=0;pi++;}if(pi<3)return false;char expr[80];int j=0;for(int i=0;p[0][i]&&j<79;i++)if(p[0][i]!=' ')expr[j++]=p[0][i];expr[j]=0;double a=atof(p[1]),b=atof(p[2]);if(a>=b)return false;ExprEval::Subst sx={'x',a};auto r=gEv.eval(expr,&sx,1);if(!r.ok)return false;double bestX=a,bestY=r.val;auto consider=[&](double xv){sx.val=xv;auto rr=gEv.eval(expr,&sx,1);if(!rr.ok)return;if(wantMax?rr.val>bestY:rr.val<bestY){bestY=rr.val;bestX=xv;}};consider(b);const int N=100;double dx=(b-a)/N;double prev=NumAnalysis::diffN(expr,'x',a);for(int i=1;i<=N;i++){double x=a+i*dx;double cur=NumAnalysis::diffN(expr,'x',x);if(prev*cur<0){double lo=x-dx,hi=x,pl=prev;for(int k=0;k<50;k++){double m=(lo+hi)/2;double fm=NumAnalysis::diffN(expr,'x',m);if(pl*fm<0)hi=m;else{lo=m;pl=fm;}}consider((lo+hi)/2);}prev=cur;}const int GN=1000;double gBestY=wantMax?-INFINITY:INFINITY;double gBestX=a;for(int i=0;i<=GN;i++){double x=a+(b-a)*i/GN;sx.val=x;auto rr=gEv.eval(expr,&sx,1);if(!rr.ok||isnan(rr.val)||isinf(rr.val))continue;if(wantMax?rr.val>gBestY:rr.val<gBestY){gBestY=rr.val;gBestX=x;}}if(wantMax?gBestY>bestY:gBestY<bestY){double lo=gBestX-(b-a)/GN,hi=gBestX+(b-a)/GN;if(lo<a)lo=a;if(hi>b)hi=b;for(int k=0;k<80;k++){double m1=lo+(hi-lo)*0.382;double m2=lo+(hi-lo)*0.618;sx.val=m1;auto r1=gEv.eval(expr,&sx,1);sx.val=m2;auto r2=gEv.eval(expr,&sx,1);if(!r1.ok||!r2.ok)break;if(wantMax?r1.val>r2.val:r1.val<r2.val)hi=m2;else lo=m1;}double xb=(lo+hi)/2;sx.val=xb;auto rr=gEv.eval(expr,&sx,1);if(rr.ok){bestX=xb;bestY=rr.val;}}snprintf(out,outLen,"%s at x=%.6g, f=%.6g",wantMax?"max":"min",bestX,bestY);return true;}
    bool doFMin(const char* s, char* out, int outLen) { return doFMinMax(s, false, out, outLen); }
    bool doFMax(const char* s, char* out, int outLen) { return doFMinMax(s, true, out, outLen); }
    bool doDiffQ(const char* s, char* out, int outLen) {char p[3][64];int pi=0,pj=0,d=0;for(int i=0;s[i]&&pi<3;i++){char c=s[i];if(c=='(')d++;else if(c==')')d--;if(c==','&&d==0){p[pi][pj]=0;pi++;pj=0;continue;}if(pj<63)p[pi][pj++]=c;}if(pi<3){p[pi][pj]=0;pi++;}if(pi<3)return false;char expr[80];int j=0;for(int i=0;p[0][i]&&j<79;i++)if(p[0][i]!=' ')expr[j++]=p[0][i];expr[j]=0;double x=atof(p[1]),h=atof(p[2]);if(h==0)h=1e-4;ExprEval::Subst sp={'x',x+h},sm={'x',x-h};auto rp=gEv.eval(expr,&sp,1),rm=gEv.eval(expr,&sm,1);if(!rp.ok||!rm.ok)return false;double q=(rp.val-rm.val)/(2*h);snprintf(out,outLen,"%.6g  (h=%.4g)",q,h);return true;}
    bool doArcLength(const char* s, char* out, int outLen) {char p[3][64];int pi=0,pj=0,d=0;for(int i=0;s[i]&&pi<3;i++){char c=s[i];if(c=='(')d++;else if(c==')')d--;if(c==','&&d==0){p[pi][pj]=0;pi++;pj=0;continue;}if(pj<63)p[pi][pj++]=c;}if(pi<3){p[pi][pj]=0;pi++;}if(pi<3)return false;char expr[80];int j=0;for(int i=0;p[0][i]&&j<79;i++)if(p[0][i]!=' ')expr[j++]=p[0][i];expr[j]=0;double a=atof(p[1]),b=atof(p[2]);if(a>=b)return false;const int N=200;double h=(b-a)/N;double sum=0;for(int i=0;i<=N;i++){double x=a+i*h;double fp=NumAnalysis::diffN(expr,'x',x);double f=sqrt(1+fp*fp);double coef=(i==0||i==N)?1:((i&1)?4:2);sum+=coef*f;}double L=h*sum/3.0;snprintf(out,outLen,"L = %.6g  (Simpson n=%d)",L,N);return true;}
    bool doLeadingTerm(const char* s, char* out, int outLen) {
        Polynomial p;
        if (parsePoly(s, p)) {
            if (p.deg == 0) { snprintf(out, outLen, "%.6g", p.c[0]); return true; }
            if (p.deg == 1) {
                if (fabs(p.c[1] - 1) < 1e-9) snprintf(out, outLen, "x");
                else snprintf(out, outLen, "%.6g*x", p.c[1]);
            } else {
                if (fabs(p.c[p.deg] - 1) < 1e-9) snprintf(out, outLen, "x^%d", p.deg);
                else snprintf(out, outLen, "%.6g*x^%d", p.c[p.deg], p.deg);
            }
            return true;
        }
        char nb[80], db[40];
        if (dpSplit(s, nb, 80, db, 40)) {
            Polynomial P, Q;
            if (parsePoly(nb, P) && parsePoly(db, Q)) {
                if (fabs(Q.c[Q.deg]) < 1e-15) return false;
                double coef = P.c[P.deg] / Q.c[Q.deg];
                int exp = P.deg - Q.deg;
                if (exp == 0) snprintf(out, outLen, "%.6g", coef);
                else if (exp == 1) {
                    if (fabs(coef - 1) < 1e-9) snprintf(out, outLen, "x");
                    else snprintf(out, outLen, "%.6g*x", coef);
                } else if (exp > 1) {
                    if (fabs(coef - 1) < 1e-9) snprintf(out, outLen, "x^%d", exp);
                    else snprintf(out, outLen, "%.6g*x^%d", coef, exp);
                } else {
                    int ne = -exp;
                    if (fabs(coef - 1) < 1e-9) snprintf(out, outLen, "1/x^%d", ne);
                    else snprintf(out, outLen, "%.6g/x^%d", coef, ne);
                }
                return true;
            }
        }
        return false;
    }

    bool doSeries(const char* s, char* out, int outLen) { return doTaylor(s, out, outLen); }
};

int CasCommands::dParseExpr() {
    int left = dParseTerm();
    while (true) { dSkip();
        if      (*dpp == '+') { dpp++; int r = dParseTerm(); left = dNewOp('+', left, r); }
        else if (*dpp == '-') { dpp++; int r = dParseTerm(); left = dNewOp('-', left, r); }
        else break;
    }
    return left;
}

CasCommands gCas;
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
            if (IS_ESC) return;
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
            if (st.del) { if (cur > 0) { int _l = strlen(input); for (int _i = cur - 1; _i < _l - 1; _i++) input[_i] = input[_i+1]; input[_l - 1] = 0; cur--; } needRedraw = true; continue; } /* Cursor edit mode */
            if (st.enter) {
                if (input[0] == 0) { needRedraw = true; continue; }
                if (G.rpn) processRPN(input);
                else        processAlgebraic(input);
                input[0] = 0; cur = 0;
                needRedraw = true;
                continue;
            }
            bool skipInsert = false;
            for (auto c : st.word) {
                if (c == '<' || c == '>') { skipInsert = true; continue; }
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
                if (cur < 79 && c >= ' ' && c < 127) {
                    char cc = (c == '_') ? '-' : ((c == '-') ? '_' : c);
                    int _l = strlen(input);
                    if (cur < _l) for (int _i = _l; _i > cur; _i--) input[_i] = input[_i-1];
                    input[cur++] = cc;
                    input[cur] = 0;
                }
            }
            if (!G.rpn) {
                if (M5Cardputer.Keyboard.isKeyPressed('<')) { if (cur > 0) cur--; needRedraw = true; }
                if (M5Cardputer.Keyboard.isKeyPressed('>')) { if (cur < (int)strlen(input)) cur++; needRedraw = true; }
            }
            (void)skipInsert;
            if (!st.word.empty()) needRedraw = true;
        }
    }
private:
    static bool isSimpleFrac(const char* s, char* num, char* den) {
        const char* sl = strchr(s, '/');
        if (!sl) return false;
        int nl = sl - s;
        if (nl <= 0 || nl >= 16) return false;
        for (int i = 0; i < nl; i++) if (!isdigit((unsigned char)s[i]) && s[i] != '-') return false;
        const char* dp = sl + 1; int dl = strlen(dp);
        if (dl <= 0 || dl >= 16) return false;
        for (int i = 0; i < dl; i++) if (!isdigit((unsigned char)dp[i])) return false;
        memcpy(num, s, nl); num[nl] = 0; strcpy(den, dp);
        return true;
    }

    void drawFrac2D(int x, int y, const char* num, const char* den, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int nw = textWidth(num), dw = textWidth(den);
        int w = (nw > dw) ? nw : dw;
        int avail = LCD_W - 4 - x;
        if (avail < 4) return;
        if (w > avail) { if (nw > avail) nw = avail; if (dw > avail) dw = avail; w = avail; }
        d.setTextColor(col, C_BG);
        d.drawString(num, x + (w - nw) / 2, y - 10);
        d.drawString(den, x + (w - dw) / 2, y + 2);
        d.drawFastHLine(x, y, w, col);
    }    int drawNatural(int x, int y, const char* s, uint16_t col) {
        auto& d = M5Cardputer.Display;
        d.setTextColor(col, C_BG);
        int cx = x;
        for (int i = 0; s[i]; ) {
            if (s[i] == '^' && (isdigit((unsigned char)s[i+1]) || s[i+1] == '-')) {
                i++;
                char tmp[4]; int tn = 0;
                while (s[i] && (isdigit((unsigned char)s[i]) || s[i]=='-') && tn < 3) tmp[tn++] = s[i++];
                tmp[tn] = 0;
                int tw = textWidth(tmp);
                if (cx + tw > LCD_W - 4) break;
                d.drawString(tmp, cx, y - 3);
                cx += tw - 1;
            } else {
                char t[2] = { s[i], 0 };
                int tw = textWidth(t);
                if (cx + tw > LCD_W - 4) break;
                d.drawString(t, cx, y);
                cx += tw;
                i++;
            }
        }
        return cx;
    }    void drawScreen(const char* input, int cur) {
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
                drawNatural(4, y, l, C_DIM); y += 13;
            }
        }
        char ansStr[40];
        if (G.ansComplex) G.ansC.toString(ansStr, sizeof(ansStr));
        else if (G.fracDisplay) {
            char tmp[40];
            if (tryDecimalToSpecial(G.ans, tmp, sizeof(tmp))) strcpy(ansStr, tmp);
            else formatNum(G.ans, ansStr, sizeof(ansStr));
        } else                  formatNum(G.ans, ansStr, sizeof(ansStr));
        d.setTextColor(C_OK, C_BG);
        d.drawString("Ans =", 4, LCD_H - FTR_H - 30);
        char _fn[20], _fd[20];
        if (isSimpleFrac(ansStr, _fn, _fd)) {
            drawFrac2D(46, LCD_H - FTR_H - 24, _fn, _fd, C_OK);
        } else {
            drawNatural(46, LCD_H - FTR_H - 30, ansStr, C_OK);
        }
        d.fillRect(0, LCD_H - FTR_H - 17, LCD_W, 16, C_PANEL);
        d.drawFastHLine(0, LCD_H - FTR_H - 17, LCD_W, C_ACC);
        d.setTextColor(C_FG, C_PANEL);
        const int fieldX = 4;
        const int fieldW = LCD_W - 8;
        const int maxChars = fieldW / 6;
        int len = strlen(input);
        int startCh = 0;
        if (len > maxChars) {
            startCh = len - maxChars;
            if (cur < startCh) startCh = cur;
            else if (cur > startCh + maxChars - 1) startCh = cur - maxChars + 1;
        }
        const char* visible = input + startCh;
        d.drawString(visible, fieldX, LCD_H - FTR_H - 14);
        int visCur = cur - startCh;
        char tmpVis[80]; int vl = strlen(visible);
        if (visCur < 0) visCur = 0;
        if (visCur > vl) visCur = vl;
        memcpy(tmpVis, visible, visCur); tmpVis[visCur] = 0;
        int cx = fieldX + textWidth(tmpVis);
        if (cx > LCD_W - 6) cx = LCD_W - 6;
        d.drawFastVLine(cx, LCD_H - FTR_H - 16, 14, C_ACC);
    }

    bool isPureIdent(const char* s, int len) {
        if (len < 1) return false;
        if (!isalpha((unsigned char)s[0])) return false;
        for (int i = 1; i < len; i++)
            if (!isalnum((unsigned char)s[i]) && s[i] != '_') return false;
        return true;
    }

    bool tryAssignOrFunc(const char* expr) {
        int len = strlen(expr), eqPos = -1, depth = 0;
        for (int i = 0; i < len; i++) {
            char c = expr[i];
            if (c == '(') depth++;
            else if (c == ')') { if (depth > 0) depth--; }
            else if (c == '=' && depth == 0) {
                if (i+1 < len && expr[i+1] == '=') return false;
                if (eqPos >= 0) return false;
                eqPos = i;
            }
        }
        if (eqPos < 0) return false;
        int ls = 0, le = eqPos - 1;
        while (ls <= le && expr[ls] == ' ') ls++;
        while (le >= ls && expr[le] == ' ') le--;
        if (ls > le) return false;
        int lhsLen = le - ls + 1;
        if (lhsLen > 15) return false;
        char lhs[16]; memcpy(lhs, expr+ls, lhsLen); lhs[lhsLen] = 0;
        int rs = eqPos + 1;
        while (rs < len && expr[rs] == ' ') rs++;
        if (rs >= len) return false;
        const char* rhs = expr + rs;
        char* lp = strchr(lhs, '(');
        if (lp) {
            char* rp = strchr(lp, ')');
            if (!rp || rp[1] != 0) return false;
            int nl = lp - lhs;
            if (nl < 1 || nl > 11) return false;
            if (rp - lp != 2) { DM.notify("1-char param", C_ERR, 1000); return true; }
            char prm = lp[1];
            if (!isalpha((unsigned char)prm)) return false;
            if (!isPureIdent(lhs, nl)) return false;
            static const char* BAD[] = {"sin","cos","tan","asin","acos","atan",
                "sinh","cosh","tanh","exp","ln","log","sqrt","abs","floor",
                "ceil","round","fact","gamma","pow","mod","gcd","min","max",0};
            for (int i = 0; BAD[i]; i++)
                if (!strcmp(lhs, BAD[i])) { DM.notify("reserved name", C_ERR, 1000); return true; }
            ExprEval::Subst us = { prm, 1.0 };
            auto tr = gEv.eval(rhs, &us, 1);
            if (!tr.ok) { DM.notify(tr.err ? tr.err : "err", C_ERR, 1000); return true; }
            gUserFns.remove(lhs);
            if (!gUserFns.add(lhs, prm, rhs)) { DM.notify("fn bank full", C_ERR, 1000); return true; }
            char m[32]; snprintf(m, sizeof(m), "Def: %s(%c)", lhs, prm);
            DM.notify(m, C_OK, 1000);
            G.pushHist(expr, 0, true);
            return true;
        }
        if (isPureIdent(lhs, lhsLen)) {
            if (lhsLen == 1) {
                char c = tolower(lhs[0]);
                if (c >= 'a' && c <= 'j') {
                    auto r = gEv.eval(rhs);
                    if (!r.ok) { DM.notify(r.err ? r.err : "err", C_ERR, 1000); return true; }
                    G.vars[c - 'a'] = r.val;
                    char vs[24]; formatNum(r.val, vs, sizeof(vs));
                    char m[32]; snprintf(m, sizeof(m), "%c = %s", toupper((unsigned char)c), vs);
                    DM.notify(m, C_OK, 800);
                    G.pushHist(expr, r.val, true);
                    return true;
                }
            }
            DM.notify("use f(x)= form", C_WARN, 1200);
            return true;
        }
        return false;
    }

    bool tryCasCommand(const char* expr) {
        int len = strlen(expr);
        int i = 0; while (i < len && expr[i]==' ') i++;
        int start = i;
        while (i < len && (isalpha((unsigned char)expr[i]) || expr[i]=='_')) i++;
        int cl = i - start;
        if (cl < 1 || cl >= 16) return false;
        char cmd[16]; memcpy(cmd, expr+start, cl); cmd[cl] = 0;
        if (i >= len || expr[i] != '(') return false;
        int e = len - 1;
        while (e > i && expr[e]==' ') e--;
        if (e <= i || expr[e] != ')') return false;
        char arg[80]; int al = e - i - 1;
        if (al < 1 || al >= 80) return false;
        memcpy(arg, expr+i+1, al); arg[al] = 0;
        char result[128];
        if (!gCas.run(cmd, arg, result, sizeof(result))) return false;
        showCasResult(cmd, result);
        return true;
    }

    void showCasResult(const char* cmd, const char* result) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header(cmd);
        DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        DM.drawWrapped(result, 4, HDR_H + 4, LCD_W - 8, C_OK);
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
        }
    }

    void processAlgebraic(const char* expr) {
        if (tryAssignOrFunc(expr)) return;
        if (tryCasCommand(expr)) return;
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
                "Norm 1/2 threshold","Auto power off (min)","Reset all","Back"
            };
            int s = DM.menu("Settings", items, 7);
            if (s < 0 || s == 6) return;
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
                char b[8]; snprintf(b, sizeof(b), "%d", G.autoOffMin);
                if (DM.inputLine("auto off min (0=off):", b, sizeof(b))) {
                    int v = atoi(b);
                    if (v >= 0 && v <= 600) G.autoOffMin = v;
                }
            }
            else if (s == 5) {
                if (DM.confirm("Reset all?")) { G.resetAll(); DM.notify("Reset done", C_OK); }
            }
        }
    }

    void historyMenu(char* input, int& cur) {
        if (G.histN == 0) { DM.notify("Empty", C_DIM); return; }
        char** labels = (char**)malloc(G.histN * sizeof(char*));
        if (!labels) { DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < G.histN; i++) {
            labels[i] = (char*)malloc(80);
            if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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
            if (!labels) return;
            for (int i = 0; i < VAR_COUNT; i++) {
                labels[i] = (char*)malloc(40); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
                if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
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
            if (!labels) return;
            for (int i = 0; i < G.bankN; i++) {
                labels[i] = (char*)malloc(48); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
                if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
                char vs[24]; formatNum(G.bank[i], vs, sizeof(vs));
                if (G.bankLabel[i][0]) snprintf(labels[i], 48, "%-8s = %s", G.bankLabel[i], vs);
                else                   snprintf(labels[i], 48, "[%d]      = %s", i, vs);
            }
            int op = G.bankN;
            labels[op] = (char*)malloc(48); labels[op+1] = (char*)malloc(48); labels[op+2] = (char*)malloc(48); labels[op+3] = (char*)malloc(48);
            if (!labels[op] || !labels[op+1] || !labels[op+2] || !labels[op+3]) { for (int k = 0; k < op + 4; k++) free(labels[k]); free(labels); return; }
            snprintf(labels[op], 48, "[+] Add current Ans"); snprintf(labels[op+1], 48, "[V] Pop to variable A..J");
            snprintf(labels[op+2], 48, "[C] Clear all bank"); snprintf(labels[op+3], 48, "[<-] Back");
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
            if (IS_ESC) return;
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
            if (IS_ESC) return;
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
        for (;;) {
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
        if (n > 0) {
            G.ans = re[0]; G.ansComplex = (fabs(im[0]) > 1e-9);
            if (G.ansComplex) G.ansC = Complex(re[0], im[0]);
        }
        DM.waitEsc();
    }
};

// ── Part 12b: 最適化 (Nelder-Mead / 勾配降下) と線形計画 (2段階シンプレックス法) ──
namespace Optim {
#define OPT_MAXN 4
#define LP_MAXN  6
#define LP_MAXM  8
#define LP_W     (LP_MAXN + 2*LP_MAXM + 1)
enum { LP_OK = 0, LP_INFEASIBLE = 1, LP_UNBOUNDED = 2, LP_ITER = 3 };
typedef double (*ObjFn)(const double* x, int n, void* ctx);
// Nelder-Mead 法 (n <= OPT_MAXN)。戻り値: 反復回数
inline int nelderMead(ObjFn f, void* ctx, int n, const double* x0, double* xb, double* fb, int maxIter = 800) {
    double P[OPT_MAXN+1][OPT_MAXN], F[OPT_MAXN+1];
    for (int v = 0; v <= n; v++) {
        for (int i = 0; i < n; i++) P[v][i] = x0[i];
        if (v > 0) P[v][v-1] += 0.1 * fabs(x0[v-1]) + 0.1;
        F[v] = f(P[v], n, ctx);
    }

    int it = 0;
    for (; it < maxIter; it++) {
        for (int a = 1; a <= n; a++) {            // 挿入ソート (F 昇順)
            double fk = F[a], pk[OPT_MAXN];
            for (int i = 0; i < n; i++) pk[i] = P[a][i];
            int b = a - 1;
            while (b >= 0 && F[b] > fk) {
                F[b+1] = F[b];
                for (int i = 0; i < n; i++) P[b+1][i] = P[b][i];
                b--;
            }
            F[b+1] = fk;
            for (int i = 0; i < n; i++) P[b+1][i] = pk[i];
        }
        double size = 0;
        for (int v = 1; v <= n; v++)
            for (int i = 0; i < n; i++) { double d = fabs(P[v][i] - P[0][i]); if (d > size) size = d; }
        if (fabs(F[n] - F[0]) <= 1e-13 * (1 + fabs(F[0])) && size <= 1e-9 * (1 + fabs(P[0][0]))) break;
        double cen[OPT_MAXN] = {0}, xr[OPT_MAXN], xe[OPT_MAXN], xc[OPT_MAXN];
        for (int v = 0; v < n; v++) for (int i = 0; i < n; i++) cen[i] += P[v][i] / n;
        for (int i = 0; i < n; i++) xr[i] = cen[i] + (cen[i] - P[n][i]);
        double fr = f(xr, n, ctx);
        if (fr < F[0]) {
            for (int i = 0; i < n; i++) xe[i] = cen[i] + 2 * (cen[i] - P[n][i]);
            double fe = f(xe, n, ctx);
            if (fe < fr) { for (int i = 0; i < n; i++) P[n][i] = xe[i]; F[n] = fe; }
            else         { for (int i = 0; i < n; i++) P[n][i] = xr[i]; F[n] = fr; }
        } else if (fr < F[n-1]) {
            for (int i = 0; i < n; i++) P[n][i] = xr[i]; F[n] = fr;
        } else {
            bool outside = (fr < F[n]);
            for (int i = 0; i < n; i++)
                xc[i] = outside ? cen[i] + 0.5 * (xr[i] - cen[i]) : cen[i] + 0.5 * (P[n][i] - cen[i]);
            double fc = f(xc, n, ctx);
            if (fc < (outside ? fr : F[n])) { for (int i = 0; i < n; i++) P[n][i] = xc[i]; F[n] = fc; }
            else {
                for (int v = 1; v <= n; v++) {
                    for (int i = 0; i < n; i++) P[v][i] = P[0][i] + 0.5 * (P[v][i] - P[0][i]);
                    F[v] = f(P[v], n, ctx);
                }
            }
        }
    }

    int bi = 0;
    for (int v = 1; v <= n; v++) if (F[v] < F[bi]) bi = v;
    for (int i = 0; i < n; i++) xb[i] = P[bi][i];
    *fb = F[bi];
    return it;
}

// 勾配降下法 (中心差分の勾配 + Armijo 後退直線探索)。戻り値: 反復回数
inline int gradDescent(ObjFn f, void* ctx, int n, const double* x0, double* xb, double* fb, int maxIter = 3000) {
    double x[OPT_MAXN], xn[OPT_MAXN], g[OPT_MAXN];
    for (int i = 0; i < n; i++) x[i] = x0[i];
    double fx = f(x, n, ctx), t = 1.0;
    int it = 0;
    for (; it < maxIter; it++) {
        double gn2 = 0;
        for (int i = 0; i < n; i++) {
            double h = 1e-6 * (1 + fabs(x[i])), xi = x[i];
            x[i] = xi + h; double fp = f(x, n, ctx);
            x[i] = xi - h; double fm = f(x, n, ctx);
            x[i] = xi;
            g[i] = (fp - fm) / (2 * h);
            gn2 += g[i] * g[i];
        }
        if (!(gn2 > 1e-20)) break;
        t = fmin(t * 2, 1e3);
        bool ok = false;
        while (t > 1e-16) {
            for (int i = 0; i < n; i++) xn[i] = x[i] - t * g[i];
            double fn = f(xn, n, ctx);
            if (fn <= fx - 1e-4 * t * gn2) { for (int i = 0; i < n; i++) x[i] = xn[i]; fx = fn; ok = true; break; }
            t *= 0.5;
        }
        if (!ok) break;
    }
    for (int i = 0; i < n; i++) xb[i] = x[i];
    *fb = fx;
    return it;
}

inline void lpPivot(double T[][LP_W], int rows, int cols, int pr, int pc) {
    double pv = T[pr][pc];
    for (int j = 0; j < cols; j++) T[pr][j] /= pv;
    for (int i = 0; i < rows; i++) {
        if (i == pr) continue;
        double fct = T[i][pc];
        if (fct != 0) for (int j = 0; j < cols; j++) T[i][j] -= fct * T[pr][j];
    }
}

// 行0 = 目的関数行 (最小化、被約費用)。Bland の規則で巡回を防止。
inline int lpIterate(double T[][LP_W], int m, int cols, int* basis, int allow) {
    const int rhs = cols - 1;
    for (int it = 0; it < 500; it++) {
        int pc = -1;
        for (int j = 0; j < allow; j++) if (T[0][j] < -1e-9) { pc = j; break; }
        if (pc < 0) return LP_OK;
        int pr = -1; double best = 1e300;
        for (int i = 1; i <= m; i++) {
            if (T[i][pc] > 1e-9) {
                double r = T[i][rhs] / T[i][pc];
                if (r < best - 1e-12 || (fabs(r - best) <= 1e-12 && pr >= 0 && basis[i] < basis[pr])) { best = r; pr = i; }
            }
        }
        if (pr < 0) return LP_UNBOUNDED;
        lpPivot(T, m + 1, cols, pr, pc);
        basis[pr] = pc;
    }
    return LP_ITER;
}

// max/min c.x  s.t.  A x (rel) b,  x >= 0。rel: -1 は <=、0 は =、+1 は >=
inline int simplexLP(int n, int m, const double* c, const double A[][LP_MAXN], const int* rel,
                     const double* b, bool maximize, double* x, double* obj) {
    if (n < 1 || n > LP_MAXN || m < 1 || m > LP_MAXM) return LP_ITER;
    double T[LP_MAXM + 1][LP_W];
    int basis[LP_MAXM + 1];
    const int cols = n + 2 * m + 1, rhs = cols - 1, art = n + m;
    for (int i = 0; i <= m; i++) for (int j = 0; j < LP_W; j++) T[i][j] = 0;
    bool phase1 = false;
    for (int i = 0; i < m; i++) {
        int row = i + 1;
        double sg = (b[i] < 0) ? -1.0 : 1.0;
        int r = (int)sg * rel[i];
        for (int j = 0; j < n; j++) T[row][j] = sg * A[i][j];
        T[row][rhs] = sg * b[i];
        if (r < 0)      { T[row][n + i] = 1;  basis[row] = n + i; }
        else if (r > 0) { T[row][n + i] = -1; T[row][art + i] = 1; basis[row] = art + i; phase1 = true; }
        else            { T[row][art + i] = 1; basis[row] = art + i; phase1 = true; }
    }
    if (phase1) {
        for (int i = 0; i < m; i++) if (T[i+1][art + i] != 0) T[0][art + i] = 1;
        for (int row = 1; row <= m; row++)
            if (basis[row] >= art) for (int j = 0; j < cols; j++) T[0][j] -= T[row][j];
        int rc = lpIterate(T, m, cols, basis, cols - 1);
        if (rc != LP_OK) return rc;
        if (-T[0][rhs] > 1e-7) return LP_INFEASIBLE;
        for (int row = 1; row <= m; row++) {
            if (basis[row] < art) continue;
            for (int j = 0; j < art; j++)
                if (fabs(T[row][j]) > 1e-9) { lpPivot(T, m + 1, cols, row, j); basis[row] = j; break; }
        }
    }

    double cost[LP_W];
    for (int j = 0; j < LP_W; j++) cost[j] = 0;
    for (int j = 0; j < n; j++) cost[j] = maximize ? -c[j] : c[j];
    for (int j = 0; j < cols; j++) T[0][j] = (j < LP_W) ? cost[j] : 0;
    T[0][rhs] = 0;
    for (int row = 1; row <= m; row++) {
        double cb = cost[basis[row]];
        if (cb != 0) for (int j = 0; j < cols; j++) T[0][j] -= cb * T[row][j];
    }

    int rc = lpIterate(T, m, cols, basis, art);
    if (rc != LP_OK) return rc;
    for (int j = 0; j < n; j++) x[j] = 0;
    for (int row = 1; row <= m; row++) if (basis[row] < n) x[basis[row]] = T[row][rhs];
    double o = 0; for (int j = 0; j < n; j++) o += c[j] * x[j];
    *obj = o;
    return LP_OK;
}
} // namespace Optim
class NumMenu {
public:
    void run() {
        const char* items[] = {
            "d/dx f(x) at a","Integral f(x) [a,b]","Sum f(i) [a,b]",
            "Product f(i) [a,b]","Optimize f (min/max)","Linear Programming","Back"
        };
        for (;;) {
        int s = DM.menu("Numerical Analysis", items, 7);
        if (s < 0 || s == 6) return;
        if (s == 4) { optimizeMenu(); continue; }
        if (s == 5) { lpMenu(); continue; }
        char e[80] = "x^2";
        if (!DM.inputLine("f(x)=", e, sizeof(e))) continue;
        char b[24];
        if (s == 0) {
            strcpy(b, "1"); if (!DM.inputLine("at x=", b, sizeof(b))) continue;
            double err = 0;
            double r = NumAnalysis::diffN(e, 'x', atof(b), 0.0, &err);
            showResult("df/dx", r, err);
        } else if (s == 1) {
            char a[24] = "0", c[24] = "1";
            if (!DM.inputLine("a:", a, sizeof(a))) continue;
            if (!DM.inputLine("b:", c, sizeof(c))) continue;
            double err = 0;
            double r = NumAnalysis::integN(e, 'x', atof(a), atof(c), 0, &err);
            showResult("Integral", r, err);
        } else if (s == 2) {
            char a[24] = "1", c[24] = "10";
            if (!DM.inputLine("a:", a, sizeof(a))) continue;
            if (!DM.inputLine("b:", c, sizeof(c))) continue;
            double r = NumAnalysis::sumN(e, 'x', atof(a), atof(c));
            showResult("Sum", r);
        } else if (s == 3) {
            char a[24] = "1", c[24] = "5";
            if (!DM.inputLine("a:", a, sizeof(a))) continue;
            if (!DM.inputLine("b:", c, sizeof(c))) continue;
            double r = NumAnalysis::prodN(e, 'x', atof(a), atof(c));
            showResult("Product", r);
        }
        }
    }
private:
    struct OptCtx { const char* expr; int n; bool neg; };
    static double optObj(const double* x, int n, void* ctx) {
        OptCtx* c = (OptCtx*)ctx;
        ExprEval::Subst sb[3] = {{'x', 0}, {'y', 0}, {'z', 0}};
        for (int i = 0; i < n && i < 3; i++) sb[i].val = x[i];
        auto r = gEv.eval(c->expr, sb, n);
        if (!r.ok || isnan(r.val) || isinf(r.val)) return 1e300;
        return c->neg ? -r.val : r.val;
    }

    void optimizeMenu() {
        char e[80] = "(x-1)^2+(y+2)^2";
        if (!DM.inputLine("f(x,y,z)=", e, sizeof(e))) return;
        char b[16] = "2";
        if (!DM.inputLine("# vars (1-3):", b, sizeof(b))) return;
        int n = atoi(b);
        if (n < 1 || n > 3) { DM.notify("1..3", C_ERR); return; }
        const char* gi[] = {"Minimize", "Maximize"};
        int goal = DM.menu("Goal", gi, 2); if (goal < 0) return;
        const char* mi[] = {"Nelder-Mead (simplex)", "Gradient descent"};
        int mt = DM.menu("Method", mi, 2); if (mt < 0) return;
        double x0[OPT_MAXN] = {0};
        const char nm[] = "xyz";
        for (int i = 0; i < n; i++) {
            char p[16]; snprintf(p, sizeof(p), "%c0 =", nm[i]);
            char v[24] = "0";
            if (!DM.inputLine(p, v, sizeof(v))) return;
            x0[i] = atof(v);
        }
        OptCtx ctx = { e, n, goal == 1 };
        if (optObj(x0, n, &ctx) >= 1e300) { DM.notify("f(x0) invalid", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Optimize");
        d.setTextColor(C_DIM, C_BG); d.drawString("Working...", 4, HDR_H + 8);
        double xb[OPT_MAXN], fb;
        int it = (mt == 0) ? Optim::nelderMead(optObj, &ctx, n, x0, xb, &fb)
                           : Optim::gradDescent(optObj, &ctx, n, x0, xb, &fb);
        if (mt == 1 && it >= 2999) {
            double xb2[OPT_MAXN], fb2;
            int it2 = Optim::nelderMead(optObj, &ctx, n, xb, xb2, &fb2);
            if (fb2 < fb) { for (int i = 0; i < n; i++) xb[i] = xb2[i]; fb = fb2; }
            it += it2;
        }
        double fv = (goal == 1) ? -fb : fb;
        d.fillScreen(C_BG); DM.header(goal == 1 ? "Maximum" : "Minimum"); DM.footer("Y:Ans", "", "ESC:back");
        d.setTextColor(C_OK, C_BG);
        for (int i = 0; i < n; i++) {
            char vs[28], l[40]; formatNum(xb[i], vs, sizeof(vs));
            snprintf(l, sizeof(l), "%c* = %s", nm[i], vs);
            d.drawString(l, 4, HDR_H + 6 + i * 13);
        }
        char vs[28], l[44]; formatNum(fv, vs, sizeof(vs));
        snprintf(l, sizeof(l), "f* = %s", vs); d.drawString(l, 4, HDR_H + 6 + n * 13);
        d.setTextColor(C_DIM, C_BG);
        snprintf(l, sizeof(l), "%s  iter=%d", mt == 0 ? "Nelder-Mead" : "GradDesc", it);
        d.drawString(l, 4, HDR_H + 10 + (n + 1) * 13);
        G.ans = fv; G.ansComplex = false;
        DM.waitEsc();
    }

    // "a1 a2 ... an [<=|=|>=] b" を解析 (rel==nullptr なら係数のみ)
    static bool lpParseRow(const char* s, int n, double* a, int* rel, double* rhs) {
        const char* p = s; char* e;
        for (int j = 0; j < n; j++) {
            while (*p == ' ' || *p == ',') p++;
            double v = strtod(p, &e);
            if (e == p) return false;
            a[j] = v; p = e;
        }
        while (*p == ' ') p++;
        if (!rel) return *p == 0;
        if (*p == '<')      { *rel = -1; p++; if (*p == '=') p++; }
        else if (*p == '>') { *rel =  1; p++; if (*p == '=') p++; }
        else if (*p == '=') { *rel =  0; p++; if (*p == '=') p++; else if (*p == '<') { *rel = -1; p++; } else if (*p == '>') { *rel = 1; p++; } }
        else return false;
        double v = strtod(p, &e);
        if (e == p) return false;
        *rhs = v; p = e;
        while (*p == ' ') p++;
        return *p == 0;
    }

    void lpMenu() {
        const char* gi[] = {"Maximize", "Minimize"};
        int goal = DM.menu("LP goal", gi, 2); if (goal < 0) return;
        char b[16] = "2";
        if (!DM.inputLine("# vars (1-6):", b, sizeof(b))) return;
        int n = atoi(b);
        if (n < 1 || n > LP_MAXN) { DM.notify("1..6", C_ERR); return; }
        strcpy(b, "2");
        if (!DM.inputLine("# constraints (1-8):", b, sizeof(b))) return;
        int m = atoi(b);
        if (m < 1 || m > LP_MAXM) { DM.notify("1..8", C_ERR); return; }
        char line[64] = "";
        double c[LP_MAXN] = {0}, A[LP_MAXM][LP_MAXN] = {{0}}, rhs[LP_MAXM] = {0};
        int rel[LP_MAXM] = {0};
        if (!DM.inputLine("c1 .. cn:", line, sizeof(line))) return;
        if (!lpParseRow(line, n, c, nullptr, nullptr)) { DM.notify("Bad objective", C_ERR); return; }
        for (int i = 0; i < m; i++) {
            char p[24]; snprintf(p, sizeof(p), "con%d a.. op b:", i + 1);
            line[0] = 0;
            if (!DM.inputLine(p, line, sizeof(line))) return;
            if (!lpParseRow(line, n, A[i], &rel[i], &rhs[i])) { DM.notify("Bad row", C_ERR); return; }
        }
        double x[LP_MAXN] = {0}, obj = 0;
        int rc = Optim::simplexLP(n, m, c, A, rel, rhs, goal == 0, x, &obj);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Linear Programming"); DM.footer("Y:Ans", "", "ESC:back");
        if (rc != Optim::LP_OK) {
            d.setTextColor(C_ERR, C_BG);
            d.drawString(rc == Optim::LP_INFEASIBLE ? "Infeasible" :
                         rc == Optim::LP_UNBOUNDED  ? "Unbounded"  : "No convergence", 4, HDR_H + 10);
            DM.waitEsc(); return;
        }
        d.setTextColor(C_OK, C_BG);
        for (int j = 0; j < n; j++) {
            char vs[28], l[40]; formatNum(x[j], vs, sizeof(vs));
            snprintf(l, sizeof(l), "x%d = %s", j + 1, vs);
            d.drawString(l, 4, HDR_H + 4 + j * 11);
        }
        char vs[28], l[44]; formatNum(obj, vs, sizeof(vs));
        d.setTextColor(C_ACC, C_BG);
        snprintf(l, sizeof(l), "obj = %s", vs); d.drawString(l, 4, HDR_H + 8 + n * 11);
        G.ans = obj; G.ansComplex = false;
        DM.waitEsc();
    }

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
        for (;;) {
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
            char l[200]; l[0] = '('; l[1] = 0;
            for (int i = 0; i < dim; i++) {
                char vs[16]; formatNum(rv[i], vs, sizeof(vs));
                strncat(l, vs, sizeof(l) - strlen(l) - 3);
                if (i < dim-1) strncat(l, ", ", sizeof(l) - strlen(l) - 1);
            }
            strncat(l, ")", sizeof(l) - strlen(l) - 1);
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
            "Eigenvalues A","Show A","Show B",
            "trace A","A^n","LU (PA=LU)","QR (A=QR)","charPoly A","Eigenvectors A","Back"
        };
        int s = DM.menu("Matrix Op", items, 17);
        if (s < 0 || s == 16) return;
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
            case 10: case 11: case 12: case 13: case 14: case 15: {
                matOpsLA(s, A, R);
            } break;
        }
    }

    void matOpsLA(int s, double* A, double* R) {
        int n = G.matAr;
        if (G.matAr != G.matAc) { DM.notify("Square only", C_ERR); return; }
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) A[i*n+j] = G.matA[i*MAT_MAX+j];
        if (s == 10) { showScalar("trace A", Mat::trace(A, n)); return; }
        if (s == 11) {
            char b[8] = "2";
            if (!DM.inputLine("Power n (int):", b, sizeof(b))) return;
            int p = atoi(b);
            if (p < -64 || p > 64) { DM.notify("Range", C_ERR); return; }
            if (!Mat::matPow(A, n, p, R)) { DM.notify("Singular", C_ERR); return; }
            showMat("A^n", R, n, n); return;
        }
        if (s == 12) {
            double L[100], U[100]; int pm[MAT_MAX]; double pv[MAT_MAX];
            if (!Mat::luDecomp(A, n, L, U, pm)) { DM.notify("Singular", C_ERR); return; }
            showMat("L", L, n, n); showMat("U", U, n, n);
            for (int i = 0; i < n; i++) pv[i] = pm[i] + 1;
            showVec("P (row perm)", pv, n); return;
        }
        if (s == 13) {
            double Q[100];
            if (!Mat::qrDecomp(A, n, Q, R)) { DM.notify("Rank deficient", C_ERR); return; }
            showMat("Q", Q, n, n); showMat("R", R, n, n); return;
        }
        if (s == 14) {
            double c[MAT_MAX+1];
            Mat::charPoly(A, n, c);
            auto& d = M5Cardputer.Display;
            d.fillScreen(C_BG); DM.header("charPoly: sum c[k] l^k"); DM.footer("","","ESC:back");
            d.setTextColor(C_OK, C_BG);
            for (int k = n; k >= 0 && HDR_H + (n-k)*12 < LCD_H - FTR_H - 4; k--) {
                char l[40], vs[24]; formatNum(c[k], vs, sizeof(vs));
                snprintf(l, sizeof(l), "c[%d] = %s", k, vs);
                d.drawString(l, 4, HDR_H + 4 + (n-k)*12);
            }
            DM.waitEsc(); return;
        }
        double er[MAT_MAX] = {0}, ei[MAT_MAX] = {0}, v[MAT_MAX];
        Mat::qrEig(A, n, er, ei);
        int shown = 0;
        for (int i = 0; i < n; i++) {
            bool dup = false;
            for (int j = 0; j < i; j++) if (fabs(er[j]-er[i]) < 1e-7 && fabs(ei[j]-ei[i]) < 1e-7) dup = true;
            if (dup) continue;
            if (fabs(ei[i]) > 1e-9) {
                double vI[MAT_MAX];
                if (!Mat::eigVecC(A, n, er[i], ei[i], v, vI)) continue;
                char t[32]; snprintf(t, sizeof(t), "Re v (l=%g+%gi)", er[i], ei[i]);
                showVec(t, v, n);
                char t2[32]; snprintf(t2, sizeof(t2), "Im v (l=%g+%gi)", er[i], ei[i]);
                showVec(t2, vI, n);
                shown++;
            } else {
                if (!Mat::eigVec(A, n, er[i], v)) continue;
                char t[32]; snprintf(t, sizeof(t), "v (l=%g)", er[i]);
                showVec(t, v, n); shown++;
            }
        }
        if (!shown) DM.notify("No eigvec", C_WARN);
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
        for (;;) {
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
        for (;;) {
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
        int n=atoi(b); if (n < 1 || n > 100000) { DM.notify("trials 1..100000", C_ERR); return; } int h=0;
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
        if (n < 1 || n > 100000) { DM.notify("trials 1..100000", C_ERR); return; }
        int* hist=(int*)calloc(faces,sizeof(int));
        if (!hist) { DM.notify("malloc fail", C_ERR); return; }
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
        if (p <= 0 || p > 1) { DM.notify("0<p<=1", C_ERR); return; }
        char tb[16]="100"; if (!DM.inputLine("trials:",tb,sizeof(tb))) return;
        int n=atoi(tb);
        if (n < 1 || n > 10000) { DM.notify("trials 1..10000", C_ERR); return; }
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
        if (trials < 1 || trials > 100000) { DM.notify("trials 1..100000", C_ERR); return; }
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
        for (;;) {
        const char* items[] = {"Factorize", "Back"};
        int s = DM.menu("Prime Factorize", items, 2);
        if (s < 0 || s == 1) return;
        char b[24]="360"; if (!DM.inputLine("Number:",b,sizeof(b))) continue;
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
    }
};

class DateCalc {
public:
    void run() {
        const char* items[] = {
            "Date difference (days)","Add/subtract days","Day of week",
            "Leap year check","Day of year","Back"
        };
        for (;;) {
        int s = DM.menu("Date Calculator", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: dateDiff(); break;
            case 1: dateAdd();  break;
            case 2: dayOfWeek(); break;
            case 3: leapYear(); break;
            case 4: dayOfYear(); break;
        }
        }
    }
private:
    static bool isLeap(int y) { return (y%4==0 && y%100!=0) || (y%400==0); }
    static int daysInMonth(int y, int m) {
        static const int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (m==2 && isLeap(y)) return 29;
        return d[m-1];
    }

    static long toJD(int y, int m, int d) {
        if (m <= 2) { y--; m += 12; }
        int A = y / 100;
        int B = 2 - A + A/4;
        return (long)(365.25*(y+4716)) + (int)(30.6001*(m+1)) + d + B - 1524;
    }

    static void fromJD(long jd, int& y, int& m, int& d) {
        long a = jd + 32044;
        long b = (4*a + 3) / 146097;
        long c = a - (146097*b)/4;
        long dd = (4*c + 3) / 1461;
        long e = c - (1461*dd)/4;
        long mm = (5*e + 2) / 153;
        d = e - (153*mm + 2)/5 + 1;
        m = mm + 3 - 12*(mm/10);
        y = 100*b + dd - 4800 + mm/10;
    }

    bool askDate(int& y, int& m, int& d) {
        char b[16];
        snprintf(b, sizeof(b), "%d", y);
        if (!DM.inputLine("Year:", b, sizeof(b))) return false; y = atoi(b);
        snprintf(b, sizeof(b), "%d", m);
        if (!DM.inputLine("Month:", b, sizeof(b))) return false; m = atoi(b);
        snprintf(b, sizeof(b), "%d", d);
        if (!DM.inputLine("Day:", b, sizeof(b))) return false; d = atoi(b);
        if (m < 1 || m > 12 || d < 1 || d > daysInMonth(y, m)) return false;
        return true;
    }

    void dateDiff() {
        int y1=2024,m1=1,d1=1, y2=2024,m2=12,d2=31;
        if (!askDate(y1,m1,d1)) return;
        if (!askDate(y2,m2,d2)) return;
        long jd1 = toJD(y1,m1,d1);
        long jd2 = toJD(y2,m2,d2);
        long diff = jd2 - jd1;
        auto& disp = M5Cardputer.Display;
        disp.fillScreen(C_BG); DM.header("Date Difference"); DM.footer("","","ESC:back");
        char l[48];
        snprintf(l,sizeof(l),"%04d-%02d-%02d -> %04d-%02d-%02d", y1,m1,d1, y2,m2,d2);
        disp.setTextColor(C_FG,C_BG); disp.drawString(l,4,HDR_H+4);
        disp.setTextColor(C_OK,C_BG); disp.setTextSize(2);
        snprintf(l,sizeof(l),"Days = %ld", diff);
        disp.drawString(l,4,HDR_H+24); disp.setTextSize(1);
        snprintf(l,sizeof(l),"Weeks = %.2f", diff/7.0);
        disp.setTextColor(C_FG,C_BG); disp.drawString(l,4,HDR_H+50);
        G.ans = (double)diff;
        DM.waitEsc();
    }

    void dateAdd() {
        int y=2024,m=1,d=1;
        if (!askDate(y,m,d)) return;
        char b[16]="30";
        if (!DM.inputLine("Days (+/-):", b, sizeof(b))) return;
        long delta = atol(b);
        long jd = toJD(y,m,d) + delta;
        int y2,m2,d2;
        fromJD(jd, y2,m2,d2);
        auto& disp = M5Cardputer.Display;
        disp.fillScreen(C_BG); DM.header("Date Add"); DM.footer("","","ESC:back");
        char l[40];
        snprintf(l,sizeof(l),"%04d-%02d-%02d %+ld days", y,m,d, delta);
        disp.setTextColor(C_FG,C_BG); disp.drawString(l,4,HDR_H+4);
        disp.setTextColor(C_OK,C_BG); disp.setTextSize(2);
        snprintf(l,sizeof(l),"%04d-%02d-%02d", y2,m2,d2);
        disp.drawString(l,4,HDR_H+24); disp.setTextSize(1);
        const char* wd[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
        int w = (int)((jd + 1) % 7);
        disp.setTextColor(C_FG,C_BG);
        snprintf(l,sizeof(l),"Day of week: %s", wd[w]);
        disp.drawString(l,4,HDR_H+50);
        DM.waitEsc();
    }

    void dayOfWeek() {
        int y=2024,m=1,d=1;
        if (!askDate(y,m,d)) return;
        long jd = toJD(y,m,d);
        const char* wd[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
        int w = (int)((jd + 1) % 7);
        if (w < 0) w += 7;
        char m2[48]; snprintf(m2, sizeof(m2), "%04d-%02d-%02d: %s", y,m,d, wd[w]);
        DM.notify(m2, C_OK, 1500);
    }

    void leapYear() {
        char b[8]="2024";
        if (!DM.inputLine("Year:", b, sizeof(b))) return;
        int y = atoi(b);
        bool leap = isLeap(y);
        char m[32]; snprintf(m, sizeof(m), "%d %s leap year", y, leap?"is":"is NOT");
        DM.notify(m, leap?C_OK:C_DIM, 1500);
    }

    void dayOfYear() {
        int y=2024,m=1,d=1;
        if (!askDate(y,m,d)) return;
        int doy = 0;
        for (int mm=1; mm<m; mm++) doy += daysInMonth(y, mm);
        doy += d;
        char m2[32]; snprintf(m2, sizeof(m2), "Day of year: %d", doy);
        DM.notify(m2, C_OK, 1500);
    }
};

class FinanceCalc {
public:
    void run() {
        const char* items[] = {
            "Compound interest","Simple interest","Loan payment (amortized)",
            "NPV (Net Present Value)","IRR (Internal Rate of Return)","Future value of annuity","Back"
        };
        for (;;) {
        int s = DM.menu("Finance", items, 7);
        if (s < 0 || s == 6) return;
        switch (s) {
            case 0: compound(); break;
            case 1: simple();   break;
            case 2: loanPay();  break;
            case 3: npv();      break;
            case 4: irr();      break;
            case 5: annuity();  break;
        }
        }
    }
private:
    void compound() {
        char b[24];
        strcpy(b,"1000"); if (!DM.inputLine("PV:", b, sizeof(b))) return; double pv = atof(b);
        strcpy(b,"0.05"); if (!DM.inputLine("Annual rate (r):", b, sizeof(b))) return; double r = atof(b);
        strcpy(b,"10"); if (!DM.inputLine("Years (n):", b, sizeof(b))) return; double n = atof(b);
        strcpy(b,"1"); if (!DM.inputLine("Compounds/year:", b, sizeof(b))) return; int m = atoi(b);
        if (m < 1) m = 1;
        double fv = pv * pow(1 + r/m, n*m);
        double interest = fv - pv;
        show2("Compound", "FV", fv, "Interest", interest);
    }

    void simple() {
        char b[24];
        strcpy(b,"1000"); if (!DM.inputLine("PV:", b, sizeof(b))) return; double pv = atof(b);
        strcpy(b,"0.05"); if (!DM.inputLine("Annual rate:", b, sizeof(b))) return; double r = atof(b);
        strcpy(b,"10"); if (!DM.inputLine("Years:", b, sizeof(b))) return; double n = atof(b);
        double fv = pv * (1 + r*n);
        double interest = fv - pv;
        show2("Simple", "FV", fv, "Interest", interest);
    }

    void loanPay() {
        char b[24];
        strcpy(b,"1000000"); if (!DM.inputLine("Loan amount:", b, sizeof(b))) return; double P = atof(b);
        strcpy(b,"0.03"); if (!DM.inputLine("Annual rate:", b, sizeof(b))) return; double r = atof(b);
        strcpy(b,"12"); if (!DM.inputLine("Payments/year:", b, sizeof(b))) return; int m = atoi(b);
        strcpy(b,"10"); if (!DM.inputLine("Years:", b, sizeof(b))) return; double years = atof(b);
        if (m < 1) m = 1;
        double per = r/m;
        double n = years * m;
        if (n < 1e-9) { DM.notify("Years must be > 0", C_ERR); return; }
        double M;
        if (per == 0) M = P / n;
        else M = P * per * pow(1+per, n) / (pow(1+per, n) - 1);
        double total = M * n;
        double interest = total - P;
        show3("Loan", "Payment", M, "Total", total, "Interest", interest);
    }

    void npv() {
        char b[24];
        strcpy(b,"0.1"); if (!DM.inputLine("Discount rate:", b, sizeof(b))) return; double r = atof(b);
        strcpy(b,"5"); if (!DM.inputLine("# cash flows:", b, sizeof(b))) return; int N = atoi(b);
        if (N < 1 || N > 30) { DM.notify("1..30", C_ERR); return; }
        double npv = 0;
        for (int i = 0; i < N; i++) {
            char p[24]; snprintf(p, sizeof(p), "CF%d:", i+1);
            char c[24]="0"; if (!DM.inputLine(p, c, sizeof(c))) return;
            npv += atof(c) / pow(1+r, i+1);
        }
        show1("NPV", npv);
    }

    void irr() {
        char b[24];
        strcpy(b,"5"); if (!DM.inputLine("# cash flows:", b, sizeof(b))) return; int N = atoi(b);
        if (N < 2 || N > 30) { DM.notify("2..30", C_ERR); return; }
        double cf[30];
        for (int i = 0; i < N; i++) {
            char p[24]; snprintf(p, sizeof(p), "CF%d:", i);
            char c[24]="0"; if (!DM.inputLine(p, c, sizeof(c))) return;
            cf[i] = atof(c);
        }
        auto npvAt = [&](double r) {
            double v = 0;
            for (int i = 0; i < N; i++) v += cf[i] / pow(1+r, i);
            return v;
        };
        double r = 0.1;
        for (int it = 0; it < 100; it++) {
            double f = npvAt(r);
            double h = 1e-6;
            double d = (npvAt(r+h) - npvAt(r-h)) / (2*h);
            if (fabs(d) < 1e-12) break;
            double rn = r - f/d;
            if (fabs(rn - r) < 1e-10) { r = rn; break; }
            r = rn;
        }
        show1("IRR", r);
    }

    void annuity() {
        char b[24];
        strcpy(b,"10000"); if (!DM.inputLine("PMT:", b, sizeof(b))) return; double pmt = atof(b);
        strcpy(b,"0.05"); if (!DM.inputLine("Annual rate:", b, sizeof(b))) return; double r = atof(b);
        strcpy(b,"12"); if (!DM.inputLine("Payments/year:", b, sizeof(b))) return; int m = atoi(b);
        strcpy(b,"10"); if (!DM.inputLine("Years:", b, sizeof(b))) return; double y = atof(b);
        if (m < 1) m = 1;
        double per = r/m;
        double n = y*m;
        double fv;
        if (per == 0) fv = pmt * n;
        else fv = pmt * (pow(1+per,n) - 1) / per;
        show1("Annuity FV", fv);
    }

    void show1(const char* t, double v) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans","S:BMP","ESC:back");
        char vs[40]; formatNum(v, vs, sizeof(vs));
        d.setTextColor(C_OK, C_BG); d.setTextSize(2);
        d.drawString(vs, 4, HDR_H+12); d.setTextSize(1);
        G.ans = v; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans",C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("fin");
        }
    }

    void show2(const char* t, const char* l1, double v1, const char* l2, double v2) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans","S:BMP","ESC:back");
        char l[48]; char vs[32];
        d.setTextColor(C_OK,C_BG);
        formatNum(v1,vs,sizeof(vs)); snprintf(l,sizeof(l),"%s = %s",l1,vs); d.drawString(l,4,HDR_H+8);
        formatNum(v2,vs,sizeof(vs)); snprintf(l,sizeof(l),"%s = %s",l2,vs); d.drawString(l,4,HDR_H+24);
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans",C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("fin");
        }
    }

    void show3(const char* t, const char* l1, double v1, const char* l2, double v2, const char* l3, double v3) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans","S:BMP","ESC:back");
        char l[48]; char vs[32];
        d.setTextColor(C_OK,C_BG);
        formatNum(v1,vs,sizeof(vs)); snprintf(l,sizeof(l),"%s = %s",l1,vs); d.drawString(l,4,HDR_H+4);
        formatNum(v2,vs,sizeof(vs)); snprintf(l,sizeof(l),"%s = %s",l2,vs); d.drawString(l,4,HDR_H+18);
        formatNum(v3,vs,sizeof(vs)); snprintf(l,sizeof(l),"%s = %s",l3,vs); d.drawString(l,4,HDR_H+32);
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans",C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("fin");
        }
    }
};

EqSolver     gEqSolver;
NumMenu      gNumMenu;
VecMatCalc   gVecMat;
GeomCalc     gGeom;
ProbSim      gProb;
PrimeFactor  gPrime;
DateCalc     gDate;
FinanceCalc  gFin;
void CalculatorCore::extToolMenu(char* input, int& cur) {
    while (true) {
        const char* items[] = {
            "Equation Solver","Numerical Analysis (d/dx, integ)","Vector / Matrix",
            "Geometry (27 shapes)","Probability Sim","Prime Factorize",
            "Date Calculator","Finance","Back"
        };
        int s = DM.menu("Tools", items, 9);
        if (s < 0 || s == 8) return;
        switch (s) {
            case 0: gEqSolver.run(); break;
            case 1: gNumMenu.run();  break;
            case 2: gVecMat.run();   break;
            case 3: gGeom.run();     break;
            case 4: gProb.run();     break;
            case 5: gPrime.run();    break;
            case 6: gDate.run();     break;
            case 7: gFin.run();      break;
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
                "Quick Stats & Graphs","List Editor","Variable Scale","Descriptive Statistics",
                "Weighted/Geo/Harm/Trim Mean","Frequency Distribution","Z-Scores",
                "CSV Load (SD)","CSV Save (SD)","Clear All Lists","List -> next module",
                "Category Freq","Freq-List Stats","Back"
            };
            int s = DM.menu("Statistics", items, 14);
            if (s < 0 || s == 13) return;
            switch (s) {
                case 0: quickStats();      break;
                case 1: editList();        break;
                case 2: scaleMenu();       break;
                case 3: descriptive();     break;
                case 4: specialMeans();    break;
                case 5: freqDist();        break;
                case 6: zScores();         break;
                case 7: csvLoad();         break;
                case 8: csvSave();         break;
                case 9: clearAll();        break;
                case 10: passToNextHint(); break;
                case 11: categoryFreq();   break;
                case 12: freqListStats();  break;
            }
        }
    }
private:
    int pickList(const char* title) {
        char** labels = (char**)malloc(STAT_LISTS * sizeof(char*));
        if (!labels) return -1;
        for (int i = 0; i < STAT_LISTS; i++) {
            labels[i] = (char*)malloc(20);
            if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return -1; }
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
                if (IS_ESC) return;
                if (st.fn && IS_UP)   { if (sel > 0) sel--; break; }
                if (st.fn && IS_DOWN) { if (sel < n) sel++; break; }
                if (st.enter) {
                    snprintf(buf, sizeof(buf), "%g", (sel < n) ? G.stat[li].data[sel] : 0.0);
                    if (DM.inputLine("Value:", buf, sizeof(buf))) {
                        char* end; double v = strtod(buf, &end);
                        if (end != buf && (isnan(v) || isinf(v))) { DM.notify("NaN/Inf rejected", C_ERR); }
                        else if (end != buf) {
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

    void quickStats() {
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
        auto& d = M5Cardputer.Display;
        auto renderQS = [&](int pg) {
            d.fillScreen(C_BG);
            char th[28]; snprintf(th, sizeof(th), "Quick %s (p%d/2)", G.stat[li].name, pg+1);
            DM.header(th);
            DM.footer("S:save  H/B/C/Q/E/K", "L/R:page G:graphs", "ESC:back");
            d.setTextColor(C_FG, C_BG);
            char l[60];
            int y = HDR_H + 2;
            if (pg == 0) {
                snprintf(l,sizeof(l),"n = %d", n);                    d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"mean = %.6g", mean);            d.setTextColor(C_OK,C_BG); d.drawString(l,4,y); y+=11; d.setTextColor(C_FG,C_BG);
                snprintf(l,sizeof(l),"med  = %.6g", med);             d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"mode = %.6g (x%d)", mode, modeCount); d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"min  = %.6g", mn);              d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"max  = %.6g", mx);              d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"sd   = %.6g", sdS);             d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"var  = %.6g", varS);            d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"range= %.6g", mx-mn);           d.drawString(l,4,y); y+=11;
            } else {
                snprintf(l,sizeof(l),"skew = %.4g", skew);            d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"kurt = %.4g", kurt);            d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"Q1   = %.5g", q1);              d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"Q3   = %.5g", q3);              d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"IQR  = %.5g", iqr);             d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"SE   = %.5g", SE);              d.drawString(l,4,y); y+=11;
                snprintf(l,sizeof(l),"CV   = %.3f%%", CV);            d.drawString(l,4,y); y+=11;
            }
        };
        auto saveAllQS = [&]() {
            unsigned long t = millis();
            for (int pp = 0; pp < 2; pp++) {
                renderQS(pp);
                char path[40]; snprintf(path, sizeof(path), "/quick_%lu_p%d.bmp", t, pp+1);
                DM.saveBMP(path);
            }
            char m[40]; snprintf(m, sizeof(m), "Saved 2 pages");
            DM.notify(m, C_OK, 1200);
        };
        int page = 0;
        while (true) {
            renderQS(page);
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC) return;
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 1) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) { saveAllQS(); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('h') || M5Cardputer.Keyboard.isKeyPressed('H')) { statGraphShow(0); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('b') || M5Cardputer.Keyboard.isKeyPressed('B')) { statGraphShow(1); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('c') || M5Cardputer.Keyboard.isKeyPressed('C')) { statGraphShow(2); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('q') || M5Cardputer.Keyboard.isKeyPressed('Q')) { statGraphShow(3); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('e') || M5Cardputer.Keyboard.isKeyPressed('E')) { statGraphShow(9); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('k') || M5Cardputer.Keyboard.isKeyPressed('K')) { statGraphShow(10); break; }
                if (M5Cardputer.Keyboard.isKeyPressed('g') || M5Cardputer.Keyboard.isKeyPressed('G')) { statGraphMenu(); break; }
            }
        }
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
                if (IS_ESC) return;
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
            if (!tmp) { DM.notify("malloc fail", C_ERR); return; }
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
        if (!hist) { DM.notify("malloc fail", C_ERR); return; }
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
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("hist");
        }
    }

    static int dblCmp(const void* a, const void* b) {
        double x = *(const double*)a, y = *(const double*)b;
        return (x > y) - (x < y);
    }

    struct XF { double x, f; };
    static int xfCmp(const void* a, const void* b) {
        double x = ((const XF*)a)->x, y = ((const XF*)b)->x;
        return (x > y) - (x < y);
    }

    // 度数リスト付き統計: out = {N, mean, sd(標本), sd(母), median, min, max}
    static bool freqStats(const double* x, const double* f, int n, double* out) {
        XF* a = (XF*)malloc(n * sizeof(XF));
        if (!a) return false;
        int m = 0; double N = 0, S = 0;
        for (int i = 0; i < n; i++) {
            if (!(f[i] > 0)) continue;
            a[m].x = x[i]; a[m].f = f[i]; m++;
            N += f[i]; S += f[i] * x[i];
        }
        if (m == 0 || N <= 0) { free(a); return false; }
        double mean = S / N, ss = 0;
        for (int i = 0; i < m; i++) { double d = a[i].x - mean; ss += a[i].f * d * d; }
        qsort(a, m, sizeof(XF), xfCmp);
        double half = N / 2, cum = 0, med = a[m-1].x;
        for (int i = 0; i < m; i++) {
            cum += a[i].f;
            if (cum > half + 1e-12) { med = a[i].x; break; }
            if (fabs(cum - half) <= 1e-12) { med = (a[i].x + a[(i+1 < m) ? i+1 : i].x) / 2; break; }
        }
        out[0] = N; out[1] = mean;
        out[2] = (N > 1) ? sqrt(ss / (N - 1)) : NAN;
        out[3] = sqrt(ss / N);
        out[4] = med; out[5] = a[0].x; out[6] = a[m-1].x;
        free(a);
        return true;
    }

    void categoryFreq() {
        int li = pickList("Category freq: list?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 1) { DM.notify("Empty", C_ERR); return; }
        double* v = (double*)malloc(n * sizeof(double));
        const int cap = 128;
        double* val = (double*)malloc(cap * sizeof(double));
        int* cnt = (int*)malloc(cap * sizeof(int));
        if (!v || !val || !cnt) { free(v); free(val); free(cnt); DM.notify("malloc fail", C_ERR); return; }
        memcpy(v, G.stat[li].data, n * sizeof(double));
        qsort(v, n, sizeof(double), dblCmp);
        int m = 0; bool trunc = false;
        for (int i = 0; i < n; ) {
            int j = i; while (j < n && v[j] == v[i]) j++;
            if (m < cap) { val[m] = v[i]; cnt[m] = j - i; m++; } else trunc = true;
            i = j;
        }
        free(v);
        auto& d = M5Cardputer.Display;
        const int rowH = 11;
        const int rows = (LCD_H - HDR_H - FTR_H) / rowH;
        int top = 0;
        while (true) {
            d.fillScreen(C_BG);
            char th[32]; snprintf(th, sizeof(th), "Categories k=%d%s n=%d", m, trunc ? "+" : "", n);
            DM.header(th); DM.footer("fn+;/.:scroll", "", "ESC:back");
            for (int r = 0; r < rows && top + r < m; r++) {
                char vs[20], l[48];
                formatNum(val[top + r], vs, sizeof(vs));
                snprintf(l, sizeof(l), "%s : %d (%.1f%%)", vs, cnt[top + r], 100.0 * cnt[top + r] / n);
                d.setTextColor((r & 1) ? C_DIM : C_FG, C_BG);
                d.drawString(l, 4, HDR_H + 2 + r * rowH);
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC) { free(val); free(cnt); return; }
                if (st.fn && IS_UP)   { if (top > 0) top--; break; }
                if (st.fn && IS_DOWN) { if (top + rows < m) top++; break; }
            }
        }
    }

    void freqListStats() {
        int xi = pickList("Data list?");      if (xi < 0) return;
        int fi = pickList("Frequency list?"); if (fi < 0) return;
        int n = G.stat[xi].count;
        if (n < 1 || G.stat[fi].count != n) { DM.notify("Mismatch / empty", C_ERR); return; }
        double o[7];
        if (!freqStats(G.stat[xi].data, G.stat[fi].data, n, o)) { DM.notify("No freq>0", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Freq-List Stats"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[44];
        snprintf(l, sizeof(l), "N = %.6g   mean = %.5g", o[0], o[1]);      d.drawString(l, 4, HDR_H + 8);
        snprintf(l, sizeof(l), "s = %.5g   sigma = %.5g", o[2], o[3]);     d.drawString(l, 4, HDR_H + 24);
        snprintf(l, sizeof(l), "median = %.5g", o[4]);                     d.drawString(l, 4, HDR_H + 40);
        snprintf(l, sizeof(l), "min = %.5g   max = %.5g", o[5], o[6]);     d.drawString(l, 4, HDR_H + 56);
        DM.waitEsc();
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
                    const char* cs = tok.c_str(); char* endp; double v = strtod(cs, &endp);
                    if (endp != cs && *endp == 0 && !isnan(v) && !isinf(v)) G.stat[col].data[G.stat[col].count++] = v;
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
            "Stepwise Selection","Predict y from x","Save residuals to L?","Linear: coef CI","Back"
        };
        for (;;) {
        int s = DM.menu("Regression", items, 13);
        if (s < 0 || s == 12) return;
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
            case 11:linearCI();   break;
        }
        }
    }
private:
    int pickList(const char* title) {
        const char* items[STAT_LISTS]; for (int _i = 0; _i < STAT_LISTS; _i++) items[_i] = statLbl(_i);
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

    void linearCI() {
        int xi = pickList("X list?"); if (xi < 0) return;
        int yi = pickList("Y list?"); if (yi < 0) return;
        int n = G.stat[xi].count;
        if (n < 3 || G.stat[yi].count != n) { DM.notify("Mismatch / n<3", C_ERR); return; }
        char b[16] = "0.95";
        if (!DM.inputLine("Confidence:", b, sizeof(b))) return;
        double c = atof(b);
        if (!(c > 0 && c < 1)) { DM.notify("0<c<1", C_ERR); return; }
        double* X = G.stat[xi].data;
        double* Y = G.stat[yi].data;
        double a, bs, r2;
        linfit(X, Y, n, a, bs, r2);
        if (isnan(bs)) { DM.notify("Sxx=0", C_ERR); return; }
        KSum kx; for (int i = 0; i < n; i++) kx.add(X[i]);
        double mx = kx.get() / n;
        KSum kxx, ksse;
        for (int i = 0; i < n; i++) {
            double dx = X[i] - mx, e = Y[i] - (a + bs * X[i]);
            kxx.add(dx*dx); ksse.add(e*e);
        }
        double Sxx = kxx.get(), sse = ksse.get();
        double se = sqrt(sse / (n - 2));
        double seB = se / sqrt(Sxx);
        double seA = se * sqrt(1.0 / n + mx * mx / Sxx);
        double t = Dist::tInv(0.5 + c / 2, n - 2);
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Linear coef CI"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[44];
        snprintf(l, sizeof(l), "b = %.5g  SE %.3g", bs, seB);               d.drawString(l, 4, HDR_H + 6);
        snprintf(l, sizeof(l), "%g%% [%.4g, %.4g]", c*100, bs - t*seB, bs + t*seB); d.drawString(l, 4, HDR_H + 20);
        snprintf(l, sizeof(l), "a = %.5g  SE %.3g", a, seA);                d.drawString(l, 4, HDR_H + 38);
        snprintf(l, sizeof(l), "%g%% [%.4g, %.4g]", c*100, a - t*seA, a + t*seA); d.drawString(l, 4, HDR_H + 52);
        d.setTextColor(C_DIM, C_BG);
        snprintf(l, sizeof(l), "s=%.4g df=%d t=%.3f r2=%.4f", se, n - 2, t, r2); d.drawString(l, 4, HDR_H + 70);
        DM.waitEsc();
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
            if (n < 3) { DM.notify("Med-Med needs n>=3", C_ERR); return; }
            int* idx = (int*)malloc(n * sizeof(int));
            if (!idx) { DM.notify("malloc fail", C_ERR); return; }
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
                if (!tmp) return (double)NAN;
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
            if (fabs(mx3 - mx1) < 1e-12) { bb = 0; aa = (my1 + my2 + my3) / 3.0; }
            else {
                bb = (my3 - my1) / (mx3 - mx1);
                aa = ((my1 - bb*mx1) + (my2 - bb*mx2) + (my3 - bb*mx3)) / 3.0;
            }
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
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = lastA; G.ansComplex = false; DM.notify("a -> Ans", C_OK); return;
            }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("reg");
        }
    }

    void multiple() {
        lastXi = -1;
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
            char th[40]; snprintf(th, sizeof(th), "MultiReg p%d/4", page+1);
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
            } else if (page == 2) {
                d.drawString("VIF (multicoll.):", 4, y); y += 11;
                for (int i = 0; i < k; i++) {
                    snprintf(l, sizeof(l), "VIF X%d = %.3g %s", i+1, VIF[i], VIF[i] > 10 ? "(HIGH!)" : "");
                    if (VIF[i] > 10) d.setTextColor(C_ERR, C_BG);
                    else             d.setTextColor(C_FG, C_BG);
                    d.drawString(l, 4, y); y += 11;
                }
            } else {
                double tcrit = Dist::tInv(0.975, n - p);
                d.setTextColor(C_ACC, C_BG);
                d.drawString("95% CI for coefficients:", 4, y); y += 11;
                d.setTextColor(C_FG, C_BG);
                for (int i = 0; i < p; i++) {
                    double lo = beta[i] - tcrit*SE[i];
                    double hi = beta[i] + tcrit*SE[i];
                    snprintf(l, sizeof(l), "b%d: %.3g [%.3g, %.3g]", i, beta[i], lo, hi);
                    d.drawString(l, 4, y); y += 11;
                }
                snprintf(l, sizeof(l), "t* = %.3f  df = %d", tcrit, n - p);
                d.setTextColor(C_DIM, C_BG);
                d.drawString(l, 4, y);
            }
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC) return;
                if (st.fn && IS_LEFT)  { if (page > 0) { page--; break; } }
                if (st.fn && IS_RIGHT) { if (page < 3) { page++; break; } }
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("mreg");
            }
        }
    }

    void stepwise() {
        lastXi = -1;
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
        for (;;) {
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
    }
private:
    int pickList(const char* title) {
        const char* items[STAT_LISTS]; for (int _i = 0; _i < STAT_LISTS; _i++) items[_i] = statLbl(_i);
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
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("p->Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("test");
        }
    }

    void zTest1() {
        int li = pickList("Sample list?"); if (li < 0) return;
        char b[16]="0"; if (!DM.inputLine("mu0:", b, sizeof(b))) return;
        double mu0 = atof(b);
        strcpy(b, "1"); if (!DM.inputLine("sigma (known):", b, sizeof(b))) return;
        double sd = atof(b); if (isnan(mu0) || isinf(mu0) || !(sd > 0)) { DM.notify("sd>0 required", C_ERR); return; }
        int side = pickSide(); if (side < 0) return;
        int n = G.stat[li].count;
        if (n < 1) { DM.notify("n<1", C_ERR); return; }
        if (sd < 1e-15) { DM.notify("sigma > 0", C_ERR); return; }
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
        if (sd < 1e-15) { DM.notify("Zero variance", C_ERR); return; }
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
            if (sp < 1e-15) { DM.notify("Zero variance both groups", C_ERR); return; }
            t = (ma - mb) / (sp * sqrt(1.0/na + 1.0/nb));
            df = na + nb - 2;
        } else {
            double seA = va/na, seB = vb/nb;
            if (seA + seB < 1e-15) { DM.notify("Zero variance both groups", C_ERR); return; }
            t = (ma - mb) / sqrt(seA + seB);
            double num = (seA + seB) * (seA + seB);
            double den = (seA*seA)/(na-1) + (seB*seB)/(nb-1);
            if (den < 1e-300) df = na + nb - 2;
            else { double q = num/den + 0.5; if (q > 1e6) q = 1e6; df = (int)q; }
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
        if (v < 1e-15) { DM.notify("Zero variance (all diffs equal)", C_ERR); return; }
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
        if (vb < 1e-15) { DM.notify("Group B has zero variance", C_ERR); return; }
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
        if (SSW < 1e-12) { DM.notify("All groups identical", C_ERR); return; }
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
            if (!found) { if (nA < 20) Alev[nA++] = a; else { DM.notify(">20 A levels", C_ERR); return; } }
            found = false;
            for (int k = 0; k < nB; k++) if (Blev[k] == b) { found = true; break; }
            if (!found) { if (nB < 20) Blev[nB++] = b; else { DM.notify(">20 B levels", C_ERR); return; } }
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
        if (dfE <= 0) { DM.notify("n too small for levels", C_ERR); return; }
        if (SSE < 1e-12) { DM.notify("SSE=0 (perfect fit)", C_ERR); return; }
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
        if (a < 0 || bb < 0 || cc < 0 || dd < 0) { DM.notify("Negative cell", C_ERR); return; }
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
            "Sign test","Spearman correlation","Kendall tau","Back"
        };
        for (;;) {
        int s = DM.menu("Non-parametric", items, 8);
        if (s < 0 || s == 7) return;
        switch (s) {
            case 0: mannWhitney();    break;
            case 1: wilcoxon();       break;
            case 2: kruskalWallis();  break;
            case 3: friedman();       break;
            case 4: signTest();       break;
            case 5: spearman();       break;
            case 6: kendallTau();     break;
        }
        }
    }

    // Medical Calculator から呼ばれるリスク・オッズ系
    void medicalRiskMenu() {
        const char* items[] = {
            "Odds Ratio + 95% CI","Risk Ratio + 95% CI",
            "Risk Difference + 95% CI","NNT (Number Needed to Treat)","Back"
        };
        for (;;) {
        int s = DM.menu("Risk / Odds (2x2)", items, 5);
        if (s < 0 || s == 4) return;
        switch (s) {
            case 0: oddsRatio();  break;
            case 1: riskRatio();  break;
            case 2: riskDiff();   break;
            case 3: nnt();        break;
        }
        }
    }

    // Medical Calculator から呼ばれる信頼区間
    void medicalCIMenu() {
        const char* items[] = {
            "Confidence Interval (mean)","CI (proportion)","Bootstrap CI (mean/median)","Back"
        };
        for (;;) {
        int s = DM.menu("Confidence Intervals", items, 4);
        if (s < 0 || s == 3) return;
        switch (s) {
            case 0: ciMean(); break;
            case 1: ciProp(); break;
            case 2: ciBootstrap(); break;
        }
        }
    }
private:
    int pickList(const char* title) {
        const char* items[STAT_LISTS]; for (int _i = 0; _i < STAT_LISTS; _i++) items[_i] = statLbl(_i);
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
            if (IS_ESC) return;
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
        if (!v) { DM.notify("malloc fail", C_ERR); return; }
        int*    g = (int*)   malloc(N * sizeof(int));
        if (!g) { free(v); DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < na; i++) { v[i] = G.stat[la].data[i]; g[i] = 0; }
        for (int i = 0; i < nb; i++) { v[na+i] = G.stat[lb].data[i]; g[na+i] = 1; }
        for (int i = 1; i < N; i++) {
            double k = v[i]; int gk = g[i]; int j = i - 1;
            while (j >= 0 && v[j] > k) { v[j+1]=v[j]; g[j+1]=g[j]; j--; }
            v[j+1] = k; g[j+1] = gk;
        }
        double* rk = (double*)malloc(N * sizeof(double));
        if (!rk) { free(v); free(g); DM.notify("malloc fail", C_ERR); return; }
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
        if (!d) { DM.notify("malloc fail", C_ERR); return; }
        int m = 0;
        for (int i = 0; i < n; i++) {
            double t = G.stat[la].data[i] - G.stat[lb].data[i];
            if (t != 0) d[m++] = t;
        }
        if (m < 2) { free(d); DM.notify("All ties", C_ERR); return; }
        double* ad = (double*)malloc(m * sizeof(double));
        if (!ad) { free(d); DM.notify("malloc fail", C_ERR); return; }
        int*    sg = (int*)   malloc(m * sizeof(int));
        if (!sg) { free(d); free(ad); DM.notify("malloc fail", C_ERR); return; }
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
        if (!v) { DM.notify("malloc fail", C_ERR); return; }
        int*    grp = (int*) malloc(total * sizeof(int));
        if (!grp) { free(v); DM.notify("malloc fail", C_ERR); return; }
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
        if (!rk) { free(v); free(grp); DM.notify("malloc fail", C_ERR); return; }
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
        if (!xr) return;
        double* yr = rankList(G.stat[lb].data, n);
        if (!yr) { free(xr); return; }
        double sx=0, sy=0, sxx=0, syy=0, sxy=0;
        for (int i = 0; i < n; i++) {
            sx += xr[i]; sy += yr[i];
            sxx += xr[i]*xr[i]; syy += yr[i]*yr[i]; sxy += xr[i]*yr[i];
        }
        double denom_rho = sqrt((n*sxx - sx*sx)*(n*syy - sy*sy)); if (denom_rho < 1e-12) { DM.notify("Spearman: zero variance", C_ERR); free(xr); free(yr); return; } double rho = (n*sxy - sx*sy) / denom_rho;
        double t = rho * sqrt((double)(n-2)/(1-rho*rho));
        double p = 2.0 * (1.0 - Dist::tCDF(fabs(t), n-2));
        free(xr); free(yr);
        char extra[32]; snprintf(extra, sizeof(extra), "rho=%.4f", rho);
        showTest("Spearman", t, n-2, p, extra);
    }

    double* rankList(double* a, int n) {
        double* tmp = (double*)malloc(n*sizeof(double));
        if (!tmp) { DM.notify("malloc fail", C_ERR); return nullptr; }
        int*    idx = (int*)   malloc(n*sizeof(int));
        if (!idx) { free(tmp); DM.notify("malloc fail", C_ERR); return nullptr; }
        for (int i = 0; i < n; i++) { tmp[i] = a[i]; idx[i] = i; }
        for (int i = 1; i < n; i++) {
            double k = tmp[i]; int ki = idx[i]; int j = i-1;
            while (j>=0 && tmp[j]>k) { tmp[j+1]=tmp[j]; idx[j+1]=idx[j]; j--; }
            tmp[j+1]=k; idx[j+1]=ki;
        }
        double* rk = (double*)malloc(n*sizeof(double));
        if (!rk) { free(tmp); free(idx); DM.notify("malloc fail", C_ERR); return nullptr; }
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
        double denom_tau = sqrt((double)(N - tieX)*(N - tieY)); if (denom_tau < 1e-12) { DM.notify("Kendall: all ties", C_ERR); return; } double tau = (double)(conc - disc) / denom_tau;
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
            if (IS_ESC) return;
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

    static int bsCmp(const void* a, const void* b) {
        double x = *(const double*)a, y = *(const double*)b;
        return (x > y) - (x < y);
    }

    // 昇順ソート済み配列の分位点 (線形補間)
    static double bsQuantile(const double* s, int B, double q) {
        double pos = q * (B - 1);
        int i = (int)floor(pos);
        if (i < 0) return s[0];
        if (i >= B - 1) return s[B - 1];
        return s[i] + (pos - i) * (s[i+1] - s[i]);
    }

    static double bsMedian(double* t, int n) {
        qsort(t, n, sizeof(double), bsCmp);
        return (n & 1) ? t[n/2] : (t[n/2 - 1] + t[n/2]) / 2;
    }

    void ciBootstrap() {
        int li = pickList("Bootstrap: list?"); if (li < 0) return;
        int n = G.stat[li].count;
        if (n < 2) { DM.notify("n<2", C_ERR); return; }
        const char* sitems[] = {"Mean", "Median"};
        int sel = DM.menu("Statistic", sitems, 2);
        if (sel < 0) return;
        const char* mitems[] = {"Percentile", "BCa"};
        int method = DM.menu("Method", mitems, 2);
        if (method < 0) return;
        char b[16] = "0.95";
        if (!DM.inputLine("Confidence:", b, sizeof(b))) return;
        double c = atof(b);
        if (!(c > 0 && c < 1)) { DM.notify("0<c<1", C_ERR); return; }
        strcpy(b, "1000");
        if (!DM.inputLine("Resamples 100-5000:", b, sizeof(b))) return;
        int B = atoi(b);
        if (B < 100 || B > 5000) { DM.notify("100..5000", C_ERR); return; }
        double* est = (double*)malloc(B * sizeof(double));
        double* tmp = (double*)malloc(n * sizeof(double));
        if (!est || !tmp) { free(est); free(tmp); DM.notify("malloc fail", C_ERR); return; }
        const double* X = G.stat[li].data;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Bootstrap CI");
        d.setTextColor(C_DIM, C_BG); d.drawString("Resampling...", 4, HDR_H + 8);
        double point;
        if (sel == 0) { double s0 = 0; for (int i = 0; i < n; i++) s0 += X[i]; point = s0 / n; }
        else { memcpy(tmp, X, n * sizeof(double)); point = bsMedian(tmp, n); }
        for (int k = 0; k < B; k++) {
            double sum = 0;
            for (int i = 0; i < n; i++) { tmp[i] = X[esp_random() % (uint32_t)n]; sum += tmp[i]; }
            est[k] = (sel == 0) ? sum / n : bsMedian(tmp, n);
        }
        double em = 0; for (int k = 0; k < B; k++) em += est[k];
        em /= B;
        double ev = 0; for (int k = 0; k < B; k++) ev += (est[k] - em) * (est[k] - em);
        double sdBoot = sqrt(ev / (B - 1));
        double lo, hi;
        if (method == 0) {
            qsort(est, B, sizeof(double), bsCmp);
            lo = bsQuantile(est, B, (1 - c) / 2);
            hi = bsQuantile(est, B, 1 - (1 - c) / 2);
        } else {
            int below = 0;
            for (int k = 0; k < B; k++) if (est[k] < point) below++;
            double z0 = (below > 0 && below < B) ? Dist::normalInv((double)below / B) : 0;
            double* jack = (double*)malloc(n * sizeof(double));
            if (!jack) { free(est); free(tmp); DM.notify("malloc fail", C_ERR); return; }
            double jackMean = 0;
            for (int i = 0; i < n; i++) {
                int m = 0;
                for (int j = 0; j < n; j++) if (j != i) tmp[m++] = X[j];
                if (sel == 0) { double s = 0; for (int k = 0; k < m; k++) s += tmp[k]; jack[i] = s / m; }
                else          { jack[i] = bsMedian(tmp, m); }
                jackMean += jack[i];
            }
            jackMean /= n;
            double num = 0, den = 0;
            for (int i = 0; i < n; i++) {
                double d = jackMean - jack[i];
                num += d*d*d;
                den += d*d;
            }
            free(jack);
            double a = (den > 1e-12) ? num / (6.0 * pow(den, 1.5)) : 0;
            double za_lo = Dist::normalInv((1-c)/2);
            double za_hi = Dist::normalInv(1-(1-c)/2);
            double t1 = z0 + (z0 + za_lo) / (1 - a * (z0 + za_lo));
            double t2 = z0 + (z0 + za_hi) / (1 - a * (z0 + za_hi));
            double q_lo = Dist::normalCDF(t1);
            double q_hi = Dist::normalCDF(t2);
            if (q_lo < 0.001) q_lo = 0.001;
            if (q_hi > 0.999) q_hi = 0.999;
            if (q_lo >= q_hi) { q_lo = (1-c)/2; q_hi = 1-(1-c)/2; }
            qsort(est, B, sizeof(double), bsCmp);
            lo = bsQuantile(est, B, q_lo);
            hi = bsQuantile(est, B, q_hi);
        }
        free(est); free(tmp);
        d.fillScreen(C_BG); DM.header("Bootstrap CI"); DM.footer("","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[44];
        snprintf(l, sizeof(l), "%s = %.5g", sel == 0 ? "mean" : "median", point); d.drawString(l, 4, HDR_H + 8);
        snprintf(l, sizeof(l), "%g%% CI = [%.4g, %.4g]", c*100, lo, hi);           d.drawString(l, 4, HDR_H + 24);
        d.setTextColor(C_DIM, C_BG);
        snprintf(l, sizeof(l), "boot SE = %.4g  B = %d  n = %d", sdBoot, B, n);     d.drawString(l, 4, HDR_H + 40);
        d.drawString(method == 0 ? "percentile method" : "BCa method", 4, HDR_H + 54);
        DM.waitEsc();
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
        if (x < 0 || n <= 0 || x > n) { DM.notify("need 0<=x<=n", C_ERR); return; }
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
        for (;;) {
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
            if (IS_ESC) return;
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
        strcpy(b, "1"); if (!DM.inputLine("sigma:", b, sizeof(b))) return; double sd = atof(b); if (isnan(mu) || isinf(mu) || !(sd > 0)) { DM.notify("sd>0 required", C_ERR); return; }
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
        if (df < 1) { DM.notify("df>=1", C_ERR); return; }
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
        if (df < 1) { DM.notify("df>=1", C_ERR); return; }
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
        if (d1 < 1 || d2 < 1) { DM.notify("d1,d2>=1", C_ERR); return; }
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
        if (n < 0) { DM.notify("n>=0", C_ERR); return; }
        strcpy(b, "0.5"); if (!DM.inputLine("p:", b, sizeof(b))) return; double p = atof(b);
        if (p <= 0 || p >= 1) { DM.notify("0<p<1", C_ERR); return; }
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
        if (l <= 0) { DM.notify("lambda>0", C_ERR); return; }
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
        if (p <= 0 || p > 1) { DM.notify("0<p<=1", C_ERR); return; }
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
        for (;;) {
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
    }
private:
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
                if (IS_ESC) { free(X); return; }
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
        if (n < p + 1) { DM.notify("n must be > p", C_ERR); return; }
        char b[8] = "2";
        if (!DM.inputLine("# factors:", b, sizeof(b))) return;
        int k = atoi(b);
        if (k < 1 || k > p) { DM.notify("Invalid k", C_ERR); return; }
        double* X = (double*)malloc(n*p*sizeof(double));
        if (!X) { DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        standardize(X, n, p);
        double R[FA_MAX_VAR*FA_MAX_VAR];
        corMatrix(X, n, p, R);
        double eig[FA_MAX_VAR], V[FA_MAX_VAR*FA_MAX_VAR];
        Mat::jacobiEig(R, p, eig, V);
        double* L = (double*)malloc(p*k*sizeof(double));
        if (!L) { free(X); DM.notify("malloc fail", C_ERR); return; }
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
        if (!X) { DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        double* cent = (double*)malloc(k*p*sizeof(double));
        if (!cent) { free(X); DM.notify("malloc fail", C_ERR); return; }
        int* assign = (int*)malloc(n*sizeof(int));
        if (!assign) { free(X); free(cent); DM.notify("malloc fail", C_ERR); return; }
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
            if (!cnt) break;
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
        if (!X) { DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < n; i++)
            for (int j = 0; j < p; j++) X[i*p+j] = G.stat[lst[j]].data[i];
        int* clust = (int*)malloc(n*sizeof(int));
        if (!clust) { free(X); DM.notify("malloc fail", C_ERR); return; }
        int* size  = (int*)calloc(n, sizeof(int));
        if (!size) { free(X); free(clust); DM.notify("malloc fail", C_ERR); return; }
        double* centroid = (double*)malloc(n*p*sizeof(double));
        if (!centroid) { free(X); free(clust); free(size); DM.notify("malloc fail", C_ERR); return; }
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
        if (!tree) { DM.notify("malloc fail", C_ERR); return; }
        int treeN = 0;
        int* idxArr = (int*)malloc(n*sizeof(int));
        if (!idxArr) { free(tree); DM.notify("malloc fail", C_ERR); return; }
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
        if (!leftIdx) return -1;
        int* rightIdx = (int*)malloc(nIdx*sizeof(int));
        if (!rightIdx) { free(leftIdx); return -1; }
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
            "Cochran's Q (heterogeneity)","Breslow-Day test","Back"
        };
        for (;;) {
        int s = DM.menu("Stratified", items, 5);
        if (s < 0 || s == 4) return;
        switch (s) {
            case 0: mhOR();          break;
            case 1: mhRR();          break;
            case 2: cochranQ();      break;
            case 3: breslowDay();    break;
        }
        }
    }

    // Medical Calculator から呼ばれる感染症の要因探索
    void medicalOutbreak() { outbreakTrace(); }
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
        if (den < 1e-12) { mhLO = 0; mhHI = 0; chiMH = 0; pMH = 1; return 0; }
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
        if (varA < 1e-12) { chiMH = 0; pMH = 1; return OR; }
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
        if (den < 1e-12) { mhLO = 0; mhHI = 0; return 0; }
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
        double den2 = sumNum * sumDen;
        if (fabs(den2) < 1e-12 || V <= 0) { mhLO = RR; mhHI = RR; return RR; }
        double var = V / den2;
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
        if (varA < 1e-12) { chiMH = 0; pMH = 1; }
        else { chiMH = (diff*diff) / varA; pMH = 1.0 - Dist::chiSquareCDF(chiMH, 1); }
        computeBreslowDay(commonVal, X2_BD, p_BD);
        showStratifiedResult("Mantel-Haenszel RR", "RR", chiMH, pMH, X2_BD, p_BD);
    }

    void outbreakTrace() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        DM.header("Outbreak / Source Trace");
        DM.footer("","","ESC:next");
        d.setTextColor(C_FG, C_BG);
        d.drawString("Setup:", 4, HDR_H + 4);
        d.setTextColor(C_DIM, C_BG);
        d.drawString("Exposure lists: 0/1 binary", 4, HDR_H + 16);
        d.drawString("Outcome list: select next", 4, HDR_H + 28);
        d.drawString("Same length required", 4, HDR_H + 40);
        d.setTextColor(C_FG, C_BG);
        d.drawString("Press any key...", 4, HDR_H + 56);
        DM.waitEsc();
        const char* items[STAT_LISTS]; for (int _i = 0; _i < STAT_LISTS; _i++) items[_i] = statLbl(_i);
        int outL = DM.menu("Outcome list?", items, STAT_LISTS);
        if (outL < 0) return;
        int N = G.stat[outL].count;
        if (N < 4) { DM.notify("Outcome too small", C_ERR); return; }
        // --- Binary (0/1) validation: Outcome ---
        for (int i = 0; i < N; i++) {
            double v = G.stat[outL].data[i];
            if (v != 0.0 && v != 1.0) {
                DM.notify("Outcome not 0/1", C_ERR, 1500);
                return;
            }
        }
        // --- Binary (0/1) validation: Exposures ---
        for (int li = 0; li < STAT_LISTS; li++) {
            if (li == outL) continue;
            if (G.stat[li].count != N) continue;
            for (int i = 0; i < N; i++) {
                double v = G.stat[li].data[i];
                if (v != 0.0 && v != 1.0) {
                    char m[40]; snprintf(m, sizeof(m), "L%d not 0/1", li+1);
                    DM.notify(m, C_ERR, 1500);
                    return;
                }
            }
        }
        // Fisher's exact test 2x2 (two-sided) via lgamma
        auto fisher2x2 = [](int a, int b, int c, int d) -> double {
            int n = a + b + c + d;
            if (n == 0) return 1.0;
            int r1 = a + b, r2 = c + d;
            int c1 = a + c;
            auto lf = [](int x) { return lgamma((double)(x + 1)); };
            double base = lf(r1) + lf(r2) + lf(c1) + lf(b + d) - lf(n);
            int kmin = r1 + c1 - n; if (kmin < 0) kmin = 0;
            int kmax = (r1 < c1) ? r1 : c1;
            auto lp = [&](int k) {
                return base - lf(k) - lf(r1 - k) - lf(c1 - k) - lf(r2 - (c1 - k));
            };
            double lpo = lp(a);
            double p = 0;
            for (int k = kmin; k <= kmax; k++) {
                double t = lp(k);
                if (t <= lpo + 1e-9) p += exp(t);
            }
            if (p > 1.0) p = 1.0;
            return p;
        };
        struct OutcomeResult {
            int li;
            double RR, RRlo, RRhi;
            double OR, ORlo, ORhi;
            int a, b, c, dCount;
            double pVal;
            int useFisher;
        };
        OutcomeResult res[STAT_LISTS];
        int nFac = 0;
        for (int li = 0; li < STAT_LISTS; li++) {
            if (li == outL) continue;
            if (G.stat[li].count != N) continue;
            int aa = 0, bb = 0, cc = 0, dd = 0;
            for (int i = 0; i < N; i++) {
                bool exp_ = (G.stat[li].data[i] != 0);
                bool ill  = (G.stat[outL].data[i] != 0);
                if (exp_ && ill)       aa++;
                else if (exp_ && !ill) bb++;
                else if (!exp_ && ill) cc++;
                else                   dd++;
            }
            if (aa + bb == 0 || cc + dd == 0) continue;
            double a2 = aa, b2 = bb, c2 = cc, d2 = dd;
            if (a2==0 || b2==0 || c2==0 || d2==0) { a2+=0.5; b2+=0.5; c2+=0.5; d2+=0.5; }
            double r1 = a2/(a2+b2), r2 = c2/(c2+d2);
            double RR = (r2 > 0) ? r1/r2 : 0;
            double seL_RR = sqrt(b2/(a2*(a2+b2)) + d2/(c2*(c2+d2)));
            double lnRR = (RR > 0) ? log(RR) : 0;
            double OR = (b2*c2 > 0) ? (a2*d2)/(b2*c2) : 0;
            double seL_OR = sqrt(1.0/a2 + 1.0/b2 + 1.0/c2 + 1.0/d2);
            double lnOR = (OR > 0) ? log(OR) : 0;
            double n_total = aa + bb + cc + dd;
            // Expected counts & auto Fisher switch
            int useFisher = 0;
            double chi = 0, pp = 1.0;
            if (n_total > 0) {
                double E11 = (double)(aa+bb)*(aa+cc)/n_total;
                double E12 = (double)(aa+bb)*(bb+dd)/n_total;
                double E21 = (double)(cc+dd)*(aa+cc)/n_total;
                double E22 = (double)(cc+dd)*(bb+dd)/n_total;
                if (E11<5 || E12<5 || E21<5 || E22<5) useFisher = 1;
                if (E11>0) chi += pow(fabs(aa-E11)-0.5, 2)/E11;
                if (E12>0) chi += pow(fabs(bb-E12)-0.5, 2)/E12;
                if (E21>0) chi += pow(fabs(cc-E21)-0.5, 2)/E21;
                if (E22>0) chi += pow(fabs(dd-E22)-0.5, 2)/E22;
            }
            pp = useFisher ? fisher2x2(aa, bb, cc, dd)
                           : 1.0 - Dist::chiSquareCDF(chi, 1);
            res[nFac].li        = li;
            res[nFac].RR        = RR;
            res[nFac].RRlo      = (RR > 0) ? exp(lnRR-1.96*seL_RR) : 0;
            res[nFac].RRhi      = (RR > 0) ? exp(lnRR+1.96*seL_RR) : 0;
            res[nFac].OR        = OR;
            res[nFac].ORlo      = (OR > 0) ? exp(lnOR-1.96*seL_OR) : 0;
            res[nFac].ORhi      = (OR > 0) ? exp(lnOR+1.96*seL_OR) : 0;
            res[nFac].a         = aa;
            res[nFac].b         = bb;
            res[nFac].c         = cc;
            res[nFac].dCount    = dd;
            res[nFac].pVal      = pp;
            res[nFac].useFisher = useFisher;
            nFac++;
        }
        if (nFac == 0) { DM.notify("No valid exposures", C_ERR); return; }
        if (nFac > 10) nFac = 10;
        for (int i = 0; i < nFac-1; i++)
            for (int j = i+1; j < nFac; j++)
                if (res[j].RR > res[i].RR) { OutcomeResult t = res[i]; res[i] = res[j]; res[j] = t; }
        int useOR = 0;
        auto redraw = [&]() {
            d.fillScreen(C_BG);
            char hd[40]; snprintf(hd, sizeof(hd), "Outbreak %s (Y=L%d)",
                                  useOR ? "OR" : "RR", outL+1);
            DM.header(hd);
            DM.footer("F:Forest  R:RR/OR", "Y:Top->Ans", "ESC:back");
            d.setTextColor(C_DIM, C_BG);
            d.drawString(useOR ? "Exp   OR    [95%CI]   p" : "Exp   RR    [95%CI]   p",
                         4, HDR_H + 2);
            for (int i = 0; i < nFac && HDR_H + 14 + i*11 < LCD_H - FTR_H - 4; i++) {
                char l[64];
                uint16_t col;
                double v  = useOR ? res[i].OR  : res[i].RR;
                double lo = useOR ? res[i].ORlo : res[i].RRlo;
                double hi = useOR ? res[i].ORhi : res[i].RRhi;
                if (v > 2.0 && res[i].pVal < 0.05) col = C_ERR;
                else if (v > 1.5)                  col = C_ACC2;
                else                               col = C_FG;
                d.setTextColor(col, C_BG);
                char pstr[12];
                if (res[i].useFisher) snprintf(pstr, sizeof(pstr), "%.3fF", res[i].pVal);
                else                  snprintf(pstr, sizeof(pstr), "%.3f",  res[i].pVal);
                snprintf(l, sizeof(l), "L%d %5.2f [%4.2f,%4.2f] %s",
                         res[i].li+1, v, lo, hi, pstr);
                d.drawString(l, 4, HDR_H + 14 + i*11);
            }
            d.setTextColor(C_ERR, C_BG);
            d.drawString(useOR ? "Red: OR>2 & p<0.05  F=Fisher"
                               : "Red: RR>2 & p<0.05  F=Fisher",
                         4, LCD_H - FTR_H - 14);
        };
        redraw();
        K_ = nFac;
        for (int i = 0; i < nFac; i++) snprintf(strataLabel[i], 16, "L%d", res[i].li+1);
        commonVal = 1.0; commonLO = 1.0; commonHI = 1.0;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y') || M5Cardputer.Keyboard.isKeyPressed('Y')) {
                G.ans = useOR ? res[0].OR : res[0].RR;
                DM.notify(useOR ? "Top OR -> Ans" : "Top RR -> Ans", C_OK);
                return;
            }
            if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) {
                useOR = !useOR;
                redraw();
            }
            if (M5Cardputer.Keyboard.isKeyPressed('f') || M5Cardputer.Keyboard.isKeyPressed('F')) {
                if (useOR) {
                    for (int i = 0; i < nFac; i++) {
                        sOR_[i] = res[i].OR; sLO_[i] = res[i].ORlo; sHI_[i] = res[i].ORhi;
                    }
                    plotKind = 0;
                    drawForestPlot("Outbreak (OR)", false);
                } else {
                    for (int i = 0; i < nFac; i++) {
                        sOR_[i] = res[i].RR; sLO_[i] = res[i].RRlo; sHI_[i] = res[i].RRhi;
                    }
                    plotKind = 1;
                    drawForestPlot("Outbreak (RR)", false);
                }
                redraw();
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
                if (IS_ESC) return;
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
        if (logMin > 1e30 || logMax < -1e30) { DM.notify("No valid data", C_ERR); return; }
        if (0 < logMin) logMin = 0;
        if (0 > logMax) logMax = 0;
        double margin = (logMax - logMin) * 0.1;
        if (margin < 0.1) margin = 0.1;
        logMin -= margin; logMax += margin;
        int totalRows = K_ + (showCommon ? 1 : 0);
        const int rowsPerPage = 8;
        int pages = (totalRows + rowsPerPage - 1) / rowsPerPage;
        if (pages < 1) pages = 1;
        int page = 0;
        int xLab = 4;
        int xPlot0 = 36;
        int xPlot1 = LCD_W - 4;
        int gw = xPlot1 - xPlot0;
        int yTop = HDR_H + 2;
        int yBot = LCD_H - FTR_H - 14;
        int gh = yBot - yTop;
        int rh = gh / (rowsPerPage + 1);
        if (rh < 10) rh = 10;
        if (rh > 14) rh = 14;
        int xRef = xPlot0 + (int)((0 - logMin)/(logMax - logMin) * gw);
        auto renderPage = [&](int p) {
            d.fillScreen(C_BG);
            char pgStr[20]; snprintf(pgStr, sizeof(pgStr), "%d-%d/%d",
                p*rowsPerPage + 1,
                (p*rowsPerPage + rowsPerPage < totalRows) ? p*rowsPerPage + rowsPerPage : totalRows,
                totalRows);
            DM.header(title);
            DM.footer("S:save all", pgStr, "ESC:back");
            for (int yy = yTop; yy < yBot; yy += 4) {
                d.drawPixel(xRef, yy,     C_DIM);
                d.drawPixel(xRef, yy + 1, C_DIM);
            }
            int startRow = p * rowsPerPage;
            int endRow = startRow + rowsPerPage;
            if (endRow > totalRows) endRow = totalRows;
            for (int r = startRow; r < endRow; r++) {
                int y = yTop + (r - startRow + 1) * rh - rh/2;
                if (r < K_) {
                    d.setTextColor(C_FG, C_BG);
                    d.drawString(strataLabel[r], xLab, y - 3);
                    if (sLO_[r] > 0 && sHI_[r] > 0) {
                        int xL = xPlot0 + (int)((log(sLO_[r]) - logMin)/(logMax - logMin) * gw);
                        int xH = xPlot0 + (int)((log(sHI_[r]) - logMin)/(logMax - logMin) * gw);
                        if (xL < xPlot0) xL = xPlot0;
                        if (xH > xPlot1) xH = xPlot1;
                        d.drawLine(xL, y, xH, y, C_ACC);
                        d.drawLine(xL, y - 2, xL, y + 2, C_ACC);
                        d.drawLine(xH, y - 2, xH, y + 2, C_ACC);
                        int xC = xPlot0 + (int)((log(sOR_[r]) - logMin)/(logMax - logMin) * gw);
                        d.fillRect(xC - 2, y - 2, 5, 5, C_OK);
                    }
                } else if (showCommon && r == K_) {
                    d.setTextColor(C_ACC2, C_BG);
                    d.drawString("Pool", xLab, y - 3);
                    if (commonLO > 0 && commonHI > 0) {
                        int xL = xPlot0 + (int)((log(commonLO) - logMin)/(logMax - logMin) * gw);
                        int xH = xPlot0 + (int)((log(commonHI) - logMin)/(logMax - logMin) * gw);
                        int xC = xPlot0 + (int)((log(commonVal) - logMin)/(logMax - logMin) * gw);
                        d.fillTriangle(xL, y, xC, y - 4, xC, y, C_ACC2);
                        d.fillTriangle(xH, y, xC, y - 4, xC, y, C_ACC2);
                        d.fillTriangle(xL, y, xC, y + 4, xC, y, C_ACC2);
                        d.fillTriangle(xH, y, xC, y + 4, xC, y, C_ACC2);
                    }
                }
            }
            d.setTextColor(C_DIM, C_BG);
            char ll[16];
            snprintf(ll, sizeof(ll), "%.2g", exp(logMin));
            d.drawString(ll, xPlot0 - 12, yBot + 1);
            d.drawString("1", xRef - 2, yBot + 1);
            snprintf(ll, sizeof(ll), "%.2g", exp(logMax));
            d.drawString(ll, xPlot1 - 18, yBot + 1);
        };
        auto saveAllPages = [&]() {
            unsigned long t = millis();
            for (int pp = 0; pp < pages; pp++) {
                renderPage(pp);
                char path[40]; snprintf(path, sizeof(path), "/forest_%lu_p%d.bmp", t, pp+1);
                DM.saveBMP(path);
            }
            renderPage(page);
            char m[40]; snprintf(m, sizeof(m), "Saved %d pages", pages);
            DM.notify(m, C_OK, 1500);
        };
        while (true) {
            renderPage(page);
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC) return;
                if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) {
                    saveAllPages();
                    break;
                }
                if (IS_DOWN) { if (page < pages-1) { page++; break; } }
                if (IS_UP)   { if (page > 0) { page--; break; } }
            }
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
        for (;;) {
        int s = DM.menu("Survival", items, 5);
        if (s < 0 || s == 4) return;
        switch (s) {
            case 0: kaplanMeier(); break;
            case 1: logRank();     break;
            case 2: coxPH();       break;
            case 3: medianSurv();  break;
        }
        }
    }
private:
    void kaplanMeier() {
        int n = G.stat[0].count;
        if (n < 2 || G.stat[1].count != n) { DM.notify("L1=time L2=event(0/1)", C_ERR); return; }
        double* T = (double*)malloc(n*sizeof(double));
        if (!T) { DM.notify("malloc fail", C_ERR); return; }
        int*    E = (int*)   malloc(n*sizeof(int));
        if (!E) { free(T); DM.notify("malloc fail", C_ERR); return; }
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
        if (tMax < 1e-12) tMax = 1;
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
            if (IS_ESC) return;
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
        if (V < 1e-15) { DM.notify("Undefined: no variance", C_WARN); return; }
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
        if (SE < 1e-15) { DM.notify("Undefined: no variance", C_WARN); return; }
        if (!isfinite(beta) || fabs(beta) > 50) { DM.notify("Cox: beta diverged", C_ERR); return; }
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
        if (!T) { DM.notify("malloc fail", C_ERR); return; }
        int*    E = (int*)   malloc(n*sizeof(int));
        if (!E) { free(T); DM.notify("malloc fail", C_ERR); return; }
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
        for (;;) {
        int s = DM.menu("Meta-Analysis", items, 6);
        if (s < 0 || s == 5) return;
        int K = inputData();
        if (K < 2) continue;
        switch (s) {
            case 0: invVar(K); break;
            case 1: dlRE(K);   break;
            case 2: forest(K); break;
            case 3: funnel(K); break;
            case 4: egger(K);  break;
        }
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
        if (fabs(mx - mn) < 1e-12) mx = mn + 1;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Forest Plot"); DM.footer("S:BMP","","ESC:back");
        int x0 = 30, x1 = LCD_W - 6, gw = x1 - x0;
        d.drawFastVLine(x0 + (int)((0 - mn)/(mx - mn) * gw), HDR_H, LCD_H - HDR_H - FTR_H, C_DIM);
        for (int i = 0; i < K; i++) {
            int rowH2 = ((LCD_H - HDR_H - FTR_H - 16) / K < 8) ? 8 : ((LCD_H - HDR_H - FTR_H - 16) / K); int yy = HDR_H + 4 + i * rowH2; if (yy > LCD_H - FTR_H - 8) continue;
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
            if (IS_ESC) return;
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
            if (IS_ESC) return;
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
        if (fabs(dn) < 1e-12) { DM.notify("Undefined: all SE identical", C_ERR); return; }
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
        if (seB < 1e-15) { DM.notify("Undefined: perfect fit", C_WARN); return; }
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
    void showGraph(int s) {
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

    void run() {
        const char* items[] = {
            "Histogram","Box-and-Whisker","Scatter plot","Q-Q plot (vs Normal)",
            "Bar chart","Pie chart","Line chart","Stem-and-leaf","Pareto chart",
            "ECDF plot","Density (KDE) approx","Strip chart","Cumulative line",
            "Mean +/- SD plot","Residuals plot","P-P plot","Multi-line overlay","Back"
        };
        for (;;) {
        int s = DM.menu("Statistical Graphs", items, 18);
        if (s < 0 || s == 17) return;
        showGraph(s);
        }
    }
private:
    int pickList(const char* title) {
        const char* it[STAT_LISTS]; for (int _i = 0; _i < STAT_LISTS; _i++) it[_i] = statLbl(_i);
        return DM.menu(title, it, STAT_LISTS);
    }

    void afterPlot(const char* prefix) {
        DM.footer("S:BMP","","ESC:back");
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
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
        if (!h) { DM.notify("malloc fail", C_ERR); return; }
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
        if (!tmp) { DM.notify("malloc fail", C_ERR); return; }
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
        if (fabs(mx - mn) < 1e-12) mx = mn + 1;
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
        int regOn = DM.confirm("Overlay regression?");
        double xmn, xmx, ymn, ymx;
        getRange(G.stat[xi].data, n, xmn, xmx);
        getRange(G.stat[yi].data, n, ymn, ymx);
        double slope = 0, intercept = 0;
        if (regOn && n >= 2) {
            double sx=0, sy=0, sxx=0, sxy=0;
            for (int i = 0; i < n; i++) {
                double xv = G.stat[xi].data[i], yv = G.stat[yi].data[i];
                sx += xv; sy += yv; sxx += xv*xv; sxy += xv*yv;
            }
            double dn = n*sxx - sx*sx;
            if (fabs(dn) > 1e-12) {
                slope = (n*sxy - sx*sy) / dn;
                intercept = (sy - slope*sx) / n;
                double y0 = intercept + slope*xmn, y1 = intercept + slope*xmx;
                if (y0 < ymn) ymn = y0; if (y0 > ymx) ymx = y0;
                if (y1 < ymn) ymn = y1; if (y1 > ymx) ymx = y1;
            } else regOn = 0;
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(regOn ? "Scatter + regression" : "Scatter");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        auto sx_ = [&](double xv) { return gx + (int)((xv - xmn)/(xmx - xmn + 1e-12) * (gw - 4)) + 2; };
        auto sy_ = [&](double yv) { return gy + gh - 2 - (int)((yv - ymn)/(ymx - ymn + 1e-12) * (gh - 4)); };
        for (int i = 0; i < n; i++) d.fillCircle(sx_(G.stat[xi].data[i]), sy_(G.stat[yi].data[i]), 1, C_ACC);
        if (regOn) {
            d.drawLine(sx_(xmn), sy_(intercept + slope*xmn), sx_(xmx), sy_(intercept + slope*xmx), C_ERR);
            char ln[40]; snprintf(ln, sizeof(ln), "y=%.4gx+%.4g", slope, intercept);
            d.setTextColor(C_ERR, C_BG); d.drawString(ln, gx + 4, gy + 2);
        }
        afterPlot("scatter");
    }

    void qqPlot() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count; if (n < 4) { DM.notify("n<4", C_ERR); return; }
        double* tmp = (double*)malloc(n*sizeof(double));
        if (!tmp) { DM.notify("malloc fail", C_ERR); return; }
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
        if (n < 1) { DM.notify("Empty list", C_ERR); return; }
        double mx = 0;
        for (int i = 0; i < n; i++) if (fabs(G.stat[li].data[i]) > mx) mx = fabs(G.stat[li].data[i]);
        if (mx < 1e-12) mx = 1;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Bar Chart");
        int bw = (LCD_W - 8) / n;
        if (bw < 2) bw = 2;
        int gh = LCD_H - HDR_H - FTR_H - 8;
        for (int i = 0; i < n; i++) {
            int h = (int)(fabs((double)G.stat[li].data[i])/mx * gh);
            uint16_t col = (G.stat[li].data[i] >= 0) ? C_ACC : C_ERR;
            d.fillRect(4 + i*bw, HDR_H + 4 + gh - h, bw - 1, h, col);
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
        if (n < 2) { DM.notify("n<2 required", C_ERR); return; }
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
                int av = (v < 0) ? -v : v;
                if (av/10 == stem) {
                    char sg = (v < 0) ? (char)45 : (char)32;
                    snprintf(tmp, sizeof(tmp), "%c%d", sg, av % 10);
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
        if (n < 1) { DM.notify("Empty list", C_ERR); return; }
        double* a = (double*)malloc(n*sizeof(double));
        if (!a) { DM.notify("malloc fail", C_ERR); return; }
        memcpy(a, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=a[i]; int j=i-1;
            while (j>=0 && a[j]<k){a[j+1]=a[j];j--;} a[j+1]=k; }
        double sum = 0, maxAbs = 1; for (int i = 0; i < n; i++) { sum += a[i]; if (fabs(a[i]) > maxAbs) maxAbs = fabs(a[i]); }
        if (sum < 1e-12) sum = 1;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Pareto Chart");
        int bw = (LCD_W - 8) / n; if (bw < 2) bw = 2;
        int gh = LCD_H - HDR_H - FTR_H - 8;
        double cum = 0;
        int prevX = -1, prevY = -1;
        for (int i = 0; i < n; i++) {
            int h = (int)(fabs((double)a[i])/maxAbs * gh);
            uint16_t col = (a[i] >= 0) ? C_ACC : C_ERR;
            d.fillRect(4 + i*bw, HDR_H + 4 + gh - h, bw - 1, h, col);
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
        if (n < 2) { DM.notify("n<2 required", C_ERR); return; }
        double* a = (double*)malloc(n*sizeof(double));
        if (!a) { DM.notify("malloc fail", C_ERR); return; }
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
        if (n < 2) { DM.notify("n<2 required", C_ERR); return; }
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
        if (!fv) { DM.notify("malloc fail", C_ERR); return; }
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
        if (n < 2) { DM.notify("n<2 required", C_ERR); return; }
        double sum = 0; double mx = 0;
        for (int i = 0; i < n; i++) { sum += G.stat[li].data[i]; if (sum > mx) mx = sum; }
        if (mx < 1e-12) mx = 1;
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Cumulative");
        int gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        sum = 0;
        int prevX = 4, prevY = HDR_H + 4 + gh - 1;
        for (int i = 0; i < n; i++) {
            sum += G.stat[li].data[i];
            if (isnan(sum) || isinf(sum)) continue;
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
        if (fabs(mx - mn) < 1e-12) mx = mn + 1;
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
        int n = G.stat[0].count;
        double* X = G.stat[0].data;
        double* Y = G.stat[1].data;
        double sx=0, sy=0, sxx=0, sxy=0;
        for (int i = 0; i < n; i++) { sx += X[i]; sy += Y[i]; sxx += X[i]*X[i]; sxy += X[i]*Y[i]; }
        double dn2 = n*sxx - sx*sx;
        if (fabs(dn2) < 1e-12) { DM.notify("Sxx=0", C_ERR); return; }
        double b = (n*sxy - sx*sy) / dn2;
        double a = (sy - b*sx) / n;
        double* res = (double*)malloc(n*sizeof(double));
        if (!res) { DM.notify("malloc fail", C_ERR); return; }
        double rmn = INFINITY, rmx = -INFINITY;
        double xmn = X[0], xmx = X[0];
        for (int i = 0; i < n; i++) {
            res[i] = Y[i] - (a + b*X[i]);
            if (res[i] < rmn) rmn = res[i];
            if (res[i] > rmx) rmx = res[i];
            if (X[i] < xmn) xmn = X[i];
            if (X[i] > xmx) xmx = X[i];
        }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Residuals vs X");
        int gx = 4, gy = HDR_H + 4, gw = LCD_W - 8, gh = LCD_H - HDR_H - FTR_H - 8;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        if (rmn < 0 && rmx > 0) {
            int yZero = gy + gh - 2 - (int)((0 - rmn)/(rmx - rmn + 1e-12) * (gh - 4));
            d.drawLine(gx, yZero, gx + gw, yZero, C_GRID);
        }
        for (int i = 0; i < n; i++) {
            int px = gx + (int)((X[i] - xmn)/(xmx - xmn + 1e-12) * (gw - 4)) + 2;
            int py = gy + gh - 2 - (int)((res[i] - rmn)/(rmx - rmn + 1e-12) * (gh - 4));
            d.fillCircle(px, py, 1, C_ACC);
        }
        free(res);
        afterPlot("resid");
    }

    void ppPlot() {
        int li = pickList("List?"); if (li < 0) return;
        int n = G.stat[li].count;
        double* a = (double*)malloc(n*sizeof(double));
        if (!a) { DM.notify("malloc fail", C_ERR); return; }
        memcpy(a, G.stat[li].data, n*sizeof(double));
        for (int i = 1; i < n; i++) { double k=a[i]; int j=i-1;
            while (j>=0 && a[j]>k){a[j+1]=a[j];j--;} a[j+1]=k; }
        double sum = 0, sum2 = 0;
        for (int i = 0; i < n; i++) { sum += a[i]; sum2 += a[i]*a[i]; }
        double m = sum/n, sd = sqrt((sum2 - n*m*m)/(n-1)); if (sd < 1e-12) { DM.notify("PP: zero variance", C_ERR); free(a); return; }
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
void statGraphShow(int kind) { gStatGraphs.showGraph(kind); }
void statGraphMenu() { gStatGraphs.run(); }
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
    if (!e) return nullptr;
    e->parent = parent;
    envList[envN++] = e;
    return e;
}

static Env globalEnv;
static const char* gPos;
static int parseDepth = 0;
static int gEvalDepth = 0;
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
    if (parseDepth > 25) { setErr("parse too deep"); return nilC; }
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
    if (parseDepth > 25) { setErr("parse too deep"); return nilC; }
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
        if (isNil(nc)) return head;
        if (isNil(head)) { head = tail = nc; }
        else { tail->cons.cdr = nc; tail = nc; }
        args = args->cons.cdr;
    }
    return head;
}

double toD(CellRef c) { if (c && c->type == T_NUM) return c->num; return 0; }
CellRef bAdd(CellRef a, Env* e) { double s = 0; while (!isNil(a) && a->type == T_CONS) { s += toD(a->cons.car); a = a->cons.cdr; } return mkNum(s); }
CellRef bSub(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(0);
    double s = toD(a->cons.car); a = a->cons.cdr;
    if (isNil(a)) return mkNum(-s);
    while (!isNil(a) && a->type == T_CONS) { s -= toD(a->cons.car); a = a->cons.cdr; }
    return mkNum(s);
}

CellRef bMul(CellRef a, Env* e) { double s = 1; while (!isNil(a) && a->type == T_CONS) { s *= toD(a->cons.car); a = a->cons.cdr; } return mkNum(s); }
CellRef bDiv(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(1);
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
    if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC;
    double d = toD(a->cons.cdr->cons.car);
    if (d == 0) { setErr("mod by zero"); return nilC; }
    return mkNum(fmod(toD(a->cons.car), d));
}

CellRef bLT(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bGT(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >  toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bLE(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) <= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bGE(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC; return (toD(a->cons.car) >= toD(a->cons.cdr->cons.car)) ? trueC : nilC; }
CellRef bEq(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC;
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
CellRef bCons(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS || isNil(a->cons.cdr)) return nilC; return mkCons(a->cons.car, a->cons.cdr->cons.car); }
CellRef bList(CellRef a, Env* e) { return a; }
CellRef bNullP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; return isNil(a->cons.car) ? trueC : nilC; }
CellRef bAtomP(CellRef a, Env* e){ if (isNil(a) || a->type != T_CONS) return trueC; CellRef c = a->cons.car; return (isNil(c) || c->type != T_CONS) ? trueC : nilC; }
CellRef bNumP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_NUM) ? trueC : nilC; }
CellRef bSymP(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) return nilC; CellRef c = a->cons.car; return (c && c->type == T_SYM) ? trueC : nilC; }
CellRef bLength(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return mkNum(0);
    int n = 0, safety = 10000; CellRef l = a->cons.car;
    while (!isNil(l) && l->type == T_CONS && safety-- > 0) { n++; l = l->cons.cdr; }
    return mkNum(n);
}

CellRef bAppend(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) return nilC;
    CellRef result = nilC, tail = nilC;
    CellRef cur = a;
    int outerSafety = 2000;
    while (!isNil(cur) && cur->type == T_CONS && outerSafety-- > 0) {
        CellRef item = cur->cons.car;
        bool isLast = isNil(cur->cons.cdr);
        if (isLast) {
            if (isNil(result)) return item;
            tail->cons.cdr = item;
            return result;
        }
        CellRef l = item;
        int safety = 2000;
        while (!isNil(l) && l->type == T_CONS && safety-- > 0) {
            CellRef nc = mkCons(l->cons.car, nilC);
            if (isNil(nc)) return result;
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
    CellRef l = a->cons.car; CellRef r = nilC; int safety = 10000;
    while (!isNil(l) && l->type == T_CONS && safety-- > 0) { r = mkCons(l->cons.car, r); l = l->cons.cdr; }
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
CellRef bSqrt(CellRef a, Env* e) { if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; } if (!isNil(a->cons.cdr)) { setErr("too many args"); return nilC; } double v = toD(a->cons.car); if (v < 0) { setErr("sqrt(neg)"); return nilC; } return mkNum(sqrt(v)); }
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
    if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; }
    CellRef l = a->cons.car;
    double s = 0; int n = 0;
    while (!isNil(l) && l->type == T_CONS) { s += toD(l->cons.car); n++; l = l->cons.cdr; }
    return (n > 0) ? mkNum(s/n) : mkNum(0);
}

CellRef bSum(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; }
    CellRef l = a->cons.car; double s = 0;
    while (!isNil(l) && l->type == T_CONS) { s += toD(l->cons.car); l = l->cons.cdr; }
    return mkNum(s);
}

CellRef bSD(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; }
    CellRef l = a->cons.car;
    double s = 0, s2 = 0; int n = 0;
    while (!isNil(l) && l->type == T_CONS) { double v = toD(l->cons.car); s += v; s2 += v*v; n++; l = l->cons.cdr; }
    if (n < 2) return mkNum(0);
    double m = s/n;
    return mkNum(sqrt((s2 - n*m*m)/(n-1)));
}

void printCell(CellRef c, char* buf, int& p, int max);
CellRef bPrint(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; }
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
        int cycGuard = 0;
        while (!isNil(c) && c->type == T_CONS && cycGuard < 256) {
            if (!first) putc1(' ');
            printCell(c->cons.car, buf, p, max);
            first = false;
            c = c->cons.cdr;
            cycGuard++;
        }
        if (!isNil(c) && c->type == T_CONS) put(" ...");
        else if (!isNil(c)) { put(" . "); printCell(c, buf, p, max); }
        putc1(')');
    }
}

CellRef applyFn(CellRef fn, CellRef args, Env* env) {
    if (gEvalDepth >= POMAS_ULISP_MAX_DEPTH) { setErr("apply depth"); return nilC; }
    DepthGuard dgA(&gEvalDepth);
    if (!fn) return nilC;
    if (fn->type == T_BUILTIN) return fn->builtin(args, env);
    if (fn->type == T_LAMBDA) {
        Env* newEnv = allocEnv(fn->lambda.env);
        if (!newEnv) { setErr("Env limit"); return nilC; }
        CellRef p = fn->lambda.params;
        CellRef a = args;
        while (!isNil(p) && p->type == T_CONS) {
            if (isNil(a)) break;
            if (!p->cons.car || p->cons.car->type != T_SYM) { setErr("bad lambda param"); return nilC; }
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
        if (isNil(nc)) return head;
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
            if (isNil(nc)) return head;
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

CellRef eval(CellRef expr, Env* env) {
    if (gEvalDepth >= POMAS_ULISP_MAX_DEPTH) { setErr("eval depth"); return nilC; }
    DepthGuard dgE(&gEvalDepth);
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
                if (clause->cons.car && clause->cons.car->type == T_SYM && !strcmp(clause->cons.car->sym, "else")) {
                    CellRef body = clause->cons.cdr;
                    CellRef r = nilC;
                    while (!isNil(body) && body->type == T_CONS) { r = eval(body->cons.car, env); body = body->cons.cdr; }
                    return r;
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
    const char* savePos = gPos;
    CellRef r = parseStr(content.c_str());
    gPos = savePos;
    return r;
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
    gEvalDepth = 0;
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
        for (;;) {
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
                if (IS_ESC) return;
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
                    if (cur < 79 && c >= ' ' && c < 127) {
                        char cc = (c == '_') ? '-' : ((c == '-') ? '_' : c);
                        input[cur++] = cc; input[cur] = 0;
                    }
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
                "Gradient field (F1=f(x,y))","Phase plane (F1=dx/dt,F2=dy/dt)",
                "Color/Style (per fn)","Inequality intersect","Shape overlay (circle/line/rect)","Param surface x,y,z(u,v)","Save plot as BMP","Back"
            };
            int s = DM.menu("Graph", items, 17);
            if (s < 0 || s == 16) return;
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
                case 9:  drawGradientField(); break;
                case 10: drawPhasePlane();    break;
                case 11: editColorStyle(); break;
                case 12: plotIneqIntersect(); break;
                case 13: editShapes(); break;
                case 14: plotParamSurface(); break;
                case 15: { plot(false); DM.saveBMPAuto("graph"); } break;
            }
        }
    }

    char  expr[MAXF][48] = {"sin(x)", "", "", ""};
    bool  active[MAXF]   = {true, false, false, false};
    uint16_t colorFn[MAXF] = {C_ACC, C_OK, C_ACC2, C_ERR};
    int      styleFn[MAXF] = {0, 0, 0, 0};
    static const int SHAPE_MAX = 8;
    uint8_t shapeK[SHAPE_MAX] = {0};
    double  shapeP[SHAPE_MAX][4] = {{0}};
    int     shapeN = 0;
    int   modeT = 0;
    double xmin = -6.28, xmax = 6.28, ymin = -2.5, ymax = 2.5;
    double zmin = -2, zmax = 2;
    double tmin = 0, tmax = 6.28, tstep = 0.05;
    int    seqStart = 1, seqEnd = 30;
    double vmin = 0, vmax = 6.283185307179586;
    int    implCell = 2;            // 陰関数: セルサイズ[px] (2..8)
    int    grid3N = 16;             // 3D: 格子分割数 (6..40)
    double view3Phi = M_PI/6, view3Theta = M_PI/8;   // 3D視点 (方位角, 仰角)
private:
    const char* colorName(uint16_t c) {
        if (c==C_ACC) return "Mint";
        if (c==C_OK) return "Green";
        if (c==C_ACC2) return "DeepMint";
        if (c==C_ERR) return "Red";
        if (c==C_WARN) return "Amber";
        if (c==0x07FF) return "Cyan";
        return "?";
    }

    void drawLineStyled(int x0, int y0, int x1, int y1, uint16_t col, int style) {
        auto& d = M5Cardputer.Display;
        if (style == 0) { d.drawLine(x0, y0, x1, y1, col); return; }
        int dx = x1 - x0, dy = y1 - y0;
        int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
        if (steps == 0) { d.drawPixel(x0, y0, col); return; }
        for (int i = 0; i <= steps; i++) {
            bool skip = (style == 1) ? ((i % 3) != 0) : ((i % 6) >= 3);
            if (skip) continue;
            int px = x0 + (int)((long)dx * i / steps);
            int py = y0 + (int)((long)dy * i / steps);
            d.drawPixel(px, py, col);
        }
    }

    void drawShapes() {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        auto toPx = [&](double xv, double yv, int& px, int& py) {
            px = gx + (int)((xv - xmin)/(xmax - xmin) * gw);
            py = gy + gh - 1 - (int)((yv - ymin)/(ymax - ymin) * gh);
        };
        for (int i = 0; i < shapeN; i++) {
            int p1x, p1y, p2x, p2y;
            toPx(shapeP[i][0], shapeP[i][1], p1x, p1y);
            if (shapeK[i] == 1) {
                int rx = (int)(shapeP[i][2]/(xmax-xmin)*gw);
                int ry = (int)(shapeP[i][2]/(ymax-ymin)*gh);
                if (rx < 1) rx = 1;
                if (ry < 1) ry = 1;
                d.drawEllipse(p1x, p1y, rx, ry, C_ACC);
            } else {
                toPx(shapeP[i][2], shapeP[i][3], p2x, p2y);
                if (shapeK[i] == 2) d.drawLine(p1x, p1y, p2x, p2y, C_ACC);
                else { int rx0 = (p1x < p2x) ? p1x : p2x; int ry0 = (p1y < p2y) ? p1y : p2y; d.drawRect(rx0, ry0, abs(p2x - p1x), abs(p2y - p1y), C_ACC); }
            }
        }
    }

    void editShapes() {
        while (true) {
            char lb[SHAPE_MAX+3][30];
            const char* L[SHAPE_MAX+3];
            for (int i = 0; i < shapeN; i++) {
                if (shapeK[i] == 1) snprintf(lb[i],30,"C%d(%.3g,%.3g)r%.3g",i+1,shapeP[i][0],shapeP[i][1],shapeP[i][2]);
                else if (shapeK[i] == 2) snprintf(lb[i],30,"L%d(%.3g,%.3g)-(%.3g,%.3g)",i+1,shapeP[i][0],shapeP[i][1],shapeP[i][2],shapeP[i][3]);
                else                     snprintf(lb[i],30,"R%d(%.3g,%.3g)-(%.3g,%.3g)",i+1,shapeP[i][0],shapeP[i][1],shapeP[i][2],shapeP[i][3]);
                L[i] = lb[i];
            }
            snprintf(lb[shapeN],30,"+ Circle");
            snprintf(lb[shapeN+1],30,"+ Line");
            snprintf(lb[shapeN+2],30,"+ Rect / Clear");
            L[shapeN]=lb[shapeN]; L[shapeN+1]=lb[shapeN+1]; L[shapeN+2]=lb[shapeN+2];
            int sel = DM.menu("Shapes", L, shapeN+3);
            if (sel < 0) return;
            auto askN = [&](double* a, int n) -> bool {
                const char* nm[] = {"x1","y1","x2","y2"};
                char b[24];
                for (int i = 0; i < n; i++) {
                    strcpy(b, i<2?"0":"1");
                    if (!DM.inputLine(nm[i], b, sizeof(b))) return false;
                    a[i] = atof(b);
                }
                return true;
            };
            if (sel < shapeN) {
                if (DM.confirm("Delete?")) {
                    for (int k = sel; k < shapeN-1; k++) {
                        shapeK[k]=shapeK[k+1];
                        for (int j=0;j<4;j++) shapeP[k][j]=shapeP[k+1][j];
                    }
                    shapeN--;
                }
            } else if (sel == shapeN + 2) {
                const char* it[] = {"Add rect","Clear all","Cancel"};
                int s2 = DM.menu("Rect/Clear", it, 3);
                if (s2 == 0) { if (shapeN < SHAPE_MAX && askN(shapeP[shapeN], 4)) { shapeK[shapeN]=3; shapeN++; } }
                else if (s2 == 1) shapeN = 0;
            } else if (shapeN >= SHAPE_MAX) {
                DM.notify("Shapes full", C_ERR);
            } else if (sel == shapeN) {
                if (askN(shapeP[shapeN], 3)) { shapeK[shapeN]=1; shapeN++; }
            } else if (sel == shapeN+1) {
                if (askN(shapeP[shapeN], 4)) { shapeK[shapeN]=2; shapeN++; }
            }
        }
    }

    void plotParamSurface() {
        if (!expr[0][0] || !expr[1][0] || !expr[2][0]) { DM.notify("Set F1=x(u,v), F2=y, F3=z", C_ERR, 1500); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Param Surface");
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        const int N = grid3N;
        ExprEval::Subst sub[2] = {{'u',0},{'v',0}};
        double u0 = tmin, u1 = tmax, v0 = vmin, v1 = vmax;
        double du = (u1-u0)/N, dv = (v1-v0)/N;
        double phi = view3Phi, theta = view3Theta;
        double cp = cos(phi), sp = sin(phi), ct = cos(theta), st_ = sin(theta);
        sub[0].val = (u0+u1)/2; sub[1].val = (v0+v1)/2;
        auto rcx = gEv.eval(expr[0], sub, 2);
        auto rcy = gEv.eval(expr[1], sub, 2);
        auto rcz = gEv.eval(expr[2], sub, 2);
        double cx0 = rcx.ok?rcx.val:0, cy0 = rcy.ok?rcy.val:0, cz0 = rcz.ok?rcz.val:0;
        double ex_=1e-9, ey_=1e-9, ez_=1e-9;
        for (int i = 0; i <= N; i++) for (int j = 0; j <= N; j++) {
            sub[0].val = u0 + i*du; sub[1].val = v0 + j*dv;
            auto r1=gEv.eval(expr[0],sub,2), r2=gEv.eval(expr[1],sub,2), r3=gEv.eval(expr[2],sub,2);
            if(r1.ok){double a=fabs(r1.val-cx0); if(a>ex_)ex_=a;}
            if(r2.ok){double a=fabs(r2.val-cy0); if(a>ey_)ey_=a;}
            if(r3.ok){double a=fabs(r3.val-cz0); if(a>ez_)ez_=a;}
        }
        double span = ex_ + ey_ + ez_;
        double scale = (gw < gh ? gw : gh) / (2.5 * (span + 1e-9));
        auto project = [&](double xv,double yv,double zv,int& px,int& py){
            double xr = (xv-cx0)*cp - (yv-cy0)*sp;
            double yr = (xv-cx0)*sp + (yv-cy0)*cp;
            double zr = (zv-cz0);
            px = gx + gw/2 + (int)(xr*scale);
            py = gy + gh/2 - (int)((-yr*st_ + zr*ct)*scale);
        };
        for (int i = 0; i <= N; i++) {
            int pX=-1, pY=-1; bool ok=false;
            for (int j = 0; j <= N; j++) {
                sub[0].val = u0 + i*du; sub[1].val = v0 + j*dv;
                auto r1=gEv.eval(expr[0],sub,2), r2=gEv.eval(expr[1],sub,2), r3=gEv.eval(expr[2],sub,2);
                if(!r1.ok||!r2.ok||!r3.ok||isnan(r1.val)||isnan(r2.val)||isnan(r3.val)||isinf(r1.val)||isinf(r2.val)||isinf(r3.val)){ok=false;continue;}
                int px,py; project(r1.val,r2.val,r3.val,px,py);
                if(ok) d.drawLine(pX,pY,px,py,C_ACC);
                pX=px; pY=py; ok=true;
            }
        }
        for (int j = 0; j <= N; j++) {
            int pX=-1, pY=-1; bool ok=false;
            for (int i = 0; i <= N; i++) {
                sub[0].val = u0 + i*du; sub[1].val = v0 + j*dv;
                auto r1=gEv.eval(expr[0],sub,2), r2=gEv.eval(expr[1],sub,2), r3=gEv.eval(expr[2],sub,2);
                if(!r1.ok||!r2.ok||!r3.ok||isnan(r1.val)||isnan(r2.val)||isnan(r3.val)||isinf(r1.val)||isinf(r2.val)||isinf(r3.val)){ok=false;continue;}
                int px,py; project(r1.val,r2.val,r3.val,px,py);
                if(ok) d.drawLine(pX,pY,px,py,C_OK);
                pX=px; pY=py; ok=true;
            }
        }
        DM.footer("fn+arrows:rotate", "S:save", "ESC:back");
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (st.fn && IS_LEFT)  { view3Phi -= 0.15; break; }
            if (st.fn && IS_RIGHT) { view3Phi += 0.15; break; }
            if (st.fn && IS_UP)    { view3Theta += 0.1; if(view3Theta>1.5)view3Theta=1.5; break; }
            if (st.fn && IS_DOWN)  { view3Theta -= 0.1; if(view3Theta<-0.2)view3Theta=-0.2; break; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("psurf");
        }
    }

    void editColorStyle() {
        const char* labels[MAXF];
        char buf[MAXF][40];
        for (int i = 0; i < MAXF; i++) {
            const char* cn = colorName(colorFn[i]);
            const char* sn = (styleFn[i]==0)?"solid":(styleFn[i]==1?"dot":"dash");
            snprintf(buf[i], sizeof(buf[i]), "F%d %s %s", i+1, cn, sn);
            labels[i] = buf[i];
        }
        int s = DM.menu("Color/Style", labels, MAXF);
        if (s < 0) return;
        const char* cn[] = {"Mint","Green","DeepMint","Red","Amber","Cyan"};
        uint16_t cv[] = {C_ACC, C_OK, C_ACC2, C_ERR, C_WARN, 0x07FF};
        int c = DM.menu("Color", cn, 6);
        if (c >= 0) colorFn[s] = cv[c];
        const char* sn[] = {"Solid","Dotted","Dashed"};
        int st = DM.menu("Style", sn, 3);
        if (st >= 0) styleFn[s] = st;
    }

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
        if (xmin >= xmax) { double t = xmin; xmin = xmax; xmax = t; }
        if (ymin >= ymax) { double t = ymin; ymin = ymax; ymax = t; }
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
            snprintf(b, sizeof(b), "%d", grid3N); if (!DM.inputLine("grid N (6-40):", b, sizeof(b))) return;
            grid3N = atoi(b); if (grid3N < 6) grid3N = 6; if (grid3N > 40) grid3N = 40;
        }
        if (modeT == 3) {
            snprintf(b, sizeof(b), "%d", implCell); if (!DM.inputLine("px/cell (2-8):", b, sizeof(b))) return;
            implCell = atoi(b); if (implCell < 2) implCell = 2; if (implCell > 8) implCell = 8;
        }
    }

    void stdWindow()  { xmin=-10; xmax=10; ymin=-10; ymax=10; tmin=0; tmax=6.28; tstep=0.05; view3Phi = M_PI/6; view3Theta = M_PI/8; DM.notify("Std", C_OK); }
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
            if (modeT == 6) DM.footer("fn+arrows:rotate", "Z X R P:prm", "ESC:back");
            else            DM.footer("Z:in X:out R:rst T:trc", "B:box P:prm +:nx", "ESC:back");
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC) return;
                if (!st.word.empty()) {
                    char c = st.word[0];
                    if (c == 'z' || c == 'Z') { zoomIn();  break; }
                    if (c == 'x' || c == 'X') { zoomOut(); break; }
                    if (c == 'r' || c == 'R') { stdWindow(); break; }
                    if (c == 't' || c == 'T') { traceOn = !traceOn; break; }
                    if (c == 'b' || c == 'B') { boxZoom(); break; }
                    if (c == 'p' || c == 'P') { paramSlider(); break; }
                    if (c == '+' || c == 'n') {
                        for (int k = 1; k <= MAXF; k++) {
                            int q = (traceFn + k) % MAXF;
                            if (active[q]) { traceFn = q; break; }
                        }
                        break;
                    }
                    if (c == 's' || c == 'S') { DM.saveBMPAuto("graph"); break; }
                }
                if (modeT == 6) {
                    if (st.fn && IS_LEFT)  { view3Phi -= 0.15; break; }
                    if (st.fn && IS_RIGHT) { view3Phi += 0.15; break; }
                    if (st.fn && IS_UP)    { view3Theta += 0.1; if (view3Theta > 1.5) view3Theta = 1.5; break; }
                    if (st.fn && IS_DOWN)  { view3Theta -= 0.1; if (view3Theta < -0.2) view3Theta = -0.2; break; }
                }
                if (traceOn) {
                    double dx = (xmax - xmin) / 80.0;
                    if (st.fn && IS_LEFT)  { tcur -= dx; break; }
                    if (st.fn && IS_RIGHT) { tcur += dx; break; }
                }
            }
        }
    }

    // パラメータスライダー: 変数 A〜J を fn+←/→ で増減し、グラフを再描画
    void paramSlider() {
        int vi = 0;
        double step = 0.1;
        auto setv = [&](double v) {
            char t[32]; snprintf(t, sizeof(t), "%.10g", v);
            G.vars[vi] = atof(t);
        };
        while (true) {
            drawAll();
            auto& d = M5Cardputer.Display;
            char b[56];
            snprintf(b, sizeof(b), "%c=%g step=%g", 'A' + vi, G.vars[vi], step);
            d.fillRect(1, HDR_H + 1, 156, 10, C_BG);
            d.setTextColor(C_OK, C_BG);
            d.drawString(b, 3, HDR_H + 2);
            DM.footer("a-j:var ,/:val", ";.:step v:set", "ESC:back");
            while (true) {
                if (!DM.pollKeys()) { delay(15); continue; }
                auto st = M5Cardputer.Keyboard.keysState();
                if (IS_ESC)   return;
                if (st.fn && IS_LEFT)  { setv(G.vars[vi] - step); break; }
                if (st.fn && IS_RIGHT) { setv(G.vars[vi] + step); break; }
                if (st.fn && IS_UP)    { step *= 10; break; }
                if (st.fn && IS_DOWN)  { step /= 10; if (step < 1e-6) step = 1e-6; break; }
                if (!st.word.empty()) {
                    char c = st.word[0];
                    if (c >= 'A' && c <= 'J') c += 32;
                    if (c >= 'a' && c <= 'j') { vi = c - 'a'; break; }
                    if (c == 'v' || c == 'V') {
                        char vb[24]; snprintf(vb, sizeof(vb), "%g", G.vars[vi]);
                        if (DM.inputLine("value:", vb, sizeof(vb))) setv(atof(vb));
                        break;
                    }
                }
            }
        }
    }

    void drawAll() {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG);
        if (fabs(xmax - xmin) < 1e-12 || fabs(ymax - ymin) < 1e-12) return;
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
        uint16_t* cols = colorFn;
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
        if (shapeN > 0) drawShapes();
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
            if (prevOk) drawLineStyled(prevX, prevY, gx + px, py, col, styleFn[f]);
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
            if (!rx.ok || !ry.ok || isnan(rx.val) || isnan(ry.val) || isinf(rx.val) || isinf(ry.val)) { prevOk = false; continue; }
            double x = rx.val, y = ry.val;
            if (x < xmin || x > xmax || y < ymin || y > ymax) { prevOk = false; continue; }
            int px = gx + (int)((x - xmin)/(xmax - xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin) * gh);
            if (prevOk) drawLineStyled(prevX, prevY, px, py, col, styleFn[f]);
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
            if (!r.ok || isnan(r.val) || isinf(r.val)) { prevOk = false; continue; }
            double rad = r.val;
            double x = rad * cos(t), y = rad * sin(t);
            if (x < xmin || x > xmax || y < ymin || y > ymax) { prevOk = false; continue; }
            int px = gx + (int)((x - xmin)/(xmax - xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax - ymin) * gh);
            if (prevOk) drawLineStyled(prevX, prevY, px, py, col, styleFn[f]);
            prevX = px; prevY = py; prevOk = true;
        }
    }

    // 陰関数 f(x,y)=0 : マーチングスクエア法。セルサイズは implCell ピクセル、
    // 鞍点セルは双線形中心値で連結を判定する。
    void plotImplicit(int f, uint16_t col) {
        auto& d = M5Cardputer.Display;
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        int cs = implCell; if (cs < 2) cs = 2; if (cs > 8) cs = 8;
        int RX = gw / cs, RY = gh / cs;
        if (RX < 4) RX = 4; if (RX > 128) RX = 128;
        if (RY < 4) RY = 4; if (RY > 128) RY = 128;
        ExprEval::Subst sub[2] = {{'x', 0}, {'y', 0}};
        double cellX = (xmax - xmin) / RX, cellY = (ymax - ymin) / RY;
        double prev[129], cur[129];
        auto evalAt = [&](double x, double y) -> double {
            sub[0].val = x; sub[1].val = y;
            auto r = gEv.eval(expr[f], sub, 2);
            return (r.ok && !isnan(r.val) && !isinf(r.val)) ? r.val : NAN;
        };
        auto toPx = [&](double x, double y, int& px, int& py) {
            px = gx + (int)((x - xmin) / (xmax - xmin) * gw);
            py = gy + gh - 1 - (int)((y - ymin) / (ymax - ymin) * gh);
        };
        auto seg = [&](double xa, double ya, double xb, double yb) {
            int x1, y1, x2, y2;
            toPx(xa, ya, x1, y1); toPx(xb, yb, x2, y2);
            d.drawLine(x1, y1, x2, y2, col);
        };
        for (int j = 0; j <= RX; j++) prev[j] = evalAt(xmin + j * cellX, ymin);
        for (int i = 0; i < RY; i++) {
            double y0 = ymin + i * cellY, y1 = y0 + cellY;
            for (int j = 0; j <= RX; j++) cur[j] = evalAt(xmin + j * cellX, y1);
            for (int j = 0; j < RX; j++) {
                double v[4] = {prev[j], prev[j+1], cur[j+1], cur[j]};   // 左下, 右下, 右上, 左上
                if (isnan(v[0]) || isnan(v[1]) || isnan(v[2]) || isnan(v[3])) continue;
                int code = 0;
                for (int k = 0; k < 4; k++) if (v[k] > 0) code |= (1 << k);
                if (code == 0 || code == 15) continue;
                double x0 = xmin + j * cellX, x1 = x0 + cellX;
                double ex[4], ey[4]; bool has[4];
                // 辺0:下, 辺1:右, 辺2:上, 辺3:左
                static const int ea[4] = {0, 1, 2, 3}, eb[4] = {1, 2, 3, 0};
                double cxv[4] = {x0, x1, x1, x0}, cyv[4] = {y0, y0, y1, y1};
                int np = 0;
                for (int e = 0; e < 4; e++) {
                    int a = ea[e], b = eb[e];
                    has[e] = ((v[a] > 0) != (v[b] > 0));
                    if (has[e]) {
                        double t = v[a] / (v[a] - v[b]);
                        ex[e] = cxv[a] + t * (cxv[b] - cxv[a]);
                        ey[e] = cyv[a] + t * (cyv[b] - cyv[a]);
                        np++;
                    }
                }
                if (np == 2) {
                    int p = -1, q = -1;
                    for (int e = 0; e < 4; e++) if (has[e]) { if (p < 0) p = e; else q = e; }
                    seg(ex[p], ey[p], ex[q], ey[q]);
                } else if (np == 4) {
                    double center = (v[0] + v[1] + v[2] + v[3]) / 4;
                    bool c1 = ((center > 0) == (v[1] > 0));
                    if (c1) { seg(ex[0], ey[0], ex[3], ey[3]); seg(ex[1], ey[1], ex[2], ey[2]); }
                    else    { seg(ex[0], ey[0], ex[1], ey[1]); seg(ex[2], ey[2], ex[3], ey[3]); }
                }
            }
            for (int j = 0; j <= RX; j++) prev[j] = cur[j];
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

    void plotIneqIntersect() {
        int activeN = 0;
        for (int f = 0; f < MAXF; f++) if (active[f] && expr[f][0]) activeN++;
        if (activeN == 0) { DM.notify("No active fn", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Ineq Intersection"); DM.footer("S:save","","ESC:back");
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        if (xmin < 0 && xmax > 0) d.drawLine(gx + (int)((-xmin)/(xmax-xmin)*gw), gy, gx + (int)((-xmin)/(xmax-xmin)*gw), gy+gh-1, C_GRID);
        if (ymin < 0 && ymax > 0) { int sy = gy + gh - 1 - (int)((-ymin)/(ymax-ymin)*gh); d.drawLine(gx, sy, gx+gw-1, sy, C_GRID); }
        ExprEval::Subst sub[2] = {{'x',0},{'y',0}};
        for (int py = 0; py < gh; py += 2) {
            double yv = ymin + (gh - 1 - py) * (ymax - ymin) / gh;
            sub[1].val = yv;
            for (int px = 0; px < gw; px += 2) {
                double xv = xmin + px * (xmax - xmin) / gw;
                sub[0].val = xv;
                bool allPos = true;
                for (int f = 0; f < MAXF; f++) {
                    if (!active[f] || !expr[f][0]) continue;
                    auto r = gEv.eval(expr[f], sub, 2);
                    if (!r.ok || r.val <= 0) { allPos = false; break; }
                }
                if (allPos) d.drawPixel(gx + px, gy + py, C_ACC);
            }
        }
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("ineqix");
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
            if (!r.ok || isnan(r.val) || isinf(r.val)) continue;
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
        const int N = grid3N;
        double dx = (xmax - xmin)/N, dy = (ymax - ymin)/N;
        ExprEval::Subst sub[2] = {{'x', 0}, {'y', 0}};
        double phi = view3Phi, theta = view3Theta;
        double cp = cos(phi), sp = sin(phi);
        double ct = cos(theta), st_ = sin(theta);
        double cx0 = (xmin + xmax)/2, cy0 = (ymin + ymax)/2, cz0 = (zmin + zmax)/2;
        double xspan3 = xmax - xmin; if (xspan3 < 1e-9) xspan3 = 1e-9;
        double scale = (gw < gh ? gw : gh) / (2.5 * xspan3);
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
                if (!r.ok || isnan(r.val) || isinf(r.val)) { ok = false; continue; }
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
                if (!r.ok || isnan(r.val) || isinf(r.val)) { ok = false; continue; }
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
        if (!r.ok || isnan(r.val) || isinf(r.val)) return;
        if (xv < xmin || xv > xmax || r.val < ymin || r.val > ymax) return;
        int px = gx + (int)((xv - xmin)/(xmax - xmin) * gw);
        if (px > gx + gw - 1) px = gx + gw - 1;
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
        if (xmin >= xmax) { double t = xmin; xmin = xmax; xmax = t; }
        if (ymin >= ymax) { double t = ymin; ymin = ymax; ymax = t; }
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

    void drawGradientField() {
        if (!expr[0][0]) { DM.notify("Set F1 = f(x,y)", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Gradient Field"); DM.footer("S:save R:range", "", "ESC:back");
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        if (xmin < 0 && xmax > 0) d.drawLine(gx + (int)((-xmin)/(xmax-xmin)*gw), gy, gx + (int)((-xmin)/(xmax-xmin)*gw), gy+gh-1, C_GRID);
        if (ymin < 0 && ymax > 0) { int sy = gy + gh - 1 - (int)((-ymin)/(ymax-ymin)*gh); d.drawLine(gx, sy, gx+gw-1, sy, C_GRID); }
        const int NX = 30, NY = 16;
        ExprEval::Subst sb[2] = {{'x',0},{'y',0}};
        double dxUnit = (xmax - xmin) / NX * 0.4;
        for (int ix = 0; ix < NX; ix++) for (int iy = 0; iy < NY; iy++) {
            double x = xmin + (ix + 0.5) * (xmax - xmin) / NX;
            double y = ymin + (iy + 0.5) * (ymax - ymin) / NY;
            sb[0].val = x; sb[1].val = y;
            auto r = gEv.eval(expr[0], sb, 2);
            if (!r.ok || isnan(r.val) || isinf(r.val)) continue;
            double slope = r.val;
            double dx = dxUnit;
            double dy = slope * dxUnit;
            double len = sqrt(dx*dx + dy*dy);
            if (len < 1e-12) continue;
            double maxL = dxUnit * 1.2;
            if (len > maxL) { dx *= maxL/len; dy *= maxL/len; }
            int px = gx + (int)((x - xmin)/(xmax-xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax-ymin) * gh);
            int qx = gx + (int)((x + dx - xmin)/(xmax-xmin) * gw);
            int qy = gy + gh - 1 - (int)((y + dy - ymin)/(ymax-ymin) * gh);
            d.drawLine(px, py, qx, qy, C_ACC);
        }
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("grad");
            if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) { editRange(); break; }
        }
    }

    void drawPhasePlane() {
        if (!expr[0][0] || !expr[1][0]) { DM.notify("Set F1=dx/dt, F2=dy/dt", C_ERR); return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header("Phase Plane"); DM.footer("S:save R:range", "", "ESC:back");
        int gx = 0, gy = HDR_H, gw = LCD_W, gh = LCD_H - HDR_H - FTR_H;
        d.drawRect(gx, gy, gw, gh, C_DIM);
        if (xmin < 0 && xmax > 0) d.drawLine(gx + (int)((-xmin)/(xmax-xmin)*gw), gy, gx + (int)((-xmin)/(xmax-xmin)*gw), gy+gh-1, C_GRID);
        if (ymin < 0 && ymax > 0) { int sy = gy + gh - 1 - (int)((-ymin)/(ymax-ymin)*gh); d.drawLine(gx, sy, gx+gw-1, sy, C_GRID); }
        const int NX = 24, NY = 13;
        ExprEval::Subst sb[2] = {{'x',0},{'y',0}};
        double mag[24*13]; double maxMag = 0;
        for (int ix = 0; ix < NX; ix++) for (int iy = 0; iy < NY; iy++) {
            double x = xmin + (ix + 0.5) * (xmax - xmin) / NX;
            double y = ymin + (iy + 0.5) * (ymax - ymin) / NY;
            sb[0].val = x; sb[1].val = y;
            auto rx = gEv.eval(expr[0], sb, 2);
            auto ry = gEv.eval(expr[1], sb, 2);
            double m = (rx.ok && ry.ok) ? sqrt(rx.val*rx.val + ry.val*ry.val) : 0;
            mag[ix*NY+iy] = m;
            if (m > maxMag && isfinite(m)) maxMag = m;
        }
        if (maxMag <= 0) maxMag = 1;
        for (int ix = 0; ix < NX; ix++) for (int iy = 0; iy < NY; iy++) {
            double x = xmin + (ix + 0.5) * (xmax - xmin) / NX;
            double y = ymin + (iy + 0.5) * (ymax - ymin) / NY;
            sb[0].val = x; sb[1].val = y;
            auto rx = gEv.eval(expr[0], sb, 2);
            auto ry = gEv.eval(expr[1], sb, 2);
            if (!rx.ok || !ry.ok || isnan(rx.val) || isnan(ry.val) || isinf(rx.val) || isinf(ry.val)) continue;
            double vx = rx.val, vy = ry.val;
            double len = sqrt(vx*vx + vy*vy);
            if (len < 1e-12) continue;
            double frac = mag[ix*NY+iy] / maxMag;
            double arrowLen = (xmax - xmin) / NX * 0.4 * (0.3 + 0.7 * frac);
            double dx = vx / len * arrowLen;
            double dy = vy / len * arrowLen;
            int px = gx + (int)((x - xmin)/(xmax-xmin) * gw);
            int py = gy + gh - 1 - (int)((y - ymin)/(ymax-ymin) * gh);
            int qx = gx + (int)((x + dx - xmin)/(xmax-xmin) * gw);
            int qy = gy + gh - 1 - (int)((y + dy - ymin)/(ymax-ymin) * gh);
            uint16_t col = (frac > 0.6) ? C_ERR : ((frac > 0.3) ? C_ACC2 : C_ACC);
            d.drawLine(px, py, qx, qy, col);
            d.fillCircle(qx, qy, 1, col);
        }
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("phase");
            if (M5Cardputer.Keyboard.isKeyPressed('r') || M5Cardputer.Keyboard.isKeyPressed('R')) { editRange(); break; }
        }
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
                if (IS_ESC) return;
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
        const char* it[] = { "Sheet A..H -> L1..L8", "L1..L8 -> Sheet A..H", "Sort rows by column", "Filter rows by column to L?", "Cancel" };
        int s = DM.menu("Sheet <-> Lists", it, 5);
        if (s < 0 || s == 4) return;
        if (s == 2) { sortMenu(); return; }
        if (s == 3) { filterMenu(); return; }
        if (!G.stat[0].data) { if (!G.allocStat()) { DM.notify("alloc fail", C_ERR); return; } }
        if (s == 0) {
            for (int c = 0; c < STAT_LISTS; c++) {
                G.stat[c].count = 0;
                for (int r = 0; r < SS_ROWS; r++) {
                    double v = evalCell(r, c);
                    if (!isnan(v) && !isinf(v) && G.stat[c].count < G.statCapacity)
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

    // ---- 範囲関数 / IF / セル参照展開 ----
    static bool ssBoundary(const char* start, const char* p) {
        return p == start || !(isalnum((unsigned char)p[-1]) || p[-1] == '_' || p[-1] == '.');
    }

    // 'A1' 形式を消費。パターン一致なら true（範囲外は valid=false）。
    static bool ssParseRef(const char*& p, int& r, int& c, bool& valid) {
        if (!isalpha((unsigned char)*p) || !isdigit((unsigned char)p[1])) return false;
        int cc = toupper((unsigned char)*p) - 'A';
        const char* q = p + 1;
        long rr = 0;
        while (isdigit((unsigned char)*q)) { if (rr < 100000) rr = rr*10 + (*q - '0'); q++; }
        rr--;
        p = q;
        valid = (cc >= 0 && cc < SS_COLS && rr >= 0 && rr < SS_ROWS);
        r = (int)rr; c = cc;
        return true;
    }

    // SUM/AVG/MIN/MAX/COUNT/PROD(範囲|セル|数値, ...) を数値へ展開
    bool ssTryRangeFn(const char*& p, const char* start, char* out, int& n, int cap, int depth) {
        if (!ssBoundary(start, p)) return false;
        char nm[8]; int k = 0; const char* q = p;
        while (isalpha((unsigned char)*q) && k < 7) nm[k++] = (char)toupper((unsigned char)*q++);
        nm[k] = 0;
        if (isalpha((unsigned char)*q) || *q != '(') return false;
        int fn = -1;
        if (!strcmp(nm, "SUM")) fn = 0;
        else if (!strcmp(nm, "AVG") || !strcmp(nm, "AVERAGE") || !strcmp(nm, "MEAN")) fn = 1;
        else if (!strcmp(nm, "MIN")) fn = 2;
        else if (!strcmp(nm, "MAX")) fn = 3;
        else if (!strcmp(nm, "COUNT")) fn = 4;
        else if (!strcmp(nm, "PROD")) fn = 5;
        if (fn < 0) return false;
        q++;
        double sum = 0, prod = 1, mn = INFINITY, mx = -INFINITY; long cnt = 0;
        auto add = [&](double v) {
            if (isnan(v)) return;
            sum += v; prod *= v; cnt++;
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        };
        while (true) {
            while (*q == ' ') q++;
            int r0, c0, r1, c1; bool v0, v1;
            if (ssParseRef(q, r0, c0, v0)) {
                if (!v0) return false;
                r1 = r0; c1 = c0;
                if (*q == ':') {
                    q++;
                    if (!ssParseRef(q, r1, c1, v1) || !v1) return false;
                }
                if (r0 > r1) { int t = r0; r0 = r1; r1 = t; }
                if (c0 > c1) { int t = c0; c0 = c1; c1 = t; }
                for (int rr = r0; rr <= r1; rr++)
                    for (int cc = c0; cc <= c1; cc++)
                        if (G.ssData[rr][cc]) add(evalCell(rr, cc, depth + 1));
            } else {
                char* e; double v = strtod(q, &e);
                if (e == q) return false;
                q = e; add(v);
            }
            while (*q == ' ') q++;
            if (*q == ',') { q++; continue; }
            if (*q == ')') { q++; break; }
            return false;
        }
        double res;
        switch (fn) {
            case 0: res = sum; break;
            case 1: res = cnt ? sum / cnt : NAN; break;
            case 2: res = cnt ? mn : NAN; break;
            case 3: res = cnt ? mx : NAN; break;
            case 4: res = (double)cnt; break;
            default: res = cnt ? prod : NAN; break;
        }
        n += snprintf(out + n, cap - n, "(%.12g)", res);
        if (n > cap - 1) n = cap - 1;
        p = q;
        return true;
    }

    // IF(cond, a, b) → ((cond)?(a):(b))
    bool ssTryIf(const char*& p, const char* start, char* out, int& n, int cap, int depth) {
        if (!ssBoundary(start, p)) return false;
        if (toupper((unsigned char)p[0]) != 'I' || toupper((unsigned char)p[1]) != 'F' || p[2] != '(') return false;
        const char* q = p + 3;
        const char* seg[3]; int len[3]; int ns = 0, lvl = 1;
        seg[0] = q;
        for (; *q; q++) {
            if (*q == '(') lvl++;
            else if (*q == ')') {
                if (--lvl == 0) break;
            } else if (*q == ',' && lvl == 1) {
                if (ns >= 2) return false;
                len[ns] = (int)(q - seg[ns]); ns++; seg[ns] = q + 1;
            }
        }
        if (*q != ')' || ns != 2) return false;
        len[2] = (int)(q - seg[2]);
        char part[3][96];
        for (int i = 0; i < 3; i++) {
            if (len[i] >= 90) return false;
            char tmp[96]; memcpy(tmp, seg[i], len[i]); tmp[len[i]] = 0;
            expandRefs(tmp, part[i], 96, depth);
        }
        n += snprintf(out + n, cap - n, "((%s)?(%s):(%s))", part[0], part[1], part[2]);
        if (n > cap - 1) n = cap - 1;
        p = q + 1;
        return true;
    }

    int expandRefs(const char* src, char* out, int cap, int depth) {
        const char* p = src;
        int n = 0;
        out[0] = 0;
        while (*p && n < cap - 20) {
            if (isalpha((unsigned char)*p)) {
                if (ssTryRangeFn(p, src, out, n, cap, depth)) continue;
                if (ssTryIf(p, src, out, n, cap, depth)) continue;
                int r, c; bool valid;
                if (ssBoundary(src, p) && ssParseRef(p, r, c, valid)) {
                    if (valid) { n += snprintf(out + n, cap - n, "(%.12g)", evalCell(r, c, depth + 1)); if (n > cap - 1) n = cap - 1; }
                    continue;
                }
                // 通常の識別子(sin, log10, pi, e1 等)は丸ごとコピー
                while ((isalnum((unsigned char)*p) || *p == '_') && n < cap - 2) out[n++] = *p++;
            } else {
                out[n++] = *p++;
            }
        }
        out[n] = 0;
        return n;
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
        char expanded[256];
        expandRefs(p, expanded, sizeof(expanded), depth);
        auto r = gEv.eval(expanded);
        return r.ok ? r.val : NAN;
    }

    int pickCol(const char* prompt) {
        char b[4] = "A";
        if (!DM.inputLine(prompt, b, sizeof(b))) return -1;
        char c = toupper((unsigned char)b[0]);
        if (c < 'A' || c > 'Z') return -1;
        int col = c - 'A';
        if (col >= SS_COLS) return -1;
        return col;
    }

    void sortMenu() {
        int col = pickCol("Sort by col (A-Z):");
        if (col < 0) return;
        const char* d[] = {"Ascending", "Descending"};
        int ord = DM.menu("Order", d, 2);
        if (ord < 0) return;
        bool asc = (ord == 0);
        int rows[SS_ROWS], n = 0;
        for (int r = 0; r < SS_ROWS; r++) {
            bool any = false;
            for (int c = 0; c < SS_COLS; c++) if (G.ssData[r][c]) { any = true; break; }
            if (any) rows[n++] = r;
        }
        for (int i = 0; i < n-1; i++) for (int j = 0; j < n-i-1; j++) {
            double a = evalCell(rows[j], col);
            double b = evalCell(rows[j+1], col);
            bool aN = isnan(a), bN = isnan(b);
            bool sw = false;
            if (aN && !bN) sw = true;
            else if (!aN && !bN) sw = asc ? (a > b) : (a < b);
            if (sw) {
                int r1 = rows[j], r2 = rows[j+1];
                for (int c = 0; c < SS_COLS; c++) { char* tm = G.ssData[r1][c]; G.ssData[r1][c] = G.ssData[r2][c]; G.ssData[r2][c] = tm; }
                rows[j] = r2; rows[j+1] = r1;
            }
        }
        char m[32]; snprintf(m, sizeof(m), "Sorted %d rows by %c", n, 'A'+col);
        DM.notify(m, C_OK, 1200);
    }

    void filterMenu() {
        int col = pickCol("Filter col (A-Z):");
        if (col < 0) return;
        const char* op[] = {"= v", "< v", "> v", "!= v"};
        int oi = DM.menu("Op", op, 4);
        if (oi < 0) return;
        char b[16] = "0";
        if (!DM.inputLine("value:", b, sizeof(b))) return;
        double v = atof(b);
        const char* L[8]; char lb[8][20];
        for (int i = 0; i < 8; i++) { snprintf(lb[i],20,"L%d (n=%d)",i+1,G.stat[i].count); L[i]=lb[i]; }
        int dst = DM.menu("Dest list", L, 8);
        if (dst < 0) return;
        if (!G.stat[dst].data) { DM.notify("alloc first", C_ERR); return; }
        G.stat[dst].count = 0;
        int matched = 0;
        for (int r = 0; r < SS_ROWS && G.stat[dst].count < G.statCapacity; r++) {
            double x = evalCell(r, col);
            if (isnan(x) || isinf(x)) continue;
            bool ok = false;
            if (oi == 0) ok = (fabs(x-v) < 1e-9);
            else if (oi == 1) ok = (x < v);
            else if (oi == 2) ok = (x > v);
            else ok = (fabs(x-v) >= 1e-9);
            if (ok) { G.stat[dst].data[G.stat[dst].count++] = x; matched++; }
        }
        char m[40]; snprintf(m, sizeof(m), "Filtered %d vals to L%d", matched, dst+1);
        DM.notify(m, C_OK, 1500);
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
// ── Medical Calculator ─ 医学・疫学系の集約メニュー ─
class MedicalPK {
public:
    void run() {
        const char* items[] = {
            "Half-life (from k)",
            "Half-life (Vd, CL)",
            "Loading dose",
            "Maintenance dose",
            "Dose by weight",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Pharmacokinetics", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: halfLifeK();  break;
            case 1: halfLifeCL(); break;
            case 2: loadingDose();break;
            case 3: maintDose();  break;
            case 4: doseWt();     break;
        }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); DM.drawWrapped(n, 4, HDR_H + 28, LCD_W - 8, C_ACC2); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("pk");
        }
    }

    void halfLifeK() {
        double k = ask("Elim k (1/h):", "0.1"); if (isnan(k) || k <= 0) return;
        double v = 0.693 / k;
        show("Half-life (k)", "t1/2", v, "t1/2 = ln2 / k");
    }

    void halfLifeCL() {
        double vd = ask("Vd (L):", "50"); if (isnan(vd) || vd <= 0) return;
        double cl = ask("CL (L/h):", "5"); if (isnan(cl) || cl <= 0) return;
        double v = 0.693 * vd / cl;
        show("Half-life (CL)", "t1/2", v, "t1/2 = 0.693*Vd/CL");
    }

    void loadingDose() {
        double vd = ask("Vd (L):", "50"); if (isnan(vd)) return;
        double ctarget = ask("Target C (mg/L):", "10"); if (isnan(ctarget) || ctarget <= 0) return;
        double F = ask("Bioavail F (0-1):", "1"); if (isnan(F) || F <= 0) return;
        double v = vd * ctarget / F;
        show("Loading Dose", "LD", v, "LD = Vd * Ctarget / F");
    }

    void maintDose() {
        double cl = ask("CL (L/h):", "5"); if (isnan(cl) || cl <= 0) return;
        double ctarget = ask("Target Css (mg/L):", "10"); if (isnan(ctarget) || ctarget <= 0) return;
        double F = ask("Bioavail F (0-1):", "1"); if (isnan(F) || F <= 0) return;
        double v = cl * ctarget / F;
        show("Maintenance Dose", "MD (/h)", v, "MD = CL*Css/F, per hour. x interval(h) = dose per interval");
    }

    void doseWt() {
        double mgkg = ask("Dose (mg/kg):", "10"); if (isnan(mgkg) || mgkg <= 0) return;
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double v = mgkg * wt;
        show("Dose by weight", "Dose", v, "Dose = mg/kg * Wt");
    }
};

MedicalPK gMedPK;
class MedicalOB {
public:
    void run() {
        const char* items[] = {
            "Naegele (EDD)",
            "Gestational age (from LMP)",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Obstetrics", items, 3);
        if (s < 0 || s == 2) return;
        cx_ = false;
        switch (s) { case 0: naegele(); break; case 1: ga(); break; }
        }
    }
private:
    static bool isLeap(int y) { return (y%4==0 && y%100!=0) || (y%400==0); }
    static int daysInMonth(int y, int m) {
        static const int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        if (m==2 && isLeap(y)) return 29;
        return d[m-1];
    }

    static long toJD(int y, int m, int d) {
        if (m <= 2) { y--; m += 12; }
        int A = y / 100;
        int B = 2 - A + A/4;
        return (long)(365.25*(y+4716)) + (int)(30.6001*(m+1)) + d + B - 1524;
    }

    static void fromJD(long jd, int& y, int& m, int& d) {
        long a = jd + 32044;
        long b = (4*a + 3) / 146097;
        long c = a - (146097*b)/4;
        long dd = (4*c + 3) / 1461;
        long e = c - (1461*dd)/4;
        long mm = (5*e + 2) / 153;
        d = e - (153*mm + 2)/5 + 1;
        m = mm + 3 - 12*(mm/10);
        y = 100*b + dd - 4800 + mm/10;
    }

    int askInt(const char* p, int defVal) {
        char b[8]; snprintf(b, sizeof(b), "%d", defVal);
        if (cx_) return defVal;
        if (!DM.inputLine(p, b, sizeof(b))) { cx_ = true; return defVal; }
        return atoi(b);
    }
    bool cx_ = false;

    void showDate(const char* t, int y, int m, int d) {
        auto& disp = M5Cardputer.Display;
        disp.fillScreen(C_BG); DM.header(t); DM.footer("","","ESC:back");
        disp.setTextColor(C_OK, C_BG); disp.setTextSize(2);
        char l[32]; snprintf(l, sizeof(l), "%04d-%02d-%02d", y, m, d);
        disp.drawString(l, 4, HDR_H + 12); disp.setTextSize(1);
        disp.setTextColor(C_ACC2, C_BG);
        disp.drawString("EDD = LMP + 280 days", 4, HDR_H + 44);
        DM.waitEsc();
    }

    void naegele() {
        int ly = askInt("LMP year:", 2024);
        int lm = askInt("LMP month:", 1);
        int ld = askInt("LMP day:", 1);
        if (lm < 1 || lm > 12 || ld < 1 || ld > daysInMonth(ly, lm)) { DM.notify("Bad date", C_ERR); return; }
        if (cx_) return;
        long jd = toJD(ly, lm, ld) + 280;
        int ey, em, ed;
        fromJD(jd, ey, em, ed);
        showDate("EDD (Naegele)", ey, em, ed);
    }

    void ga() {
        int ly = askInt("LMP year:", 2024);
        int lm = askInt("LMP month:", 1);
        int ld = askInt("LMP day:", 1);
        if (lm < 1 || lm > 12 || ld < 1 || ld > daysInMonth(ly, lm)) { DM.notify("Bad date", C_ERR); return; }
        int cy = askInt("Today year:", 2024);
        int cm = askInt("Today month:", 6);
        int cd = askInt("Today day:", 1);
        if (cx_) return;
        long days = toJD(cy, cm, cd) - toJD(ly, lm, ld);
        if (days < 0) { DM.notify("Negative days", C_ERR); return; }
        int weeks = days / 7;
        int dd2 = days % 7;
        char m[40]; snprintf(m, sizeof(m), "GA = %dw %dd (%ld d)", weeks, dd2, days);
        DM.notify(m, C_OK, 2000);
        G.ans = (double)days;
    }
};

MedicalOB gMedOB;
class MedicalEndo {
public:
    void run() {
        const char* items[] = {
            "HOMA-IR (insulin resistance)",
            "Free T4 index",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Endocrine", items, 3);
        if (s < 0 || s == 2) return;
        switch (s) { case 0: homaIr(); break; case 1: ft4(); break; }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); DM.drawWrapped(n, 4, HDR_H + 28, LCD_W - 8, C_ACC2); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("endo");
        }
    }

    void homaIr() {
        double glu = ask("Fasting glu (mg/dL):", "90"); if (isnan(glu) || glu <= 0) return;
        double ins = ask("Fasting insulin (uU/mL):", "10"); if (isnan(ins) || ins <= 0) return;
        double v = (glu * ins) / 405.0;
        const char* n = (v < 2.0) ? "Normal (<2.0)"
                      : (v < 2.9) ? "Borderline (2.0-2.8)"
                      : "Insulin resistant (>=2.9)";
        show("HOMA-IR", "HOMA-IR", v, n);
    }

    void ft4() {
        double t4 = ask("Total T4 (ug/dL):", "8"); if (isnan(t4) || t4 <= 0) return;
        double t3u = ask("T3 uptake (%):", "30"); if (isnan(t3u) || t3u <= 0) return;
        double v = t4 * (t3u / 30.0);
        const char* n = (v < 4.5) ? "Low (<4.5)"
                      : (v < 12)  ? "Normal (4.5-12)"
                      : "High (>12)";
        show("Free T4 Index", "FTI", v, n);
    }
};

MedicalEndo gMedEndo;
class MedicalLytes {
public:
    void run() {
        const char* items[] = {
            "Corrected Ca (Payne)",
            "Corrected Na (Katz)",
            "Anion gap",
            "Serum osmolality",
            "Winter's formula (pCO2 exp)",
            "Delta gap / Delta ratio",
            "Free water deficit",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Lytes / Acid-Base", items, 8);
        if (s < 0 || s == 7) return;
        switch (s) {
            case 0: corrCa();    break;
            case 1: corrNa();    break;
            case 2: anionGap();  break;
            case 3: osmol();     break;
            case 4: winters();   break;
            case 5: deltaGap();  break;
            case 6: fwd();       break;
        }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); DM.drawWrapped(n, 4, HDR_H + 28, LCD_W - 8, C_ACC2); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("lytes");
        }
    }

    void corrCa() {
        double ca = ask("Ca (mg/dL):", "9.0"); if (isnan(ca) || ca <= 0) return;
        double alb = ask("Albumin (g/dL):", "4.0"); if (isnan(alb) || alb < 0) return;
        double v = ca + 0.8 * (4.0 - alb);
        show("Corrected Ca (Payne)", "Ca_corr", v, "Ca + 0.8*(4.0-Alb)");
    }

    void corrNa() {
        double na = ask("Na (mEq/L):", "140"); if (isnan(na) || na <= 0) return;
        double glu = ask("Glucose (mg/dL):", "100"); if (isnan(glu) || glu < 0) return;
        double v = na + 1.6 * (glu - 100) / 100.0;
        show("Corrected Na (Katz)", "Na_corr", v, "Na + 1.6*(Glu-100)/100");
    }

    void anionGap() {
        double na = ask("Na (mEq/L):", "140"); if (isnan(na) || na <= 0) return;
        double cl = ask("Cl (mEq/L):", "104"); if (isnan(cl) || cl <= 0) return;
        double hco3 = ask("HCO3 (mEq/L):", "24"); if (isnan(hco3) || hco3 <= 0) return;
        double v = na - (cl + hco3);
        const char* n = (v > 12) ? "High AG (>12) - MUDPILES"
                      : (v < 8)  ? "Low AG (<8)"
                      : "Normal (8-12)";
        show("Anion Gap", "AG", v, n);
    }

    void osmol() {
        double na = ask("Na (mEq/L):", "140"); if (isnan(na) || na <= 0) return;
        double glu = ask("Glucose (mg/dL):", "100"); if (isnan(glu) || glu < 0) return;
        double bun = ask("BUN (mg/dL):", "14"); if (isnan(bun) || bun < 0) return;
        double v = 2*na + glu/18.0 + bun/2.8;
        show("Serum Osmolality", "Osm", v, "2Na + Glu/18 + BUN/2.8");
    }

    void winters() {
        double hco3 = ask("HCO3 (mEq/L):", "24"); if (isnan(hco3) || hco3 <= 0) return;
        double pco2 = ask("Measured pCO2:", "40"); if (isnan(pco2) || pco2 <= 0) return;
        double exp_lo = 1.5 * hco3 + 6;
        double exp_hi = 1.5 * hco3 + 10;
        const char* jd_ = (pco2 < exp_lo) ? "Meas<exp: +resp alk" : (pco2 > exp_hi) ? "Meas>exp: +resp acid" : "Appropriate";
        char nb[96]; snprintf(nb, sizeof(nb), "Exp pCO2 %.1f-%.1f. %s", exp_lo, exp_hi, jd_);
        double exp_ = 1.5*hco3 + 8;
        show("Winter's formula", "pCO2_exp", exp_, nb);
    }

    void deltaGap() {
        double na = ask("Na (mEq/L):", "140"); if (isnan(na) || na <= 0) return;
        double cl = ask("Cl (mEq/L):", "104"); if (isnan(cl) || cl <= 0) return;
        double hco3 = ask("HCO3 (mEq/L):", "24"); if (isnan(hco3) || hco3 <= 0) return;
        double ag = na - (cl + hco3);
        double dAG = ag - 12.0;
        double dHCO3 = 24.0 - hco3;
        double ratio = (fabs(dHCO3) > 1e-6) ? dAG / dHCO3 : 0;
        const char* n = (ratio < 0.4) ? "Pure NAGMA"
                      : (ratio <= 1.0) ? "Mixed / NAGMA"
                      : (ratio <= 2.0) ? "Pure HAGMA"
                      : "Mixed HAGMA + metabolic alk";
        char nb[80]; snprintf(nb, sizeof(nb), "dAG=%.1f dHCO3=%.1f - %s", dAG, dHCO3, n);
        show("Delta ratio", "dAG/dHCO3", ratio, nb);
    }

    void fwd() {
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double na = ask("Na (mEq/L):", "160"); if (isnan(na) || na <= 0) return;
        double v = 0.6 * wt * (na / 140.0 - 1.0);
        const char* n = (v > 0) ? "Free water deficit (L)" : "Na <140: excess";
        show("Free Water Deficit", "FWD", v, n);
    }
};

MedicalLytes gMedLytes;
class MedicalRenal {
public:
    void run() {
        const char* items[] = {
            "eGFR CKD-EPI 2021",
            "Cockcroft-Gault CrCl",
            "Schwartz (pediatric)",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Renal Function", items, 4);
        if (s < 0 || s == 3) return;
        switch (s) { case 0: ckdEpi(); break; case 1: cockcroft(); break; case 2: schwartz(); break; }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); DM.drawWrapped(n, 4, HDR_H + 28, LCD_W - 8, C_ACC2); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("renal");
        }
    }

    const char* gStage(double egfr) {
        if (egfr >= 90) return "G1 (>=90) normal";
        if (egfr >= 60) return "G2 (60-89) mild";
        if (egfr >= 45) return "G3a (45-59)";
        if (egfr >= 30) return "G3b (30-44)";
        if (egfr >= 15) return "G4 (15-29) severe";
        return "G5 (<15) failure";
    }

    void ckdEpi() {
        double scr = ask("SCr (mg/dL):", "1.0"); if (isnan(scr) || scr <= 0) return;
        double age = ask("Age (years):", "50"); if (isnan(age) || age <= 0) return;
        int female = (int)ask("Female? (0/1):", "0");
        double k = female ? 0.7 : 0.9;
        double a = female ? -0.241 : -0.302;
        double ratio = scr / k;
        double egfr = 142.0 * pow(fmin(ratio,1.0), a) * pow(fmax(ratio,1.0), -1.200) * pow(0.9938, age);
        if (female) egfr *= 1.012;
        show("eGFR CKD-EPI 2021", "eGFR", egfr, gStage(egfr));
    }

    void cockcroft() {
        double age = ask("Age (years):", "50"); if (isnan(age) || age <= 0) return;
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double scr = ask("SCr (mg/dL):", "1.0"); if (isnan(scr) || scr <= 0) return;
        int female = (int)ask("Female? (0/1):", "0");
        double crcl = ((140 - age) * wt) / (72.0 * scr);
        if (female) crcl *= 0.85;
        show("Cockcroft-Gault CrCl", "CrCl", crcl, gStage(crcl));
    }

    void schwartz() {
        double ht = ask("Height (cm):", "120"); if (isnan(ht)) return;
        double scr = ask("SCr (mg/dL):", "0.6"); if (isnan(scr) || scr <= 0) return;
        double egfr = 0.413 * ht / scr;
        show("Schwartz (peds)", "eGFR", egfr, gStage(egfr));
    }
};

MedicalRenal gMedRenal;
class MedicalBody {
public:
    void run() {
        const char* items[] = {
            "BMI (body mass index)",
            "BSA Mosteller",
            "BSA DuBois",
            "IBW (Devine, male/female)",
            "ABW (adjusted body weight)",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Body Composition", items, 6);
        if (s < 0 || s == 5) return;
        switch (s) {
            case 0: bmi();       break;
            case 1: bsaMost();   break;
            case 2: bsaDub();    break;
            case 3: ibw();       break;
            case 4: abw();       break;
        }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); DM.drawWrapped(n, 4, HDR_H + 28, LCD_W - 8, C_ACC2); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("body");
        }
    }

    void bmi() {
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double ht = ask("Height (cm):", "170"); if (isnan(ht) || ht <= 0) return;
        double m = ht / 100.0;
        double v = wt / (m*m);
        const char* n = (v < 18.5) ? "Underweight (<18.5)"
                      : (v < 25)   ? "Normal (18.5-24.9)"
                      : (v < 30)   ? "Overweight (25-29.9)"
                      : (v < 35)   ? "Obese I (30-34.9)"
                      : (v < 40)   ? "Obese II (35-39.9)"
                      : "Obese III (>=40)";
        show("BMI", "BMI", v, n);
    }

    void bsaMost() {
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double ht = ask("Height (cm):", "170"); if (isnan(ht) || ht <= 0) return;
        double v = sqrt(ht * wt / 3600.0);
        show("BSA Mosteller", "BSA", v, "m^2");
    }

    void bsaDub() {
        double wt = ask("Weight (kg):", "70"); if (isnan(wt) || wt <= 0) return;
        double ht = ask("Height (cm):", "170"); if (isnan(ht) || ht <= 0) return;
        double v = 0.007184 * pow(wt, 0.425) * pow(ht, 0.725);
        show("BSA DuBois", "BSA", v, "m^2");
    }

    void ibw() {
        double ht = ask("Height (cm):", "170"); if (isnan(ht) || ht <= 0) return;
        double inches = ht / 2.54;
        double male = 50.0 + 2.3 * (inches - 60);
        double female = 45.5 + 2.3 * (inches - 60);
        if (male < 0) male = 0;
        if (female < 0) female = 0;
        char nb[64]; snprintf(nb, sizeof(nb), "Male %.1f kg / Female %.1f kg", male, female);
        double fq = ask("Female? (0/1):", "0"); if (isnan(fq) || (fq != 0 && fq != 1)) return;
        show("IBW Devine", fq != 0 ? "Female" : "Male", fq != 0 ? female : male, nb);
    }

    void abw() {
        double wt = ask("Actual Wt (kg):", "80"); if (isnan(wt) || wt <= 0) return;
        double ht = ask("Height (cm):", "170"); if (isnan(ht) || ht <= 0) return;
        double inches = ht / 2.54;
        double fq = ask("Female? (0/1):", "0"); if (isnan(fq) || (fq != 0 && fq != 1)) return;
        double ibw = (fq != 0 ? 45.5 : 50.0) + 2.3 * (inches - 60);
        if (ibw < 0) ibw = 0;
        double v = ibw + 0.4 * (wt - ibw);
        show("Adjusted BW", "ABW", v, "IBW + 0.4(ABW-IBW)");
    }
};

MedicalBody gMedBody;
class MedicalScores {
public:
    void run() {
        const char* items[] = {
            "GCS (Glasgow Coma Scale)",
            "CURB-65 (pneumonia)",
            "qSOFA (sepsis)",
            "Child-Pugh (cirrhosis)",
            "MELD score",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Clinical Scores", items, 6);
        if (s < 0 || s == 5) return;
        cx_ = false;
        switch (s) {
            case 0: gcs();      break;
            case 1: curb65();   break;
            case 2: qsofa();    break;
            case 3: childPugh();break;
            case 4: meld();     break;
        }
        }
    }
private:
    bool cx_ = false;
    void show(const char* t, const char* l1, double v1, const char* interp) {
        if (cx_) { cx_ = false; return; }
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[64]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (interp && *interp) {
            d.setTextColor(C_ACC2, C_BG);
            DM.drawWrapped(interp, 4, HDR_H + 28, LCD_W - 8, C_ACC2);
        }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) {
                DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S'))
                DM.saveBMPAuto("score");
        }
    }

    int askInt(const char* p, int defVal) {
        char b[8]; snprintf(b, sizeof(b), "%d", defVal);
        if (cx_) return defVal;
        if (!DM.inputLine(p, b, sizeof(b))) { cx_ = true; return defVal; }
        return atoi(b);
    }

    void gcs() {
        int eye = askInt("Eye (1-4):", 4);
        int verbal = askInt("Verbal (1-5):", 5);
        int motor = askInt("Motor (1-6):", 6);
        if (eye < 1 || eye > 4 || verbal < 1 || verbal > 5 || motor < 1 || motor > 6) { DM.notify("E1-4 V1-5 M1-6", C_ERR); return; }
        int total = eye + verbal + motor;
        const char* n = (total <= 8) ? "Severe (<=8) intubate"
                      : (total <= 12) ? "Moderate (9-12)"
                      : "Mild (13-15)";
        char ib[80]; snprintf(ib, sizeof(ib), "E%d V%d M%d - %s", eye, verbal, motor, n);
        show("GCS", "Total", total, ib);
    }

    void curb65() {
        int conf = askInt("Confusion (0/1):", 0);
        int urea = askInt("Urea>7 (0/1):", 0);
        int rr = askInt("RR>=30 (0/1):", 0);
        int bp = askInt("SBP<90|DBP<=60 (0/1):", 0);
        int age = askInt("Age>=65 (0/1):", 0);
        if (conf<0||conf>1||urea<0||urea>1||rr<0||rr>1||bp<0||bp>1||age<0||age>1) { DM.notify("0/1 only", C_ERR); return; }
        int t = conf + urea + rr + bp + age;
        const char* n = (t <= 1) ? "Low risk (outpatient)"
                      : (t == 2) ? "Moderate (short admission)"
                      : "High risk (>2) admit";
        show("CURB-65", "Score", t, n);
    }

    void qsofa() {
        int rr = askInt("RR>=22 (0/1):", 0);
        int sbp = askInt("SBP<=100 (0/1):", 0);
        int gcs = askInt("GCS<15 (0/1):", 0);
        if (rr<0||rr>1||sbp<0||sbp>1||gcs<0||gcs>1) { DM.notify("0/1 only", C_ERR); return; }
        int t = rr + sbp + gcs;
        const char* n = (t >= 2) ? "High sepsis risk (>=2)" : "Low risk (<2)";
        show("qSOFA", "Score", t, n);
    }

    void childPugh() {
        int enc = askInt("Encephalopathy (1-3):", 1);
        int asc = askInt("Ascites (1-3):", 1);
        int bili = askInt("Bilirubin (1-3):", 1);
        int alb = askInt("Albumin (1-3):", 1);
        int inr = askInt("INR (1-3):", 1);
        if (enc<1||enc>3||asc<1||asc>3||bili<1||bili>3||alb<1||alb>3||inr<1||inr>3) { DM.notify("1-3 each", C_ERR); return; }
        int t = enc + asc + bili + alb + inr;
        const char* cls = (t <= 6) ? "Class A (5-6)" : (t <= 9) ? "Class B (7-9)" : "Class C (10-15)";
        show("Child-Pugh", "Score", t, cls);
    }

    void meld() {
        char b[16];
        strcpy(b, "1.0"); if (!DM.inputLine("Bilirubin mg/dL:", b, sizeof(b))) return;
        double bili = atof(b); if (bili < 1.0) bili = 1.0;
        strcpy(b, "1.0"); if (!DM.inputLine("INR:", b, sizeof(b))) return;
        double inr = atof(b); if (inr < 1.0) inr = 1.0;
        strcpy(b, "1.0"); if (!DM.inputLine("Creatinine mg/dL:", b, sizeof(b))) return;
        double cr = atof(b);
        if (cr < 1.0) cr = 1.0;
        if (cr > 4.0) cr = 4.0;
        double score = 3.78*log(bili) + 11.2*log(inr) + 9.57*log(cr) + 6.43;
        if (score < 6) score = 6;
        if (score > 40) score = 40;
        const char* n = (score < 10) ? "Low (3mo mort ~2%)"
                      : (score < 20) ? "Moderate (~6-20%)"
                      : (score < 30) ? "High (~25-50%)"
                      : "Very high (>50%)";
        show("MELD", "Score", score, n);
    }
};

MedicalScores gMedScores;
class MedicalHeme {
public:
    void run() {
        const char* items[] = {
            "ANC (absolute neutrophil count)",
            "MCV / MCH / MCHC",
            "Mentzer index",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Hematology", items, 4);
        if (s < 0 || s == 3) return;
        switch (s) { case 0: anc(); break; case 1: rbcIdx(); break; case 2: mentzer(); break; }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show3(const char* t, const char* l1, double v1, const char* l2, double v2, const char* l3, double v3) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[48]; char vs[24];
        formatNum(v1, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        formatNum(v2, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l2, vs); d.drawString(l, 4, HDR_H + 24);
        formatNum(v3, vs, sizeof(vs)); snprintf(l, sizeof(l), "%s = %s", l3, vs); d.drawString(l, 4, HDR_H + 40);
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("heme");
        }
    }

    void anc() {
        double wbc = ask("WBC (x10^3/uL):", "5.0"); if (isnan(wbc) || wbc <= 0) return;
        double neut = ask("% neutrophils:", "60"); if (isnan(neut) || neut < 0 || neut > 100) return;
        double bands = ask("% bands:", "0"); if (isnan(bands) || bands < 0 || bands > 100) return;
        double a = wbc * (neut + bands) * 10.0;
        show3("ANC", "ANC", a, "WBC", wbc, "Neut+Bands%", neut+bands);
    }

    void rbcIdx() {
        double hb = ask("Hb (g/dL):", "14"); if (isnan(hb) || hb <= 0) return;
        double hct = ask("Hct (%):", "42"); if (isnan(hct) || hct <= 0) return;
        double rbc = ask("RBC (x10^6/uL):", "5.0"); if (isnan(rbc) || rbc <= 0) return;
        double mcv = hct * 10.0 / rbc;
        double mch = hb * 10.0 / rbc;
        double mchc = hb * 100.0 / hct;
        show3("RBC Indices", "MCV", mcv, "MCH", mch, "MCHC", mchc);
    }

    void mentzer() {
        double mcv = ask("MCV (fL):", "80"); if (isnan(mcv) || mcv <= 0) return;
        double rbc = ask("RBC (x10^6/uL):", "5.0"); if (isnan(rbc) || rbc <= 0) return;
        double mi = mcv / rbc;
        show3("Mentzer Index", "MI", mi, "MCV", mcv, "RBC", rbc);
    }
};

MedicalHeme gMedHeme;
class MedicalPulm {
public:
    void run() {
        const char* items[] = {
            "P/F ratio (ARDS)",
            "A-a gradient",
            "Oxygenation index (OI)",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Pulmonology", items, 4);
        if (s < 0 || s == 3) return;
        switch (s) { case 0: pfRatio(); break; case 1: aaGrad(); break; case 2: oxIdx(); break; }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[48]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); d.drawString(n, 4, HDR_H + 28); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) { DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S')) DM.saveBMPAuto("pulm");
        }
    }

    void pfRatio() {
        double pao2 = ask("PaO2 (mmHg):", "80"); if (isnan(pao2) || pao2 <= 0) return;
        double fio2 = ask("FiO2 (0-1):", "0.21"); if (isnan(fio2) || fio2 <= 0) return;
        double pf = pao2 / fio2;
        const char* n = (pf < 100) ? "Severe ARDS (<100)"
                      : (pf < 200) ? "Moderate ARDS (100-200)"
                      : (pf < 300) ? "Mild ARDS (200-300)"
                      : "Normal (>300)";
        show("P/F Ratio", "PF", pf, n);
    }

    void aaGrad() {
        double pao2 = ask("PaO2:", "80");  if (isnan(pao2) || pao2 <= 0) return;
        double paco2 = ask("PaCO2:", "40"); if (isnan(paco2) || paco2 <= 0) return;
        double fio2 = ask("FiO2 (0-1):", "0.21"); if (isnan(fio2) || fio2 <= 0 || fio2 > 1) return;
        double age = ask("Age (y):", "40"); if (isnan(age) || age <= 0) return;
        double pao2alv = fio2 * 713 - paco2 / 0.8;
        double grad = pao2alv - pao2;
        double expNorm = age / 4.0 + 4;
        char nb[40]; snprintf(nb, sizeof(nb), "Exp normal = %.1f", expNorm);
        show("A-a Gradient", "Aa", grad, nb);
    }

    void oxIdx() {
        double map = ask("Mean airway P:", "10"); if (isnan(map) || map <= 0) return;
        double fio2 = ask("FiO2 (0-1):", "0.5"); if (isnan(fio2) || fio2 <= 0 || fio2 > 1) return;
        double pao2 = ask("PaO2:", "60"); if (isnan(pao2) || pao2 <= 0) return;
        double oi = (map * fio2 * 100.0) / pao2;
        const char* n = (oi > 25) ? "Severe (>25)" : (oi > 15) ? "Moderate (15-25)" : "Mild (<15)";
        show("Oxygenation Index", "OI", oi, n);
    }
};

MedicalPulm gMedPulm;
class MedicalCardio {
public:
    void run() {
        const char* items[] = {
            "MAP (mean arterial pressure)",
            "Pulse pressure",
            "Rate-pressure product",
            "Shock index",
            "Back"
        };
        for (;;) {
        int s = DM.menu("Cardiology / Vitals", items, 5);
        if (s < 0 || s == 4) return;
        switch (s) {
            case 0: mapCalc();  break;
            case 1: ppCalc();   break;
            case 2: rppCalc();  break;
            case 3: shockIdx(); break;
        }
        }
    }
private:
    double ask(const char* p, const char* d) {
        char b[24]; strncpy(b, d, 24); b[23] = 0;
        if (!DM.inputLine(p, b, sizeof(b))) return NAN;
        return atof(b);
    }

    void show(const char* t, const char* l1, double v1,
              const char* l2, double v2, const char* n) {
        auto& d = M5Cardputer.Display;
        d.fillScreen(C_BG); DM.header(t); DM.footer("Y:Ans S:BMP","","ESC:back");
        d.setTextColor(C_OK, C_BG);
        char l[48]; char vs[24];
        formatNum(v1, vs, sizeof(vs));
        snprintf(l, sizeof(l), "%s = %s", l1, vs); d.drawString(l, 4, HDR_H + 8);
        if (l2 && *l2) {
            formatNum(v2, vs, sizeof(vs));
            snprintf(l, sizeof(l), "%s = %s", l2, vs); d.drawString(l, 4, HDR_H + 24);
        }
        if (n && *n) { d.setTextColor(C_ACC2, C_BG); d.drawString(n, 4, HDR_H + 40); }
        G.ans = v1; G.ansComplex = false;
        while (true) {
            if (!DM.pollKeys()) { delay(15); continue; }
            auto st = M5Cardputer.Keyboard.keysState();
            if (IS_ESC) return;
            if (M5Cardputer.Keyboard.isKeyPressed('y')||M5Cardputer.Keyboard.isKeyPressed('Y')) {
                DM.notify("-> Ans", C_OK); return; }
            if (M5Cardputer.Keyboard.isKeyPressed('s')||M5Cardputer.Keyboard.isKeyPressed('S'))
                DM.saveBMPAuto("cardio");
        }
    }

    void mapCalc() {
        double sbp = ask("SBP:", "120"); if (isnan(sbp) || sbp <= 0) return;
        double dbp = ask("DBP:", "80");  if (isnan(dbp) || dbp <= 0) return;
        if (dbp > sbp) { DM.notify("DBP must be <= SBP", C_ERR); return; }
        double m = (sbp + 2*dbp) / 3.0;
        const char* n = (m < 65) ? "LOW (<65) hypoperfusion"
                      : (m > 100) ? "HIGH (>100)"
                      : "Normal (70-100)";
        show("MAP", "MAP", m, "SBP", sbp, n);
    }

    void ppCalc() {
        double sbp = ask("SBP:", "120"); if (isnan(sbp) || sbp <= 0) return;
        double dbp = ask("DBP:", "80");  if (isnan(dbp) || dbp <= 0) return;
        if (dbp > sbp) { DM.notify("DBP must be <= SBP", C_ERR); return; }
        show("Pulse Pressure", "PP", sbp-dbp, "SBP-DBP", sbp-dbp, "");
    }

    void rppCalc() {
        double hr = ask("HR (bpm):", "70"); if (isnan(hr) || hr <= 0) return;
        double sbp = ask("SBP:", "120");    if (isnan(sbp) || sbp <= 0) return;
        double v = hr * sbp;
        const char* n = (v > 20000) ? "HIGH (>20000)" : "Normal";
        show("Rate-Pressure Product", "RPP", v, "HR*SBP", v, n);
    }

    void shockIdx() {
        double hr = ask("HR:", "80");  if (isnan(hr) || hr <= 0) return;
        double sbp = ask("SBP:", "120"); if (isnan(sbp) || sbp <= 0) return;
        double si = hr / sbp;
        const char* n = (si >= 0.9) ? "Elevated (>=0.9)" : "Normal (<0.9)";
        show("Shock Index", "SI", si, "HR/SBP", si, n);
    }
};

MedicalCardio gMedCardio;
class MedicalCalc {
public:
    void run() {
        while (true) {
            const char* top[] = {
                "Clinical Calculators",
                "Epidemiology & Statistics",
                "Back"
            };
            int s = DM.menu("Medical Calculator", top, 3);
            if (s < 0 || s == 2) return;
            if (s == 0) clinicalMenu();
            else         epiMenu();
        }
    }
private:
    void clinicalMenu() {
        while (true) {
            const char* items[] = {
                "Cardiology / Vitals (MAP/PP/RPP/SI)",
                "Pulmonology (P/F, A-a, OI)",
                "Hematology (ANC, RBC idx, Mentzer)",
                "Clinical Scores (GCS/CURB/qSOFA/Child/MELD)",
                "Body Composition (BMI/BSA/IBW/ABW)",
                "Renal Function (eGFR/CrCl)",
                "Lytes / Acid-Base (Ca/Na/AG/Osm)",
                "Endocrine (HOMA-IR / Free T4)",
                "Obstetrics (EDD / GA)",
                "Pharmacokinetics (t1/2, LD/MD)",
                "Back"
            };
            int s = DM.menu("Clinical Calculators", items, 11);
            if (s < 0 || s == 10) return;
            switch (s) {
                case 0: gMedCardio.run(); break;
                case 1: gMedPulm.run();   break;
                case 2: gMedHeme.run();   break;
                case 3: gMedScores.run(); break;
                case 4: gMedBody.run();   break;
                case 5: gMedRenal.run();  break;
                case 6: gMedLytes.run();  break;
                case 7: gMedEndo.run();   break;
                case 8: gMedOB.run();     break;
                case 9: gMedPK.run();     break;
            }
        }
    }

    void epiMenu() {
        while (true) {
            const char* items[] = {
                "Survival Analysis (KM/Log-rank/Cox)",
                "Meta-Analysis (Forest/Funnel/Egger)",
                "Outbreak / Source Trace",
                "Risk / Odds Ratio (2x2)",
                "Confidence Intervals (mean/prop)",
                "Back"
            };
            int s = DM.menu("Epidemiology & Statistics", items, 6);
            if (s < 0 || s == 5) return;
            switch (s) {
                case 0: gSurvival.run();               break;
                case 1: gMeta.run();                   break;
                case 2: gStatStrata.medicalOutbreak(); break;
                case 3: gStatNonpar.medicalRiskMenu(); break;
                case 4: gStatNonpar.medicalCIMenu();   break;
            }
        }
    }
};

MedicalCalc gMedical;
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
        G.autoOffMin = p.getInt("autooff", 10);
        String ssid  = p.getString("wifissid", "");
        String pass  = p.getString("wifipass", "");
        strncpy(G.wifiSSID, ssid.c_str(), 32);
        strncpy(G.wifiPASS, pass.c_str(), 32);
        gUserFns.clear();
        String fns = p.getString("userfns", "");
        int fstart = 0;
        while (fstart < (int)fns.length()) {
            int nl2 = fns.indexOf('\n', fstart);
            if (nl2 < 0) nl2 = fns.length();
            String line = fns.substring(fstart, nl2);
            fstart = nl2 + 1;
            int t1 = line.indexOf('\t');
            if (t1 < 0) continue;
            int t2 = line.indexOf('\t', t1 + 1);
            if (t2 < 0) continue;
            String nm = line.substring(0, t1);
            String prm = line.substring(t1 + 1, t2);
            String bd = line.substring(t2 + 1);
            if (nm.length() > 0 && nm.length() < 12 && prm.length() == 1 && bd.length() > 0 && bd.length() < 64)
                gUserFns.add(nm.c_str(), prm[0], bd.c_str());
        }
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
        p.putInt("autooff", G.autoOffMin);
        p.putString("wifissid", G.wifiSSID);
        p.putString("wifipass", G.wifiPASS);
        String fns = "";
        for (int i = 0; i < gUserFns.n; i++) {
            if (!gUserFns.f[i].active) continue;
            fns += gUserFns.f[i].name;
            fns += "\t";
            fns += (char)gUserFns.f[i].param;
            fns += "\t";
            fns += gUserFns.f[i].body;
            fns += "\n";
        }
        p.putString("userfns", fns);
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
        if (!labels) { WiFi.scanDelete(); DM.notify("malloc fail", C_ERR); return; }
        for (int i = 0; i < n; i++) {
            labels[i] = (char*)malloc(48); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); WiFi.scanDelete(); DM.notify("malloc fail", C_ERR); return; }
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
        strncpy(G.wifiSSID, ssid, 32); G.wifiSSID[32] = 0;
        strncpy(G.wifiPASS, pass, 32); G.wifiPASS[32] = 0;
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
                if (IS_ESC) {
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
                if (IS_ESC) return;
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
        M4[c] += t1*dn2*((double)n*n - 3.0*(double)n + 3.0) + 6.0*dn2*M2[c] - 4.0*dn*M3[c];
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
                if (IS_ESC) return;
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
    auto drawAbout = [&]() {
        d.fillScreen(C_BG); DM.header("About"); DM.footer("S:splash","","ESC:back");
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
    };
    drawAbout();
    while (true) {
        if (!DM.pollKeys()) { delay(15); continue; }
        auto st = M5Cardputer.Keyboard.keysState();
        if (IS_ESC) return;
        if (M5Cardputer.Keyboard.isKeyPressed('s') || M5Cardputer.Keyboard.isKeyPressed('S')) {
            DM.splash();
            drawAbout();
        }
    }
}

void statSubMenu() {
    while (true) {
        const char* items[] = {
            "Lists & Descriptive","Regression / Multi","Tests (z/t/F/ANOVA/chi)",
            "Non-parametric","Distributions","Multivariate (PCA/k-means/etc)",
            "Stratified (M-H)","17 Statistical Graphs","CSV Stream Stats (SD)","Back"
        };
        int s = DM.menu("Statistics", items, 10);
        if (s < 0 || s == 9) return;
        switch (s) {
            case 0: gStatCore.run();    break;
            case 1: gStatRegress.run(); break;
            case 2: gStatTests.run();   break;
            case 3: gStatNonpar.run();  break;
            case 4: gDistBrowser.run(); break;
            case 5: gStatMV.run();      break;
            case 6: gStatStrata.run();  break;
            case 7: gStatGraphs.run();  break;
            case 8: gCsvStream.run();   break;
        }
    }
}

void settingsScreen();
void listUserFns() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(C_BG); DM.header("User Functions"); DM.footer("","","ESC:back");
    d.setTextColor(C_FG, C_BG);
    int y = HDR_H + 2;
    if (gUserFns.n == 0) { d.drawString("(none)", 4, y); }
    for (int i = 0; i < gUserFns.n && y < LCD_H - FTR_H - 4; i++) {
        if (!gUserFns.f[i].active) continue;
        char l[72];
        snprintf(l, sizeof(l), "%s(%c) = %s", gUserFns.f[i].name, gUserFns.f[i].param, gUserFns.f[i].body);
        d.drawString(l, 4, y);
        y += 11;
    }
    DM.waitEsc();
}

void exportUserFns() {
    if (!G.sdOK) { DM.notify("No SD", C_ERR); return; }
    SdGuard guard;
    File f = SD.open("/userfns.txt", FILE_WRITE);
    if (!f) { DM.notify("Open fail", C_ERR); return; }
    for (int i = 0; i < gUserFns.n; i++) {
        if (!gUserFns.f[i].active) continue;
        f.print(gUserFns.f[i].name);
        f.print('\t');
        f.print((char)gUserFns.f[i].param);
        f.print('\t');
        f.println(gUserFns.f[i].body);
    }
    f.close();
    char m[32]; snprintf(m, sizeof(m), "Exported %d fns", gUserFns.n);
    DM.notify(m, C_OK, 1200);
}

void importUserFns(bool replaceAll) {
    if (!G.sdOK) { DM.notify("No SD", C_ERR); return; }
    SdGuard guard;
    File f = SD.open("/userfns.txt");
    if (!f) { DM.notify("Not found", C_ERR); return; }
    if (replaceAll) gUserFns.clear();
    int added = 0;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;
        int t1 = line.indexOf('\t');
        if (t1 < 0) continue;
        int t2 = line.indexOf('\t', t1 + 1);
        if (t2 < 0) continue;
        String nm = line.substring(0, t1);
        String prm = line.substring(t1 + 1, t2);
        String bd = line.substring(t2 + 1);
        if (nm.length() > 0 && nm.length() < 12 && prm.length() == 1 && bd.length() > 0 && bd.length() < 64) {
            gUserFns.remove(nm.c_str());
            if (gUserFns.add(nm.c_str(), prm[0], bd.c_str())) added++;
        }
    }
    f.close();
    char m[40]; snprintf(m, sizeof(m), "Imported %d fns", added);
    DM.notify(m, C_OK, 1200);
}

void deleteUserFn() {
    if (gUserFns.n == 0) { DM.notify("None", C_DIM); return; }
    char** labels = (char**)malloc(gUserFns.n * sizeof(char*));
    if (!labels) { DM.notify("malloc fail", C_ERR); return; }
    for (int i = 0; i < gUserFns.n; i++) {
        labels[i] = (char*)malloc(40); if (!labels[i]) { for (int k = 0; k < i; k++) free(labels[k]); free(labels); return; }
        snprintf(labels[i], 40, "%s(%c)", gUserFns.f[i].name, gUserFns.f[i].param);
    }

    const char** clabels = (const char**)labels;
    int s = DM.menu("Delete which?", clabels, gUserFns.n);
    for (int i = 0; i < gUserFns.n; i++) free(labels[i]);
    free(labels);
    if (s >= 0 && s < gUserFns.n) {
        char nm[12]; strncpy(nm, gUserFns.f[s].name, 11); nm[11] = 0;
        gUserFns.remove(nm);
        DM.notify("Deleted", C_OK);
    }
}

void userFuncsMenu() {
    while (true) {
        const char* items[] = {
            "List functions",
            "SD export (/userfns.txt)",
            "SD import (merge)",
            "SD import (replace)",
            "Delete one",
            "Clear all",
            "Back"
        };
        int s = DM.menu("User Functions", items, 7);
        if (s < 0 || s == 6) return;
        switch (s) {
            case 0: listUserFns(); break;
            case 1: exportUserFns(); break;
            case 2: importUserFns(false); break;
            case 3: importUserFns(true); break;
            case 4: deleteUserFn(); break;
            case 5: if (DM.confirm("Clear all user fns?")) { gUserFns.clear(); DM.notify("Cleared", C_OK); } break;
        }
    }
}

void toolsMenu() {
    while (true) {
        const char* items[] = {
            "Spreadsheet (45x26)","uLisp Interpreter","Wi-Fi/Maxima (srv)",
            "User Functions (NVS/SD)",
            "Power / About","Settings (NVS persistent)","Save BMP screenshot now","Back"
        };
        int s = DM.menu("Tools", items, 8);
        if (s < 0 || s == 7) return;
        switch (s) {
            case 0: gSheet.run();      break;
            case 1: gULisp.run();      break;
            case 2: gWiFi.run();       break;
            case 3: userFuncsMenu();   break;
            case 4: gPwr.show();       break;
            case 5: settingsScreen();  break;
            case 6: DM.saveBMPAuto("screen"); break;
        }
    }
}

void settingsScreen() {
    while (true) {
        const char* items[] = {
            "Angle: DEG/RAD/GRAD","Display: Norm/Fix/Sci/Eng","Display digits",
            "Auto power off (min)","Save settings now","Reset all (clear NVS)","Back"
        };
        int s = DM.menu("Settings", items, 7);
        if (s < 0 || s == 6) return;
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
        else if (s == 3) {
            char b[8]; snprintf(b, sizeof(b), "%d", G.autoOffMin);
            if (DM.inputLine("auto off min (0=off):", b, sizeof(b))) {
                int v = atoi(b);
                if (v >= 0 && v <= 600) G.autoOffMin = v;
            }
        }
        else if (s == 4) { gNVS.save(); DM.notify("Saved to NVS", C_OK); }
        else if (s == 5) {
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
        "Calculator","CAS","Statistics","Medical Calculator","Scientific Reference",
        "Graph","Tools (Sheet/uLisp/WiFi)","About"
    };
    int s = DM.menu(POMAS_NAME " " POMAS_VER, items, 8);
    gLastInputT = millis();
    switch (s) {
        case 0: gCalc.run();       break;
        case 1: gCAS.run();        break;
        case 2: statSubMenu();     break;
        case 3: gMedical.run();    break;
        case 4: gSciRef.run();     break;
        case 5: gGraph.run();      break;
        case 6: toolsMenu();       break;
        case 7: aboutScreen();     break;
        default: break;
    }
    if (G.autoOffMin > 0 && millis() - gLastInputT > (unsigned long)G.autoOffMin * 60000UL) {
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
