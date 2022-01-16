#ifndef PID_v1_h
#define PID_v1_h
#define LIBRARY_VERSION 1.0.0

class PID
{
public:

   //Constants used in some of the functions below
   #define AUTOMATIC  1
   #define MANUAL  0
   #define DIRECT  0
   #define REVERSE  1

   //commonly used functions **************************************************************************
   PID(double Kp, double Ki, double Kd, int ControllerDirection = DIRECT);

   double GetITerm()
   {
      return(ITerm);
   }

   bool *GetDebugPointer()
   {
      return(&m_debug);
   }

   void SetMode(int Mode);               // * sets PID to either Manual (0) or Auto (non-0)

   double Compute(double input);                       // * performs the PID calculation.  it should be
                                       //   called every time loop() cycles. ON/OFF and
                                       //   calculation frequency can be set using SetMode
                                       //   SetSampleTime respectively

   void SetTarget(double target);

   void SetOutputLimits(double Min, double Max);

   void SetTunings(double Kp, double Ki, double Kd);

   void SetControllerDirection(int Direction);
   void SetSampleTime(int NewSampleTime);  // * sets the frequency, in Milliseconds, with which 
                                           //   the PID calculation is performed.  default is 100
                                
  //Display functions ****************************************************************
   double GetKp();                    // These functions query the pid for interal values.
   double GetKi();                    //  they were created mainly for the pid front-end,
   double GetKd();                    // where it's important to know what is actually 
   int GetMode();                  //  inside the PID.
   int GetDirection();                //
   double GetOutput();

private:
   void Initialize();
   
   double dispKp;          // * we'll hold on to the tuning parameters in user-entered 
   double dispKi;          //   format for display purposes
   double dispKd;          //
    
   double kp;                  // * (P)roportional Tuning Parameter
   double ki;                  // * (I)ntegral Tuning Parameter
   double kd;                  // * (D)erivative Tuning Parameter

   int controllerDirection;

   double m_input;              // * Pointers to the Input, Output, and Setpoint variables
   double m_output;             //   This creates a hard link between the variables and the 
   double m_target;           //   PID, freeing the user from having to constantly tell us
                                  //   what these values are.  with pointers we'll just know.

   bool  m_debug;           
   unsigned long lastTime;
   double ITerm, lastInput;

   unsigned long SampleTime;
   double outMin, outMax;
   bool inAuto;
};
#endif

