# 第20章 uLisp 関数リファレンス


---
## この章で学ぶこと

- uLisp の概要と Lisp の特徴
- REPL の起動と操作方法
- データ型（数値・シンボル・文字列・真偽値・リスト・ラムダ）
- 算術関数（+ - * / mod）
- 比較関数（= < > <= >=）
- リスト操作（car/cdr/cons/list/append/reverse/nth）
- 数学関数（sin/cos/exp/log/sqrt/abs/floor/ceil/round など）
- 統計関数（sum/mean/sd）
- 高階関数（map/filter/reduce/apply）
- 特殊形式（quote/if/cond/and/or/define/lambda/let/while/begin）
- I/O 関数（print/save/load）
- GC（ガベージコレクション）
- 制限事項と エラー
- 実用例とスクリプト

---

## 20.1 uLisp とは

**μLisp（マイクロリスプ）** は、PoMaS に組み込まれた **Lisp 言語のサブセット実装**です。

### Lisp の特徴

- **1958 年**に John McCarthy が設計した、現存する最古の高水準プログラミング言語の一つ
- **S 式（S-expression）** による括弧中心の統一記法
- **関数型プログラミング**が自然に書ける
- **コードとデータの同質性**（コードをデータとして操作できる）
- **ガベージコレクション**による自動メモリ管理

### PoMaS の uLisp 実装の特徴

| 項目 | 内容 |
|---|---|
| セルプール | 2,500 セル（動的確保） |
| 環境（Env） | 128 個 |
| 評価深度 | 400 段（ESP32 v3.x）/ 100 段（v2.x） |
| GC | マーク & スイープ（手動 or 自動） |
| 再帰 | 深度制限内で可（`fact(80)` 程度） |
| I/O | `print` / SD への `save` / `load` |
| 標準関数 | 80+ の組み込み関数 |

### Lisp の構文

すべての式は **括弧で囲まれたリスト** です。最初の要素が関数、残りが引数です:

```
(関数名 引数1 引数2 ...)
```

例:
```
(+ 1 2 3)       ; → 6
(sin 0)         ; → 0
(list 1 2 3)    ; → (1 2 3)
```

---

## 20.2 REPL の起動と操作

### REPL とは

**REPL** は **Read-Eval-Print Loop** の略で、対話型の実行環境です:

1. **Read**: 入力した式を読み込む
2. **Eval**: 評価（計算）する
3. **Print**: 結果を表示する
4. **Loop**: 1 に戻る

### 起動

```
メインメニュー → 7 (Tools) → 2 (uLisp Interpreter) → 1 (REPL)
```

### 画面構成

```
┌─────────────────────────────────┐
│▌uLisp REPL                      │
├─────────────────────────────────┤
│(+ 1 2) -> 3                     │
│(* 5 6) -> 30                    │
│(list 1 2 3) -> (1 2 3)          │
│> _                              │
├─────────────────────────────────┤
│ENT:eval Fn+E:exit ESC:back      │
└─────────────────────────────────┘
```

- 上段: 直近 6 件の入力履歴
- `>`: 入力プロンプト
- `_`: カーソル

### 操作

| キー | 動作 |
|---|---|
| 文字キー | 式を入力 |
| `Enter` | 評価実行 |
| `Del` | 1 文字削除 |
| `ESC`（`` ` ``） | REPL を抜ける |

### uLisp メニュー

```
┌─────────────────────────────────┐
│▌uLisp                           │
├─────────────────────────────────┤
│1 REPL (interactive)             │
│2 Run script from SD             │
│3 Run GC now                     │
│4 Reset interpreter              │
│5 Examples                       │
│6 Back                           │
└─────────────────────────────────┘
```

| 項目 | 説明 |
|---|---|
| REPL | 対話型シェル |
| Run script | SD カードの `.lsp` ファイル実行 |
| Run GC | ガベージコレクション手動実行 |
| Reset | インタプリタ完全初期化 |
| Examples | サンプルコード |

---

## 20.3 データ型

### 数値（Number）

内部的には C の `double`（倍精度浮動小数点）。整数も実数も区別しません。

```
42          ; 整数
3.14        ; 小数
-7          ; 負の数
1e10        ; 指数表記
```

### シンボル（Symbol）

変数名・関数名として使われる識別子。評価されると、束縛された値に置き換わります。

```
x           ; シンボル x
foo         ; シンボル foo
+           ; シンボル +（組み込み関数）
```

### 文字列（String）

ダブルクォートで囲んだ文字列。`print` などで使用。

```
"Hello"
"PoMaS 0.92"
```

エスケープ: `\"`（ダブルクォート）、`\\`（バックスラッシュ）

