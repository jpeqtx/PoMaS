# 第15章 用語集

---

## この章で学ぶこと

- 数学記号（基本演算・集合・微積分・比較）
- アルゴリズム用語（数値解法・ODE・線形代数・統計）
- データベース構造用語（PoMaS 内部）
- ハードウェア用語（MCU・ディスプレイ・ストレージ）
- ソフトウェア用語（フレームワーク・ビルド・バージョン）
- 略語一覧（数学・統計・多変量・医学・組込・通信）
- 数学定数（8 種）
- 単位記号（SI 基本・組立・接頭辞）
- キーボード記号

---

## 15.1 この章の使い方
PoMaS および本書に登場する用語を、五十音順・アルファベット順でまとめます。
わからない用語が出てきたら、この章を参照してください。
---
## 15.2 あ行
**Ans（アンズ）**
直前の計算結果。`Ans` キーワードで参照できる。
**NVS（不揮発性ストレージ）**
電源を切ってもデータが消えないストレージ。
---
## 15.3 か行
**基数（きすう）**
数の桁を表す基本の数。10 進数は基数 10。
**求根（きゅうこん）**
方程式 f(x) = 0 の解（根）を求めること。
**共分散（きょうぶんさん）**
2 つの変数の関係の強さを表す指標。
**極限（きょくげん）**
変数がある値に近づくときの関数の値。
**組込み関数（くみこみかんすう）**
PoMaS に最初から用意されている関数（sin, cos など）。
**行列（ぎょうれつ）**
数を縦横に並べた表。数学・物理で使う。
**固有値（こゆうち）**
行列の性質を表す数。線形代数で重要。
---
## 15.4 さ行
**最小二乗法（さいしょうにじょうほう）**
データに最も当てはまる直線を求める方法。
**三角関数（さんかくかんすう）**
sin, cos, tan。角度と辺の比の関係を表す。
**自然対数（しぜんたいすう）**
底が e（≈ 2.718）の対数。`ln` で表す。
**重回帰（じゅうかいき）**
複数の説明変数で目的変数を予測する回帰。
**主成分分析（しゅせいぶんぶんせき）**
多次元データを少ない次元に要約する手法。PCA。
**順列（じゅんれつ）**
順番を区別した並べ方。nPr で計算。
**相関係数（そうかんけいすう）**
2 変数の線形関係の強さ。-1〜1 の範囲。
**総和（そうわ）**
数列の和。Σ（シグマ）記号で表す。
---
## 15.5 た行
**対数（たいすう）**
log。10 の何乗かを表す。`log` は底 10、`ln` は自然対数。
**多項式（たこうしき）**
x^n の項の和で表される式。
**中央値（ちゅうおうち）**
データを小さい順に並べたときの真ん中の値。
**標準偏差（ひょうじゅんへんさ）**
データの散らばり具合を表す。sd で表す。
**度数分布（どすうぶんぷ）**
データがどの値にどれだけ分布するか。
**統計検定（とうけいけんてい）**
データから仮説を判定する手法。p 値で判断。
---
## 15.6 な行
**二分法（にぶんほう）**
区間を半分に絞って根を探す数値解法。
**ニュートン法（ニュートンほう）**
接線を使って根を探す数値解法。
---
## 15.7 は行
**p 値（ピーち）**
帰無仮説が正しい確率。0.05 未満で「有意」。
**微分（びぶん）**
関数の変化率。導関数を求めること。
**ヒストグラム**
度数分布を棒グラフにしたもの。
**標準誤差（ひょうじゅんごさ）**
標本平均のばらつき。SE で表す。
**不定積分（ふていせきぶん）**
積分定数 C を含む積分。
**分散（ぶんさん）**
データの散らばりの 2 乗平均。
**母集団（ぼしゅうだん）**
統計の対象全体。
**補集合（ほしゅうごう）**
ある集合に含まれない要素の集合。
---
## 15.8 ま行
**マンハッタン距離**
|a-b| + |c-d| で計算される距離。
**メモリ M（メモリエム）**
古典的な電卓の独立記憶領域。
---
## 15.9 や行・ら行
**余弦（よげん）**
cos。三角関数の一つ。
**ライン（回帰直線）**
2 変数の関係を表す直線。
**リスク比（リスクひ）**
2 群のリスクの比。RR で表す。
---
## 15.10 アルファベット順
**Ans** — 直前の計算結果
**CAS** — Computer Algebra System（数式処理システム）
**CDF** — 累積分布関数
**CI** — 信頼区間
**DEG** — 度数法（Degree）
**GRA** — グラジアン
**GC** — ガベージコレクション
**KDE** — カーネル密度推定
**Kruskal-Wallis** — ノンパラ 3 群以上
**Lisp** — プログラミング言語の一種
**MANOVA** — 多変量分散分析
**Maxima** — 数式処理システム（外部）
**M-H** — Mantel-Haenszel（層別解析）
**NNT** — Number Needed to Treat
**NVS** — Non-Volatile Storage
**OR** — Odds Ratio（オッズ比）
**PCA** — 主成分分析
**PDF** — 確率密度関数
**PMF** — 確率質量関数
**Q1, Q3** — 第 1、第 3 四分位数
**RAD** — 弧度法（Radian）
**REPL** — Read-Eval-Print Loop
**RR** — Risk Ratio（リスク比）
**SD** — 標準偏差（Standard Deviation）
**SE** — 標準誤差（Standard Error）
**UI** — User Interface
**uLisp** — 小型 Lisp 実装
---
## 15.11 数学記号
| 記号 | 意味 |
|---|---|
| Σ | 総和 |
| Π | 総乗 |
| √ | 平方根 |
| ∫ | 積分 |
| ∂ | 偏微分 |
| π | 円周率 |
| ∞ | 無限大 |
| ≈ | ほぼ等しい |
| ≤ | 以下 |
| ≥ | 以上 |
| ≠ | 等しくない |
---
## 15.13 数学記号詳細

