# 第14章 エラーメッセージ一覧

---

## この章で学ぶこと

- エラー解決の 5 ステップ
- Calculator の完全エラー一覧
- uLisp の完全エラー一覧
- Graph / Statistics / Distribution のエラー
- Medical Calculator / SD / Wi-Fi のエラー
- エラーログの見方（シリアルモニタ）
- カテゴリ別トラブルシューティング
- 復旧手順（軽度→中度→重度→最深部）
- エラー予防のベストプラクティス

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
**nil とエラーの区別**
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
**2.4GHz 帯のみ**
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
## 14.11 エラー解決の標準手順

### 14.11.1 5 ステップ

1. **エラーメッセージを記録**: 画面に表示された文字列を書き留める
2. **カテゴリ特定**: Calculator / uLisp / Graph / Statistics / SD / Wi-Fi のいずれか
3. **本章で検索**: 該当メッセージを探す
4. **症状別チェック**: 14.2〜14.8 の表を参照
5. **再現テスト**: 同じ操作を繰り返して再現性を確認

### 14.11.2 エラーログの記録

シリアルモニタ（115200 bps）で起動ログを確認できます。エラー発生時は以下を記録:

```
[起動ログ]
[エラー発生操作の手順]
[画面に表示されたメッセージ]
[再現性: 毎回 / 時々 / 1 回だけ]
```

### 14.11.3 エラー発生後の復旧優先度

| 優先度 | 状況 | 対処 |
|---|---|---|
| 高 | 画面フリーズ・再起動ループ | リセット・再書き込み |
| 中 | 特定機能が動かない | 機能別対処 |
| 低 | メッセージ表示のみ | メッセージに従う |

---

## 14.12 エラーメッセージ詳細（網羅）

### 14.12.1 Calculator の完全エラー一覧

| メッセージ | 原因 | 例 | 対処 |
|---|---|---|---|
| `syntax` | 構文エラー | `2+` | 式を完成 |
| `) expected` | 閉じ括弧不足 | `sin(30` | `)` 追加 |
| `] expected` | 配列の閉じ括弧不足 | `[1, 2` | `]` 追加 |
| `: expected` | 三項演算子の `:` 不足 | `1 ? 2` | `1 ? 2 : 3` |
| `trailing chars` | 式の後ろに余分な文字 | `2+3 abc` | 余分削除 |
| `unknown id` | 未知の変数・関数 | `xyz` | 変数名確認 |
| `unknown fn` | 未知の関数 | `foo(5)` | 関数名確認 |
| `unknown fn` | 配列関数の誤り | `mean[]` | `mean([1,2])` |
| `division by zero` | ゼロ除算 | `5/0` | 除数確認 |
| `mod by zero` | mod のゼロ除算 | `5 mod 0` | 除数確認 |
| `/0` | 除算の内部エラー | 多段の除算 | 途中結果確認 |
| `user fn needs 1 arg` | ユーザー関数の引数不一致 | `f(1,2)` | 1 引数に修正 |
| `user fn recursion` | ユーザー関数の再帰深度超過 | 無限再帰 | 再帰条件見直し |
| `array fn unsupported` | 配列関数の未対応 | `foo([1,2])` | sum/mean/min/max 等のみ |
| `syntax` (数値直後) | `0x` の後が不正 | `0xZZ` | 16進確認 |
| `NaN` | 計算結果が NaN | `sqrt(-1)` | 入力確認 |

### 14.12.2 uLisp の完全エラー一覧