### 真偽値（Boolean）

- `#t`: 真
- `nil`: 偽（空リストと同じ）

```
(= 1 1)     ; → #t
(= 1 2)     ; → nil
(< 3 5)     ; → #t
```

### nil（空リスト）

Lisp では `nil` が **偽** と **空リスト** の両方を表します。

```
nil         ; → nil
'()         ; → nil（空リスト）
(null? nil) ; → #t
```

### リスト（List）

S 式の基本構造。`car`（先頭）と `cdr`（残り）で構成。

```
'(1 2 3)            ; 3 要素のリスト
'(1 (2 3) 4)        ; ネストしたリスト
'(a b c)            ; シンボルのリスト
```

### ラムダ（Lambda）

無名関数。`lambda` で生成。

```
(lambda (x) (* x x))    ; x の 2 乗を返す関数
```

### 組み込み関数（Builtin）

C++ 側で実装された関数。`+`, `car`, `sin` など。

---

## 20.4 算術関数

### `+`（加算）

任意個の引数を加算。

```
(+ 1 2)         ; → 3
(+ 1 2 3 4 5)   ; → 15
(+)             ; → 0（引数なしは 0）
(+ 2.5 3.5)     ; → 6.0
```

### `-`（減算）

- 1 引数: 符号反転
- 2 引数以上: 左から順に減算

```
(- 10 3)        ; → 7
(- 10 3 2)      ; → 5
(- 5)           ; → -5
```

### `*`（乗算）

任意個の引数を乗算。

```
(* 5 6)         ; → 30
(* 2 3 4)       ; → 24
(*)             ; → 1（引数なしは 1）
```

### `/`（除算）

- 1 引数: 逆数
- 2 引数以上: 左から順に除算

```
(/ 20 4)        ; → 5
(/ 20 4 5)      ; → 1
(/ 5)           ; → 0.2
(/ 1 0)         ; → エラー: division by zero
```

### `mod`（剰余）

2 引数必須。第 1 引数を第 2 引数で割った余り。

```
(mod 10 3)      ; → 1
(mod 17 5)      ; → 2
(mod 10 0)      ; → エラー: mod by zero
```

### ゼロ除算のエラー

`/` と `mod` は 0 除算でエラーを返します。エラーメッセージは REPL 履歴に `ERR: division by zero` などと表示されます。

---

## 20.5 比較関数

### `=`（等号）

数値・シンボル・文字列の等価比較。

```
(= 5 5)         ; → #t
(= 5 6)         ; → nil
(= 'a 'a)       ; → #t
(= "abc" "abc") ; → #t
```

### `equal?`（同義）

`=` と同じ動作。

```
(equal? 5 5)    ; → #t
```

### `<`（小なり）

```
(< 3 5)         ; → #t
(< 5 3)         ; → nil
(< 5 5)         ; → nil
```

### `>`（大なり）

```
(> 5 3)         ; → #t
(> 3 5)         ; → nil
```

### `<=`（以下）

```
(<= 3 5)        ; → #t
(<= 5 5)        ; → #t
(<= 6 5)        ; → nil
```

### `>=`（以上）

```
(>= 5 3)        ; → #t
(>= 5 5)        ; → #t
(>= 3 5)        ; → nil
```

---

## 20.6 リスト操作

### `car`（先頭取得）

リストの第 1 要素を返す。

```
(car '(1 2 3))  ; → 1
(car '((a) b))  ; → (a)
(car nil)       ; → nil
```

### `cdr`（残り取得）

リストの第 2 要素以降を返す。

```
(cdr '(1 2 3))  ; → (2 3)
(cdr '(1))      ; → nil
(cdr nil)       ; → nil
```

### `cons`（結合）

先頭要素と残りのリストを結合。

```
(cons 1 '(2 3))     ; → (1 2 3)
(cons 1 nil)        ; → (1)
(cons 1 (cons 2 nil)) ; → (1 2)
```

### `list`（リスト生成）

任意個の引数をリスト化。

```
(list 1 2 3)        ; → (1 2 3)
(list)              ; → nil
(list 1 2 3 4 5)    ; → (1 2 3 4 5)
(list 'a 'b 'c)     ; → (a b c)
```

### `null?`（空リスト判定）

引数が `nil` かどうか。

```
(null? nil)     ; → #t
(null? '())     ; → #t
(null? '(1))    ; → nil
(null? 5)       ; → nil
```

### `atom?`（非リスト判定）

引数がリストでないかどうか。

```
(atom? 5)       ; → #t
(atom? 'a)      ; → #t
(atom? '(1 2))  ; → nil
(atom? nil)     ; → #t
```