### 15.13.1 基本演算記号

| 記号 | 名称 | 意味 | 例 |
|---|---|---|---|
| `+` | プラス | 加算 | 2+3=5 |
| `−` | マイナス | 減算 / 負号 | 5−3=2 |
| `×` | 掛け算 | 乗算（PoMaS では `*`） | 2×3=6 |
| `÷` | 割り算 | 除算（PoMaS では `/`） | 6÷2=3 |
| `±` | プラスマイナス | 正負両方 | ±5 |
| `∓` | マイナスプラス | 逆順 | ∓5 |

### 15.13.2 べき乗・根

| 記号 | 名称 | 意味 | PoMaS 入力 |
|---|---|---|---|
| `x²` | 2 乗 | x の 2 乗 | `x^2` |
| `x³` | 3 乗 | x の 3 乗 | `x^3` |
| `xⁿ` | n 乗 | x の n 乗 | `x^n` |
| `√x` | 平方根 | √x | `sqrt(x)` |
| `∛x` | 立方根 | ∛x | `cbrt(x)` |
| `ⁿ√x` | n 乗根 | x の n 乗根 | `nrt(n, x)` |

### 15.13.3 集合・論理

| 記号 | 名称 | 意味 |
|---|---|---|
| `∈` | 属する | a ∈ A |
| `∉` | 属さない | a ∉ A |
| `⊂` | 部分集合 | A ⊂ B |
| `∪` | 和集合 | A ∪ B |
| `∩` | 積集合 | A ∩ B |
| `∅` | 空集合 | 要素なし |
| `∀` | 全ての | ∀x |
| `∃` | 存在する | ∃x |
| `∧` | 論理積 | A ∧ B |
| `∨` | 論理和 | A ∨ B |
| `¬` | 否定 | ¬A |
| `→` | 含意 | A → B |
| `↔` | 同値 | A ↔ B |

### 15.13.4 微積分

| 記号 | 名称 | 意味 |
|---|---|---|
| `d/dx` | 微分 | x について微分 |
| `∂/∂x` | 偏微分 | x に関する偏微分 |
| `∫` | 積分 | 不定積分 |
| `∫ₐᵇ` | 定積分 | a から b まで |
| `Σ` | 総和 | シグマ |
| `Π` | 総乗 | パイ |
| `lim` | 極限 | lim (x→a) |
| `∞` | 無限大 | 無限 |
| `∇` | ナブラ | 勾配作用素 |
| `Δ` | デルタ | 変化量 |

### 15.13.5 比較

| 記号 | 名称 | 意味 |
|---|---|---|
| `=` | 等号 | 等しい |
| `≠` | 不等号 | 等しくない |
| `≈` | 近似 | ほぼ等しい |
| `≡` | 合同 | 恒等的に等しい |
| `<` | 小なり | より小さい |
| `>` | 大なり | より大きい |
| `≤` | 以下 | 以下 |
| `≥` | 以上 | 以上 |
| `≪` | ずっと小 | 十分小さい |
| `≫` | ずっと大 | 十分大きい |

### 15.13.6 その他

| 記号 | 名称 | 意味 |
|---|---|---|
| `∵` | なぜなら | 理由 |
| `∴` | ゆえに | 結論 |
| `∝` | 比例 | A ∝ B |
| `∠` | 角 | 角度 |
| `⊥` | 垂直 | 直交 |
| `∥` | 平行 | 平行 |
| `°` | 度 | 角度 |
| `′` | 分 | 角度 |
| `″` | 秒 | 角度 |
| `‰` | パーミル | 千分率 |
| `%` | パーセント | 百分率 |
| `π` | パイ | 円周率 |
| `∞` | 無限大 | 無限 |

---

## 15.14 アルゴリズム用語

### 15.14.1 数値解法

| 用語 | 英語 | 意味 |
|---|---|---|
| 二分法 | Bisection | 区間を半分に絞る |
| ニュートン法 | Newton-Raphson | 接線で根へ |
| 割線法 | Secant | 2 点を結ぶ線 |
| 補間法 | Interpolation | 点の間を推定 |
| 反復法 | Iteration | 繰り返し改善 |
| 収束 | Convergence | 解に近づく |
| 発散 | Divergence | 解から遠ざかる |
| 収束判定 | Convergence check | 誤差閾値 |

### 15.14.2 微分方程式

| 用語 | 英語 | 意味 |
|---|---|---|
| オイラー法 | Euler method | 1 次近似 |
| RK4 | Runge-Kutta 4th | 4 次精度 |
| 陽解法 | Explicit method | 現在値のみ |
| 陰解法 | Implicit method | 未来値含む |
| 安定性 | Stability | 誤差増幅しない |

