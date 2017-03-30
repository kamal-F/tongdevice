#include "Arduino.h"
#include <Servo.h>
#include <NewPing.h>
#include <SoftwareSerial.h>
#include "MyKey.h"

#define ID_TONG "2" //id tong sampah

#define NETRALPOS  45
#define ORGANIKPOS  0
#define NONORGANIKPOS  90
#define DISTANCE_CLIENT 40 //jarak org dengan tong

#define TRIGGER_PIN 11
#define ECHO_PIN 12
#define MAX_DISTANCE 200

#define SERVO_PIN 10

#define DEBUG false

const String HOST = "192.168.43.114"; //your localhost at same ssid, ifconfig

Servo myservo;  // create servo object to control a servo
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
SoftwareSerial ser(A2, A3); // RX 2, TX 3

int posisi = 0;

String vIdtong;
String vstatusopen;
String vIdopener;


void setup() {

	myservo.attach(SERVO_PIN);  // attaches the servo on pin 9 to the servo object
	Serial.begin(9600); //Set up a serial connection for 9600 bps.

	myservo.write(NETRALPOS);

	ser.begin(9600); // 9600 stable baud rate for uno
	delay(100);
	connectWifi();
	delay(100);

	vIdtong = "";
	vstatusopen = "";
	vIdopener = "";

}

void loop() {

	/*for(int i=0;i<180;i++){
		myservo.write(i);
		delay(50);
	}*/
	myservo.write(NETRALPOS);


	//delay(500);

	unsigned int uS = sonar.ping_cm();

	//Serial.println(uS);

	//cek jarak, kurang dari 30 cm, test < 4
	if(uS < DISTANCE_CLIENT){
		//---send to esp
//String(ID_TONG)

		String request = "{\"idtong\":\"" + String(ID_TONG) + "\",\"idopener\":\"" + vIdopener + "\"}";

		//Serial.println(request);
		String cmd = "AT+CIPSTART=\"TCP\",\"";
		cmd += HOST;
		cmd += "\",80\r\n";

		sendCommand(cmd, 100, DEBUG);
		//delay(50);

		String cmd2 = "POST /tongcerdas/api.php";
		cmd2 += " HTTP/1.1\r\n";
		cmd2 += "Host: " + HOST + "\r\n";
		cmd2 += "Accept: application/json\r\n";
		cmd2 += "Content-Type: application/json\r\n";
		cmd2 += "Content-Length: ";
		cmd2 += request.length();
		cmd2 += "\r\n\r\n";
		cmd2 += request;

		String pjg = "AT+CIPSEND=";
		pjg += cmd2.length();
		pjg += "\r\n";

		String closeCommand = "AT+CIPCLOSE";
		closeCommand += "\r\n";

		sendCommand(pjg, 100, DEBUG);
		//delay(50);

		//delay(100);
		String isi = sendCommand(cmd2, 500, DEBUG);

		//Serial.println(cmd2);
		delay(100);

		int ygke = isi.indexOf("idtong") - 2;
		int ygakhir = isi.indexOf("}") + 1;

		String result = isi.substring(ygke, ygakhir);

		Serial.println(result);
		//parse
		//find idtong
		ygke = result.indexOf("idtong\":")+sizeof("idtong\":")-1;
		ygakhir = result.indexOf(",\"statusopen");
		vIdtong = result.substring(ygke+1,ygakhir-1);

		//find statusopen
		ygke = result.indexOf("statusopen\":") + sizeof("statusopen\":")-1;
		ygakhir = result.indexOf(",\"idopener");
		vstatusopen = result.substring(ygke+1,ygakhir-1);


		//find idopener
		ygke = result.indexOf("idopener\":") + sizeof("idopener\":")-1;
		ygakhir = result.indexOf("}");
		vIdopener = result.substring(ygke+1,ygakhir-1);

		//if(vIdopener != "null"){vIdopener= result.substring(ygke+1,ygakhir-1);}

		//ser.clearWriteError();

		//{"idtong":"2","statusopen":"0","idopener":null}

		Serial.println(vstatusopen);
		Serial.println(result);

		if(vstatusopen == "1"){
			Serial.println("terbuka");
		} else {
			Serial.println("tertutup");

		}
		//Serial.println((int)vstatusopen.substring(1, vstatusopen.length()));
		delay(50);
		sendCommand(closeCommand, 100, DEBUG);

		//Serial.println(result);

		//post to server: id tong
		//response to parse
		//id tong, status open/close

		//if status open
		//delay 500 atau 1000 wait client buang sampah
		//cek proximity, ada di range berapa utk organik/non organik

		//if (organik){
		myservo.write(ORGANIKPOS);
		delay(500);
		myservo.write(NETRALPOS);
		//}else if(nonorganik){
		//myservo.write(NONORGANIKPOS);
		//delay(200);
		//myservo.write(NETRALPOS);
		//} else{
		//myservo.write(NETRALPOS);
		//}

		//db di server
		//idtong, status, whoisopener


	} else{
		//post to server: id tong, close door
	}

}


String sendCommand(String command, int timeout, boolean debug) {
	String response = "";

	ser.print(command); // send the read character to the esp8266

	unsigned long time = millis();

	while ((time + timeout) > millis()) {
		while (ser.available()) {
			// The esp has data so display its output to the serial window
			char c = ser.read(); // read the next character.
			response += c;
		}
	}

	if (debug) {
		Serial.print(response);
	}

	return response;
}
void connectWifi() {
	//Set-wifi
	Serial.print("Restart Module...\n");
	sendCommand("AT+RST\r\n", 1000, DEBUG);
	delay(100);
	Serial.print("Set wifi mode...\n");
	sendCommand("AT+CWMODE=1\r\n", 1000, DEBUG); //
	delay(100);
	Serial.print("Connect to access point...\n");
	sendCommand("AT+CWJAP=\"" + SSID + "\",\"" + PASS + "\"\r\n", 1000, DEBUG);
	delay(1000);
	Serial.print("Check IP Address...\n");
	sendCommand("AT+CIFSR\r\n", 2000, DEBUG); // get ip address
	delay(2000);
}
