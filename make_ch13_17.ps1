$base = "C:\Users\YNGMT\Documents\Arduino\pomas_0_92\docs\manual"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

# ========== 第13章 設定 ==========
$ch13 = @'
# 第13章 設定

[← 第12章 ツール](12_tools.md) | [目次](index.md) | [第14章 エラーメッセージ →](14_errors.md)

---

## 13.1 設定とは

PoMaS の動作をカスタマイズする項目です。
変更した設定は **NVS（不揮発性ストレージ）** に保存され、再起動後も維持されます。

### アクセス方法

**方法 1**: メインメニュー → `6 Tools` → `5 Settings`
**方法 2**: Calculator 画面で `Fn + 1`

### 設定画面

```
┌─────────────────────────────────┐
│▌Settings                        │
├─────────────────────────────────┤
│1 Angle: DEG/RAD/GRAD            │
│2 Display: Norm/Fix/Sci/Eng      │
│3 Display digits                 │
│4 Save settings now              │
│5 Reset all (clear NVS)          │
│6 Back                           │
└─────────────────────────────────┘
```

---

## 13.2 Angle（角度モード）

三角関数の角度単位を選択。

| モード | 単位 | 1 周 | 用途 |
|---|---|---|---|
| **DEG** | 度数法 | 360° | 日常的（デフォルト） |
| **RAD** | 弧度法 | 2π ≈ 6.28 | 数学・物理 |
| **GRAD** | グラジアン | 400 | 測量 |

### 例: sin(30) の結果

| モード | sin(30) |
|---|---|
| DEG | 0.5 |
| RAD | -0.988 |
| GRAD | 0.454 |

### 切り替え手順

```
[1] Settings → 1 (Angle)
[2] DEG / RAD / GRAD を選択
[3] 変更は即座に反映
```

ヘッダのバッジで現在のモードを確認できます。

---

## 13.3 Display（表示形式）

数値の表示形式を選択。

| 形式 | 例 | 説明 |
|---|---|---|
| **Norm** | 1.23456 | 通常（デフォルト） |
| **Fix** | 1.23 | 固定小数点 |
| **Sci** | 1.23e+00 | 科学表記 |
| **Eng** | 1.23k | 工学表記（3 桁区切り） |

### Norm 1 / Norm 2 の違い

現在のバージョンでは、Display が Norm のとき `Norm 1` / `Norm 2` の
閾値設定があります。

| モード | 小さい数の閾値 | 例（0.005） |
|---|---|---|
| Norm 1 | 0.01 未満で指数表示 | 5e-3 |
| Norm 2 | 1e-9 未満で指数表示 | 0.005 |

---

## 13.4 Display digits（表示桁数）

`3 (Display digits)` で 0〜9 の範囲で設定。

| 桁数 | 例（π） |
|---|---|
| 3 | 3.142 |
| 5 | 3.14159 |
| 7 | 3.1415927 |
| 9 | 3.141592654 |

**クイック操作**:
- `Fn + ,` — 1 桁増やす
- `Fn + /` — 1 桁減らす

---

## 13.5 Save settings now（手動保存）

`4 (Save settings now)` で NVS に即時保存。

**自動保存**:
PoMaS は **60 秒ごと** に自動保存も行いますが、
重要な設定変更後は手動保存を推奨。

---

## 13.6 Reset all（NVS クリア）

`5 (Reset all)` で全設定を初期化。

```
[1] Reset all を選択
[2] "Clear NVS settings?" の確認
[3] Y で実行
→ 全設定がデフォルトに戻る
```

⚠️ **確認必須**
誤って実行すると、以下がすべてリセットされます：
- Angle / Display / 桁数
- Wi-Fi の SSID / Pass
- RPN モード / 分数モード

---

## 13.7 NVS とは

**NVS（Non-Volatile Storage）** は、電源を切っても
データが保持されるストレージ領域です。

### NVS の仕組み

```
[1] 設定変更
[2] NVS に書き込み
[3] 電源 OFF
[4] 電源 ON → NVS から読み込み
[5] 設定が復元される
```

### 保存される項目

| キー | 内容 |
|---|---|
| angle | 角度モード |
| disp | 表示形式 |
| digits | 表示桁数 |
| norm | Norm 1/2 |
| rpn | RPN モード |
| frac | 分数モード |
| wifissid | Wi-Fi SSID |
| wifipass | Wi-Fi Pass |

---

## 13.8 この章のまとめ