### 15.14.3 線形代数

| 用語 | 英語 | 意味 |
|---|---|---|
| ガウス消去 | Gaussian elimination | 行列の三角化 |
| LU 分解 | LU decomposition | 下三角 × 上三角 |
| QR 分解 | QR decomposition | 直交 × 上三角 |
| ヤコビ法 | Jacobi | 対称行列固有値 |
| QR 法 | QR algorithm | 一般行列固有値 |
| 特異値分解 | SVD | A = UΣV* |

### 15.14.4 統計

| 用語 | 英語 | 意味 |
|---|---|---|
| 最尤推定 | MLE | 尤度最大化 |
| 最小二乗法 | LSM | 誤差 2 乗和最小 |
| ベイズ推定 | Bayesian | 事後確率 |
| ブートストラップ | Bootstrap | リサンプリング |
| ジャックナイフ | Jackknife | 1 つ抜き再計算 |

---

## 15.15 データベース構造用語

### 15.15.1 PoMaS 内部構造

| 用語 | 意味 |
|---|---|
| `constDB[]` | 科学定数配列（386 件） |
| `unitDB[]` | 単位換算配列（532 件） |
| `lawDB[]` | 科学法則配列 Part 1（170 件） |
| `lawDB2[]` | 科学法則配列 Part 2（130 件） |
| `ELEM_INFO[]` | 元素情報配列（118 件） |
| `PERIODIC[]` | 元素記号配列 |
| `ATOMIC_W[]` | 原子量配列 |
| `lawCalcDB[]` | 法則計算式配列（15 件） |

### 15.15.2 データ構造

| 用語 | 英語 | 意味 |
|---|---|---|
| 構造体 | struct | 複合データ |
| 配列 | array | 連続データ |
| 列挙型 | enum | 列挙値 |
| 共用体 | union | メモリ共有 |
| ポインタ | pointer | アドレス参照 |

### 15.15.3 メモリ配置

| 用語 | 意味 |
|---|---|
| PROGMEM | Flash 配置（読み取り専用） |
| IRAM | 命令 RAM |
| DRAM | データ RAM |
| PSRAM | 外部 PSRAM（非搭載） |
| heap | 動的確保領域 |
| stack | スタック領域 |

---

## 15.16 ハードウェア用語

### 15.16.1 マイコン

| 用語 | 意味 |
|---|---|
| ESP32-S3 | Espressif 製 MCU |
| Dual Core | 2 コア（240MHz） |
| Flash | 8MB（ファームウェア格納） |
| SRAM | 512KB（作業用） |
| PSRAM | 外部メモリ（Cardputer-Adv は非搭載） |

### 15.16.2 ディスプレイ

| 用語 | 意味 |
|---|---|
| ST7789V2 | LCD コントローラ |
| 240 × 135 | 画面解像度（px） |
| RGB565 | 16bit 色（65,536 色） |
| SPI | 通信プロトコル |

### 15.16.3 キーボード

| 用語 | 意味 |
|---|---|
| TCA8418 | キーボードコントローラ |
| IOMatrix | マトリックススキャン |
| fn | 修飾キー |
| ESC | バッククォートキー |

### 15.16.4 ストレージ

| 用語 | 意味 |
|---|---|
| microSD | SD カード |
| SPI | 通信プロトコル |
| FAT32 | ファイルシステム |
| CS | チップセレクト |

### 15.16.5 電源

| 用語 | 意味 |
|---|---|
| Li-Ion | リチウムイオン電池 |
| USB-C | 充電端子 |
| mA | 電流（ミリアンペア） |
| 電源 OFF | 長押し 2 秒 |

---

## 15.17 ソフトウェア用語

### 15.17.1 フレームワーク

| 用語 | 意味 |
|---|---|
| Arduino | 開発フレームワーク |
| M5Cardputer | ハードウェア抽象化 |
| M5Unified | 統合ライブラリ |
| M5GFX | グラフィックス |
| FreeRTOS | リアルタイム OS |

### 15.17.2 ビルド

| 用語 | 意味 |
|---|---|
| FQBN | Fully Qualified Board Name |
| ctags | シンボル抽出ツール |
| prototype | 関数プロトタイプ |
| linked | リンク済み（最終バイナリ） |
| merged.bin | 8MB 統合バイナリ |

### 15.17.3 バージョン管理

| 用語 | 意味 |
|---|---|
| SemVer | セマンティックバージョニング |
| MAJOR | 互換性なし |
| MINOR | 後方互換あり |
| PATCH | バグ修正のみ |

---

## 15.18 略語一覧

### 15.18.1 アルゴリズム・数学

| 略語 | 正式名 | 意味 |
|---|---|---|
| CAS | Computer Algebra System | 数式処理 |
| LSM | Least Squares Method | 最小二乗法 |
| MLE | Maximum Likelihood Estimation | 最尤推定 |
| MC | Monte Carlo | モンテカルロ |
| ODE | Ordinary Differential Equation | 常微分方程式 |
| PDE | Partial Differential Equation | 偏微分方程式 |
| RK4 | Runge-Kutta 4th | 4 次ルンゲクッタ |
| LU | Lower-Upper | LU 分解 |
| QR | Orthogonal-Triangular | QR 分解 |
| SVD | Singular Value Decomposition | 特異値分解 |

