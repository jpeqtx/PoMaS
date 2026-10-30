# 第13章 設定

---

## この章で学ぶこと

- 設定へのアクセス方法
- 角度モード（DEG/RAD/GRAD）
- 表示形式（Norm/Fix/Sci/Eng）
- 表示桁数（0〜9）
- Norm 1 / Norm 2 の閾値
- NVS の物理構造・キー一覧・書き込み寿命
- NVS バックアップと復元
- 工場出荷状態への復元
- 設定が反映されない場合の対処
- 設定の組み合わせとプロファイル管理

---

## 13.1 設定とは
PoMaS の動作をカスタマイズする項目です。
変更した設定は **NVS（不揮発性ストレージ）** に保存され、再起動後も維持されます。
### アクセス方法
**方法 1**: メインメニュー → `6 Tools` → `5 Settings`
**方法 2**: Calculator 画面で `Fn + 1`
### 設定画面
```
┌─────────────────────────────────┐
│▌Settings                        │
├─────────────────────────────────┤
│1 Angle: DEG/RAD/GRAD            │
│2 Display: Norm/Fix/Sci/Eng      │
│3 Display digits                 │
│4 Save settings now              │
│5 Reset all (clear NVS)          │
│6 Back                           │
└─────────────────────────────────┘
```
---
## 13.2 Angle（角度モード）
三角関数の角度単位を選択。
| モード | 単位 | 1 周 | 用途 |
|---|---|---|---|
| **DEG** | 度数法 | 360° | 日常的（デフォルト） |
| **RAD** | 弧度法 | 2π ≈ 6.28 | 数学・物理 |
| **GRAD** | グラジアン | 400 | 測量 |
### 例: sin(30) の結果
| モード | sin(30) |
|---|---|
| DEG | 0.5 |
| RAD | -0.988 |
| GRAD | 0.454 |
### 切り替え手順
```
[1] Settings → 1 (Angle)
[2] DEG / RAD / GRAD を選択
[3] 変更は即座に反映
```
ヘッダのバッジで現在のモードを確認できます。
---
## 13.3 Display（表示形式）
数値の表示形式を選択。
| 形式 | 例 | 説明 |
|---|---|---|
| **Norm** | 1.23456 | 通常（デフォルト） |
| **Fix** | 1.23 | 固定小数点 |
| **Sci** | 1.23e+00 | 科学表記 |
| **Eng** | 1.23k | 工学表記（3 桁区切り） |
### Norm 1 / Norm 2 の違い
現在のバージョンでは、Display が Norm のとき `Norm 1` / `Norm 2` の
閾値設定があります。
| モード | 小さい数の閾値 | 例（0.005） |
|---|---|---|
| Norm 1 | 0.01 未満で指数表示 | 5e-3 |
| Norm 2 | 1e-9 未満で指数表示 | 0.005 |
---
## 13.4 Display digits（表示桁数）
`3 (Display digits)` で 0〜9 の範囲で設定。
| 桁数 | 例（π） |
|---|---|
| 3 | 3.142 |
| 5 | 3.14159 |
| 7 | 3.1415927 |
| 9 | 3.141592654 |
**クイック操作**:
- `Fn + ,` — 1 桁増やす
- `Fn + /` — 1 桁減らす
---
## 13.5 Save settings now（手動保存）
`4 (Save settings now)` で NVS に即時保存。
**自動保存**:
PoMaS は **60 秒ごと** に自動保存も行いますが、
重要な設定変更後は手動保存を推奨。
---
## 13.6 Reset all（NVS クリア）
`5 (Reset all)` で全設定を初期化。
```
[1] Reset all を選択
[2] "Clear NVS settings?" の確認
[3] Y で実行
→ 全設定がデフォルトに戻る
```
**確認必須**
誤って実行すると、以下がすべてリセットされます：
- Angle / Display / 桁数
- Wi-Fi の SSID / Pass
- RPN モード / 分数モード
---
## 13.7 NVS とは
**NVS（Non-Volatile Storage）** は、電源を切っても
データが保持されるストレージ領域です。
### NVS の仕組み
```
[1] 設定変更
[2] NVS に書き込み
[3] 電源 OFF
[4] 電源 ON → NVS から読み込み
[5] 設定が復元される
```
### 保存される項目
| キー | 内容 |
|---|---|
| angle | 角度モード |
| disp | 表示形式 |
| digits | 表示桁数 |
| norm | Norm 1/2 |
| rpn | RPN モード |
| frac | 分数モード |
| wifissid | Wi-Fi SSID |
| wifipass | Wi-Fi Pass |
---
## 13.9 NVS データの詳細

