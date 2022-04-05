#include <TM1637.h>
//#include <SoftwareSerial.h>

#define J1 9
#define J2 8
#define J3 10
#define P0 2
#define P1 3
#define P2 14

#define CLK 13
#define DIO 12

//TM1637Display display(CLK, DIO);
TM1637 tm1637(CLK,DIO);
struct Peserta {
    volatile bool is_active;
    int score;
} peserta[3];

int modd(int x, int m) {
  if(x<0)
    return x+m;
  return x%m;
}

void decTeam(int team) {
  peserta[team].score--;
  if(peserta[team].score <= -9)
    peserta[team].score=-9;
}

void incTeam(int team) {
  peserta[team].score += 2;
  if(peserta[team].score >= 100)
    peserta[team].score=99;
}

void drawTeam(int team) {
 tm1637.clearDisplay();
 tm1637.display(team,10+team);
}
void drawActive() {
  tm1637.clearDisplay();
  if(peserta[0].is_active){
    tm1637.display(0,10);
  }
  if(peserta[1].is_active){
    tm1637.display(1,11);
  }
  if(peserta[2].is_active){
    tm1637.display(2,12);
  }
}
void drawTeamWithScore(int team) {
  tm1637.clearDisplay();
  tm1637.display(0,10+team);
  int score = peserta[team].score;
  if(score<0){
    tm1637.display(1,'-');
    score*=-1;
  }
  else
    tm1637.display(1,score/10);
  tm1637.display(2,score%10);
  
}

void drawTeams() {
  tm1637.clearDisplay();
  tm1637.display(0,10); // A
  tm1637.display(1,11); // b
  tm1637.display(2,12); // C
}

void drawTest(){
  tm1637.clearDisplay();
  tm1637.display(0,13); // A
  tm1637.display(1,14); // b
  tm1637.display(2,15); // C
}

volatile bool interrupt_active;

double last_isr_call;
bool checkBouncing() {
    double tmp = last_isr_call;
    last_isr_call = millis();
    return last_isr_call > tmp + 2000;
}

void setAllPesertaActive() {
    for(int i=0;i<3;i++)
        peserta[i].is_active = true;
}
int peserta_idx;
int answeringcount;


enum class STATE{GABUT, STANDBY, JUDGING, WA};
STATE program_state; // 0 standby, 1 answermode, 

void setup()
{
    program_state = STATE::GABUT;

    pinMode(P2,INPUT_PULLUP);
    pinMode(J1,INPUT_PULLUP);
    pinMode(J2,INPUT_PULLUP);
    pinMode(J3,INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(P0),ISRButton0,CHANGE);
    attachInterrupt(digitalPinToInterrupt(P1),ISRButton1,CHANGE);
    digitalWrite(P0,HIGH);
    digitalWrite(P1,HIGH);
    digitalWrite(P2,HIGH);
    PCICR |= B00000010; // We activate the interrupts of the PC port
    PCMSK1 |= B00000001; // We activate the interrupts on pin A0
//    PCICR |= (1<<PCIE1)
    for(int i=0;i<3;i++) {
      peserta[i].score = 0;
    }

    tm1637.init();
    tm1637.set(BRIGHT_TYPICAL);
}


void loop()
{
    if(true)
    switch(program_state) {
        case STATE::GABUT: // baru menyala -> gk ngapangapain sampe J1 ditekan
            answeringcount = 0;
            if (digitalRead(J1) == LOW) {
                setAllPesertaActive();
                interrupt_active = true;
                program_state = STATE::STANDBY;
                delay(500);
            }
            tm1637.clearDisplay();
            break;
        case STATE::STANDBY:
            if(digitalRead(J1) == LOW) {
                interrupt_active = false;
                /// show nilai here..
                drawTeamWithScore(0);
                delay(2000);
                drawTeamWithScore(1);
                delay(2000);
                drawTeamWithScore(2);
                delay(2000);
                interrupt_active = true;
            }
            if(!interrupt_active) {
                program_state = STATE::JUDGING;
            }
            // code to show teams here... abc.. kecuali yg gk boleh ngejawab..
            drawActive();
            break;
        case STATE::JUDGING:
            tm1637.clearDisplay();
//            tm1637.display(2,15); // F
            drawTeam(peserta_idx);
            if(digitalRead(J2) == LOW) { // correct
//                peserta[peserta_idx].score+=2;
                incTeam(peserta_idx);
                drawTeamWithScore(peserta_idx);
                delay(1000);
                program_state = STATE::GABUT;
                break;
            }
            if(digitalRead(J3) == LOW) {  //wrong
                answeringcount++;
//                peserta[peserta_idx].score--;
                decTeam(peserta_idx);
                peserta[peserta_idx].is_active=false;
                drawTeamWithScore(peserta_idx);
                delay(1000);
                interrupt_active = true;
                if(answeringcount < 2) {
                    program_state = STATE::STANDBY;
                }
                else {
                    program_state = STATE::GABUT;
                }
            }
//            if(digitalRead(J1) == LOW) {
//              program_state = STATE::GABUT;
//            }
            break;
        case STATE::WA:
            //if(digitalRead(J2) == HIGH) {
            //    peserta[peserta_idx].score += 1;
            //    setAllPesertaActive();
            //    program_state = STATE::GABUT;
            //}
            //else if(digitalRead(J3) == HIGH) {
            //    peserta[peserta_idx].score -= 1;
            //    interrupt_active = true;
            //    program_state = STATE::GABUT;
            //}
            break;
    }
//    drawTeams();
//    tm1637.clearDisplay();
}

void resetToStandby() {
    last_isr_call = 0;
    peserta_idx = 0;
    interrupt_active = true;
}

void ISRButton0() {
    if(checkBouncing()){
        if(interrupt_active) {
            if(peserta[0].is_active) {
                peserta_idx = 0;
                interrupt_active = false;
                peserta[0].is_active = false;
//                program_state = STATE::JUDGING;
                
            }
        }
//        drawTest();
//        delay(1000);
    }
}

void ISRButton1() {
    if(checkBouncing()) {
        if(interrupt_active) {
            if(peserta[1].is_active) {
                peserta_idx = 1;
                interrupt_active = false;
                peserta[1].is_active = false;
                
            }
        }
//        drawTest();
//        delay(1000);
    }
}

void ISRButton2() {
    if(checkBouncing()){
        if(interrupt_active) {
            if(peserta[2].is_active) {
                peserta_idx = 2;
                interrupt_active = false;
                peserta[2].is_active = false;
                program_state = STATE::JUDGING;
                delay(1000);
            }
        }
//        drawTest();
//        delay(1000);
    }
}


ISR (PCINT1_vect) {
    ISRButton2();
//    drawTest();
//    delay(1000);
}