### 15.18.2 統計

| 略語 | 正式名 | 意味 |
|---|---|---|
| ANOVA | Analysis of Variance | 分散分析 |
| ANCOVA | Analysis of Covariance | 共分散分析 |
| MANOVA | Multivariate ANOVA | 多変量分散分析 |
| GLM | Generalized Linear Model | 一般化線形モデル |
| CI | Confidence Interval | 信頼区間 |
| SE | Standard Error | 標準誤差 |
| SD | Standard Deviation | 標準偏差 |
| CV | Coefficient of Variation | 変動係数 |
| IQR | Interquartile Range | 四分位範囲 |
| OR | Odds Ratio | オッズ比 |
| RR | Risk Ratio | リスク比 |
| RD | Risk Difference | リスク差 |
| NNT | Number Needed to Treat | 治療必要数 |
| NNH | Number Needed to Harm | 有害必要数 |
| HR | Hazard Ratio | ハザード比 |
| KM | Kaplan-Meier | KM 推定 |
| FDR | False Discovery Rate | 偽発見率 |
| FWER | Family-Wise Error Rate | 家系別誤り率 |
| VIF | Variance Inflation Factor | 分散膨張因子 |

### 15.18.3 多変量

| 略語 | 正式名 | 意味 |
|---|---|---|
| PCA | Principal Component Analysis | 主成分分析 |
| FA | Factor Analysis | 因子分析 |
| EFA | Exploratory FA | 探索的因子分析 |
| CFA | Confirmatory FA | 確認的因子分析 |
| CART | Classification and Regression Tree | 決定木 |
| LDA | Linear Discriminant Analysis | 線形判別分析 |
| QDA | Quadratic Discriminant Analysis | 2 次判別分析 |

### 15.18.4 医学・疫学

| 略語 | 正式名 | 意味 |
|---|---|---|
| GCS | Glasgow Coma Scale | 意識レベル |
| CURB-65 | Confusion, Urea, RR, BP, Age | 肺炎重症度 |
| qSOFA | quick Sequential Organ Failure Assessment | 敗血症迅速評価 |
| MELD | Model for End-stage Liver Disease | 肝疾患モデル |
| CKD-EPI | Chronic Kidney Disease Epidemiology | 腎機能式 |
| BSA | Body Surface Area | 体表面積 |
| IBW | Ideal Body Weight | 理想体重 |
| ABW | Adjusted Body Weight | 補正体重 |
| MAP | Mean Arterial Pressure | 平均動脈圧 |
| PP | Pulse Pressure | 脈圧 |
| RPP | Rate-Pressure Product | 二重積 |
| SI | Shock Index | ショック指数 |
| PF | PaO2/FiO2 ratio | P/F 比 |
| A-a | Alveolar-arterial gradient | 肺胞動脈較差 |
| OI | Oxygenation Index | 酸素化指数 |
| ANC | Absolute Neutrophil Count | 好中球数 |
| MCV | Mean Corpuscular Volume | 平均赤血球容積 |
| MCH | Mean Corpuscular Hemoglobin | 平均色素量 |
| MCHC | Mean Corpuscular Hemoglobin Concentration | 平均色素濃度 |
| MI | Mentzer Index | メンツァー指数 |
| AG | Anion Gap | アニオンギャップ |
| FWD | Free Water Deficit | 自由水欠乏 |
| HOMA-IR | Homeostatic Model Assessment | インスリン抵抗性 |
| FTI | Free T4 Index | 遊離 T4 指数 |
| EDD | Estimated Due Date | 分娩予定日 |
| GA | Gestational Age | 在胎週数 |
| LD | Loading Dose | 負荷投与量 |
| MD | Maintenance Dose | 維持投与量 |
| t1/2 | Half-life | 半減期 |

### 15.18.5 ハードウェア・組込

| 略語 | 正式名 | 意味 |
|---|---|---|
| MCU | Microcontroller Unit | マイコン |
| SoC | System on Chip | システムオンチップ |
| GPIO | General Purpose I/O | 汎用入出力 |
| SPI | Serial Peripheral Interface | シリアル通信 |
| I2C | Inter-Integrated Circuit | 2 線式通信 |
| UART | Universal Asynchronous Receiver-Transmitter | シリアル通信 |
| USB | Universal Serial Bus | USB 端子 |
| PWM | Pulse Width Modulation | パルス幅変調 |
| ADC | Analog-to-Digital Converter | AD 変換 |
| DAC | Digital-to-Analog Converter | DA 変換 |

### 15.18.6 ソフトウェア

| 略語 | 正式名 | 意味 |
|---|---|---|
| API | Application Programming Interface | 外部インターフェース |
| FQBN | Fully Qualified Board Name | ボード識別子 |
| IDE | Integrated Development Environment | 統合開発環境 |
| REPL | Read-Eval-Print Loop | 対話環境 |
| GC | Garbage Collection | メモリ自動回収 |
| NVS | Non-Volatile Storage | 不揮発記憶 |
| SD | Secure Digital | SD カード |
| CSV | Comma-Separated Values | カンマ区切り |
| BMP | BitMaP | 画像形式 |
| UTF | Unicode Transformation Format | 文字コード |
| BOM | Byte Order Mark | バイト順序マーク |
| CRLF | Carriage Return + Line Feed | 改行コード |
| LF | Line Feed | 改行コード |

