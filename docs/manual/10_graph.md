# 第10章 グラフ

[← 第9章 統計 応用](09_statistics_adv.md) | [目次](index.md) | [第11章 科学資料 →](11_sciref.md)

---

## 10.1 グラフモードとは

**Graph モード** は、関数のグラフを描画するモードです。

### グラフでできること

| 機能 | 説明 |
|---|---|
| 関数グラフの描画 | y = f(x) の曲線 |
| パラメトリック | x(t), y(t) の媒介変数表示 |
| 極座標 | r = f(t) の極座標 |
| 陰関数 | f(x, y) = 0 の曲線 |
| 不等式 | f(x, y) > 0 の領域 |
| 数列 | a_n のプロット |
| 3D プロット | z = f(x, y) のワイヤーフレーム |
| トレース | カーソルで座標を読む |
| G-Solve | 根・極値・交点・積分 |
| 接線・法線 | 指定点の接線/法線 |

### 画面サイズとアスペクト比

PoMaS の画面は **240 × 135 ピクセル**で、
グラフ描画領域は **240 × 105 ピクセル**（ヘッダ・フッタを除く）です。

```
画面全体: 240 × 135
グラフ領域: 240 × 105
→ 横長の領域
```

⚠️ **アスペクト比の注意**
X 軸と Y 軸のスケールが異なるため、
数学的な円が楕円に見える場合があります。
真円表示には **Square aspect (1:1)** を使います（10.4 節）。

---

## 10.2 Graph モードの起動

メインメニュー → `5` または Enter で **Graph** を選択。

### メニュー

```
┌─────────────────────────────────┐
│▌Graph                           │
├─────────────────────────────────┤
│1 Plot current functions         │
│2 Edit functions                 │
│3 Plot type                      │
│4 Range / Window                 │
│5 Standard window                │
│6 Trig window                    │
│7 Square aspect (1:1)            │
│8 G-Solve                        │
│9 Tangent / Normal at x          │
│10 Save plot as BMP              │
│11 Back                          │
└─────────────────────────────────┘
```

---

## 10.3 関数の編集（Edit functions）

### 基本操作

`2 (Edit functions)` で 4 本まで関数を登録できます。

```
┌─────────────────────────────────┐
│▌Edit Fns                        │
├─────────────────────────────────┤
│1 *: sin(x)                      │  ← 有効・登録済み
│2  : <empty>                     │  ← 無効・未登録
│3  : <empty>                     │
│4  : <empty>                     │
└─────────────────────────────────┘
```

- `*` = 有効（プロットされる）
- ` ` = 無効（プロットされない）
- `<empty>` = 未登録

### 例: 関数 y = sin(x) を登録

```
[1] Graph → 2 (Edit functions)
[2] 1 番目を選択
[3] "sin(x)" と入力
[4] Enter
→ *1: sin(x)
```

### 例: 複数関数の登録

```
[1] sin(x)
[2] cos(x)
[3] tan(x)
→ 3 本の三角関数が同時にプロット可能
```

### 関数の無効化

空文字列（何も入力しない）で Enter すると、その枠は無効になります。

---

## 10.4 Plot type（描画タイプ）

`3 (Plot type)` で描画の種類を選択します。

```
┌─────────────────────────────────┐
│▌Plot Type                       │
├─────────────────────────────────┤
│1 y = f(x)                       │
│2 Parametric x(t),y(t)           │
│3 Polar r=f(t)                   │
│4 Implicit f(x,y)=0              │
│5 Inequality f(x,y)>0            │
│6 Sequence a_n                   │
│7 3D z=f(x,y)                    │
│8 Back                           │
└─────────────────────────────────┘
```

### タイプ 1: y = f(x)

**最も基本的なグラフ。**

**例: y = sin(x)**

```
[1] Plot type → 1 (y=f(x))
[2] Edit functions → sin(x)
[3] Plot current functions
→ 正弦波が表示される
```

**入力書式**:
- `sin(x)`, `cos(x)`, `tan(x)`
- `x^2`, `x^3 - 2x + 1`
- `exp(x)`, `log(x)`
- `1/x`, `sqrt(x)`

### タイプ 2: Parametric（パラメトリック）

**x(t), y(t) を媒介変数 t で表す。**

**例: 円（半径 1）**

```
[1] Plot type → 2 (Parametric)
[2] Edit functions → cos(t);sin(t)
[3] Plot current functions
→ 円が表示される
```

⚠️ **セミコロン区切り**
x(t) と y(t) は `;`（セミコロン）で区切ります。

**例: リサージュ図形**

```
sin(3t);sin(2t)
```

**例: 螺旋**

```
t*cos(t);t*sin(t)
```

