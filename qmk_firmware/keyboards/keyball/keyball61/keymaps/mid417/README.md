# Keyball61 ファームウェア（カスタマイズ）

[かみだい](https://twitter.com/d_kamiichi)様のファームウェアを現時点の最新QMKファームウェアに適用し、
[takashicompany](https://zenn.dev/takashicompany/articles/69b87160cda4b9)様のファームウェア参考に、私好みに変更させていただいています。

QMK Firmwareのビルド環境がない人は [Releases](https://github.com/mid417/keyball/releases)  からダウンロードし、
Pro Micro Web Update などで書き込んでください。

## 特徴（かみだい様、takashicompany様のFWとの違い）

- OLED系の変更
    - マウスレイヤー時は、OLEDのLayer行とサブキーボードのロゴを白黒反転
    - OLEDの回転方向を左右どちらがマスターになっても外側を向くように変更
    - OLEDレイヤー行の表示方法を変更（現在のレイヤーが白黒反転、マウスレイヤー時は１行反転）
- スクロールモード系の変更
    - レイヤー2の場合にスクロールモード
    - user3キーでスクロールモード(takashicompany様のFWと同じような使用感)
    - スクロールモード時の水平方向感度調整
- 音量・画面の明るさなどのEXTRAKEYを有効化
- マウスレイヤーの解放時間を短縮（1,000ms → 800ms）
- CPIのデフォルト値を1,100に設定（1kUp/1kDownで操作しやすいように） 

## RDPで修飾キー同時押し設定が抜ける問題の対応

- Remapなどで CTRL+S などを1キーに設定していた場合、RDP側にうまく届いたりCTRLなどの修飾キーが抜けて伝達される場合がありました。  
  これに対応するため、QMK Firmware本家のIssueである以下を適用すると改善を確認できましたので取り入れています。
    - <https://github.com/qmk/qmk_firmware/pull/19405>
- 自分でビルドする人は、添付の `action_util.c`を差し替えてビルドしてください。  
※公開時点では未テスト → 私の環境ではうまくいっている模様

## その他

- 当ファームウェア、ソースコードを使用したことでの不利益や故障などは責任は負いかねます。
