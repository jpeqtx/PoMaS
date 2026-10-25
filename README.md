# PoMaS — Pocket Math Station

**M5Stack Cardputer-Adv 向けのポケット数学ステーション**

PoMaS は、M5Stack Cardputer-Adv (ESP32-S3) 上で動作する多機能関数電卓です。
数式計算から統計解析、グラフ描画、μLisp インタプリタまで、数学・科学の作業を
1 台で完結させることを目指しています。

> 🌐 **Non-Japanese readers**: this README and [MANUAL.md](docs/manual/index.md) can be
> read using your browser's translation feature.
> A print-friendly PDF is available from the [Releases](https://github.com/jpeqtx/PoMaS/releases) page.

> 🌐 **日本語で読む方へ**: 詳細な操作マニュアルは [docs/manual/index.md](docs/manual/index.md) を参照してください。
> 中学生でも読める AtoZ ガイドとして、全 17 章構成で執筆中です。

---

## ✨ 主な機能

| モード | 内容 |
|---|---|
| **Calculator** | 数式計算、履歴、メモリ、変数 A〜J、Ans バンク、複素数、基数変換、ビット演算、RPN |
| **CAS** | 多項式の評価・微分・積分・加減乗算・根、数値テイラー展開、極限、記号置換 |
| **Statistics** | 記述統計、回帰 9 種、仮説検定 11 種、ノンパラメトリック、リスク比、分布ブラウザ、多変量解析、層別解析、生存分析、メタアナリシス、統計グラフ 17 種、CSV ストリーム統計 |
| **SciRef** | 科学定数 386 件、科学法則 300 件、単位換算 532 単位、周期表 118 元素 |
| **Graph** | y=f(x)、パラメトリック、極座標、陰関数、不等式、数列、3D ワイヤフレーム、Trace、Zoom、G-Solve |
| **Tools** | スプレッドシート 45×26、μLisp インタプリタ、Wi-Fi / Maxima 連携（⚠️ サーバーは各自用意）、電源情報、設定（NVS 永続化） |

---

## 🚀 クイックスタート

### 必要なもの

- **M5Stack Cardputer-Adv** (ESP32-S3FN8, 8MB Flash)
- USB-C ケーブル
- PC（書き込み用）

### ファームウェア書き込み

1. [esptool](https://github.com/espressif/esptool) をインストール
2. [Releases](https://github.com/jpeqtx/PoMaS/releases) から `PoMaS_v0.92_Cardputer-Adv.bin` をダウンロード
3. Cardputer-Adv を USB 接続
4. 以下のコマンドを実行：

```bash
esptool.py --chip esp32s3 --port COM3 --baud 921600 \
  write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB \
  0x0 PoMaS_v0.92_Cardputer-Adv.bin
```

`COM3` は環境に合わせて変更してください。


### M5Stack Launcher を使ったインストール（マルチブート対応）

サードパーティ製の **M5Stack Launcher** を一度書き込んでおくと、以降は`.bin`ファイルをSDカードに入れるだけで、PoMaSを含む複数のファームウェアを切り替えて使えるようになります。

1. [Launcher Releases](https://github.com/bmorcelli/Launcher/releases) から `Launcher-Cardputer.bin` をダウンロード
2. M5Burner または esptool で Launcher を書き込み
3. PoMaSの`.bin`ファイル（`pomas_0_92.ino.bin` または `pomas_0_92.ino.merged.bin`）をSDカードのルートにコピー
4. Cardputerの電源を入れ、Launcher起動画面で `Enter` を押してLauncherを起動
5. SDカード内の`.bin`ファイルを選択してインストール

インストール後は、起動時に何も押さなければ最後にインストールしたファームウェアが自動起動します。Launcherに戻りたいときは、起動時に `Enter` を押してください。


### 基本操作

| キー | 動作 |
|---|---|
| `Fn + Esc`（バッククォート） | 戻る / キャンセル |
| `Enter` | 決定 / 実行 |
| `Del` | バックスペース |
| `Fn + 数字` | 各モードのショートカット |
| `;` / `.` | 上下移動 |
| `,` / `/` | 左右移動 |

詳細は **[操作マニュアル](docs/manual/index.md)** を参照してください。

---

## 🛠 ビルド（ソースから）

### Arduino IDE 2.x での手順

1. [Arduino IDE](https://www.arduino.cc/en/software) をインストール
2. **ボードマネージャ** で **M5Stack** を追加
3. **ライブラリマネージャ** で以下をインストール：
   - `M5Cardputer` 1.1.1 以上
   - `M5Unified` 0.2.21 以上
   - `M5GFX` 0.2.28 以上
4. `pomas_0_92.ino` と `splash_data.h` を同じフォルダに配置
5. ボード設定：

   | 項目 | 値 |
   |---|---|
   | ボード | M5Stack Cardputer |
   | Partition Scheme | **Huge APP (3MB No OTA/1MB SPIFFS)** |
   | Flash Size | **8MB** |
   | PSRAM | **Disabled** |

6. `Ctrl + R` でコンパイル → `Ctrl + U` で書き込み

### ビルドサイズの目安

| 項目 | 使用量 |
|---|---|
| Flash | 1.62 MB / 3.15 MB (51%) |
| RAM (Global) | 136 KB / 327 KB (41%) |

---

## 📋 動作環境

| 項目 | 内容 |
|---|---|
| ハードウェア | M5Stack Cardputer-Adv (ESP32-S3FN8) |
| Flash | 8 MB |
| PSRAM | 非搭載 |
| 画面 | ST7789V2 240×135 |
| Arduino core | M5Stack 3.3.9 |
| パーティション | huge_app |

---

## 📊 実装状況（v0.92）

### ✅ 実装済み

| モード | サブ機能 | 状態 |
|---|---|---|
| **Calculator** | 四則・関数・階乗・組合せ | ✅ |
| | 履歴・メモリ・変数 A〜J | ✅ |
| | Ans バンク（最大 20 件） | ✅ |
| | 複素数演算 | ✅ |
| | 基数変換（DEC/HEX/OCT/BIN） | ✅ |
| | ビット演算 | ✅ |
| | RPN モード | ✅ |
| | 拡張ツール（方程式 / 数値解析 / 行列 / 幾何 / 確率 / 素因数） | ✅ |
| **CAS** | 多項式評価・微分・積分・加減乗算 | ✅ |
| | 多項式の根（複素数対応） | ✅ |
| | 数値テイラー展開 / 極限 / 記号置換 | ✅ |
| **Statistics** | Lists & Descriptive | ✅ |
| | 回帰 9 種 | ✅ |
| | 検定 11 種 | ✅ |
| | ノンパラ 7 種 / リスク 5 種 | ✅ |
| | 分布ブラウザ 7 種 | ✅ |
| | 多変量解析 5 種 | ✅ |
| | 層別解析 5 種 | ✅ |
| | 生存分析 4 種 | ✅ |
| | メタアナリシス 5 種 | ✅ |
| | 統計グラフ 17 種 | ✅ |
| | CSV ストリーム統計 | ✅ |
| **SciRef** | 科学定数 386 件 | ✅ |
| | 科学法則 300 件 | ✅ |
| | 単位換算 532 単位 / 20 カテゴリ | ✅ |
| | 周期表 118 元素 | ✅ |
| | シンボル検索 | ✅ |
| **Graph** | y=f(x) / Parametric / Polar | ✅ |
| | Implicit / Inequality | ✅ |
| | Sequence / 3D wireframe | ✅ |
| | Trace / Zoom / Box zoom | ✅ |
| | G-Solve（根・極値・交点・積分） | ✅ |
| | Tangent / Normal | ✅ |
| | Square aspect (1:1) | ✅ |
| **Tools** | Spreadsheet 45×26 + 数式セル | ✅ |
| | CSV 保存 / 読込 | ✅ |
| | μLisp インタプリタ | ✅ |
| | GC（Env マーク&スイープ） | ✅ |
| | Wi-Fi 接続 / スキャン | ✅ |
| | 電源 / SD 情報 | ✅ |
| | 設定 NVS 永続化 | ✅ |
| **UI** | Paper UI v2 | ✅ |
| | スプラッシュ | ✅ |
| | BMP スクリーンショット | ✅ |
| | ESC カスケード防止 | ✅ |
| | 長文自動切り詰め | ✅ |
| **Security** | μlisp 引数ガード / nil チェック | ✅ |
| | ゼロ除算エラー化 | ✅ |
| | パーサ深さ制限 | ✅ |
| | メモリ境界チェック | ✅ |

### 🔧 既知の制限

| 項目 | 内容 | 制約理由 |
|---|---|---|
| 統計リスト | 200 要素まで | PSRAM 非搭載のため |
| μLisp セルプール | 2500 個 | RAM 制約 |
| Maxima 連携 | **クライアント側のみ実装**。Maxima 本体・HTTP プロキシサーバーは非同梱 | サーバー未同梱 |
| Wi-Fi | 2.4GHz 帯のみ | ESP32-S3 仕様 |
| 画面サイズ | 240×135 | ハードウェア固定 |

### 🚧 未完了項目（v0.93 以降）

| 優先度 | 項目 | 予定バージョン |
|---|---|---|
| 高 | UI 磨き（配色・フォント・レイアウト微調整） | v0.93 |
| 高 | スクリーンショット自動生成 | v0.93 |
| 中 | Maxima プロキシ同梱 / セルフホスト手順（現状は各自構築が必要） | v0.94 |
| 中 | μLisp の `let*` / `letrec` / `case` 追加 | v0.94 |
| 中 | 3D プロットの回転操作 | v0.94 |
| 低 | 数式履歴の NVS 永続化 | v0.95 |
| 低 | スプレッドシートのセル参照範囲 `A1:A10` | v0.95 |
| 低 | CSV エクスポート形式オプション | v1.0 |
| 低 | 多言語対応（日本語 / 英語切替） | v1.0 |

### 🐛 修正履歴（v0.92 で解消、合計 89 件）

| Step | 分類 | 件数 | 概要 |
|---|---|---|---|
| 1 | Critical | 6 | tDist 型・qrEig・implicit Y-flip・plotSeq・bSave 型・geomInv |
| 2 | High | 10 | resetAll・malloc・VIF・trace・CsvStream・bitOps・空行列 |
| 3 | High | 4 | ESC カスケード・E 変数・Square aspect |
| 4+ | Medium | 40 | μLisp GC・引数ガード・evalArgs・Env bool 化・Maxima URL |
| 5 | Security | 17 | GC マーク&スイープ・parseList EOF・if/cond/let ガード・ゼロ除算 |
| 6 | 性能・安全 | 5 | printCell 書き換え・引数過多チェック・深度制限・引数上限拡張・bAppend 最適化 |
| 7 | 軽微 | 7 | hex/strtod 順序・dead code・radical 上限・バッジ clamp・scan 上限・決定木 -1・qrEig 収束 |

### 🧪 動作検証

| 項目 | 結果 |
|---|---|
| コンパイル | ✅ 成功 |
| Flash 使用率 | 51% (1.62 MB / 3.15 MB) |
| RAM 使用率 | 41% (136 KB / 327 KB) |
| 実機動作 | ⏳ 一部確認済み / 詳細検証は継続中 |

---

## 📁 プロジェクト構成

```
PoMaS/
├── pomas_0_92.ino           # メインスケッチ（約 12,800 行）
├── splash_data.h            # スプラッシュ画像（RLE 圧縮）
├── README.md                # このファイル
├── docs/
│   ├── index.html           # GitHub Pages トップ
│   ├── manual.html          # マニュアル HTML 版
│   └── manual/              # マニュアル Markdown 分割版
│       ├── index.md
│       ├── 01_intro.md
│       └── ...（順次追加中）
├── .gitignore
└── LICENSE
```

---

## 🤝 コントリビュート

Issue / Pull Request を歓迎します。

バグ報告の際は以下を含めてください：
- 使用中のファームウェアバージョン
- 再現手順
- 期待した動作と実際の動作

---

## 📄 ライセンス

**MIT License** — 詳細は [LICENSE](LICENSE) を参照。

---

## 👤 作者

**YNGMT** — [GitHub (@jpeqtx)](https://github.com/jpeqtx)

PoMaS は YNGMT によるオリジナル実装です。

---

## 🙏 謝辞

- [M5Stack](https://m5stack.com/) — Cardputer-Adv ハードウェア
- [M5Unified](https://github.com/m5stack/M5Unified) — 統合ライブラリ
- [M5GFX](https://github.com/m5stack/M5GFX) — グラフィックスライブラリ

これらのプロジェクトなしには PoMaS は存在しません。

---

## 📈 数値

| 指標 | 値 |
|---|---|
| ソースコード行数 | 約 12,800 行 |
| モード数 | 6 |
| 統計サブモード数 | 12 |
| 組み込み関数数 | 80+ |
| 科学定数 | 386 |
| 科学法則 | 300 |
| 単位換算 | 532 |
| 周期表元素 | 118 |
| 統計グラフ種類 | 17 |
| 幾何図形 | 27 |