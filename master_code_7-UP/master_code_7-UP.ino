//Master Code : Team 7-UP
#include <Wire.h> //I2C

//HAHAHAHAHAHAHAHAH

#define PAY_ADR    0x14 // Payload I2C Address
//Register Map
#define PAY_ID     0x14 // PAYLOAD Address
#define PAY_SS     0x02 // PAYLOAD Status Address
#define PAY_DATA_L 0xB0 // PAYLOAD Low Data & Data Address
#define PAY_DATA_H 0xB1 // PAYLOAD High Data
#define PAY_PM     0xAA // PAYLOAD Power Management(Reset)

#define CM_PAY_ID  0x31 //Register for command ID&Address
#define CM_PAY_SS  0x32 //Register for command Status

void setup()
{
  //initial temp. variable for payload data
  byte id;
  byte payload_status;

  Wire.begin(); //คำสั่งให้เป็นตัว Master

  Serial.begin(9600); // start serial for debug at baudrate 9600
 
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //Initial Message Guideline For Command Payload 
  Serial.println("#####+++++ Payload Function Command +++++#####");
  Serial.println("______________________________________________\n"); 
  Serial.println("Enter '1' : Get Payload ID & Payload Address");
  Serial.println("Enter '2' : Get Data From Payload");
  Serial.println("Enter '3' : View Payload Status");
  Serial.println("Enter '9' : Reset Power Management on Payload "); 
  Serial.println("______________________________________________\n");
  
  request_byte(PAY_ADR, PAY_ID, &id, 1);             //request id
  request_byte(PAY_ADR, PAY_SS, &payload_status, 1); //request status

  Serial.println("Payload ID : " + String(id)); //Show Payload ID 
  Serial.println("Payload Status : 0B" + String(payload_status, BIN) + "\n"); //Show Payload Status Address 
}

void loop()
{
  byte payload_data[2]; // data[0] -> low , [1] -> high
  byte payload_status;  //Payload Status Value
  byte payload_adr;     //Payload Address Value
  byte payload_pm;      //Payload Reset Value

  if (Serial.available())
  {

    int c = -99;
    while (Serial.available())
    {
      c = Serial.parseInt();
      Serial.read();
    }
    Serial.println(c);
    if (c == 1) //command slave to Get Payload ID & Address
    {
      byte data_send = 0b00010100;              //create buffer to command Payload ID&Address
      write_byte(PAY_ADR, CM_PAY_ID, data_send); //write buffer to #PAY_ADR ,at register : #CM_PAY_ID,buffer #data_send
      Serial.println("Command Get Payload ID & Address!"); 
     
      request_byte(PAY_ADR, PAY_ID, &payload_adr, 1 ); //request #PAY_ADR, to send Payload Address back, save it on #payload_adr,1 byte
      Serial.println("Payload ID : " + String(payload_adr,DEC));        //Show ID
      Serial.println("Payload Address : 0x" + String(payload_adr,HEX) + "\n"); //Show Address
    }
    else if (c == 2) //command slave to Get Data From Payload
    {
      Serial.println("Command Get Data From Payload!"); 
 
      request_byte(PAY_ADR, PAY_DATA_L, payload_data , 2 ); //request #PAY_ADR, to send Payload Data back, save it on #payload_data,2 byte
        //decode data from 2 byte [0-> low , 1 -> high]
        //payload_data[0] = 0bllllllll
        //payload_data[1] = 0bhhhhhhhh
        //paydata = 0bhhhhhhhhhllllllll and convert to int, after below calculation 
      int pay_data = payload_data[0] + (payload_data[1] << 8);
        //Show Result From Payload
      //Serial.println("Payload UV Data : " + String(pay_data,DEC) + " mV. ,From Bit:(Low:" + String(payload_data[0], DEC) + "),(High:" 
     // + String(payload_data[1], DEC)+ ") \n");
      Serial.println("Payload UV Data : " + String(pay_data,DEC) + " mV.");
    }
    else if (c == 3) //command slave to Get Payload Status
    {
      byte data_send = 0b00001100;               //create buffer to command Payload Status
      write_byte(PAY_ADR, CM_PAY_SS, data_send); //write buffer to #PAY_ADR ,at register : #CM_PAY_SS,buffer #data_send
      Serial.println("Command Get Payload Status!");
      
      request_byte(PAY_ADR, PAY_SS, &payload_status, 1 ); //request #PAY_ADR, to send Payload Status back, save it on #payload_status,1 byte
      Serial.println("Payload Status: 0x" + String(payload_status, HEX) + " == 0B" + String(payload_status, BIN) + "\n"); //Show Status HEX , Binary code
    }
    else if (c == 9)//command slave to Reset Payload
    {
      byte data_send = 0b00000001;            //create buffer to command Payload Power Management 
      write_byte(PAY_ADR, PAY_PM, data_send); //write buffer to #PAY_ADR ,at register : #PAY_PM,buffer #data_send
      Serial.println("Command Power Management (Reset Payload)!");
      
      request_byte(PAY_ADR, PAY_PM, &payload_pm, 1);  //request #PAY_ADR, to send Command Reset Payload back, save it on #payload_pm,1 byte
      delay(1000); //waiting Payload Reset
      Serial.println("Payload Reset Successfully \n"); //Show Payload Reset finished
    }
  }
}

void write_byte(const int slaveAddress, const int registerAddress, byte data_send)
{
  Wire.beginTransmission(slaveAddress); //คำสั่งให้ Master ส่งข้อมูลให้ Slave
  Wire.write(registerAddress);          //ให้ส่งข้อมูล register ไปให้ตัว Slave
  Wire.write(data_send);
  Wire.endTransmission(); //หยุดการส่ง
  delay(1); //wait to already send 
  //up to i2c data transfer rate if 100kHz -> use approx. time : 1 mSecond   
}

void request_byte(const int slaveAddress, const int registerAddress, byte *data_in, const int nBytes)
{
  Wire.beginTransmission(slaveAddress);   //คำสั่งให้ Master ส่งข้อมูลให้ Slave
  Wire.write(registerAddress);            //ส่งข้อมูล register ที่ต้องการ ไปให้ตัว Slave
  Wire.endTransmission();                 //หยุดการส่ง
  delay(250);                               //delay 250 millisecond for slave prepare data
  Wire.requestFrom(slaveAddress, nBytes); //ขอข้อมูลจาก Slave
  int i = 0;
  while (Wire.available())
  {
    data_in[i++] = Wire.read();
  }
  if (i != nBytes)
    Serial.println("Error!!! :" + String(i) + String(nBytes));
}
