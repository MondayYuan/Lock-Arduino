#include <Servo.h>
#include <SPI.h>
#include <RFID.h>
#include <IRremote.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

//finger
SoftwareSerial mySerial(2, 3);//(Rx, Tx)

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

const int ledPin=7;//led灯 用于测试
const int RECV_PIN = 8; // 红外一体化接收头
const int touch=A5;//触摸开关

//舵机
Servo myservo;

//红外
IRrecv irrecv(RECV_PIN);
decode_results results;

const int openPos=80;//开门角度
const int closePos=15;//关门角度

int pos=closePos;

const int servoPin=6;

//rc522接口
#define RST_PIN 9
#define SS_PIN  10
//////////////////////////////////
//SDA-digital 10
//SCK-digital 13
//MOSI-digital 11
//MISO-digital 12
//IRQ-不用连接
//RST-digital 9
/////////////////////////////////
//D10 - 读卡器CS引脚、D5 - 读卡器RST引脚
RFID rfid(SS_PIN,RST_PIN);   
unsigned char status;
unsigned char str[MAX_LEN];  //MAX_LEN为16，数组最大长度

//卡号库
const String ZhouYiyuan="1137921344";
const String LiWei="9311417510 ";
const String LiMengshan="83748389";
const String LiYufan="5351171459";
const String transportation_card_zyy="71438126107";

const int num_cards = 5;
const String total_cards[]={ZhouYiyuan, LiWei, LiMengshan, LiYufan, transportation_card_zyy};

String cardNumber;//储存卡号
char bluetoothVal;//储存蓝牙接收的信号

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  // set the data rate for the sensor serial port
  finger.begin(57600);
  
  //if (finger.verifyPassword()) {
    //Serial.println("Found fingerprint sensor!");
  //}
  
  SPI.begin();
  rfid.init(); //初始化
  myservo.attach(servoPin);
  pinMode(ledPin,OUTPUT);
  irrecv.enableIRIn(); // 初始化红外解码
}

void loop()
{
  fingerDetect();
  rfidCard();
  touchSwitch();
  IRremote();
  bluetooth();
}

//控制舵机
void openDoor()
{
  Serial.println("open door");
  myservo.attach(servoPin);
  for(pos=closePos;pos<=openPos;++pos)
  {
    myservo.write(pos);
    delay(15);
  }

  delay(500);

  for(pos=openPos;pos>=closePos;--pos)
  {
    myservo.write(pos);
    delay(15);
  }
  
  myservo.detach();
}

//led开
void ledOn()
{
  digitalWrite(ledPin,HIGH);
  Serial.println("Led ON");
}

//led关
void ledOff()
{
  digitalWrite(ledPin,LOW);
  Serial.println("Led OFF");
}

//校园卡刷卡模块
void rfidCard()
{
    //Search card, return card types
  if (rfid.findCard(PICC_REQIDL, str) == MI_OK) {
    Serial.println("Find the card!");
    // Show card type
    ShowCardType(str);
    //防冲突检测,读取卡序列号
    if (rfid.anticoll(str) == MI_OK) {
      Serial.print("The card's number is  : ");
      //显示卡序列号
      char number[10];
      int n;
      for(int i = 0; i < 4; i++){
        //Serial.print(0x0F & (str[i] >> 4));
        sprintf(number,"%d",0x0F & (str[i] >> 4));
        cardNumber+=number;
        //Serial.print(0x0F & str[i]);
        sprintf(number,"%d",0x0F & str[i]);
        cardNumber+=number;
      }
      //Serial.println("");
      Serial.println(cardNumber);
    }
    //选卡（锁定卡片，防止多数读取，去掉本行将连续读卡）
    rfid.selectTag(str);

    bool find_card = false;
    for(int i=0;i < num_cards; ++i)
    {
      if(cardNumber==total_cards[i])
      {
        find_card = true;
        break;
      }
    }

    if(find_card)
    {
      Serial.println("Hello,my host.");
      ledOn();
      openDoor();
      ledOff();
    }
    else
       Serial.println("Sorry,you are not authorized");
  }
  rfid.halt();  //命令卡片进入休眠状态
  cardNumber="";
}

void ShowCardType(unsigned char * type)
{
  Serial.print("Card type: ");
  if(type[0]==0x04&&type[1]==0x00) 
    Serial.println("MFOne-S50");
  else if(type[0]==0x02&&type[1]==0x00)
    Serial.println("MFOne-S70");
  else if(type[0]==0x44&&type[1]==0x00)
    Serial.println("MF-UltraLight");
  else if(type[0]==0x08&&type[1]==0x00)
    Serial.println("MF-Pro");
  else if(type[0]==0x44&&type[1]==0x03)
    Serial.println("MF Desire");
  else
    Serial.println("Unknown");
}

//触摸开关模块
void touchSwitch()
{
  if(digitalRead(touch))
  {
    Serial.println("Be touched");
    ledOn();
    openDoor();
    ledOff();
    //delay(1000);
  }
}

//红外模块
void IRremote(){
if (irrecv.decode(&results))
{
if(results.value == 0xFD00FF ||results.value == 0xFFA25D ) //接收到电源键按下的命令
{
    Serial.println("IRrecv Received");
    ledOn();
    openDoor();
    ledOff();
    //delay(1000);
}
irrecv.resume(); // 接收下一个编码
}
}

//蓝牙模块
void bluetooth(){
   bluetoothVal=Serial.read();
  if(bluetoothVal=='w')
{
    Serial.println("Bluetooth Received");
    ledOn();
    openDoor();
    ledOff();
    //delay(1000);
  } 
}

void fingerDetect()
{
  int id = getFingerprintIDez();
  if(id >= 0)
  {
    ledOn();
    openDoor();
    ledOff();
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID; 
}

