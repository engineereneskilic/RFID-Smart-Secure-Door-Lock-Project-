#include <WatchDog.h>
#include <Servo.h> // motor
#include <SPI.h> // spi haberleşme ama yine araştır
#include <MFRC522.h> // rfid yeni sistem


//wifi bağlanma
/*
#define ag_ismi  "yipranmak_paslanmaktan_iyidir"
#define ag_sifresi "abbBc9b23a"
#define SERVER "api.pushetta.com"
String API("a790df84ee3b87e9a71079830c6f6c2bfec684db");
String CHANNEL("kapialarm");

*/

Servo sg90_motor; // not servo 1kg kaldırır

//Bluetooth
String Ses;


// RFID Kart Özellikleri 
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//Kart buzzer özellikleri
int buzzerPin=2; // Buzzer'in + bacağının bnağlı olduğu pin

int hatalikart=0; 
int calis =0;  

//Ultrasonic Sensör
int u_trigPin = 6; //ultrasonic sensörde kullanılan tetikleme pini
int u_echoPin = 7; //ultrasonic sensörde kullanılan yankı pini
int u_sure=0;      //ultrasonic sensörde yollanan ver alınan ses dalgası arasında geçen süre
int u_mesafe=0;    //sensörün ölçtüğü mesafe

//KapıDurumu true:açık,false:kapalı;
bool kapidurum=true;
void setup() 
{
 
   pinMode(u_trigPin, OUTPUT); 
  pinMode(u_echoPin, INPUT);
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init() ;  // Initiate MFRC522
  Serial.println("Put your card to the reader...");
  Serial.println();
  pinMode(buzzerPin,OUTPUT); // Buzzer'in bağlı olduğu pini çıkış olarak ayarladık.
 
  sg90_motor.attach(5);// sg90 motorumuzu başlattık
  //kapiyiKapat();

  // Ağa bağlanma işlemleri
  /*
   if(Serial.find("OK")){         //esp modülü ile bağlantıyı kurabilmişsek modül "AT" komutuna "OK" komutu ile geri dönüş yapıyor.
     Serial.println("AT+CWMODE=1"); //esp modülümüzün WiFi modunu STA şekline getiriyoruz. Bu mod ile modülümüz başka ağlara bağlanabilecek.
     delay(2000);
     String baglantiKomutu=String("AT+CWJAP=\"")+ag_ismi+"\",\""+ag_sifresi+"\"";
    Serial.println(baglantiKomutu);
    
     delay(5000);
 }
*/
}
void loop(){

//  alarm sistemi metodu



//hatalı kart(Alarm Tetiklenmesi)
if(hatalikart>=10){
  AlarmiAc();
  hatalikart=0;
}

// Kapı hareket sensörü(Alarm Tetiklenmesi)
  digitalWrite(u_trigPin, LOW); //önce trig pinini LOW yapıyoruz ki daha sonra göndereceğimiz sinyali temiz bir şekilde gönderelim
  delayMicroseconds(5);       //5 mikrosaniye bekliyoruz
  digitalWrite(u_trigPin, HIGH); //daha sonra trig pinini HIGH yapıp bir ses dalgası yolluyoruz.
  delayMicroseconds(10);
  digitalWrite(u_trigPin, LOW);
  u_sure = pulseIn(u_echoPin, HIGH); //echo pini ile yolladığımız ses dalgasının kaç saniyede geri döndüğünü hesaplıyoruz. 
  u_mesafe = (u_sure/2) / 29.1;      // burada kullandığımız formül YOL = HIZ x ZAMAN formülü. Ses hızı sabit 331km/saat birim dönüştürmesi yapılınca yandaki formül geliyor.
  Serial.println(u_mesafe);
    if (u_mesafe >=0 && u_mesafe<=50 && !kapidurum){   //mesafe 10 cmden küçük ve kapı kapalıysa kapı zorlanmış kilidi kırılmış demektir..
       //delay(5000); // kapının kapanmasını bekle
       AlarmiAc();               
  }

// son


//Serial.println(rfid_sinyal);
//Kızılötesi mesafe sensörü(dokunma ile kapı kapatma)
int kizilotesi_degeri = analogRead(A0); //A0 Analog Input Pininden sensör değerimizi okuyoruz ve sensorValue değişkenine atıyoruz.
if(kizilotesi_degeri<=150){
  kapiyiKapat();
}
//Kızılötesi sensör bir şekilde kapatıldığında(bozulduğunda kablosu çıktığında)(Alarm tetiklenmesi)
if(kizilotesi_degeri==1023) AlarmiAc();
  



//Bluettoh Sensörü
while (Serial.available()){
delay(10);
char c = Serial.read();
if (c == '#') {break;}
Ses += c;
}
if(Ses.length()>0){
  Serial.println(Ses);
  if(Ses=="Kapıyı aç"){ // gelen ses aç ise
    if(!kapidurum)kapiyiAc(); // eğer kapı durumu kapalı ise aç
  }else if(Ses=="Kapıyı kapat"){ // gelen ses kapalı ise
    if(kapidurum)kapiyiKapat(); // eğer kapıdurumu açıksa kapat
  }
}
Ses="";// sesi işi bittiğinde temizle


 

   // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }


   String kayitli_kartlar[] = {"42 E2 1F 45"};
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  for(int i=0;i<=1;i++){
    if (content.substring(1) == kayitli_kartlar[i]) //change here the UID of the card/cards that you want to give access
    {
      //MesajBildirimi(CHANNEL,"Hırsız Var !");
      kapiyiAc();
      
      return true;
    }
   
   else   {
      kapiyiKapat();
      return false;
    }
  }
}
/*
void MesajBildirimi(String CHANNEL,String text) { 

  Serial.println(String("AT+CIPSTART=\"TCP\",\"") + SERVER + "\",80"); //server'a bağlanma
  
  delay(3000);
 String   alarm =("POST /api/pushes/");
    alarm+= (CHANNEL);
    alarm+= ("/ HTTP/1.1");
    alarm+= ("\r\n");
    alarm+= ("Host: ");
    alarm+= (SERVER);
    alarm+= ("\r\n");
    alarm+= ("Authorization: Token ");
    alarm+= (API);
    alarm+= ("\r\n");
    alarm+= ("Content-Type: application/json");
    alarm+= ("\r\n");
    alarm+= ("Content-Length: ");
    alarm+= (text.length() + 46);
    alarm+= ("\r\n\r\n");
    alarm+= ("{ \"body\" : \"");
    alarm+= (text);
    alarm+= ("\", \"message_type\" : \"text/plain\" }");
    alarm+= ("\r\n\r\n");
    
    Serial.print("AT+CIPSEND=");
 delay(100);
 Serial.println(alarm.length());
 // Sorun oluşmadıysa veriyi gönder.
 if(Serial.find(">")){
 Serial.println(alarm);
 Serial.println("AT+CIPCLOSE=0");
 delay(100);

 }
 else{
 Serial.println("AT+CIPCLOSE=0");
 }
    
  }
*/
void kapiyiAc(){
      kapidurum=true;
     Serial.println("Authorized access");
      Serial.println();

        BuzzerOnay();
        // Servo motor kapıyı aç
        sg90_motor.write(90);
    
     
}
void kapiyiKapat(){
  kapidurum=false;
  hatalikart++;
      Serial.println(" Access denied");
      BuzzerUyari();

       // Servo motor kapıyı kapat
        sg90_motor.write(60);

}

void AlarmiAc(){
  Serial.println("Alarm çalışıyor..");
  digitalWrite(buzzerPin,HIGH); //Buzzer'a güç göndererek ses çıkarmasını sağladık.
//  MesajBildirimi(CHANNEL,"Hırsız Var !");
}
void BuzzerOnay(){
    //dit dit
      digitalWrite(buzzerPin,HIGH); //Buzzer'a güç göndererek ses çıkarmasını sağladık.
      delay(100);                  // 1 saniye boyunca çalmasını söyledik.
      digitalWrite(buzzerPin,LOW); //Buzzerın gücünü keserek susmasını sağladık sağladık.
      delay(100);
      digitalWrite(buzzerPin,HIGH); //Buzzer'a güç göndererek ses çıkarmasını sağladık.
      delay(100);
       digitalWrite(buzzerPin,LOW); //Buzzerın gücünü keserek susmasını sağladık sağladık.
        
}
void BuzzerUyari(){
      digitalWrite(buzzerPin,HIGH); //Buzzer'a güç göndererek ses çıkarmasını sağladık.
      delay(1500);                  // 1 saniye boyunca çalmasını söyledik.
      digitalWrite(buzzerPin,LOW); //Buzzerın gücünü keserek susmasını sağladık sağladık.
}