- 設定は Tools → 5、または Calculator で Fn+1
- 角度モード（DEG/RAD/GRAD）
- 表示形式（Norm/Fix/Sci/Eng）
- 表示桁数（0〜9）
- NVS に永続化、60 秒ごとに自動保存
- Reset all で全設定クリア

### 確認問題

📝 **Q13-1**: 角度モードを切り替えるには？
<details><summary>答え</summary>
Tools → 5 → 1 (Angle)、またはヘッダのバッジを確認。
</details>

📝 **Q13-2**: 表示桁数を 1 桁増やすクイックキーは？
<details><summary>答え</summary>
Fn + ,（カンマ）
</details>

📝 **Q13-3**: NVS とは何の略？
<details><summary>答え</summary>
Non-Volatile Storage（不揮発性ストレージ）
</details>

---

[← 第12章 ツール](12_tools.md) | [目次](index.md) | [第14章 エラーメッセージ →](14_errors.md)
'@

[System.IO.File]::WriteAllText("$base\13_settings.md", $ch13, $utf8NoBom)
Write-Host "Created: 13_settings.md" -ForegroundColor Green

# ========== 第14章 エラーメッセージ一覧 ==========
$ch14 = @'
# 第14章 エラーメッセージ一覧

[← 第13章 設定](13_settings.md) | [目次](index.md) | [第15章 用語集 →](15_glossary.md)

---

## 14.1 この章の使い方

PoMaS が表示するエラーメッセージと、その対処法をまとめます。
エラーが出たら、まずこの章で該当メッセージを探してください。

---

## 14.2 Calculator のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `syntax` | 構文エラー | 括弧・演算子を確認 |
| `) expected` | 閉じ括弧がない | `)` を追加 |
| `trailing chars` | 式の後ろに不正な文字 | 余分な文字を削除 |
| `unknown id` | 未知の変数・関数 | 変数名・関数名を確認 |
| `unknown fn` | 未知の関数 | 関数名を確認 |
| `division by zero` | ゼロ除算 | 除数を確認 |
| `mod by zero` | mod のゼロ除算 | 除数を確認 |

### 例: `2+` の実行

```
→ エラー: syntax
```

**対処**: 式を完成させる（`2+3` など）。

### 例: `sin(30` の実行

```
→ エラー: ) expected
```

**対処**: `sin(30)` と閉じ括弧を追加。

---

## 14.3 μLisp のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `unbound symbol` | 未定義の変数・関数 | 先に define で定義 |
| `not callable` | 関数でないものの呼び出し | 引数を確認 |
| `if: bad args` | if の引数不足 | (if 条件 真 偽) の形式 |
| `if: no then` | if の真式なし | 真の場合の式を追加 |
| `define: bad args` | define の引数不正 | (define name value) の形式 |
| `define: no value` | define の値なし | 値を追加 |
| `define: bad target` | define の対象が不正 | シンボル名か関数定義形式 |
| `lambda: bad args` | lambda の引数不正 | (lambda (params) body) の形式 |
| `let: bad args` | let の引数不正 | (let ((var val)) body) の形式 |
| `while: bad args` | while の引数不正 | (while cond body) の形式 |
| `quote: no arg` | quote の引数なし | (quote expr) の形式 |
| `parse too deep` | パーサ再帰深度超過 | ネストを浅くする（200 段超） |
| `unexpected EOF` | 閉じ括弧不足 | `)` を追加 |
| `μLisp cell pool exhausted; run GC from menu` | セルプール枯渇 | Tools → uLisp → Run GC now |
| `env table full` | 環境変数表が満杯 | Reset interpreter |
| `Env limit` | Env プール上限 | Reset interpreter |
| `loop limit` | while のループ上限超過 | ループ条件を見直す |
| `missing arg` | 組み込み関数の引数不足 | 引数を追加 |
| `too many args` | 組み込み関数の引数過多 | 引数を減らす |
| `sym malloc fail` | シンボル用メモリ確保失敗 | メモリ不足、Reset interpreter |
| `str malloc fail` | 文字列用メモリ確保失敗 | メモリ不足、Reset interpreter |

### 例: `(car 1 2)` の実行

```
→ nil（エラーではなく nil を返す）
```

⚠️ **nil とエラーの区別**
一部の関数は不正な引数に対して nil を返します。
エラーメッセージが出ないからと言って、正しく動いているとは限りません。

---