| メッセージ | 原因 | 対処 |
|---|---|---|
| `unbound symbol` | 未定義変数・関数 | define で定義 |
| `not callable` | 関数でないものを呼出 | 引数種類確認 |
| `if: bad args` | if の引数不足 | (if cond t f) 形式 |
| `if: no then` | if の真式なし | 真式追加 |
| `define: bad args` | define の形式不正 | (define name value) |
| `define: no target` | define の対象なし | シンボル指定 |
| `define: no value` | define の値なし | 値追加 |
| `define: bad target` | 対象がシンボルでない | シンボル名 |
| `lambda: bad args` | lambda の形式不正 | (lambda (a) b) |
| `lambda: bad param` | lambda の引数がシンボルでない | シンボル |
| `let: bad args` | let の形式不正 | (let ((v v)) b) |
| `while: bad args` | while の形式不正 | (while c b) |
| `quote: no arg` | quote の引数なし | (quote x) |
| `parse too deep` | パーサ深度超過（>25） | ネスト削減 |
| `parse too deep (200)` | パーサ深度超過（>200） | ネスト削減 |
| `unexpected EOF` | 閉じ括弧不足 | `)` 追加 |
| `μLisp cell pool exhausted; run GC from menu` | セル枯渇 | Run GC now |
| `env table full` | 環境変数表満杯 | Reset interpreter |
| `Env limit` | Env プール上限（128） | Reset interpreter |
| `loop limit` | while のループ上限（100,000） | 条件見直し |
| `missing arg` | 組み込み関数の引数不足 | 引数追加 |
| `too many args` | 組み込み関数の引数過多 | 引数削減 |
| `sym malloc fail` | シンボル用メモリ失敗 | Reset interpreter |
| `str malloc fail` | 文字列用メモリ失敗 | Reset interpreter |
| `sqrt(neg)` | 負数の平方根 | abs 等で対応 |
| `eval depth` | eval 深度超過（400/100） | 再帰浅く |
| `apply depth` | apply 深度超過 | 同上 |
| `bad lambda param` | lambda のパラメータ不正 | シンボル指定 |
| `bad target` | define の対象不正 | シンボル |
| `array fn unsupported` | 配列関数未対応 | 個別に sum など |

### 14.12.3 Graph の完全エラー一覧

| メッセージ | 原因 | 対処 |
|---|---|---|
| `No active fn` | 関数未登録 | Edit functions |
| `No fn` | 関数未登録 | 同上 |
| `No sign change` | 根が見つからない | Range 拡大 |
| `No intersect` | 交点なし | 2 関数登録確認 |
| `Not found` | x at y の解なし | Range 拡大 |
| `Singular` | 行列特異 | 逆行列確認 |
| `Square only` | 正方行列でない | サイズ確認 |
| `Dim mismatch` | 次元不一致 | 行列サイズ |
| `Rank deficient` | QR 分解で列フルランクでない | 独立列確認 |
| `3D only` | 外積の次元不正 | 3D ベクトル使用 |
| `No eigvec` | 固有ベクトル計算失敗 | 特異行列？ |

### 14.12.4 Statistics の完全エラー一覧

| メッセージ | 原因 | 対処 |
|---|---|---|
| `Empty` | データなし | リスト入力 |
| `n<1` | データ件数不足 | 1 件以上 |
| `n<2` | データ件数不足 | 2 件以上 |
| `n<3` | データ件数不足 | 3 件以上 |
| `n<4` | データ件数不足 | 4 件以上 |
| `Mismatch` | リストサイズ不一致 | サイズ統一 |
| `Total = 0` | 合計ゼロ | データ確認 |
| `E > 0 required` | 期待度数 0 以下 | 期待値正に |
| `var > 0 required` | 分散 0 以下 | データ確認 |
| `Zero variance` | 分散ゼロ | データ分散確認 |
| `Zero variance (all diffs equal)` | 対応あり t 検定で全ての差が同一 | データ確認 |
| `malloc fail` | メモリ確保失敗 | データ削減 |
| `Singular` | 行列特異（重回帰） | 変数削減 |
| `Egger needs K>=3` | Egger 検定は K≥3 | 研究追加 |
| `RD = 0` | リスク差ゼロ（NNT計算不能） | データ確認 |
| `No candidates` | ステップワイズ候補なし | リスト準備 |
| `Spearman: zero variance` | 全値同一 | データ確認 |
| `Kendall: all ties` | 全組タイ | データ確認 |
| `PP: zero variance` | PP プロットで分散 0 | データ確認 |
| `Med-Med needs n>=3` | Med-Med は n≥3 | データ追加 |
| `dfE <= 0` | ANOVA の自由度不足 | 群数削減 |
| `SSE=0 (perfect fit)` | ANOVA で完全一致 | データ確認 |

