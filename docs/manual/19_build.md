# 第19章 ビルドと書き込み


---
## この章で学ぶこと

- ビルド環境（Arduino IDE / PlatformIO / arduino-cli）
- ボードパッケージのインストール（M5Stack 3.3.10）
- 必要ライブラリ 3 点（M5Cardputer / M5Unified / M5GFX）
- FQBN とツールメニュー設定
- ビルド実行と生成物
- 書き込みモード（G0 + リセット）
- ソース構造（Part 1〜20）
- Arduino ctags プロトタイプ問題と対策
- サイズとメモリ（Flash 56%、RAM 44%）
- トラブルシューティング
- カスタム改変ガイド
- CI/CD（GitHub Actions）

---

## 19.1 概要

この章では、PoMaS をソースからビルドして実機に書き込む手順を解説します。

| 対象読者 | 推奨する章 |
|---|---|
| 一般ユーザー | 19.2〜19.4（プリビルド版の書き込み） |
| 開発者 | 19.1〜19.12（全工程） |
| 改変したい方 | 19.1〜19.12（全工程） |

---

## 19.2 必要な環境

### ハードウェア

| 項目 | 要件 |
|---|---|
| 機種 | M5Stack Cardputer-Adv（Cardputer 旧版は非対応） |
| Flash | 8 MB（4 MB 版は不可） |
| USB | USB-C ケーブル（データ通信対応） |
| PC | Windows / macOS / Linux |

### ソフトウェア

- **方法 1**: Arduino IDE 2.x 系
- **方法 2**: PlatformIO（VS Code）
- **方法 3**: arduino-cli（コマンドライン）

---

## 19.3 ボードパッケージのインストール

### Arduino IDE の場合

**Step 1: 環境設定**

1. Arduino IDE → ファイル → 環境設定
2. 「追加のボードマネージャ URL」に追加:

```
https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
```

**Step 2: ボードマネージャ**

1. ツール → ボード → ボードマネージャ
2. 検索: m5stack
3. **M5Stack by M5Stack** を選択
4. バージョン **3.3.10** をインストール（PoMaS 0.92 検証済み）

**バージョン指定**: PoMaS 0.92 は **M5Stack 3.3.10** で検証されています。3.2.x 系では `SET_LOOP_TASK_STACK_SIZE` マクロの挙動が異なり、uLisp スタック拡張が効かない可能性があります。

### PlatformIO の場合

```
platformio.ini
─────────────────────────────────
[env:m5stack-cardputer]
platform = espressif32
board = m5stack-stamps3
framework = arduino
board_build.flash_mode = qio
board_build.f_flash = 80000000L
board_build.partitions = huge_app.csv
monitor_speed = 115200
lib_deps =
    m5stack/M5Cardputer@1.1.1
    m5stack/M5Unified@0.2.26
    m5stack/M5GFX@0.2.33
build_flags =
    -DCORE_DEBUG_LEVEL=0
```

---

## 19.4 ライブラリのインストール

### 必要ライブラリ（3 点）

| ライブラリ | バージョン | 入手元 |
|---|---|---|
| M5Cardputer | 1.1.1 | Library Manager |
| M5Unified | 0.2.26 | Library Manager |
| M5GFX | 0.2.33 | Library Manager |

### インストール手順

1. ツール → ライブラリを管理
2. 検索: M5Cardputer → 1.1.1 をインストール
3. 検索: M5Unified → 0.2.26 をインストール
4. 検索: M5GFX → 0.2.33 をインストール

**依存**: SD/FS/SPI/Preferences/WiFi/HTTPClient/NetworkClientSecure は ESP32 コア同梱。

**WiFiEspAT 警告**: `WiFiEspAT.disabled_*` フォルダがあると `invalid library: no header files found` 警告が出ますが、ビルドには無害です。気になる場合は該当フォルダを `libraries` 外へ移動してください。

---

## 19.5 FQBN の設定

```
m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app
```

### Arduino IDE ツールメニュー

