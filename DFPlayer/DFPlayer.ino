#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

HardwareSerial mp3Serial(1);
DFRobotDFPlayerMini mp3;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Booting...");

  // GPIO16 = RX, GPIO17 = TX  （DFPlayer TX -> IO16, DFPlayer RX <- IO17）
  mp3Serial.begin(9600, SERIAL_8N1, 19, 23);
  Serial.println("Init DFPlayer...");
  delay(1000);  // 給 DFPlayer 上電時間

  if (!mp3.begin(mp3Serial)) {
    Serial.println("DFPlayer init failed!");
    Serial.println("Check: power, GND, IO16<-TX, IO17->RX, SD card.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("DFPlayer online.");

  // 測一下卡裡到底有沒有檔案
  uint16_t files = mp3.readFileCounts();
  Serial.print("Total files on SD: ");
  Serial.println(files);

  mp3.volume(25);   // 0~30，先開大一點
  Serial.println("Playing track 1...");
  mp3.play(1);      // 對應 0001.mp3
}

void loop() {
  // 持續印 DFPlayer 狀態
  static unsigned long last = 0;
  if (millis() - last > 2000) {
    last = millis();
    Serial.print("State = ");
    Serial.println(mp3.readState());     // 512/513 等等
    Serial.print("Current track = ");
    Serial.println(mp3.readCurrentFileNumber());
  }
}