## 14.4 Graph のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `No active fn` | 関数未登録 | Edit functions で登録 |
| `No fn` | 関数未登録 | 同上 |
| `No sign change` | 根が見つからない | 範囲を広げる |
| `No intersect` | 交点なし | 範囲を確認 |
| `Not found` | x at y の解なし | 範囲を広げる |
| `Singular` | 行列が特異 | 逆行列の存在確認 |
| `Square only` | 正方行列でない | 行列サイズ確認 |
| `Dim mismatch` | 次元不一致 | 行列・ベクトルのサイズ確認 |

### 例: G-Solve → Root で根が見つからない

```
→ No sign change
```

**対処**: Range を広げる、または初期関数の値域を確認。

---

## 14.5 Statistics のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `Empty` | データなし | リストにデータ入力 |
| `n<2` | データ件数不足 | 2 件以上必要 |
| `n<4` | 4 件以上必要 | データ追加 |
| `Mismatch` | リストサイズ不一致 | サイズを揃える |
| `Total = 0` | 合計がゼロ | データを確認 |
| `E > 0 required` | 期待度数が 0 以下 | 期待値を正に |
| `var > 0 required` | 分散が 0 以下 | データを確認 |
| `malloc fail` | メモリ確保失敗 | データを減らす |
| `Singular` | 行列が特異 | データを見直す |
| `Egger needs K>=3` | Egger 検定は K≥3 必要 | 研究を追加 |
| `RD = 0` | リスク差ゼロ（NNT計算不能） | データを確認 |

---

## 14.6 SD カードのエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `No SD` / `No SD card` | SD カード未挿入/未認識 | カード挿入、R でリトライ |
| `File not found` | ファイルなし | パスを確認 |
| `Open failed` | ファイルオープン失敗 | 書き込み権限確認 |
| `Not found` | パス不正 | パスを確認 |
| `Write open FAILED` | 書き込み失敗 | 容量・フォーマット確認 |
| `SD init failed` | SD 初期化失敗 | カードを挿し直す |

---

## 14.7 Wi-Fi のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `No SSID set` | SSID 未設定 | Set SSID/Pass |
| `SSID not found` | SSID が見つからない | 範囲・スペル確認 |
| `Auth failed` | 認証失敗 | パスワード確認 |
| `Timeout` | 接続タイムアウト | 電波状況確認 |
| `Not connected` | Wi-Fi 未接続 | Connect 実行 |
| `No networks` | ネットワークなし | スキャン再実行 |
| `HTTP X` | HTTP エラーコード | X の内容で判断 |

⚠️ **2.4GHz 帯のみ**
5GHz 帯のネットワークには接続できません。

---

## 14.8 その他のエラー

| メッセージ | 意味 | 対処 |
|---|---|---|
| `Bank full` | Ans バンク満杯 | エントリ削除 |
| `Bank empty` | Ans バンク空 | エントリ追加 |
| `PSRAM alloc failed` | 統計用メモリ確保失敗 | 再起動 |
| `Invalid k` | k の値が不正 | 範囲内で指定 |
| `n too large` | データ件数超過 | データ削減 |
| `1..10` | 範囲外の値 | 正しい範囲で入力 |
| `1..5` | 同上 | 同上 |
| `2..50` | 同上 | 同上 |

---

## 14.9 エラーが出ないのに動かない

エラーメッセージが出ないが、期待通りに動かない場合。

### 症状別チェック

| 症状 | チェック項目 |
|---|---|
| グラフが表示されない | 関数登録・Range・Plot type |
| 数式が計算されない | Enter キーを押したか |
| 入力した値が反映されない | Enter で確定したか |
| Ans が更新されない | 計算が成功したか |
| ボタンが効かない | Fn キーとの組み合わせ順序 |
| 画面がフリーズ | 長い計算中、ESC で中断 |

---

## 14.10 この章のまとめ

- エラーが出たら、まずメッセージを確認
- 構文エラー → 括弧・演算子
- μLisp エラー → 定義・構文
- SD / Wi-Fi エラー → 接続・設定
- エラーが出なくても動かない場合 → 入力確定・操作順序

### 確認問題

📝 **Q14-1**: `division by zero` が出たら？
<details><summary>答え</summary>
除数を確認。0 で割っていないかチェック。
</details>

📝 **Q14-2**: `unbound symbol` が出たら？
<details><summary>答え</summary>
変数・関数が未定義。先に define で定義する。
</details>

📝 **Q14-3**: `μLisp cell pool exhausted` が出たら？
<details><summary>答え</summary>
Tools → uLisp → Run GC now、または Reset interpreter。
</details>