### 14.12.5 Distribution の完全エラー一覧

| メッセージ | 原因 | 対処 |
|---|---|---|
| `0<p<1` | 二項分布の p 範囲外 | p を 0〜1 |
| `0<p<=1` | 幾何分布の p 範囲外 | p を 0〜1 |
| `df>=1` | t 分布・χ² 分布の df 範囲外 | df ≥ 1 |
| `d1,d2>=1` | F 分布の d1,d2 範囲外 | d ≥ 1 |
| `sd>0 required` | 正規分布の σ が 0 以下 | σ > 0 |
| `n>=0` | 二項分布の n が負 | n ≥ 0 |
| `lambda>0` | ポアソン分布の λ が 0 以下 | λ > 0 |

### 14.12.6 Medical Calculator のエラー

| メッセージ | 原因 |
|---|---|
| `E1-4 V1-5 M1-6` | GCS 入力範囲外 |
| `0/1 only` | CURB-65/qSOFA が 0/1 以外 |
| `1-3 each` | Child-Pugh が 1-3 以外 |
| `need 0<=x<=n` | CI proportion の x 範囲外 |
| `DBP must be <= SBP` | 血圧の大小逆転 |
| `Cox: beta diverged` | Cox 回帰の発散 |

### 14.12.7 SD カードのエラー

| メッセージ | 原因 | 対処 |
|---|---|---|
| `No SD` | SD 未挿入/未認識 | カード挿入 |
| `No SD card` | 同上 | 同上 |
| `File not found` | ファイルなし | パス確認 |
| `Open failed` | オープン失敗 | 権限確認 |
| `Not found` | パス不正 | パス確認 |
| `Write open FAILED` | 書き込み失敗 | 容量・フォーマット |
| `SD init failed` | 初期化失敗 | 挿し直し |
| `BMP save failed` | BMP 保存失敗 | SD 確認 |

### 14.12.8 Wi-Fi のエラー

| メッセージ | 原因 | 対処 |
|---|---|---|
| `No SSID set` | SSID 未設定 | Set SSID/Pass |
| `SSID not found` | 範囲外 or スペル | スキャン |
| `Auth failed` | 認証失敗 | パスワード確認 |
| `Timeout` | タイムアウト | 電波状況 |
| `Not connected` | 未接続 | Connect 実行 |
| `No networks` | ネットワークなし | スキャン再実行 |
| `HTTP X` | HTTP エラーコード X | X の内容で判断 |
| `Connect Failed` | 接続失敗 | 上記参照 |
| `Auth failed (pass?)` | パスワード疑い | パスワード再確認 |
| `2.4GHz only (no 5G)` | 5GHz 非対応 | 2.4GHz AP |

### 14.12.9 その他のエラー

| メッセージ | 原因 | 対処 |
|---|---|---|
| `Bank full` | Ans バンク満杯（20） | エントリ削除 |
| `Bank empty` | Ans バンク空 | エントリ追加 |
| `PSRAM alloc failed` | 統計用メモリ確保失敗 | 再起動 |
| `Invalid k` | k 値範囲外 | 範囲内指定 |
| `n too large` | データ件数超過 | データ削減 |
| `1..10` | 範囲外の値 | 範囲内で |
| `1..5` | 同上 | 同上 |
| `2..50` | 同上 | 同上 |
| `2..100` | 同上 | 同上 |
| `3..20` | 同上 | 同上 |

---

## 14.13 エラーログの見方

### 14.13.1 シリアルモニタ出力例

