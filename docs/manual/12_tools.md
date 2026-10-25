# 第12章 ツール

[← 第11章 科学資料](11_sciref.md) | [目次](index.md) | [第13章 設定 →](13_settings.md)

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

⚠️ **SUM, AVERAGE 等の集計関数は未対応**
現バージョンでは、基本的な四則演算のみ対応。
集計は手動で行うか、Statistics モードを使ってください。

### 例: 九九の表を作る

```
[1] B1 に 1、C1 に 2、D1 に 3 ...（1 行目）
[2] A2 に 1、A3 に 2、A4 に 3 ...（A 列）
[3] B2 に =B$1*$A2 のような絶対参照は未対応
→ 各セルに手動で =B1*A2 等を入力
```

⚠️ **絶対参照（$）は未対応**
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

⚠️ **2.4GHz 帯のみ対応**
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

⚠️ **プロキシ設定が必要**
デフォルトの URL は `YOUR-PROXY-HOST`（プレースホルダ）です。
実際に使うには、自分で Maxima サーバーを立てるか、
HTTP プロキシを経由する必要があります。

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

## 12.11 この章のまとめ

- Tools モードには 5 つのツール
- Spreadsheet: 45×26 の表計算（基本四則のみ）
- uLisp: Lisp サブセット、80+ の組み込み関数
- Wi-Fi: 2.4GHz 帯のみ、Maxima 連携は要プロキシ
- Power/About: バッテリー・SD 情報
- Settings: 設定は NVS に永続化

### 確認問題

📝 **Q12-1**: スプレッドシートのセル数を入力するとき、正しい形式は？
<details><summary>答え</summary>
A1, B3, Z45 のような形式（列の文字 + 行の数字）
</details>

📝 **Q12-2**: μLisp で階乗を計算する関数を定義するには？
<details><summary>答え</summary>
`(define (fact n) (if (<= n 1) 1 (* n (fact (- n 1)))))`
</details>

📝 **Q12-3**: μLisp でセルプールが枯渇したらどうする？
<details><summary>答え</summary>
Tools → 2 → 3 (Run GC now) を実行、または Reset interpreter。
</details>

📝 **Q12-4**: Wi-Fi 接続で 5GHz 帯が使えない理由は？
<details><summary>答え</summary>
ESP32-S3 が 2.4GHz 帯のみ対応のため。
</details>

📝 **Q12-5**: Maxima 連携を使うには何が必要？
<details><summary>答え</summary>
Maxima サーバーをセルフホストし、URL プレフィックスを設定する。
</details>

📝 **Q12-6**: 画面のスクリーンショットを保存するには？
<details><summary>答え</summary>
Tools → 6、または各画面で S キー。
</details>

---

[← 第11章 科学資料](11_sciref.md) | [目次](index.md) | [第13章 設定 →](13_settings.md)