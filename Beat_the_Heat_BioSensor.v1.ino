/*
  Beat the Heat Biosensor

  This biosensor takes the input of a Pulse Sensor clipped to your ear through A0
  and uses this to calculate the user's heart rate. This is then used, along with
  the scaling of a potentiometer connected to A1, to control the speed of a fan
  (can substitute a motor). The faster the user runs, the faster the fan will run,
  keeping the runner cool as they workout. The heart rate is displayed on an LCD
  for the user's benefit.

  The circuit:
  Pulse Sensor connected to A0
  Potentiometer connected to A1
  A motor or fan connected to pin 6
  An LCD connected to pins 12, 11, 2, 3, 4, 5

  Created 25 April 2018
  By Erin Cressman and Erik Dyrli
  Modified 30 April 2018
  By Erin Cressman

  http://url/of/online/tutorial.cc

  ENGR 375 Bioinstrumentation and Measurement
  Messiah College Department of Engineering
*/

// Initialize LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,2,3,4,5);

//  Initialize Variables
const int fanPin = 6; // fan connected to pin 6;
int PulseSensorPurplePin = 0; // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
float potSig; // Input signal from potentiometer
int count = 0; // To track number of data points
float timeGap = 0.5; //
int sensorSum =0; // For calculating average
float sensorAverage[4]; // Average of last 10 data points
long prevBeatTime = 0; // Time of last beat
long beatTime=0; //Time of beat
float beatInterval = 0; // Time since last beat
int beatCount = 0; // Number of beats
float beatIntervalSum =0; // Sum of time between beats
float beatIntervalAvg = 0; // Average time between beats
int HR = 0; // Calculated HR
boolean upward = false; // True if pulse has positive slope
boolean downward = false; // True if pulse has negative slope
float scaled = 20.0; // result of HR*potSig
int motor = 0; // mapped result of scaled for motor function
int Signal; // PulseSensor signal  


void setup() {
  // Initialize LCD and Serial Plot
  lcd.begin(16,2);
  Serial.begin(9600);

  // Initialize input and output pins
  pinMode(PulseSensorPurplePin, INPUT);
  pinMode(A1, INPUT);
  pinMode(fanPin, OUTPUT);
}

void loop() {
  // Read PulseSensor value  
  Signal = analogRead(PulseSensorPurplePin);  
  
  // Add value to running sum and keep count for taking average
  sensorSum = sensorSum+Signal;
  count = count+1;

  // check to see if 13 points have been collected -> calculate and display avg
  if(count > 12){
    
    // print average of 13 data points and store in 1st spot in Array, saving previous in other spots
    for (int i=4; i > 1; i--){
      sensorAverage[i] = sensorAverage[i-1];
    }
    sensorAverage[1] = sensorAverage[0];
    sensorAverage[0] = sensorSum/count;

    //Reset variables
    count = 0;
    sensorSum = 0;
    upward = false;
    downward = false;

    // Determine if there is switch in direction of slope
    for (int n = 1; n < 4; n++){
      if (sensorAverage[n] < sensorAverage[n-1]){
        downward = true;
      }else if (sensorAverage[n] > sensorAverage[n-1]){
        upward = true;
      }
    }

    // Check for change in slope direction
    if (upward == true && downward == true){
      // Calcuate time passed between last change
      beatTime = millis();
      delay(5);
      beatInterval = beatTime - prevBeatTime;
      beatIntervalSum += beatInterval;
      prevBeatTime = beatTime;
      beatCount += 1;    

      // Clear array
      for (int n = 0; n < 4; n++){
        sensorAverage[n] = 0;
      }
    }

    // Check to see if 20 intervals have been taken
    if (beatCount > 19){
      // Average intervals and calculate HR, constraining to reasonable values
      beatIntervalAvg = beatIntervalSum/beatCount;  
      HR = 60000/(2*beatIntervalAvg); // Divide by 2 since slope chances twice in one heart beat
          
      // Reset variables
      beatCount = 0;
      beatIntervalSum = 0;

      // Display HR or error message on LCD
      if (HR > 240 || HR < 35){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor Error");
      }
      else if (HR > 210){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Heart Rate:");
        lcd.setCursor(0, 1);
        lcd.print("EXCEEDS 210");
      }
      else if (HR < 40){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Heart Rate:");
        lcd.setCursor(0, 1);
        lcd.print("BELOW 40");
      }
      else{
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Heart Rate:");
        lcd.setCursor(0, 1);
        lcd.print(HR);
      } 
    }
   }

  // Read potentiometer value and find gain for motor speed
  potSig = abs((analogRead(A1))/512.0) ;
  
  // Control fan speed based on HR and Potentiometer
  HR = constrain(HR, 40, 150);
  scaled = HR*potSig;
  motor = map(scaled, 0, 310, 45, 250);
  //Serial.print(scaled);
  //Serial.print(",   ");
  //Serial.println(motor);
  analogWrite(fanPin, motor);

  // Delay to prevent overload
  delay(10);
}