### `length`（リスト長）

```
(length '(1 2 3))     ; → 3
(length nil)          ; → 0
(length '(1 (2 3)))   ; → 2（トップレベルのみ）
```

### `append`（連結）

複数のリストを連結。

```
(append '(1 2) '(3 4))          ; → (1 2 3 4)
(append '(1) '(2) '(3) '(4))    ; → (1 2 3 4)
(append nil '(1 2))             ; → (1 2)
(append '(1 2) nil)             ; → (1 2)
```

### `reverse`（逆順）

```
(reverse '(1 2 3))    ; → (3 2 1)
(reverse nil)         ; → nil
(reverse '(1))        ; → (1)
```

### `nth`（n 番目取得）

0 始まり。

```
(nth 0 '(1 2 3))      ; → 1
(nth 1 '(1 2 3))      ; → 2
(nth 2 '(1 2 3))      ; → 3
(nth 5 '(1 2 3))      ; → nil（範囲外）
```

---

## 20.7 数学関数

### 三角関数

| 関数 | 説明 | 例 |
|---|---|---|
| `sin` | 正弦（弧度法） | `(sin 0)` → 0 |
| `cos` | 余弦 | `(cos 0)` → 1 |
| `tan` | 正接 | `(tan 0)` → 0 |

注: uLisp 内部では **弧度法（ラジアン）** を使用します。度数法で計算したい場合は `(* pi 30/180)` のように変換します。

```
(sin (/ pi 2))    ; → 1（π/2）
(cos pi)          ; → -1
```

### 指数・対数

| 関数 | 説明 | 例 |
|---|---|---|
| `exp` | e の x 乗 | `(exp 1)` → 2.718... |
| `log` | 自然対数（底 e） | `(log (exp 1))` → 1 |
| `sqrt` | 平方根 | `(sqrt 16)` → 4 |

**負数の平方根**: `(sqrt -1)` は `ERR: sqrt(neg)` エラーを返します（安全のため）。

### 絶対値・丸め

| 関数 | 説明 | 例 |
|---|---|---|
| `abs` | 絶対値 | `(abs -5)` → 5 |
| `floor` | 切り捨て | `(floor 3.7)` → 3 |
| `ceil` | 切り上げ | `(ceil 3.2)` → 4 |
| `round` | 四捨五入 | `(round 3.5)` → 4 |

### 累乗

| 関数 | 説明 |
|---|---|
| `expt` | 累乗（`(expt base exp)`） |
| `pow` | 累乗（`expt` のエイリアス） |

```
(expt 2 10)     ; → 1024
(pow 2 0.5)     ; → 1.414...（√2）
```

### 最小・最大

| 関数 | 説明 | 例 |
|---|---|---|
| `min` | 最小値 | `(min 3 5 1)` → 1 |
| `max` | 最大値 | `(max 3 5 1)` → 5 |

任意個の引数を受け付けます。

### 乱数

```
(random)        ; → 0〜1 の一様乱数
```

---

## 20.8 統計関数

### `sum`（総和）

リストの要素の合計。

```
(sum '(1 2 3 4 5))    ; → 15
(sum nil)             ; → 0
```

### `mean`（平均）

```
(mean '(10 20 30))    ; → 20
(mean nil)            ; → 0
```

### `sd`（標準偏差、標本）

```
(sd '(10 20 30 40 50))    ; → 15.811...
```

n-1 で割る標本標準偏差です。n < 2 のときは 0 を返します。

---

## 20.9 高階関数

### `map`（写像）

各要素に関数を適用した新しいリストを返す。

```
(map (lambda (x) (* x x)) '(1 2 3 4 5))
; → (1 4 9 16 25)

(map (lambda (x) (+ x 10)) '(1 2 3))
; → (11 12 13)
```

### `filter`（選択）

述語関数が真を返す要素のみを集める。

```
(filter (lambda (x) (> x 2)) '(1 2 3 4 5))
; → (3 4 5)

(filter (lambda (x) (= (mod x 2) 0)) '(1 2 3 4 5 6))
; → (2 4 6)
```

### `reduce`（畳み込み）

`(reduce 関数 初期値 リスト)` の形式。

```
(reduce + 0 '(1 2 3 4 5))
; → 15

(reduce * 1 '(1 2 3 4 5))
; → 120（5!）

(reduce (lambda (a b) (if (> a b) a b)) 0 '(3 7 2 9 1))
; → 9（最大値）
```

### `apply`（適用）

関数と引数リストを分離して渡す。

```
(apply + '(1 2 3 4 5))
; → 15（sum と同じ）

(apply max '(3 7 2 9 1))
; → 9
```
---

