# 第21章 チートシート集

[← 第20章 uLisp](20_ulisp.md) | [目次](#/) | [第22章 FAQ →](22_faq.md)

---

## 21.1 この章の使い方

この章は **全モードの要点を 1 ページに集約** したチートシートです。マニュアルを読む時間がないとき、操作を忘れたとき、素早く確認したいときに使います。

### 対象読者

- 既に PoMaS の基本操作を覚えた方
- 各機能の要点だけを素早く確認したい方
- マニュアルの代替として、要点集だけ手元に置きたい方

---

## 21.2 起動・終了・基本操作

### 起動と終了

```
起動: 電源ボタン 2 秒長押し → スプラッシュ 2.5 秒 → メインメニュー
終了: 電源ボタン 2 秒長押し
スリープ: 無操作 10 分で自動電源 OFF（設定可）
減光: 無操作 60 秒で自動減光、任意キーで復帰
```

### メニュー操作

| キー | 動作 |
|---|---|
| `;` / `Fn+;` | 上移動 |
| `.` / `Fn+.` | 下移動 |
| `1` 〜 `9` | 該当番号へ |
| `a` 〜 `z` | 10 番目以降へ |
| `Enter` | 決定 |
| `` ` ``（ESC） | 戻る |
| `Fn+ESC` | 戻る（代替） |
| `Fn+,` `Fn+/` | ページ送り |

### 入力ダイアログ

| キー | 動作 |
|---|---|
| 文字キー | 入力 |
| `Enter` | 確定 |
| `Del` | 1 文字削除 |
| `Fn+Del` | 全消去 |
| `Fn+←→` | カーソル移動 |
| `Fn+↑↓` | 先頭/末尾 |
| `ESC` | キャンセル |

---

## 21.3 Calculator モード チートシート

### 基本入力

```
四則演算: 2+3, 10-4, 6*7, 20/4
べき乗: 2^10
階乗: 5!
剰余: 10%3 または mod(10,3)
括弧: (2+3)*4
```

### 主要関数（35 種）

| 分類 | 関数 |
|---|---|
| 三角 | sin, cos, tan, asin, acos, atan, csc, sec, cot |
| 双曲線 | sinh, cosh, tanh, asinh, acosh, atanh |
| 指数 | exp |
| 対数 | ln, log, log10, log2, logn |
| 累乗根 | sqrt, cbrt, nrt(n,x), pow(a,b) |
| 丸め | abs, sign, sgn, floor, ceil, round, int, frac |
| 組合せ | nCr, nPr, fact |
| 素数 | isPrime, nextPrime, prevPrime |
| その他 | hypot, min, max, mod, gcd, lcm, clamp, if, dms, atan2 |
| 乱数 | random, randInt, randN, randR |
| 温度 | toF, toC, toK |
| 角度 | deg, rad |

### 定数

`pi`, `e`, `euler`, `phi`, `gamma`, `inf`, `nan`, `ans`, `mem`

### 変数・メモリ

```
変数 A〜J: Fn+4 で設定、A〜J で参照
メモリ M: Fn+3 で操作（M+/M-/MR/MC）
Ans: 直前の計算結果
Ans バンク: Fn+b で最大 20 件保存
```

### ショートカット

| キー | 機能 |
|---|---|
| `Fn+1` | 設定 |
| `Fn+2` | 履歴 |
| `Fn+3` | メモリ M |
| `Fn+4` | 変数 A〜J |
| `Fn+5` | 基数変換 |
| `Fn+6` | ビット演算 |
| `Fn+7` | 複素数 |
| `Fn+8` | 拡張ツール |
| `Fn+9` | RPN モード |
| `Fn+0` | ヘルプ |
| `Fn+=` | 分数⇄小数 |
| `Fn+a` | Ans 挿入 |
| `Fn+b` | Ans バンク |
| `Fn+,` `Fn+/` | 表示桁数 ±1 |

---

## 21.4 CAS モード チートシート（34 コマンド）

### 代数

| コマンド | 例 |
|---|---|
| `Factor(p)` | 因数分解 |
| `cFactor(p)` | 複素数範囲 |
| `Expand(p)` | 展開 |
| `completeSquare(p)` | 平方完成 |
| `getNum(f)` | 分子 |
| `getDenom(f)` | 分母 |
| `propFrac(f)` | 帯分数化 |
| `comDenom(expr)` | 通分 |
| `Left(eq)` | 等式左辺 |
| `Right(eq)` | 等式右辺 |

### 三角・式変換

| コマンド | 例 |
|---|---|
| `tExpand(sin(A+B))` | 加法定理 |
| `tCollect(sin(x)^2)` | 縮約 |
| `toLn(2^x)` | 対数変換 |
| `toExp(exp(x))` | 指数統一 |
| `toSin(cos(x))` | sin へ |
| `toCos(sin(x))` | cos へ |
| `toLogbase(a,b)` | 底変換 |

### 微積分

| コマンド | 例 |
|---|---|
| `Derivative(f,x)` | 微分 |
| `Taylor(f,x0,n)` | テイラー展開 |
| `Series(f,x0,n)` | 級数展開 |
| `LeadingTerm(p)` | 主要項 |
| `Tangent(f,a)` | 接線 |
| `Normal(f,a)` | 法線 |
| `ArcLength(f,a,b)` | 弧長 |
| `DiffQ(f,a,h)` | 差分商 |
| `fMin(f,a,b)` / `fMax` | 極値探索 |

### 方程式・ODE

| コマンド | 例 |
|---|---|
| `cSolve(eq)` | 方程式（複素解含む） |
| `cZeros(expr)` | 零点 |
| `deSolve(...)` | 1 階 ODE |
| `deSolve2(a,b)` | 2 階 ODE |
| `RK4(f,x0,y0,xe,h)` | RK4 数値解 |
| `ImpDiff(F(x,y))` | 陰微分 |

---

## 21.5 Statistics モード チートシート

### 記述統計

| 指標 | 意味 |
|---|---|
| n | データ件数 |
| sum | 合計 |
| mean | 平均 |
| sd(samp) / sd(pop) | 標本/母標準偏差 |
| var(samp) | 標本分散 |
| SE | 標準誤差 |
| CV | 変動係数 |
| min/max | 最小/最大 |
| Q1/med/Q3 | 四分位数 |
| IQR | 四分位範囲 |
| skew / kurt | 歪度 / 尖度 |
| mode | 最頻値 |

### 回帰（9 種）

Linear / Quadratic / Power / Exponential / Logarithmic / Logistic / Med-Med / Multiple / Stepwise

### 検定（11 種）

| 検定 | 用途 |
|---|---|
| z-test (1-sample) | σ 既知の 1 標本 |
| t-test (1-sample) | σ 未知の 1 標本 |
| t-test (2-sample eq) | 等分散 2 標本 |
| Welch's t-test | 非等分散 2 標本 |
| Paired t-test | 対応あり |
| F-test | 分散比較 |
| One-way ANOVA | 3 群以上 |
| Two-way ANOVA | 2 因子 |
| Chi-Square GOF | 適合度 |
| Chi-Square Independence | 独立性 |
| Fisher's exact | 2×2 小標本 |

### ノンパラ（7 種）

Mann-Whitney U / Wilcoxon / Kruskal-Wallis / Friedman / Sign test / Spearman / Kendall τ

### リスク指標

OR（オッズ比）/ RR（リスク比）/ RD（リスク差）/ NNT / CI（信頼区間）

### 分布（7 種）

Normal N(μ,σ) / Student t(df) / Chi-Square(df) / F(d1,d2) / Binomial(n,p) / Poisson(λ) / Geometric(p)

### 多変量（5 種）

PCA / Factor Analysis / k-means / Hierarchical / Decision Tree

### 層別解析

MH-OR / MH-RR / Breslow-Day / Cochran's Q / Outbreak trace

### 生存分析

Kaplan-Meier / Log-rank / Cox PH / Median survival

### メタアナリシス

IV / DL / Forest / Funnel / Egger

### 統計グラフ（17 種）

Histogram / Box / Scatter / Q-Q / Bar / Pie / Line / Stem-Leaf / Pareto / ECDF / KDE / Strip / Cumulative / Mean±SD / Residuals / P-P / Multi-line

---

## 21.6 Graph モード チートシート

### プロットタイプ（7 種）

| タイプ | 入力例 |
|---|---|
| y = f(x) | `sin(x)` |
| Parametric | `cos(t);sin(t)` |
| Polar | `1+cos(t)` |
| Implicit | `x^2+y^2-4` |
| Inequality | `y-x`（y > x） |
| Sequence | `1/n` |
| 3D | `sin(x)*cos(y)` |

### プロット中のキー

| キー | 動作 |
|---|---|
| `Z` | ズームイン |
| `X` | ズームアウト |
| `R` | 標準ウィンドウ |
| `T` | トレース ON/OFF |
| `B` | ボックスズーム |
| `+` / `n` | 次関数 |
| `S` | BMP 保存 |
| `Fn+←→↑↓` | 3D 回転 |
| `ESC` | 戻る |

### G-Solve

| 番号 | 機能 |
|---|---|
| 1 | Root（根） |
| 2 | Maximum（極大） |
| 3 | Minimum（極小） |
| 4 | Intersection（交点） |
| 5 | y at x |
| 6 | x at y |
| 7 | Definite integral |

### 便利な範囲

- 標準: -10〜10
- 三角: -2π〜2π、-1.5〜1.5
- Square aspect: 縦横比 1:1（真円表示）

---

## 21.7 SciRef モード チートシート

| メニュー | 件数 | 内容 |
|---|---|---|
| Constants | 386 | 物理・数学・EE・天文・惑星など 11 カテゴリ |
| Scientific Laws | 300 | 物理・化学・数学・地球科学など 7 カテゴリ |
| Unit Converter | 532 | 長さ・面積・体積・質量など 20 カテゴリ |
| Periodic Table | 118 | 全元素 |
| Search | — | シンボル/名前検索 |

### 定数カテゴリ（11）

Physical (38) / Math (28) / EE (40) / Astronomy (12) / Planet (64) / DwarfMoon (32) / Geo (23) / Atmos (11) / Ocean (4) / Element (130) / Biology (30)

### 法則カテゴリ（7）

Physics (100) / Chemistry (70) / Math (50) / GeoSci (30) / Astronomy (20) / Biology (20) / EE (11)

### 単位カテゴリ（20）

Length / Area / Volume / Mass / Velocity / Acceleration / Force / Pressure / Energy / Power / Temperature / Angle / Time / Digital Info / Frequency / Flow / Density / Viscosity / Radiation / Misc

### 操作

- `Y`: Ans へ転送
- `B`: Ans バンクへ追加
- `S`: BMP 保存
- `Enter`: 詳細表示

---

## 21.8 Medical Calculator チートシート

### Clinical Calculators（10 領域）

| 領域 | 関数 |
|---|---|
| Cardiology | MAP / PP / RPP / Shock Index |
| Pulmonology | P/F / A-a / OI |
| Hematology | ANC / MCV·MCH·MCHC / Mentzer |
| Clinical Scores | GCS / CURB-65 / qSOFA / Child-Pugh / MELD |
| Body | BMI / BSA / IBW / ABW |
| Renal | CKD-EPI / Cockcroft-Gault / Schwartz |
| Lytes | Corrected Ca·Na / AG / Osm / Winter's / Delta / FWD |
| Endocrine | HOMA-IR / FTI |
| Obstetrics | Naegele / GA |
| PK | t1/2 / LD / MD / Dose |

### Epidemiology & Statistics（5 項目）

Survival / Meta-analysis / Outbreak / Risk-OR / CI

---

## 21.9 Tools モード チートシート

### Spreadsheet（45×26）

| 操作 | キー |
|---|---|
| セル移動 | `Fn+;. ,/` |
| 編集 | `Enter` |
| 削除 | `Del` |
| CSV 保存 | `S` |
| CSV 読込 | `L` |
| Sheet⇄L1..L8 | `T` |

### 数式例

`=A1+B1` / `=SUM(A1:A5)` / `=AVG(A1:A5)` / `=IF(A1>5, 100, 0)`

### uLisp

| 操作 | キー |
|---|---|
| REPL 開始 | メニュー 1 |
| 評価 | `Enter` |
| REPL 抜ける | `Fn+E` |
| Run GC | メニュー 3 |
| Reset | メニュー 4 |

### uLisp 主要関数

算術: `+ - * / mod`
リスト: `car cdr cons list append reverse nth length`
数学: `sin cos tan exp log sqrt abs floor ceil round expt pow`
統計: `sum mean sd`
高階: `map filter reduce apply`
特殊: `quote if cond and or define lambda let while`

### Wi-Fi

- 2.4GHz のみ対応
- `Tools → 3 → 1` で SSID/Pass
- `Tools → 3 → 3` で接続
- Maxima 連携はセルフホスト必須

### 設定

`Tools → 5` または `Fn+1`（電卓から）
- Angle: DEG/RAD/GRA
- Display: Norm/Fix/Sci/Eng
- Digits: 0〜9
- Auto power off: 0〜600 分

---

## 21.10 キーボード全ショートカット

### Fn + 数字（Calculator）

`1` 設定 / `2` 履歴 / `3` メモリ / `4` 変数 / `5` 基数 / `6` ビット / `7` 複素数 / `8` 拡張 / `9` RPN / `0` ヘルプ

### Fn + 記号

`=` 分数 / `,` 桁数+ / `/` 桁数- / `;` 上 / `.` 下

### Fn + 文字

`a` Ans 挿入 / `b` Ans バンク / `E` REPL 抜ける

### 全モード

`Fn+ESC` 戻る / `ESC` 戻る

### Graph プロット中

`Z` `X` `R` `T` `B` `+` `S`

---

## 21.11 トラブル別ショートカット

### 画面が暗い

→ 任意キー押下で復帰（減光中）

### メニューから戻れない

→ `` ` `` または `Fn+ESC`

### 計算結果が想定と違う

→ ヘッダのバッジ確認（DEG/RAD、NRM/FIX）

### uLisp のセル枯渇

→ `Tools → 2 → 3 (Run GC now)`

### SD カードが認識されない

→ `Tools → 4 → R (retry SD)`

### Wi-Fi が繋がらない

→ 2.4GHz AP か確認、パスワード再入力

### 設定が反映されない

→ `Settings → Save settings now`

### 電源が入らない

→ USB 充電 1.5-2 時間

---

## 21.12 この章のまとめ

- 全モードのチートシートを 1 章に集約
- Calculator: 35 関数 + 定数 + 変数 + ショートカット
- CAS: 34 コマンド完全リファレンス
- Statistics: 記述 + 回帰 + 検定 + ノンパラ + 分布 + 多変量 + 生存 + メタ
- Graph: 7 プロットタイプ + G-Solve + キー操作
- SciRef: 386 定数 + 300 法則 + 532 単位 + 118 元素
- Medical: 10 領域 40 関数
- Tools: Spreadsheet / uLisp / Wi-Fi / 設定
- 全キーボードショートカット一覧
- トラブル別ショートカット

### 確認問題

Q21-1: Calculator で階乗を計算するには？ → `5!` と入力

Q21-2: 複素数メニューのショートカットは？ → `Fn+7`

Q21-3: CAS の微分コマンドは？ → `Derivative(f,x)`

Q21-4: 統計の正規分布 CDF は？ → `Distributions → Normal → CDF`

Q21-5: 真円を表示するには？ → Square aspect (1:1)

Q21-6: 医学の GCS はどこ？ → `Medical → 1 → 4 → 1`

Q21-7: uLisp のセル枯渇の対処は？ → `Run GC now`

Q21-8: 設定画面を電卓から開くには？ → `Fn+1`

---

[← 第20章 uLisp](20_ulisp.md) | [目次](#/) | [第22章 FAQ →](22_faq.md)