# 第17章 付録

---

## この章で学ぶこと

- 完全キーアサイン表（9 モード別）
- 画面遷移図（メインメニューからの全経路）
- 関数索引（数学 70・CAS 34・uLisp 80）
- 略語集（医学・数学・ハード・通信）
- 参考文献（数学・統計・物理・化学・医学・プログラミング・ハード）
- 変更履歴詳細（v0.91 → v0.92）
- 用語集補足（PoMaS 内部特殊用語）
- 数値データ一覧（物理・数学・天文高精度値）
- あいうえお順索引・アルファベット順索引

---

## 17.1 組み込み関数一覧（Calculator）
### 三角関数
`sin, cos, tan, asin, acos, atan, csc, sec, cot`
### 双曲線関数
`sinh, cosh, tanh, asinh, acosh, atanh, csch, sech, coth`
### 指数・対数
`exp, ln, log, log10, log2, logn(a,b)`
### 累乗・根
`sqrt, cbrt, nrt(n,x), pow(a,b), ^`
### 丸め
`abs, sign, sgn, floor, ceil, round, int, frac`
### 組合せ・順列
`nCr(n,r), nPr(n,r), fact(n)`
### 素数
`isPrime(n), prime(n), nextPrime(n), prevPrime(n)`
### 乱数
`random(), randInt(n), randN(sd), randR(a,b)`
### その他
`gamma, lgamma, erf, erfc, deg, rad, hypot(a,b), min(a,b), max(a,b), mod(a,b), gcd(a,b), lcm(a,b), toF, toC, toK, if(c,t,f), clamp(x,lo,hi), dms(d,m,s), atan2(y,x)`
---
## 17.2 定数一覧
| 名前 | 値 | 用途 |
|---|---|---|
| `pi` | 3.14159... | 円周率 |
| `e` | 2.71828... | オイラー数 |
| `euler` | 2.71828... | オイラー数（エイリアス） |
| `phi` | 1.61803... | 黄金比 |
| `gamma` | 0.57721... | オイラー・マスケローニ |
| `inf` | ∞ | 無限大 |
| `nan` | NaN | 非数 |
| `ans` | 直前の結果 | — |
| `mem` | メモリ M | — |
---
## 17.3 キー早見表
### 共通キー
| キー | 動作 |
|---|---|
| `Fn + Esc` | 戻る / キャンセル |
| `Enter` | 決定 / 実行 |
| `Del` | バックスペース |
| `;` | 上 |
| `.` | 下 |
| `,` | 左 |
| `/` | 右 |
### Calculator ショートカット
| キー | 機能 |
|---|---|
| `Fn + 1` | 設定 |
| `Fn + 2` | 履歴 |
| `Fn + 3` | メモリ M |
| `Fn + 4` | 変数 A〜J |
| `Fn + 5` | 基数変換 |
| `Fn + 6` | ビット演算 |
| `Fn + 7` | 複素数 |
| `Fn + 8` | 拡張ツール |
| `Fn + 9` | RPN モード切替 |
| `Fn + 0` | ヘルプ |
| `Fn + =` | 分数 ⇄ 小数 |
| `Fn + a` | Ans 挿入 |
| `Fn + b` | Ans バンク |
| `Fn + ,` | 桁数増 |
| `Fn + /` | 桁数減 |
### Graph プロット中
| キー | 動作 |
|---|---|
| `Z` | ズームイン |
| `X` | ズームアウト |
| `R` | 標準ウィンドウ |
| `T` | トレース ON/OFF |
| `B` | ボックスズーム |
| `+` | 次関数切替 |
| `S` | BMP 保存 |
### その他画面
| キー | 動作 |
|---|---|
| `S` | BMP 保存 |
| `Y` | Ans に転送 |
| `B` | Ans バンクに追加 |
| `R` | SD 再初期化（Power 画面） |
| `T` | SD テスト / Sheet⇄L1..L8 |
---
## 17.4 記号一覧
### 数学記号
| 記号 | 意味 |
|---|---|
| `+` | 加算 |
| `-` | 減算 |
| `*` | 乗算 |
| `/` | 除算 |
| `^` | 累乗 |
| `!` | 階乗 |
| `%` | 剰余（または百分率） |
| `(` `)` | 括弧 |
| `,` | 引数区切り |
| `;` | パラメトリック区切り |
### 言語記号（μLisp）
| 記号 | 意味 |
|---|---|
| `(` `)` | リスト・関数呼び出し |
| `'` | quote（評価抑止） |
| `#t` | 真 |
| `#f` | 偽（または `nil`） |
---
## 17.5 データベース一覧
### 科学定数（386 件）
| カテゴリ | 件数 |
|---|---|
| Physical | 38 |
| Math | 8 |
| EE | 40 |
| Astronomy | 12 |
| Planet | 64 |
| DwarfMoon | 32 |
| Geo | 23 |
| Atmos | 11 |
| Ocean | 4 |
| Element | 124 |
| Biology | 30 |
### 科学法則（300 件）
| カテゴリ | 件数 |
|---|---|
| Physics | 100 |
| Chemistry | 70 |
| Math | 50 |
| GeoSci | 30 |
| Astronomy | 20 |
| Biology | 20 |
| EE | 11 |
### 単位換算（532 単位）
| カテゴリ | 例 |
|---|---|
| Length | m, km, mile, 尺 |
| Area | m², 坪, acre |
| Volume | L, gallon, 合 |
| Mass | kg, lb, 貫 |
| Temperature | °C, °F, K |
| 他 15 カテゴリ | |
---
## 17.6 ファイル形式
### BMP（スクリーンショット）
| 項目 | 値 |
|---|---|
| サイズ | 240 × 135 px |
| 色深度 | 24 bit RGB |
| サイズ | 約 97 KB |
| 拡張子 | .bmp |
### CSV（スプレッドシート・統計）
```
A1,B1,C1
A2,B2,C2
```
カンマ区切り、改行で行区切り。
### μLisp スクリプト
```
(define ...)
(print ...)
```
拡張子 `.lsp`。
---
## 17.7 制限事項一覧
| 項目 | 制限 | 理由 |
|---|---|---|
| 統計リスト | 200 件 | PSRAM 非搭載 |
| μLisp セル | 2500 個 | RAM 制約 |
| μLisp Env | 128 個 | RAM 制約 |
| パーサ深度 | 200 段 | スタック保護 |
| 多項式次数 | 16 | バッファ制約 |
| 行列サイズ | 10×10 | メモリ |
| ベクトル次元 | 10 | メモリ |
| スプレッドシート | 45×26 | 画面・メモリ |
| 関数プロット | 4 本 | 画面 |
| Maxima 連携 | クライアント側のみ実装（送信ロジックのみ）。送信先サーバーは非同梱 | サーバー未実装 |
---
## 17.8 バージョン履歴
### v0.92（現行）
- Paper UI v2（淡ミント紙 + グラファイト）
- 89 件のバグ修正（Step 1〜7）
- GC マーク&スイープ、Env 寿命管理
- 引数ガード、パーサ深度制限
- 陰関数 Y-flip 修正
- Square aspect (1:1) 追加
- E 変数と e 定数の分離
### v0.91（過去）
- 初期実装
- 基本 6 モード搭載
---
## 17.9 参考リンク
- [GitHub リポジトリ](https://github.com/jpeqtx/PoMaS)
- [GitHub Pages](https://jpeqtx.github.io/PoMaS/)
- [M5Stack 公式](https://m5stack.com/)
- [M5Unified](https://github.com/m5stack/M5Unified)
- [M5GFX](https://github.com/m5stack/M5GFX)
---
## 17.10 ライセンス
MIT License
---
## 17.11 謝辞
- M5Stack ハードウェア
- M5Unified / M5GFX ライブラリ
- すべてのテスター・コントリビューター
---
**PoMaS v0.92 — Paper UI v2**
**Author: jpeqtx**

---

## 17.12 完全キーアサイン表

### 17.12.1 メインメニュー

| キー | 動作 |
|---|---|
| `1` 〜 `8` | 各モードへ直接ジャンプ |
| `Enter` | 選択中の項目を決定 |
| `;` / `Fn + ;` | 上移動 |
| `.` / `Fn + .` | 下移動 |
| `Fn + Esc` / `` ` `` | （メインメニューでは無効） |

### 17.12.2 Calculator モード

| キー | 動作 |
|---|---|
| `0` 〜 `9` | 数字入力 |
| `.` | 小数点 |
| `+ - * /` | 演算子 |
| `^` | 累乗 |
| `(` `)` | 括弧 |
| `,` | 引数区切り |
| `!` | 階乗 |
| `%` | 剰余 |
| `a` 〜 `z` | 変数名・関数名 |
| `A` 〜 `J` | 変数参照 |
| `Enter` | 評価実行 |
| `Del` | 1 文字削除 |
| `Fn + 1` | 設定 |
| `Fn + 2` | 履歴 |
| `Fn + 3` | メモリ M |
| `Fn + 4` | 変数 A〜J |
| `Fn + 5` | 基数変換 |
| `Fn + 6` | ビット演算 |
| `Fn + 7` | 複素数 |
| `Fn + 8` | 拡張ツール |
| `Fn + 9` | RPN モード切替 |
| `Fn + 0` | ヘルプ |
| `Fn + =` | 分数⇄小数 |
| `Fn + a` | Ans 挿入 |
| `Fn + b` | Ans バンク |
| `Fn + ,` | 表示桁数 +1 |
| `Fn + /` | 表示桁数 -1 |
| `Fn + Esc` | メインメニューへ |
| `<`（Shift+,） | カーソル左 |
| `>`（Shift+.） | カーソル右 |

### 17.12.3 CAS モード

| キー | 動作 |
|---|---|
| 数値・文字 | 式入力 |
| `Enter` | 評価実行 |
| `Del` | 1 文字削除 |
| `Y` | 結果を Ans へ |
| `Fn + Esc` / `` ` `` | CAS メニューへ |

### 17.12.4 Statistics モード

| キー | 動作 |
|---|---|
| `;` `.` `,` `/` | リスト内移動 |
| `Enter` | セル編集 |
| `Del` | セル削除 |
| `A` | 追加 |
| `S` | ソート |
| `Y` | 平均を Ans へ |
| `S` | BMP 保存 |
| `Fn + Esc` | メニューへ |

### 17.12.5 Graph プロット中

| キー | 動作 |
|---|---|
| `Z` | ズームイン |
| `X` | ズームアウト |
| `R` | 標準ウィンドウ |
| `T` | トレース ON/OFF |
| `B` | ボックスズーム |
| `+` または `n` | 次関数切替 |
| `S` | BMP 保存 |
| `Fn + ←` | 3D 方位角 - |
| `Fn + →` | 3D 方位角 + |
| `Fn + ↑` | 3D 仰角 + |
| `Fn + ↓` | 3D 仰角 - |
| `Fn + ;` | トレース上 |
| `Fn + .` | トレース下 |
| `Fn + ,` | トレース左 |
| `Fn + /` | トレース右 |
| `Fn + Esc` | Graph メニューへ |

### 17.12.6 SciRef モード

| キー | 動作 |
|---|---|
| `Y` | 値を Ans へ |
| `B` | Ans バンクへ追加 |
| `S` | BMP 保存 |
| `;` `.` `,` `/` | 周期表ナビ |
| `Enter` | 詳細表示 |
| `Fn + Esc` | SciRef メニューへ |

### 17.12.7 Tools - uLisp REPL

| キー | 動作 |
|---|---|
| 文字 | 式入力 |
| `Enter` | 評価実行 |
| `Del` | 1 文字削除 |
| `Fn + E` | REPL 抜ける |
| `Fn + Esc` | uLisp メニューへ |

### 17.12.8 Tools - Spreadsheet

| キー | 動作 |
|---|---|
| `;` `.` `,` `/` | セル移動 |
| `Enter` | セル編集 |
| `Del` | セル消去 |
| `S` | CSV 保存 |
| `L` | CSV 読み込み |
| `T` | Sheet ⇄ L1..L8 |
| `Fn + Esc` | Tools メニューへ |

### 17.12.9 共通キー（全モード）

| キー | 動作 |
|---|---|
| `Fn + Esc` | 前の画面に戻る |
| `ESC`（`` ` ``） | 同上 |
| `Enter` | 決定 |
| `Del` | バックスペース |
| `Fn + 数字` | モード固有のショートカット |

---

## 17.13 画面遷移図

### 17.13.1 メインメニューからの遷移

```
[メインメニュー]
├── 1 Calculator
│   ├── Fn+1 Settings
│   ├── Fn+2 History
│   ├── Fn+3 Memory
│   ├── Fn+4 Variables
│   ├── Fn+5 Base Convert
│   ├── Fn+6 Bit Ops
│   ├── Fn+7 Complex
│   ├── Fn+8 Extended Tools
│   ├── Fn+9 RPN toggle
│   └── Fn+0 Help
├── 2 CAS
│   └── 10 コマンド
├── 3 Statistics
│   ├── 1 Lists & Descriptive
│   ├── 2 Regression
│   ├── 3 Tests
│   ├── 4 Non-parametric
│   ├── 5 Distributions
│   ├── 6 Multivariate
│   ├── 7 Stratified
│   ├── 8 Survival
│   ├── 9 Meta-analysis
│   ├── 10 17 Statistical Graphs
│   └── 11 CSV Stream
├── 4 Medical Calculator
│   ├── 1 Clinical Calculators (10 領域)
│   └── 2 Epidemiology (5 項目)
├── 5 SciRef
│   ├── 1 Constants (386)
│   ├── 2 Laws (300)
│   ├── 3 Unit Converter (532)
│   ├── 4 Periodic Table (118)
│   └── 5 Search
├── 6 Graph
│   ├── 1 Plot
│   ├── 2 Edit functions
│   ├── 3 Plot type (7 種)
│   ├── 4 Range
│   ├── 5 Standard window
│   ├── 6 Trig window
│   ├── 7 Square aspect
│   ├── 8 G-Solve
│   ├── 9 Tangent/Normal
│   └── 10 Save BMP
├── 7 Tools
│   ├── 1 Spreadsheet
│   ├── 2 uLisp
│   ├── 3 Wi-Fi/Maxima
│   ├── 4 Power/About
│   ├── 5 Settings
│   └── 6 Save BMP
└── 8 About
    └── S: スプラッシュ再表示
```

### 17.13.2 ESC の階層動作

```
[サブ機能の奥] --ESC--> [サブメニュー] --ESC--> [メインメニュー]
```

ESC を押すたびに 1 階層ずつ戻ります。連続押しは ESC クールダウンで 1 階層のみ反応。

---

## 17.14 関数索引

### 17.14.1 数学関数（アルファベット順）

| 関数 | 分類 | 章 |
|---|---|---|
| abs | 丸め | 5.18 |
| acos | 逆三角 | 5.13 |
| asin | 逆三角 | 5.13 |
| atan | 逆三角 | 5.13 |
| atan2 | 三角 | 5.23 |
| avg | 配列 | 5.24 |
| cbrt | 累乗根 | 5.17 |
| ceil | 丸め | 5.18 |
| clamp | その他 | 5.23 |
| cos | 三角 | 5.12 |
| cosh | 双曲線 | 5.15 |
| cot | 三角 | 5.14 |
| count | 配列 | 5.24 |
| csc | 三角 | 5.14 |
| dms | 角度 | 5.22 |
| erf | その他 | 5.23 |
| erfc | その他 | 5.23 |
| exp | 指数 | 5.16 |
| fact | 組合せ | 5.19 |
| floor | 丸め | 5.18 |
| frac | 丸め | 5.18 |
| gamma | その他 | 5.23 |
| gcd | 素数 | 5.20 |
| hypot | その他 | 5.23 |
| if | その他 | 5.23 |
| int | 丸め | 5.18 |
| isPrime | 素数 | 5.20 |
| lcm | 素数 | 5.20 |
| lgamma | その他 | 5.23 |
| ln | 対数 | 5.16 |
| log | 対数 | 5.16 |
| log10 | 対数 | 5.16 |
| log2 | 対数 | 5.16 |
| logn | 対数 | 5.16 |
| max | 配列 | 5.24 |
| mean | 配列 | 5.24 |
| min | 配列 | 5.24 |
| mod | その他 | 5.23 |
| nCr | 組合せ | 5.19 |
| nextPrime | 素数 | 5.20 |
| nPr | 組合せ | 5.19 |
| nrt | 累乗根 | 5.17 |
| pow | 累乗 | 5.17 |
| prevPrime | 素数 | 5.20 |
| prod | 配列 | 5.24 |
| rad | 角度 | 5.22 |
| randInt | 乱数 | 5.21 |
| randN | 乱数 | 5.21 |
| randR | 乱数 | 5.21 |
| random | 乱数 | 5.21 |
| round | 丸め | 5.18 |
| sec | 三角 | 5.14 |
| sgn | 丸め | 5.18 |
| sign | 丸め | 5.18 |
| sin | 三角 | 5.12 |
| sinh | 双曲線 | 5.15 |
| sqrt | 累乗根 | 5.17 |
| sum | 配列 | 5.24 |
| tan | 三角 | 5.12 |
| tanh | 双曲線 | 5.15 |
| toC | 温度 | 5.22 |
| toF | 温度 | 5.22 |
| toK | 温度 | 5.22 |

### 17.14.2 CAS コマンド

| コマンド | 章 |
|---|---|
| ArcLength | 7.15 |
| cFactor | 7.13 |
| completeSquare | 7.13 |
| comDenom | 7.13 |
| cSolve | 7.16 |
| cZeros | 7.16 |
| deSolve | 7.17 |
| deSolve2 | 7.17 |
| Derivative | 7.15 |
| DiffQ | 7.15 |
| Expand | 7.13 |
| Factor | 7.13 |
| fMax | 7.15 |
| fMin | 7.15 |
| getDenom | 7.13 |
| getNum | 7.13 |
| ImpDiff | 7.17 |
| implicitDiff | 7.17 |
| LeadingTerm | 7.15 |
| Left | 7.13 |
| Normal | 7.15 |
| propFrac | 7.13 |
| Right | 7.13 |
| RK4 | 7.17 |
| Series | 7.15 |
| Tangent | 7.15 |
| Taylor | 7.15 |
| tCollect | 7.14 |
| tExpand | 7.14 |
| toCos | 7.14 |
| toExp | 7.14 |
| toLn | 7.14 |
| toLogbase | 7.14 |
| toSin | 7.14 |

### 17.14.3 uLisp 組み込み関数

詳細は第 20 章を参照。分類別:

- **算術**: `+ - * / mod`
- **比較**: `= equal? < > <= >=`
- **リスト**: `car cdr cons list null? atom? number? symbol? length append reverse nth`
- **数学**: `sin cos tan exp log sqrt abs floor ceil round expt pow min max random`
- **統計**: `sum mean sd`
- **高階**: `map filter reduce apply`
- **I/O**: `print save load`
- **特殊形式**: `quote if cond and or not define setq lambda let while begin progn`

---

## 17.15 略語集

| 略語 | 正式名 | 意味 |
|---|---|---|
| CAS | Computer Algebra System | 数式処理システム |
| ANOVA | Analysis of Variance | 分散分析 |
| CI | Confidence Interval | 信頼区間 |
| SD | Standard Deviation | 標準偏差 |
| SE | Standard Error | 標準誤差 |
| CV | Coefficient of Variation | 変動係数 |
| IQR | Interquartile Range | 四分位範囲 |
| OR | Odds Ratio | オッズ比 |
| RR | Risk Ratio | リスク比 |
| RD | Risk Difference | リスク差 |
| NNT | Number Needed to Treat | 治療必要数 |
| HR | Hazard Ratio | ハザード比 |
| KM | Kaplan-Meier | 生存曲線 |
| PCA | Principal Component Analysis | 主成分分析 |
| FA | Factor Analysis | 因子分析 |
| CART | Classification and Regression Tree | 決定木 |
| VIF | Variance Inflation Factor | 分散膨張因子 |
| GCS | Glasgow Coma Scale | 意識レベル |
| CURB-65 | Confusion, Urea, RR, BP, Age | 肺炎重症度 |
| qSOFA | quick SOFA | 敗血症迅速評価 |
| MELD | Model for End-stage Liver Disease | 肝疾患モデル |
| CKD-EPI | Chronic Kidney Disease Epidemiology | 腎機能式 |
| MAP | Mean Arterial Pressure | 平均動脈圧 |
| PP | Pulse Pressure | 脈圧 |
| RPP | Rate-Pressure Product | 二重積 |
| SI | Shock Index | ショック指数 |
| PF | PaO2/FiO2 | P/F 比 |
| ANC | Absolute Neutrophil Count | 好中球数 |
| MCV | Mean Corpuscular Volume | 平均赤血球容積 |
| AG | Anion Gap | アニオンギャップ |
| HOMA-IR | Homeostatic Model Assessment | インスリン抵抗性 |
| FQBN | Fully Qualified Board Name | ボード識別子 |
| NVS | Non-Volatile Storage | 不揮発記憶 |
| REPL | Read-Eval-Print Loop | 対話環境 |
| GC | Garbage Collection | メモリ回収 |
| CSV | Comma-Separated Values | カンマ区切り |
| BMP | BitMaP | 画像形式 |
| FQBN | Fully Qualified Board Name | ボード識別子 |
| SPI | Serial Peripheral Interface | シリアル通信 |
| I2C | Inter-Integrated Circuit | 2 線式通信 |
| UART | Universal Asynchronous Receiver-Transmitter | シリアル通信 |
| SSID | Service Set Identifier | Wi-Fi ネットワーク名 |
| AP | Access Point | アクセスポイント |
| IP | Internet Protocol | インターネット |
| MAC | Media Access Control | 物理アドレス |
| HTTP | Hypertext Transfer Protocol | Web 通信 |
| URL | Uniform Resource Locator | アドレス |
| RSSI | Received Signal Strength Indicator | 受信強度 |

---

## 17.16 参考文献

### 17.16.1 数学

- 杉浦光夫『解析入門 I・II』東京大学出版会
- 齋藤正彦『線型代数入門』東京大学出版会
- ブルバキ『数学原論』

### 17.16.2 統計

- 東京大学教養学部統計学教室『統計学入門』東京大学出版会
- 竹村彰通『現代数理統計学』創文社
- Hastie, Tibshirani, Friedman "The Elements of Statistical Learning"

### 17.16.3 物理・化学

- Feynman "The Feynman Lectures on Physics"
- Atkins "Physical Chemistry"
- CODATA 2022 (NIST)

### 17.16.4 医学・疫学

- 日本集中治療医学会『ICU のための統計学』
- Rothman "Modern Epidemiology"
- Cochrane Handbook for Systematic Reviews

### 17.16.5 プログラミング

- Abelson, Sussman "Structure and Interpretation of Computer Programs"
- Graham "On Lisp"
- Kernighan, Ritchie "The C Programming Language"

### 17.16.6 ハードウェア

- Espressif "ESP32-S3 Technical Reference Manual"
- M5Stack "Cardputer-Adv Documentation"

---

## 17.17 変更履歴（詳細）

### 17.17.1 v0.92（2026-10-30）

**新機能**:
- Paper UI v2（淡ミント紙 + グラファイト + ミントアクセント）
- 医学計算 10 領域 40 関数
- 統計 17 種グラフ
- CSV ストリーム統計
- Auto power off（分単位）
- uLisp スタック 32KB（v3.x 対応環境）
- uLisp eval/apply 深度ガード（400/100 段）
- uLisp cond else サポート

**バグ修正**:
- 47 件の確定バグ（0除算、NaN、inf、バッファオーバーフロー、書式不一致、再帰深度、GC 危険）
- uLisp GC の λ.env マーク
- 入力検証の強化
- グラフの陰関数描画、Square aspect オプション
- 統計計算修正（tDist, qrEig, VIF, chiIndep など）
- ctags プロトタイプ衝突回避（前方宣言 + static inline）
- `enum BaseMode : int` の基底型統一

**削除**:
- 9 件のデッドコード（Fraction::neg, Complex::sqrtC, numStr, luSolve, bGCCall, ExprEval::match, CasCommands::doFactor, StatMultivar::pickList, DisplayMgr::clearBody）
- 未使用変数 ansLine

**ビルド情報**:
- FQBN: m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app
- Flash: 1,776,859 B / 3,145,728 B（56%）
- RAM: 146,212 B / 327,680 B（44%）

### 17.17.2 v0.91（過去）

**初期実装**:
- 6 モード（Calculator, CAS, Statistics, SciRef, Graph, Tools）
- 基本 CAS（多項式中心）
- 統計（回帰、検定、ノンパラ、分布、多変量、層別、生存、メタ）
- 定数 386 / 法則 300 / 単位 532 / 周期表 118
- uLisp 基本実装
- グラフ 7 種

---

## 17.18 用語集補足

### 17.18.1 PoMaS 内部の特殊用語

| 用語 | 意味 |
|---|---|
| **Node** | 微分ノードツリーの 1 要素（DNode） |
| **Cell** | uLisp のセル（2500 個） |
| **Env** | uLisp の環境（128 個） |
| **pool** | uLisp のセルプール |
| **freeList** | uLisp の空きセルリスト |
| **mark** | uLisp GC のマークフェーズ |
| **sweep** | uLisp GC のスイープフェーズ |
| **GState** | PoMaS のグローバル状態（G 変数） |
| **DM** | DisplayMgr の略（画面管理） |
| **gEv** | ExprEval の略（式評価） |
| **gCas** | CasCommands の略（CAS エンジン） |

### 17.18.2 数値計算用語

| 用語 | 意味 |
|---|---|
| 中心差分 | (f(x+h)-f(x-h))/(2h) |
| Richardson 補外 | 誤差項を消す外挿法 |
| 適応 Simpson | 区間を適応的に分割 |
| 二分法 | 区間を半分に絞る |
| ニュートン法 | 接線で根へ |
| 黄金分割 | 最適化の 1 種 |
| Durand-Kerner | 多項式の全根を求める |
| ヤコビ反復 | 対称行列固有値 |

---

## 17.19 数値データ一覧

### 17.19.1 主要な物理定数（CODATA 2022）

| 定数 | 値 | 相対標準不確かさ |
|---|---|---|
| 光速 c | 299792458 m/s | 0（定義値） |
| プランク h | 6.62607015e-34 J·s | 0（定義値） |
| 電子電荷 e | 1.602176634e-19 C | 0（定義値） |
| ボルツマン k | 1.380649e-23 J/K | 0（定義値） |
| アボガドロ N_A | 6.02214076e23 /mol | 0（定義値） |
| 万有引力 G | 6.67430e-11 m³/(kg·s²) | 2.2e-5 |

### 17.19.2 数学定数（高精度）

| 定数 | 値（20 桁） |
|---|---|
| π | 3.14159265358979323846 |
| e | 2.71828182845904523536 |
| φ | 1.61803398874989484820 |
| γ | 0.57721566490153286061 |
| √2 | 1.41421356237309504880 |
| √3 | 1.73205080756887729353 |
| ln2 | 0.69314718055994530942 |
| ln10 | 2.30258509299404568402 |

### 17.19.3 天文定数（IAU 2015）

| 定数 | 値 |
|---|---|
| AU | 1.495978707e11 m |
| ly | 9.4607304725808e15 m |
| pc | 3.08567758149137e16 m |
| M_sun | 1.9884e30 kg |
| R_sun | 6.957e8 m |

---

## 17.20 索引（あいうえお順）

**あ行**
- アークコサイン（acos）— 5.13
- アークサイン（asin）— 5.13
- アークタンジェント（atan）— 5.13
- 圧力単位 — 11.13.8
- Ans — 5.4
- Ans バンク — 5.9
- 医学計算 — 18 章

**か行**
- 回路計算 — 16.14
- 回帰分析 — 8.10
- 階乗 — 5.19
- 科学定数 — 11.11
- 科学法則 — 11.12
- 確率分布 — 9.12
- 仮説検定 — 8.11
- 加法定理 — 7.14
- 関数グラフ — 10.3
- 基数変換 — 6.3
- 企業分析 — 16.15
- 気象データ — 16.10
- 幾何計算 — 6.19
- 行列 — 6.18
- 完全平方 — 7.13
- 関数 — 5.12〜5.24
- 記号検索 — 11.15

**さ行**
- 最適化 — 6.17
- 差分商 — 7.15
- 三角関数 — 5.12
- 三重積分 — 7.15
- 自己組織化 — 9.13
- 指数関数 — 5.16
- 指数分布 — 9.12.6
- 事業継続 — 16.15
- 重回帰 — 8.10
- 集計関数 — 5.24
- ジュール — 11.13.9
- 循環参照 — 12.11
- 条件分岐 — 12.16
- 消費電力 — 16.14
- 女性ホルモン — 18.10
- 食中毒 — 9.14
- 初等関数 — 5.12

**た行**
- 対数 — 5.16
- 多重比較 — 8.18
- 多変量解析 — 9.13
- 単位換算 — 11.13
- 中央値 — 8.9
- 調和平均 — 8.6
- 定義域 — 10.5
- 定積分 — 7.15
- テイラー展開 — 7.15
- データベース — 6.6〜6.9
- 統計 — 8 章
- 統計グラフ — 9.17
- 動画サンプル — 16.16
- 投票データ — 16.12

**な行**
- 内部状態 — 13.9
- 内分泌 — 18.10
- 二項定理 — 5.19
- 二項分布 — 9.12.5
- ニュートン法 — 6.16
- 入力検証 — 14.12
- 熱伝導 — 16.14
- 燃料電池 — 16.11
- ネットワーク — 12.19
- 年次成長率 — 16.15
- ノンパラメトリック — 9.10

**は行**
- パイプライン — 12.13
- 箱ひげ図 — 9.17.2
- パスカル — 11.13.8
- 白血球 — 18.5
- バグ修正履歴 — 17.17
- 反応速度 — 16.11
- 光合成 — 16.12
- 引き算 — 5.2
- ヒストグラム — 9.17.1
- 非線形回帰 — 8.10
- 微分 — 7.15
- 微分方程式 — 7.17
- 病態生理 — 18 章
- ファイル形式 — 17.6
- フィルタ — 12.13
- 複素数 — 6.12
- 浮動小数点 — 17.19
- フーリエ — 7.14
- 部分分数 — 7.13
- 分散 — 8.9
- 分数 — 6.6

**ま行**
- マイクロ — 17.19
- 前処理 — 12.13
- 待機時間 — 13.8
- 待ち行列 — 16.16
- マクロ — 20.14
- 摩擦係数 — 16.10
- 満足度 — 16.12
- 右辺 — 7.13
- 未知数 — 7.16
- 無次元量 — 11.11
- メタアナリシス — 9.16
- メモリ M — 5.8
- 文字列 — 20.3

**や行・ら行・わ行**
- ヤコビ — 17.18
- 有効数字 — 17.19
- 輸送現象 — 16.14
- 陽関数 — 10.3
- 要素 — 11.14
- ラプラス — 7.15
- 力学 — 16.10
- リスク指標 — 9.11
- 履歴 — 5.7
- 臨床スコア — 18.6
- 累乗 — 5.17
- レポート作成 — 16.9
- 論理演算 — 6.14
- 和集合 — 17.14

---

## 17.21 索引（アルファベット順）

| 用語 | 章 |
|---|---|
| abs | 5.18 |
| acos | 5.13 |
| AND | 6.14 |
| ANOVA | 8.15 |
| Ans | 5.4 |
| Ans Bank | 5.9 |
| append | 20.6 |
| apply | 20.9 |
| ArcLength | 7.15 |
| asin | 5.13 |
| atan | 5.13 |
| atan2 | 5.23 |
| avg | 5.24 |
| Benjamin-Hochberg | 8.18 |
| Bernoulli | 8.18 |
| Binomial | 9.12.5 |
| BMI | 18.7 |
| Bonferroni | 8.18 |
| Bridge | 20 |
| BSA | 18.7 |
| C | 20.3 |
| CAS | 7 |
| cbrt | 5.17 |
| ceil | 5.18 |
| Cell | 17.18 |
| cFactor | 7.13 |
| charPoly | 6.18 |
| Child-Pugh | 18.6 |
| clamp | 5.23 |
| CKD-EPI | 18.8 |
| Cockcroft-Gault | 18.8 |
| Cohen's d | 8.19 |
| comDenom | 7.13 |
| completeSquare | 7.13 |
| cond | 20.10 |
| cond | 12.16 |
| cond | 20.10 |
| Cox PH | 9.15 |
| Cramér's V | 8.19 |
| csc | 5.14 |
| cSolve | 7.16 |
| cubic | 6.16 |
| CURB-65 | 18.6 |
| CV | 8.9 |
| CZeros | 7.16 |
| define | 20.10 |
| deSolve | 7.17 |
| deSolve2 | 7.17 |
| Derivative | 7.15 |
| DiffQ | 7.15 |
| Distribution | 9.12 |
| dms | 5.22 |
| Egger's test | 9.16 |
| Env | 17.18 |
| erf | 5.23 |
| erfc | 5.23 |
| Expand | 7.13 |
| exp | 5.16 |
| Factor | 7.13 |
| fact | 5.19 |
| fDist | 9.12.4 |
| Fisher | 8.17 |
| floor | 5.18 |
| fMax | 7.15 |
| fMin | 7.15 |
| Forest plot | 9.16 |
| frac | 5.18 |
| Friedman | 9.10.4 |
| F-test | 8.14 |
| Funnel plot | 9.16 |
| GCS | 18.6 |
| GC | 20.13 |
| getDenom | 7.13 |
| getNum | 7.13 |
| Graph | 10 |
| hbar | 11.11 |
| HOMA-IR | 18.10 |
| HR | 9.15 |
| if | 20.10 |
| ImpDiff | 7.17 |
| implicitDiff | 7.17 |
| int | 5.18 |
| IQR | 8.9 |
| isPrime | 5.20 |
| KDE | 9.17 |
| Kendall | 9.10 |
| KM | 9.15 |
| lambda | 20.10 |
| lcm | 5.20 |
| LeadingTerm | 7.15 |
| Left | 7.13 |
| let | 20.10 |
| lgamma | 5.23 |
| ln | 5.16 |
| log | 5.16 |
| log10 | 5.16 |
| log2 | 5.16 |
| logn | 5.16 |
| Log-rank | 9.15 |
| Mann-Whitney U | 9.10 |
| MAP | 18.3 |
| max | 5.24 |
| mean | 5.24 |
| med | 8.9 |
| MELD | 18.6 |
| Meta-analysis | 9.16 |
| min | 5.24 |
| mod | 5.23 |
| NaN | 5.24 |
| nCr | 5.19 |
| nextPrime | 5.20 |
| Normal | 7.15 |
| nPr | 5.19 |
| nrt | 5.17 |
| OR | 9.11 |
| PCA | 9.13 |
| pow | 5.17 |
| prevPrime | 5.20 |
| print | 20.11 |
| prod | 5.24 |
| propFrac | 7.13 |
| qSOFA | 18.6 |
| rad | 5.22 |
| randInt | 5.21 |
| randN | 5.21 |
| randR | 5.21 |
| random | 5.21 |
| reduce | 20.9 |
| REPL | 20.2 |
| reverse | 20.6 |
| Right | 7.13 |
| RK4 | 7.17 |
| RR | 9.11 |
| save | 20.11 |
| sd | 20.8 |
| sec | 5.14 |
| Series | 7.15 |
| setq | 20.10 |
| sgn | 5.18 |
| sign | 5.18 |
| sin | 5.12 |
| sinh | 5.15 |
| Spearman | 9.10 |
| sqrt | 5.17 |
| sum | 5.24 |
| swap | 6.15 |
| tan | 5.12 |
| tanh | 5.15 |
| Taylor | 7.15 |
| tCollect | 7.14 |
| tExpand | 7.14 |
| toC | 5.22 |
| toCos | 7.14 |
| toExp | 7.14 |
| toF | 5.22 |
| toK | 5.22 |
| toLn | 7.14 |
| toLogbase | 7.14 |
| toSin | 7.14 |
| uLisp | 20 |
| while | 20.10 |
| XOR | 6.14 |

---

## 17.22 この章のまとめ

- 完全キーアサイン表（9 モード別）
- 画面遷移図（メインメニューからの全経路）
- 関数索引（数学 70 関数、CAS 34 コマンド、uLisp 80 関数）
- 略語集（医学・数学・ハード・通信）
- 参考文献（数学・統計・物理・化学・医学・プログラミング・ハード）
- 変更履歴詳細（v0.91 → v0.92）
- 用語集補足（PoMaS 内部の特殊用語）
- 数値データ一覧（物理・数学・天文の高精度値）
- あいうえお順索引
- アルファベット順索引

### 確認問題

Q17-1: Fn + ESC の動作は？ → 前の画面に戻る

Q17-2: Ans バンクの最大数は？ → 20 件

Q17-3: 統計リストはいくつ？ → 8 つ（L1〜L8、各 200 件）

Q17-4: 科学定数の出典は？ → CODATA 2022

Q17-5: uLisp セルの上限は？ → 2500 個

Q17-6: 変更履歴で v0.92 のバグ修正数は？ → 47 件

---

[← 第16章 実用例集](16_examples.md) | [目次](#/) | [README に戻る](../../README.md)
