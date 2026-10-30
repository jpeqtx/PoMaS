# 第7章 CAS（数式処理）
[← 第6章 電卓 応用](06_calculator_adv.md) | [目次](#/) | [第8章 統計 基本 →](08_statistics.md)
---
## この章で学ぶこと

- CAS（Computer Algebra System）の考え方
- 多項式の入力書式
- 代数コマンド（Factor/cFactor/Expand/completeSquare など）
- 三角関数の式変形（tExpand/tCollect/toLn/toExp など）
- 微積分コマンド（Derivative/Taylor/Integral/Tangent など）
- 方程式コマンド（cSolve/cZeros）
- 微分方程式コマンド（deSolve/deSolve2/RK4/ImpDiff）
- 34 コマンドの完全リファレンス

---

## 7.1 CAS とは
**CAS（Computer Algebra System、コンピュータ代数システム）** は、
数値を計算するのではなく、**数式そのものを操作する** システムです。
### 普通の電卓 vs CAS
**普通の電卓**:
```
x = 3 のとき、x^2 + 2x + 1 = ?
→ 3^2 + 2*3 + 1 = 9 + 6 + 1 = 16
```
**CAS**:
```
x^2 + 2x + 1 = ?
→ (x + 1)^2   ← 式のまま変形
```
**CAS の利点**:
- 数式をそのまま操作できる
- 変数 x を含んだまま計算できる
- 微分・積分を記号的に実行できる
### PoMaS の CAS でできること
PoMaS の CAS は、**多項式** を中心に扱います。
多項式とは、`x^3 - 2x^2 + 5x - 3` のような、
x のべき乗の和で表される式です。
| 機能 | 例 |
|---|---|
| 多項式の評価 | x=2 のとき x²+3x+2 = 12 |
| 微分 | (x³)' = 3x² |
| 積分 | ∫x² dx = x³/3 + C |
| 加減乗算 | (x+1)(x-1) = x²-1 |
| 根 | x²-5x+6 = 0 → x=2, 3 |
| テイラー展開 | sin(x) ≈ x - x³/6 + ... |
| 極限 | lim(sin(x)/x, x→0) = 1 |
| 記号置換 | x²+3x+2 の x→y+1 |
---
## 7.2 CAS モードの画面
メインメニュー → `2` または Enter で **CAS** を選択。
```
┌─────────────────────────────────┐
│▌CAS                             │
├─────────────────────────────────┤
│1 Polynomial: Eval at x          │
│2 Polynomial: Differentiate      │
│3 Polynomial: Integrate          │
│4 Polynomial: Add                │
│5 Polynomial: Multiply           │
│6 Polynomial: Roots              │
│7 Numerical Taylor (any f)       │
│8 Numerical Limit (any f)        │
│9 Substitute symbol              │
│10 Back                          │
└─────────────────────────────────┘
```
---
## 7.3 多項式の入力書式
| 書き方 | 意味 |
|---|---|
| `x^3 - 2x + 1` | x³ - 2x + 1 |
| `3x^2 + 2x - 5` | 3x² + 2x - 5 |
| `x` | x（係数 1） |
| `-x^2` | -x²（係数 -1） |
| `5` | 定数 5 |
### 正しい例
```
x^3 - 2x + 1
1 + 2x - x^2
3x^2 + 2x - 5
x + 1
```
### 間違った例
```
x*x*x - 2x + 1     ← * は使わない
x^3 - 2*x + 1      ← 係数と x の間は * 不要
3 x^2 + 2x - 5     ← 係数と x の間に空白不要
```
**係数と x の関係**
- `2x` = 係数 2 の x
- `x` = 係数 1 の x（1 は省略）
- `-x` = 係数 -1 の x
### 次数の上限
PoMaS が扱える多項式は、**最大 16 次** まで。
---
## 7.4 多項式の評価（Eval at x）
### 例: x³ - 2x + 1 を x=2 で評価
```
[1] CAS → 1 (Eval at x)
[2] p(x): x^3 - 2x + 1
[3] x = 2
[4] Enter
→ p(2) = 5
```
**確認**: 2³ - 2×2 + 1 = 8 - 4 + 1 = 5
### 例: x² + 3x + 2 を x=-1 で評価
```
[1] p(x): x^2 + 3x + 2
[2] x = -1
→ p(-1) = 0
```
**確認**: (-1)² + 3(-1) + 2 = 1 - 3 + 2 = 0
**因数の確認**
x=-1 で 0 になるということは、
多項式が (x+1) を因数に持つことを意味します。
### 結果の利用
結果は `Ans` に保存されます。
```
[1] p(2) = 5 → Ans = 5
[2] Fn+Esc で Calculator に戻る
[3] Ans * 10 = 50
```
---
## 7.5 多項式の微分（Differentiate）
### 例: x³ - 2x + 1 の微分
```
[1] CAS → 2 (Differentiate)
[2] p(x): x^3 - 2x + 1
→ p'(x) = 3x^2 - 2
```
**確認**:
- (x³)' = 3x²
- (-2x)' = -2
- (1)' = 0
- よって p'(x) = 3x² - 2
### 例: x⁴ + 2x² の微分
```
[1] p(x): x^4 + 2x^2
→ p'(x) = 4x^3 + 4x
```
### 微分の公式（参考）
| 関数 | 導関数 |
|---|---|
| xⁿ | nx^(n-1) |
| 定数 k | 0 |
| x | 1 |
| kx | k |
### 高次微分
1 階微分のみ対応。2 階微分は、結果を再度微分します。
```
[1] p(x): x^3
→ p'(x) = 3x^2
[2] もう一度実行 → (3x^2)' = 6x
[3] もう一度実行 → (6x)' = 6
```
---
## 7.6 多項式の積分（Integrate）
### 例: 3x² - 2x + 1 の積分
```
[1] CAS → 3 (Integrate)
[2] p(x): 3x^2 - 2x + 1
→ ∫p(x)dx = x^3 - x^2 + x + C
```
**確認**:
- ∫3x² dx = x³
- ∫(-2x) dx = -x²
- ∫1 dx = x
- 積分定数 C を付加
**積分定数 C**
不定積分には必ず積分定数 C が付きます。
### 例: x⁴ の積分
```
[1] p(x): x^4
→ ∫p(x)dx = x^5/5 + C
```
### 積分の公式（参考）
| 関数 | 不定積分 |
|---|---|
| xⁿ (n≠-1) | x^(n+1)/(n+1) + C |
| 定数 k | kx + C |
| 1 | x + C |
### 定積分の計算
PoMaS の CAS は不定積分のみ対応。
定積分は、結果を Calculator で計算するか、
電卓の拡張ツール（Fn+8 → 2 → 2 Integral）で数値的に求めます。
---
## 7.7 多項式の加減乗算
### 加算（Add）
**例: (x + 1) + (x - 1)**
```
[1] CAS → 4 (Add)
[2] p(x): x + 1
[3] q(x): x - 1
→ p + q = 2x
```
### 減算
減算は Add で 2 つ目の符号を逆にして実現。
**例: (x + 1) - (x - 1) = 2**
```
[1] p(x): x + 1
[2] q(x): -x + 1    ← 符号を逆に
→ p + q = 2
```
### 乗算（Multiply）
**例: (x + 1) × (x - 1)**
```
[1] CAS → 5 (Multiply)
[2] p(x): x + 1
[3] q(x): x - 1
→ p * q = x^2 - 1
```
**確認**: (x+1)(x-1) = x² - x + x - 1 = x² - 1
### 例: (x + 2)² の展開
```
[1] p(x): x + 2
[2] q(x): x + 2
→ p * q = x^2 + 4x + 4
```
### 例: (2x - 3)(x + 4)
```
[1] p(x): 2x - 3
[2] q(x): x + 4
→ p * q = 2x^2 + 5x - 12
```
**確認**: 2x·x + 2x·4 - 3·x - 3·4 = 2x² + 5x - 12
---
## 7.8 多項式の根（Roots）
多項式 = 0 となる x の値を求めます（複素数対応）。
### 例: x² - 5x + 6 = 0 の根
```
[1] CAS → 6 (Roots)
[2] p(x): x^2 - 5x + 6
→ r1 = 3, r2 = 2
```
**確認**: (x-3)(x-2) = 0 → x = 3, 2
### 例: x² + 1 = 0 の根（複素数）
```
[1] p(x): x^2 + 1
→ r1 = i, r2 = -i
```
**確認**: x² = -1 → x = ±i
### 例: x³ - 6x² + 11x - 6 = 0 の根
```
[1] p(x): x^3 - 6x^2 + 11x - 6
→ r1 = 3, r2 = 2, r3 = 1
```
**確認**: (x-1)(x-2)(x-3) = 0 → x = 1, 2, 3
### 根の精度
PoMaS は **Durand-Kerner 法** という数値解法で根を求めます。
5 次以上でも近似解が得られます。
**精度の限界**
- 重解（同じ根が複数）の検出は苦手
- 非常に近い根は、わずかに異なる値になる場合がある
---
## 7.9 テイラー展開（Numerical Taylor）
### テイラー展開とは
複雑な関数を、多項式で近似する技法です。
```
sin(x) ≈ x - x³/6 + x⁵/120 - ...
```
### 例: sin(x) のテイラー展開（x0=0, 次数 5）
```
[1] CAS → 7 (Taylor)
[2] f(x): sin(x)
[3] center x0: 0
[4] order (1-7): 5
→ x - 0.16667(x-0)^3 + 0.00833(x-0)^5
```
**確認**:
- sin(x) の x=0 での展開:
  - 1 次: x
  - 3 次: -x³/3! = -x³/6 ≈ -0.16667x³
  - 5 次: +x⁵/5! = x⁵/120 ≈ 0.00833x⁵
### 例: e^x のテイラー展開（x0=0, 次数 5）
```
[1] f(x): exp(x)
[2] center x0: 0
[3] order: 5
→ 1 + x + 0.5x^2 + 0.16667x^3 + 0.04167x^4 + 0.00833x^5
```
**確認**:
- e^0 = 1
- xⁿ/n!
- x⁰: 1, x¹: 1, x²: 0.5, x³: 0.16667, x⁴: 0.04167, x⁵: 0.00833
### 中心 x0 を変える
x0 ≠ 0 のときは、x-a の多項式になります。
**例: sin(x) の x0=π/2 での展開**
```
[1] f(x): sin(x)
[2] center x0: 1.5708 (π/2)
[3] order: 3
→ 1 - 0.5(x-1.5708)^2 + ...
```
sin は x=π/2 で最大値 1 なので、定数項が 1 になります。
### 精度の確認
テイラー展開の近似精度は、x0 からの距離で決まります。
x0 に近いほど高精度、離れるほど誤差が大きくなります。
---
## 7.10 極限（Numerical Limit）
### 例: lim(x→0) sin(x)/x
**理論値**: 1
```
[1] CAS → 8 (Limit)
[2] f(x): sin(x)/x
[3] x ->: 0
→ Limit = 1.000000
```
### 例: lim(x→0) (1-cos(x))/x²
**理論値**: 1/2
```
[1] f(x): (1-cos(x))/x^2
[2] x ->: 0
→ Limit = 0.500000
```
### 例: lim(x→∞) 1/x
**理論値**: 0
```
[1] f(x): 1/x
[2] x ->: 1000000   ← 大きな値
→ Limit = 0.000001
```
**無限大の極限**
真の ∞ は入力できないので、代わりに大きな値で近似します。
### 極限が存在しない場合
左右の極限が一致しない場合、エラーメッセージが表示されます。
**例: lim(x→0) 1/x**
```
→ L=-1000000 R=1000000 (diff)
```
---
## 7.11 記号置換（Substitute）
式の中の x を、別の式に置き換えます。
### 例: x² + 3x + 2 の x → y+1
```
[1] CAS → 9 (Substitute)
[2] f(x): x^2 + 3x + 2
[3] Replace x = y+1
→ (y+1)^2 + 3(y+1) + 2
```
結果は展開されず、置換後の式がそのまま表示されます。
### 使用例
**例: f(x) = x² で x → a+b**
```
[1] f(x): x^2
[2] x = a+b
→ (a+b)^2
```
**期待**: (a+b)² = a² + 2ab + b²
PoMaS は展開しないので、結果を手で展開するか、
Calculator で計算します。
---
## 7.12 CAS コマンド早見表（34 コマンド）

PoMaS の CAS は電卓入力欄に `CommandName(arg)` 形式で記述して Enter で実行します。

| 分類 | コマンド |
|---|---|
| 代数 | `Factor` `cFactor` `Expand` `completeSquare` `getNum` `getDenom` `propFrac` `comDenom` `Left` `Right` |
| 三角・式変換 | `tExpand` `tCollect` `toLn` `toExp` `toSin` `toCos` `toLogbase` |
| 微積分 | `Derivative` `Taylor` `Series` `LeadingTerm` `Tangent` `Normal` `ArcLength` `DiffQ` `fMin` `fMax` |
| 方程式 | `cSolve` `cZeros` |
| 微分方程式 | `deSolve` `deSolve2` `RK4` `ImpDiff` `implicitDiff` |

各コマンドの詳細を以下に示します。

---

## 7.13 代数コマンド

### Factor — 多項式の有理数範囲での因数分解

**構文**: `Factor(p)`

**用途**: 多項式 p を有理数係数の範囲で 1 次式・2 次式の積に分解。

**例**:
```
Factor(x^2-5x+6)     → (x-2)(x-3)
Factor(x^2-1)        → (x-1)(x+1)
Factor(x^3-6x^2+11x-6) → (x-1)(x-2)(x-3)
Factor(2x^2+5x-3)    → (2x-1)(x+3)
```

**備考**: 有理数根のみ検出。無理数根を含む因数分解はできない。

**関連**: cFactor

### cFactor — 複素数範囲での因数分解

**構文**: `cFactor(p)`

**用途**: 複素数根を持つ因数も含めて分解。

**例**:
```
cFactor(x^2+1)       → (x-i)(x+i)
cFactor(x^2+4)       → (x-2i)(x+2i)
cFactor(x^2-2)       → (x-1.41421)(x+1.41421)
```

**備考**: 無理数根も数値近似で出力。

**関連**: Factor

### Expand — 多項式の展開

**構文**: `Expand(p)`

**用途**: (x+1)(x-1) → x²-1 のように展開。

**例**:
```
Expand((x+1)(x-1))       → x^2-1
Expand((x+2)^2)          → x^2+4x+4
Expand((x+1)(x+2)(x+3))  → x^3+6x^2+11x+6
```

**関連**: Factor

### completeSquare — 平方完成

**構文**: `completeSquare(p)`

**用途**: 2 次式 ax²+bx+c を a(x-p)²+q の形に変形。

**例**:
```
completeSquare(x^2-4x+3)   → (x-2)^2-1
completeSquare(x^2+6x+10)  → (x+3)^2+1
completeSquare(2x^2+8x+5)  → 2(x+2)^2-3
```

**用途**: 2 次関数の頂点計算、積分の準備

### getNum — 分数の分子

**構文**: `getNum(f)`

**例**:
```
getNum(3/4)         → 3
getNum((x+1)/(x-1)) → x+1
getNum(x/2)         → x
```

**関連**: getDenom

### getDenom — 分数の分母

**構文**: `getDenom(f)`

**例**:
```
getDenom(3/4)         → 4
getDenom((x+1)/(x-1)) → x-1
getDenom(x/2)         → 2
```

**関連**: getNum

### propFrac — 仮分数を帯分数に

**構文**: `propFrac(f)`

**用途**: (2x+3)/(x-1) を商 + 真分数に変形。

**例**:
```
propFrac((2x+3)/(x-1))   → 2 + 5/(x-1)
propFrac((x^2+1)/x)      → x + 1/x
propFrac((x^3+2)/(x+1))  → x^2-x+1 + 1/(x+1)
```

**用途**: 有理関数の積分準備、極限計算

### comDenom — 通分

**構文**: `comDenom(expr)`

**用途**: 複数の分数を共通分母でまとめる。

**例**:
```
comDenom(1/x + 1/(x+1))       → (2x+1)/(x^2+x)
comDenom(1/2 + 1/3)           → 5/6
comDenom((x+1)/(x-1) - 1)     → 2/(x-1)
```

**関連**: propFrac

### Left — 等式の左辺

**構文**: `Left(eq)`

**用途**: `a = b` の a を抽出。

**例**:
```
Left(x+1=5)          → x+1
Left(2x+y=10)        → 2x+y
Left(x^2=4)          → x^2
```

**関連**: Right

### Right — 等式の右辺

**構文**: `Right(eq)`

**例**:
```
Right(x+1=5)         → 5
Right(2x+y=10)       → 10
Right(x^2=4)         → 4
```

**関連**: Left

---

## 7.14 三角関数の式変形

### tExpand — 加法定理で展開

**構文**: `tExpand(sin(A+B))` または `tExpand(cos(A-B))`

**例**:
```
tExpand(sin(A+B))    → sin(A)cos(B)+cos(A)sin(B)
tExpand(sin(A-B))    → sin(A)cos(B)-cos(A)sin(B)
tExpand(cos(A+B))    → cos(A)cos(B)-sin(A)sin(B)
tExpand(cos(A-B))    → cos(A)cos(B)+sin(A)sin(B)
```

**関連**: tCollect

### tCollect — 三角関数の縮約

**構文**: `tCollect(sin²x)` または `tCollect(sinx*cosx)`

**用途**: 積を和に、2 乗を 1 次に変換。

**例**:
```
tCollect(sin(x)^2)        → (1-cos(2x))/2
tCollect(cos(x)^2)        → (1+cos(2x))/2
tCollect(sin(x)*cos(x))   → sin(2x)/2
```

**関連**: tExpand

### toLn — 対数への変換

**構文**: `toLn(a^x)`

**用途**: 一般の底の指数関数を e の累乗に変換。

**例**:
```
toLn(2^x)            → e^(x*ln(2))
toLn(10^x)           → e^(x*ln(10))
```

**関連**: toExp

### toExp — e^x への変換

**構文**: `toExp(exp(x))`

**用途**: 指数関数を e の累乗の形に統一。

**例**:
```
toExp(exp(x))         → e^(x)
toExp(exp(2x))        → e^(2x)
```

**関連**: toLn

### toSin — cos を sin に変換

**構文**: `toSin(cos(x))`

**例**:
```
toSin(cos(x))         → sin(pi/2-x)
toSin(cos(2x))        → sin(pi/2-2x)
```

**関連**: toCos

### toCos — sin を cos に変換

**構文**: `toCos(sin(x))`

**例**:
```
toCos(sin(x))         → cos(pi/2-x)
toCos(sin(3x))        → cos(pi/2-3x)
```

**関連**: toSin

### toLogbase — 底を変更

**構文**: `toLogbase(a, b)` または `toLogbase(log_a(b))`

**用途**: log_a(b) を任意の底で表現。

**例**:
```
toLogbase(2, 8)       → ln(8)/ln(2)
toLogbase(3, 81)      → ln(81)/ln(3)
```

---

## 7.15 微積分コマンド

### Derivative — 記号微分

**構文**: `Derivative(f(x), x)`

**用途**: 任意の関数を x について記号微分。

**例**:
```
Derivative(x^3, x)          → 3x^2
Derivative(sin(x), x)       → cos(x)
Derivative(exp(x), x)       → exp(x)
Derivative(sin(x)*cos(x), x) → cos(x)^2-sin(x)^2
Derivative(x^2*exp(x), x)   → 2x*exp(x)+x^2*exp(x)
```

**対応関数**: 多項式、sin、cos、tan、exp、ln、sqrt、四則演算、合成関数

**備考**: 多項式は係数比較による完全記号微分、その他は微分ノードツリーで処理

### Taylor — テイラー展開

**構文**: `Taylor(f(x), x0, n)`

**用途**: f(x) を x = x0 の周りで n 次までテイラー展開。

**例**:
```
Taylor(sin(x), 0, 5)     → x-x^3/6+x^5/120
Taylor(exp(x), 0, 4)     → 1+x+x^2/2+x^3/6+x^4/24
Taylor(cos(x), 0, 4)     → 1-x^2/2+x^4/24
Taylor(ln(1+x), 0, 3)    → x-x^2/2+x^3/3
```

**次数範囲**: 1〜7（n=7 で 7 次近似）

**中心 x0**: 任意の値（x0 = 0 でマクローリン展開）

**関連**: Series

### Series — 級数展開（Taylor のエイリアス）

**構文**: `Series(f(x), x0, n)`

**用途**: `Taylor` と同じ。級数展開。

**例**:
```
Series(sin(x), 0, 5)     → x-x^3/6+x^5/120
Series(1/(1-x), 0, 5)    → 1+x+x^2+x^3+x^4+x^5
```

### LeadingTerm — 主要項の抽出

**構文**: `LeadingTerm(p)`

**用途**: 多項式 p の最高次項を返す。

**例**:
```
LeadingTerm(x^3+2x+1)      → x^3
LeadingTerm(3x^5-x^2+1)    → 3x^5
LeadingTerm(5)             → 5
```

**用途**: 極限計算、漸近解析

### Tangent — 接線の方程式

**構文**: `Tangent(f(x), a)`

**用途**: 曲線 y = f(x) の x = a における接線。

**例**:
```
Tangent(x^2, 2)          → y = 4(x-2)+4 = 4x-4
Tangent(sin(x), 0)       → y = 1(x-0)+0 = x
Tangent(x^3, 1)          → y = 3(x-1)+1 = 3x-2
```

**戻り値形式**: `y = m(x-a)+f(a)` または展開形 `y = mx+b`

**関連**: Normal

### Normal — 法線の方程式

**構文**: `Normal(f(x), a)`

**用途**: 曲線 y = f(x) の x = a における法線（接線に垂直）。

**例**:
```
Normal(x^2, 2)           → y = -1/4(x-2)+4
Normal(x^3, 1)           → y = -1/3(x-1)+1
```

**傾き**: -1/f'(a)（f'(a) ≠ 0）、f'(a)=0 のとき x=a の垂線

**関連**: Tangent

### ArcLength — 曲線の長さ

**構文**: `ArcLength(f(x), a, b)`

**用途**: 曲線 y = f(x) の x = a から b までの弧長。

**計算**: L = ∫ₐᵇ √(1 + f'(x)²) dx（数値積分）

**例**:
```
ArcLength(x^2, 0, 1)     → 1.47894
ArcLength(sin(x), 0, pi) → 3.82020
```

**用途**: 曲線の実長、測地線、軌道計算

### DiffQ — 中心差分商

**構文**: `DiffQ(f(x), a, h)`

**用途**: 数値微分の中心差分商 [f(a+h) - f(a-h)] / (2h)。

**例**:
```
DiffQ(x^2, 3, 0.001)     → 6.00000
DiffQ(sin(x), 0, 0.001)  → 0.99999
```

**用途**: 微分の数値確認、h の影響観察

### fMin — 数値最小値探索

**構文**: `fMin(f(x), a, b)`

**用途**: 区間 [a,b] 内の f(x) の最小値を数値探索。

**例**:
```
fMin(x^2, -5, 5)          → min at x=0, f=0
fMin((x-3)^2+1, 0, 10)    → min at x=3, f=1
```

**アルゴリズム**: グリッドサーチ（1000 点）+ 黄金分割（80 反復）

**関連**: fMax

### fMax — 数値最大値探索

**構文**: `fMax(f(x), a, b)`

**用途**: 区間 [a,b] 内の f(x) の最大値を数値探索。

**例**:
```
fMax(-(x-3)^2+9, 0, 10)   → max at x=3, f=9
fMax(sin(x), 0, pi)       → max at x=1.5708, f=1
```

**関連**: fMin

---

## 7.16 方程式コマンド

### cSolve — 方程式を解く（複素解含む）

**構文**: `cSolve(eq)` または `cSolve(expr)`

**用途**: 方程式 eq を 0 について解く（cSolve(expr) は expr = 0 と同じ）。

**例**:
```
cSolve(x^2-5x+6=0)       → x=3 ; x=2
cSolve(x^2+1=0)          → x=i ; x=-i
cSolve(x^3-6x^2+11x-6=0) → x=1 ; x=2 ; x=3
cSolve(2x^2+x-1=0)       → x=0.5 ; x=-1
```

**範囲**: 最大 16 次多項式

**戻り値**: 全根（実根 + 複素根）を `x=... ; x=...` 形式で表示

**関連**: cZeros, 方程式ソルバー (Fn+8 → 1)

### cZeros — 方程式の零点（cSolve のエイリアス）

**構文**: `cZeros(expr)` = `cSolve(expr = 0)`

**例**:
```
cZeros(x^2-5x+6)    → x=3 ; x=2
cZeros(x^2+4)       → x=2i ; x=-2i
```

**関連**: cSolve

---

## 7.17 微分方程式コマンド

### deSolve — 1 階 ODE 解析解

**構文**: `deSolve(dy/dx = f)` — 各種の形に対応

**対応形**:

| 形 | 例 | 戻り値 |
|---|---|---|
| y' = f(x) | `deSolve(y'=x^2)` | `y = x^3/3 + C` |
| y' = ky | `deSolve(y'=2y)` | `y = C*e^(2x)` |
| y' = ay+b | `deSolve(y'=2y+3)` | `y = -1.5 + C*e^(2x)` |
| y' = P(x)y+Q(x) | 一般形 | 一般解（文字列） |
| y' = 0 | `deSolve(y'=0)` | `y = C` |

**例**:
```
deSolve(y'=cos(x))     → y = sin(x) + C
deSolve(y'=1/x)        → y = ln|x| + C
deSolve(y'=e^x)        → y = e^x + C
deSolve(y'=3y+1)       → y = -1/3 + C*e^(3x)
```

**備考**: 変数分離形・同次形・完全微分形など一般形は未対応（部分完了）

**関連**: deSolve2, RK4

### deSolve2 — 2 階線形 ODE（定数係数）

**構文**: `deSolve2(a, b)` で y'' + a y' + b y = 0 を解く

**例**:
```
deSolve2(1, 1)          → 減衰振動 y = e^(-x/2)(C1 cos(x√3/2)+C2 sin(x√3/2))
deSolve2(-3, 2)         → y''-3y'+2y=0 → y = C1 e^(2x) + C2 e^(x)
deSolve2(0, 1)          → y''+y=0 → y = C1 cos(x) + C2 sin(x)
```

**判別式 D = a² - 4b**:
- D > 0: 実根 2 個 → `y = C1 e^(r1 x) + C2 e^(r2 x)`
- D = 0: 重根 → `y = (C1 + C2 x) e^(r x)`
- D < 0: 複素根 → `y = e^(αx)(C1 cos(βx) + C2 sin(βx))`

### RK4 — Runge-Kutta 4 次

**構文**: `RK4(f, x0, y0, xe, h)`

**用途**: y' = f(x,y)、初期値 (x0, y0) から xe まで、ステップ h で RK4 数値解。

**例**:
```
RK4(y, 0, 1, 2, 0.1)   → y(2) ≈ 7.38906（e^2 ≈ 7.389）
RK4(-x*y, 0, 1, 1, 0.1) → y(1) ≈ 0.60653
```

**用途**: 数値的にしか解けない ODE、シミュレーション

### ImpDiff / implicitDiff — 陰微分

**構文**: `ImpDiff(F(x,y))` または `implicitDiff(F(x,y))`

**用途**: F(x,y) = 0 から dy/dx = -F_x / F_y を記号計算。

**例**:
```
ImpDiff(x^2+y^2-4)      → dy/dx = -(2x)/(2y) = -x/y
ImpDiff(x*y-1)          → dy/dx = -(y)/(x) = -y/x
ImpDiff(sin(x)+cos(y))  → dy/dx = -(cos(x))/(-sin(y)) = cos(x)/sin(y)
```

**用途**: 陰関数曲線の接線・法線、位相平面解析

---

## 7.18 CAS コマンドの組み合わせ例

### 例 1: 曲線の性質を一括解析

```
f(x) = x^3 - 3x^2 + 2
[1] Derivative(x^3-3x^2+2, x)  → 3x^2-6x
[2] cZeros(3x^2-6x)            → x=0 ; x=2
[3] fMin(x^3-3x^2+2, -10, 10)  → min at x=2, f=-2
[4] fMax(x^3-3x^2+2, -10, 10)  → max at x=0, f=2
[5] Tangent(x^3-3x^2+2, 1)     → y = -3(x-1)+0 = -3x+3
```

### 例 2: 有理関数の積分準備

```
[1] propFrac((x^2+1)/(x-1))    → x+1 + 2/(x-1)
[2] getNum((x^2+1)/(x-1))      → x^2+1
[3] getDenom((x^2+1)/(x-1))    → x-1
[4] comDenom(1/(x-1) - 1/(x+1)) → 2/(x^2-1)
```

### 例 3: 極限・漸近の解析

```
[1] Taylor(sin(x), 0, 5)       → x-x^3/6+x^5/120
[2] LeadingTerm(3x^5-x^2+1)    → 3x^5
[3] ArcLength(x^2, 0, 1)       → 1.47894
```

### 例 4: ODE の数値解と解析解の比較

```
[1] deSolve(y'=y)              → y = C*e^x
[2] RK4(y, 0, 1, 1, 0.01)      → y(1) ≈ 2.71828
[3] exp(1)                     → 2.71828（一致確認）
```

---

## 7.19 CAS の制限事項

### 対応範囲

| 対象 | 対応 |
|---|---|
| 多項式の次数 | 最大 16 次 |
| 有理関数 | ○（部分分数分解、通分） |
| 三角関数 | 加法定理・縮約のみ |
| 指数・対数 | 底変換・e^x 統一のみ |
| 微分 | sin/cos/tan/exp/ln/sqrt の組み合わせ |
| 積分 | 多項式のみ（任意関数は数値積分 Fn+8 → 2） |
| ODE | 1 階（一部形のみ）、2 階定数係数 |

### 未対応の高度機能

- 記号積分（部分積分・置換積分の自動化）
- 連立方程式の記号解
- 行列式の記号計算
- ベクトル解析（grad/div/curl の記号化）
- 特殊関数（Bessel, Legendre, Hermite など）
- 級数の収束判定
- 極限の記号計算（`Limit` コマンドは未実装、数値のみ）

これらは外部 Maxima 連携（Tools → 3 → 6）で補完できます。

---

## 7.20 CAS の使い方のコツ

### 1. 変数は x に統一

CAS コマンドは原則 x を変数として扱います。`Derivative(f, x)` の第 2 引数で他の変数を指定可能（1 文字のみ）。

### 2. 多項式は展開形で入力

`(x+1)(x+1)` ではなく `x^2+2x+1` の形で入力してください（parsePoly が展開形のみ対応）。

### 3. 冪乗は `^` 記号

`x**2` や `x*x` ではなく `x^2` を使う。

### 4. 分数はそのまま

`1/2` は 0.5 に変換されません（有理数として保持）。`3/4 + 1/4 = 1` と厳密に計算されます。

### 5. 結果は Ans へ

CAS コマンドの結果は `Ans` に転送できます。続けて数値計算に使えます。

```
[1] cSolve(x^2-5x+6=0)     → x=3 ; x=2
[2] Y キー → Ans = 3
[3] Ans * 2                → 6
```

---

## 7.21 CAS コマンド実行の内部処理

### 呼出順序

Calculator の入力欄に文字列を入れると、以下の順で処理されます:

```
[1] tryAssignOrFunc  — a = b, f(x) = ... の形式を検出
[2] tryCasCommand    — CommandName(arg) の形式を検出
[3] 通常評価 (ExprEval)
```

CAS コマンドは `CommandName(` の後に `(...)` を続ける形式のみ認識されます。

```
Factor(x^2-1)        正しい
Factor (x^2-1)       空白あり → 通常評価として失敗
factor(x^2-1)        cFactor と区別不能 → tryCasCommand で小文字許容
```

### 記号微分エンジン

Derivative / ImpDiff / Tangent / Normal は **微分ノードツリー** で処理:

- 各演算（+、-、*、/、^）と関数（sin、cos、exp、ln、sqrt）を DNode としてツリー化
- 再帰的に微分規則を適用
- 簡略化パスで 0*x → 0、x^1 → x などを削減

最大 200 ノードまで、深度 120 まで対応。

---

## 7.22 CAS と他モードの連携

### Calculator との連携

```
CAS で結果 → Y キー → Ans → Calculator で継続計算
```

### Graph との連携

```
[1] CAS で Derivative(f, x) を計算
[2] 結果を手動で Graph の関数入力にコピー
[3] 元の関数と導関数を同時プロット
```

### Statistics との連携

```
[1] CAS で回帰モデルの式変形
[2] 結果を Statistics の回帰分析に活用
```

### Maxima との連携

CAS で未対応の高度な計算は Maxima サーバーへ送信:

```
Tools → 3 → 6 (Send to Maxima)
URL prefix: http://your-proxy/maxima?expr=
Maxima expr: integrate(sin(x)*cos(x), x)
→ 結果を HTTP で受信
```

---

## 7.23 この章のまとめ
- CAS は数式をそのまま操作する
- 多項式の評価・微分・積分・加減乗算
- 根の計算（複素数対応）
- テイラー展開（任意の関数、次数 1〜7）
- 極限（数値的アプローチ）
- 記号置換（展開は手動）
- 入力は `x^2 + 3x + 2` の形式（`*` 記号は不要）
### 確認問題
**Q7-1**: x³ - 2x + 1 を x=2 で評価すると？
<details><summary>答え</summary>
8 - 4 + 1 = 5
</details>
**Q7-2**: x³ - 2x + 1 の導関数は？
<details><summary>答え</summary>
3x² - 2
</details>
**Q7-3**: ∫3x² dx は？
<details><summary>答え</summary>
x³ + C
</details>
**Q7-4**: x² - 5x + 6 = 0 の根は？
<details><summary>答え</summary>
x = 2, 3（(x-2)(x-3) = 0）
</details>
**Q7-5**: (x+1)(x-1) を展開すると？
<details><summary>答え</summary>
x² - 1
</details>
**Q7-6**: sin(x) の x0=0 での 3 次テイラー近似は？
<details><summary>答え</summary>
x - x³/6 ≈ x - 0.16667x³
</details>
---
[← 第6章 電卓 応用](06_calculator_adv.md) | [目次](#/) | [第8章 統計 基本 →](08_statistics.md)
