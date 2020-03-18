#include <M5StickC.h>

#define MODE_A 0
#define MODE_B 1
#define stat_OK 0
#define stat_NG 1
uint8_t disp_mode = MODE_A;

#define BTN_A_PIN 37
#define BTN_ON LOW
#define BTN_OFF HIGH
uint8_t prev_btn_a = BTN_OFF;
uint8_t btn_a = BTN_OFF;

#define POSE_P_X 0
#define POSE_M_X 1
#define POSE_P_Y 2
#define POSE_M_Y 3
#define POSE_P_Z 4
#define POSE_M_Z 5
uint8_t pose = POSE_P_X;
uint8_t prev_pose = POSE_P_X;

int mihari_count = 0;
uint8_t mihari_mode = stat_OK;

// 加速度  センサで取得できる値の単位は[g]なので、通常の[m/s^2]単位で考えるなら9.8倍する
float accX_g = 0;
float accY_g = 0;
float accZ_g = 0;
float accX_mpss = 0;
float accY_mpss = 0;
float accZ_mpss = 0;
float prev_accX_mpss = 0;
float prev_accY_mpss = 0;
float prev_accZ_mpss = 0;
float accX_diff = 0;
float accY_diff = 0;
float accZ_diff = 0;

// 角速度
float gyroX_dps = 0;
float gyroY_dps = 0;
float gyroZ_dps = 0;

// SPEAKER HAT。
const int servo_pin = 26;
int freq = 50;
int ledChannel = 0;
int resolution = 10;
extern const unsigned char m5stack_startup_music[];

///////////////////////////////////////////////////////////////

void setup()
{
  M5.begin();
  pinMode(BTN_A_PIN, INPUT_PULLUP);
  // 6軸センサ初期化
  M5.MPU6886.Init();
  // LCD display
  M5.Lcd.setRotation(1); // ボタンBが上になる向き
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);

  // SPEAKER 初期化
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(servo_pin, ledChannel);
  ledcWrite(ledChannel, 256); //0°

  // 内臓LED 初期化
  pinMode(GPIO_NUM_10, OUTPUT);
  digitalWrite(GPIO_NUM_10, HIGH);
}

///////////////////////////////////////////////////////////////

void playMusic(const uint8_t *music_data, uint16_t sample_rate)
{
  uint32_t length = strlen((char *)music_data);
  uint16_t delay_interval = ((uint32_t)1000000 / sample_rate);
  for (int i = 0; i < length; i++)
  {
    ledcWriteTone(ledChannel, music_data[i] * 50);
    delayMicroseconds(delay_interval);
  }
}

///////////////////////////////////////////////////////////////

void loop()
{
  btn_a = digitalRead(BTN_A_PIN);

  if (prev_btn_a == BTN_OFF && btn_a == BTN_ON)
  {
    M5.Lcd.fillScreen(BLACK);
    if (disp_mode == MODE_A)
    {
      disp_mode = MODE_B;
      M5.Lcd.setTextSize(1);
    }
    else
    {
      disp_mode = MODE_A;
      M5.Lcd.setTextSize(1);
      M5.Lcd.setRotation(1); // ボタンBが上になる向き
    }
  }

  prev_btn_a = btn_a;

  // 加速度取得
  M5.MPU6886.getAccelData(&accX_g, &accY_g, &accZ_g);
  accX_mpss = accX_g * 9.8;
  accY_mpss = accY_g * 9.8;
  accZ_mpss = accZ_g * 9.8;

  // 角速度取得
  M5.MPU6886.getGyroData(&gyroX_dps, &gyroY_dps, &gyroZ_dps);

  if (disp_mode == MODE_A)
  {
    // ============ MODE_A ============
    ledcWriteTone(ledChannel, 0);

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setRotation(1);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.print(" No watched...");
    M5.Lcd.setCursor(0, 35);
    M5.Lcd.print(" Please press button A");
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.print("  to MIHARI mode !!");

    mihari_count = 0;
    mihari_mode = stat_OK;
  }
  else
  {
    // ============ MODE_B ============
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.print("    - M I H A R I -");

    // 取得した値を表示する
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("Acc1 : %.2f  %.2f  %.2f   ", accX_mpss, accY_mpss, accZ_mpss);
    // M5.Lcd.setCursor(0, 45);
    // M5.Lcd.printf("Acc2 : %.2f  %.2f  %.2f   ", prev_accX_mpss, prev_accY_mpss, prev_accZ_mpss);

    // 前回の値との差を取得
    accX_diff = prev_accX_mpss - accX_mpss;
    accY_diff = prev_accY_mpss - accY_mpss;
    accZ_diff = prev_accZ_mpss - accZ_mpss;

    M5.Lcd.setCursor(0, 55);
    M5.Lcd.printf("Acc3 : %.2f  %.2f  %.2f   ", accX_diff, accY_diff, accZ_diff);

    // 判定
    if (accX_diff >= 2)
      mihari_count++;
    if (accX_diff <= -2)
      mihari_count++;
    if (accY_diff >= 2)
      mihari_count++;
    if (accY_diff <= -2)
      mihari_count++;
    if (accZ_diff >= 2)
      mihari_count++;
    if (accZ_diff <= -2)
      mihari_count++;

    // 値をprevに格納
    prev_accX_mpss = accX_mpss;
    prev_accY_mpss = accY_mpss;
    prev_accZ_mpss = accZ_mpss;

    if (mihari_count >= 3)
      mihari_mode = stat_NG;

    if (mihari_mode == stat_NG)

      ledcWriteTone(ledChannel, 1250);
    digitalWrite(GPIO_NUM_10, LOW);
    delay(800);
    ledcWriteTone(ledChannel, 0);
    digitalWrite(GPIO_NUM_10, HIGH);
    delay(400);
  }
  delay(800);
}