```
[SD] Init starting...
[SD] Try 1 at 4000000 Hz
[SD] OK at 4000000 Hz (Type: SDHC, Size: 16384MB)
PoMaS splash displayed
[NVS] loaded 10 keys
[WiFi] mode OFF
```

### 14.13.2 起動失敗のログ

```
[SD] Try 1 at 4000000 Hz
[SD] Failed at 4000000 Hz, retrying...
[SD] Try 2 at 10000000 Hz
[SD] Failed at 10000000 Hz, retrying...
[SD] Try 3 at 25000000 Hz
[SD] Failed at 25000000 Hz, retrying...
[SD] All attempts failed
  - Card not inserted or not seated
  - Card not FAT32 format (try reformat)
  - Card > 32GB (use 32GB or smaller)
  - Card defective
```

→ SD カードを確認

### 14.13.3 クラッシュ時のログ

```
Guru Meditation Error: Core 1 panic'ed (LoadProhibited)
Core 1 register dump:
PC : 0x42012345 PS : 0x00060d30 A0 : 0x82012345
Backtrace: 0x42012345:0x3fca0000 ...
```

→ スタックトレースから関数特定

---

## 14.14 カテゴリ別トラブルシューティング

### 14.14.1 起動しない

| 症状 | 原因 | 対処 |
|---|---|---|
| 電源ボタン押しても無反応 | バッテリー切れ | USB 給電 |
| スプラッシュ後ブラックアウト | Flash 破損 | 再書き込み |
| 起動ループ | クラッシュ | erash_flash + 再書き込み |
| SD 検出で停止 | SD 相性 | SD 抜きで起動テスト |

### 14.14.2 画面異常

| 症状 | 原因 | 対処 |
|---|---|---|
| 真っ暗 | バックライト OFF | 電源リセット |
| 乱れ | SPI 通信エラー | SD 抜き差し |
| 一部欠け | 描画バグ | 画面再描画（ESC 連打） |
| 色化け | カラーパレット不正 | ファームウェア再書き込み |

### 14.14.3 操作不能

