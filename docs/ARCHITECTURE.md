# Architecture

M5Chain-BITMANは、アニメーションを再利用できるCoreと、機種固有の入出力を扱うAdapterに分かれています。

```text
Controller Adapter
  ├─ IMU sample
  ├─ button event
  └─ status output
          │
          ▼
     BITMAN Core
  ├─ MotionClassifier
  ├─ BitmanEngine
  ├─ Sprite catalog
  └─ Frame8
          │ 8×8 monochrome frame
          ▼
 Chain Mono Display Driver
```

## BITMAN Core

`include/bitman`と`src/core`はArduinoやM5Stack APIへ依存しません。

- `MotionClassifier`: 加速度とジャイロから地面の辺、回転方向、運動状態を判定
- `BitmanEngine`: 通常動作、地面切替、デモの時間軸を管理
- `Frame8`: 8×8フレームの回転、反転、移動を提供
- `sprites.cpp`: 人物と終了演出の基準ビットマップを保持
- `BitmanCore`: MotionClassifierとBitmanEngineを接続

## Controller Adapter

コントローラー固有処理は`IBitmanController`の実装へ閉じ込めます。現在の`AtomS3RController`は次を担当します。

- M5Unifiedの初期化
- BMI270からのIMU取得
- ボタンイベントの変換
- AtomS3R本体画面への状態表示

別コントローラーへ移植する場合は、Adapterを追加して`src/main.cpp`の生成部分とピン設定を差し替えます。BITMAN Coreの変更は不要です。

## Chain Mono Driver

`ChainMonoDisplay`はM5Chainライブラリを利用し、`Frame8`をChain Monoへ送信します。現在のAtomS3R配線はUART2のRX=G1、TX=G2です。

## PCテスト

PCビルドにはCoreだけを含めます。M5Unified、Arduino、実機IMUを使わずに次を検証できます。

- 8×8ビット演算
- スプライト
- 傾き判定とヒステリシス
- 地面切替アニメーション
- デモの時間軸
- IMU入力から表示フレームまでの統合経路
