# Changelog

- Fixed the multi-panel demo lap so BITMAN reaches each end panel before the
  corner rotation, crosses the opposite edge, and returns to the start.

このプロジェクトの主な変更を記録します。

## [Unreleased]

- 実演動画を参考に、片脚を次の地面へ掛ける8コマのまたぎ動作を追加
- 静止中に腕振り、左右ステップ、ジャンプ、腕上げ、逆立ち風の動作をランダム再生
- 複数Chain Monoの通常同期表示と、デモ時のパネル間往復移動に対応
- 複数パネルのデモ移動を1枚目から終端へ進んで戻る一周に変更

## [0.1.0] - 2026-06-21

- Arduino非依存のBITMAN Coreを実装
- AtomS3R Controller Adapterを実装
- Chain Mono UARTドライバーを実装
- 重力方向の4辺判定とヒステリシスを実装
- 屈伸、地面切替、一回転デモ、四角形の終了演出を実装
- 公式BITMAN掲載写真を基準に人物スプライトを統一
- CMakeによるPC単体テストとASCIIプレビューを追加
- PlatformIOによるAtomS3Rビルドと書き込み設定を追加
