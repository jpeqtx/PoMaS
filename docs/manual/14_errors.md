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