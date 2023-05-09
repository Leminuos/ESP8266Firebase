/*
Hệ thống BMS(Build Management System)
Khí gas nặng hơn không khí => lắng đọng và lơ lửng dưới sàn nhà  => nên đặt module gần nơi phát ra khí gas, gần sàn nhà để phát hiện kịp thời
Khi xuất hiện khí gas => Mở cửa sổ để thoát khí => Cảnh báo => tất cả các thiết bị điện tắt 
Khi phát hiện cháy => Mở cửa => Cánh báo => Tất cả các thiết bị điện tắt => Hệ thống phun nước
*/
/*
https://github.com/mobizt/Firebase-ESP8266/blob/master/README.md
Read Data: Thông qua các hàm get: getInt, getFloat, getDouble, getBool, getString, getJSON, getArray, getFile.
Trả về giá trị boolean.
Trả về true nếu thoả mãn các điều kiện sau:
Serve trả về mã trạng thái HTTP 200
Các loại dữ liệu phù hợp giữa yêu cầu và phản hồi
Cú pháp chung:
Firebase.get(fbdo, <path>)
fbdo là một kiểu dữ liệu FirebaseData lưu trữ dữ liệu trả về từ Firebase
<path> là đường dẫn trên Firebase Realtime Database để lấy dữ liệu.
Kiểu dữ liệu của giá trị được trả về: fbdo.dataType()
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Thông tin kết nối firebase
#define FIREBASE_HOST "esp8266-c8c00-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "8WKPbzbAt7i8xAJBDe3R93sDoelobZbPeJx7CX5G"

// Thông tin kết nối WiFi
#define SSID "Nguyen Hai"
#define PASS "66668888"

// Các biến truyền/ nhận dữ liệu
String inputString = "";
String sendString = "";
bool stringComplete = false;

// Trạng thái các thiết bị trong nhà
boolean stateLedPK = 0, stateFanPK = 0;                  // Phòng khách
boolean stateLedPN = 0, stateAirPN = 0;                  // Phòng ngủ
boolean stateLedPB = 0, stateFanPB = 0;                  // Phòng bếp

// Các biến Management System
boolean stateGas = 0, stateFire = 0;        // Trạng thái khí gas và cháy nhà 
float humi, temp;                           // Biến dùng lưu trữ nhiệt độ, độ ẩm phòng khách
float tempPB;                               // Biến dùng lưu trữ nhiệt độ phòng bếp
unsigned long previousTime;                 // Lưu thời gian trước đó

FirebaseData firebaseData;                   // Đối tượnng firebasedata

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP:");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // put your main code here, to run repeatedly:
   while(Serial.available() > 0){
    char inChar = (char) Serial.read();
    inputString += inChar;
    if(inChar == '\n'){
      stringComplete = true;
    }
    if(stringComplete){
    //Serial.print("Data nhận:");
    //Serial.println(inputString);
    //===============================================================
    // Phòng khách
    if(inputString.indexOf("L1PK") >= 0){
      //Serial.println("Đèn phòng khách bật!");
      Firebase.setString(firebaseData, "/Livingroom/Led", "ON");
    }
    else if(inputString.indexOf("L0PK") >= 0){
      //Serial.println("Đèn phòng khách tắt!");
      Firebase.setString(firebaseData, "/Livingroom/Led", "OFF");
    }
    if(inputString.indexOf("F1PK") >= 0){
      //Serial.println("Quạt phòng khách bật!");
      Firebase.setString(firebaseData, "/Livingroom/Fan", "ON");
    }
    else if(inputString.indexOf("F0PK") >= 0){
      //Serial.println("Quạt phòng khách tắt!");
      Firebase.setString(firebaseData, "/Livingroom/Fan", "OFF");
    }
    //==================================================================
    // Phòng bếp
    if(inputString.indexOf("L1PB") >= 0){
      //Serial.println("Đèn phòng bếp bật!");
      Firebase.setString(firebaseData, "/Kitchen/Led", "ON");
    }
    else if(inputString.indexOf("L0PK") >= 0){
      //Serial.println("Đèn phòng bếp tắt!");
      Firebase.setString(firebaseData, "/Kitchen/Led", "OFF");
    }
    if(inputString.indexOf("F1PK") >= 0){
      //Serial.println("Quạt phòng bếp bật!");
      Firebase.setString(firebaseData, "/Kitchen/Fan", "ON");
    }
    else if(inputString.indexOf("F0PK") >= 0){
      //Serial.println("Quạt phòng bếp tắt!");
      Firebase.setString(firebaseData, "/Kitchen/Fan", "OFF");
    }
    //=================================================================
    // Phòng ngủ
     if(inputString.indexOf("L1PN") >= 0){
      //Serial.println("Đèn phòng ngủ bật!");
      Firebase.setString(firebaseData, "/Bedroom/Led", "ON");
    }
    else if(inputString.indexOf("L0PN") >= 0){
      //Serial.println("Đèn phòng ngủ tắt!");
      Firebase.setString(firebaseData, "/Bedroom/Led", "OFF");
    }
    if(inputString.indexOf("A1PN") >= 0){
      //Serial.println("Điều hoà phòng ngủ bật!");
      Firebase.setString(firebaseData, "/Bedroom/Air conditioner", "ON");
    }
    else if(inputString.indexOf("A0PN") >= 0){
      //Serial.println("Điều hoà phòng ngủ tắt!");
      Firebase.setString(firebaseData, "/Bedroom/Air conditioner", "OFF");
    }
    //=========================================================================
    // Management System
    if(inputString.indexOf("T") >= 0 && inputString.indexOf("C") >= 0){
    String tempString="";
    // Tách dữ liệu
    tempString = inputString.substring(inputString.indexOf("T") + 1, inputString.indexOf("C"));
    temp = tempString.toFloat();
    Serial.print("Nhiệt độ:");
    Serial.println(temp);
    Firebase.setString(firebaseData, "/Management System/Temperature", temp);
    }
    if(inputString.indexOf("H") >= 0 && inputString.indexOf("%") >= 0){
    String humiString="";
    // Tách dữ liệu
    humiString = inputString.substring(inputString.indexOf("H") + 1, inputString.indexOf("%"));
    humi = humiString.toFloat();
    Serial.print("Độ ẩm:");
    Serial.println(humi);
    Firebase.setString(firebaseData, "/Management System/Humidity", humi);
    }
    if(inputString.indexOf("B") >= 0 && inputString.indexOf("C") >= 0){
    String tempPBString="";
    // Tách dữ liệu
    tempPBString = inputString.substring(inputString.indexOf("B") + 1, inputString.indexOf("C"));
    tempPB = tempPBString.toFloat();
    Serial.print("Nhiệt độ bếp:");
    Serial.println(tempPB);
    Firebase.setString(firebaseData, "/Kitchen/Temperature", tempPB);
    }
    inputString = "";
    stringComplete = false;
    }
  }
  //======================================================================================================
  // Nhận dữ liệu từ firebase
  // Phòng khách
  if(Firebase.getString(firebaseData, "/Livingroom/Led")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateLedPK = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateLedPK = 0;
    }
  }
  if(Firebase.getString(firebaseData, "/Livingroom/Fan")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateFanPK = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateFanPK = 0;
    }
  }
  // Phòng bếp
  if(Firebase.getString(firebaseData, "/Kitchen/Led")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateLedPB = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateLedPB = 0;
    }
  }
  if(Firebase.getString(firebaseData, "/Kitchen/Fan")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateFanPB = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateFanPB = 0;
    }
  }
  // Phòng ngủ
  if(Firebase.getString(firebaseData, "/Bedroom/Led")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateLedPN = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateLedPN = 0;
    }
  }
  if(Firebase.getString(firebaseData, "/Bedroom/Air conditioner")){
    Serial.println(firebaseData.stringData());
    if(firebaseData.stringData() == "ON"){
      stateAirPN = 1;
    }else if(firebaseData.stringData() == "OFF"){
      stateAirPN = 0;
    }
  }
  // Gửi dữ liệu tới Mega
  if(millis() - previousTime > 1000){  // Sau 1s sẽ gửi dữ liệu
    sendString = "L" + String(stateLedPK) + "PK" + "F" + String(stateFanPK) + "PN" +
                 "L" + String(stateLedPB) + "PB" + "F" + String(stateFanPB) + "PB" +
                 "L" + String(stateLedPN) + "PN" + "A" + String(stateAirPN) + "PN";
    Serial.println(sendString); 
    previousTime = millis();
  }
}