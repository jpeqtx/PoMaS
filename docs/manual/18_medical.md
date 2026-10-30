# 第18章 医学計算

[← 第17章 付録](17_appendix.md) | [目次](#/) | [第19章 ビルドと書き込み →](19_build.md)

---

## この章で学ぶこと

- 医学計算モードの構成（Clinical Calculators 10 領域 ＋ Epidemiology & Statistics 5 項目）
- 各計算の入力項目・既定値・計算式・判定基準
- 結果画面の操作（Ans への転送、BMP 保存）
- 入力エラーの意味と対処
- 使用上の注意（教育・参考用途）

---

## 18.1 医学計算モードとは

**Medical Calculator** は、臨床でよく使う計算式と疫学・統計の機能を 1 つのメニューにまとめたモードです。値を順に入力するだけで、結果と判定コメントが表示されます。

### 使用上の注意

本モードの計算式・判定基準は、ファームウェアに実装された内容に従います。教育・参考用途の計算機であり、診断や治療方針の決定は、必ず臨床判断と各施設の基準に基づいて行ってください。数値を入力する前に、単位（mg/dL、mEq/L など）を確認してください。

---

## 18.2 起動とメニュー構成

メインメニューで `4`（Medical Calculator）を選びます。

```
Medical Calculator
├─ 1 Clinical Calculators
│    ├─ 1 Cardiology / Vitals
│    ├─ 2 Pulmonology
│    ├─ 3 Hematology
│    ├─ 4 Clinical Scores
│    ├─ 5 Body Composition
│    ├─ 6 Renal Function
│    ├─ 7 Lytes / Acid-Base
│    ├─ 8 Endocrine
│    ├─ 9 Obstetrics
│    └─ 10 Pharmacokinetics
└─ 2 Epidemiology & Statistics
     ├─ 1 Survival Analysis
     ├─ 2 Meta-Analysis
     ├─ 3 Outbreak / Source Trace
     ├─ 4 Risk / Odds Ratio (2x2)
     └─ 5 Confidence Intervals
```

本章で `Medical → 1 → 6 → 1` と書く場合は、「Medical Calculator → Clinical Calculators → Renal Function → eGFR CKD-EPI 2021」を表します。

### 入力と結果画面

- 各項目は入力ダイアログで 1 つずつ尋ねられます。既定値が入っているので、そのまま `Enter` でも進めます。
- 数値入力で ESC を押すと、計算を中止してメニューに戻ります。Clinical Scores と Obstetrics も同様で、途中の入力をキャンセルすると残りの入力を求めず、結果を表示せずにメニューへ戻ります。
- 入力値が範囲外（0 以下など）の場合は、メッセージを出さずにメニューへ戻る項目があります。エラーメッセージが出る項目は、各節に記載しています。

結果画面のキーは次のとおりです。

| キー | 動作 |
|---|---|
| `Y` | 結果を `Ans` に転送（電卓で `Ans` として使用可能） |
| `S` | 画面を BMP で保存（SD カードが必要） |
| `ESC` | メニューに戻る |

---

## 18.3 Cardiology / Vitals（循環・バイタル）

パス: `Medical → 1 → 1`

### 18.3.1 MAP（平均動脈圧）

| 項目 | 内容 |
|---|---|
| 入力 | SBP（既定 120）、DBP（既定 80） |
| 式 | MAP = (SBP + 2×DBP) / 3 |
| 判定 | < 65: LOW（hypoperfusion）／ 70〜100: Normal ／ > 100: HIGH |

例: SBP 120、DBP 80 → MAP = 93.3

エラー: DBP が SBP より大きいと `DBP must be <= SBP` と表示されます。

### 18.3.2 Pulse Pressure（脈圧）

PP = SBP − DBP。判定コメントはありません。入力とエラーは MAP と同じです。

### 18.3.3 Rate-Pressure Product（RPP）

| 項目 | 内容 |
|---|---|
| 入力 | HR（既定 70）、SBP（既定 120） |
| 式 | RPP = HR × SBP |
| 判定 | > 20000: HIGH ／ それ以外: Normal |

### 18.3.4 Shock Index

| 項目 | 内容 |
|---|---|
| 入力 | HR（既定 80）、SBP（既定 120） |
| 式 | SI = HR / SBP |
| 判定 | ≥ 0.9: Elevated ／ < 0.9: Normal |

---

## 18.4 Pulmonology（呼吸器）

パス: `Medical → 1 → 2`

### 18.4.1 P/F ratio

| 項目 | 内容 |
|---|---|
| 入力 | PaO2 [mmHg]（既定 80）、FiO2 [0〜1]（既定 0.21） |
| 式 | P/F = PaO2 / FiO2 |
| 判定 | < 100: Severe ARDS ／ 100〜200: Moderate ／ 200〜300: Mild ／ ≥ 300: Normal |

FiO2 は 0.21 のような小数で入力します（21 ではありません）。

### 18.4.2 A-a gradient

| 項目 | 内容 |
|---|---|
| 入力 | PaO2、PaCO2、FiO2 [0〜1]、年齢（既定 80 / 40 / 0.21 / 40） |
| 式 | PAO2 = FiO2 × 713 − PaCO2 / 0.8、A-a = PAO2 − PaO2 |
| 補足表示 | 年齢別の期待正常値 = 年齢 / 4 + 4 |

例: PaO2 80、PaCO2 40、FiO2 0.21、年齢 40 → A-a = 99.7 に対し、期待正常値は 14.0

FiO2 が 0 以下または 1 を超える場合は、何も表示せずに戻ります。

### 18.4.3 Oxygenation Index（OI）

| 項目 | 内容 |
|---|---|
| 入力 | 平均気道内圧（既定 10）、FiO2（既定 0.5）、PaO2（既定 60） |
| 式 | OI = 平均気道内圧 × FiO2 × 100 / PaO2 |
| 判定 | > 25: Severe ／ 15〜25: Moderate ／ < 15: Mild |

---

## 18.5 Hematology（血液）

パス: `Medical → 1 → 3`

### 18.5.1 ANC（絶対好中球数）

| 項目 | 内容 |
|---|---|
| 入力 | WBC [×10³/µL]（既定 5.0）、好中球 %（既定 60）、桿状核 %（既定 0） |
| 式 | ANC = WBC × (好中球% + 桿状核%) × 10 |
| 表示 | ANC [/µL]、WBC、好中球＋桿状核 % |

例: WBC 5.0、好中球 60%、桿状核 0% → ANC = 3000 /µL

%の入力が 0〜100 の範囲外なら、何も表示せずに戻ります。

### 18.5.2 赤血球指数（MCV / MCH / MCHC）

| 項目 | 内容 |
|---|---|
| 入力 | Hb [g/dL]（既定 14）、Hct [%]（既定 42）、RBC [×10⁶/µL]（既定 5.0） |
| 式 | MCV = Hct × 10 / RBC [fL]、MCH = Hb × 10 / RBC [pg]、MCHC = Hb × 100 / Hct [g/dL] |

### 18.5.3 Mentzer index

| 項目 | 内容 |
|---|---|
| 入力 | MCV [fL]（既定 80）、RBC [×10⁶/µL]（既定 5.0） |
| 式 | MI = MCV / RBC |

MI の値と、入力した MCV・RBC を並べて表示します。判定コメントは表示しません。

---

## 18.6 Clinical Scores（臨床スコア）

パス: `Medical → 1 → 4`

スコア類は、各項目を整数で入力します。

### 18.6.1 GCS（Glasgow Coma Scale）

| 項目 | 範囲 |
|---|---|
| Eye | 1〜4 |
| Verbal | 1〜5 |
| Motor | 1〜6 |

合計 = E + V + M（3〜15）。

| 合計 | 判定 |
|---|---|
| ≤ 8 | Severe（intubate） |
| 9〜12 | Moderate |
| 13〜15 | Mild |

範囲外の入力は `E1-4 V1-5 M1-6` と表示されます。

### 18.6.2 CURB-65

5 項目を 0（なし）/ 1（あり）で入力します。合計 0〜5。

| 項目 | 内容 |
|---|---|
| Confusion | 意識障害 |
| Urea > 7 | 尿素 > 7 mmol/L |
| RR ≥ 30 | 呼吸数 30 回/分以上 |
| SBP < 90 または DBP ≤ 60 | 低血圧 |
| Age ≥ 65 | 65 歳以上 |

| 合計 | 判定 |
|---|---|
| 0〜1 | Low risk（outpatient） |
| 2 | Moderate（short admission） |
| 3 以上 | High risk（admit） |

0/1 以外を入力すると `0/1 only` と表示されます。

### 18.6.3 qSOFA

3 項目を 0/1 で入力します（RR ≥ 22、SBP ≤ 100、GCS < 15）。合計 2 以上で High sepsis risk、1 以下で Low risk です。0/1 以外は `0/1 only`。

### 18.6.4 Child-Pugh

5 項目（Encephalopathy、Ascites、Bilirubin、Albumin、INR）について、**検査値ではなく 1〜3 の点数** を入力します。合計 5〜15。

| 合計 | クラス |
|---|---|
| 5〜6 | Class A |
| 7〜9 | Class B |
| 10〜15 | Class C |

範囲外は `1-3 each` と表示されます。

### 18.6.5 MELD

| 項目 | 内容 |
|---|---|
| 入力 | Bilirubin [mg/dL]、INR、Creatinine [mg/dL]（既定はいずれも 1.0） |
| 式 | 3.78·ln(Bili) + 11.2·ln(INR) + 9.57·ln(Cr) + 6.43 |
| 下限・上限 | Bili・INR・Cr は 1.0 未満を 1.0 として扱い、Cr は 4.0 を上限とします。スコアは 6〜40 に制限されます。 |

| スコア | 判定コメント |
|---|---|
| < 10 | Low（3 か月死亡率 約 2%） |
| 10〜19 | Moderate（約 6〜20%） |
| 20〜29 | High（約 25〜50%） |
| 30 以上 | Very high（> 50%） |

例: Bili 2.0、INR 1.5、Cr 1.2 → MELD = 15.3

Na による補正（MELD-Na）や透析の補正は含まれません。

---

## 18.7 Body Composition（体格）

パス: `Medical → 1 → 5`

### 18.7.1 BMI

| 項目 | 内容 |
|---|---|
| 入力 | 体重 [kg]（既定 70）、身長 [cm]（既定 170） |
| 式 | BMI = 体重 / (身長[m])² |

| BMI | 判定 |
|---|---|
| < 18.5 | Underweight |
| 18.5〜24.9 | Normal |
| 25〜29.9 | Overweight |
| 30〜34.9 | Obese I |
| 35〜39.9 | Obese II |
| ≥ 40 | Obese III |

例: 70 kg、170 cm → BMI = 24.2

### 18.7.2 BSA（体表面積）

| 方式 | 式 | 例（70 kg、170 cm） |
|---|---|---|
| Mosteller | √(身長[cm] × 体重[kg] / 3600) | 1.82 m² |
| DuBois | 0.007184 × 体重^0.425 × 身長^0.725 | 1.81 m² |

### 18.7.3 IBW（Devine、理想体重）

入力は身長 [cm] のみです。身長をインチに換算して、次の式で計算します。

- 男性: 50 + 2.3 × (インチ − 60) kg
- 女性: 45.5 + 2.3 × (インチ − 60) kg

計算前に `Female? (0/1):` を尋ねます（既定値 0 = 男性）。0 = 男性、1 = 女性を入力すると、選択した性別の式で IBW を計算します。0/1 以外や ESC では何も表示せずメニューへ戻ります。結果画面のラベルは `Male` または `Female` となり、`Ans` へ転送されるのは選択した性別の値です。注記には両方の値が表示されます。例: 170 cm → 男性 65.9 kg／女性 61.4 kg

### 18.7.4 ABW（補正体重）

| 項目 | 内容 |
|---|---|
| 入力 | 実体重 [kg]（既定 80）、身長 [cm]（既定 170） |
| 式 | ABW = IBW + 0.4 × (実体重 − IBW) |

この計算の IBW にも、性別入力（`Female? (0/1):`、既定値 0 = 男性）で選んだ式（男性 50.0／女性 45.5 を基点とする Devine 式）が使われます。0/1 以外や ESC では何も表示せずメニューへ戻ります。

---

## 18.8 Renal Function（腎機能）

パス: `Medical → 1 → 6`

eGFR・CrCl の結果には、次の CKD ステージが併記されます。

| 値 | ステージ |
|---|---|
| ≥ 90 | G1（normal） |
| 60〜89 | G2（mild） |
| 45〜59 | G3a |
| 30〜44 | G3b |
| 15〜29 | G4（severe） |
| < 15 | G5（failure） |

### 18.8.1 eGFR CKD-EPI 2021

| 項目 | 内容 |
|---|---|
| 入力 | SCr [mg/dL]（既定 1.0）、年齢（既定 50）、Female（0/1） |
| 式 | 142 × min(SCr/k, 1)^a × max(SCr/k, 1)^−1.200 × 0.9938^年齢（女性は × 1.012） |
| 係数 | 男性: k = 0.9、a = −0.302 ／ 女性: k = 0.7、a = −0.241 |

例: SCr 1.0、50 歳、男性 → eGFR = 91.7（G1）

結果の単位は mL/min/1.73m² です。

### 18.8.2 Cockcroft-Gault

| 項目 | 内容 |
|---|---|
| 入力 | 年齢（既定 50）、体重 [kg]（既定 70）、SCr [mg/dL]（既定 1.0）、Female（0/1） |
| 式 | CrCl = (140 − 年齢) × 体重 / (72 × SCr)（女性は × 0.85） |

例: 50 歳、70 kg、SCr 1.0、男性 → CrCl = 87.5 mL/min

併記されるステージは CKD の G 分類を流用した参考表示です。薬剤の用量調整には、各薬剤の添付文書の基準を使ってください。

### 18.8.3 Schwartz（小児）

| 項目 | 内容 |
|---|---|
| 入力 | 身長 [cm]（既定 120）、SCr [mg/dL]（既定 0.6） |
| 式 | eGFR = 0.413 × 身長 / SCr |

例: 120 cm、SCr 0.6 → eGFR = 82.6

---

## 18.9 Lytes / Acid-Base（電解質・酸塩基）

パス: `Medical → 1 → 7`

| 項目 | 入力（既定値） | 式 | 備考 |
|---|---|---|---|
| 1 Corrected Ca（Payne） | Ca [mg/dL]（9.0）、Alb [g/dL]（4.0） | Ca + 0.8 × (4.0 − Alb) | 例: Ca 9.0、Alb 2.5 → 10.2 |
| 2 Corrected Na（Katz） | Na [mEq/L]（140）、Glu [mg/dL]（100） | Na + 1.6 × (Glu − 100) / 100 | 例: Na 140、Glu 400 → 144.8 |
| 3 Anion gap | Na（140）、Cl（104）、HCO3（24） | AG = Na − (Cl + HCO3) | > 12: High AG（MUDPILES）／ < 8: Low ／ 8〜12: Normal |
| 4 Serum osmolality | Na（140）、Glu（100）、BUN（14） | 2Na + Glu/18 + BUN/2.8 | 例: 既定値で 290.6 |
| 5 Winter's formula | HCO3（24）、pCO2（40） | 予測 pCO2 = 1.5 × HCO3 + 8（範囲 +6〜+10） | 下記参照 |
| 6 Delta gap / ratio | Na（140）、Cl（104）、HCO3（24） | ΔAG = AG − 12、ΔHCO3 = 24 − HCO3、比 = ΔAG / ΔHCO3 | 下記参照 |
| 7 Free water deficit | 体重 [kg]（70）、Na（160） | 0.6 × 体重 × (Na/140 − 1) | 例: 70 kg、Na 160 → 6.0 L |

**Winter's formula**: 画面には予測 pCO2 の中央値（1.5 × HCO3 + 8）と、範囲（+6〜+10）が表示されます。入力した測定 pCO2 は予測範囲と比較され、注記に判定が表示されます。`Meas<exp: +resp alk`（予測より低い＝呼吸性アルカローシスを合併）、`Meas>exp: +resp acid`（予測より高い＝呼吸性アシドーシスを合併）、`Appropriate`（予測範囲内で適切）のいずれかです。

**Delta ratio の判定**:

| 比 | 判定 |
|---|---|
| < 0.4 | Pure NAGMA |
| 0.4〜1.0 | Mixed / NAGMA |
| 1.0〜2.0 | Pure HAGMA |
| > 2.0 | Mixed HAGMA ＋ metabolic alkalosis |

ΔHCO3 がほぼ 0 の場合、比は 0 として扱われます。

**Free water deficit**: 結果が負（Na < 140）のときは、`Na <140: excess` と表示されます。

---

## 18.10 Endocrine（内分泌）

パス: `Medical → 1 → 8`

### 18.10.1 HOMA-IR

| 項目 | 内容 |
|---|---|
| 入力 | 空腹時血糖 [mg/dL]（既定 90）、空腹時インスリン [µU/mL]（既定 10） |
| 式 | HOMA-IR = 血糖 × インスリン / 405 |

| HOMA-IR | 判定 |
|---|---|
| < 2.0 | Normal |
| 2.0〜2.8 | Borderline |
| ≥ 2.9 | Insulin resistant |

例: 既定値 → 2.2（Borderline）

### 18.10.2 Free T4 index（FTI）

| 項目 | 内容 |
|---|---|
| 入力 | Total T4 [µg/dL]（既定 8）、T3 uptake [%]（既定 30） |
| 式 | FTI = Total T4 × (T3 uptake / 30) |

| FTI | 判定 |
|---|---|
| < 4.5 | Low |
| 4.5〜12 | Normal |
| > 12 | High |

---

## 18.11 Obstetrics（産科）

パス: `Medical → 1 → 9`

日付は、年・月・日を別々に入力します。

### 18.11.1 Naegele（分娩予定日）

| 項目 | 内容 |
|---|---|
| 入力 | LMP（最終月経初日）の年・月・日（既定 2024 / 1 / 1） |
| 式 | EDD = LMP + 280 日 |

例: LMP 2024-01-01 → EDD = 2024-10-07

存在しない日付（13 月、2 月 30 日など）は `Bad date` と表示されます。

### 18.11.2 Gestational age（在胎週数）

| 項目 | 内容 |
|---|---|
| 入力 | LMP の年・月・日、今日の年・月・日 |
| 表示 | `GA = ○w ○d (○ d)` |

例: LMP 2024-01-01、今日 2024-06-01 → GA = 21w 5d（152 日）

`Ans` には、経過日数（週数ではなく日数）が入ります。今日が LMP より前の場合は `Negative days` と表示されます。

---

## 18.12 Pharmacokinetics（薬物動態）

パス: `Medical → 1 → 10`

| 項目 | 入力（既定値） | 式 |
|---|---|---|
| 1 Half-life（k から） | k [1/h]（0.1） | t1/2 = 0.693 / k |
| 2 Half-life（Vd, CL） | Vd [L]（50）、CL [L/h]（5） | t1/2 = 0.693 × Vd / CL |
| 3 Loading dose | Vd [L]（50）、目標濃度 [mg/L]（10）、F [0〜1]（1） | LD = Vd × Ctarget / F |
| 4 Maintenance dose | CL [L/h]（5）、目標 Css [mg/L]（10）、F（1） | MD = CL × Css / F |
| 5 Dose by weight | 用量 [mg/kg]（10）、体重 [kg]（70） | Dose = mg/kg × 体重 |

- 各入力は、0 以下だとキャンセルされます（Vd の Loading dose を除く）。
- Maintenance dose の結果は、**単位時間あたりの投与量**です。結果のラベルは `MD (/h)`、注記は `MD = CL*Css/F, per hour. x interval(h) = dose per interval` と表示されます。投与間隔あたりの 1 回量にするには、間隔 [h] を掛けてください。

例: Vd 50 L、目標 10 mg/L、F = 1 → LD = 500 mg

---

## 18.13 Epidemiology & Statistics（疫学・統計）

パス: `Medical → 2`

このメニューは、Statistics モードの機能を医学向けに呼び出す入口です。操作の詳細は、第8章・第9章を参照してください。

| 項目 | 内容 |
|---|---|
| 1 Survival Analysis | Kaplan-Meier、Log-rank（2 群）、Cox PH（共変量 1 つ）、Median survival |
| 2 Meta-Analysis | Inverse variance（固定/ランダム）、DerSimonian-Laird、Forest plot、Funnel plot、Egger's test |
| 3 Outbreak / Source Trace | 食中毒などの原因食の探索（暴露リストとアウトカムリストを比較） |
| 4 Risk / Odds Ratio (2x2) | OR + 95% CI、RR + 95% CI、RD + 95% CI、NNT |
| 5 Confidence Intervals | 平均の CI、割合の CI、ブートストラップ CI（平均/中央値） |

### 18.13.1 Survival Analysis

データはリストに入れておきます（**L1 = 時間、L2 = イベント（0/1）**）。リストが条件を満たさない場合は `L1=time L2=event(0/1)` と表示されます。

### 18.13.2 Meta-Analysis

最初に研究数（2〜50）を尋ねられ、続けて各研究の効果量 `y[i]` と分散 `var[i]` を入力します。範囲外の研究数は `2..50` と表示されます。

### 18.13.3 Outbreak / Source Trace

1. 暴露リスト（0/1 の二値）を、事前にリストへ入力します。
2. アウトカムリスト（0/1）を選びます。全リストの長さは同じである必要があります。
3. 各暴露とアウトカムの 2×2 表を作り、Fisher の正確確率検定（両側）などで関連を評価します。

アウトカムの件数が 4 未満だと `Outcome too small`、0/1 以外が含まれると `Outcome not 0/1` と表示されます。

### 18.13.4 Risk / Odds Ratio と Confidence Intervals

2×2 表（a/b/c/d）または平均・割合の値を入力して、点推定と 95% 信頼区間を求めます。

---

## 18.14 エラーメッセージ一覧（本章の範囲）

| メッセージ | 原因 | 対処 |
|---|---|---|
| `E1-4 V1-5 M1-6` | GCS の入力範囲外 | Eye 1〜4、Verbal 1〜5、Motor 1〜6 |
| `0/1 only` | CURB-65・qSOFA に 0/1 以外を入力 | 0 か 1 で入力 |
| `1-3 each` | Child-Pugh に 1〜3 以外を入力 | 各項目を 1〜3 の点数で入力 |
| `DBP must be <= SBP` | 拡張期血圧が収縮期血圧より大きい | SBP と DBP の入力順を確認 |
| `Bad date` | 存在しない日付 | 年・月・日を確認 |
| `Negative days` | 今日が LMP より前 | 日付を確認 |
| `L1=time L2=event(0/1)` | 生存分析のリストが不適切 | L1 に時間、L2 に 0/1 を同じ件数入力 |
| `2..50` | メタアナリシスの研究数が範囲外 | 2〜50 で入力 |
| `Outcome too small` | アウトカムが 4 件未満 | 件数を増やす |
| `Outcome not 0/1` | アウトカムに 0/1 以外が含まれる | 0/1 のみにする |
| `malloc fail` | メモリ確保失敗 | データ件数を減らす、再起動 |

その他のエラーは、第14章を参照してください。

---

## 18.15 この章のまとめ

- メインメニューの `4` から Medical Calculator に入り、Clinical Calculators（10 領域）と Epidemiology & Statistics（5 項目）を選びます。
- 臨床計算は、入力 → 結果 ＋ 判定コメントの流れです。結果画面で `Y`（Ans へ）、`S`（BMP 保存）が使えます。
- FiO2 は 0〜1 の小数、Child-Pugh は 1〜3 の点数で入力します。
- Winter's formula は予測範囲に加え、測定 pCO2 との比較判定（+resp alk／+resp acid／Appropriate）を注記に表示します。
- IBW と ABW は `Female? (0/1):` の入力で男女別の Devine 式を使い、`Ans` には選択した性別の値が入ります。
- Maintenance dose は単位時間あたりの量で、ラベルは `MD (/h)`、間隔 [h] を掛けると 1 回量になります。
- 本モードは教育・参考用途です。臨床判断の代わりにはなりません。

### 確認問題

Q18-1: 医学計算モードに入るには？ → メインメニューで `4`（Medical Calculator）

Q18-2: SBP 120、DBP 80 の MAP は？ → 93.3

Q18-3: P/F ratio で FiO2 はどう入力する？ → 0〜1 の小数（例: 0.21）

Q18-4: Child-Pugh では何を入力する？ → 検査値ではなく、各項目の 1〜3 の点数

Q18-5: LMP 2024-01-01 の分娩予定日は？ → 2024-10-07（LMP + 280 日）

Q18-6: Cockcroft-Gault で女性の場合は？ → 結果に 0.85 を掛ける

Q18-7: Winter's formula で表示される予測 pCO2 の式は？ → 1.5 × HCO3 + 8（範囲は +6〜+10）

Q18-8: 生存分析のデータの置き方は？ → L1 に時間、L2 にイベント（0/1）

---

[← 第17章 付録](17_appendix.md) | [目次](#/) | [第19章 ビルドと書き込み →](19_build.md)