## 20.10 特殊形式

特殊形式は評価ルールが通常の関数と異なります（引数が自動評価されない、または条件付きで評価される）。

### `quote`（クォート）

引数を評価せずにそのまま返す。省略記法: `'式`

```
(quote (1 2 3))     ; → (1 2 3)
'(1 2 3)            ; → (1 2 3)（省略形）
'x                  ; → x（シンボル x そのもの）
''x                 ; → (quote x)
```

### `if`（条件分岐）

`(if 条件 真式 偽式)` の形式。偽式は省略可能（省略時 nil）。

```
(if (= 1 1) "yes" "no")     ; → "yes"
(if (= 1 2) "yes" "no")     ; → "no"
(if (> 5 3) 100)            ; → 100
(if (< 5 3) 100)            ; → nil
```

### `cond`（多分岐）

`(cond (条件1 結果1) (条件2 結果2) ... (else デフォルト))` の形式。

```
(cond ((> 5 10) "big")
      ((< 5 0)  "neg")
      (else     "mid"))
; → "mid"
```

**else の扱い**: 最後の節に `else` シンボルを置くと、常に真として扱われます。

### `and`（論理積）

すべて真なら最後の値を返す。1 つでも偽なら nil。

```
(and #t #t #t)      ; → #t
(and #t nil #t)     ; → nil
(and 1 2 3)         ; → 3
(and)               ; → #t
```

### `or`（論理和）

いずれか真ならその値を返す。すべて偽なら nil。

```
(or nil #t nil)     ; → #t
(or nil nil nil)    ; → nil
(or 1 2 3)          ; → 1（最初の真値）
(or)                ; → nil
```

### `not`（否定）

```
(not #t)            ; → nil
(not nil)           ; → #t
(not 5)             ; → nil（0 以外は真）
```

### `define`（変数定義）

```
(define x 42)       ; x = 42
(define y 100)      ; y = 100
(+ x y)             ; → 142
```

### `define`（関数定義）

`(define (関数名 引数...) 本体)` の形式。

```
(define (square n) (* n n))
(square 5)          ; → 25

(define (add a b) (+ a b))
(add 3 4)           ; → 7
```

### `setq`（再代入）

既存の変数に新しい値を代入。

```
(define x 10)
(setq x 20)
x                   ; → 20
```

### `lambda`（無名関数）

```
(lambda (x) (* x x))    ; 関数オブジェクト
((lambda (x) (* x x)) 5) ; → 25（即時呼び出し）
```

### `let`（ローカル束縛）

`(let ((変数1 値1) (変数2 値2)...) 本体)` の形式。

```
(let ((a 10) (b 20))
  (+ a b))
; → 30

; 外側の x には影響しない
(define x 100)
(let ((x 5)) (* x 2))   ; → 10
x                       ; → 100
```

### `while`（ループ）

`(while 条件 本体...)` の形式。条件が真の間、本体を繰り返す。

```
(define i 0)
(define sum 0)
(while (< i 5)
  (setq sum (+ sum i))
  (setq i (+ i 1)))
sum                     ; → 10（0+1+2+3+4）
```

安全装置: ループが 100,000 回を超えると `ERR: loop limit` で停止。

### `begin` / `progn`（順次実行）

複数の式を順に評価し、最後の値を返す。`let`・`while`・`lambda` の本体は暗黙的に `begin` と同じ動作をします。

```
(begin
  (print "step 1")
  (print "step 2")
  (print "step 3")
  (+ 1 2 3))
; → 6（"step 1" "step 2" "step 3" が表示される）
```

`progn` は `begin` のエイリアス。

---

## 20.11 I/O 関数

### `print`（表示）

引数の値を **シリアルモニタ** に出力します（画面には出ません）。

```
(print "Hello, PoMaS!")
(print (+ 1 2 3))
```

PC で Arduino IDE のシリアルモニタを 115200 bps で開くと表示されます。

### `save`（SD 保存）

オブジェクトを S 式文字列として SD カードに保存。

```
(save "data.lsp" '(1 2 3 4 5))
(save "func.lsp" (lambda (x) (* x x)))
```

ファイルは `/data.lsp` のように SD カードのルートに作成されます。

### `load`（SD 読み込み）

SD カードのファイルを S 式として読み込み、パース結果を返す。

```
(load "data.lsp")       ; → (1 2 3 4 5)
```

**注意**: `load` は S 式を **パースして返すだけ** で、評価はしません。評価が必要な場合は `eval`（未実装）またはスクリプト実行メニューを使ってください。

実際には uLisp メニュー → `2 (Run script from SD)` でスクリプト全体を評価できます。

