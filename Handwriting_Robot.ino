#include <Servo.h>

Servo pen;

// Servo on PWM pin 15
const int penServoPin = 15;
const int penDelay = 150; //delay for servo to reach position

//servo angle for up and down
const int penUp = 90;
const int penDown = 125;

// defines pins numbers
const int xstepPin = 12;
const int xdirPin = 11;
const int ystepPin = 13;
const int ydirPin = 14;

const int Control = 24;
const int xlimit = 25;
const int ylimit = 26;

const float away = 1;
const float origin = 0;

int xDir, yDir;

// Motor steps to go 1 millimeter.

// Calculate steps per mm. Enter here.
const float StepsPerMillimeterX = 54.3478;
const float StepsPerMillimeterY = 54.3478;

//Makes 200 pulses for making one full cycle rotation - full-stepping
//if you need micro-stepping - 3200 pulses
const int stepsPerRevolution = 64;

// Drawing robot limits, in mm
const float Xmin = 0;
const float Xmax = 155;
const float Ymin = 0;
const float Ymax = 155;

//initialise the positions of x and y
float Xpos = Xmin;
float Ypos = Ymin;

char line[50];    //initialise an array to store received G codes
int lineIndex = 0;

char temp[15]; //temporary array to process G code

void xstepper_rev(int xdir)
{
  //set the direction of rotation
  digitalWrite(xdirPin, xdir);

  //one pulse for the motor to take one step
  digitalWrite(xstepPin, HIGH);
  delayMicroseconds(800);
  digitalWrite(xstepPin, LOW);
  delayMicroseconds(800);
}

void ystepper_rev(int ydir)
{
  //set the direction of rotation
  digitalWrite(ydirPin, ydir);

  //one pulse for the motor to take one step
  digitalWrite(ystepPin, HIGH);
  delayMicroseconds(800);
  digitalWrite(ystepPin, LOW);
  delayMicroseconds(800);

}

void not_writing()
{
  pen.write(penUp);  //pen up
  delay(penDelay);  //delay for theservo to reach position
  Serial.println("pen is not writing");
}

void writing()
{
  pen.write(penDown);   //pen down
  delay(penDelay);      //delay for theservo to reach position
  Serial.println("pen is writing now");

}

void setup()
{
  //declare all outputs
  pinMode(xstepPin, OUTPUT);
  pinMode(xdirPin, OUTPUT);

  pinMode(ystepPin, OUTPUT);
  pinMode(ydirPin, OUTPUT);

  //declare input for homing controls
  pinMode(Control, INPUT);
  pinMode(xlimit, INPUT);
  pinMode(ylimit, INPUT);

  //pullup input
  digitalWrite(Control, HIGH);

  //servo pin
  pen.attach(penServoPin);

  //initially set servo up
  pen.write(penUp);
  delay(penDelay);

  Serial.begin( 9600 );
  Serial.println("Mini CNC Plotter alive and kicking!");
  Serial.print("X range is from ");
  Serial.print(Xmin);
  Serial.print(" to ");
  Serial.print(Xmax);
  Serial.println(" mm.");
  Serial.print("Y range is from ");
  Serial.print(Ymin);
  Serial.print(" to ");
  Serial.print(Ymax);
  Serial.println(" mm.");
}

void loop()
{
  //Manual homing of motors to (0,0)
  if (digitalRead(Control) == LOW)
  {
    int c = digitalRead(xlimit);
    int d = digitalRead(ylimit);
    if ((c == 0) && (d == 0))
    {
      xstepper_rev(origin);
    }
    else if ((c == 0) && (d == 1))
    {
      xstepper_rev(away);
    }
    else if ((c == 1) && (d == 0))
    {
      ystepper_rev(origin);
    }
    else if ((c == 1) && (d == 1))
    {
      ystepper_rev(away);
    }
  }
  
//********** Handwriting **************
  else if (digitalRead(Control) == HIGH)
  {
    // Serial reception
    while ( Serial.available() > 0 ) //if data is received from the g code sender
    {
      char e = Serial.read();   //read the data
      line[lineIndex] = e;  //store the data in the array

      //increment the index so that  the next char is stored in the next position
      lineIndex = lineIndex + 1;  

      //check if a new line or return catridge is received - indicates end of line
      if ((e == '\n') || (e == '\r'))
      {
        line[lineIndex] = '\0'; //add a null character in th end to indicate end of char array
        
        Serial.print("Received :");
        Serial.println(line);
        Serial.print("lineIndex =");
        Serial.println(lineIndex);

        //check if it is a g code or a M code
        if ((line[0] == 'G') || (line[0] == 'M'))
        {
          //process the g code
          gcode_processing(line);

          //make the index to 0
          lineIndex = 0;

          //send "OK" to indicate that processing is done
          //and the machine is ready to receive the next code
          Serial.println("ok");
        }
      }
    }
  }
}

