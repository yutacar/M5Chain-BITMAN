# Contributing

IssueやPull Requestを歓迎します。変更は、ハードウェア非依存のCoreと機種固有Adapterの境界を保ってください。

## 開発手順

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/bitman_preview
```

AtomS3R向けの確認:

```bash
pio run -e m5stack-atoms3r
```

## Pull Requestの条件

- 既存テストがすべて成功すること
- Coreの変更にはPC単体テストを追加または更新すること
- ハードウェアAPIを`include/bitman`および`src/core`へ持ち込まないこと
- 新しいコントローラーは`IBitmanController`を実装すること
- 動作や配線が変わる場合はREADMEも更新すること

実機を使えない変更では、その旨をPull Requestへ記載してください。