### タイプ 3: Polar（極座標）

**r = f(t) の形で曲線を描く。**

**例: カージオイド（ハート形）**

```
[1] Plot type → 3 (Polar)
[2] Edit functions → 1+cos(t)
[3] Plot current functions
→ ハート形の曲線
```

**例: バラ曲線**

```
cos(3t)
```

**例: 螺旋**

```
t
```

### タイプ 4: Implicit（陰関数）

**f(x, y) = 0 の形の曲線。**

**例: 円 x² + y² = 4**

```
[1] Plot type → 4 (Implicit)
[2] Edit functions → x^2+y^2-4
[3] Plot current functions
→ 円が表示される
```

**例: 楕円 x²/4 + y² = 1**

```
x^2/4+y^2-1
```

**例: 双曲線 x² - y² = 1**

```
x^2-y^2-1
```

💡 **陰関数のコツ**
`x^2 + y^2 - 4 = 0` をそのまま入力します。
（`=0` は省略可）

### タイプ 5: Inequality（不等式）

**f(x, y) > 0 の領域を塗りつぶす。**

**例: x + y > 0 の領域**

```
[1] Plot type → 5 (Inequality)
[2] Edit functions → x+y
[3] Plot current functions
→ 直線 y = -x の上の領域が塗られる
```

**例: 単位円の内側**

```
4-x^2-y^2
```

（`4 - x² - y² > 0` は x² + y² < 4、つまり半径 2 の円の内側）

### タイプ 6: Sequence（数列）

**a_n を n についてプロット。**

**例: a_n = 1/n**

```
[1] Plot type → 6 (Sequence)
[2] Edit functions → 1/n
[3] Range → n start: 1, n end: 30
[4] Plot current functions
→ 点のプロット（1, 1/2, 1/3, ...）
```

**例: a_n = n²**

```
n^2
```

### タイプ 7: 3D

**z = f(x, y) のワイヤーフレーム。**

**例: z = sin(x)·cos(y)**

```
[1] Plot type → 7 (3D)
[2] Edit functions → sin(x)*cos(y)
[3] Range → zmin: -2, zmax: 2
[4] Plot current functions
→ 3D 曲面のワイヤーフレーム
```

⚠️ **3D は軽量表示**
PoMaS の 3D はワイヤーフレーム（線のみ）で、
塗りつぶしはしません。

---

## 10.5 Range / Window（範囲設定）

`4 (Range / Window)` で表示範囲を設定します。

### 設定項目

| 項目 | 意味 |
|---|---|
| xmin, xmax | X 軸の最小値・最大値 |
| ymin, ymax | Y 軸の最小値・最大値 |
| tmin, tmax, tstep | 媒介変数（Parametric/Polar 用） |
| n start, n end | 数列の開始・終了（Sequence 用） |
| zmin, zmax | Z 軸の範囲（3D 用） |

### 例: -5 〜 5 の範囲

```
[1] Range → 4
[2] xmin: -5
[3] xmax: 5
[4] ymin: -5
[5] ymax: 5
→ X, Y とも -5〜5 の範囲
```

### 標準ウィンドウ（Standard window）

`5 (Standard window)` で以下の範囲にリセット：

- xmin: -10, xmax: 10
- ymin: -10, ymax: 10

### 三角関数ウィンドウ（Trig window）

`6 (Trig window)` で三角関数に最適な範囲にリセット：

- xmin: -2π ≈ -6.28
- xmax: +2π ≈ +6.28
- ymin: -1.5, ymax: 1.5

sin(x), cos(x) の 1 周期以上が見えます。

### Square aspect (1:1)

`7 (Square aspect)` で X/Y の等比スケールに調整：

**動作**:
- 現在の X 範囲を保つ
- Y 範囲を (X 範囲) × (105/240) に自動調整

**例**: X 範囲 -3〜3（幅 6）のとき
- Y 範囲 = 6 × 105/240 = 2.625 → -1.3125 〜 +1.3125

**使い方**:
```
[1] Range で -3, 3, -3, 3 を設定
[2] Square aspect を実行
[3] Implicit で x^2+y^2-4 をプロット
→ 真円が表示される
```

---

## 10.6 プロット実行（Plot）

`1 (Plot current functions)` で描画します。

### プロット中の画面

```
┌─────────────────────────────────┐
│▌y=f(x)                          │
├─────────────────────────────────┤
│                                 │
│         /\                      │
│        /  \                     │
│       /    \                    │
│      /      \                   │
│─────/────────\──────── x 軸     │
│    /          \                 │
│                                 │
├─────────────────────────────────┤
│Z:in X:out R:reset T:trace       │
└─────────────────────────────────┘
```

### プロット中のキー操作

