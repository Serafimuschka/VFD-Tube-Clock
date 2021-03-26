#define HR_INCR   4
#define MN_INCR   5
#define SC_INCR   6

#define CLK       7
#define REG_HR_H  8
#define REG_HR_L  9
#define REG_MN_H  10
#define REG_MN_L  11
#define REG_SC_H  12
#define REG_SC_L  13

#define IV3A_1    0
#define IV3A_2    1
#define IV3A_3    2
#define IV3A_4    3
#define IV3A_5    4
#define IV3A_6    5
#define IV3A_10   6
#define IV3A_P    7

byte              counter(0);

bool              hrLatch(false),
                  mnLatch(false),
                  scLatch(false);

unsigned short    timer(0),
                  secs(0),
                  mins(0),
                  hrs(0);

bool*             segmatrix;                  

void decodeNumber(byte in, byte addr, bool* &matrix)
{
  bool mx[8]{ false };
  switch (in)
  {
  case 0:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_4] = true;
    mx[IV3A_5] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  case 1:
    mx[IV3A_1] = true;
    mx[IV3A_10] = true;
    break;
  case 2:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_5] = true;
    mx[IV3A_6] = true;
    break;
  case 3:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  case 4:
    mx[IV3A_1] = true;
    mx[IV3A_3] = true;
    mx[IV3A_4] = true;
    mx[IV3A_10] = true;
    break;
  case 5:
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_4] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  case 6:
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_4] = true;
    mx[IV3A_5] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  case 7:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_10] = true;
    break;
  case 8:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_4] = true;
    mx[IV3A_5] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  case 9:
    mx[IV3A_1] = true;
    mx[IV3A_2] = true;
    mx[IV3A_3] = true;
    mx[IV3A_4] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  default:
    mx[IV3A_1] = true;
    mx[IV3A_3] = true;
    mx[IV3A_5] = true;
    mx[IV3A_6] = true;
    mx[IV3A_10] = true;
    break;
  }
  for (byte i = addr; i < addr + 8; i++)
    matrix[i] = mx[i - addr];
}

void setup() 
{
  segmatrix = new bool[48]{ false };

  for (byte i = HR_INCR; i <= SC_INCR; i++)
    pinMode(i, INPUT);
  
  for (byte i = CLK; i <= REG_SC_L; i++) 
    pinMode(i, OUTPUT);
    
  TCCR0A = (1 << WGM01);
  OCR0A = 0xF9;

  TIMSK0 |= (1 << OCIE0A);
  sei();

  TCCR0B |= (1 << CS01);
  TCCR0B |= (1 << CS00);
}

void loop() 
{
  // msec counter
  if (timer >= 1000)
  {
    timer = 0;
    secs++;
    if (secs == 60)
    {
      secs = 0;
      mins++;
      if (mins == 60)
      {
        mins = 0;
        hrs++;
        if (hrs == 24)
        {
          hrs = 0;
          mins = 0;
          secs = 0;
        }
      }
    }

    // convert time to matrix
    decodeNumber((hrs / 10) % 10, 0, segmatrix);
    decodeNumber(hrs % 10, 8, segmatrix);
    decodeNumber((mins / 10) % 10, 16, segmatrix);
    decodeNumber(mins % 10, 24, segmatrix);
    decodeNumber((secs / 10) % 10, 32, segmatrix);
    decodeNumber(secs % 10, 40, segmatrix);
  }
  
  // time adjusting
  bool  h = digitalRead(HR_INCR),
        m = digitalRead(MN_INCR),
        s = digitalRead(SC_INCR);
        
  if ((hrLatch | mnLatch | scLatch) & !((h ^ m) ^ s))
  {
    hrLatch = false;
    mnLatch = false;
    scLatch = false;
  }
        
  else if ((h ^ m) ^ s)
  {
    if (!hrLatch & !mnLatch & !scLatch)
    {
      if (h)
      {
        hrs++;
        hrLatch = true;
        decodeNumber((hrs / 10) % 10, 0, segmatrix);
        decodeNumber(hrs % 10, 8, segmatrix);
      }
      else if (m)
      {
        mins++;
        mnLatch = true;
        decodeNumber((mins / 10) % 10, 16, segmatrix);
        decodeNumber(mins % 10, 24, segmatrix);
      }
      else if (s)
      {
        secs++;
        scLatch = true;
        decodeNumber((secs / 10) % 10, 32, segmatrix);
        decodeNumber(secs % 10, 40, segmatrix);
      }
    }
    timer = 0;
  }
  
  // shift registers output
  if (!(timer % 125))
  {
    digitalWrite(CLK, HIGH);
    digitalWrite(REG_HR_H, segmatrix[0 + counter]);
    digitalWrite(REG_HR_L, segmatrix[8 + counter]);
    digitalWrite(REG_MN_H, segmatrix[16 + counter]);
    digitalWrite(REG_MN_L, segmatrix[24 + counter]);
    digitalWrite(REG_SC_H, segmatrix[32 + counter]);
    digitalWrite(REG_SC_L, segmatrix[40 + counter]);
    digitalWrite(CLK, LOW);
    counter++;
    if (counter == 8) counter = 0;
  }
}

ISR(TIMER0_COMPA_vect)
{
  timer++;
}