| 症状 | 原因 | 対処 |
|---|---|---|
| キー無反応 | 減光中（60 秒無操作） | 任意キー押下で復帰 |
| ESC で戻れない | fn+ESC でない | ESC = `` ` `` のみ |
| fn キー効かない | fn 押し方誤り | 押しっぱなしで他キー |
| キーリピートがおかしい | 押しっぱなし | 一度離す |

### 14.14.4 機能不全

| 症状 | 原因 | 対処 |
|---|---|---|
| uLisp GC が走らない | 自動 GC なし | Run GC now |
| 統計計算が遅い | データ過多 | データ削減 |
| グラフが真っ黒 | zmin/zmax 外 | Range 調整 |
| CSV 読み込み失敗 | フォーマット誤り | カンマ区切り確認 |

---

## 14.15 復旧手順

### 14.15.1 軽度（設定リセットのみ）

```
[1] Tools → 5 → 5 (Reset all)
[2] 再起動
```

### 14.15.2 中度（NVS クリア）

```
[1] Tools → 5 → 5 (Reset all)
[2] Tools → 4 → R (SD 再初期化)
[3] 電源 OFF → ON
```

### 14.15.3 重度（Flash 再書き込み）

```
esptool.py --chip esp32s3 --port COM3 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.merged.bin
```

### 14.15.4 最深部（Flash 全消去）

```
esptool.py --chip esp32s3 --port COM3 erase_flash
esptool.py --chip esp32s3 --port COM3 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.merged.bin
```

---

## 14.16 エラー予防のベストプラクティス

### 14.16.1 入力時の注意

- 括弧の対応を常に確認
- 全角文字を使わない
- 関数名は小文字（sin, cos など）
- 変数は大文字 1 文字（A〜J）

### 14.16.2 データ管理

- 統計データは 200 件以内
- 文字列は 63 文字以内（ユーザー関数）
- 保存前に SD カード確認

### 14.16.3 定期的なメンテナンス

- 月 1 回: NVS バックアップ（Save settings now）
- 3 か月: SD カードフォーマット
- 6 か月: ファームウェア更新確認

### 14.16.4 予期せぬ停止への備え

- 重要な計算結果は Ans バンクに保存
- 長い入力は適度に Enter で区切る
- バッテリー残量 20% 以上を維持

---

## 14.17 エラー発生時の情報収集

サポートを受ける際の情報:

```
[機種] M5Stack Cardputer-Adv
[ファームウェア] PoMaS 0.92
[ビルド] FQBN: m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app
[SD] あり/なし、容量、フォーマット
[バッテリー] 残量 %
[症状] 発生した操作
[エラーメッセージ] 全文
[再現性] 毎回/時々/1回だけ
[起動ログ] シリアルモニタの出力
```

---

## 14.18 既知の制限（回避策あり）

### 14.18.1 現バージョンの制限

| 制限 | 回避策 |
|---|---|
| 5GHz Wi-Fi 非対応 | 2.4GHz AP を使用 |
| 統計リスト 200 件まで | データ分割 |
| uLisp セル 2500 個 | Run GC now 定期実行 |
| 陰関数描画がやや粗い | implCell 小に |
| Maxima サーバー未同梱 | セルフホスト |

### 14.18.2 修正予定

- 画面輝度設定（v0.93）
- スリープ時間設定（v0.93）
- 陰関数高解像度化（v0.94 検討）

---

## 14.19 エラー自動報告（未実装）

PoMaS はエラー自動報告機能を持ちません。クラッシュログは手動でシリアルモニタからコピーしてください。

将来的には Wi-Fi 経由でクラッシュレポートを送信する機能を検討中。

---

## 14.20 この章のまとめ

- エラー発生時は 5 ステップで解決: 記録 → カテゴリ → 検索 → 確認 → 再現テスト
- Calculator / uLisp / Graph / Statistics / SD / Wi-Fi の 6 カテゴリ
- 完全エラー一覧を 14.12 に網羅
- 復旧は段階的に: 設定リセット → NVS クリア → Flash 再書き込み → Flash 全消去
- 予防: 入力確認、データ管理、定期メンテナンス
- サポート情報は 14.17 の形式で収集

### 確認問題

**Q14-1**: `division by zero` が出たら？
<details><summary>答え</summary>
除数を確認。0 で割っていないかチェック。
</details>
**Q14-2**: `unbound symbol` が出たら？
<details><summary>答え</summary>
変数・関数が未定義。先に define で定義する。
</details>
**Q14-3**: `μLisp cell pool exhausted` が出たら？
<details><summary>答え</summary>
Tools → uLisp → Run GC now、または Reset interpreter。
</details>
**Q14-4**: `No SD` が出たら？
<details><summary>答え</summary>
SD カードを挿入して、Tools → Power → R でリトライ。
</details>


Q14-5: `division by zero` が出たら？ → 除数確認、0 で割っていないか

Q14-6: `unbound symbol` が出たら？ → define で先に定義

Q14-7: `μLisp cell pool exhausted` が出たら？ → Run GC now または Reset interpreter

Q14-8: `No SD` が出たら？ → SD 挿入 → Tools → 4 → R でリトライ

Q14-9: `Auth failed` が出たら？ → Wi-Fi パスワード確認

Q14-10: `n<2` が出たら？ → データ 2 件以上必要

Q14-11: 復旧の最終手段は？ → erase_flash → merged.bin 書き込み

Q14-12: サポート情報に含めるべきは？ → 機種・FW 版・FQBN・症状・ログ・再現性

---

## 14.21 エラーメッセージ完全データベース（拡張）

本節は PoMaS ソースに含まれる全エラーメッセージを網羅した完全リファレンスです。

### 14.21.1 ExprEval（式評価エンジン）のエラー

| エラー | 発生箇所 | 詳細条件 |
|---|---|---|
| `syntax` | parsePrimary | 予期しない文字（演算子・記号・閉じ括弧の連続など） |
| `) expected` | parsePrimary / parseExpr | 開き括弧に対応する閉じ括弧がない |
| `] expected` | parseIdent（配列関数） | 配列リテラルの閉じ括弧がない |
| `: expected` | parseExpr（三項演算子） | `?` に対応する `:` がない |
| `trailing chars` | eval | 式の評価後に余分な文字がある |
| `unknown id` | parseIdent | 未定義の識別子 |
| `unknown fn` | callFn | 未知の関数呼び出し |
| `array fn unsupported` | parseIdent | 配列非対応の関数に配列を渡した |
| `user fn needs 1 arg` | parseIdent | ユーザー関数の引数数が 1 でない |
| `user fn recursion` | parseIdent | ユーザー関数の再帰深度超過（32 段） |
| `/0` | parseTerm | 除算の除数がゼロ |
| `NaN` | 内部計算 | 計算結果が非数（sqrt(-1)、log(-1) など） |

### 14.21.2 Dist（確率分布）のエラー

| エラー | 条件 |
|---|---|
| `sd>0 required` | 正規分布 σ ≤ 0 |
| `lambda>0` | ポアソン分布 λ ≤ 0 |
| `df>=1` | t 分布・χ² 分布の自由度 < 1 |
| `d1,d2>=1` | F 分布の自由度 < 1 |
| `n>=0` | 二項分布の試行回数 < 0 |
| `0<p<1` | 二項分布 p が 0 以下 or 1 以上 |
| `0<p<=1` | 幾何分布 p が 0 以下 or 1 超 |

### 14.21.3 NumAnalysis のエラー

| エラー | 条件 |
|---|---|
| `No conv` | Newton-Raphson の収束失敗 |
| `No sign change` | 二分法で f(a)·f(b) ≥ 0 |
| `Singular` | 行列が特異（det=0） |
| `Rank deficient` | QR 分解で列フルランクでない |
| `No eigvec` | 固有ベクトル計算失敗 |
| `3D only` | 3D ベクトル以外で外積を計算 |

### 14.21.4 StatCore / StatTests のエラー

| エラー | 条件 |
|---|---|
| `Empty` | リストが空 |
| `n<1` | データ件数 < 1 |
| `n<2` | データ件数 < 2 |
| `n<3` | データ件数 < 3 |
| `n<4` | データ件数 < 4 |
| `Mismatch` | 2 リストのサイズ不一致 |
| `Total = 0` | χ² 検定で合計ゼロ |
| `E > 0 required` | 期待度数 ≤ 0 |
| `var > 0 required` | 分散 ≤ 0 |
| `Zero variance` | 分散ゼロ（検定不能） |
| `Zero variance (all diffs equal)` | 対応あり t 検定で全差同一 |
| `malloc fail` | メモリ確保失敗 |
| `Singular` | 行列特異（重回帰） |
| `Egger needs K>=3` | Egger 検定の K < 3 |
| `RD = 0` | NNT 計算で RD=0 |
| `No candidates` | ステップワイズ候補なし |
| `Spearman: zero variance` | Spearman で分散ゼロ |
| `Kendall: all ties` | Kendall で全組タイ |
| `PP: zero variance` | PP プロットで分散ゼロ |
| `Med-Med needs n>=3` | Med-Med で n < 3 |
| `dfE <= 0` | ANOVA の誤差自由度 ≤ 0 |
| `SSE=0 (perfect fit)` | ANOVA で SSE=0 |

### 14.21.5 Graph のエラー

| エラー | 条件 |
|---|---|
| `No active fn` | 有効な関数がない |
| `No fn` | 関数未登録 |
| `No intersect` | 2 関数の交点なし |
| `Not found` | x at y の解なし |
| `Dim mismatch` | ベクトル・行列のサイズ不一致 |
| `Square only` | 正方行列が必要な操作で非正方 |
| `No SD` | SD カード未挿入 |

### 14.21.6 Medical Calculator のエラー

| エラー | 条件 |
|---|---|
| `E1-4 V1-5 M1-6` | GCS の入力範囲外 |
| `0/1 only` | CURB-65/qSOFA で 0/1 以外 |
| `1-3 each` | Child-Pugh で 1-3 以外 |
| `need 0<=x<=n` | CI proportion で x の範囲外 |
| `DBP must be <= SBP` | 血圧の大小逆転 |
| `Cox: beta diverged` | Cox 回帰の β 発散 |
| `n too small for levels` | 2-way ANOVA で群数不足 |

### 14.21.7 Tools / Spreadsheet のエラー

| エラー | 条件 |
|---|---|
| `alloc fail` | メモリ確保失敗 |
| `Dim mismatch` | 範囲指定の次元不一致 |
| `Bad row` | CSV 読み込みで行形式不正 |
| `Bad objective` | 線形計画法の目的関数不正 |

### 14.21.8 SD カードのエラー

| エラー | 条件 |
|---|---|
| `No SD` | SD 未挿入 |
| `No SD card` | SD 未認識 |
| `File not found` | ファイルなし |
| `Open failed` | オープン失敗 |
| `Not found` | パス不正 |
| `Write open FAILED` | 書き込み失敗 |
| `SD init failed` | 初期化失敗 |
| `BMP save failed` | BMP 保存失敗 |

### 14.21.9 Wi-Fi / Maxima のエラー

| エラー | 条件 |
|---|---|
| `No SSID set` | SSID 未設定 |
| `SSID not found` | SSID が範囲外 |
| `Auth failed` | 認証失敗 |
| `Timeout` | 接続タイムアウト |
| `Not connected` | 未接続 |
| `No networks` | ネットワークなし |
| `HTTP X` | HTTP ステータスコード X |
| `2.4GHz only (no 5G)` | 5GHz 帯 |

---

## 14.22 症状別フローチャート

### 14.22.1 起動しない場合

```
電源ボタン押下 → 反応なし
├─ USB 給電でも反応なし → 電源回路故障（要修理）
├─ USB 給電で起動する → バッテリー切れ
│   ├─ 1.5-2h 充電後に起動する → 正常
│   └─ 充電しても起動しない → バッテリー故障
└─ スプラッシュ表示 → メインメニュー
    ├─ メニュー表示 → 正常
    └─ ブラックアウト → Flash 破損（再書き込み）
