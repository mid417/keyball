# Keyball61 ファームウェア（カスタマイズ）

[かみだい](https://twitter.com/d_kamiichi) 様のファームウェアを現時点の最新QMKファームウェア、Keyballファームウェア（[Yowkees](https://github.com/Yowkees/keyball) 様）に適用し、
[takachicompany](https://zenn.dev/takashicompany/articles/69b87160cda4b9) 様のファームウェア参考に、私好みに変更させていただいています。

QMK Firmwareのビルド環境がない人は [Releases](https://github.com/mid417/keybball/releases)  からダウンロードし、
Pro Micro Web Update などで書き込んでください。

## 特徴

- 音量・輝度などのEXTRAKEYを有効化
- マウスレイヤー時は、OLEDのLayer行とサブキーボードのロゴを白黒反転
- user3キーで、スクロールモード  
- マウスレイヤーの解放時間を短縮（1,000ms → 800ms）
- CPIのデフォルト値を1,100に設定（1kUp/1kDownで操作しやすいように） 
- RDPで修飾キーが抜ける問題の対応
    - https://github.com/qmk/qmk_firmware/pull/19405 で上手く動作しているようなので、添付の `action_util.c`を差し替えてビルドしてください。

## その他

- 当ファームウェア、ソースコードを使用したことでの不利益や故障などは責任は負いかねます。