---

## 20.12 グローバル定数

### 定数

| シンボル | 値 |
|---|---|
| `pi` | 3.14159... |
| `e` | 2.71828... |

```
(* 2 pi)             ; → 6.283...
(exp 1)              ; → 2.718...（e と同じ）
```

### 変数

`define` で作った変数はグローバル環境に保存されます。`resetPool` で明示的に初期化しない限り永続します。

---

## 20.13 GC（ガベージコレクション）

### 自動 GC

PoMaS の uLisp は **マーク & スイープ方式** の GC を持ちますが、**自動的には走りません**。セルプールが枯渇すると `ERR: μLisp cell pool exhausted` になります。

### 手動 GC

以下の 3 つの方法で GC を実行できます:

1. **uLisp メニュー → 3 (Run GC now)**
2. **REPL 起動時**（環境数が 100 を超えている場合に自動実行）
3. **Reset interpreter**（完全初期化）

### GC の動作

```
[1] nilC, trueC, globalEnv をマーク
[2] 到達可能なセルを再帰的にマーク
[3] 到達不可能なセルを freeList に戻す
[4] 未使用の Env を削除
```

### GC の注意点

- **REPL 内での `(gc)` 呼び出しは無効化**されています（安全性のため）
- **評価中に GC が走ると、未マークの一時セルが回収されて危険**です
- メニューから GC を実行する場合は、評価が完了した状態で行ってください

---

## 20.14 制限事項

### 容量制限

| 項目 | 上限 |
|---|---|
| セルプール | 2,500 セル |
| 環境（Env） | 128 個 |
| 評価深度 | 400 段（v3.x）/ 100 段（v2.x） |
| パーサ深度 | 200 段 |
| シンボル長 | 11 文字以内推奨 |
| 文字列長 | 256 文字（パース時） |

### 深度制限（eval/apply）

`SET_LOOP_TASK_STACK_SIZE(32768)` が有効な環境（ESP32 Arduino v3.x）では **最大 400 段** の再帰が可能です。v2.x 環境では 100 段です。

**fact(80)** のような深い再帰は 400 段環境で動作します。

```
(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))
(fact 80)   ; → 7.15e+118（深度 400 環境）
(fact 200)  ; → ERR: eval depth（深度 400 超過）
```

### ループ制限

`while` のループ回数が **100,000 回** を超えると `ERR: loop limit` で停止します。無限ループ防止のためです。

```
(define i 0)
(while #t (setq i (+ i 1)))
; → ERR: loop limit（10 万回で停止）
```

### 未実装の機能

以下の機能は **意図的に未実装** です:

- マクロ（`defmacro`）
- 末尾呼び出し最適化（TCO）
- 継続（call/cc）
- 多値返却
- 動的スコープ
- 名前空間・モジュール
- ハッシュテーブル
- ベクタ（配列は list のみ）

これらが必要な場合は、外部の Maxima や本格的な Lisp 処理系を使用してください。

---

## 20.15 エラーメッセージ

| メッセージ | 意味 | 対処 |
|---|---|---|
| `unbound symbol` | 未定義の変数・関数 | `define` で定義 |
| `not callable` | 関数でないものの呼び出し | 引数の種類を確認 |
| `if: bad args` | `if` の引数不正 | `(if 条件 真 偽)` の形式 |
| `if: no then` | `if` の真式なし | 真式を追加 |
| `define: bad args` | `define` の形式不正 | `(define name value)` など |
| `define: no value` | `define` の値なし | 値を追加 |
| `define: bad target` | 対象がシンボルでない | シンボル名を指定 |
| `lambda: bad args` | `lambda` の形式不正 | `(lambda (args) body)` |
| `let: bad args` | `let` の形式不正 | `(let ((v val)) body)` |
| `while: bad args` | `while` の形式不正 | `(while cond body)` |
| `quote: no arg` | `quote` の引数なし | `(quote expr)` |
| `parse too deep` | パーサ深度超過 | ネストを浅くする |
| `unexpected EOF` | 閉じ括弧不足 | `)` を追加 |
| `μLisp cell pool exhausted; run GC from menu` | セル枯渇 | Run GC now |
| `env table full` | 環境変数表満杯 | Reset interpreter |
| `Env limit` | Env プール上限 | Reset interpreter |
| `loop limit` | while ループ上限 | 条件を見直す |
| `missing arg` | 組み込み関数の引数不足 | 引数を追加 |
| `too many args` | 組み込み関数の引数過多 | 引数を減らす |
| `division by zero` | ゼロ除算 | 除数確認 |
| `mod by zero` | mod のゼロ除算 | 除数確認 |
| `sqrt(neg)` | 負数の平方根 | 絶対値などで対応 |
| `sym malloc fail` | シンボル用メモリ失敗 | Reset interpreter |
| `str malloc fail` | 文字列用メモリ失敗 | Reset interpreter |
| `eval depth` | eval 深度超過 | 再帰を浅くする |
| `apply depth` | apply 深度超過 | 同上 |
| `user fn recursion` | ユーザー関数の再帰深度超過 | 再帰を見直す |