### 13.9.1 NVS の物理構造

ESP32 の NVS は **Flash パーティション `nvs`** 上に格納されます。

- サイズ: 約 20KB（partition scheme による）
- 構造: ページ（4KB）× 複数、各ページに 126 エントリ
- キー: 最大 15 文字の文字列
- 値型: int / bool / String（最大 4000 バイト）

### 13.9.2 キー一覧

| キー | 型 | 既定値 | 内容 |
|---|---|---|---|
| `angle` | int | 0 (DEG) | 角度モード |
| `disp` | int | 0 (NORM) | 表示形式 |
| `digits` | int | 4 | 表示桁数 0-9 |
| `norm` | int | 1 | Norm 1/2 |
| `rpn` | bool | false | RPN モード |
| `frac` | bool | false | 分数表示 |
| `autooff` | int | 10 | 自動電源 OFF 分 |
| `wifissid` | String | "" | Wi-Fi SSID |
| `wifipass` | String | "" | Wi-Fi Password |
| `userfns` | String | "" | ユーザー関数 |

### 13.9.3 NVS の書き込み寿命

Flash の書き換え寿命は約 **10 万回**。PoMaS は 60 秒ごとに自動保存するため、頻繁な書き換えを避けるため、変更があった場合のみ書き込む最適化が組み込まれています。

**推定寿命**: 24 時間連続使用 × 10 万回 / 1440 回/日 = 約 69 日連続（その後も Flash 全体の wear leveling で延命）。

---

## 13.10 NVS バックアップと復元

### 13.10.1 手動バックアップ

NVS 全体は `esptool.py` で吸い出し可能です:

```
esptool.py --chip esp32s3 --port COM3 read_flash 0x9000 0x5000 nvs_backup.bin
```

`0x9000` は nvs パーティション開始アドレス（huge_app scheme 時）。

### 13.10.2 バックアップの復元

```
esptool.py --chip esp32s3 --port COM3 write_flash 0x9000 nvs_backup.bin
```

### 13.10.3 設定の個別復元

`Tools → 5 Settings → Save settings now` で NVS に明示保存した後、`Reset all` で消去 → 再起動 → 手動で再設定するのが最も安全。

### 13.10.4 設定の移植

`Tools → 5 Settings` の各項目を別機体で手動再設定。SSID/Pass は NVS に直接書かれているため、機体間で共有するには手入力が必要。

---

## 13.11 工場出荷状態への復元

### 13.11.1 NVS のみリセット

```
Tools → 5 → 5 (Reset all)
→ "Clear NVS settings?" の確認 → Y
```

これで angle / disp / digits / norm / rpn / frac / autooff / wifissid / wifipass / userfns が全て初期化されます。

### 13.11.2 Flash 全体のリセット

完全な工場出荷状態にするには:

```
esptool.py --chip esp32s3 --port COM3 erase_flash
esptool.py --chip esp32s3 --port COM3 write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 pomas_0_92.ino.merged.bin
```

### 13.11.3 SD カードのリセット

SD カード内のファイルも削除する場合:

```
SD カードを取り外し、PC で FAT32 フォーマット
または /userfns.txt, /data.csv, /stat.csv, /sheet.csv, /script.lsp, /*.bmp を削除
```

---

## 13.12 設定が反映されない場合

### 13.12.1 症状別チェック

| 症状 | 原因 | 対処 |
|---|---|---|
| 角度モード変更が反映されない | 保存前の再起動 | Settings → Save settings now |
| 表示形式が変わらない | 現在の計算結果表示のみ | 次の計算から反映 |
| Wi-Fi 接続できない | SSID/Pass 誤り or 5GHz | 2.4GHz AP、パスワード再確認 |
| 自動電源 OFF が効かない | autoOffMin = 0 | 0 以外を設定 |
| ユーザー関数が消えた | NVS クリア実行 | Reset all 後に再登録 |

### 13.12.2 NVS が壊れた場合

電源断中の書き込みで NVS が破損する可能性があります。