### 15.18.7 通信・ネットワーク

| 略語 | 正式名 | 意味 |
|---|---|---|
| Wi-Fi | Wireless Fidelity | 無線 LAN |
| SSID | Service Set Identifier | ネットワーク名 |
| AP | Access Point | アクセスポイント |
| IP | Internet Protocol | インターネット |
| MAC | Media Access Control | 物理アドレス |
| HTTP | Hypertext Transfer Protocol | Web 通信 |
| URL | Uniform Resource Locator | アドレス |
| GET | HTTP GET request | 取得リクエスト |
| RSSI | Received Signal Strength Indicator | 受信強度 |

---

## 15.19 数学定数（用語集版）

| 名称 | 記号 | 値 | 意味 |
|---|---|---|---|
| 円周率 | π | 3.14159... | 円周 / 直径 |
| ネイピア数 | e | 2.71828... | 自然対数の底 |
| 黄金比 | φ | 1.61803... | (1+√5)/2 |
| オイラー=マスケローニ | γ | 0.57721... | 極限値 |
| カタラン定数 | G | 0.91596... | 級数和 |
| アペリー定数 | ζ(3) | 1.20205... | ゼータ関数の値 |
| プラスチック数 | ρ | 1.32471... | x³=x+1 の実根 |
| 白銀比 | δ_S | 2.41421... | 1+√2 |

---

## 15.20 単位記号

### 15.20.1 SI 基本単位

| 記号 | 名称 | 量 |
|---|---|---|
| m | メートル | 長さ |
| kg | キログラム | 質量 |
| s | 秒 | 時間 |
| A | アンペア | 電流 |
| K | ケルビン | 温度 |
| mol | モル | 物質量 |
| cd | カンデラ | 光度 |

### 15.20.2 SI 組立単位

| 記号 | 名称 | 相当 |
|---|---|---|
| N | ニュートン | kg·m/s² |
| J | ジュール | N·m |
| W | ワット | J/s |
| Pa | パスカル | N/m² |
| Hz | ヘルツ | 1/s |
| V | ボルト | W/A |
| Ω | オーム | V/A |
| F | ファラド | C/V |
| C | クーロン | A·s |
| Wb | ウェーバ | V·s |
| T | テスラ | Wb/m² |
| H | ヘンリー | Wb/A |

### 15.20.3 SI 接頭辞

| 記号 | 名称 | 倍率 |
|---|---|---|
| Y | ヨタ | 10²⁴ |
| Z | ゼタ | 10²¹ |
| E | エクサ | 10¹⁸ |
| P | ペタ | 10¹⁵ |
| T | テラ | 10¹² |
| G | ギガ | 10⁹ |
| M | メガ | 10⁶ |
| k | キロ | 10³ |
| h | ヘクト | 10² |
| da | デカ | 10¹ |
| （なし） | — | 10⁰ |
| d | デシ | 10⁻¹ |
| c | センチ | 10⁻² |
| m | ミリ | 10⁻³ |
| µ | マイクロ | 10⁻⁶ |
| n | ナノ | 10⁻⁹ |
| p | ピコ | 10⁻¹² |
| f | フェムト | 10⁻¹⁵ |
| a | アト | 10⁻¹⁸ |
| z | ゼプト | 10⁻²¹ |
| y | ヨクト | 10⁻²⁴ |

---

## 15.21 キーボード記号

### 15.21.1 よく使う記号

| 記号 | 意味 | PoMaS での入力 |
|---|---|---|
| `+` | 加算 | そのまま |
| `-` | 減算 | そのまま |
| `*` | 乗算 | そのまま |
| `/` | 除算 | そのまま |
| `^` | 累乗 | そのまま |
| `!` | 階乗 | そのまま |
| `%` | 剰余 | そのまま |
| `(` `)` | 括弧 | そのまま |
| `[` `]` | 配列 | そのまま |
| `,` | 引数区切り | そのまま |
| `;` | パラメトリック区切り | そのまま |
| `'` | quote（Lisp） | そのまま |
| `=` | 等号 | そのまま |

### 15.21.2 特殊キー

| キー | 動作 |
|---|---|
| `Enter` | 決定 / 実行 |
| `Del` | バックスペース |
| `Fn + Esc` | 戻る |
| `;` | 上移動 |
| `.` | 下移動 |
| `,` | 左移動 |
| `/` | 右移動 |

---

## 15.22 この章のまとめ

- 数学記号の完全リファレンス（基本演算・集合・微積分・比較）
- アルゴリズム用語（数値解法・ODE・線形代数・統計）
- データベース構造用語（PoMaS 内部構造を含む）
- ハードウェア用語（MCU・ディスプレイ・ストレージ）
- ソフトウェア用語（フレームワーク・ビルド・バージョン）
- 略語一覧（数学・統計・多変量・医学・組込・通信）
- 数学定数・単位記号・キーボード記号

### 確認問題

Q15-1: PoMaS の科学定数は何件？ → 386 件（11 カテゴリ）

Q15-2: 統計の標準誤差の略語は？ → SE（Standard Error）

Q15-3: FQBN の意味は？ → Fully Qualified Board Name

