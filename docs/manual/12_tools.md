# 第12章 ツール
[← 第11章 科学資料](11_sciref.md) | [目次](#/) | [第13章 設定 →](13_settings.md)
---
## この章で学ぶこと

- Tools モードの全体像
- Spreadsheet（45×26）の操作・数式・CSV I/O
- Sheet ⇄ Statistics 連携・ソート・フィルタ
- uLisp インタプリタ（REPL・特殊形式・I/O・GC）
- uLisp 実用例（階乗・フィボナッチ・素数・統計）
- Wi-Fi 接続（2.4GHz のみ）・Maxima 連携
- Power / About（バッテリー・SD 情報）
- Settings（NVS 永続化）
- BMP スクリーンショット
- 統合使用例

---

## 12.1 Tools モードとは
**Tools モード** は、PoMaS の便利機能を集めたモードです。
電卓・統計・グラフ以外の、ちょっとした作業をここで行います。
### 搭載ツール
| ツール | 内容 |
|---|---|
| **Spreadsheet** | 45×26 の表計算 |
| **uLisp Interpreter** | Lisp プログラミング環境 |
| **Wi-Fi / Maxima** | ネット接続・外部計算エンジン |
| **Power / About** | バッテリー・SD カード情報 |
| **Settings (NVS)** | 設定の永続化 |
---
## 12.2 Tools モードの起動
メインメニュー → `6` または Enter で **Tools** を選択。
### メニュー
```
┌─────────────────────────────────┐
│▌Tools                           │
├─────────────────────────────────┤
│1 Spreadsheet (45x26)            │
│2 uLisp Interpreter              │
│3 Wi-Fi / Maxima                 │
│4 Power / About                  │
│5 Settings (NVS persistent)      │
│6 Save BMP screenshot now        │
│7 Back                           │
└─────────────────────────────────┘
```
---
## 12.3 Spreadsheet（スプレッドシート）
### 概要
**45 行 × 26 列**の表計算シート。
| 項目 | 値 |
|---|---|
| 行数 | 45（1〜45） |
| 列数 | 26（A〜Z） |
| セル参照 | A1, B3, Z45 など |
| 数式 | `=A1+B1` など |
### 起動
```
Tools → 1 (Spreadsheet)
```
### 画面レイアウト
```
┌─────────────────────────────────┐
│▌Sheet A1                        │  ← ヘッダ（現在のセル）
├─────────────────────────────────┤
│  │ A  │ B  │ C  │ D  │           │
│1 │10  │20  │    │    │           │
│2 │30  │40  │    │    │           │
│3 │    │    │    │    │           │
│4 │    │    │    │    │           │
├─────────────────────────────────┤
│ENT:edit T:Lists S:save L:load   │
└─────────────────────────────────┘
```
### 基本操作
| キー | 動作 |
|---|---|
| `;` `.` `,` `/` | セル移動 |
| `Enter` | セル編集 |
| `Del` | セル消去 |
| `S` | CSV 保存 |
| `L` | CSV 読み込み |
| `T` | Sheet ⇄ L1..L8 相互変換 |
| `Fn + Esc` | Tools メニューへ |
### セル編集
**例: A1 に 100 を入力**
```
[1] A1 にカーソルを移動
[2] Enter
[3] "100" と入力
[4] Enter
→ A1 に 100 が入る
```
### 数式の入力
**例: A3 に =A1+A2 を入力（合計）**
```
[1] A3 にカーソルを移動
[2] Enter
[3] "=A1+A2" と入力
[4] Enter
→ A3 に A1+A2 の計算結果が表示される
```
### 使用できる数式
| 式 | 内容 |
|---|---|
| `=A1+B1` | セル参照 |
| `=A1*2` | 定数倍 |
| `=A1/10` | 除算 |
| `=SUM(...)` は不可 | 関数は非対応 |
**SUM, AVERAGE 等の集計関数は未対応**
現バージョンでは、基本的な四則演算のみ対応。
集計は手動で行うか、Statistics モードを使ってください。
### 例: 九九の表を作る
```
[1] B1 に 1、C1 に 2、D1 に 3 ...（1 行目）
[2] A2 に 1、A3 に 2、A4 に 3 ...（A 列）
[3] B2 に =B$1*$A2 のような絶対参照は未対応
→ 各セルに手動で =B1*A2 等を入力
```
**絶対参照（$）は未対応**
現状では、各セルに個別に数式を入力する必要があります。
### CSV 保存（S）
```
[1] S キー
[2] Path: /sheet.csv（デフォルト）
[3] Enter
→ SD カードに保存
```
### CSV 読み込み（L）
```
[1] L キー
[2] Path: /sheet.csv（デフォルト）
[3] Enter
→ CSV から復元
```
### Sheet ⇄ L1..L8（T）
シートと Statistics のリストを相互変換。
```
[1] T キー
[2] メニュー:
    1 Sheet A..H → L1..L8
    2 L1..L8 → Sheet A..H
    3 Cancel
```
**用途**:
- シートで整理したデータを Statistics で統計解析
- Statistics のリストをシートで可視化
---
## 12.4 uLisp Interpreter（Lisp プログラミング）
### 概要
**μLisp** は、Lisp 言語のサブセットを実装したプログラミング環境です。
### Lisp とは
**Lisp（リスプ）** は、1958 年に開発された、現存する最古の高水準プログラミング言語の一つです。
**特徴**:
- 括弧 `()` で式を表現
- 関数型プログラミング
- シンプルな構文（`(関数 引数1 引数2 ...)`）
- リスト処理が得意
**例: 足し算**
```lisp
(+ 1 2 3)   ; → 6
```
### 起動
```
Tools → 2 (uLisp)
```
### メニュー
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
### REPL（対話モード）
REPL は **Read-Eval-Print Loop** の略。
入力 → 評価 → 表示 → 繰り返し。
```
Tools → 2 → 1 (REPL)
```
```
┌─────────────────────────────────┐
│▌uLisp REPL                      │
├─────────────────────────────────┤
│(+ 1 2) -> 3                     │
│(* 5 6) -> 30                    │
│                                 │
│> _                              │
├─────────────────────────────────┤
│ENT:eval Fn+E:exit ESC:back      │
└─────────────────────────────────┘
```
### 基本例
| 入力 | 結果 |
|---|---|
| `(+ 1 2 3)` | 6 |
| `(* 5 6)` | 30 |
| `(- 10 3)` | 7 |
| `(/ 20 4)` | 5 |
| `(mod 10 3)` | 1 |
| `(sqrt 16)` | 4 |
| `(sin 0)` | 0 |
| `(< 3 5)` | #t（真） |
| `(= 5 5)` | #t |
### 変数の定義
```lisp
(define x 42)     ; x = 42
(define y 100)    ; y = 100
(+ x y)           ; → 142
```
### 関数の定義
```lisp
(define (square n) (* n n))
(square 5)        ; → 25
```
### 例: 階乗
```lisp
(define (fact n)
  (if (<= n 1)
      1
      (* n (fact (- n 1)))))
(fact 5)   ; → 120
```
### 例: フィボナッチ数列
```lisp
(define (fib n)
  (if (< n 2)
      n
      (+ (fib (- n 1)) (fib (- n 2)))))
(fib 10)   ; → 55
```
### 例: クロージャ（関数を返す関数）
```lisp
(define (make-adder n)
  (lambda (x) (+ x n)))
(define add5 (make-adder 5))
(add5 3)   ; → 8
```
### 高階関数
| 関数 | 例 |
|---|---|
| `map` | `(map (lambda (x) (* x x)) '(1 2 3))` → `(1 4 9)` |
| `filter` | `(filter (lambda (x) (> x 2)) '(1 2 3 4))` → `(3 4)` |
| `reduce` | `(reduce + 0 '(1 2 3 4))` → 10 |
### リスト操作
| 関数 | 例 |
|---|---|
| `car` | `(car '(1 2 3))` → 1 |
| `cdr` | `(cdr '(1 2 3))` → (2 3) |
| `cons` | `(cons 1 '(2 3))` → (1 2 3) |
| `list` | `(list 1 2 3)` → (1 2 3) |
| `length` | `(length '(1 2 3))` → 3 |
| `reverse` | `(reverse '(1 2 3))` → (3 2 1) |
### 条件分岐
```lisp
(if 条件 真の場合 偽の場合)
(cond ((条件1) 結果1)
      ((条件2) 結果2)
      (else デフォルト))
```
**例**:
```lisp
(if (> 5 3) "big" "small")   ; → "big"
```
### ループ
```lisp
(while 条件
  式1
  式2
  ...)
```
### スクリプトファイルの実行
**Step 1: PC で .lsp ファイルを作成**
```lisp
(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))
(print (fact 10))
```
**Step 2: SD カードに保存**（例: `/script.lsp`）
**Step 3: PoMaS で実行**
```
Tools → 2 → 2 (Run script)
Path: /script.lsp
→ 実行結果が表示される
```
### ガベージコレクション（GC）
μLisp は自動メモリ管理（GC）を持ちますが、
PoMaS では**明示的な (gc) 呼び出しを無効化**しています
（安全性のため）。
**GC のタイミング**:
- メニューから `Run GC now` を実行
- または REPL 起動時に自動実行（環境数が 100 を超えた場合）
**セルプール枯渇時**:
```
ERR: μLisp cell pool exhausted
```
→ `Tools → 2 → 3 (Run GC now)` で回収
### Reset interpreter
インタプリタを完全リセット。
```
Tools → 2 → 4 (Reset)
→ すべての定義・変数がクリア
```
### Examples（サンプル集）
```
Tools → 2 → 5 (Examples)
```
以下のサンプルコードが実行できます：
- `(+ 1 2 3)` — 足し算
- `(define (sq x) (* x x))` — 関数定義
- 階乗（再帰）
- map（高階関数）
- 統計デモ
### 組み込み関数一覧
**算術**: `+ - * / mod`
**比較**: `< > <= >= =`
**リスト**: `car cdr cons list null? atom? length append reverse nth`
**高階**: `map filter reduce apply`
**数学**: `sin cos tan exp log sqrt abs floor ceil round expt pow`
**統計**: `mean sum sd`
**その他**: `print save load euler pi random`
---
## 12.5 Wi-Fi / Maxima
### 概要
Wi-Fi 接続と、外部の数式処理システム **Maxima** との連携機能。
### 起動
```
Tools → 3 (Wi-Fi / Maxima)
```
### メニュー
```
┌─────────────────────────────────┐
│▌Wi-Fi / Maxima                  │
├─────────────────────────────────┤
│1 Set SSID/Pass                  │
│2 Scan networks                  │
│3 Connect                        │
│4 Disconnect                     │
│5 Status                         │
│6 Send to Maxima (URL)           │
│7 Back                           │
└─────────────────────────────────┘
```
### SSID/Pass の設定（1）
**手動入力**:
```
[1] Tools → 3 → 1
[2] SSID: MyWiFi
[3] Pass: mypassword
→ NVS に保存される
```
### ネットワークスキャン（2）
周囲の Wi-Fi ネットワークを検索。
```
[1] Tools → 3 → 2
→ "Scanning..." 表示
→ 利用可能な SSID 一覧（最大 30 件）
```
**表示例**:
```
* MyWiFi          -45 dBm
  GuestWiFi       -62 dBm
* Office-5G       -78 dBm
```
`*` はセキュリティあり（暗号化されている）。
### 接続（3）
**手順**:
```
[1] Tools → 3 → 3 (Connect)
→ "Connecting..." 表示
→ 接続成功: IP アドレス表示
→ 接続失敗: 理由表示
```
**接続成功時**:
```
IP: 192.168.1.100
RSSI: -45 dBm
MAC: aa:bb:cc:dd:ee:ff
```
**接続失敗の理由**:
| メッセージ | 原因 |
|---|---|
| SSID not found | SSID 間違い / 範囲外 |
| Auth failed | パスワード間違い |
| Timeout | 応答なし |
**2.4GHz 帯のみ対応**
5GHz 帯の Wi-Fi には接続できません（ESP32-S3 の仕様）。
### 切断（4）
Wi-Fi 接続を切断。
### ステータス（5）
現在の Wi-Fi 状態を表示。
### Maxima 連携（6）
外部の Maxima サーバーに数式を送信して計算結果を受け取ります。
**手順**:
```
[1] Tools → 3 → 6
[2] URL prefix: http://your-proxy/maxima?expr=
[3] Maxima expr: diff(sin(x),x)
→ HTTP リクエスト送信
→ 結果表示
```
**重要：クライアント側のみ実装（サーバーは各自用意）**
PoMaS 側には数式を HTTP GET で送信するクライアントコードのみが実装されています。
Maxima 本体および HTTP プロキシサーバーは**同梱されていません**。
デフォルトの URL は `YOUR-PROXY-HOST`（プレースホルダ）です。
利用するには、以下のようなサーバーを各自で構築し、PoMaS から URL を指定してください：
1. Maxima 本体が動作するサーバー（Raspberry Pi / PC 等）
2. HTTP GET で `expr` パラメータを受け取り、Maxima を呼び出して結果を返す CGI/PHP/Python スクリプト
3. PoMaS の [2] URL prefix にそのエンドポイントを入力

サーバー未構築の場合、Maxima 連携は HTTP エラーで終了します（**バグではありません**）。
**セルフホスト例**:
- Raspberry Pi に Maxima をインストール
- CGI スクリプトで HTTP エンドポイントを公開
- PoMaS から URL で指定
---
## 12.6 Power / About
### 起動
```
Tools → 4 (Power / About)
```
### 表示内容
```
┌─────────────────────────────────┐
│▌Power / About                   │
├─────────────────────────────────┤
│Battery: 78% (charging)          │
│PSRAM: none (normal)             │
│SD: OK (16384MB)                 │
│Wi-Fi: MyWiFi                    │
│Free heap: 125 KB                │
│                                 │
│PoMaS ver.0.92                   │
│Created by YNGMT                 │
└─────────────────────────────────┘
```
### 各項目
| 項目 | 内容 |
|---|---|
| Battery | バッテリー残量・充電状態 |
| PSRAM | PSRAM 有無（Cardputer-Adv は非搭載） |
| SD | SD カード状態・容量 |
| Wi-Fi | 接続中の SSID |
| Free heap | 空きヒープメモリ |
### 操作
| キー | 動作 |
|---|---|
| `R` | SD カードを再初期化 |
| `T` | SD カードのテスト |
| `Fn + Esc` | Tools メニューへ |
### SD カード再初期化（R）
SD カードが認識されない場合の再試行。
```
[1] R キー
→ "Re-initializing SD..." 表示
→ 成功: SD card detected! / 容量・速度表示
→ 失敗: SD init failed / チェック項目表示
```
### SD カードテスト（T）
SD カードの読み書きテスト。
```
[1] T キー
→ 書き込みテスト
→ 読み込みテスト
→ クリーンアップ
→ "ALL TESTS PASSED" 表示
```
---
## 12.7 Settings（NVS 永続化）
詳細は **第13章 設定** で扱います。
ここでは概要のみ。
### 起動
```
Tools → 5 (Settings)
```
### 設定項目
| 項目 | 内容 |
|---|---|
| Angle | DEG / RAD / GRAD |
| Display | Norm / Fix / Sci / Eng |
| Digits | 表示桁数（0〜9） |
| Norm | Norm 1 / Norm 2 |
| Save | 手動保存 |
| Clear NVS | 設定リセット |
---
## 12.8 BMP スクリーンショット
### Save BMP screenshot now（Tools → 6）
現在の画面を SD カードに BMP 保存。
```
[1] Tools → 6
→ /screen_XXXXX.bmp として保存
```
各画面でも `S` キーで保存できます（画面内に応じて）。
### BMP 仕様
| 項目 | 値 |
|---|---|
| サイズ | 240 × 135 px |
| 色深度 | 24 bit RGB |
| 形式 | 標準 BMP |
| ファイルサイズ | 約 97 KB |
---
## 12.9 実用例
### 例 1: μLisp で統計計算
**問題**: 平均と標準偏差を計算
```
Tools → 2 → 1 (REPL)
(define data '(10 20 30 40 50))
(mean data)   ; → 30
(sd data)     ; → 15.811
(sum data)    ; → 150
```
### 例 2: μLisp で素数を探す
```
(define (prime? n)
  (if (< n 2)
      #f
      (let ((i 2))
        (while (<= (* i i) n)
          (if (= (mod n i) 0) #f)
          (define i (+ i 1))))))
```
（簡易版、実装に応じた動作）
### 例 3: スプレッドシートで家計簿
```
A 列: 日付（1/1, 1/2, ...）
B 列: 支出（1000, 500, ...）
C 列: 累計（=B1, =C1+B2, =C2+B3, ...）
```
### 例 4: Wi-Fi 接続 + Maxima
```
[1] Wi-Fi 接続
[2] Maxima 連携で微分計算
[3] 結果を Ans に転送
```
---
## 12.10 トラブルシューティング
### スプレッドシート
| 症状 | 対処 |
|---|---|
| 数式が計算されない | `=` で始まっているか確認 |
| セル参照が効かない | セル番号は A1, B3 の形式 |
| CSV 保存失敗 | SD カードを確認 |
### μLisp
| 症状 | 対処 |
|---|---|
| `ERR: unbound symbol` | 変数・関数未定義。またはスペルミス |
| `ERR: cell pool exhausted` | Run GC now を実行 |
| 無限ループ | ESC で中断、Reset interpreter |
### Wi-Fi
| 症状 | 対処 |
|---|---|
| 接続できない | 2.4GHz 帯か確認、パスワード再確認 |
| SSID が見えない | スキャン再実行、範囲内か確認 |
### SD カード
| 症状 | 対処 |
|---|---|
| 認識されない | R でリトライ、カード挿し直し |
| 保存できない | FAT32 か確認、容量確認 |
---
## 12.11 Spreadsheet 詳細操作

### 12.11.1 セルアドレス体系

**列**: A〜Z（26 列）、**行**: 1〜45（45 行）

**セルアドレス例**:
- A1（左上隅）
- Z45（右下隅）
- B3（B 列 3 行目）
- AA1（26 列目、実装範囲外）

**範囲指定**: 単一セルのみ対応（A1:B5 形式は未対応）

### 12.11.2 データ入力の種類

| 種類 | 入力例 | 表示 |
|---|---|---|
| 数値 | `100`, `3.14`, `-5` | そのまま |
| 数式 | `=A1+B1` | 計算結果 |
| 文字列 | `Hello` | 文字列そのまま |
| 空白 | （空） | 空欄 |

### 12.11.3 数式の評価

**計算タイミング**: セル表示時に毎回評価（再帰深度 20 まで）

**使用可能な演算子**:
- `+` `-` `*` `/` `%`
- `^`（べき乗）
- `()`（括弧）
- 比較演算子は未対応

**組み込み関数**:
- `SUM(A1:A5)` → A1 から A5 の合計
- `AVG(A1:A5)` → 平均
- `MIN`, `MAX`, `COUNT`, `PROD`
- `IF(cond, a, b)` → 条件分岐

**例**:
```
A1: 10
A2: 20
A3: 30
B1: =SUM(A1:A3)     → 60
B2: =AVG(A1:A3)     → 20
B3: =IF(A1>5, 100, 0) → 100
```

### 12.11.4 セル参照の展開

数式内の `A1` は自動的に `(10)` のように値に置換されます。

**例**: `=A1+A2*2` → `(10)+(20)*2` → 50

**循環参照**: 深度 20 で打ち切り、NaN を返す

### 12.11.5 セル移動

| キー | 動作 |
|---|---|
| `Fn + ;` | 上移動 |
| `Fn + .` | 下移動 |
| `Fn + ,` | 左移動 |
| `Fn + /` | 右移動 |

### 12.11.6 編集モード

`Enter` で編集開始 → 文字入力 → `Enter` で確定 → `ESC` でキャンセル

### 12.11.7 削除

`Del` でセル内容を消去。

### 12.11.8 スクロール

カーソルが画面外に出ると自動スクロール。右端にスクロールバー表示。

---

## 12.12 Spreadsheet 実用例

### 12.12.1 家計簿

```
    A      B      C      D
1  日付   収入   支出   残高
2  1/1    0      500    =-C2
3  1/2    1000   300    =D2+B3-C3
4  1/3    0      800    =D3+B4-C4
5  1/4    500    200    =D4+B5-C5
```

**操作**:
1. D2 に `=-C2`
2. D3 に `=D2+B3-C3`
3. D4 以降は D3 をコピーするか手動入力

### 12.12.2 成績表

```
    A      B      C      D      E      F      G
1  名前   国語   数学   英語   理科   社会   合計
2  太郎   80     90     85     75     70     =SUM(B2:F2)
3  次郎   70     75     80     85     90     =SUM(B3:F3)
4  三郎   90     85     75     80     85     =SUM(B4:F4)
```

**G 列の合計**: `=SUM(B2:F2)`

### 12.12.3 九九の表

```
    A      B      C      D      E
1          1      2      3      4
2   1      =B$1*$A2  ← 絶対参照は未対応
3   2      =C1*A3
4   3      =D1*A4
```

**注意**: 絶対参照（$）未対応のため各セル個別入力

### 12.12.4 平均・標準偏差

```
    A
1   10
2   20
3   30
4   40
5   50
6   =AVG(A1:A5)      → 30
7   =SUM(A1:A5)      → 150
8   =COUNT(A1:A5)    → 5
9   =MIN(A1:A5)      → 10
10  =MAX(A1:A5)      → 50
```

---

## 12.13 Spreadsheet ↔ Statistics 連携

### 12.13.1 Sheet → L1..L8 変換

**操作**:
```
[1] Sheet 画面で T キー
[2] "Sheet A..H → L1..L8"
→ 各列の数値が L1〜L8 に転送される
```

**用途**: シートで整理したデータを統計モードで解析

**注意**: A 列 → L1、B 列 → L2、...、H 列 → L8

### 12.13.2 L1..L8 → Sheet 変換

**操作**:
```
[1] Sheet 画面で T キー
[2] "L1..L8 → Sheet A..H"
→ 各リストの値が A〜H 列に転送される
```

**用途**: 統計モードのデータをシートで可視化

### 12.13.3 ソート

**操作**:
```
[1] Sheet 画面で T キー
[2] "Sort rows by column"
[3] Sort by col: A（A〜Z から選択）
[4] Order: Ascending / Descending
```

**用途**: データを列の値で並べ替え

### 12.13.4 フィルタ

**操作**:
```
[1] Sheet 画面で T キー
[2] "Filter rows to list"
[3] Filter col: B
[4] Op: =, <, >, !=
[5] Value: 100
[6] Dest list: L1
→ B 列が 100 の行の B 列値が L1 に転送
```

**用途**: 条件に合うデータの抽出

---

## 12.14 Spreadsheet CSV 入出力

### 12.14.1 CSV 保存

**操作**:
```
[1] S キー
[2] Path: /sheet.csv
[3] Enter
→ SD カードに保存
```

**形式**: 各セルをカンマ区切り、行を改行で区切り

**例**:
```
日付,収入,支出,残高
1/1,0,500,-500
1/2,1000,300,200
```

### 12.14.2 CSV 読み込み

**操作**:
```
[1] L キー
[2] Path: /sheet.csv
[3] Enter
→ シートに復元
```

**注意**: 既存データは上書き、行数 45 列 26 まで

### 12.14.3 CSV の互換性

Excel / LibreOffice / Google Sheets と互換。PC で編集した CSV を SD カードにコピーして読み込めます。

---

## 12.15 uLisp 詳細リファレンス

### 12.15.1 Lisp の評価モデル

**全式が S 式**: `(関数 引数1 引数2 ...)`

**評価順**:
1. シンボル → 環境から値を探す
2. リスト → 先頭要素を関数として評価
3. 数値・文字列 → そのまま

### 12.15.2 REPL の操作

**画面**:
```
┌─────────────────────────────────┐
│▌uLisp REPL                      │
├─────────────────────────────────┤
│(+ 1 2) -> 3                     │
│(* 5 6) -> 30                    │
│(list 1 2 3) -> (1 2 3)          │
│                                 │
│> _                              │
├─────────────────────────────────┤
│ENT:eval Fn+E:exit ESC:back      │
└─────────────────────────────────┘
```

**キー操作**:
- 文字キー: 式を入力
- `Enter`: 評価実行
- `Del`: 1 文字削除
- `Fn + E`: REPL を抜ける
- `ESC`: メニューに戻る

### 12.15.3 算術の詳細

**可変長引数**:
```
(+ 1 2 3 4 5)    → 15
(*)              → 1
(+)              → 0
```

**単項マイナス**:
```
(- 5)            → -5
(- 10 3)         → 7
(- 10 3 2)       → 5
```

**除算**:
```
(/ 20 4)         → 5
(/ 20 4 5)       → 1
(/ 5)            → 0.2
```

### 12.15.4 比較の詳細

| 関数 | 意味 | 例 |
|---|---|---|
| `=` | 等号 | `(= 5 5)` → `#t` |
| `equal?` | 等号（同義） | `(equal? "a" "a")` → `#t` |
| `<` | 小なり | `(< 3 5)` → `#t` |
| `>` | 大なり | `(> 5 3)` → `#t` |
| `<=` | 以下 | `(<= 5 5)` → `#t` |
| `>=` | 以上 | `(>= 5 3)` → `#t` |

### 12.15.5 リスト操作の詳細

**car / cdr**:
```
(car '(1 2 3))    → 1
(cdr '(1 2 3))    → (2 3)
(car nil)         → nil
(cdr nil)         → nil
```

**cons**:
```
(cons 1 '(2 3))   → (1 2 3)
(cons 1 nil)      → (1)
(cons 1 (cons 2 nil)) → (1 2)
```

**append / reverse**:
```
(append '(1 2) '(3 4))      → (1 2 3 4)
(append '(1) '(2) '(3))     → (1 2 3)
(reverse '(1 2 3))          → (3 2 1)
```

**nth**:
```
(nth 0 '(a b c))  → a
(nth 2 '(a b c))  → c
(nth 5 '(a b c))  → nil
```

### 12.15.6 数学関数の詳細

| 関数 | 例 |
|---|---|
| `sin` | `(sin 0)` → 0（radian） |
| `cos` | `(cos 0)` → 1 |
| `tan` | `(tan 0)` → 0 |
| `exp` | `(exp 1)` → 2.718 |
| `log` | `(log 2.718)` → 1 |
| `sqrt` | `(sqrt 16)` → 4 |
| `abs` | `(abs -5)` → 5 |
| `floor` | `(floor 3.7)` → 3 |
| `ceil` | `(ceil 3.2)` → 4 |
| `round` | `(round 3.5)` → 4 |
| `min` | `(min 3 5 1)` → 1 |
| `max` | `(max 3 5 1)` → 5 |
| `expt` | `(expt 2 10)` → 1024 |
| `pow` | `(pow 2 0.5)` → 1.414 |
| `random` | `(random)` → 0〜1 |

### 12.15.7 統計関数

```
(sum '(1 2 3 4 5))    → 15
(mean '(10 20 30))    → 20
(sd '(10 20 30 40 50)) → 15.811
```

### 12.15.8 高階関数

**map**:
```
(map (lambda (x) (* x x)) '(1 2 3 4 5))
→ (1 4 9 16 25)
```

**filter**:
```
(filter (lambda (x) (> x 2)) '(1 2 3 4 5))
→ (3 4 5)
```

**reduce**:
```
(reduce + 0 '(1 2 3 4 5))    → 15
(reduce * 1 '(1 2 3 4 5))    → 120
```

**apply**:
```
(apply + '(1 2 3 4 5))       → 15
(apply max '(3 7 2 9 1))     → 9
```

---

## 12.16 uLisp 特殊形式 詳細

### 12.16.1 quote（評価抑止）

```
(quote x)        → x（シンボル x そのもの）
'x               → x
'(1 2 3)         → (1 2 3)
(quote (a b))    → (a b)
```

### 12.16.2 if（条件分岐）

```
(if cond then else)
(if (= 1 1) "yes" "no")     → "yes"
(if (= 1 2) "yes" "no")     → "no"
(if (> 5 3) 100)            → 100  （else 省略時は nil）
```

### 12.16.3 cond（多分岐）

```
(cond
  ((条件1) 結果1)
  ((条件2) 結果2)
  (else    デフォルト))
```

**例**:
```
(cond ((> 5 10) "big")
      ((< 5 0)  "neg")
      (else     "mid"))
→ "mid"
```

### 12.16.4 and / or

```
(and #t #t)     → #t
(and #t nil)    → nil
(and 1 2 3)     → 3（最後の真値）
(or nil #t)     → #t
(or 1 2 3)      → 1（最初の真値）
```

### 12.16.5 not

```
(not #t)        → nil
(not nil)       → #t
(not 5)         → nil
```

### 12.16.6 define（変数・関数定義）

```
(define x 42)               ; 変数
(define (square n) (* n n)) ; 関数
```

### 12.16.7 setq（再代入）

```
(define x 10)
(setq x 20)
x                            → 20
```

### 12.16.8 lambda（無名関数）

```
(lambda (x) (* x x))        ; 関数
((lambda (x) (* x x)) 5)    → 25
```

### 12.16.9 let（ローカル束縛）

```
(let ((a 10) (b 20))
  (+ a b))
→ 30
```

### 12.16.10 while（ループ）

```
(define i 0)
(define s 0)
(while (< i 5)
  (setq s (+ s i))
  (setq i (+ i 1)))
s                            → 10
```

### 12.16.11 begin / progn（順次実行）

```
(begin
  (print "a")
  (print "b")
  42)
→ 42
```

---

## 12.17 uLisp I/O 関数

### 12.17.1 print（表示）

**用途**: 値をシリアルモニタに出力（115200 bps）。

```
(print "Hello")
(print (+ 1 2 3))
(print (list 1 2 3))
```

**表示先**: PC の Arduino IDE → シリアルモニタ

### 12.17.2 save（SD 保存）

```
(save "data.lsp" '(1 2 3 4 5))
(save "data.lsp" (lambda (x) (* x x)))
```

**保存先**: SD カードルート（`/data.lsp`）

### 12.17.3 load（SD 読み込み）

```
(load "data.lsp")
→ (1 2 3 4 5)   （パース結果を返す）
```

**注意**: load は S 式をパースして返すだけ。評価は `Run script from SD` メニューを使う。

---

## 12.18 uLisp 実用例集

### 12.18.1 階乗

```
(define (fact n)
  (if (<= n 1) 1 (* n (fact (- n 1)))))
(fact 5)     → 120
(fact 10)    → 3628800
```

### 12.18.2 フィボナッチ

```
(define (fib n)
  (if (< n 2) n
    (+ (fib (- n 1)) (fib (- n 2)))))
(fib 10)     → 55
```

### 12.18.3 素数判定

```
(define (prime? n)
  (if (< n 2) nil
    (if (= n 2) #t
      (if (= (mod n 2) 0) nil #t))))
(prime? 97)  → #t
(prime? 100) → nil
```

### 12.18.4 リストの二乗

```
(map (lambda (x) (* x x)) '(1 2 3 4 5))
→ (1 4 9 16 25)
```

### 12.18.5 偶数のみ抽出

```
(filter (lambda (x) (= (mod x 2) 0)) '(1 2 3 4 5 6))
→ (2 4 6)
```

### 12.18.6 統計計算

```
(define data '(10 20 30 40 50))
(sum data)   → 150
(mean data)  → 30
(sd data)    → 15.811
```

### 12.18.7 三角関数表

```
(define deg2rad (lambda (d) (* d (/ pi 180))))
(define angles '(0 30 45 60 90))
(map (lambda (d) (sin (deg2rad d))) angles)
→ (0 0.5 0.707 0.866 1)
```

### 12.18.8 クロージャ（カウンタ）

```
(define (make-counter)
  (let ((count 0))
    (lambda ()
      (setq count (+ count 1))
      count)))
(define c (make-counter))
(c)  → 1
(c)  → 2
(c)  → 3
```

---

## 12.19 Wi-Fi 詳細

### 12.19.1 2.4GHz 帯のみ対応

**重要**: ESP32-S3 は 2.4GHz 帯のみ対応。5GHz の AP には接続できません。

### 12.19.2 SSID/Pass の設定

**方法 1: 手動入力**:
```
[1] Tools → 3 → 1
[2] SSID: MyWiFi
[3] Pass: mypassword
→ NVS に保存
```

**方法 2: スキャン → 選択**:
```
[1] Tools → 3 → 2 (Scan)
[2] 一覧から AP 選択
[3] Pass 入力
→ NVS に保存
```

### 12.19.3 接続フロー

```
[1] Tools → 3 → 3 (Connect)
→ "Connecting..." 表示
→ 最大 20 秒待機
→ 成功: IP 表示
→ 失敗: 理由表示
```

### 12.19.4 接続失敗の理由

| メッセージ | 原因 | 対処 |
|---|---|---|
| SSID not found | SSID 誤り or 範囲外 | 再スキャン |
| Auth failed | パスワード誤り | WPA2 対応か確認 |
| Timeout | 応答なし | 電波状況確認 |

### 12.19.5 Maxima 連携

**前提**: Maxima + HTTP プロキシサーバーのセルフホスト必須

**手順**:
```
[1] Wi-Fi 接続
[2] Tools → 3 → 6 (Send to Maxima)
[3] URL prefix: http://your-proxy/maxima?expr=
[4] Maxima expr: diff(sin(x),x)
[5] 結果表示
```

**サーバー例**: Raspberry Pi に Maxima をインストール、CGI/PHP/Python で HTTP エンドポイント作成。

---

## 12.20 Power / About 詳細

### 12.20.1 表示内容

```
Battery: 78% (charging)
PSRAM: none (normal)
SD: OK (16384MB)
Wi-Fi: MyWiFi
Free heap: 125 KB

PoMaS ver.0.92
Created by YNGMT
```

### 12.20.2 操作

| キー | 動作 |
|---|---|
| `R` | SD カード再初期化 |
| `T` | SD カードテスト |
| `ESC` | 戻る |

### 12.20.3 SD 再初期化（R）

**用途**: SD カードが認識されない場合の再試行。

**処理**: 4 段階のクロック（25MHz → 1MHz）でリトライ

### 12.20.4 SD テスト（T）

**処理**:
1. `/pomas_test.txt` に書き込み
2. 読み込み確認
3. 削除

**成功**: `ALL TESTS PASSED`

---

## 12.21 統合使用例

### 12.21.1 データ収集 → 統計 → 可視化

```
[1] Sheet に測定データを入力（A 列 = x、B 列 = y）
[2] T キー → "Sheet A..H → L1..L8"
[3] Statistics → 2 (Regression) → 1 (Linear)
[4] X: L1, Y: L2 → 回帰式取得
[5] Statistics → 10 → 3 (Scatter) で散布図確認
[6] Graph → F1 = 回帰式で理論線を重ねる
```

### 12.21.2 uLisp でカスタム計算 → Statistics

```
[1] uLisp REPL で (define data '(1 2 3 4 5 6 7 8 9 10))
[2] (mean data) → 5.5
[3] (sd data) → 3.028
[4] 結果を Ans にコピーして Statistics で活用
```

### 12.21.3 Wi-Fi 経由で外部計算

```
[1] Wi-Fi 接続
[2] Maxima 連携で記号積分:
  integrate(sin(x)*cos(x), x)
→ sin(x)^2/2
[3] 結果を Ans にコピーして CAS で検算
```

---

## 12.22 この章のまとめ
- Tools モードには 5 つのツール
- Spreadsheet: 45×26 の表計算（基本四則のみ）
- uLisp: Lisp サブセット、80+ の組み込み関数
- Wi-Fi: 2.4GHz 帯のみ、Maxima 連携は要プロキシ
- Power/About: バッテリー・SD 情報
- Settings: 設定は NVS に永続化
### 確認問題
**Q12-1**: スプレッドシートのセル数を入力するとき、正しい形式は？
<details><summary>答え</summary>
A1, B3, Z45 のような形式（列の文字 + 行の数字）
</details>
**Q12-2**: μLisp で階乗を計算する関数を定義するには？
<details><summary>答え</summary>
`(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))`
</details>
**Q12-3**: μLisp でセルプールが枯渇したらどうする？
<details><summary>答え</summary>
Tools → 2 → 3 (Run GC now) を実行、または Reset interpreter。
</details>
**Q12-4**: Wi-Fi 接続で 5GHz 帯が使えない理由は？
<details><summary>答え</summary>
ESP32-S3 が 2.4GHz 帯のみ対応のため。
</details>
**Q12-5**: Maxima 連携を使うには何が必要？
<details><summary>答え</summary>
Maxima サーバーをセルフホストし、URL プレフィックスを設定する。
</details>
**Q12-6**: 画面のスクリーンショットを保存するには？
<details><summary>答え</summary>
Tools → 6、または各画面で S キー。
</details>
---
[← 第11章 科学資料](11_sciref.md) | [目次](#/) | [第13章 設定 →](13_settings.md)
