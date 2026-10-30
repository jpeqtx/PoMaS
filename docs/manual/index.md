# PoMaS — Pocket Mathematics Station

**M5Stack Cardputer-Adv 用 多機能数学ステーション** / 操作マニュアル完全版

---

## 概要

PoMaS（ポマス、Pocket Mathematics Station）は、M5Stack Cardputer-Adv 上で動作する多機能関数電卓です。数式計算から CAS、統計解析、グラフ描画、科学資料検索、uLisp インタプリタまで、数学・科学の作業を 1 台に統合しています。

- 手のひらサイズ（約 85 × 55 × 15 mm）
- 物理キーボードによる高速入力
- 明るい Paper UI（目に優しい配色）
- オフラインで全機能が動作
- 全 24 章の操作マニュアル（本マニュアル）

---

## クイックスタート

**初めての方はまずこちら:**

- [第0章 クイックスタート](00_quickstart.md) — 5 分で使える最短ガイド
- [第21章 チートシート集](21_cheatsheet.md) — 全モード要点集約
- [第22章 FAQ（100 選）](22_faq.md) — よくある質問
- [第23章 シナリオ別インデックス](23_scenarios.md) — 目的から逆引き

---

## 全 24 章の目次

### 第 0 部 クイックスタート
- [第0章 クイックスタート](00_quickstart.md) — 5 分で使える最短ガイド

### 第 I 部 はじめに
- [第1章 はじめに](01_intro.md) — PoMaS の全体像、対象読者、学習順序
- [第2章 ハードウェア](02_hardware.md) — Cardputer-Adv の仕様、GPIO、電源
- [第3章 基本操作](03_basic.md) — 起動・終了、画面構成、メニュー操作

### 第 II 部 キーボード
- [第4章 キーボード詳細](04_keyboard.md) — 全キーアサイン、ショートカット、代替キー

### 第 III 部 Calculator
- [第5章 電卓 基本](05_calculator.md) — 四則演算、関数、変数、Ans バンク、個別リファレンス
- [第6章 電卓 応用](06_calculator_adv.md) — 複素数、基数変換、ビット演算、RPN、拡張ツール

### 第 IV 部 CAS
- [第7章 CAS](07_cas.md) — 34 コマンド完全リファレンス（微分・積分・因数分解・ODE）

### 第 V 部 Statistics
- [第8章 統計 基本](08_statistics.md) — 記述統計、回帰、検定の基礎と詳細
- [第9章 統計 応用](09_statistics_adv.md) — ノンパラ、リスク、多変量、生存、メタ、17 グラフ

### 第 VI 部 Graph
- [第10章 グラフ](10_graph.md) — 7 種プロット、G-Solve、3D、陰関数、パラメトリック

### 第 VII 部 SciRef
- [第11章 科学資料](11_sciref.md) — 定数 386、法則 300、単位 532、周期表 118

### 第 VIII 部 Tools
- [第12章 ツール](12_tools.md) — Spreadsheet、uLisp、Wi-Fi、Power/About

### 第 IX 部 設定
- [第13章 設定](13_settings.md) — NVS 永続化、角度・表示・自動電源 OFF

### 第 X 部 リファレンス
- [第14章 エラーメッセージ](14_errors.md) — 全エラー一覧、トラブル対処、復旧手順
- [第15章 用語集](15_glossary.md) — 数学記号、略語、アルゴリズム用語
- [第16章 実用例集](16_examples.md) — 数学・物理・化学・医学・工学・金融のレシピ
- [第17章 付録](17_appendix.md) — 完全キー表、索引、変更履歴、参考文献

### 第 XI 部 医学計算
- [第18章 医学計算](18_medical.md) — 10 領域 40 関数の完全リファレンス

### 第 XII 部 ビルドと開発
- [第19章 ビルドと書き込み](19_build.md) — Arduino IDE、PlatformIO、カスタム改変