//**************** Processing Gcode**************
void gcode_processing(char* line)
{
  //initialise the index to zero
  int currentIndex = 0;
  
  //temporary array
  
  if (line[currentIndex] == 'G')  // if it is a G code
  {
    //read the next two indices to check the code and copy it into temp array
    temp[0] = line[currentIndex + 1]; 
    temp[1] = line[currentIndex + 2];

    //add null char to temp array
    temp[2] = '\0'; 
    
    Serial.print("G Command: ");
    Serial.println(temp);

    //convert the values of the temp array to integer
    int code = atoi(temp);
    
    Serial.print("code =");
    Serial.println(code);

    //check the code
    if ((code == 1) || (code == 2) || (code == 3))
    {
      //if the code is 1 or 2 or 3
      //find the position of  X, Y, F in the array
      char* xIndex = strchr(line, 'X');
      char* yIndex = strchr(line, 'Y');
      char* FIndex = strchr(line, 'F');

      //convert the values after X,Y, F index to float to check the cordinates
      float newXpos = atof(xIndex + 1);
      float newYpos = atof(yIndex + 1);
      float Fvalue = atof(FIndex + 1);
      
      Serial.print("Fvalue =");
      Serial.println(Fvalue);
      Serial.print("xIndex =");
      Serial.println(xIndex);
      Serial.print("yIndex =");
      Serial.println(yIndex);
      Serial.print("newXpos, newYpos =");
      Serial.print("(");
      Serial.print(newXpos);
      Serial.print(",");
      Serial.print(newYpos);
      Serial.println(")");

      //if there is no F value,Draw
      if (Fvalue == 0)
      {
        draw(newXpos, newYpos);
      }
    }
    //else ingore the G code
    else
    {
      //ignore other gcodes
    }
  }
 // if it is a M code
  else if (line[currentIndex] == 'M')
  {
    //read the next two indices to check the code and copy it into temp array
    temp[0] = line[currentIndex + 1];
    temp[1] = line[currentIndex + 2];
    
    //add null char to temp array
    temp[2] = '\0';
    
    Serial.print("M Command: ");
    Serial.println(temp);

     //convert the values of the temp array to integer
    int code = atoi(temp);
    Serial.print("code =");
    Serial.println(code);
    //if the code is 3, the pen should write
    if (code == 03)
    {
      writing();
    }
    //if the code is 5, the pen should not write
    else
    {
      not_writing();
    }
  }
}



void draw(float x, float y)
{
  
  Serial.println("entered draw");
  if (x< Xmin)
  {
    x=Xmin;
  }
  if (x> Xmax)
  {
    x=Xmax;
  }
 if (y< Ymin)
  {
    y=Ymin;
  }
  if (y> Ymax)
  {
    y=Ymax;
  }
  //convert the millimeters into steps
  x = float(x * StepsPerMillimeterX);
  
  //convert the millimeters into steps
  y = float(y * StepsPerMillimeterY);

  //old positions -> initially zero
  float x0 = Xpos;
  float y0 = Ypos;

  //initialize change
  float ChangeInX = 0;
  float ChangeInY = 0;
  
  //if new x is greater than old x, move away from the origin
  if (x > x0) 
  {
    Serial.println("x>x0");
    xDir = away;
    Serial.print("X direction =");
    Serial.println(xDir);
    //calculate the change in x to find the number of steps
    ChangeInX = x - x0;
  }
  else  //if old x is greater than new x, move towards the origin
  {
    Serial.println("x0>x");
    xDir = origin;
    //calculate the change in x to find the number of steps
    ChangeInX = x0 - x;
    Serial.print("X direction =");
    Serial.println(xDir);
  }

  //if new y is greater than old y, move away from the origin
  if (y > y0)
  {
    Serial.println("y>y0");
    yDir = away;
    //calculate the change in y to find the number of steps
    ChangeInY = y - y0;
    Serial.print("Y direction =");
    Serial.println(yDir);
  }
  
  else //if old y is greater than new y, move towards the origin
  {
    Serial.println("y0>y");
    
    yDir = origin;
     //calculate the change in y to find the number of steps
    ChangeInY = y0 - y;
    Serial.print("Y direction =");
    Serial.println(yDir);
  }

  // print the number of steps
  Serial.print("Change in X =");
  Serial.println(ChangeInX);
  Serial.print("Change in Y =");
  Serial.println(ChangeInY);

  //if the change in x is more than the change in Y then move xstepper first and theny stepper
  if (ChangeInX >= ChangeInY)
  {
    for (float i = 0; i < ChangeInX; i++)
    {
      xstepper_rev(xDir);
      ChangeInY ++;
      if (ChangeInY > ChangeInX)
      {
        ChangeInY --;
        ystepper_rev(yDir);
      }
    }
  }

  //if the change in Y is more than the change in X then move xstepper first and theny stepper
  else if (ChangeInY > ChangeInX)
  {
    for (float i = 0; i < ChangeInY; i++)
    {
      ystepper_rev(yDir);
      ChangeInX ++;
      if (ChangeInX > ChangeInY)
      {
        ChangeInX --;
        xstepper_rev(xDir);
      }
    }
  }
  Serial.println("Finished Drawing");
  delay(150); //line delay
  //set the current values to xpos and ypos to use for next calculations
  Xpos = x;
  Ypos = y;
}