```

### 14.22.2 計算がおかしい場合

```
計算結果が想定と違う
├─ 1+1=3 のような基本エラー
│   └─ Flash 破損の可能性 → 再書き込み
├─ sin(30)≠0.5
│   └─ 角度モード確認（DEG/RAD/GRA）→ Fn+1
├─ 10/0 でエラー
│   └─ division by zero（正常動作）
├─ 大きな数の桁落ち
│   └─ 倍精度の限界（正常動作）
└─ 特定の関数で失敗
    └─ 第14.21 で詳細確認
```

### 14.22.3 SD カードが認識されない場合

```
SD カード認識失敗
├─ Tools → 4 → R でリトライ
│   ├─ 成功 → 一時的な問題
│   └─ 失敗 → 次へ
├─ カードを抜き差し
│   ├─ 成功 → 接触不良
│   └─ 失敗 → 次へ
├─ 別の SD カードで試す
│   ├─ 成功 → カード相性
│   └─ 失敗 → 次へ
├─ PC で FAT32 フォーマット
│   ├─ 成功 → フォーマット問題
│   └─ 失敗 → SD スロット故障
└─ SD スロット故障（要修理）
```

### 14.22.4 Wi-Fi に繋がらない場合

```
Wi-Fi 接続失敗
├─ SSID not found
│   ├─ 2.4GHz AP か確認 → 5GHz は不可
│   ├─ SSID のスペル確認
│   └─ 範囲内か確認
├─ Auth failed
│   ├─ パスワード再確認
│   ├─ WPA2 対応か確認 → WPA3 不可
│   └─ 特殊文字の入力ミス確認
├─ Timeout
│   ├─ AP を再起動
│   └─ 別の AP で試す
└─ 接続しても通信失敗
    └─ IP アドレス取得確認（Status で）