| 項目 | 値 |
|---|---|
| ボード | M5Stack Cardputer |
| Upload Speed | 921600 |
| USB Mode | Hardware CDC and JTAG |
| CDC On Boot | Enabled |
| CPU Frequency | 240MHz |
| Flash Mode | QIO 80MHz |
| Flash Size | 8MB (64Mb) |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| PSRAM | Disabled |
| Loop Core | 1 |
| Events Core | 1 |
| Erase All Flash | Disabled |

### arduino-cli

```
arduino-cli compile --fqbn "m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app" pomas_0_92
```

---

## 19.6 ビルドの実行

### Arduino IDE

1. `pomas_0_92.ino` を開く
2. **検証 (Ctrl+R)**: コンパイルのみ
3. **書き込み (Ctrl+U)**: コンパイル + 書き込み

**成功時の末尾ログ**:

```
最大3145728バイトのフラッシュメモリのうち、スケッチが1776859バイト（56%）を使っています。
最大327680バイトのRAMのうち、グローバル変数が146212バイト（44%）を使っていて、
ローカル変数で181468バイト使うことができます。
```

### PlatformIO

```
pio run -e m5stack-cardputer -t upload
```

### 書き込みモード（ダウンロードモード）

`Connecting...` で止まる場合:

1. **G0 ボタン**（リセット横の小ボタン）を押し続ける
2. **リセットボタン**を押す
3. G0 を離す
4. ダウンロードモードに入るので再書き込み

---

## 19.7 生成物

ビルド後に生成されるファイル:

| ファイル | 書き込み先 |
|---|---|
| `pomas_0_92.ino.bin` | 0x10000 |
| `pomas_0_92.ino.bootloader.bin` | 0x0 |
| `pomas_0_92.ino.partitions.bin` | 0x8000 |
| `pomas_0_92.ino.merged.bin` | 0x0（8MB 一括） |
| `boot_app0.bin` | 0xe000 |

### 個別書き込み（esptool）

```
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash ^
  0x0     pomas_0_92.ino.bootloader.bin ^
  0x8000  pomas_0_92.ino.partitions.bin ^
  0xe000  boot_app0.bin ^
  0x10000 pomas_0_92.ino.bin
```

### 一括書き込み（merged.bin）

```
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash ^
  0x0 pomas_0_92.ino.merged.bin
```

---

## 19.8 ソース構造とビルド時の注意点

### Part 1〜20 構成

`pomas_0_92.ino` は 1 ファイルですが、コメントで 20 Part に分割されています:

| Part | 内容 |
|---|---|
| 1 | 共通ヘッダ・型・GState・補助関数 |
| 2 | DisplayMgr |
| 3 | ExprEval |
| 4 | NumAnalysis / Mat / Dist |
| 5 | 科学定数 DB |
| 6 | 単位換算 DB |
| 7〜8 | 科学法則辞書 |
| 9 | SciRef |
| 10 | CalculatorCore / CasCommands |
| 11 | 方程式・数値解析・行列・幾何 |
| 12 | 確率・素因数・拡張ツール |
| 13 | StatCore |
| 14 | StatRegress / StatTests |
| 15 | ノンパラ・リスク・分布ブラウザ |
| 16 | 多変量解析 |
| 17 | 層別・生存・メタ・統計グラフ |
| 18 | uLisp |
| 19 | グラフエンジン・スプレッドシート |
| 20 | setup / loop / メインメニュー |

### Arduino ctags プロトタイプ生成問題

Arduino IDE は `.ino` 関数プロトタイプを **ctags で自動生成**し、ファイル先頭に挿入します。これが型定義より前に関数プロトタイプを置く原因になります。

**PoMaS での対策（実装済み）**

1. **前方宣言の挿入**

```
#include "splash_data.h"
// --- Forward decls for ctags-generated prototypes ---
enum BaseMode : int;
struct LawEntry;
```

2. **`static inline` 修飾**

```
static inline void formatInt(long long v, char* out, int n, BaseMode b) { ... }
static inline const LawEntry* lawAt(int idx) { ... }
```

3. **enum 基底型の統一**

```
enum BaseMode : int { BASE_DEC=10, BASE_HEX=16, BASE_OCT=8, BASE_BIN=2 };
```

前方宣言と定義の基底型が一致していないと `underlying type mismatch` エラーになります。

**新規関数追加時の注意**