---

## 20.16 実用例集

### 例 1: 階乗

```
(define (fact n)
  (if (<= n 1)
      1
      (* n (fact (- n 1)))))

(fact 5)    ; → 120
(fact 10)   ; → 3628800
```

### 例 2: フィボナッチ数列

```
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))

(fib 10)    ; → 55
(fib 20)    ; → 6765
```

### 例 3: 素数判定（試し割り）

```
(define (is-prime n)
  (if (< n 2)
      nil
      (if (= n 2)
          #t
          (let ((i 2) (result #t))
            (while (<= (* i i) n)
              (if (= (mod n i) 0) (setq result nil))
              (setq i (+ i 1)))
            result))))

(is-prime 7)    ; → #t
(is-prime 10)   ; → nil
(is-prime 97)   ; → #t
```

### 例 4: リストの二乗

```
(map (lambda (x) (* x x)) '(1 2 3 4 5))
; → (1 4 9 16 25)
```

### 例 5: 偶数のみ抽出

```
(filter (lambda (x) (= (mod x 2) 0)) '(1 2 3 4 5 6 7 8))
; → (2 4 6 8)
```

### 例 6: 合計と平均と標準偏差

```
(define data '(10 20 30 40 50))
(sum data)      ; → 150
(mean data)     ; → 30
(sd data)       ; → 15.811
```

### 例 7: リストの最大値

```
(define (my-max lst)
  (reduce (lambda (a b) (if (> a b) a b)) (car lst) lst))

(my-max '(3 7 2 9 1))
; → 9
```

### 例 8: 三角関数表

```
(define deg2rad (lambda (d) (* d (/ pi 180))))
(define angles '(0 30 45 60 90))
(map (lambda (d) (sin (deg2rad d))) angles)
; → (0 0.5 0.707 0.866 1)
```

### 例 9: クロージャ（カウンタ）

```
(define (make-counter)
  (let ((count 0))
    (lambda ()
      (setq count (+ count 1))
      count)))

(define c1 (make-counter))
(c1)    ; → 1
(c1)    ; → 2
(c1)    ; → 3
```

### 例 10: スクリプトファイル

**script.lsp の内容**:

```
(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))
(print (fact 10))
(print (fact 20))
(print "Done!")
```

**実行**:

```
Tools → 2 (uLisp) → 2 (Run script from SD)
Path: /script.lsp
→ シリアルモニタに結果出力
```

---

## 20.17 組み込み関数早見表

### 特殊形式（10 個）

`quote` `if` `cond` `and` `or` `not` `define` `setq` `lambda` `let` `while` `begin` `progn`

### 算術（5 個）

`+` `-` `*` `/` `mod`

### 比較（6 個）

`=` `equal?` `<` `>` `<=` `>=`

### リスト（12 個）

`car` `cdr` `cons` `list` `null?` `atom?` `number?` `symbol?` `length` `append` `reverse` `nth`

### 数学（16 個）

`sin` `cos` `tan` `exp` `log` `sqrt` `abs` `floor` `ceil` `round` `expt` `pow` `min` `max` `random` `mean`

### 統計（3 個）

`sum` `mean` `sd`

### 高階（4 個）

`map` `filter` `reduce` `apply`

### I/O（3 個）

`print` `save` `load`

### 定数（2 個）

`pi` `e`

**総計: 約 80 関数**

---

## 20.18 この章のまとめ

- uLisp は PoMaS に組み込まれた Lisp サブセット
- S 式 `(関数 引数...)` の統一記法
- 数値・シンボル・文字列・真偽値・リスト・ラムダ
- 80+ の組み込み関数（算術・比較・リスト・数学・統計・高階・I/O）
- 特殊形式（quote/if/cond/define/lambda/let/while）
- マーク & スイープ GC（手動）
- 深度 400 段（v3.x）/ 100 段（v2.x）
- セル 2500 / Env 128 / ループ 10 万回が上限
- マクロ・TCO・継続・多値などは意図的に未実装

### 確認問題

Q20-1: `(+ 1 2 3)` の結果は？ → 6

