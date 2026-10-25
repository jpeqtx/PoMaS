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