1. 型定義は使用関数より前に置く
2. または前方宣言をファイル先頭に追加
3. `static inline` の使用を検討

---

## 19.9 サイズとメモリ

### v0.92 ビルド実績

| 項目 | 使用量 | 上限 | 使用率 |
|---|---|---|---|
| Flash | 1,776,859 B | 3,145,728 B | 56% |
| RAM（グローバル） | 146,212 B | 327,680 B | 44% |
| RAM（残り） | 181,468 B | — | ローカル用 |

### メモリ削減のヒント

- 大きなテーブルは `PROGMEM` に置く
- `String` は短命に、長寿命は `char[]` にコピー
- グローバル配列は `static` 化
- 一時バッファは必要十分サイズに

### PSRAM 非搭載の制約

Cardputer-Adv は PSRAM 非搭載のため、`ps_malloc` は `malloc` にフォールバック。統計リストは各 200 件が上限。

---

## 19.10 トラブルシューティング

### コンパイルエラー

| エラー | 原因 | 対処 |
|---|---|---|
| `'BaseMode' has not been declared` | ctags プロトタイプ | 前方宣言を追加 |
| `'LawEntry' does not name a type` | 同上 | 同上 |
| `underlying type mismatch in enum` | 基底型不一致 | 両方に `: int` |
| `'mu' was not declared` | 変数名誤り | `mu0` 等に修正 |
| `inconsistent types 'float' and 'double'` | 型混在 | `(double)NAN` 等キャスト |
| `expected ';' before ...` | セミコロン漏れ | 該当行前後を確認 |
| `region 'iram0_0_seg' overflowed` | Flash/RAM 超過 | パーティション・最適化調整 |

### 書き込みエラー

| エラー | 対処 |
|---|---|
| `Failed to connect` | G0 + リセットでダウンロードモード |
| `Failed to write to target Flash` | USB ケーブル・ポート変更 |
| `Wrong offset or size` | Erase All Flash 後に再書き込み |
| `MD5 of file does not match` | 再書き込み |
| `Permission denied: COM3` | 他アプリを閉じる |

### 起動時トラブル

| 症状 | 対処 |
|---|---|
| ブラックアウト | 再書き込み |
| SD: NOT detected | Tools → Power → R でリトライ |
| Wi-Fi 接続しない | 2.4GHz 帯であることを確認 |
| 動作が重い | 再起動、データ削減 |
| uLisp `cell pool exhausted` | Run GC now |

### Flash 全消去（復旧）

```
esptool.py --chip esp32s3 --port COM3 erase_flash
```

その後に `merged.bin` を 0x0 に書き込む。

---

## 19.11 参考リンク