Q15-4: NVS の正式名は？ → Non-Volatile Storage

Q15-5: HR の意味は？ → Hazard Ratio（ハザード比）

Q15-6: SI 接頭辞「μ」は？ → マイクロ（10⁻⁶）

---

## 15.23 統計用語の拡張（120 語）

### 15.23.1 記述統計（25 語）

| 用語 | 英語 | 意味 |
|---|---|---|
| 平均 | Mean | Σx/n |
| 中央値 | Median | 中央値 |
| 最頻値 | Mode | 最多出現値 |
| 分散 | Variance | 散らばりの 2 乗平均 |
| 標準偏差 | Standard Deviation | √分散 |
| 標準誤差 | Standard Error | SD/√n |
| 変動係数 | CV | SD/|mean|×100% |
| 範囲 | Range | max - min |
| 四分位数 | Quartile | Q1/Q2/Q3 |
| 四分位範囲 | IQR | Q3 - Q1 |
| 歪度 | Skewness | 左右非対称度 |
| 尖度 | Kurtosis | 尖り具合 |
| 五数要約 | Five-number summary | min/Q1/med/Q3/max |
| 外れ値 | Outlier | 極端な値 |
| ロバスト | Robust | 外れ値に強い |
| トリム平均 | Trimmed mean | 両端除く |
| 重み付き平均 | Weighted mean | 重み付き和 |
| 幾何平均 | Geometric mean | 積の n 乗根 |
| 調和平均 | Harmonic mean | 逆数和の逆数 |
| 移動平均 | Moving average | 部分平均 |
| 中央値絶対偏差 | MAD | median(|x-med|) |
| 百分位数 | Percentile | 累積%の値 |
| 箱ひげ図 | Boxplot | 分布の可視化 |
| ヒストグラム | Histogram | 度数分布 |
| 経験分布関数 | ECDF | 累積分布 |

### 15.23.2 確率分布（25 語）

| 用語 | 英語 | 意味 |
|---|---|---|
| 確率変数 | Random variable | 確率的に決まる値 |
| 離散分布 | Discrete distribution | 飛び飛びの値 |
| 連続分布 | Continuous distribution | 連続値 |
| 確率質量関数 | PMF | 離散確率 |
| 確率密度関数 | PDF | 連続密度 |
| 累積分布関数 | CDF | P(X ≤ x) |
| 逆関数 | Inverse CDF | 分位点 |
| 期待値 | Expected value | 平均 E[X] |
| 分散 | Variance | E[(X-μ)²] |
| 標準偏差 | Standard deviation | √Var |
| モーメント | Moment | E[X^n] |
| 中心モーメント | Central moment | E[(X-μ)^n] |
| 正規分布 | Normal distribution | ベル曲線 |
| 標準正規分布 | Standard normal | N(0,1) |
| 二項分布 | Binomial | B(n,p) |
| ポアソン分布 | Poisson | Pois(λ) |
| 幾何分布 | Geometric | Geom(p) |
| t 分布 | Student's t | 小標本用 |
| χ² 分布 | Chi-square | 適合度用 |
| F 分布 | F-distribution | 分散比 |
| 一様分布 | Uniform | U(a,b) |
| 指数分布 | Exponential | Exp(λ) |
| ガンマ分布 | Gamma | 一般化指数 |
| ベータ分布 | Beta | 0-1 区間 |
| 対数正規分布 | Lognormal | 対称でない |

### 15.23.3 仮説検定（25 語）

| 用語 | 英語 | 意味 |
|---|---|---|
| 帰無仮説 | Null hypothesis | H₀ |
| 対立仮説 | Alternative | H₁ |
| 有意水準 | Significance level | α |
| p 値 | p-value | H₀ 真での確率 |
| 検出力 | Power | 1-β |
| Type I エラー | Type I error | 偽陽性 |
| Type II エラー | Type II error | 偽陰性 |
| 片側検定 | One-sided test | 方向あり |
| 両側検定 | Two-sided test | 方向なし |
| 効果量 | Effect size | 差の大きさ |
| 検定統計量 | Test statistic | t, z, F, χ² |
| 棄却域 | Rejection region | 棄却範囲 |
| 信頼区間 | CI | 推定範囲 |
| 多重比較 | Multiple comparison | 複数検定 |
| Bonferroni | Bonferroni | α/m |
| Holm | Holm | 段階的 |
| FDR | False discovery rate | 偽発見率 |
| 検出力分析 | Power analysis | サンプルサイズ |
| サンプルサイズ | Sample size | n |
| 対応あり | Paired | 同一対象 |
| 対応なし | Unpaired | 別対象 |
| 等分散 | Equal variance | σ₁=σ₂ |
| 非等分散 | Unequal variance | σ₁≠σ₂ |
| 正規性 | Normality | 正規分布 |
| 等分散性 | Homoscedasticity | 分散均一 |

### 15.23.4 回帰分析（25 語）

