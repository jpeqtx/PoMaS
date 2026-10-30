$path = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\MANUAL.md"

$content = @'
# PoMaS 操作マニュアル v0.92

**M5Stack Cardputer-Adv 用 多機能数学ステーション 完全版**

---

## 目次

1. はじめに
2. 動作環境・インストール
3. 基本操作
4. Calculator モード
5. CAS モード
6. Statistics モード
7. Scientific Reference モード
8. Graph モード
9. Tools モード
10. 設定（NVS 永続化）
11. トラブルシューティング
12. 付録（関数一覧 / キー早見表）

---

## 1. はじめに

PoMaS（Pocket Math Station）は、M5Stack Cardputer-Adv 上で動作する多機能関数電卓です。
数式計算・CAS・統計解析・グラフ描画・科学資料検索・μLisp インタプリタを 1 台に統合しています。

設計思想：
- **1 画面 1 機能** — 情報過多を避け、迷わない UI
- **キーボード完結** — タッチ不要、全ての操作が物理キーで完結
- **Paper UI v2** — 淡ミント紙背景 + グラファイト文字で長時間作業に優しい

---

## 2. 動作環境・インストール

### 2.1 ハードウェア要件

| 項目 | 要件 |
|---|---|
| デバイス | M5Stack Cardputer-Adv（ESP32-S3FN8） |
| Flash | 8 MB |
| PSRAM | 非搭載（200 要素までの統計リスト） |
| 画面 | ST7789V2 240×135 px |
| SD カード | 任意（FAT32, 32GB 以下推奨） |

### 2.2 ファームウェア書き込み