Q20-2: `(map (lambda (x) (* x x)) '(1 2 3))` の結果は？ → (1 4 9)

Q20-3: `(if (> 5 3) "yes" "no")` の結果は？ → "yes"

Q20-4: `(fact 5)` を定義して実行する手順は？ → `(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))` その後 `(fact 5)` → 120

Q20-5: `ERR: cell pool exhausted` が出たら？ → Tools → uLisp → 3 (Run GC now)

Q20-6: uLisp のマクロ（defmacro）は使える？ → 未実装（意図的除外）

---

## 20.19 全組み込み関数の完全リファレンス

### 20.19.1 算術関数（5）

| 関数 | 引数 | 例 | 結果 |
|---|---|---|---|
| `+` | 可変長 | `(+ 1 2 3)` | 6 |
| `-` | 可変長 | `(- 10 3 2)` | 5 |
| `*` | 可変長 | `(* 2 3 4)` | 24 |
| `/` | 可変長 | `(/ 20 4)` | 5 |
| `mod` | 2 | `(mod 10 3)` | 1 |

### 20.19.2 比較関数（6）

| 関数 | 例 | 結果 |
|---|---|---|
| `=` | `(= 5 5)` | `#t` |
| `equal?` | `(equal? "a" "a")` | `#t` |
| `<` | `(< 3 5)` | `#t` |
| `>` | `(> 5 3)` | `#t` |
| `<=` | `(<= 5 5)` | `#t` |
| `>=` | `(>= 5 3)` | `#t` |

### 20.19.3 リスト操作（12）

| 関数 | 例 | 結果 |
|---|---|---|
| `car` | `(car '(1 2 3))` | 1 |
| `cdr` | `(cdr '(1 2 3))` | (2 3) |
| `cons` | `(cons 1 '(2 3))` | (1 2 3) |
| `list` | `(list 1 2 3)` | (1 2 3) |
| `null?` | `(null? nil)` | `#t` |
| `atom?` | `(atom? 5)` | `#t` |
| `number?` | `(number? 5)` | `#t` |
| `symbol?` | `(symbol? 'a)` | `#t` |
| `length` | `(length '(1 2 3))` | 3 |
| `append` | `(append '(1) '(2))` | (1 2) |
| `reverse` | `(reverse '(1 2 3))` | (3 2 1) |
| `nth` | `(nth 1 '(a b c))` | b |

### 20.19.4 数学関数（16）

| 関数 | 例 | 結果 |
|---|---|---|
| `sin` | `(sin 0)` | 0 |
| `cos` | `(cos 0)` | 1 |
| `tan` | `(tan 0)` | 0 |
| `exp` | `(exp 1)` | 2.718 |
| `log` | `(log 2.718)` | 1 |
| `sqrt` | `(sqrt 16)` | 4 |
| `abs` | `(abs -5)` | 5 |
| `floor` | `(floor 3.7)` | 3 |
| `ceil` | `(ceil 3.2)` | 4 |
| `round` | `(round 3.5)` | 4 |
| `expt` | `(expt 2 10)` | 1024 |
| `pow` | `(pow 2 0.5)` | 1.414 |
| `min` | `(min 3 5 1)` | 1 |
| `max` | `(max 3 5 1)` | 5 |
| `random` | `(random)` | 0〜1 |
| `mean` | `(mean '(10 20))` | 15 |

### 20.19.5 統計関数（3）

| 関数 | 例 | 結果 |
|---|---|---|
| `sum` | `(sum '(1 2 3 4 5))` | 15 |
| `mean` | `(mean '(10 20 30))` | 20 |
| `sd` | `(sd '(10 20 30 40 50))` | 15.811 |

### 20.19.6 高階関数（4）

| 関数 | 例 | 結果 |
|---|---|---|
| `map` | `(map (lambda (x) (* x x)) '(1 2 3))` | (1 4 9) |
| `filter` | `(filter (lambda (x) (> x 2)) '(1 2 3 4))` | (3 4) |
| `reduce` | `(reduce + 0 '(1 2 3 4))` | 10 |
| `apply` | `(apply + '(1 2 3))` | 6 |

### 20.19.7 I/O 関数（3）

| 関数 | 例 | 動作 |
|---|---|---|
| `print` | `(print "Hi")` | シリアル出力 |
| `save` | `(save "f.lsp" '(1 2 3))` | SD 保存 |
| `load` | `(load "f.lsp")` | SD 読込 |

### 20.19.8 特殊形式（12）

