//Witty Cloud NodeMCU ESP12F

int ldrPin   =A0;

int redPin = 15;
int greenPin = 12;
int bluePin = 13;
 
//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
 
void setup()
{
  Serial.begin(115200);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  
}
 
void loop()
{

  int ldrValue = int(analogRead(ldrPin)/(1024/6));
  Serial.println(ldrValue);

  if(1024> ldrValue && ldrValue> 1024/6*5){
    setColor(0, 0, 255);  // blue
  }
  if(1024/6*5> ldrValue && ldrValue> 1024/6*4){
    setColor(0, 255, 255);  // aqua
  }
  if(1024/6*4> ldrValue && ldrValue> 1024/6*3){
    setColor(0, 255, 0);  // green
  }
  if(1024/6*3> ldrValue && ldrValue> 1024/6*2){
    setColor(255, 255, 0);  // yellow
  }
  if(1024/6*2> ldrValue && ldrValue> 1024/6*1){
    setColor(80, 0, 80);  // purple
  }
  if(1024/6*1> ldrValue && ldrValue> 1024/6*0){
    setColor(255, 0, 0);  // red
  }
  
  
//  setColor(255, 0, 0);  // red
//  delay(1000);
//  setColor(0, 255, 0);  // green
//  delay(1000);
//  setColor(0, 0, 255);  // blue
//  delay(1000);
//  setColor(255, 255, 0);  // yellow
//  delay(1000);  
//  setColor(80, 0, 80);  // purple
//  delay(1000);
//  setColor(0, 255, 255);  // aqua
//  delay(1000);
}
 
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}