| キー | 機能 |
|---|---|
| `Z` | ズームイン |
| `X` | ズームアウト |
| `R` | 標準ウィンドウにリセット |
| `T` | トレース ON/OFF |
| `B` | ボックスズーム（数値入力） |
| `+` または `n` | 次の関数に切り替え（トレース時） |
| `S` | BMP 保存 |
| `Fn + Esc` | グラフメニューに戻る |

---

## 10.7 ズームとトレース

### ズームイン（Z）

画面中央を中心に、範囲を 1/2 倍に縮小します。

```
[1] プロット中に Z を押す
→ xmin, xmax が現在の 1/4 幅、ymin, ymax が 1/4 高さに
```

**連続ズーム**: Z を押すたびに縮小。

### ズームアウト（X）

画面中央を中心に、範囲を 2 倍に拡大します。

### ボックスズーム（B）

数値入力で任意の範囲にジャンプ：

```
[1] B を押す
[2] zoom xmin: -1
[3] zoom xmax: 1
[4] zoom ymin: -1
[5] zoom ymax: 1
→ -1〜1 の範囲に拡大
```

### トレース（T）

カーソルで曲線上の座標を読み取ります。

```
[1] プロット中に T を押す
→ トレースモード ON
[2] ← / → で X 座標を移動
→ 画面右上に「x=..., y=..., F1」が表示
```

**例**:
```
x=1.234  y=0.944  F1
```

（F1 = 1 番目の関数）

### 複数関数の切り替え

トレース中に `+` を押すと、次の関数に切り替わります。

```
1 本目 → 2 本目 → 3 本目 → 4 本目 → 1 本目
```

---

## 10.8 G-Solve（自動解析）

`8 (G-Solve)` で、グラフの重要な点を自動計算します。

### メニュー

```
┌─────────────────────────────────┐
│▌G-Solve                         │
├─────────────────────────────────┤
│1 Root (sign change scan)        │
│2 Maximum (gradient ascent)      │
│3 Minimum (gradient descent)     │
│4 Intersection f1 = f2           │
│5 y at x                         │
│6 x at y (root of f-y)           │
│7 Definite integral              │
│8 Back                           │
└─────────────────────────────────┘
```

### 1. Root（根）

**f(x) = 0 となる x を探します。**

**例**: sin(x) = 0 の根を x ∈ [-2π, 2π] で探す

```
[1] G-Solve → 1 (Root)
→ x = -6.2832
  x = -3.1416
  x = 0
  x = 3.1416
  x = 6.2832
```

（-2π, -π, 0, π, 2π）

### 2. Maximum（極大値）

**局所的な最大値を探します。**

**例**: y = sin(x) の極大値

```
[1] G-Solve → 2 (Maximum)
→ x=1.5708 y=1.0000
  x=-4.7124 y=1.0000
```

（π/2, -3π/2 で最大値 1）

### 3. Minimum（極小値）

**局所的な最小値を探します。**

### 4. Intersection（交点）

**2 つの関数の交点を探します。**

**例**: y = sin(x) と y = 0.5 の交点

```
[1] 2 本の関数を登録:
    F1 = sin(x)
    F2 = 0.5
[2] G-Solve → 4 (Intersection)
→ x=0.5236 y=0.5000
  x=2.6180 y=0.5000
```

（π/6 ≈ 0.5236, 5π/6 ≈ 2.618）

### 5. y at x

**x を指定して y を計算。**

```
[1] G-Solve → 5 (y at x)
[2] x: 1.5708
→ y = 1.0000
```

（sin(π/2) = 1）

### 6. x at y

**y を指定して x を探す（f(x) = y の解）。**

### 7. Definite integral（定積分）

**指定区間の定積分を数値計算。**

**例**: ∫₀^π sin(x) dx

```
[1] G-Solve → 7 (Integral)
[2] from a: 0
[3] to b: 3.1416
→ Integral = 2.0000
```

**確認**: ∫₀^π sin(x) dx = [-cos(x)]₀^π = 1 - (-1) = 2

---

## 10.9 接線・法線

`9 (Tangent / Normal at x)` で、指定点の接線と法線の方程式を計算します。

### 例: y = x² の x=2 での接線

**数学的解**:
- f(2) = 4（接点 (2, 4)）
- f'(x) = 2x → f'(2) = 4（傾き）
- 接線: y = 4(x - 2) + 4 = 4x - 4
- 法線: y = -1/4 · (x - 2) + 4

**PoMaS での操作**:

```
[1] 関数 x^2 を登録
[2] Graph → 9 (Tangent/Normal)
[3] at x: 2
→ At (x, y) = (2, 4)
  slope = 4
  Tan: y = 4(X - 2) + 4
  Norm: y = -0.25(X - 2) + 4
```

