# PoMaS — Pocket Math Station

**M5Stack Cardputer-Adv 用 多機能数学ステーション**

---

## 概要

PoMaS（ポマス、Pocket Math Station）は、M5Stack Cardputer-Adv 上で動作する多機能関数電卓です。
数式計算から CAS、統計解析、グラフ描画、科学資料検索、μLisp インタプリタまで、
数学・科学の作業を 1 台に統合しています。

- 手のひらサイズ（約 85 × 55 × 15 mm）
- 物理キーボードによる高速入力
- 明るい Paper UI（目に優しい配色）
- オフラインで全機能が動作

---

## 主な機能

| モード | 機能 |
|---|---|
| **Calculator** | 四則演算・関数・履歴・メモリ・変数 A〜J・Ans バンク・複素数・基数変換・ビット演算・RPN・拡張ツール（方程式 / 数値解析 / 行列 / 幾何 / 確率 / 素因数） |
| **CAS** | 多項式の評価・微分・積分・加減乗算・根・テイラー展開・極限・記号置換 |
| **Statistics** | 記述統計・回帰 9 種・仮説検定 11 種・ノンパラメトリック・リスク指標・分布ブラウザ・多変量解析・層別解析・生存分析・メタアナリシス・統計グラフ 17 種・CSV ストリーム |
| **SciRef** | 科学定数 386 件・科学法則 300 件・単位換算 532 単位・周期表 118 元素 |
| **Graph** | y=f(x)・パラメトリック・極座標・陰関数・不等式・数列・3D・Trace・Zoom・G-Solve |
| **Tools** | スプレッドシート 45×26・μLisp インタプリタ・Wi-Fi / Maxima 連携（⚠️ サーバーは各自用意）・電源情報・設定（NVS 永続化） |

---

## インストール

### 必要なもの

- M5Stack Cardputer-Adv（ESP32-S3FN8, 8MB Flash）
- USB-C ケーブル
- PC（書き込み用）
- microSD カード（任意）

### 方法 1: M5Stack Launcher（マルチブート対応・推奨）

Launcher を使うと、PoMaS と他のファームウェアを SD カードで切り替えて使えます。

1. [Launcher Releases](https://github.com/bmorcelli/Launcher/releases) から `Launcher-Cardputer.bin` をダウンロード
2. M5Burner または esptool で Launcher を書き込み
3. PoMaS の `.bin` を SD カードのルートにコピー
4. 電源 ON → `Enter` で Launcher 起動 → PoMaS を選択

### 方法 2: esptool で直接書き込み

1. [esptool](https://github.com/espressif/esptool) をインストール
2. [Releases](https://github.com/jpeqtx/PoMaS/releases/latest) から `.bin` をダウンロード
3. Cardputer-Adv を USB 接続
4. 以下を実行：

esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.bin

`COM3` は環境に合わせて変更してください。

---

## リリース情報

### v0.92（初公開）

- **Paper UI v2** を適用（淡ミント紙背景 + グラファイト文字 + ミントアクセント）
- 包括的バグ修正（Critical 6 件、High 10 件、Medium / Low 40 件以上）
- μLisp GC の安全性向上（λ.env マーク、Env マーク&スイープ）
- 入力検証の強化（if / cond / let / define / lambda / 算術関数）
- グラフの陰関数描画修正、Square aspect オプション追加
- 統計の各種計算修正（tDist、qrEig、VIF、chiIndep 等）
- ESC カスケード防止、長文の自動切り詰め

---

## マニュアル

全 17 章の取扱説明書を用意しています。
サイドバー、または [第1章 はじめに](01_intro.md) から参照してください。

| 部 | 章 |
|---|---|
| 第 I 部 はじめに | 1〜3章 |
| 第 II 部 キーボード | 4章 |
| 第 III 部 Calculator | 5〜6章 |
| 第 IV 部 CAS | 7章 |
| 第 V 部 Statistics | 8〜9章 |
| 第 VI 部 Graph | 10章 |
| 第 VII 部 SciRef | 11章 |
| 第 VIII 部 Tools | 12章 |
| 第 IX 部 設定 | 13章 |
| 第 X 部 リファレンス | 14〜17章 |

---

## リンク

- [GitHub リポジトリ](https://github.com/jpeqtx/PoMaS)
- [最新版をダウンロード](https://github.com/jpeqtx/PoMaS/releases/latest)
- [Issues](https://github.com/jpeqtx/PoMaS/issues)

---

## ライセンス

MIT License

## 作者

YNGMT（GitHub: [@jpeqtx](https://github.com/jpeqtx)）