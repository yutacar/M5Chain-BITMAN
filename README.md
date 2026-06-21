# M5Chain-BITMAN

M5Stack Chain Mono上で、明和電機のBITMANに着想を得た8×8キャラクターを動かすオープンソース実装です。現在はAtomS3RからChain Monoを直接制御します。

アニメーション、傾き判定、デモシーケンスはArduino非依存の`BITMAN Core`に分離され、PCで単体テストできます。コントローラーAdapterを差し替えることで、M5Stack Coreシリーズやｽﾀｯｸﾁｬﾝなどへの移植を想定しています。

人物の基準ビットマップは[明和電機の公式BITMAN](https://www.maywadenki.com/products/goods/bitman/)掲載写真に合わせ、2×2の頭、細い胴、胴から離れた腕、長い脚と2ドットの足で構成しています。屈伸・移動・ジャンプなどの派生コマも、この共通骨格から作成しています。

## ハードウェア

- AtomS3R
- Chain Mono
- HY2.0-4PケーブルまたはChain Bridge
- USB-C電源

AtomS3RのHY2.0-4PとChain Monoの**入力側**を接続します。Chain Monoの矢印がAtomS3Rから外向きになることを確認してください。

| AtomS3R | Chain Mono |
|---|---|
| GND | GND |
| 5V | 5V |
| G2 (UART TX) | RX |
| G1 (UART RX) | TX |

> Chain Monoには入力側と出力側があります。逆向きに接続すると表示できません。電源を切った状態で配線してください。

## 操作

- シングルクリック: デモ再生
- ダブルクリック: デモ再生
- 長押し: Chain Monoの明るさ変更

通常はBMI270が示す重力方向に応じて、8×8の下・右・上・左の辺を地面として扱います。回転中は走らず、次の地面へ片脚を掛けるまたぎ動作で移動します。静止中は上下の屈伸に加え、腕振り、左右ステップ、ジャンプ、腕上げ、逆立ち風の短い動作を2.6～5.6秒のランダムな間隔で再生します。

地面が切り替わると、旧地面の中央から角へ寄り、片脚を次の辺へ掛け、体重を移して新しい地面へ立つ8コマ・約2.1秒の遷移を再生します。専用コマと待機動作は[BITMANの実演動画](https://www.youtube.com/watch?v=gI0ZVmFz6mQ)を参考にしています。

デモは、最初の屈伸から時計回りに四辺を一周し、各地面で屈伸してから、従来と同じ収縮する四角形のアニメーションで終了します。

## PCテスト

必要環境: CMake 3.16以上、C++17対応コンパイラ。

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

通常動作と一回転デモのASCIIプレビュー:

```bash
./build/bitman_preview
```

PCテストの対象:

- 8x8ビット順と左右反転
- スプライトカタログ
- 下／右／上／左の地面分類
- 地面切替のヒステリシス
- 回転中のまたぎ動作と静止時のランダム待機動作
- 8×8フレームの90度回転
- 一回転デモ、四角形の終了演出、ダンスモードへの復帰
- IMU入力から8x8出力までの統合パイプライン

## AtomS3Rファームウェア

必要環境: [PlatformIO Core](https://platformio.org/install/cli)。依存ライブラリは初回ビルド時に取得されます。

```bash
pio run -e m5stack-atoms3r
```

書き込み:

```bash
pio run -e m5stack-atoms3r -t upload --upload-port /dev/cu.usbmodemXXXX
```

書き込みはmacOSのNative USB CDCでRAM stubへの切替時に通信が切れる構成へ対応するため、115200bpsのROMブートローダー直接書き込み（`--no-stub`）に設定しています。`Connecting...`で停止する場合は、AtomS3Rのリセットを約2秒長押しし、内部の緑LEDが点灯したら離してから再実行してください。

## 構造

```text
include/bitman/             Arduino非依存インターフェース
src/core/                   BITMAN Core
src/controllers/            コントローラー別Adapter
src/drivers/                Chain Monoドライバー
tests/                      PC単体テスト
tools/preview.cpp           PC用ASCIIプレビュー
```

別コントローラーへ移植するときは`IBitmanController`を実装し、UARTピン設定を差し替えます。BITMAN Coreとスプライトは変更しません。

詳しい責務分割と移植手順は[アーキテクチャ資料](docs/ARCHITECTURE.md)を参照してください。

## 対応状況

| コントローラー | 状態 |
|---|---|
| AtomS3R | 対応済み |
| M5Stack Core Basic | Adapter未実装 |
| M5Stopwatch | Adapter未実装 |
| ｽﾀｯｸﾁｬﾝ | Adapter未実装 |

## ライセンスとクレジット

ソースコードは[MIT License](LICENSE)で公開します。貢献方法は[CONTRIBUTING.md](CONTRIBUTING.md)を参照してください。

BITMANは明和電機とクワクボリョウタ氏により共同開発された製品です。本リポジトリは非公式のファンによるM5Stack向け実装であり、権利者による公式プロジェクトではありません。詳細は[NOTICE.md](NOTICE.md)を参照してください。