| 形式 | 例 |
|---|---|
| `quote` | `(quote x)` / `'x` |
| `if` | `(if c t f)` |
| `cond` | `(cond (c1 r1) (else r))` |
| `and` | `(and #t #t)` |
| `or` | `(or nil #t)` |
| `not` | `(not #t)` |
| `define` | `(define x 42)` |
| `setq` | `(setq x 20)` |
| `lambda` | `(lambda (x) (* x x))` |
| `let` | `(let ((a 1)) a)` |
| `while` | `(while c body)` |
| `begin` | `(begin e1 e2)` |

---

## 20.20 Lisp デザインパターン

### 20.20.1 再帰パターン

**階乗**:
```
(define (fact n)
  (if (<= n 1) 1 (* n (fact (- n 1)))))
```

**フィボナッチ（メモ化なし）**:
```
(define (fib n)
  (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))
```

**フィボナッチ（メモ化）**:
```
(define memo (list))
(define (fib-memo n)
  (if (< n 2) n
    (let ((cached (assoc n memo)))
      (if cached (cdr cached)
        (let ((v (+ (fib-memo (- n 1)) (fib-memo (- n 2)))))
          (setq memo (cons (cons n v) memo))
          v)))))
```

### 20.20.2 高階関数パターン

**畳み込み**:
```
(define (my-sum lst) (reduce + 0 lst))
(define (my-prod lst) (reduce * 1 lst))
(define (my-max lst) (reduce (lambda (a b) (if (> a b) a b)) 0 lst))
```

**フィルタ + マップ**:
```
(define (squares-of-evens lst)
  (map (lambda (x) (* x x))
       (filter (lambda (x) (= (mod x 2) 0)) lst)))
```

**合成**:
```
(define (compose f g) (lambda (x) (f (g x))))
(define inc-sq (compose (lambda (x) (+ x 1)) (lambda (x) (* x x))))
(inc-sq 3)
```

### 20.20.3 クロージャパターン

**カウンタ**:
```
(define (make-counter)
  (let ((n 0))
    (lambda () (setq n (+ n 1)) n)))
(define c1 (make-counter))
(c1)
(c1)
```

**アキュムレータ**:
```
(define (make-accumulator total)
  (lambda (x) (setq total (+ total x)) total))
(define acc (make-accumulator 100))
(acc 10)
(acc 20)
```

---

## 20.21 実用的スクリプト集

### 20.21.1 数列生成

**range**:
```
(define (range a b)
  (if (> a b) nil (cons a (range (+ a 1) b))))
(range 1 10)
```

**等差数列**:
```
(define (arith a d n)
  (if (= n 0) nil (cons a (arith (+ a d) d (- n 1)))))
(arith 0 5 5)
```

**等比数列**:
```
(define (geom a r n)
  (if (= n 0) nil (cons a (geom (* a r) r (- n 1)))))
(geom 1 2 6)
```

### 20.21.2 データ変換

**リストの二乗**:
```
(map (lambda (x) (* x x)) '(1 2 3 4))
```

**リストの合計と平均**:
```
(define (stats lst) (list (sum lst) (mean lst) (sd lst)))
(stats '(10 20 30 40 50))
```

### 20.21.3 テキスト処理

**文字列の長さ**:
```
(length "Hello")
```

**文字列連結**:
```
(append "Hello" " " "World")
```

---

## 20.22 パフォーマンスのヒント

### 20.22.1 メモリ節約

- 中間リストを避ける（`reduce` を使う）
- `setq` より `let` でスコープを絞る
- セルプール枯渇前に Run GC

### 20.22.2 実行速度

- 再帰より反復（`while` を使う）
- `map` / `filter` で C++ 実装の速度を活用
- 計算結果のキャッシュ

### 20.22.3 デバッグ

- `print` で中間値出力（シリアルモニタ）
- 小さな式で分割テスト
- エラーメッセージを確認（第 20.15 参照）

---

## 20.23 この章の追加まとめ

- 全 80 関数の完全リファレンス（8 分類）
- Lisp デザインパターン（再帰/高階/クロージャ）
- 実用スクリプト集（数列/データ変換/テキスト）
- パフォーマンスのヒント（メモリ/速度/デバッグ）

### 確認問題（追加）

Q20-7: `(map (lambda (x) (* x 2)) '(1 2 3))` の結果は？ → (2 4 6)

Q20-8: `(reduce + 0 '(1 2 3 4 5))` の結果は？ → 15

Q20-9: メモ化パターンの目的は？ → 再帰計算の重複を避けて高速化

Q20-10: クロージャで状態を保持するには？ → `let` でスコープ内変数をキャプチャ

Q20-11: セル枯渇の最初の対処は？ → Run GC now

---

[← 第19章 ビルドと書き込み](19_build.md) | [目次](#/) | [付録へ戻る](#/)