📝 **Q14-4**: `No SD` が出たら？
<details><summary>答え</summary>
SD カードを挿入して、Tools → Power → R でリトライ。
</details>

---

[← 第13章 設定](13_settings.md) | [目次](index.md) | [第15章 用語集 →](15_glossary.md)
'@

[System.IO.File]::WriteAllText("$base\14_errors.md", $ch14, $utf8NoBom)
Write-Host "Created: 14_errors.md" -ForegroundColor Green

# ========== 第15章 用語集 ==========
$ch15 = @'
# 第15章 用語集

[← 第14章 エラーメッセージ](14_errors.md) | [目次](index.md) | [第16章 実用例集 →](16_examples.md)

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

## 15.12 この章のまとめ

- PoMaS で使われる用語を網羅
- 数学・統計・プログラミングの基礎用語
- アルファベット順索引付き

---

[← 第14章 エラーメッセージ](14_errors.md) | [目次](index.md) | [第16章 実用例集 →](16_examples.md)
'@

[System.IO.File]::WriteAllText("$base\15_glossary.md", $ch15, $utf8NoBom)
Write-Host "Created: 15_glossary.md" -ForegroundColor Green

# ========== 第16章 実用例集 ==========
$ch16 = @'
# 第16章 実用例集

[← 第15章 用語集](15_glossary.md) | [目次](index.md) | [第17章 付録 →](17_appendix.md)

---

## 16.1 この章の使い方

PoMaS の実用的な使い方を、シナリオ別にまとめます。
「こういうとき、どう使うか」が分かるレシピ集です。

---

## 16.2 数学の問題を解く

### 例 1: 二次方程式を解く

**問題**: x² - 5x + 6 = 0

```
[1] Calculator → Fn+8 (Tools)
[2] 1 (Equation Solver) → 3 (Quadratic)
[3] a: 1
[4] b: -5
[5] c: 6
→ x1 = 3, x2 = 2
```

### 例 2: 三角関数の値を求める

**問題**: sin(30°), cos(60°), tan(45°)

```
[1] 角度モードを DEG に設定
[2] Calculator で sin(30) → 0.5
[3] cos(60) → 0.5
[4] tan(45) → 1
```

### 例 3: 積分を計算する

**問題**: ∫₀¹ x² dx

```
[1] Calculator → Fn+8 → 2 (Num Analysis)
[2] 2 (Integral)
[3] f(x): x^2
[4] a: 0
[5] b: 1
→ 0.3333333（1/3）
```

### 例 4: 階乗を計算する

**問題**: 10!

```
Calculator で 10! Enter → 3,628,800
```

---

## 16.3 グラフを描く

### 例 1: 円を描く

```
[1] Graph → Edit functions: x^2+y^2-4
[2] Plot type: Implicit
[3] Range: -3, 3, -3, 3
[4] Square aspect (1:1)
[5] Plot current functions
→ 真円
```

### 例 2: 正弦波を描く

```
[1] Graph → Edit functions: sin(x)
[2] Plot type: y=f(x)
[3] Trig window
[4] Plot
```

### 例 3: 2 曲線の交点

```
[1] F1 = sin(x), F2 = 0.5
[2] Plot
[3] G-Solve → 4 (Intersection)
→ x = 0.5236, x = 2.6180
```

### 例 4: 極大値を求める

**問題**: y = -x² + 4x の最大値

```
[1] F1 = -x^2+4x
[2] Plot
[3] G-Solve → 2 (Maximum)
→ x=2, y=4
```

---

## 16.4 統計分析

### 例 1: テストの平均と標準偏差

**データ**: 65, 70, 75, 80, 85

```
[1] Statistics → 1 → 1 (List Editor)
[2] L1 → A → "65,70,75,80,85"
[3] Statistics → 1 → 3 (Descriptive)
[4] List: L1
→ mean = 75, sd = 7.906
```

### 例 2: 2 群の平均比較

**データ**: L1=[60,65,70], L2=[75,80,85]

```
[1] L1, L2 に入力
[2] Statistics → 3 (Tests) → 3 (2-sample t)
[3] Group A: 1, Group B: 2
[4] Side: Two-sided
→ p < 0.05 → 有意差あり
```

### 例 3: 散布図と回帰直線

```
[1] L1=[1,2,3,4,5], L2=[2,4,6,8,10]
[2] Statistics → 2 → 1 (Linear)
[3] X: L1, Y: L2
→ y = 2x, r² = 1.0
```