**症状**: 起動時に設定が初期値に戻る、または特定項目のみ毎回リセット

**対処**:
```
[1] Erase Flash 全消去
[2] 再書き込み
[3] 設定を再入力
```

### 13.12.3 保存頻度の調整

60 秒ごとの自動保存を無効化するには（非推奨）:

ソースの `loop()` 内 `lastSave` の判定間隔を変更（改変時）。

---

## 13.13 各設定の組み合わせ

### 13.13.1 角度モード × 表示形式

| DEG + NRM | 日常利用（既定） |
| DEG + FIX(4) | 固定小数点、測定値向け |
| RAD + NRM | 数学・物理計算 |
| RAD + SCI | 科学技術計算 |
| GRAD + NRM | 測量（一部の国） |

### 13.13.2 表示桁数 × Norm 閾値

| digits | Norm 1 | Norm 2 |
|---|---|---|
| 3 | 概算 (1.23) | 概算 (0.005) |
| 5 | 通常 (1.2346) | 通常 (0.005) |
| 9 | 最大精度 | 最大精度 |

**推奨**: 通常は digits=4〜6、科学計算は digits=7〜9。

### 13.13.3 RPN × 分数モード

| Algebraic + Frac | 分数表示で厳密計算（推奨） |
| RPN + Frac | スタックで厳密計算 |
| Algebraic + Decimal | 一般計算 |
| RPN + Decimal | HP 電卓風 |

---

## 13.14 設定の変更履歴

### 13.14.1 v0.92 での設定変更

- `autooff` キー新規追加（既定 10 分）
- `userfns` キー追加（SD 併用）

### 13.14.2 v0.91 からの移行

v0.91 の NVS は互換。`autooff` は既定値 10 分で自動追加。

### 13.14.3 将来の拡張予定

- 画面輝度設定（v0.93 予定）
- スリープ時間設定（v0.93 予定）
- 言語切替（日/英）

---

## 13.15 設定 API（ソースコード改変時）

### 13.15.1 値の読み込み

```
Preferences p;
p.begin("pomas", true);  // readonly
int angle = p.getInt("angle", 0);
bool rpn  = p.getBool("rpn", false);
String ssid = p.getString("wifissid", "");
p.end();
```

### 13.15.2 値の書き込み

```
Preferences p;
p.begin("pomas", false);  // readwrite
p.putInt("angle", G.angle);
p.putBool("rpn", G.rpn);
p.putString("wifissid", G.wifiSSID);
p.end();
```

### 13.15.3 値の削除

```
p.remove("angle");   // 個別削除
p.clear();            // 全削除
```

### 13.15.4 新しいキーの追加例

```
// GState に追加
int myCustomSetting = 42;

// gNVS.load() 内
G.myCustomSetting = p.getInt("mysetting", 42);

// gNVS.save() 内
p.putInt("mysetting", G.myCustomSetting);
```

---

## 13.16 設定画面のカスタマイズ

### 13.16.1 メニュー項目の追加

`settingsScreen()` の items[] に追加:

```
const char* items[] = {
    "Angle: DEG/RAD/GRAD",
    "Display: Norm/Fix/Sci/Eng",
    "Display digits",
    "Auto power off (min)",
    "My new setting",     // ← 追加
    "Save settings now",
    "Reset all (clear NVS)",
    "Back"
};
int s = DM.menu("Settings", items, 8);  // 7 → 8
```

### 13.16.2 ハンドラの追加

```
else if (s == 4) {
    // 新しい設定の処理
    char b[8]; snprintf(b, sizeof(b), "%d", G.myCustomSetting);
    if (DM.inputLine("Value:", b, sizeof(b))) {
        int v = atoi(b);
        if (v >= 0 && v <= 100) G.myCustomSetting = v;
    }
}
else if (s == 5) { gNVS.save(); ... }  // 以降ずれる
```

---

## 13.17 設定のデバッグ

### 13.17.1 NVS 値の確認

シリアルモニタで全キーを出力するデバッグコード例:

```
void dumpNVS() {
    Preferences p;
    p.begin("pomas", true);
    Serial.printf("angle=%d\n", p.getInt("angle", -1));
    Serial.printf("disp=%d\n", p.getInt("disp", -1));
    Serial.printf("digits=%d\n", p.getInt("digits", -1));
    Serial.printf("autooff=%d\n", p.getInt("autooff", -1));
    Serial.printf("rpn=%d\n", p.getBool("rpn", false));
    Serial.printf("ssid=%s\n", p.getString("wifissid", "").c_str());
    p.end();
}
```

