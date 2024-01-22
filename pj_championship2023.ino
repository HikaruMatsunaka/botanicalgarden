#include <Wire.h> // I2C通信に関するライブラリをインクルード
#include <MPU6050.h> // MPU6050ジャイロセンサーライブラリをインクルード
#include <SoftwareSerial.h> // ソフトウェアシリアル通信ライブラリをインクルード
#include <DFRobotDFPlayerMini.h> // DFPlayer Miniのライブラリをインクルード
#include <avr/wdt.h> // ウォッチドッグタイマーライブラリをインクルード


MPU6050 mpu; // MPU6050クラスのインスタンスを作成

SoftwareSerial mySoftwareSerial(10, 11); // RXをピン10、TXをピン11に設定する
DFRobotDFPlayerMini myDFPlayer; // DFPlayer Miniのクラスのインスタンスを作成

// ジャイロセンサーからの読み取り値を保存する変数
int16_t ax, ay, az;
int16_t gx, gy, gz;
bool isPlaying = false; // 音が再生中かどうかの状態を保持するフラグ
unsigned long previousMillis = 0; // 前回の読み取り時間を記録
const long interval = 2000; // 読み取り間隔を2秒(2000ミリ秒)に設定

void setup() {
  randomSeed(analogRead(0));
  mySoftwareSerial.begin(9600); // DFPlayer Miniとの通信を9600bpsで開始
  Serial.begin(9600); // シリアル通信を開始
  Wire.begin(); // I2C通信を開始
  mpu.initialize(); // MPU6050センサーを初期化

  if (!myDFPlayer.begin(mySoftwareSerial)) { // DFPlayer Miniの初期化
    Serial.println("DFPlayer Mini not detected!"); // DFPlayer Miniが検出されなかった場合のエラーメッセージ
    while (true); // 何もしないループに入る
  }

  myDFPlayer.volume(30); // 音量を10に設定(範囲は0から30)
  myDFPlayer.play(1); // 初期化のために最初のトラックを再生
  Serial.println("Setup complete!");
  wdt_enable(WDTO_8S);  // ウォッチドッグタイマーを8秒で設定
  int trackNumber = random(1, 11); // 1から10までのランダムな数を生成

}

void loop() {
  wdt_reset();   // ウォッチドッグタイマーをリセット
  unsigned long currentMillis = millis(); // 現在の時間を取得

  // 前回の読み取りから3秒以上経過しているか確認
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // 現在の時間を記録

    // センサーからのデータを読み込む
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // Roll角を計算(ラジアンから度に変換)
    float roll = atan2(ax, az) * 180.0 / PI;

    // センサーのX軸の値をシリアルモニターに表示
    Serial.print("roll: ");
    Serial.println(roll);
    wdt_reset(); 

    // Roll角が45度を超えたかどうかをチェック
    if (roll > 30 && !isPlaying) {
      Serial.println("Attempting to play track...");
      delay(1500);
      int trackNumber = random(1, 10); // 1から10までのランダムな数を生成
      myDFPlayer.play(trackNumber); // ランダムに選ばれたトラックを再生
      delay(1000); // コマンドが実行されるのを待つ
      Serial.println("Playback started.");
      wdt_reset(); 
      isPlaying = true;
    } else if (roll < 10 && isPlaying) {
      // Roll角が15度未満になったら、再生可能な状態にリセット
      Serial.println("Playback stopped.");
      isPlaying = false;
      wdt_reset(); 
    }
  }
  // 他の処理...
}