| 用語 | 英語 | 意味 |
|---|---|---|
| 単回帰 | Simple regression | 1 説明変数 |
| 重回帰 | Multiple regression | 2+ 説明変数 |
| 目的変数 | Response variable | y |
| 説明変数 | Predictor | x |
| 回帰係数 | Regression coefficient | a, b |
| 切片 | Intercept | a |
| 傾き | Slope | b |
| 残差 | Residual | y - ŷ |
| 決定係数 | R² | 当てはまり |
| 調整済み R² | Adjusted R² | 変数数補正 |
| 標準誤差 | SE | 推定の誤差 |
| t 値 | t-value | 係数/SE |
| F 値 | F-value | モデル全体 |
| 多重共線性 | Multicollinearity | 変数間の相関 |
| VIF | Variance Inflation Factor | 多重共線性指標 |
| 自己相関 | Autocorrelation | 残差の相関 |
| 不均一分散 | Heteroscedasticity | 分散の非一様 |
| 外れ値 | Outlier | 極端な値 |
| 影響点 | Influential point | 回帰への影響大 |
| テコ比 | Leverage | 影響度 |
| Cook 距離 | Cook's distance | 影響度指標 |
| 予測 | Prediction | 新データ推定 |
| 外挿 | Extrapolation | 範囲外予測 |
| 内挿 | Interpolation | 範囲内推定 |
| 非線形回帰 | Nonlinear regression | 曲線 |

### 15.23.5 多変量・生存・メタ（20 語）

| 用語 | 英語 | 意味 |
|---|---|---|
| 主成分分析 | PCA | 次元圧縮 |
| 因子分析 | Factor analysis | 潜在因子 |
| クラスタリング | Clustering | グループ化 |
| k-means | k-means | 分割クラスタ |
| 階層クラスタ | Hierarchical | 樹形図 |
| Ward 法 | Ward's method | クラスタ併合 |
| 決定木 | Decision tree | 木構造 |
| ランダムフォレスト | Random forest | 決定木の集合 |
| 生存分析 | Survival analysis | 時間とイベント |
| 打ち切り | Censoring | 観測途中終了 |
| 生存関数 | Survival function | S(t) |
| ハザード関数 | Hazard function | h(t) |
| ハザード比 | HR | h₁/h₀ |
| ログランク検定 | Log-rank | 生存曲線比較 |
| Cox 回帰 | Cox PH | 共変量 |
| メタアナリシス | Meta-analysis | 複数研究統合 |
| 固定効果 | Fixed effect | 同じ真値 |
| ランダム効果 | Random effect | 効果の分布 |
| 異質性 | Heterogeneity | I² |
| 出版バイアス | Publication bias | Egger |

---

## 15.24 ハードウェア用語の拡張（60 語）

### 15.24.1 ESP32-S3 関連（20 語）

| 用語 | 意味 |
|---|---|
| MCU | マイクロコントローラ |
| SoC | システムオンチップ |
| Xtensa | CPU アーキテクチャ |
| LX7 | ESP32-S3 の CPU コア |
| Dual Core | 2 コア |
| Core 0 | プロトコルコア |
| Core 1 | アプリケーションコア |
| FreeRTOS | リアルタイム OS |
| Task | タスク（スレッド） |
| ISR | 割り込みハンドラ |
| GPIO | 汎用入出力 |
| PWM | パルス幅変調 |
| ADC | AD 変換 |
| DAC | DA 変換 |
| I2C | 2 線式シリアル |
| SPI | 4 線式シリアル |
| UART | 非同期シリアル |
| I2S | 音声用シリアル |
| PSRAM | 外部 RAM |
| Flash | 不揮発メモリ |

### 15.24.2 メモリ関連（15 語）

| 用語 | 意味 |
|---|---|
| SRAM | 静的 RAM（高速） |
| DRAM | 動的 RAM |
| IRAM | 命令 RAM |
| DRAM | データ RAM |
| RTC RAM | 低電力保持 RAM |
| NVS | 不揮発ストレージ |
| Heap | 動的確保領域 |
| Stack | スタック領域 |
| Static | 静的領域 |
| PROGMEM | Flash 上の定数 |
| Partition | パーティション |
| Bootloader | 起動コード |
| OTA | 無線更新 |
| SPIFFS | SPI Flash FS |
| FAT32 | ファイルシステム |

### 15.24.3 表示関連（15 語）

| 用語 | 意味 |
|---|---|
| LCD | 液晶ディスプレイ |
| TFT | 薄膜トランジスタ |
| IPS | 広視野角 |
| ST7789V2 | LCD コントローラ |
| RGB565 | 16bit カラー |
| Font | フォント |
| Sprite | 描画バッファ |
| Backlight | バックライト |
| PWM 調光 | 明るさ制御 |
| Pixel | 画素 |
| Resolution | 解像度 |
| Aspect | アスペクト比 |
| Refresh | リフレッシュ |
| FPS | フレームレート |
| Dithering | ディザリング |

### 15.24.4 通信関連（10 語）

| 用語 | 意味 |
|---|---|
| Wi-Fi | 無線 LAN |
| BLE | Bluetooth Low Energy |
| USB CDC | USB シリアル |
| USB MSC | USB マスストレージ |
| USB DFU | USB ファーム更新 |
| MAC | 物理アドレス |
| IP | インターネットプロトコル |
| SSID | ネットワーク名 |
| AP | アクセスポイント |
| RSSI | 受信強度 |

---

## 15.25 ソフトウェア用語の拡張（40 語）

### 15.25.1 開発環境（10 語）