### 13.17.2 設定変更のログ

```
void logSetting(const char* key, int old, int new) {
    Serial.printf("[NVS] %s: %d -> %d\n", key, old, new);
}
```

### 13.17.3 NVS エラーの検出

```
if (!p.begin("pomas", false)) {
    Serial.println("[NVS] begin failed");
}
size_t written = p.putInt("key", 42);
if (written == 0) {
    Serial.println("[NVS] write failed");
}
```

---

## 13.18 複数プロファイルの管理

### 13.18.1 名前空間の切り替え

複数の設定セットを保持するには名前空間を分ける:

```
p.begin("pomas_work", false);   // 仕事用
p.begin("pomas_personal", false); // 個人用
```

### 13.18.2 プロファイル切替 UI 例

```
const char* profiles[] = {"Default", "Work", "Personal"};
int sel = DM.menu("Profile", profiles, 3);
if (sel == 0) p.begin("pomas", false);
else if (sel == 1) p.begin("pomas_work", false);
else p.begin("pomas_personal", false);
```

---

## 13.19 セキュリティ考慮事項

### 13.19.1 Wi-Fi パスワードの保存

- NVS は **平文** 保存（ESP32 標準）
- Flash 全体は物理アクセスで読み出し可能
- **機密性の高いパスワードは別管理を推奨**

### 13.19.2 SD カードのセキュリティ

- `/userfns.txt` などの SD ファイルは平文
- SD カード抜き取りで情報漏洩リスク
- 秘匿データは SD に置かない

### 13.19.3 NVS 暗号化（NVS Encryption）

ESP32 は NVS 暗号化に対応しますが、鍵管理の複雑さから PoMaS では未実装。必要なら esp-idf で `CONFIG_NVS_ENCRYPTION` を有効化して独自ビルド。

---

## 13.20 この章のまとめ

- 設定は NVS に永続化、60 秒ごと自動保存
- 10 個の主要キー（angle/disp/digits/norm/rpn/frac/autooff/wifissid/wifipass/userfns）
- アクセスは Tools → 5、または電卓で Fn+1
- 角度モード（DEG/RAD/GRAD）
- 表示形式（Norm/Fix/Sci/Eng）と表示桁数 0-9
- Auto power off（0-600 分、0 で無効）
- Reset all で NVS クリア
- Flash 全消去は erase_flash → merged.bin 書き込み
- 複数プロファイルは名前空間切替で実現
- Wi-Fi パスワードは平文保存、機密データは SD に置かない

### 確認問題

Q13-4: NVS とは何の略？ → Non-Volatile Storage

Q13-5: 設定変更の自動保存間隔は？ → 60 秒

Q13-6: 表示桁数を増やすクイックキーは？ → Fn + ,

Q13-7: Auto power off の既定値は？ → 10 分

Q13-8: NVS を初期化するメニューは？ → Tools → 5 → 5 (Reset all)

Q13-9: 設定が反映されない時の最初の対処は？ → Settings → Save settings now

---


---

## 13.21 設定の品質保証

### 13.21.1 設定の一貫性チェック

PoMaS 起動時に以下の整合性を確認します:

| 項目 | 確認内容 | 失敗時の動作 |
|---|---|---|
| angle | 0/1/2 のいずれか | 既定値 0（DEG） |
| disp | 0/1/2/3 のいずれか | 既定値 0（NORM） |
| digits | 0〜9 | 既定値 4 |
| norm | 1/2 | 既定値 1 |
| autooff | 0〜600 | 既定値 10 |

範囲外の値が NVS に書き込まれていた場合、起動時に既定値にフォールバックします。

### 13.21.2 NVS 破損時のリカバリ

NVS 破損のパターン:

| パターン | 症状 | 復旧 |
|---|---|---|
| パーティション全体破損 | 起動時に毎回初期化 | erase_flash |
| 一部キー破損 | 特定項目のみ初期化 | Reset all → 再設定 |
| 書き込み途中で停電 | 最後の設定が失われる | 再設定 |

### 13.21.3 設定保存のタイミング

