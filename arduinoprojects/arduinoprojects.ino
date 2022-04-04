#include <TM1637.h>

#define J1 10
#define J2 11
#define J3 12

struct Peserta {
    volatile bool is_active;
    int score;
} peserta[3];

volatile bool interrupt_active;

double last_isr_call;
bool checkBouncing() {
    double tmp = last_isr_call;
    last_isr_call = millis();
    return last_isr_call > tmp + 150
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
}

void loop()
{
    switch(program_state) {
        case STATE::GABUT: // baru menyala -> gk ngapangapain sampe J1 ditekan
            answeringcount = 0;
            if (digitalRead(J1) == HIGH) {
                setAllPesertaActive();
                interrupt_active = true;
                program_state = STATE::STANDBY;
            }
            break;
        case STATE::STANDBY:
            if(digitalRead(J1) == HIGH) {
                interrupt_active = false;
                /// show nilai here..
                interrupt_active = true;
            }
            if(!interrupt_active) {
                program_state = STATE::JUDGING;
            }
            // code to show teams here... abc.. kecuali yg gk boleh ngejawab..
            break;
        case STATE::JUDGING:
            if(digitalRead(J2) == HIGH) {
                peserta[peserta_idx].score++;
                program_state = STATE::GABUT;
                break;
            }
            if(digitalRead(J3) == HIGH) {
                answeringcount++;
                peserta[peserta_idx].score--;
                interrupt_active = true;
                if(answeringcount < 2) {
                    program_state = STATE::STANDBY;
                }
                else {
                    program_state = STATE::GABUT;
                }
            }
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
}

void resetToStandby() {
    last_isr_call = 0;
    peserta_idx = 0;
    interrupt_active = true;
}

void ISRButton0() {
    if(checkBouncing())
        if(interrupt_active) {
            if(peserta[0].is_active) {
                peserta_idx = 0;
                interrupt_active = false;
                peserta[0].is_active = false;
            }
        }
}

void ISRButton1() {
    if(checkBouncing())
        if(interrupt_active) {
            if(peserta[1].is_active) {
                peserta_idx = 1;
                interrupt_active = false;
                peserta[1].is_active = false;
            }
        }
}

void ISRButton2() {
    if(checkBouncing())
        if(interrupt_active) {
            if(peserta[2].is_active) {
                peserta_idx = 2;
                interrupt_active = false;
                peserta[2].is_active = false;
            }
        }
}


ISR (PCINT2_vect) {
    ISRButton2();
}