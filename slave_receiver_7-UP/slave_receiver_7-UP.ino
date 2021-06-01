//Payload Code : Team 7-UP
#include <Wire.h> //I2C

//Slave Register Address Map
#define PAY_ID      0x14 // PAYLOAD Address 
#define PAY_SS      0x02 // PAYLOAD Status Address
#define PAY_DATA_L  0xB0 // PAYLOAD Low Data & Data Address
#define PAY_DATA_H  0xB1 // PAYLOAD High Data
#define PAY_PM      0xAA // PAYLOAD Power Management(Reset)

#define CM_PAY_ID   0x31 //Register for command ID&Address
#define CM_PAY_SS   0x32 //Register for command Status

//Slave Register value
#define PAY_ID_VAL  0x14 // Payload ID
#define SLAVE_ADRR  0x14 // Address for I2C Bus Communication

//initial data buffer from master
//at array[0] -> received payload register address
//at array[1] -> data to write data to payload register
byte registerRequested[2] = {0xFF,0xFF};

//Define flag Command Payload
bool isReceived = 0;
bool isRequestData =0;
bool cmd_id =0;
bool cmd_data = 0;
bool cmd_ss = 0;
bool cmd_pm = 0;

int voltageA0 = 0; //variable check voltage

int uv_data=0; //variable sensor data

byte pay_ss_val;
byte return_byte = 0xFF;
byte return_array[2];
byte pay_val_data_l;
byte pay_val_data_h;
byte pay_pm_value = 0b00000000;

void setup(){

  Wire.begin(SLAVE_ADRR);                // join i2c bus with address #8
  
  pinMode(LED_BUILTIN,OUTPUT);

  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);           // start serial for output
  Serial.println("Slave Address: 0x" + String(SLAVE_ADRR, HEX) + " initial success!! \n");

}

void(* resetFunc) (void) = 0; //Function Reset

void  Reset_Payload() //Function Reset Payload
{
    Serial.println("Master Command --> Reset Payload! \n");
    delay(1000);
    resetFunc();
}

int Get_Sensor(){ //Function UV Sensor
  long sum=0;
for(int i=0;i<1024;i++){
  int sensorValue = analogRead(A0);
  sum=sensorValue+sum;
}
  sum =sum >>10;
  uv_data = sum*4980/1023; 
}

void loop() { //Loop Running

  //Read Sensor Status from analog
  voltageA0 = analogRead(A0);

  bool reader1_ss = 1; //Logic for Read Voltage Pin analog : A0

  //if (voltageA0 == 0)
  if (voltageA0 != 0) //update code
  {
    //reader1_ss = 1;  //Logic = 0 if Voltage == 0
    reader1_ss = 1;  //Logic = 1 if Voltage != 0 update code
  }

  pay_ss_val =  1 * 8 + reader1_ss * 4 + reader1_ss * 2 + reader1_ss - reader1_ss * 3 ;
  //pay_ss_val = 1<<3|reader1_ss<<2|reader1_ss<<1|reader1_ss<<0 - reader1_ss * 3;

  if (isReceived) //if Master -->> Request Data From Payload
  {
    //clear flag = 0
    isReceived=0;
    
    switch (registerRequested[0]) //check data 
    {
      case PAY_ID:      return_byte = PAY_ID_VAL;   break; //Return Payload ID & Address --> Master

      case CM_PAY_ID:   cmd_id = 1;                 break; //Master --> Request Payload ID & Address
                        
      case PAY_SS:      return_byte = pay_ss_val;   break; //Return Payload Status --> Master
      
      case CM_PAY_SS:   cmd_ss=1;                   break; //Master --> Request Payload Status
                        
      case PAY_DATA_L:  isRequestData = 1;          break; //Master --> Request Payload Data
      case PAY_PM:      cmd_pm = 1;                 break; //Master --> Command Reset Payload
      default :         return_byte = 0xFF;         break;
    }

    if(isRequestData) //if Master -->> Request Sensor Data From Payload
    {
      Get_Sensor(); //Start UV Sensor Function

      pay_val_data_l = uv_data & 0x000000FF ; //uv_data % 255 
      pay_val_data_h = uv_data  >> 8 ;        //if( uv_data > 255 ) >> 8

      //set return byte array
      return_array[0] = pay_val_data_l; 
      return_array[1] = pay_val_data_h;

      //debug message Show Register , Sensor Data , Bit(Low,High)
      Serial.print("Register:0x" + String(registerRequested[0], HEX) + " , ");
      Serial.println("Sending UV Data : " + String(uv_data, DEC) + " mV." + "\n");
      /*,(Low:");
      Serial.print(return_array[0]);
      Serial.print(" At Register:0x" + String(PAY_DATA_L, HEX) + " , High:");
      Serial.print(return_array[1]);
      Serial.println(" At Register:0x" + String(PAY_DATA_H, HEX) + " ) -->Master \n");*/
      delay(20);
    }
    
    if (cmd_id == 1) //Command ID & Address
    {
      cmd_id =0; //clear flag = 0
      Serial.print("Register: 0x" + String(registerRequested[0], HEX)); //Show Register the Master Send to Payload
      Serial.println(" , Data send : " + String(registerRequested[1], HEX) + " -->From Master"); //Show data_send from Master
      Serial.println("Sending Payload ID : " + String(PAY_ID_VAL, DEC) + " -->Master");  //Show Payload ID 
      Serial.println("Sending Payload Address : 0x" + String(PAY_ID_VAL, HEX) + " -->Master \n"); //Show Payload Address
    }
    if (cmd_ss == 1) //Command Payload Status 
    {
      cmd_ss =0; //clear flag = 0
      Serial.print("Register: 0x" + String(registerRequested[0], HEX)); //Show Register the Master Send to Payload
      Serial.println(" , Data send : " + String(registerRequested[1], HEX) + " -->From Master"); //Show data_send from Master
      Serial.println("Register Send Status : 0B" + String(pay_ss_val, BIN) + " -->Master \n"); //Show Payload Status
    }
    if (cmd_pm == 1) //Command Reset Payload
    {
      cmd_pm =0; //clear flag = 0
      Serial.print("Register: 0x" + String(registerRequested[0], HEX)); //Show Register the Master Send to Payload
      Serial.println(" , Data send : " + String(registerRequested[1], HEX) + " -->From Master"); //Show data_send from Master
      pay_pm_value = registerRequested[1]; //set register from master
      Reset_Payload(); //Start Reset Payload Function
    }
  }
  //delay(25); //for reduce loop iteration time
}

void requestEvent() //check isRequest data and send {buffer data} back to master
{
  if(isRequestData) //check process is request sensor data 
  {
    isRequestData=0;
    
    //send return byte array [ 1byte / 1write() ] 
    Wire.write(return_array[0]);
    Wire.write(return_array[1]);
    
    //clear flag
    return_array[0] = 0xFF;
    return_array[1] = 0xFF;
  }
  //send return data to master
  Wire.write(return_byte);
  //clear buffer
  return_byte = 0xFF;
}

void receiveEvent(int bytes)
{
  //set received flag to 1 and read from master
  isReceived = 1;//รับข้อมูลตัวอักษรตามจำนวน byte แล้วพิมพ์ออกมา
  int i =0; //set intial received index array 
  while (Wire.available())
  {
    // read i2c data from master-> buffer #registerRequested
    // set buffer at i and i = i+1 to change received index
    registerRequested[i++] = Wire.read(); 
  }
  if (i != bytes) //total received not equal to bytes on bus
    //decode message occur Error
    Serial.println("Error!!! :" + String(i) + String(bytes));
}