```
[1] 明示保存: Settings → Save settings now
[2] 自動保存: 60 秒ごと（loop 内）
[3] 変更時保存: 各設定の変更直後（メモリ内のみ）
[4] 電源 OFF 前: 自動保存（最後の 60 秒以内の変更は保証）
```

---

## 13.22 設定とモード動作の関係

### 13.22.1 angle 設定の影響範囲

| モード | angle の影響 |
|---|---|
| Calculator | 三角関数・逆三角関数 |
| CAS | trigExpand など（部分的） |
| Statistics | 分布計算の一部 |
| Graph | Polar / Parametric の角度 |
| その他 | なし |

### 13.22.2 disp / digits 設定の影響範囲

| モード | 影響 |
|---|---|
| Calculator | 結果表示 |
| CAS | 係数表示 |
| Statistics | 統計量表示 |
| Graph | 座標表示 |

### 13.22.3 autooff 設定の動作

```
autoOffMin = 0   → 自動 OFF 無効
autoOffMin = 1   → 1 分無操作で OFF
autoOffMin = 10  → 10 分無操作で OFF（既定）
autoOffMin = 60  → 1 時間無操作で OFF
autoOffMin = 600 → 10 時間無操作で OFF（最大）
```

**注意**: 減光（60 秒）は autoOffMin と独立して動作します。

---

## 13.23 設定の移行ガイド

### 13.23.1 v0.91 → v0.92 の移行

| キー | v0.91 | v0.92 | 移行 |
|---|---|---|---|
| angle | あり | あり | そのまま |
| disp | あり | あり | そのまま |
| digits | あり | あり | そのまま |
| norm | あり | あり | そのまま |
| rpn | あり | あり | そのまま |
| frac | あり | あり | そのまま |
| autooff | なし | あり | 自動追加（既定 10） |
| userfns | なし | あり | 空文字列で追加 |

### 13.23.2 機種変更時の移行

NVS は機種ごとに独立。同じ設定を別機種で使うには:
1. 旧機種で設定をスクリーンショット（Settings 画面で S キー）
2. 新機種で同じ値を手入力

---

## 13.24 設定画面のベストプラクティス

### 13.24.1 最初に設定すべき項目

1. **Angle**: 用途に応じて DEG/RAD
2. **Auto power off**: 電池持ちを優先するなら短め（5 分）
3. **Display digits**: 用途に応じて 4〜6

### 13.24.2 定期的な見直し

| 頻度 | 確認項目 |
|---|---|
| 月 1 回 | NVS 保存（Save settings now） |
| 3 か月 | 不要なユーザー関数の削除 |
| 半年 | NVS バックアップ |

### 13.24.3 トラブル時の初期化フロー

```
[1] 設定がおかしい → Reset all
[2] それでもダメ → NVS クリア（erase_flash）
[3] それでもダメ → ファームウェア再書き込み
[4] それでもダメ → ハードウェア故障（要修理）
```

---

## 13.25 上級者向け設定 Tips

### 13.25.1 複数用途のプロファイル切替

`pomas`, `pomas_work`, `pomas_personal` など名前空間を分けて管理:

```
[1] デフォルト: pomas（一般計算）
[2] 仕事用: pomas_work（RAD + SCI 表示）
[3] 学習用: pomas_learn（DEG + NRM 表示）
```

### 13.25.2 表示形式の素早い切替

`Fn+,` / `Fn+/` で桁数変更。`Fn+1` で表示形式変更メニュー。

### 13.25.3 電池持ちの最適化

- `autoOffMin = 3`（3 分）
- 明るさを下げる（v0.93 予定）
- 使わないときは電源 OFF

---

## 13.26 この章の追加まとめ

- 設定の整合性は起動時にチェック、範囲外は既定値
- angle/disp は主に Calculator に影響
- autooff は減光（60 秒）と独立
- v0.91 → v0.92 は互換（autooff が自動追加）
- プロファイル管理は名前空間切替で
- 電池持ち最適化: autooff 短め + 明るさ控えめ

### 確認問題（追加）

Q13-10: angle 設定が影響するモードは？ → Calculator/CAS/Statistics/Graph

Q13-11: NVS 破損時の最初の対処は？ → Reset all

Q13-12: v0.91 から移行時に追加されるキーは？ → autooff

---

[← 第12章 ツール](12_tools.md) | [目次](#/) | [第14章 エラーメッセージ →](14_errors.md)