| 用語 | 意味 |
|---|---|
| Arduino IDE | Arduino 統合開発環境 |
| PlatformIO | クロスプラットフォーム開発 |
| arduino-cli | コマンドライン版 |
| FQBN | ボード識別子 |
| Library Manager | ライブラリ管理 |
| Board Manager | ボード管理 |
| Sketch | Arduino ソース |
| Serial Monitor | シリアルモニタ |
| ESP-IDF | ESP32 公式 SDK |
| Toolchain | ツールチェーン |

### 15.25.2 ビルド関連（10 語）

| 用語 | 意味 |
|---|---|
| Compile | コンパイル |
| Link | リンク |
| Object | オブジェクトファイル |
| Binary | バイナリ |
| Flash | Flash 書き込み |
| Bin | バイナリファイル |
| ELF | Executable format |
| MAP | メモリマップ |
| ctags | シンボル抽出 |
| Prototype | 関数プロトタイプ |

### 15.25.3 バージョン管理（10 語）

| 用語 | 意味 |
|---|---|
| Git | 分散型バージョン管理 |
| GitHub | Git ホスティング |
| Commit | コミット |
| Branch | ブランチ |
| Merge | マージ |
| Tag | タグ |
| Release | リリース |
| Issue | 課題管理 |
| Pull Request | PR |
| Semantic Version | SemVer |

### 15.25.4 ドキュメント（10 語）

| 用語 | 意味 |
|---|---|
| Markdown | 軽量マークアップ |
| docsify | ドキュメント生成 |
| GitHub Pages | 静的サイト公開 |
| README | 概要ファイル |
| CHANGELOG | 変更履歴 |
| LICENSE | ライセンス |
| .gitignore | 無視ファイル指定 |
| .nojekyll | Jekyll 無効化 |
| Sidebar | サイドバー |
| TOC | 目次 |

---

## 15.26 数学記号の拡張

### 15.26.1 論理記号

| 記号 | 名称 | 意味 |
|---|---|---|
| ∀ | 全称記号 | すべての |
| ∃ | 存在記号 | 存在する |
| ∄ | 非存在 | 存在しない |
| ∧ | 論理積 | AND |
| ∨ | 論理和 | OR |
| ¬ | 否定 | NOT |
| → | 含意 | ならば |
| ↔ | 同値 | 必要十分 |
| ⊨ | 論理的含意 | モデルで真 |
| ⊢ | 証明可能 | 導出可能 |

### 15.26.2 集合記号

| 記号 | 名称 | 意味 |
|---|---|---|
| ∈ | 要素 | 属する |
| ∉ | 非要素 | 属さない |
| ⊂ | 真部分集合 | 含まれる |
| ⊆ | 部分集合 | 含まれるか等しい |
| ⊃ | 真上位集合 | 含む |
| ⊇ | 上位集合 | 含むか等しい |
| ∪ | 和集合 | 合併 |
| ∩ | 積集合 | 共通部分 |
| ∅ | 空集合 | 要素なし |
| ℕ | 自然数 | {0,1,2,...} |
| ℤ | 整数 | {...,-1,0,1,...} |
| ℚ | 有理数 | p/q |
| ℝ | 実数 | 連続直線 |
| ℂ | 複素数 | a+bi |
| |A| | 濃度 | 元の個数 |
| A×B | 直積 | 順序対の集合 |
| P(A) | 冪集合 | 部分集合全体 |

### 15.26.3 微積分記号

| 記号 | 名称 | 意味 |
|---|---|---|
| dy/dx | 微分 | 変化率 |
| ∂f/∂x | 偏微分 | 一変数微分 |
| ∇f | 勾配 | 傾きベクトル |
| ∇·F | 発散 | div |
| ∇×F | 回転 | curl |
| ∫ | 不定積分 | 原始関数 |
| ∫ₐᵇ | 定積分 | 区間積分 |
| ∮ | 周回積分 | 閉曲線 |
| ∬ | 二重積分 | 面積分 |
| Σ | 総和 | シグマ |
| Π | 総乗 | パイ |
| lim | 極限 | 近づく値 |
| ∞ | 無限大 | 無限 |
| ≈ | 漸近 | 近づく |
| ∼ | 同程度 | オーダー |
| O(f) | ビッグオー | 上界 |
| o(f) | スモールオー | より小 |
| Θ(f) | シータ | 上下界 |

---

## 15.27 この章の追加まとめ

- 統計用語 120 語を 5 分類で網羅（記述/分布/検定/回帰/多変量）
- ハードウェア用語 60 語（ESP32/メモリ/表示/通信）
- ソフトウェア用語 40 語（開発/ビルド/バージョン/ドキュメント）
- 数学記号の拡張（論理/集合/微積分）

### 確認問題（追加）

Q15-7: `IQR` の意味は？ → Interquartile Range（Q3 - Q1）

Q15-8: `VIF` が高いと何が問題？ → 多重共線性（回帰の不安定化）

Q15-9: `I²` が 75% 超のとき何を意味する？ → 研究間の異質性が高い

Q15-10: `RGB565` は何色表現？ → 16 bit、65,536 色

Q15-11: `SemVer` の形式は？ → MAJOR.MINOR.PATCH

Q15-12: `∇×F` は何を表す？ → ベクトル場の回転（curl）

---

[← 第14章 エラーメッセージ](14_errors.md) | [目次](#/) | [第16章 実用例集 →](16_examples.md)