```

### 14.22.5 uLisp で無限ループした場合

```
uLisp が反応しない
├─ ESC 押下
│   ├─ 復帰 → ループを中断
│   └─ 反応なし → 次へ
├─ ESC 連打（3 回）
│   ├─ 復帰 → ループ中断
│   └─ 反応なし → 次へ
├─ 電源ボタン長押し（強制終了）
└─ Reset interpreter で初期化
```

---

## 14.23 復旧判断マトリックス

| 症状の重さ | 復旧手順 | 想定原因 |
|---|---|---|
| 軽微（通知エラー） | メッセージに従う | 入力ミス |
| 中度（特定機能不全） | 該当機能の再起動 | 一時的な不整合 |
| 重度（複数機能不全） | Reset all | NVS 破損 |
| 深刻（起動失敗） | 電源 OFF → ON | 一時的なフリーズ |
| 危機（起動ループ） | erase_flash + 再書き込み | Flash 破損 |
| 絶望（電源入らない） | USB 充電 | バッテリー切れ |

---

## 14.24 エラー予防のチェックリスト

### 毎回の操作前

- [ ] ヘッダのバッジ（角度モード・表示形式）を確認
- [ ] 入力式の括弧の対応を確認
- [ ] 関数名のスペルを確認（sin, cos など）

### 定期的なメンテナンス

- [ ] 月 1 回: Save settings now で NVS バックアップ
- [ ] 3 か月: SD カードの不要ファイル削除
- [ ] 半年: ファームウェア更新確認

### データ管理

- [ ] 重要な計算結果は Ans バンクに保存
- [ ] 統計データは CSV で SD に保存
- [ ] ユーザー関数は SD にエクスポート

---

## 14.25 サポート依頼時のテンプレート

```
【機種】M5Stack Cardputer-Adv
【ファームウェア】PoMaS 0.92
【ビルド】FQBN: m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app
【SD】あり / なし、容量: XXX GB、フォーマット: FAT32 / exFAT
【バッテリー】XX%
【発生操作】
  1. メインメニュー → 3 (Statistics) → 1
  2. L1 に [1,2,3,4,5] を入力
  3. Descriptive を実行
