# 第11章 科学資料
[← 第10章 グラフ](10_graph.md) | [目次](#/) | [第12章 ツール →](12_tools.md)
---
## 11.1 SciRef モードとは
**SciRef（Scientific Reference、科学資料）** は、
科学・数学・工学のデータを素早く参照するモードです。
### 搭載データ
| 種別 | 件数 | 内容 |
|---|---|---|
| **科学定数** | 386 件 | 物理・化学・天文・生物など |
| **科学法則** | 300 件 | 物理・化学・数学・地球科学など |
| **単位換算** | 532 単位 | 20 カテゴリ |
| **周期表** | 118 元素 | 全元素の原子量 |
| **記号検索** | 全定数対象 | シンボル/名前で検索 |
---
## 11.2 SciRef モードの起動
メインメニュー → `4` または Enter で **Scientific Reference** を選択。
### メニュー
```
┌─────────────────────────────────┐
│▌SciRef                          │
├─────────────────────────────────┤
│1 Constants (386)                │
│2 Scientific Laws (300)          │
│3 Unit Converter (532)           │
│4 Periodic Table (118)           │
│5 Search by Symbol               │
│6 Back                           │
└─────────────────────────────────┘
```
---
## 11.3 Constants（科学定数）
### 起動
```
SciRef → 1 (Constants)
```
### カテゴリ
11 のカテゴリに分類されています。
| カテゴリ | 件数 | 例 |
|---|---|---|
| **Physical** | 38 | 光速、プランク定数、重力定数 |
| **Math** | 8 | π、e、黄金比 |
| **EE** | 40 | 材料の抵抗率、移動度 |
| **Astronomy** | 12 | AU、光年、太陽質量 |
| **Planet** | 64 | 8 惑星 × 8 項目 |
| **DwarfMoon** | 32 | 準惑星・主要衛星 |
| **Geo** | 23 | 地球半径、重力、地殻速度 |
| **Atmos** | 11 | 海面気圧、気温減率 |
| **Ocean** | 4 | 海水密度、音速 |
| **Element** | 124 | 化学元素の原子量など |
| **Biology** | 30 | アミノ酸、DNA |
### 操作フロー
```
[1] Constants
[2] カテゴリ選択（例: Physical）
[3] 定数一覧から選択
[4] 詳細画面表示
```
### 詳細画面
**例: 光速 c**
```
┌─────────────────────────────────┐
│▌c                               │
├─────────────────────────────────┤
│Speed of light in vacuum         │
│                                 │
│299792458                        │  ← 値（大字）
│m/s                              │  ← 単位
│[Physical]                       │  ← カテゴリ
├─────────────────────────────────┤
│Y:Ans  B:Bank+       S:Save      │
└─────────────────────────────────┘
```
### 操作
| キー | 動作 |
|---|---|
| `Y` | Ans に転送 |
| `B` | Ans バンクに追加 |
| `S` | BMP 保存 |
| `Fn + Esc` | 一覧に戻る |
### Ans に転送（Y）
定数の値を Ans にセット。続けて計算できます。
**例: 光速 c から 1 秒間の距離を計算**
```
[1] Constants → Physical → c
[2] Y（Ans = 299792458）
[3] Fn + Esc で Calculator に戻る
[4] Ans * 1 = 299792458
```
### Ans バンクに追加（B）
複数の定数を記録しておきたい場合に使います。
```
[1] 定数詳細で B を押す
[2] Bank+ (n/20) と表示
→ Ans バンクに保存される
```
最大 20 件まで。`Fn + b` でバンク一覧を開けます。
---
## 11.4 Scientific Laws（科学法則）
### 起動
```
SciRef → 2 (Scientific Laws)
```
### カテゴリ
| カテゴリ | 件数 |
|---|---|
| **Physics** | 100 |
| **Chemistry** | 70 |
| **Math** | 50 |
| **GeoSci** | 30 |
| **Astronomy** | 20 |
| **Biology** | 20 |
| **EE** | 11 |
### 操作フロー
```
[1] Scientific Laws
[2] カテゴリ選択（例: Physics）
[3] 法則一覧から選択
[4] 詳細画面表示
```
### 詳細画面
**例: ニュートンの第 2 法則**
```
┌─────────────────────────────────┐
│▌Newton's Second Law             │
├─────────────────────────────────┤
│Formula:                         │
│F = m a                          │
│                                 │
│Note:                            │
│Net force = mass x acceleration. │
│F=dp/dt.                         │
├─────────────────────────────────┤
│S:Save              ESC:back     │
└─────────────────────────────────┘
```
### 各エントリの構成
- **Name**: 法則名（英語）
- **Formula**: 数式
- **Notes**: 説明
- **Category**: カテゴリ
### 注目の法則
**物理**:
- ニュートン 3 法則
- ケプラー 3 法則
- マクスウェル方程式
- シュレディンガー方程式
- 不確定性原理
- 特殊相対性理論
**化学**:
- ヘスの法則
- ル・シャトリエの原理
- アレニウスの式
- ファラデーの電気分解
- ウッドワード・ホフマン則
**数学**:
- ピタゴラスの定理
- フェルマーの最終定理（言及）
- オイラーの等式
- ベイズの定理
- 中心極限定理
---
## 11.5 Unit Converter（単位換算）
### 起動
```
SciRef → 3 (Unit Converter)
```
### 20 カテゴリ
| カテゴリ | 例 |
|---|---|
| Length | m, km, inch, mile, 尺 |
| Area | m², 坪, acre |
| Volume | L, gallon, 合 |
| Mass | kg, lb, 貫 |
| Velocity | m/s, km/h, knot |
| Acceleration | m/s², gal |
| Force | N, kgf, lbf |
| Pressure | Pa, bar, psi |
| Energy | J, cal, kWh |
| Power | W, hp, PS |
| Temperature | °C, °F, K |
| Angle | deg, rad, grad |
| Time | s, min, h, day |
| Digital Info | bit, byte, KB, MB |
| Frequency | Hz, rpm |
| Volumetric Flow | L/s, gpm |
| Density | kg/m³, g/cm³ |
| Viscosity | Pa·s, poise |
| Radiation | Bq, Gy, Sv |
| Misc Quantity | dozen, percent |
### 操作フロー
```
[1] Unit Converter
[2] カテゴリ選択（例: Length）
[3] From 単位を選択
[4] To 単位を選択
[5] 値を入力
[6] 結果表示
```
### 例: 1 マイルを km に換算
```
[1] Unit Converter → Length
[2] From: mile
[3] To: km
[4] Value: 1
→ 1 mi
  = 1.609344 km
```
### 例: 100 °F を °C に換算
```
[1] Unit Converter → Temperature
[2] From: Fahrenheit
[3] To: Celsius
[4] Value: 100
→ 100 °F
  = 37.7778 °C
```
### 例: 100 坪を m² に換算
```
[1] Unit Converter → Area
[2] From: tsubo
[3] To: m²
[4] Value: 100
→ 100 tsubo
  = 330.5785 m²
```
### 操作
| キー | 動作 |
|---|---|
| `Y` | Ans に転送 |
| `S` | BMP 保存 |
| `Fn + Esc` | 一覧に戻る |
### 温度換算の計算式
温度だけは他の単位と異なり、**オフセット**があります。
```
°C → K: K = °C + 273.15
°F → °C: °C = (°F - 32) × 5/9
```
PoMaS は内部でこの変換を自動処理します。
---
## 11.6 Periodic Table（周期表）
### 起動
```
SciRef → 4 (Periodic Table)
```
### 周期表画面
```
┌─────────────────────────────────┐
│▌Periodic Table                  │
├─────────────────────────────────┤
│H              He                │
│Li Be      B C N O F Ne          │
│Na Mg      Al Si P S Cl Ar       │
│K  Ca Sc Ti V Cr Mn Fe Co Ni Cu Zn Ga Ge As Se Br Kr
│...                              │
├─────────────────────────────────┤
│Z=1 H 1.008 g/mol                │
└─────────────────────────────────┘
```
118 元素が配置されています。
### 操作
| キー | 動作 |
|---|---|
| `;` | 上の元素へ |
| `.` | 下の元素へ |
| `,` | 左の元素へ |
| `/` | 右の元素へ |
| `Enter` | 元素詳細を表示 |
| `Fn + Esc` | SciRef メニューへ |
### 元素の位置
標準的な周期表の配置：
- **1〜2 族**: 左端（アルカリ金属・アルカリ土類）
- **3〜12 族**: 中央（遷移金属）
- **13〜18 族**: 右側（p ブロック）
- **ランタノイド**: 別行（La〜Lu）
- **アクチノイド**: 別行（Ac〜Lr）
### 色分け
| ブロック | 色 |
|---|---|
| s ブロック | 淡青緑 |
| 1〜2 族 | 淡橙 |
| ランタノイド | 淡黄 |
| アクチノイド | 淡桃 |
| d ブロック | 淡水色 |
| p ブロック | 淡灰 |
### 選択中の元素
画面下部に選択中の元素情報が表示されます。
```
Z=1  H  1.008 g/mol
```
### 元素詳細
`Enter` で詳細画面：
```
┌─────────────────────────────────┐
│▌Element 1                       │
├─────────────────────────────────┤
│H                                │  ← 元素記号（特大）
│Z=1                              │
│1.008 g/mol                      │
│Hydrogen                         │
├─────────────────────────────────┤
│Y:weight->Ans        ESC:back    │
└─────────────────────────────────┘
```
### Y キーで Ans に転送
原子量を Ans にセット。分子量の計算などに使えます。
**例: H₂O の分子量を計算**
```
[1] Periodic Table → H → Y（Ans = 1.008）
[2] Fn + Esc → Periodic Table → O → Y（Ans = 15.999）
[3] 1.008 * 2 + 15.999 = 18.015
```
---
## 11.7 Search by Symbol（記号検索）
### 起動
```
SciRef → 5 (Search by Symbol)
```
### 使い方
シンボルや名前の一部を入力すると、
一致する定数を検索します。
**例: "grav" で検索**
```
[1] Search by Symbol
[2] 検索語: grav
→ 該当する定数の一覧:
  G: Newtonian constant of gravitation
  g0_p: Standard gravity (g0)
  g_p: Earth surface gravity (g)
  ...
```
### 検索の仕組み
- 大文字小文字を区別しない
- シンボル・名前の**部分一致**
- 最大 32 件まで表示
**例: "planck" で検索**
```
→ h: Planck constant
  hbar: Reduced Planck constant
```
### 結果から詳細へ
一覧から選ぶと、定数詳細画面に移動。
`Y` で Ans 転送、`B` で Ans バンク追加が可能。
---
## 11.8 実用例
### 例 1: 光の速さから 1 光年を計算
```
[1] Constants → Physical → c（光速）
[2] Y（Ans = 299792458）
[3] Fn + Esc → Calculator
[4] Ans * 60 * 60 * 24 * 365.25
→ 9.4607e15 m（1 光年）
```
### 例 2: 地球の重力加速度から自由落下距離
**問題**: 物体を 5 秒間自由落下させたときの落下距離
```
[1] Constants → Geo → g_p (9.80665)
[2] Y（Ans = 9.80665）
[3] Calculator
[4] 0.5 * Ans * 5^2
→ 122.58 m
```
公式: h = (1/2)gt²
### 例 3: 摂氏 25°C を華氏に換算
```
[1] Unit Converter → Temperature
[2] From: Celsius, To: Fahrenheit
[3] Value: 25
→ 77 °F
```
### 例 4: アボガドロ数から分子数を求める
**問題**: 水 18g に含まれる水分子の数
```
[1] Constants → Physical → N_A
[2] Y（Ans = 6.022e23）
[3] Calculator
[4] Ans * 1（18g = 1mol）
→ 6.022e23
```
### 例 5: 周期表で分子量を計算
**問題**: CO₂ の分子量
```
[1] Periodic Table → C → Y（12.011）
[2] Fn + Esc → Periodic Table → O → Y（15.999）
[3] Calculator
[4] 12.011 + 2*15.999 = 44.009
```
### 例 6: 電磁気の単位換算
**問題**: 1 T（テスラ）は何ガウスか
```
[1] Unit Converter → Misc (or Density 等)
```
⚠️ **テスラ/ガウスは搭載範囲外**
電磁気の磁束密度は、Misc カテゴリにも該当がありません。
必要に応じて、Calculator で直接計算してください。
（1 T = 10000 G）
---
## 11.9 データの構造
### 科学定数の出典
| 分類 | 主な出典 |
|---|---|
| 物理定数 | CODATA 2022 |
| 数学定数 | 標準的な数学定数 |
| 天文定数 | IAU 2015 推奨値 |
| 惑星データ | NASA JPL |
| 化学元素 | IUPAC 標準原子量 |
| 生物データ | 標準的な生化学値 |
### 単位換算の精度
各単位の換算係数は、国際標準に基づきます。
| 例 | 係数 |
|---|---|
| 1 inch | 0.0254 m（厳密値） |
| 1 mile | 1609.344 m（厳密値） |
| 1 lb | 0.45359237 kg（厳密値） |
| 1 US gallon | 0.003785411784 m³（厳密値） |
---
## 11.10 トラブルシューティング
| 症状 | 原因 | 対処 |
|---|---|---|
| カテゴリが「Empty」 | データ未登録 | 別カテゴリを選択 |
| 検索で見つからない | 検索語が短い | 3 文字以上で検索 |
| Ans に転送されない | Ans バンクが満杯 | バンク整理後、再実行 |
| 周期表の元素が小さい | 画面サイズの制約 | Enter で詳細画面へ |
| 単位換算の結果が変 | 温度のオフセット | 温度は特別処理（自動） |
---
## 11.11 この章のまとめ
- SciRef は科学・数学・工学のリファレンスモード
- 科学定数 386 件（11 カテゴリ）
- 科学法則 300 件（7 カテゴリ）
- 単位換算 532 単位（20 カテゴリ）
- 周期表 118 元素
- 記号検索で素早くアクセス
- Y キーで Ans 転送、B キーで Ans バンク追加
### 確認問題
📝 **Q11-1**: 光速 c の値は？
<details><summary>答え</summary>
299,792,458 m/s（Constants → Physical → c）
</details>
📝 **Q11-2**: 1 マイルを km に換算すると？
<details><summary>答え</summary>
1.609344 km（Unit Converter → Length）
</details>
📝 **Q11-3**: 摂氏 100°C を華氏にすると？
<details><summary>答え</summary>
212 °F（Unit Converter → Temperature）
</details>
📝 **Q11-4**: 水の分子量を周期表から計算するには？
<details><summary>答え</summary>
H = 1.008 × 2 + O = 15.999 × 1 = 18.015
（Periodic Table で各元素の Y キーで Ans 転送）
</details>
📝 **Q11-5**: 定数を Ans バンクに保存するキーは？
<details><summary>答え</summary>
B キー（Bank+）。
</details>
📝 **Q11-6**: 「planck」で検索すると何が見つかる？
<details><summary>答え</summary>
プランク定数 h と、換算プランク定数 hbar。
</details>
---
[← 第10章 グラフ](10_graph.md) | [目次](#/) | [第12章 ツール →](12_tools.md)