### 第 XIII 部 uLisp リファレンス
- [第20章 uLisp 関数リファレンス](20_ulisp.md) — 全 80 関数、特殊形式、GC、実用例

### 第 XIV 部 クイックリファレンス
- [第21章 チートシート集](21_cheatsheet.md) — 全モードの要点を 1 ページに集約
- [第22章 FAQ（100 選）](22_faq.md) — よくある質問 10 カテゴリ 100 問
- [第23章 シナリオ別インデックス](23_scenarios.md) — 目的から逆引き + 上級テクニック

---

## 学習ロードマップ

### 初級（30 分）

1. [第0章 クイックスタート](00_quickstart.md)
2. [第3章 基本操作](03_basic.md)
3. [第4章 キーボード詳細](04_keyboard.md)

### 中級（2 時間）

4. [第5章 電卓 基本](05_calculator.md)
5. [第6章 電卓 応用](06_calculator_adv.md)
6. [第8章 統計 基本](08_statistics.md)
7. [第10章 グラフ](10_graph.md)

### 上級（10 時間）

8. [第7章 CAS](07_cas.md)
9. [第9章 統計 応用](09_statistics_adv.md)
10. [第11章 科学資料](11_sciref.md)
11. [第12章 ツール](12_tools.md)

### エキスパート（30 時間）

12. [第18章 医学計算](18_medical.md)
13. [第19章 ビルドと書き込み](19_build.md)
14. [第20章 uLisp 関数リファレンス](20_ulisp.md)

---

## 目的別インデックス

### やりたいことから探す

| 目的 | 章 |
|---|---|
| 電卓として使いたい | [第5章](05_calculator.md) |
| 微分・積分を計算したい | [第7章 CAS](07_cas.md) |
| 統計分析をしたい | [第8章](08_statistics.md) / [第9章](09_statistics_adv.md) |
| グラフを描きたい | [第10章](10_graph.md) |
| 科学定数を調べたい | [第11章](11_sciref.md) |
| 医学計算をしたい | [第18章](18_medical.md) |
| 表計算をしたい | [第12章](12_tools.md) |
| Lisp プログラミングをしたい | [第20章](20_ulisp.md) |
| ファームウェアをビルドしたい | [第19章](19_build.md) |
| エラーを解決したい | [第14章](14_errors.md) |

---

## PoMaS の主な機能

| モード | 機能 |
|---|---|
| **Calculator** | 四則演算・関数・履歴・メモリ・変数 A〜J・Ans バンク・複素数・基数変換・ビット演算・RPN・拡張ツール |
| **CAS** | 多項式の評価・微分・積分・加減乗算・根・テイラー展開・極限・記号置換・ODE |
| **Statistics** | 記述統計・回帰 9 種・仮説検定 11 種・ノンパラ・リスク指標・分布 7 種・多変量 5 種・層別・生存・メタ・統計グラフ 17 種・CSV ストリーム |
| **Medical Calculator** | 10 領域 40 関数（循環・呼吸・血液・スコア・体格・腎・電解質・内分泌・産科・PK） |
| **SciRef** | 科学定数 386 件・科学法則 300 件・単位換算 532 単位・周期表 118 元素 |
| **Graph** | y=f(x)・パラメトリック・極座標・陰関数・不等式・数列・3D・G-Solve |
| **Tools** | スプレッドシート 45×26・uLisp インタプリタ・Wi-Fi / Maxima・NVS 設定 |

---

## リンク

- [GitHub リポジトリ](https://github.com/jpeqtx/PoMaS)
- [最新版をダウンロード](https://github.com/jpeqtx/PoMaS/releases/latest)
- [Issues](https://github.com/jpeqtx/PoMaS/issues)
- [トップページに戻る](../index.html)

---

## ライセンス

MIT License

## 作者

YNGMT（GitHub: [@jpeqtx](https://github.com/jpeqtx)）

---

**PoMaS v0.92 — Paper UI v2**