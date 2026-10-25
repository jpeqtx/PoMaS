# 第16章 実用例集
[← 第15章 用語集](15_glossary.md) | [目次](#/) | [第17章 付録 →](17_appendix.md)
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
[← 第15章 用語集](15_glossary.md) | [目次](#/) | [第17章 付録 →](17_appendix.md)
