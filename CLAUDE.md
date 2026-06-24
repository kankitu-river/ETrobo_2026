# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要

ETrobo 2026 は LEGO SPIKE Prime を使ったライントレースロボットの制御プログラム群です。
各サンプルは ETrobo フレームワークの親 Makefile にインクルードされる形でビルドされます（単体ビルド不可）。

## サンプル一覧

| ディレクトリ | 目的 |
|---|---|
| `sample_c5/` | ライントレース走行本体。フォースセンサを押すと走行開始 |
| `sample_hsv/sample_hsv/` | カラーセンサの HSV 値を USB 経由でテラタームに表示するデバッグツール（2秒ごと） |
| `sample_refle/sample_refle/` | カラーセンサの反射光値（`ref=`）をテラタームにリアルタイム表示するデバッグツール（100ms ごと） |

デバッグツール 2 種は走行しない。センサ値の確認専用。

## ビルドシステム

各サンプルの `Makefile.inc` を ETrobo 親フレームワークがインクルードしてビルドします。
このリポジトリ単体ではビルドできません。テストやCIも存在しません。

`Makefile.inc` の主な変数:

- `APPL_COBJS` — コンパイル対象のオブジェクトファイル
- `APPL_DIRS` — 追加ソースディレクトリ
- `APPL_LIBS` — リンクするライブラリ（`-lm` など）
- `INCLUDES` — ヘッダ検索パス

## アーキテクチャ

### ランタイムスタック

| 層 | 技術 |
|---|---|
| ハードウェア抽象 | Pybricks SPIKE API（`spike/pup/motor.h` / `colorsensor.h` / `forcesensor.h`） |
| RTOS | TOPPERS カーネル（タスク・サイクリックハンドラ API） |
| コンポーネント | TECS（`app.cdl` で定義、`tecsgen.cfg` を生成） |
| カーネル設定 | `app.cfg`（タスク・サイクリックハンドラ・スタックサイズを定義） |

### ログ出力の注意

この環境では `printf` は USB に出力されない場合がある。`syslog(LOG_NOTICE, ...)` を使ってテラタームに出力する。

### sample_c5 のタスク構成

`app.cfg` で定義される 2 タスク:

- **MAIN_TASK**（優先度 5）: `main_task()` — フォースセンサ待ち → `LineTracer_Configure()` → サイクリックハンドラ起動 → タスク終了
- **LINE_TRACER_TASK**（優先度 6、100ms 周期）: `tracer_task()` — カラーセンサ読み取りとモータ差動制御

### ハードウェアポート（sample_c5）

| ポート | デバイス |
|---|---|
| E | カラーセンサ（輝度入力） |
| B | 左モータ |
| A | 右モータ |
| D | フォースセンサ（スタートトリガ） |

HSV / 反射光サンプルはカラーセンサ（ポート E）のみ使用。

## ライントレースアルゴリズム（sample_c5）

`LineTracer/LineTracer.h` のパラメータで挙動を調整する:

```c
#define WHITE_BRIGHTNESS  (80)   // 白の輝度
#define BLACK_BRIGHTNESS  (10)   // 黒の輝度
#define STEERING_COEF     (0.4F) // 操舵係数（大きくするほど急旋回）
#define BASE_SPEED        (30)   // 走行基準スピード（0–100）
#define LEFT_EDGE         (1)    // 左エッジ追従
#define RIGHT_EDGE        (1)    // 右エッジ追従
```

目標輝度 = `(WHITE_BRIGHTNESS + BLACK_BRIGHTNESS) / 2 = 45`

```
steering_amount = (目標輝度 - 現在輝度) × STEERING_COEF
左モータ出力   = BASE_SPEED - steering_amount
右モータ出力   = BASE_SPEED + steering_amount
```

## テラタームでセンサ値を確認する手順

1. SPIKE と PC を USB ケーブルで接続
2. テラタームを起動 → 「新しい接続」→「シリアル」→ SPIKE の COMポートを選択
3. 「設定」→「シリアルポート」→ ボーレートを **115200** に設定
4. SPIKE のスタートボタンを押す
