#include <MD_MAX72xx.h>
#include <SPI.h>
#define DEBUG 1   // Enable or disable (default) debugging output

#if DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }      // Print a string followed by a value (decimal)
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); } // Print a string followed by a value (hex)
#define PRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); } // Print a string followed by a value (binary)
#define PRINTC(s, v)  { Serial.print(F(s)); Serial.print((char)v); }  // Print a string followed by a value (char)
#define PRINTS(s)     { Serial.print(F(s)); }                       // Print a string
#else
#define PRINT(s, v)   // Print a string followed by a value (decimal)
#define PRINTX(s, v)  // Print a string followed by a value (hex)
#define PRINTB(s, v)  // Print a string followed by a value (binary)
#define PRINTC(s, v)  // Print a string followed by a value (char)
#define PRINTS(s)     // Print a string
#endif

// --------------------
// MD_MAX72xx hardware definitions and object
// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
//
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 12
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);                      // SPI hardware interface
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins

#define ANIMATION_DELAY 75  // milliseconds



int gamemap[8][8];
void resetMap() {
  for(int i=0;i<8;i++){
    for(int j=0;j<8;j++){
      gamemap[i][j] = 0;
    }
  }
}




//int dirlst[] = {{1,0},{0,1},{-1,0},{0,-1}};

int hamiltonian[8][8] = {{1 ,2 ,3 ,4 ,21,22,23,24},
                         {0 ,7 ,6 ,5 ,20,27,26,25},
                         {63,8 ,17,18,19,28,29,30},
                         {62,9 ,16,35,34,33,32,31},
                         {61,10,15,36,37,38,39,40},
                         {60,11,14,45,44,43,42,41},
                         {59,12,13,46,47,48,49,50},
                         {58,57,56,55,54,53,52,51}};
int flatten_hamiltonian[64][3];


void buildFlattenHamiltonian() {
  for(int i=0;i<8;i++)
    for(int j=0;j<8;j++)
    {
      flatten_hamiltonian[hamiltonian[i][j]][0] = i;
      flatten_hamiltonian[hamiltonian[i][j]][1] = j;
      flatten_hamiltonian[hamiltonian[i][j]][2] = 0;
    }
}

struct Snake {
    int pos_i;
    int pos_j;
    int pos_hamilt;
    struct Snake* next;
};

struct Snake memory[64];
int MEMORY_IDX;

void resetSnake() {
  for(int i=0;i<64;i++)
  {
    memory[i].next = nullptr;
    memory[i].pos_hamilt = -1;
    memory[i].pos_i = -1;
    memory[i].pos_j = -1;
  }
  MEMORY_IDX = -1;
}

struct Snake* createSnake() {
  if (MEMORY_IDX>=63)
    return nullptr;
  struct Snake* ret = memory + (++MEMORY_IDX);

  //make sure it's clean
  ret->next = nullptr;
  ret->pos_hamilt = -1;
  ret->pos_i = -1;
  ret->pos_j = -1;

  return ret;
}

struct Snake* head;
struct Snake* tail;
int food;
void drawSnake() {
  struct Snake* tmp = head;
  while(tmp != nullptr) {
    gamemap[tmp->pos_i][tmp->pos_j] = 1;
    tmp = tmp->next;
  }
}

void drawFood() {
  int food_i = flatten_hamiltonian[food][0];
  int food_j = flatten_hamiltonian[food][1];
  gamemap[food_i][food_j] = 1;
}


void updateBodiesPosition(struct Snake* start, int pos_i, int pos_j) {
  if(start == nullptr)
    return;
  int pos_before_i = start->pos_i;
  int pos_before_j = start->pos_j;
  flatten_hamiltonian[start->pos_hamilt][2] = 0;
  start->pos_i = pos_i;
  start->pos_j = pos_j;
  start->pos_hamilt = hamiltonian[pos_i][pos_j];
  flatten_hamiltonian[start->pos_hamilt][2] = 1;
  updateBodiesPosition(start->next, pos_before_i, pos_before_j);
}

int decideDirection() { // returns hamiltonian position
  int head_pos_hamilt = head->pos_hamilt;
  int hamiltonian_distance_to_food = food - head_pos_hamilt;
  if(head_pos_hamilt > food)
    hamiltonian_distance_to_food += 64;

  return 0;

}

void moveSnake(int di, int dj){
  int newpos_i = head->pos_i + di;
  int newpos_j = head->pos_j + dj;
  if(newpos_i >= 8) return;
  if(newpos_j >= 8) return;
  int new_hamilt_pos = hamiltonian[newpos_i][newpos_j];
  if (food == new_hamilt_pos)
  {
    struct Snake* newhead = createSnake();
    newhead->pos_hamilt = food;
    newhead->pos_i = newpos_i;
    newhead->pos_j = newpos_j;
    newhead->next = head;
    head = newhead;
    do{
      food = random(64);
    }while(gamemap[flatten_hamiltonian[food][0]][flatten_hamiltonian[food][1]] == 1);
    return;
  } 
  updateBodiesPosition(head,head->pos_i+di,head->pos_j+dj);
}

void draw() {
  for(int i=0;i<8;i++)
    for(int j=0;j<8;j++) {
      if(gamemap[i][j])
        mx.setPoint(i,j,true);
    }
}


void resetMatrix(void)
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY/2);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
}

void updateSnake() {

  //follow hamiltonian (reverse order)
  int moveto_hamilt = head->pos_hamilt-1;
  if(moveto_hamilt<0)moveto_hamilt = 63;
  int moveto_i = flatten_hamiltonian[moveto_hamilt][0];
  int moveto_j = flatten_hamiltonian[moveto_hamilt][1];
  int di = (moveto_i-head->pos_i);
  int dj = (moveto_j-head->pos_j);
  moveSnake(di,dj);
}

void setup()
{
  randomSeed(analogRead(0));
  resetMap();
  buildFlattenHamiltonian();
  tail = createSnake();
  tail->pos_i = 0;
  tail->pos_j = 0;
  tail->pos_hamilt = hamiltonian[0][0];
  tail->next  = nullptr;
  head = createSnake();
  head->pos_i = 1;
  head->pos_j = 0;
  head->pos_hamilt = hamiltonian[1][0];
  head->next = tail;

  food = hamiltonian[4][0];
  mx.begin();
  resetMatrix();
  //prevTimeAnim = millis();
  #if DEBUG
  Serial.begin(57600);
  #endif
  //PRINTS("\n[MD_MAX72XX Pacman]");
}

void loop(void)
{
  
//  moveSnake(1,0);
  //follow hamiltonian (reverse order)


  #if DEBUG
  Serial.print(head->pos_i);
  Serial.print(tail->pos_i);
  Serial.print(head->next->pos_i);
  if(head->next->next == nullptr)
    Serial.print("only2|");
//  Serial.print(moveto_hamilt);
  Serial.print("|");
  Serial.print(head->pos_hamilt);
  Serial.println();
  #endif

  updateSnake();
  
  resetMap();
  drawSnake();
  drawFood();
  resetMatrix();
  draw();
  delay(100);
  //for(int i=0;i<8;i++)
  //  for(int j=0;j<8;j++)
  //  {
  //    resetMatrix();
  //    mx.setPoint(i,j,true);    
  //    mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON); 
  //    delay(200);
  //  }
  //return;
}
