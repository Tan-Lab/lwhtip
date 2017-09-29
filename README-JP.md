# lwhtip README-JP

# 概要
lwhtipは、TTC標準JJ-300.00v3(以下HTIP)をC言語で実装したソフトウェアです。
本ソフトウェアは、ホームネットワークの各端末上で実行することでネットワークトポロジを推定するための情報を送信・転送します。
lwhtipは、HTIP L2Agent・HTIP NW機器の２つのプログラムとライブラリにより構成されます。
HTIP L2Agentは、ホームネットワーク中の機器の機器情報を定期的に送信します。
HTIP NW機器はネットワーク中のレイヤ２スイッチにインストールすることで、レイヤ２スイッチのMACアドレステーブルを定期的に送信します。
これらの情報を収集し解析することでネットワークトポロジを推定することが可能となります。

# 要件
lwhtipは以下の要件を満たすコンピュータ上でコンパイル及び実行可能です。

* Linuxオペレーティングシステムで動作すること
* Cコンパイラ・autotoolsをインストールし実行可能であること
* (HTIP NW機器) brctlをインストールし実行可能であること

# インストール
lwhtipはターミナルアプリケーション上で以下のコマンドを実行することで利用できます。

        autoreconf -i
        ./configure
        make
        make install

# 利用方法
lwhtipはHTIP L2AgentとHTIP NW機器の２つのプログラムで構成されます。

## HTIP L2Agent
HTIP L2Agentは、実行ホストの機器情報を定期的にブロードキャストします。
利用する際は以下のコマンドを実行してください。

        l2agent

コンパイルのみでインストールしない場合パスを指定して実行してください。

        src/bin/l2agent

## HTIP L2Agent (HTIP-NW機器)
HTIP-NW機器(L2スイッチ)上のHTIP L2Agentは、実行ホストの機器情報と接続構成情報を定期的にブロードキャストします。
利用する前に brctl コマンドでブリッジを作成してください。
例えば br0 というブリッジインタフェースを作成し、en0とen1のネットワークインタフェースをブリッジする場合、以下のコマンドを実行してください。

        brctl addbr br0
        brctl addif br0 en0
        brctl addif br0 en1

ブリッジインタフェースを作成した後以下のコマンドを実行しHTIP L2Agent(HTIP-NW機器)を開始してください。

        l2switch -i br0

brctl及びl2switchはブリッジインタフェースを操作するため、root権を持つユーザーで実行してください。
動作しない場合は、brctlコマンドで作成した br0 ブリッジインタフェースの情報やブリッジインタフェースがActiveになっているか、などを確認してください。

# ドキュメント
README.md、README-jp.md、Doxygenにより生成されるdocディレクトリ以下を参照してください。
Doxygenによるドキュメント作成はDoxygenをインストールし以下のコマンドをターミナルアプリケーション上で実行してください。

        doxygen Doxyfile

# ライセンス
MITライセンス, Copyright (c) 2017 Takashi OKADA.