### 例 4: サイコロの公平性

**データ**: 観測 [18,22,17,20,21,22], 期待 [20,20,20,20,20,20]

```
[1] L1 = 観測, L2 = 期待
[2] Tests → 9 (Chi-Square GOF)
→ p = 0.954 → 偏っていない
```

---

## 16.5 科学計算

### 例 1: 1 光年を計算

```
[1] Constants → Physical → c
[2] Y (Ans = 299792458)
[3] Calculator
[4] Ans * 60 * 60 * 24 * 365.25
→ 9.46e15 m
```

### 例 2: 自由落下

**問題**: 5 秒間で何 m 落ちるか

```
[1] Constants → g_p (9.80665)
[2] Y → Ans
[3] 0.5 * Ans * 25
→ 122.58 m
```

### 例 3: 摂氏から華氏

```
[1] Unit Converter → Temperature
[2] Celsius → Fahrenheit
[3] 100
→ 212°F
```

---

## 16.6 プログラミング（μLisp）

### 例 1: フィボナッチ数列

```lisp
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))

(fib 10)   ; → 55
```

### 例 2: 素数判定

```lisp
(define (is-prime n)
  (if (< n 2) #f
    (if (= n 2) #t
      (if (= (mod n 2) 0) #f #t))))
```

### 例 3: リストの二乗

```lisp
(map (lambda (x) (* x x)) '(1 2 3 4 5))
; → (1 4 9 16 25)
```

### 例 4: 合計と平均

```lisp
(define data '(10 20 30 40 50))
(sum data)   ; → 150
(mean data)  ; → 30
(sd data)    ; → 15.811
```

---

## 16.7 スプレッドシート活用法

### 例: 家計簿

```
A 列: 日付
B 列: 収入
C 列: 支出
D 列: 残高（=B1-C1, =D1+B2-C2, ...）
```

### 例: 成績表

```
A 列: 名前
B〜F 列: 5 科目の点数
G 列: 合計（=B1+C1+D1+E1+F1）
```

---

## 16.8 複合シナリオ

### 例: 実験データの分析

```
[1] CSV ファイルを SD に保存
[2] Statistics → 11 (CSV Stream)
[3] Path: /data.csv
[4] 各列の統計を表示
[5] 回帰列を指定して相関分析
[6] 散布図で可視化
```

### 例: 統計レポート作成

```
[1] データ入力（L1〜L5）
[2] 記述統計（L1）
[3] 回帰分析（L1 vs L2）
[4] グラフで可視化（Histogram, Scatter）
[5] BMP 保存
```

---

## 16.9 この章のまとめ

- 数学: 方程式・微積分・三角関数
- グラフ: 円・正弦波・交点・極値
- 統計: 平均・検定・回帰
- 科学: 定数・単位換算
- プログラミング: μLisp
- スプレッドシート: 家計簿・成績表

### 確認問題

📝 **Q16-1**: x² - 5x + 6 = 0 を解く手順は？
<details><summary>答え</summary>
Fn+8 → 1 → 3 (Quadratic) で a=1, b=-5, c=6
</details>

📝 **Q16-2**: 円を真円で表示するには？
<details><summary>答え</summary>
Graph → Square aspect (1:1) → Implicit で x^2+y^2-4
</details>

📝 **Q16-3**: 5 科目の平均を μLisp で計算するには？
<details><summary>答え</summary>
`(mean '(科目1 科目2 科目3 科目4 科目5))`
</details>

---

[← 第15章 用語集](15_glossary.md) | [目次](index.md) | [第17章 付録 →](17_appendix.md)
'@

[System.IO.File]::WriteAllText("$base\16_examples.md", $ch16, $utf8NoBom)
Write-Host "Created: 16_examples.md" -ForegroundColor Green

# ========== 第17章 付録 ==========
$ch17 = @'
# 第17章 付録

[← 第16章 実用例集](16_examples.md) | [目次](index.md) | [README に戻る](../../README.md)

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

[← 第16章 実用例集](16_examples.md) | [目次](index.md) | [README に戻る](../../README.md)
'@

[System.IO.File]::WriteAllText("$base\17_appendix.md", $ch17, $utf8NoBom)
Write-Host "Created: 17_appendix.md" -ForegroundColor Green

Write-Host ""
Write-Host "=== 5 章生成完了 ===" -ForegroundColor Cyan
Get-ChildItem $base\*.md | Sort-Object Name | Select-Object Name, Length