//02180201028 Beyazıt Bestami Sarıkaya - Güvenli Kasam

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <UniversalTelegramBot.h> // Telegram kütüphanesi: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/archive/master.zip
#include <ArduinoJson.h> //ArduinoJson kütüphanesi https://github.com/bblanchon/ArduinoJson
#define LDR 15 //Işık sensörünün portu

// Kablosuz Ağ Bilgileri
const char* ssid = "WIFI Adı";
const char* password = "WIFI Şifresi";

String BOTtoken = "**********"; // Telegram üzerinden alınan Token

String CHAT_ID = "********"; // Kişiye ait ID bilgisi
int openCount = 0; // Kasanın açılma sayısı
bool sendPhoto = false;

// Telegram bağlantısını kurma
WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

// Kamera için gerekli değişkenlerin oluşturulması
#define FLASH_LED_PIN 4
bool flashState = LOW;
int requestDelay = 3000;
int flashsensibility = 100;
int effect = 2;

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//Kamera pinlerinin belirlenmesi
//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//Kameranın yapılandırmasının yapılması
void configInitCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;


  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 1;  
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12; 
    config.fb_count = 1;
  }
  
  //Kameranın başlatılması
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera başlatılamadı 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //Kameranın parlaklık, kontrast, efekt gibi yapılandırılmasının yapılması
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);
  s->set_brightness(s, 2);
  s->set_contrast(s, 2);
  s->set_saturation(s, 2);
  s->set_special_effect(s, effect);
}