- [M5Stack 公式](https://m5stack.com/)
- [M5Cardputer ライブラリ](https://github.com/m5stack/M5Cardputer)
- [M5Unified](https://github.com/m5stack/M5Unified)
- [M5GFX](https://github.com/m5stack/M5GFX)
- [esptool](https://github.com/espressif/esptool)
- [Launcher (bmorcelli)](https://github.com/bmorcelli/Launcher)

---

## 19.12 この章のまとめ

- FQBN: `m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app`
- ライブラリは 3 点（M5Cardputer / M5Unified / M5GFX）
- ctags プロトタイプ問題は前方宣言 + static inline で解決
- サイズは Flash 56% / RAM 44%（v0.92）
- 書き込み時止まったら G0 + リセット
- merged.bin で 0x0 一括書き込みが復旧に便利

### 確認問題

Q19-1: FQBN の文字列は？ → `m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app`

Q19-2: 必要ライブラリ 3 つは？ → M5Cardputer 1.1.1 / M5Unified 0.2.26 / M5GFX 0.2.33

Q19-3: `Connecting...` で止まった時の対処は？ → G0 + リセット → 再書き込み

Q19-4: merged.bin の用途は？ → 8MB 全領域一括書き込み（復旧時便利）

Q19-5: `'BaseMode' has not been declared` の原因は？ → ctags プロトタイプ生成 → 前方宣言追加

---

## 19.13 PlatformIO 詳細設定

### プロジェクト構成

```
pomas_0_92/
├── pomas_0_92.ino
├── splash_data.h
├── platformio.ini      ← 新規作成
└── docs/               （GitHub Pages、ビルド対象外）
```

### platformio.ini 完全版

```
[env:m5stack-cardputer]
platform = espressif32
board = m5stack-stamps3
framework = arduino
board_build.filesystem = spiffs
board_build.flash_mode = qio
board_build.f_flash = 80000000L
board_build.partitions = huge_app.csv
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
upload_speed = 921600
lib_deps =
    m5stack/M5Cardputer@1.1.1
    m5stack/M5Unified@0.2.26
    m5stack/M5GFX@0.2.33
build_flags =
    -DCORE_DEBUG_LEVEL=0
    -DBOARD_HAS_PSRAM=0
    -O2
build_unflags =
    -Os
```

### ビルドとアップロード

```
; コンパイルのみ
pio run -e m5stack-cardputer

; コンパイル + 書き込み
pio run -e m5stack-cardputer -t upload

; シリアルモニタ
pio device monitor -b 115200

; クリーンビルド
pio run -e m5stack-cardputer -t clean
```

### よくある問題

| 症状 | 対処 |
|---|---|
| `board not found: m5stack-stamps3` | `platformio platform update` |
| M5Stack 3.3.10 と異なるバージョン | `platform_packages` で明示指定 |
| Flash 不足エラー | `-Os` を `-O2` に変更、またはコード削減 |
| リンクエラー | `build_unflags = -Os` を追加 |

---

## 19.14 カスタム改変ガイド

### 新しい関数を Calculator に追加

**手順 1: `ExprEval::callFn` に追加**

```
// Part 3 内、na == 1 の分岐に追加
if (!strcasecmp(nm,"myFunc")) return myFunction(a[0]);
```

**手順 2: 関数本体を定義**

```
static double myFunction(double x) {
    return x * 2.0 + 1.0;
}
```

**手順 3: ビルド・書き込み**

新しい関数は電卓で `myFunc(5)` のように呼び出せます。

### 新しい科学定数を追加

**手順 1: `constDB[]` に追加（Part 5 内）**

```
{"My custom constant", "MC", 1.23456789e10, "units", "Physical"},
```

**手順 2: カテゴリを追加する場合**

```
// CONST_CATS[] に追加
"Physical", "Math", "EE", "Astronomy", "Planet", "DwarfMoon",
"Geo", "Atmos", "Ocean", "Element", "Biology",
"MyCategory",  // ← 追加
```

### 新しい単位を追加

**手順 1: `unitDB[]` に追加（Part 6 内）**

```
{"my unit", "mu", 0.0254, 0, UC_LENGTH},
```

`factor` は SI 基本単位への換算係数、`offset` は温度のオフセット（通常 0）。

### 新しい統計検定を追加

**手順 1: `StatTests::run` のメニューに追加**

```
const char* items[] = {
    "z-test (1-sample)",
    // ...
    "My new test",  // ← 追加
    "Back"
};
int s = DM.menu("Tests", items, 12);  // 12 → 13 に増やす
```

**手順 2: `switch` に case を追加**

```
case 11: myNewTest(); break;
case 12: return;
```

**手順 3: 関数本体を定義**

```
void myNewTest() {
    // 入力 → 計算 → showTest(...)
}
```

### 新しい医学計算を追加

**手順 1: メニュー項目追加**

`MedicalCardio::run` などの items[] に追加。

**手順 2: ask() で入力を取得**

```
double v = ask("Value:", "0");
if (isnan(v) || v <= 0) return;
```

**手順 3: show() で結果表示**

```
show("My Score", "Value", v, "interpretation text");
```

### 新しい uLisp 組み込み関数を追加

**手順 1: bMyFunc を定義（Part 18 内）**

```
CellRef bMyFunc(CellRef a, Env* e) {
    if (isNil(a) || a->type != T_CONS) { setErr("missing arg"); return nilC; }
    double x = toD(a->cons.car);
    return mkNum(x * x);
}
```

**手順 2: initEnv に登録**

```
globalEnv.set("myFunc", mkBuiltin(bMyFunc));
```

**手順 3: ビルド後、REPL で使用**

```
(myFunc 5)   ; → 25
```

### グラフのプロットタイプを追加

**手順 1: `GraphEngine::pickType` のメニューに追加**

**手順 2: `GraphEngine::plot` の switch に case 追加**

**手順 3: プロット関数を実装**（`plotYofX` などを参考）

```
void plotMyType(int f, uint16_t col) {
    // DisplayMgr::Display への描画
}
```

---

## 19.15 CI/CD（GitHub Actions）

### 自動ビルド設定例

`.github/workflows/build.yml`:

```
name: Build PoMaS
on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: arduino/setup-arduino-cli@v1
      - name: Install M5Stack core
        run: |
          arduino-cli config init
          arduino-cli config add board_manager.additional_urls https://static-cdn.m5stack.com/resource/arduino/package_m5stack_index.json
          arduino-cli core update-index
          arduino-cli core install m5stack:esp32@3.3.10
      - name: Install libraries
        run: |
          arduino-cli lib install "M5Cardputer@1.1.1"
          arduino-cli lib install "M5Unified@0.2.26"
          arduino-cli lib install "M5GFX@0.2.33"
      - name: Compile
        run: |
          arduino-cli compile --fqbn "m5stack:esp32:m5stack_cardputer:FlashSize=8M,PartitionScheme=huge_app" .
```

### GitHub Releases への自動アップロード

タグ push 時に `.bin` を自動アップロードする設定例（softprops/action-gh-release 使用）。

---

## 19.16 追加のトラブルシューティング

### メモリ関連

| 症状 | 原因 | 対処 |
|---|---|---|
| `malloc fail` | ヒープ枯渇 | グローバル配列を縮小 |
| `region dram0_0_seg overflowed` | RAM 超過 | 大きな配列を PROGMEM へ |
| `statCapacity` が 0 | ps_malloc 失敗 | 統計機能を再起動 |

### 描画関連

| 症状 | 原因 | 対処 |
|---|---|---|
| 画面が真っ暗 | バックライト OFF | 電源リセット |
| 表示が乱れる | SPI 通信エラー | SD カード抜き差し |
| 一部が欠ける | 画面バッファ境界 | drawRect 範囲確認 |

### SD カード関連

| 症状 | 原因 | 対処 |
|---|---|---|
| FAT32 でない | フォーマット誤り | PC で FAT32 再フォーマット |
| 64GB 以上 | 容量非対応 | 32GB 以下を使用 |
| 速度低下 | 相性問題 | Class 10 以上推奨 |

### Wi-Fi 関連

| 症状 | 原因 | 対処 |
|---|---|---|
| 5GHz 非対応 | ESP32-S3 仕様 | 2.4GHz AP を使用 |
| Auth failed | WPA3 | WPA2 に変更 |
| RSSI 低い | 距離・障害物 | AP に近づける |

---

## 19.17 パフォーマンスチューニング

### CPU 周波数

`240MHz` が標準。電池持ちを優先する場合は `80MHz` も選択可能（遅くなる）。

### 最適化レベル

`platformio.ini` の `-Os`（サイズ） vs `-O2`（速度）。
PoMaS 0.92 は `-Os` で Flash 56% に収まっています。

### フラッシュモード

`QIO 80MHz`（推奨）。`DIO 40MHz` は互換性重視の場合。

### パーティションスキーム

`huge_app`（3MB app / 1MB SPIFFS）が必須。これより小さいと 1.7MB のアプリが収まりません。

---

## 19.18 参考資料

### 関連リポジトリ

- [M5Cardputer-UserDemo](https://github.com/m5stack/M5Cardputer-UserDemo)
- [M5Unified-Examples](https://github.com/m5stack/M5Unified/tree/master/examples)
- [M5GFX-Examples](https://github.com/m5stack/M5GFX/tree/master/examples)

### データシート

- ESP32-S3 Technical Reference
- ST7789V2 LCD Controller
- TCA8418 Keyboard Controller

---

## 19.19 章末補足

本章で扱えなかった細かい設定・改造方法は、GitHub Discussions または Issues で質問してください。

PoMaS はオープンソースです。バグ報告・機能提案・プルリクエストを歓迎します。

---

## 19.20 ライブラリ内部動作解説

### 19.20.1 M5Cardputer ライブラリ（1.1.1）

**役割**: Cardputer-Adv のハードウェア抽象化

**主要コンポーネント**:
- `M5Cardputer.begin(cfg)` — ハードウェア初期化
- `M5Cardputer.update()` — イベントポーリング
- `M5Cardputer.Display` — LCD アクセス
- `M5Cardputer.Keyboard` — キー入力
- `M5Cardputer.Power` — 電源管理

**内部動作**:
```
[1] I2C バス初期化（キーボード TCA8418 接続）
[2] LCD SPI 初期化（ST7789V2）
[3] バッテリー ADC 初期化
[4] update() で 50Hz 周期のキーボードスキャン
```

### 19.20.2 M5Unified ライブラリ（0.2.26）

**役割**: M5Stack 全機種の統合 API

**PoMaS での使用**:
- 限定的（M5Cardputer が内部で使用）
- 直接呼び出しは殆どなし

### 19.20.3 M5GFX ライブラリ（0.2.33）

**役割**: グラフィックス描画

**主要 API**:
- `fillScreen(color)` — 画面全体塗りつぶし
- `fillRect(x, y, w, h, color)` — 矩形
- `drawString(text, x, y)` — 文字列
- `drawLine(x0, y0, x1, y1, color)` — 線
- `drawCircle(x, y, r, color)` — 円
- `setFont(font)` — フォント設定

**内部動作**:
```
[1] スプライトバッファ（メモリ上）
[2] DMA 転送で SPI 経由 LCD へ
[3] 部分更新で高速化
```

### 19.20.4 標準ライブラリ

| ライブラリ | 役割 |
|---|---|
| WiFi | Wi-Fi 接続 |
| HTTPClient | HTTP 通信 |
| Preferences | NVS 永続化 |
| SD | SD カード |
| FS | ファイルシステム |
| SPI | SPI 通信 |

---

## 19.21 ビルドエラー完全リファレンス

### 19.21.1 コンパイルエラー

| エラー | 原因 | 対処 |
|---|---|---|
| `'X' has not been declared` | ctags プロトタイプ問題 | 前方宣言追加 |
| `'X' does not name a type` | 型未定義 | 同上 |
| `underlying type mismatch` | enum 基底型不一致 | 両方に `: int` |
| `expected ';' before` | セミコロン漏れ | 該当行前後確認 |
| `missing terminating "` | 文字列リテラル閉じ忘れ | ダブルクォート確認 |
| `too many arguments` | 関数引数過多 | 引数削減 |
| `no matching function` | 関数オーバーロード不一致 | 引数の型確認 |
| `undefined reference to 'X'` | リンク漏れ | ライブラリ追加 |
| `inconsistent types 'float' and 'double'` | 三項演算子型混在 | キャスト追加 |
| `region 'iram0_0_seg' overflowed` | メモリ超過 | サイズ削減 |

### 19.21.2 リンカエラー

| エラー | 原因 | 対処 |
|---|---|---|
| `multiple definition of 'X'` | グローバル変数の重複定義 | static 化 |
| `undefined reference to 'X'` | 関数未定義 | 実装追加 |
| `section '.text' will not fit` | Flash 超過 | パーティション変更 |

### 19.21.3 書き込みエラー

| エラー | 原因 | 対処 |
|---|---|---|
| `Failed to connect` | ダウンロードモード未突入 | G0 + リセット |
| `Permission denied: COM3` | ポート使用中 | 他アプリ閉じる |
| `MD5 of file does not match` | 転送破損 | 再書き込み |
| `Wrong offset or size` | パーティション不整合 | Erase All Flash |

---

## 19.22 ビルドの最適化

### 19.22.1 コンパイルオプション

| オプション | 効果 | トレードオフ |
|---|---|---|
| `-Os` | サイズ優先 | 速度犠牲 |
| `-O2` | 速度優先 | サイズ増 |
| `-O3` | 最大速度 | サイズ大幅増 |
| `-flto` | Link-Time Optimization | ビルド時間長 |

PoMaS は `-Os` で Flash 56% に収まっています。

### 19.22.2 PROGMEM の使用

**効果**: 定数データを Flash に配置、RAM 節約

**PoMaS での使用**:
- `constDB[]`（386 件）
- `unitDB[]`（532 件）
- `lawDB[]` / `lawDB2[]`（300 件）
- `ELEM_INFO[]`（118 件）

**コード例**:
```
static const Constant constDB[] PROGMEM = {
    {"Speed of light", "c", 299792458.0, "m/s", "Physical"},
    ...
};
// 読み出し
Constant c;
memcpy_P(&c, &constDB[i], sizeof(Constant));
```

### 19.22.3 インライン化

**効果**: 関数呼び出しオーバーヘッド削減

**PoMaS での使用**:
- `toRad` / `fromRad` / `formatNum` など
- `static inline` で内部リンケージ

---

## 19.23 カスタムファームウェア作成

### 19.23.1 機能追加の手順

**Step 1: 新しいモードを追加**

```
// GState に追加（Part 1）
enum AppMode { MODE_MENU=0, MODE_CALC, ..., MODE_MYCUSTOM };

// メインメニューに追加（Part 20）
const char* items[] = {..., "My Custom Mode"};

// loop() 内の switch に case 追加
case 8: gMyCustom.run(); break;
```

**Step 2: 新しいクラスを追加**

```
class MyCustom {
public:
    void run() {
        // メニュー表示・入力処理
    }
};
MyCustom gMyCustom;
```

**Step 3: ビルド・書き込み**

通常のビルドフローで完成。

### 19.23.2 既存機能の改変

**例: Calculator の関数追加**

Part 3 の `ExprEval::callFn` に分岐追加:

```
if (!strcasecmp(nm,"myFunc")) return myFunction(a[0]);
```

**例: メディカル計算の追加**

Part 18 の `MedicalCardio::run` の items[] に追加:

```
const char* items[] = {
    "MAP (Mean Arterial Pressure)",
    "My New Score",
    "Back"
};
```

### 19.23.3 UI の変更

**例: 配色の変更**

Part 2 の色定義を変更:

```
#define C_BG       0xF7FE
#define C_FG       0x18E3
#define C_ACC      0x34C6
// 新しい色を試す
#define C_ACC      0xFFE0
```

---

## 19.24 ファームウェア公開の手順

### 19.24.1 ビルド成果物の準備

```
[1] Arduino IDE でビルド（Ctrl+R）
[2] 生成物:
    pomas_0_92.ino.bin
    pomas_0_92.ino.bootloader.bin
    pomas_0_92.ino.partitions.bin
    pomas_0_92.ino.merged.bin
    boot_app0.bin
[3] pomas_0_92.ino.merged.bin を GitHub Release に添付
```

### 19.24.2 リリースノート記載項目

- バージョン番号
- ビルド日時
- FQBN
- 主要な変更（CHANGELOG 参照）
- 既知の制限
- インストール手順

### 19.24.3 GitHub Release 作成

```
[1] GitHub → Releases → Draft a new release
[2] Tag: v0.92
[3] Title: PoMaS v0.92
[4] Description: CHANGELOG から引用
[5] Attach: pomas_0_92.ino.merged.bin
[6] Publish release
```

---

## 19.25 この章の追加まとめ

- ライブラリ内部動作（M5Cardputer / M5GFX / 標準）
- ビルドエラー完全リファレンス（コンパイル / リンク / 書き込み）
- ビルド最適化（-Os / PROGMEM / inline）
- カスタムファームウェア作成（モード/関数/UI 追加）
- ファームウェア公開手順（GitHub Release）

### 確認問題（追加）

Q19-6: PROGMEM の目的は？ → 定数データを Flash に配置して RAM 節約

Q19-7: `-Os` と `-O2` の違いは？ → -Os サイズ優先、-O2 速度優先

Q19-8: 新しいモードを追加する手順は？ → GState の AppMode 拡張 → items[] 追加 → loop() に case 追加

Q19-9: ビルド成果物のうち一括書き込み用は？ → pomas_0_92.ino.merged.bin

Q19-10: GitHub Release の添付ファイルは？ → merged.bin（0x0 書き込み用）

---

[← 第18章 医学計算](18_medical.md) | [目次](#/) | [第20章 uLisp 関数リファレンス →](20_ulisp.md)