【エラーメッセージ（全文）】
  ERR: Zero variance
【再現性】
  毎回 / 時々 / 1 回だけ
【起動ログ（シリアルモニタ）】
  [SD] Init starting...
  [SD] OK at 4000000 Hz
  ...
【スクリーンショット（あれば）】
  添付
```

---

## 14.26 この章の追加まとめ

- 全エラーメッセージを 9 分類で網羅（14.21）
- 症状別フローチャート 5 種（14.22）
- 復旧判断マトリックス（14.23）
- エラー予防チェックリスト（14.24）
- サポート依頼テンプレート（14.25）

### 確認問題（追加）

Q14-13: `syntax` エラーの主な原因は？ → 予期しない文字（演算子の連続、閉じ括弧の欠落など）

Q14-14: `No sign change` は何を意味する？ → 二分法で区間 [a,b] の両端が同符号

Q14-15: `Rank deficient` の対処は？ → 線形独立な列を確認、変数を減らす

Q14-16: 起動ループに陥ったときの最終手段は？ → erase_flash + merged.bin 再書き込み

Q14-17: サポート依頼に含めるべき情報は？ → 機種・FW 版・FQBN・症状・エラー全文・ログ・再現性

---

[← 第13章 設定](13_settings.md) | [目次](#/) | [第15章 用語集 →](15_glossary.md)
