# Room Monitoring System

ESP32 + キーパッド + LCDで在室状況をWebに表示するシステム。

## システム構成

```
[ESP32 + Keypad + LCD] --(WiFi/HTTP)--> [Flask on Railway] <--(Browser)--> 閲覧者
```

## 配線

### LCD1602 (I2C)

| LCD | ESP32 |
|-----|-------|
| VCC | 5V    |
| GND | GND   |
| SDA | GPIO21|
| SCL | GPIO22|

### Membrane Keypad 4x4

| Keypad Pin | ESP32 GPIO | 役割   |
|-----------|------------|--------|
| 1         | GPIO13     | Row 1  |
| 2         | GPIO12     | Row 2  |
| 3         | GPIO14     | Row 3  |
| 4         | GPIO27     | Row 4  |
| 5         | GPIO26     | Col 1  |
| 6         | GPIO25     | Col 2  |
| 7         | GPIO33     | Col 3  |
| 8         | GPIO32     | Col 4  |

キーパッドは左から Pin1(Row1) ~ Pin8(Col4) の順。

## ESP32 セットアップ

### 必要なライブラリ (Arduino IDE Library Manager)

- `Keypad` by Mark Stanley
- `LiquidCrystal I2C` by Frank de Brabander
- `ArduinoJson` by Benoit Blanchon

### 手順

1. Arduino IDEでESP32ボードを追加（ボードマネージャーで `esp32` を検索してインストール）
2. 上記ライブラリをインストール
3. `esp32/room_monitor.ino` を開く
4. WiFi SSID/パスワードを書き換える
5. `SERVER_URL` をRailwayのURLに書き換える
6. ボードを `ESP32 Dev Module` に設定してアップロード

### 操作方法

| キー | 動作 |
|------|------|
| A    | ステータス切り替え + 分数入力モードへ |
| 0-9  | 分数を入力 |
| *    | 確定・送信 |
| #    | 1文字削除（バックスペース） |
| B    | キャンセル |

## Web (Railway) デプロイ

### ローカルテスト

```bash
cd web
pip install -r requirements.txt
python app.py
```

curlでテスト:

```bash
# ステータス送信
curl -X POST http://localhost:5000/api/status \
  -H "Content-Type: application/json" \
  -d '{"status": "in_room", "minutes": 30}'

# ステータス確認
curl http://localhost:5000/api/status
```

ブラウザで http://localhost:5000 を開いて表示確認。

### Railway デプロイ

1. [Railway](https://railway.app) にログイン
2. `New Project` → `Deploy from GitHub repo` またはCLIでデプロイ
3. Root Directoryを `web` に設定
4. 自動でビルド・デプロイされる
5. 生成されたURLを ESP32 の `SERVER_URL` に設定