void handleNewMessages(int numNewMessages) {
  Serial.println(numNewMessages);
  Serial.print("Algilanan mesaj: ");
 

  //Bot'a gönderilen mesajın, kasa sahibine ait olup olmadığının kontrol edilmesi 
  for (int i = 0; i < numNewMessages; i++) {
    //Gelen mesajın kimden geldiğinin alınması
    String chat_id = String(bot.messages[i].chat_id);
    //Mesajı gönderen kişinin, kasa sahibi olup olmadığının kontrol edilmesi.
    if (chat_id != CHAT_ID){
      //Eğer mesaj gönderen kişi kasa sahibi değilse "Yetkisiz kullanıcı!" diye mesaj yolluyor.
      bot.sendMessage(chat_id, "Yetkisiz kullanıcı!", "");
      continue;
    }
    
    // Alınan mesajın serial ekrana yazdırılması
    String text = bot.messages[i].text;
    Serial.println(text);

    //Kasa sahibinin telegramda kullandığı ismin alınması.
    String from_name = bot.messages[i].from_name;
    
    //Alınan mesajın kontrol işlemleri gerçekleştiriliyor.
    if (text == "/baslat") {
      String welcome = "Hoşgeldin , " + from_name + "\n";
      welcome += "Aşağıda yer alan komutları kullanarak istediğin bilgiyi alabilirsin. Eğer kasan senden habersiz açılırsa anında bilgilendireceğim.\n";
      welcome += "/fotograf : Kasanın içeriğini görmeni sağlar\n";
      welcome += "/bilgi : Sistem hakkında bilgi edinmeni sağlar\n";
      welcome += "/ayarlar : Ayarlarda değişiklik yapmanı sağlar\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
     if (text == "/ayarlar") {
      String welcome = "Aşağıda yer alan kumutları kullanalarak sistemin ayar değişikliğini yapabilirsin.\n";
      welcome += "/varsayilan : Varsayılan ayarları görmeni sağlar\n";
      welcome += "/kullanilan : Kullanılan ayarları görmeni sağlar\n";
      welcome += "/hassasiyet : Sensörlerden okunan veri, ne sıkılıkla fotoğrafa dönüştürülsün?\n";
      welcome += "/flashassasiyeti : Flaş ne kadar süre açık kalsın?\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
     if (text == "/kullanilan") {
      String welcome = "Kameranın kullanılan ayarları;\n";
      welcome += "Hassasiyeti: "+ intToStringDelay(requestDelay) +" olarak ayarlanmıştır\n";
      welcome += "Flaş Hassasiyeti: "+ intToStringFlash(flashsensibility) +" milisaniye olarak ayarlanmıştır\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/varsayilan") {
      String welcome = "Kameranın kullanılan ayarları;\n";
      welcome += "Hassasiyeti: 3sn olarak ayarlanmıştır\n";
      welcome += "Flaş Hassasiyeti: 100 milisaniye olarak ayarlanmıştır\n";
       welcome += "/varsayilanayarlaradon : Bu komutu kullanarak varsayılan ayarlara dönebilirsin.\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
     if (text == "/bilgi") {
      String welcome = "Kasa şimdiye kadar " + String(openCount) + " kere açıldı.;\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/fotograf") {
      sendPhoto = true;
      Serial.println("Yeni fotograf istegi");
    }
    if (text == "/varsayilanayarlaradon") {
      flashsensibility = 100;
      requestDelay = 3000;
      bot.sendMessage(CHAT_ID, "Varsayılan ayarlara dönüldü.", "");
    }
     if (text == "/hassasiyet") {
      String welcome = "Kameranın olmasını istediğiniz hassasiyetini seçiniz;\n";
      welcome += "/"+ intToStringDelay(500) +"\n";
       welcome += "/"+ intToStringDelay(1000) +"\n";
        welcome += "/"+ intToStringDelay(3000) +"\n";
         welcome += "/"+ intToStringDelay(5000) +"\n";
          welcome += "/"+ intToStringDelay(10000) +"\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/flashassasiyeti") {
      String welcome = "Flaşın olmasını istediğiniz hassasiyetini seçiniz;\n";
      welcome += "/"+ intToStringFlash(100) +"\n";
       welcome += "/"+ intToStringFlash(1000) +"\n";
        welcome += "/"+ intToStringFlash(3000) +"\n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/"+intToStringFlash(100)) {
      flashsensibility = 100;
      bot.sendMessage(CHAT_ID, "Hassasiyet 100 olarak değiştirildi.", "");
    }
     if (text == "/"+intToStringFlash(1000)) {
      flashsensibility = 1000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 1000 olarak değiştirildi.", "");
    }
     if (text == "/"+intToStringFlash(3000)) {
      flashsensibility = 3000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 3000 olarak değiştirildi.", "");
    }
    if (text == "/"+intToStringDelay(500)) {
      requestDelay = 500;
      bot.sendMessage(CHAT_ID, "Hassasiyet 0.5sn olarak değiştirildi.", "");
    }
    if (text == "/"+intToStringDelay(1000)) {
      requestDelay = 1000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 1sn olarak değiştirildi.", "");
    }
    if (text == "/"+intToStringDelay(3000)) {
      requestDelay = 3000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 3sn olarak değiştirildi.", "");
    }
    if (text == "/"+intToStringDelay(5000)) {
      requestDelay = 5000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 5sn olarak değiştirildi.", "");
    }
    if (text == "/"+intToStringDelay(10000)) {
      requestDelay = 10000;
      bot.sendMessage(CHAT_ID, "Hassasiyet 10sn olarak değiştirildi.", "");
    }
  }
}

  String intToStringDelay(int requestDelay){
    if(requestDelay == 500){
      return "05sn";
      }
      else if(requestDelay == 1000){
        return "1sn";
        }
         else if(requestDelay == 3000){
        return "3sn";
        }
         else if(requestDelay == 5000){
        return "5sn";
        }
         else if(requestDelay == 10000){
        return "10sn";
        }
  }
  String intToStringFlash(int flashsensibility){
    if(flashsensibility == 100){
      return "100";
      }
      else if(flashsensibility == 1000){
        return "1000";
        }
         else if(flashsensibility == 3000){
        return "3000";
        }
  }
  //Telegrama fotoğraf yollama fonksiyonu 
String sendPhotoTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Fotograf cekilirken hata olustu!");
    delay(1000);
    ESP.restart();
    return "Fotograf cekilirken hata olustu!";
  }  
  
  Serial.println("Baglaniyor " + String(myDomain));


  if (clientTCP.connect(myDomain, 443)) {
    Serial.println("TCP baglantisi basarili!");
    
    String head = "--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + CHAT_ID + "\r\n--RandomNerdTutorials\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--RandomNerdTutorials--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot"+BOTtoken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(myDomain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=RandomNerdTutorials");
    clientTCP.println();
    clientTCP.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        clientTCP.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        clientTCP.write(fbBuf, remainder);
      }
    }  
    
    clientTCP.print(tail);
    
    esp_camera_fb_return(fb);
    
    int waitTime = 10000; 
    long startTimer = millis();
    boolean state = false;
    
    while ((startTimer + waitTime) > millis()){
      Serial.print(".");
      delay(100);      
      while (clientTCP.available()) {
        char c = clientTCP.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length()>0) break;
    }
    clientTCP.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("api.telegram.org baglantisi basarisiz.");
  }
  return getBody;
}
void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  // Serial monitörün başlatılması
  Serial.begin(115200);

  // Led flaşın output olarak belirlenmesi
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, flashState);

  // Kamera ayarlarının yapılandırılması için gereken fonksiyonun yazılması
  configInitCamera();

  //  WIFI ağına bağlanma işlemi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Baglaniliyor ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Adres: ");
  Serial.println(WiFi.localIP());
  pinMode(13, INPUT_PULLUP);
}

void loop() {
  //Hareket sensörünün tetiklenmesi durumunda yapılacak işlemler
  if(digitalRead(13) == HIGH){
     openCount++;
     flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      delay(flashsensibility);
      sendPhotoTelegram();
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      bot.sendMessage(CHAT_ID, "Kasa Açıldı!", "");
      delay(requestDelay); 
    }
    //Işık sensörünün tetiklenmesi durumunda yapılacak işlemler
    if(digitalRead(LDR) == LOW){
      openCount++;
       flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      delay(flashsensibility);
      sendPhotoTelegram();
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      bot.sendMessage(CHAT_ID, "Kasa Açıldı!", "");
      delay(requestDelay); 
    }
    //sendPhoto değişkeninin true olması durumunda yapılacak işlemler
  if (sendPhoto) {
    flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      delay(flashsensibility);
    sendPhotoTelegram(); 
    flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
    sendPhoto = false; 
    delay(100); 
  }
  
  //Yeni mesaj gelmiş mi? 
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("Yeni mesaj algilandi");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