---

## 10.10 BMP 保存

`10 (Save plot as BMP)` でプロット画像を SD カードに保存。

### 保存手順

```
[1] プロット中に S キー、またはメニューから 10
→ /graph_XXXXX.bmp として保存
```

（XXXXX は連番）

### プロット中の Save キー（S）

プロット中に直接 `S` を押しても保存できます。
メニューに戻る必要はありません。

### 保存先

SD カードのルートディレクトリ。ファイル名は自動生成。

---

## 10.11 実用例

### 例 1: 三角関数の重ね合わせ

**目標**: y = sin(x) と y = cos(x) を同時に表示

```
[1] Edit functions:
    F1 = sin(x)
    F2 = cos(x)
[2] Plot type: y=f(x)
[3] Range: -2π 〜 2π
[4] Plot current functions
→ 2 本の曲線が同時に表示
```

### 例 2: 2 次関数の頂点を求める

**目標**: y = x² - 4x + 3 の頂点

```
[1] F1 = x^2-4x+3
[2] Plot
[3] G-Solve → 3 (Minimum)
→ x=2.0 y=-1.0
```

頂点は (2, -1)。平方完成で (x-2)² - 1 と確認できます。

### 例 3: 陰関数でハートを描く

**目標**: (x² + y² - 1)³ - x²y³ = 0（ハート曲線）

```
[1] Plot type: Implicit
[2] F1 = (x^2+y^2-1)^3-x^2*y^3
[3] Range: -2, 2, -2, 2
[4] Square aspect
[5] Plot
→ ハート形が表示される
```

### 例 4: カージオイド（極座標）

**目標**: r = 1 + cos(t)

```
[1] Plot type: Polar
[2] F1 = 1+cos(t)
[3] Range: tmin 0, tmax 2π
[4] Plot
→ ハート形の曲線
```

### 例 5: リサージュ図形

**目標**: x = sin(3t), y = sin(2t)

```
[1] Plot type: Parametric
[2] F1 = sin(3t);sin(2t)
[3] Range: tmin 0, tmax 2π
[4] Plot
→ 複雑な曲線パターン
```

---

## 10.12 トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| 円が楕円になる | アスペクト比の違い | Square aspect (1:1) |
| 何も表示されない | 関数が空 / Range 外 | 関数登録と Range を確認 |
| 極座標で閉じない | tmax < 2π | tmax: 6.2832 |
| 陰関数がギザギザ | RES=40 の粗さ | 仕様（次期改善予定） |
| 3D が真っ黒 | zmin/zmax 外 | Range で z 範囲を調整 |
| 交点が見つからない | 2 関数未登録 | F1, F2 を両方登録 |

---

## 10.13 この章のまとめ

- Graph モードは 7 種類のプロットタイプ
- y=f(x), Parametric, Polar, Implicit, Inequality, Sequence, 3D
- Range / Window で表示範囲を設定
- Square aspect (1:1) で真円表示
- Z / X / R / B でズーム操作
- T でトレース、+ で複数関数切り替え
- G-Solve で根・極値・交点・定積分
- Tangent / Normal で接線・法線
- S キーで BMP 保存

### 確認問題

📝 **Q10-1**: y = sin(x) のグラフを表示する手順は？
<details><summary>答え</summary>
Plot type: y=f(x) → Edit functions: sin(x) → Plot current functions
</details>

📝 **Q10-2**: 円を描くには 2 通りの方法があります。それぞれ何を使いますか？
<details><summary>答え</summary>
1. Parametric で cos(t);sin(t)
2. Implicit で x^2+y^2-4
</details>

📝 **Q10-3**: 真円を表示するには？
<details><summary>答え</summary>
Square aspect (1:1) を実行。Y 範囲が X 範囲 × 105/240 に調整される。
</details>

📝 **Q10-4**: トレースモードで複数の関数を切り替えるには？
<details><summary>答え</summary>
T でトレース ON → + キーで次の関数へ。
</details>

📝 **Q10-5**: G-Solve の Root とは？
<details><summary>答え</summary>
f(x) = 0 となる x（根）を探す機能。
</details>

📝 **Q10-6**: 定積分を計算する G-Solve の項目は？
<details><summary>答え</summary>
7 (Definite integral)。区間 a, b を入力。
</details>

📝 **Q10-7**: プロット画像を保存するキーは？
<details><summary>答え</summary>
S キー（プロット中）、または メニューから 10 (Save plot as BMP)。
</details>

---

[← 第9章 統計 応用](09_statistics_adv.md) | [目次](index.md) | [第11章 科学資料 →](11_sciref.md)