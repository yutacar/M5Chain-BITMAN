# M5Chain-BITMAN

M5Stack Chain Mono上で、明和電機のBITMANに着想を得た8×8キャラクターを動かすオープンソース実装です。現在はAtomS3R単体、またはAtomic Motion Base v1.2に載せたAtomS3RからChain Monoを制御します。

アニメーション、傾き判定、デモシーケンスはArduino非依存の`BITMAN Core`に分離され、PCで単体テストできます。コントローラーAdapterを差し替えることで、M5Stack Coreシリーズやｽﾀｯｸﾁｬﾝなどへの移植を想定しています。

人物の基準ビットマップは[明和電機の公式BITMAN](https://www.maywadenki.com/products/goods/bitman/)掲載写真に合わせ、2×2の頭、細い胴、胴から離れた腕、長い脚と2ドットの足で構成しています。屈伸・移動・ジャンプなどの派生コマも、この共通骨格から作成しています。

## ハードウェア

- AtomS3R
- Atomic ToChain Base
- Atomic Motion Base v1.2
- Chain Mono
- HY2.0-4PケーブルまたはChain Bridge
- USB-C電源

### Atomic ToChain Base（推奨構成）

AtomS3RをAtomic ToChain Baseへ装着し、ToChain Baseのオス型HY2.0-4PコネクターをChain Monoの**入力側**へ接続します。購入時の既定は次の配線です。

| Atomic ToChain Base | AtomS3R | Chain Mono入力側 |
|---|---|---|
| GND | GND | GND |
| VCC | 5V | 5V |
| IO1 | G5 (UART TX) | RX (IO1) |
| IO2 | G6 (UART RX) | TX (IO2) |

標準のPlatformIO環境`m5stack-atoms3r`は、このToChain Baseを最初に探索します。内部のはんだパッドを変更した個体では、実際の配線に合わせて固定環境のGPIO設定を変更してください。

### Atomic Motion Base v1.2 PORT B

AtomS3RをAtomic Motion Base v1.2へ装着し、黒い**PORT B**とChain Monoの**入力側**をHY2.0-4Pケーブルで接続します。Chain Monoの矢印がAtomic Motion Baseから外向きになることを確認してください。

| Atomic Motion Base v1.2 PORT B | AtomS3R | Chain Mono |
|---|---|---|
| 黒 | GND | GND |
| 赤 | 5V | 5V |
| 黄 | G8 (UART TX) | RX (IO1) |
| 白 | G7 (UART RX) | TX (IO2) |

Atomic Motion Base v1.2の電源スイッチをONにして使用してください。標準のPlatformIO環境`m5stack-atoms3r`は、ToChain Baseが見つからなかった場合にこのPORT Bを探索します。

### AtomS3R直結（従来構成）

従来のG1/G2直結を使用する場合は、次の配線と`m5stack-atoms3r-direct`環境を使用します。

| AtomS3R | Chain Mono |
|---|---|
| GND | GND |
| 5V | 5V |
| G2 (UART TX) | RX |
| G1 (UART RX) | TX |

> Chain Monoには入力側と出力側があります。逆向きに接続すると表示できません。電源を切った状態で配線してください。PORT Bは黒、PORT Cは青なので取り違えにも注意してください。

## 操作

- シングルクリック: デモ再生
- ダブルクリック: デモ再生
- 長押し: Chain Monoの明るさ変更

通常はBMI270が示す重力方向に応じて、8×8の下・右・上・左の辺を地面として扱います。回転中は走らず、次の地面へ片脚を掛けるまたぎ動作で移動します。静止中は上下の屈伸に加え、腕振り、左右ステップ、ジャンプ、腕上げ、逆立ち風の短い動作を2.6～5.6秒のランダムな間隔で再生します。

地面が切り替わると、旧地面の中央から角へ寄り、片脚を次の辺へ掛け、体重を移して新しい地面へ立つ8コマ・約2.1秒の遷移を再生します。専用コマと待機動作は[BITMANの実演動画](https://www.youtube.com/watch?v=gI0ZVmFz6mQ)を参考にしています。

デモは、最初の屈伸から時計回りに四辺を一周し、各地面で屈伸してから、従来と同じ収縮する四角形のアニメーションで終了します。

Chain Monoを複数連結した場合、通常動作では全パネルに同じBITMANを同期表示します。デモ中は連結順を横方向の仮想キャンバスとして扱います。BITMANは下辺を1枚目から最後のパネルまで進み、全身が端へ到達してから右辺・上辺へ回転します。続いて上辺を1枚目まで戻り、端で左辺・下辺へ回転して一周します。一周後は従来どおり、四角形の終了演出を全パネルへ同期表示します。物理配置も連結順に左から右へ並べてください。

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
- 複数パネル境界をまたぐ仮想キャンバス分割

## AtomS3Rファームウェア

必要環境: [PlatformIO Core](https://platformio.org/install/cli)。依存ライブラリは初回ビルド時に取得されます。

```bash
pio run -e m5stack-atoms3r
```

標準環境は起動時にAtomic ToChain Base（`RX=G6 / TX=G5`）、Atomic Motion Base v1.2 PORT B（`RX=G7 / TX=G8`）、AtomS3R直結（`RX=G1 / TX=G2`）の順で自動探索します。切断後の再接続では最後に成功した方式を先に試します。

接続方式を固定して検証する場合は、次の環境を使用します。

```bash
pio run -e m5stack-atoms3r-tochain
pio run -e m5stack-atoms3r-motion-v12
pio run -e m5stack-atoms3r-direct
```

AtomS3Rの画面には、探索中は`Mono scanning`、接続後は`Mono TOCHAIN`、`Mono PORT B`、または`Mono DIRECT`と表示されます。

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