1. [esptool](https://github.com/espressif/esptool) をインストール
2. [Releases](https://github.com/jpeqtx/PoMaS/releases) から `PoMaS_v0.92_Cardputer-Adv.bin` をダウンロード
3. Cardputer-Adv を USB-C で接続
4. 書き込み：

```bash
esptool.py --chip esp32s3 --port COM3 --baud 921600 \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB \
  0x0 PoMaS_v0.92_Cardputer-Adv.bin
```

`COM3` は環境に合わせて変更。G0 ボタンを押しながらリセットでダウンロードモードに入れます。

### 2.3 起動

リセット → スプラッシュ（約 2.5 秒）→ メインメニュー

---

## 3. 基本操作

### 3.1 共通キー

| キー | 動作 |
|---|---|
| `Fn + Esc`（バッククォート） | 戻る / キャンセル |
| `Enter` | 決定 / 実行 |
| `Del` | バックスペース |
| `Fn + 数字` | 各モードのショートカット |
| `;` | 上 |
| `.` | 下 |
| `,` | 左 |
| `/` | 右 |

### 3.2 メインメニュー

1. **Calculator** — 数式電卓
2. **CAS** — 数式処理
3. **Statistics** — 統計解析
4. **Scientific Reference** — 科学資料
5. **Graph** — グラフ描画
6. **Tools** — スプレッドシート / μLisp / Wi-Fi
7. **About** — バージョン情報

---

## 4. Calculator モード

### 4.1 基本計算

式を入力して `Enter`。対応構文：

| 演算子 / 関数 | 例 |
|---|---|
| 四則演算 | `2+3*4` → 14 |
| べき乗 | `2^10` → 1024 |
| 階乗 | `5!` → 120 |
| 剰余 | `10 mod 3` → 1 |
| 平方根 | `sqrt(2)` |
| 三角関数 | `sin(pi/2)`, `cos(60)`, `tan(45)` |
| 対数 | `ln(e)`, `log(100)`, `log2(8)` |
| 組合せ | `nCr(5,2)`, `nPr(5,2)` |
| 素数判定 | `isPrime(97)` |
| 文字定数 | `pi`, `e`, `euler` |
| 変数 | `A`〜`J`, `E` は変数として使用可 |
| 直前の結果 | `Ans` |

### 4.2 ショートカット

| キー | 機能 |
|---|---|
| `Fn+1` | 設定（角度・表示形式・桁数） |
| `Fn+2` | 履歴呼び出し（最大 100 件） |
| `Fn+3` | メモリ M（M+ / M- / MR / MC / Set） |
| `Fn+4` | 変数 A〜J |
| `Fn+5` | 基数変換（DEC / HEX / OCT / BIN） |
| `Fn+6` | ビット演算（AND / OR / XOR / NOT / SHL / SHR） |
| `Fn+7` | 複素数演算（加減乗除 / 極形式 / 累乗 / 関数） |
| `Fn+8` | 拡張ツール（下記） |
| `Fn+9` | RPN モード切替 |
| `Fn+0` | ヘルプ |
| `Fn+=` | 分数 ⇄ 小数表示 |
| `Fn+a` | Ans を式に挿入 |
| `Fn+b` | Ans バンク（最大 20 件） |

### 4.3 拡張ツール（Fn+8）

| 機能 | 内容 |
|---|---|
| 方程式ソルバー | Newton / 二分法 / 2〜4 次方程式 / 連立 1 次方程式 / 一般多項式（deg ≤ 8） |
| 数値解析 | 微分 / 積分 / 総和 / 総乗 |
| ベクトル・行列 | ドット積 / クロス積 / ノルム / 加減算 / 行列乗算 / 行列式 / 逆行列 / 転置 / 階数 / 固有値 |
| 幾何（27 図形） | 三角形 3 種 / 四角形 5 種 / 円 / 扇形 / 楕円 / 環 / 正多角形 / 立体 8 種 / 正多面体 5 種 / トーラス |
| 確率シミュレーション | コイン / サイコロ / ガチャ / 乱数 / 正規乱数 / 指数乱数 / 誕生日パラドックス |
| 素因数分解 | 64 ビットまでの整数 |

---

## 5. CAS モード

多項式の記号処理。`x^3 - 2x + 1` のような式を入力。

| 機能 | 説明 |
|---|---|
| Eval at x | 多項式の x 指定評価 |
| Differentiate | 1 階導関数 |
| Integrate | 不定積分（+ C） |
| Add | 2 多項式の和 |
| Multiply | 2 多項式の積 |
| Roots | 根（複素数対応） |
| Taylor | 任意関数の数値テイラー展開（次数 1〜7） |
| Limit | 数値極限（両側極限の一致判定） |
| Substitute | 記号置換（例: `x` → `y+1`） |

多項式の書式例：
- `1+2x-x^2`
- `3x^2-2x+1`
- `x^3-x`

---

## 6. Statistics モード

### 6.1 Lists & Descriptive

- L1〜L8 の 8 リスト、各 200 要素（PSRAM 搭載時 999）
- 操作：`Enter`（編集）/ `Del`（削除）/ `S`（ソート）/ `A`（追加）
- 追加時はカンマ区切りで複数値入力可：`1, 2, 3, 4`
- 記述統計ページ 1：n, sum, sum2, mean, sd(samp), sd(pop), var(samp), SE, CV, min, max
- 記述統計ページ 2：Q1, med, Q3, IQR, skew, kurt, mode

### 6.2 Variable Scale

各リストに尺度を設定：比例 / 間隔 / 順序 / 名義

### 6.3 特殊平均

- 重み付き平均（L1=値, L2=重み）
- 幾何平均
- 調和平均
- トリム平均（alpha 0〜0.5）

### 6.4 Frequency Distribution

ビン数指定でヒストグラム + 頻度表を表示。

### 6.5 Z-Scores

指定リストの Z-score を別リストに保存。

### 6.6 Regression（回帰）

| 種類 | 式 |
|---|---|
| Linear | y = a + bx |
| Quadratic | y = a + bx + cx² |
| Power | y = a · x^b |
| Exponential | y = a · e^(bx) |
| Logarithmic | y = a + b·ln(x) |
| Logistic | logit(y) = a + bx |
| Med-Med | 中央値ベースの線形回帰 |
| Multiple | 多重線形回帰（k ≤ 5） |
| Stepwise | 変数増加法による変数選択 |

出力：a, b, c, r, r², 予測値。残差の保存も可能。

### 6.7 Tests（仮説検定）

| 検定 | 内容 |
|---|---|
| z-test (1-sample) | σ 既知の 1 標本検定 |
| t-test (1-sample) | 1 標本 t 検定 |
| t-test (2-sample, eq.) | 等分散 2 標本 t 検定 |
| Welch's t-test | 不等分散 2 標本 t 検定 |
| Paired t-test | 対応あり t 検定 |
| F-test | 分散比検定 |
| One-way ANOVA | 一元配置分散分析 |
| Two-way ANOVA | 二元配置分散分析 |
| Chi-Square GOF | 適合度検定 |
| Chi-Square Independence | 独立性検定（r×c） |
| Fisher's exact (2×2) | フィッシャー正確検定 |

出力：検定統計量、df、p 値、有意判定（α=0.05）

### 6.8 Non-parametric / Risk

| 機能 | 内容 |
|---|---|
| Mann-Whitney U | ノンパラ 2 群比較 |
| Wilcoxon signed rank | 対応ありノンパラ検定 |
| Kruskal-Wallis | ノンパラ 3 群以上 |
| Friedman | 反復測定ノンパラ |
| Sign test | 符号検定 |
| Spearman correlation | 順位相関 |
| Kendall tau | 順位相関（ケンドール） |
| Odds Ratio + 95% CI | オッズ比 |
| Risk Ratio + 95% CI | リスク比 |
| Risk Difference + 95% CI | リスク差 |
| NNT | 治療必要数 |
| CI (mean) | 平均の信頼区間 |
| CI (proportion, Wilson) | 比率の信頼区間 |

### 6.9 Distributions（分布ブラウザ）

- 正規 N(μ, σ)
- t 分布（df）
- χ² 分布（df）
- F 分布（d1, d2）
- 二項（n, p）
- ポアソン（λ）
- 幾何（p）

各分布で：PMF/PDF、CDF、逆関数、PDF プロット

### 6.10 Multivariate（多変量解析）

| 機能 | 内容 |
|---|---|
| PCA | 主成分分析（固有値・寄与率・負荷量） |
| Factor Analysis | 因子分析（Varimax 回転） |
| k-means | k 平均クラスタリング |
| Hierarchical (Ward) | 階層クラスタリング |
| Decision Tree | CART 回帰木 |

### 6.11 Stratified（層別解析）

- Mantel-Haenszel OR（per-stratum + pooled + Forest plot）
- Mantel-Haenszel RR
- Food Poisoning Trace（食中毒原因食品特定）
- Cochran's Q（異質性検定）
- Breslow-Day 検定

### 6.12 Survival（生存分析）

- Kaplan-Meier（生存曲線）
- Log-rank 検定
- Cox 比例ハザード（1 共変量）
- Median Survival（生存中央値）

### 6.13 Meta-analysis（メタアナリシス）

- Inverse variance（固定効果）
- DerSimonian-Laird（ランダム効果）
- Forest plot
- Funnel plot
- Egger's test（出版バイアス）

### 6.14 Statistical Graphs（17 種）

Histogram / Box-and-Whisker / Scatter / Q-Q / Bar / Pie / Line / Stem-and-leaf / Pareto / ECDF / KDE / Strip / Cumulative / Mean±SD / Residuals / P-P / Multi-line

### 6.15 CSV Stream Stats

SD カードの CSV を行単位でストリーミング処理。列ごとの統計 + 指定 2 列の回帰。

---

## 7. Scientific Reference モード

| カテゴリ | 内容 |
|---|---|
| Constants（386 件） | 物理 / 数学 / EE / 天文 / 惑星 / 準惑星 / 地球物理 / 大気 / 海洋 / 元素 / 生物 |
| Scientific Laws（300 件） | 物理 / 化学 / 数学 / 地球科学 / 天文 / 生物 / EE |
| Unit Converter（532 単位 / 20 カテゴリ） | 長さ / 面積 / 体積 / 質量 / 速度 / 加速度 / 力 / 圧力 / エネルギー / 仕事率 / 温度 / 角度 / 時間 / 情報量 / 周波数 / 流量 / 密度 / 粘度 / 放射線 / その他 |
| Periodic Table（118 元素） | 元素記号・原子量。`Y` で Ans に転送、`B` で Ans バンクへ |
| Search by Symbol | 記号・名前で定数検索 |

---

## 8. Graph モード

### 8.1 メニュー

| 項目 | 機能 |
|---|---|
| Plot current functions | 現在の関数をプロット |
| Edit functions | 最大 4 本の関数編集 |
| Plot type | 描画タイプ選択 |
| Range / Window | 範囲設定 |
| Standard window | -10〜10 |
| Trig window | -2π〜2π |
| **Square aspect (1:1)** | 真円表示用に Y 範囲を自動調整 |
| G-Solve | 根 / 極値 / 交点 / 定積分 |
| Tangent / Normal | 接線・法線 |
| Save plot as BMP | BMP 保存 |

### 8.2 Plot type

| タイプ | 入力例 |
|---|---|
| y = f(x) | `sin(x)` |
| Parametric | `cos(t);sin(t)` （セミコロン区切り） |
| Polar | `1+cos(t)` |
| Implicit | `x^2+y^2-4` |
| Inequality | `x^2+y^2-1` （f(x,y) > 0 領域塗り） |
| Sequence | `1/n` （n を使用） |
| 3D | `sin(x)*cos(y)` |

### 8.3 プロット中の操作

| キー | 動作 |
|---|---|
| `Z` | ズームイン |
| `X` | ズームアウト |
| `R` | 標準ウィンドウ |
| `T` | トレース ON/OFF |
| `B` | ボックスズーム |
| `+` | 次関数に切替 |
| `←` / `→` | トレースカーソル移動 |
| `S` | BMP 保存 |

### 8.4 G-Solve

- Root（符号変化スキャン + 二分法）
- Maximum（極大）
- Minimum（極小）
- Intersection（2 関数の交点）
- y at x（x 指定で y を計算）
- x at y（y 指定で x を計算）
- Definite Integral（定積分）

---

## 9. Tools モード

### 9.1 Spreadsheet（45×26）

- セル参照：`A1`, `B3` など
- 数式：`=A1+B1`, `=A1*2` など
- `Enter` 編集 / `Del` 消去
- `S` CSV 保存 / `L` CSV 読み込み
- `T` Sheet ⇄ L1..L8 相互変換

### 9.2 uLisp Interpreter

Scheme 風の Lisp サブセット。

**利用可能な特殊フォーム**：
- `define`, `setq`, `lambda`, `let`
- `if`, `cond`, `and`, `or`, `not`
- `while`, `begin`, `progn`, `quote`

**ビルトイン関数**：
- 算術：`+ - * / mod`
- 比較：`< > <= >= =`
- リスト：`car cdr cons list null? atom? length append reverse nth`
- 高階：`map filter reduce apply`
- 数学：`sin cos tan exp log sqrt abs floor ceil round expt pow`
- 統計：`mean sum sd`
- その他：`print save load euler pi random`

**実行例**：

```lisp
(define (make-adder n) (lambda (x) (+ x n)))
(define add5 (make-adder 5))
(add5 3)   ; => 8

(map (lambda (x) (* x x)) '(1 2 3 4 5))   ; => (1 4 9 16 25)
(reduce + 0 '(1 2 3 4 5))                  ; => 15
```

**GC（ガベージコレクション）**：
- セルプール：2500 個
- 明示的 `(gc)` は無効化（安全性のため削除）
- 枯渇時：**Tools → uLisp → Run GC now** を実行
- または **Reset interpreter**

### 9.3 Wi-Fi / Maxima

- SSID / Pass 設定
- ネットワークスキャン（最大 30 件）
- 接続 / 切断 / ステータス
- Maxima 連携（プロキシ URL は各自設定）

### 9.4 Power / About

- バッテリー残量・充電状態
- PSRAM 有無
- SD カード状態
- Wi-Fi 状態
- Free heap
- `R`：SD 再試行 / `T`：SD テスト

### 9.5 Settings（NVS 永続化）

- Angle: DEG / RAD / GRAD
- Display: Norm / Fix / Sci / Eng
- 桁数（0〜9）
- Norm 1 / 2 切替
- Save settings now
- Clear NVS

---

## 10. 設定（NVS 永続化）

設定は不揮発性ストレージ（NVS）に保存され、再起動後も維持されます。
変更後は **Save settings now** を実行してください。
60 秒ごとに自動保存も走ります。

---

## 11. トラブルシューティング

### 11.1 起動しない / 画面が真っ暗

- バッテリー残量を確認（USB 充電）
- G0 ボタン + リセットで再起動
- USB シリアルで bootlog を確認（115200 bps）

### 11.2 SD カードが認識されない

- FAT32 形式、32GB 以下を使用
- 接点を清掃
- `Tools → Power → R` でリトライ
- `Tools → Power → T` で読み書きテスト

### 11.3 μLisp で `ERR: unbound symbol`

- セルプール枯渇の可能性
- **Tools → uLisp → Run GC now**
- または **Reset interpreter**

### 11.4 Wi-Fi に接続できない

- 2.4GHz 帯のみ対応（5GHz 不可）
- SSID / Pass を再確認
- ルーターの MAC フィルタを確認

### 11.5 グラフが楕円になる

- **Square aspect (1:1)** を選択
- または Range を手動調整

### 11.6 統計リストの上限

- PSRAM 非搭載のため 200 要素まで
- 999 要素は PSRAM 搭載デバイスが必要

---

## 12. 付録

### 12.1 キー早見表

| キー | 機能 |
|---|---|
| `Fn + Esc` | 戻る / キャンセル |
| `Enter` | 決定 / 実行 |
| `Del` | バックスペース |
| `;` | 上 |
| `.` | 下 |
| `,` | 左 |
| `/` | 右 |

### 12.2 組み込み関数一覧（Calculator）

`sin, cos, tan, asin, acos, atan, sinh, cosh, tanh, asinh, acosh, atanh, csc, sec, cot, csch, sech, coth, exp, ln, log, log10, log2, sqrt, cbrt, abs, sign, sgn, floor, ceil, round, int, frac, fact, gamma, lgamma, erf, erfc, deg, rad, isPrime, prime, nextPrime, prevPrime, random, randInt, rint, randN, toF, toC, toK, pow, nrt, atan2, hypot, min, max, mod, gcd, lcm, nPr, nCr, logn, randR, if, clamp, dms`

### 12.3 定数

- `pi` — 円周率
- `e` — オイラー数
- `euler` — オイラー数（エイリアス）
- `phi` — 黄金比
- `gamma` — オイラー・マスケローニ定数
- `inf` — 無限大
- `nan` — NaN
- `ans` — 直前の結果
- `mem` — メモリ M

### 12.4 変数

- `A`〜`J` — ユーザー変数（大文字）
- `a`〜`j` — ユーザー変数（小文字）
- `E` — 変数 E（v0.92 以降）
- `x`, `t`, `n` — 数式内の予約変数（未定義時 0）

---

**PoMaS v0.92 — Paper UI v2**
**Author: jpeqtx**
'@

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText($path, $content, $utf8NoBom)

Write-Host "Created: $path" -ForegroundColor Green
Write-Host ("Size: " + (Get-Item $path).Length + " bytes") -ForegroundColor Cyan
Write-Host ("Lines: " + (Get-Content $path).Count) -ForegroundColor Cyan