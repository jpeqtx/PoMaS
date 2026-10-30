# PoMaS — Pocket Mathematics Station

[![Version](https://img.shields.io/badge/version-0.92-2a5c3a)](https://github.com/jpeqtx/PoMaS/releases/latest)
[![Platform](https://img.shields.io/badge/platform-M5Stack%20Cardputer--Adv-1a5c9a)](https://m5stack.com/)
[![License](https://img.shields.io/badge/license-MIT-2a5c3a)](LICENSE)
[![Docs](https://img.shields.io/badge/manual-24%20chapters-7ab88a)](https://jpeqtx.github.io/PoMaS/)

M5Stack Cardputer-Adv 上で動作する多機能数学ステーション。
電卓・CAS・統計解析・医学計算・グラフ描画・科学資料検索・表計算・uLisp インタプリタを 1 ファームウェアに統合しました。

## スクリーンショット

```
┌─────────────────────────────────┐
│▌PoMaS 0.92          DEG|NRM     │
├─────────────────────────────────┤
│1 Calculator                     │
│2 CAS                            │
│3 Statistics                     │
│4 Medical Calculator             │
│5 Scientific Reference           │
│6 Graph                          │
│7 Tools (Sheet/uLisp/WiFi)       │
│8 About                          │
├─────────────────────────────────┤
│ENT:OK                ESC:back   │
└─────────────────────────────────┘
```

## 主な機能

### 1. Calculator（電卓）

- 四則演算・べき乗・階乗・剰余
- 三角関数・双曲線関数・指数・対数・累乗根・組合せ・順列・素数判定・乱数
- 履歴 100 件・メモリ M・変数 A〜J・Ans バンク（最大 20 件）
- 複素数（四則・極形式・累乗・exp/sin/cos/log）
- 基数変換（10/16/8/2 進数）・ビット演算（AND/OR/XOR/NOT/SHL/SHR）
- RPN モード
- 拡張ツール（方程式・数値解析・ベクトル/行列・幾何 27 図形・確率シミュレーション・素因数分解）

### 2. CAS（数式処理）

- 多項式の評価・微分・積分・加減乗算
- 根の計算（複素数対応、Durand-Kerner）
- テイラー展開（任意関数、次数 1〜7）
- 極限（数値的）
- 記号置換

### 3. Statistics（統計）

- 記述統計・特殊平均（重み付き/幾何/調和/トリム）
- 回帰 9 種・重回帰・ステップワイズ
- 検定 11 種（z/t/F/ANOVA/χ²/Fisher）
- ノンパラメトリック 7 種
- リスク指標（OR/RR/RD/NNT/CI）
- 確率分布 7 種（正規/t/χ²/F/二項/ポアソン/幾何）
- 多変量解析（PCA/因子分析/k-means/階層クラスタ/決定木）
- 層別解析（Mantel-Haenszel/Breslow-Day/Cochran's Q）
- 生存分析（KM/Log-rank/Cox PH）
- メタアナリシス（IV/DL/Forest/Funnel/Egger）
- 統計グラフ 17 種
- CSV ストリーム統計

### 4. Medical Calculator（医学計算）

10 領域・40 関数以上:

- **Cardiology**: MAP・Pulse Pressure・RPP・Shock Index
- **Pulmonology**: P/F ratio・A-a gradient・Oxygenation Index
- **Hematology**: ANC・MCV/MCH/MCHC・Mentzer index
- **Clinical Scores**: GCS・CURB-65・qSOFA・Child-Pugh・MELD
- **Body Composition**: BMI・BSA（Mosteller/DuBois）・IBW（Devine）・ABW
- **Renal**: CKD-EPI 2021・Cockcroft-Gault・Schwartz
- **Lytes/Acid-Base**: Corrected Ca/Na・Anion gap・Osmolality・Winter's formula・Delta gap・Free water deficit
- **Endocrine**: HOMA-IR・Free T4 index
- **Obstetrics**: Naegele（EDD）・Gestational age
- **Pharmacokinetics**: Half-life・Loading dose・Maintenance dose・Dose by weight

疫学メニュー: 生存分析・メタアナリシス・食中毒原因追跡・リスク/オッズ比・信頼区間。

### 5. SciRef（科学資料）

- 科学定数 386 件（11 カテゴリ）
- 科学法則 300 件（7 カテゴリ）
- 単位換算 532 単位（20 カテゴリ）
- 周期表 118 元素
- 記号/名前検索

### 6. Graph（グラフ）

- y=f(x)・パラメトリック・極座標・陰関数・不等式・数列・3D
- Trace（カーソル読み取り）
- Zoom（Z/X/R/B）・Square aspect (1:1)
- G-Solve（根・極値・交点・定積分）
- Tangent / Normal（接線・法線）
- BMP 保存

### 7. Tools（ツール）

- スプレッドシート 45×26（CSV I/O・Sheet↔L1..L8 変換）
- uLisp インタプリタ（GC 付き・REPL・スクリプト実行）
- Wi-Fi 接続・Maxima 連携（クライアント側）
- Power / About（バッテリー・SD 情報）
- Settings（NVS 永続化）
- Auto power off（分単位設定、NVS 保存）

## インストール

### 必要なもの

- M5Stack Cardputer-Adv（ESP32-S3FN8, 8MB Flash）
- USB-C ケーブル
- PC（書き込み用）
- microSD カード（任意）

### 方法 1: M5Stack Launcher（マルチブート対応・推奨）

1. [Launcher Releases](https://github.com/bmorcelli/Launcher/releases) から Launcher-Cardputer.bin をダウンロード
2. M5Burner または esptool で Launcher を書き込み
3. PoMaS の .bin を SD カードのルートにコピー
4. 電源 ON → Enter で Launcher 起動 → PoMaS を選択

### 方法 2: esptool で直接書き込み

```
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.bin
```

COM3 は環境に合わせて変更してください。

### 方法 3: 統合 .bin（オフセット 0x0 書き込み）

```
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.merged.bin
```

merged.bin は 8MB 全領域を含むため、初回書き込みや復旧時に便利。

## マニュアル

全 24 章の取扱説明書: [https://jpeqtx.github.io/PoMaS/](https://jpeqtx.github.io/PoMaS/)

| 部 | 章 |
|---|---|
| I はじめに | 1〜3 |
| II キーボード | 4 |
| III Calculator | 5〜6 |
| IV CAS | 7 |
| V Statistics | 8〜9 |
| VI Graph | 10 |
| VII SciRef | 11 |
| VIII Tools | 12 |
| IX 設定 | 13 |
| X リファレンス | 14〜17 |
| XI Medical | 18 |
| XII ビルド | 19 |
| XIII uLisp リファレンス | 20 |
| XIV クイックリファレンス | 21〜23 |

## ビルド（ソースから）

FQBN:

```
m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app
```

必要ライブラリ:

- M5Cardputer 1.1.1
- M5Unified 0.2.26
- M5GFX 0.2.33

詳細は [第19章 ビルドと書き込み](docs/manual/19_build.md) を参照。

## リリース情報

### v0.92（初公開）

- Paper UI v2（淡ミント紙 + グラファイト + ミントアクセント）
- バグ修正累計 47 件・デッドコード削除 9 件
- uLisp GC 安全性向上・eval/apply 深度ガード
- 医学計算 10 領域追加
- 統計 17 種グラフ・CSV ストリーム
- Auto power off（分単位設定）
- uLisp スタック 32KB 増強（v3.x 対応環境）

## ファイル構成

```
pomas_0_92/
├── pomas_0_92.ino          メインスケッチ
├── splash_data.h           スプラッシュ画像
├── README.md               このファイル
└── docs/                   GitHub Pages
    ├── index.html          トップページ
    ├── .nojekyll           Jekyll 無効化
    └── manual/             操作マニュアル
        ├── index.html      docsify
        ├── index.md        目次
        ├── _sidebar.md     サイドバー
        └── 00〜23_*.md     各章
```

## 動作環境

| 項目 | 値 |
|---|---|
| 機種 | M5Stack Cardputer-Adv (ESP32-S3FN8) |
| Flash | 8 MB |
| RAM | 512 KB (PSRAM なし) |
| 画面 | 240 × 135 px, RGB565 |
| Flash 使用率 | 約 56%（v0.92 ビルド時） |
| RAM 使用率 | 約 44%（グローバル変数） |
| 連続使用時間 | 約 4〜6 時間（使用状況による） |

## リンク

- [GitHub リポジトリ](https://github.com/jpeqtx/PoMaS)
- [最新版をダウンロード](https://github.com/jpeqtx/PoMaS/releases/latest)
- [Issues](https://github.com/jpeqtx/PoMaS/issues)
- [マニュアル（全 24 章）](https://jpeqtx.github.io/PoMaS/)

## ライセンス

MIT License

## 作者

YNGMT（GitHub: [@jpeqtx](https://github.com/jpeqtx)）