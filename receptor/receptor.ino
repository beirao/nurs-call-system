#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <UTFTGLUE.h>
#include "Manchester.h"

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define RX_PIN 11
#define BUTTON 12
#define BUZZER 13
MCUFRIEND_kbv tft;

#define MAX_ELEMENTS_FILE 100
#define MAX_ELEMENTS_ROOMS_TAB 10
#define NB_REMOTE 50
#define DELTA_ACQUISITION 4

// Global vars
struct Element
{
    int number;
    int timestamp;
    Element *next;
};
typedef struct Element Element;


struct File
{
    int size;
    Element *premier;
};

typedef struct Pile Pile;
struct Pile
{
    Element *premier;
};
typedef struct File File;


int boolBuzzer = false;
int tabRooms[MAX_ELEMENTS_ROOMS_TAB];
int tabRoomsIsUpdate[MAX_ELEMENTS_ROOMS_TAB];
int lastTabRooms[MAX_ELEMENTS_ROOMS_TAB];
File *fileResponse = malloc(sizeof(struct File));
int timestamp = 0;
int secondeTrigger = 0;
int millisecondeTrigger = 0;

void setup() {
  Serial.begin(115200);
  man.setupReceive(RX_PIN, MAN_1200);
  man.beginReceive();

  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON, INPUT);

  initScreen();
  buzzer();

  fileResponse->premier = NULL;


  // initialisation fileResponse
  for (int i = 0; i < MAX_ELEMENTS_FILE; i++) {
    enfiler(fileResponse,0,2);
  }
  Serial.println("");
  Serial.println("fileResponse : ");
  displayFile(fileResponse);

  for(int i ; i < MAX_ELEMENTS_ROOMS_TAB ; i++) {
    tabRooms[i] = 0;
    tabRoomsIsUpdate[i] = 1;   
  }
  timestamp = 0;
  secondeTrigger = 0;
  millisecondeTrigger = 0;


  Serial.println("\n Rooms call order : ");
  displayTab(tabRooms);
  Serial.println("\n eTabRoomsIsUpdate : ");
  displayTab(tabRoomsIsUpdate);
}

void loop() {
  if(digitalRead(BUTTON) && testTabRoomsIsUpdate()){
    for (int j = 0; j < MAX_ELEMENTS_ROOMS_TAB; j++){
      tabRoomsIsUpdate[j] = 1;    
    }  
    updateTabRooms(fileResponse, tabRooms);
    setCamaDisplay();
    isTabUpdated();
    boolBuzzer = false;
  }

  if (secondeTrigger > 120) {
    timestamp++;
    // Serial.println(timestamp);

    secondeTrigger = 0;
    if(boolBuzzer){
      buzzer();
    }

    // if (timestamp % DELTA_ACQUISITION == 0) {
    updateTabRooms(fileResponse, tabRooms);
    setCamaDisplay();
    isTabUpdated();

    // Serial.println("");
    // Serial.println("updateTabRooms !");
    // displayFile(fileResponse);     
    // Serial.println("");   
    // Serial.println("-----");
    // Serial.println("Rooms call order : ");
    // displayTab(tabRooms);
    // Serial.println("");   
    // Serial.println("-----");
    // Serial.println("tabRoomsIsUpdate : ");
    // displayTab(tabRoomsIsUpdate);
    // Serial.println("");
    // Serial.println("-----");
    // Serial.println("timestamp :");
    // Serial.println(timestamp);  
    // }
  }

  if (millisecondeTrigger > 1000) {
    secondeTrigger++;
    millisecondeTrigger = 0;
  }

  // update bool buzzer
  boolBuzzer = false;
  for (int i = 0 ; i < MAX_ELEMENTS_ROOMS_TAB; i++){
    if(tabRoomsIsUpdate[i] != 1){
      boolBuzzer = true;
      break;
    }
  }
  
  
  if (timestamp > 60*60*24*7){ // reset all week
    timestamp = 0;

    for (int i = 0; i<MAX_ELEMENTS_FILE; i++) {
      enfiler(fileResponse,0,2);
    }
    // Serial.println("");
    // Serial.println("fileResponse : ");
    // displayFile(fileResponse);

    for(int i ; i < MAX_ELEMENTS_ROOMS_TAB ; i++) {
      tabRooms[i] = 0;
    }
  }
  millisecondeTrigger++;
  
  if (man.receiveComplete()) {
    int temp = man.getMessage();
    Serial.println(temp);
    man.beginReceive(); //start listening for next message right after you retrieve the message

    if(temp <= NB_REMOTE){
      enfiler(fileResponse,temp,timestamp);

      // DEBUG
      // Serial.println("");
      // Serial.println("fileResponse : ");
      // displayFile(fileResponse);       
      // Serial.println("");
      // Serial.print("Seconde : ");
      // Serial.println(timestamp);
    }
    else if (temp > NB_REMOTE && temp <= 2*NB_REMOTE){
      for (int i = 0 ; i < MAX_ELEMENTS_ROOMS_TAB; i++){
        if(tabRoomsIsUpdate[i] != 1 && tabRooms[i] == (temp-NB_REMOTE)){
          tabRoomsIsUpdate[i] = 1;
        }
      }
    }
  }
}

// void setCamaDisplay(){
//     if(tabRoomsIsUpdate[0] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(90,2,90,60,BLACK);
//     tft.setCursor(100,12);
//     tft.setTextSize(6);
//     tft.println(tabRooms[0]);

//     if(tabRoomsIsUpdate[1] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(20,40,80,50,BLACK);
//     tft.setCursor(30,50);
//     tft.setTextSize(4);
//     tft.println(tabRooms[1]);
    
//     if(tabRoomsIsUpdate[2] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(150,70,80,50,BLACK);
//     tft.setCursor(160,80);
//     tft.setTextSize(4);
//     tft.println(tabRooms[2]);

//     if(tabRoomsIsUpdate[3] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(20,100,80,50,BLACK);
//     tft.setCursor(30,110);
//     tft.setTextSize(4);
//     tft.println(tabRooms[3]);
    
//     if(tabRoomsIsUpdate[4] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(150,130,80,50,BLACK);
//     tft.setCursor(160,140);
//     tft.setTextSize(4);
//     tft.println(tabRooms[4]);

//     if(tabRoomsIsUpdate[5] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(20,160,80,50,BLACK);
//     tft.setCursor(30,170);
//     tft.setTextSize(4);
//     tft.println(tabRooms[5]);
    
//     if(tabRoomsIsUpdate[6] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(150,190,80,50,BLACK);
//     tft.setCursor(160,200);
//     tft.setTextSize(4);
//     tft.println(tabRooms[6]);
    
//     if(tabRoomsIsUpdate[7] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(20,220,80,50,BLACK);
//     tft.setCursor(30,230);
//     tft.setTextSize(4);
//     tft.println(tabRooms[7]);

//     if(tabRoomsIsUpdate[8] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(150,250,80,50,BLACK);
//     tft.setCursor(160,260);
//     tft.setTextSize(4);
//     tft.println(tabRooms[8]);

//     if(tabRoomsIsUpdate[9] == 1) tft.setTextColor(WHITE);
//     else                         tft.setTextColor(RED);
//     tft.fillRect(20,280,80,50,BLACK);
//     tft.setCursor(30,290);
//     tft.setTextSize(4);
//     tft.println(tabRooms[9]);
    

//     tft.setTextColor(WHITE);
// }

void setCamaDisplay(){
    if(tabRoomsIsUpdate[0] == 1) tft.setTextColor(WHITE);
    else                         tft.setTextColor(RED);
    tft.fillRect(100,10,80,60,BLACK);
    tft.setCursor(110,20);
    tft.setTextSize(6);
    tft.println(tabRooms[0]);

    if(tabRoomsIsUpdate[1] == 1) tft.setTextColor(WHITE);
    else                         tft.setTextColor(RED);
    tft.fillRect(100,95,70,50,BLACK);
    tft.setCursor(110,105);
    tft.setTextSize(4);
    tft.println(tabRooms[1]);
    
    if(tabRoomsIsUpdate[2] == 1) tft.setTextColor(WHITE);
    else                         tft.setTextColor(RED);
    tft.fillRect(100,155,70,50,BLACK);
    tft.setCursor(110,165);
    tft.setTextSize(4);
    tft.println(tabRooms[2]);

    if(tabRoomsIsUpdate[3] == 1) tft.setTextColor(WHITE);
    else                         tft.setTextColor(RED);
    tft.fillRect(100,215,70,50,BLACK);
    tft.setCursor(110,225);
    tft.setTextSize(4);
    tft.println(tabRooms[3]);
    
    if(tabRoomsIsUpdate[4] == 1) tft.setTextColor(WHITE);
    else                         tft.setTextColor(RED);
    tft.fillRect(100,275,70,50,BLACK);
    tft.setCursor(110,285);
    tft.setTextSize(4);
    tft.println(tabRooms[4]);

    tft.setTextColor(WHITE);
}

bool testTabRoomsIsUpdate(){
  bool ret = false;

  for (int j = 0; j < MAX_ELEMENTS_ROOMS_TAB; j++){
    if(tabRoomsIsUpdate[j] == 0) {
      ret = true;
    }    
  }
  return ret;
}

bool isTabUpdated() {
  bool ret = true;
  for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB; i++){
    if(tabRooms[i] != lastTabRooms[i]) {
      ret = false;
      for (int j = 0; j < MAX_ELEMENTS_ROOMS_TAB; j++){
        lastTabRooms[j] = tabRooms[j];      
      }   
      break;
    }
  }
  return ret;
}

void updateTabRooms(File *file, int tabRooms[]){
    int sensibility = DELTA_ACQUISITION; //sec
    int lastTimestamp = file->premier->timestamp;
    int lastBed = 0;
    int nbIteration = 0;
    int trig = 0;
    int lastTabRoomsIsUpdate[MAX_ELEMENTS_ROOMS_TAB];

    Pile *pile = malloc(sizeof(*pile));
    pile->premier = NULL;

    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if (file->premier != NULL)
    {
        Element *temp = file->premier;
        while (temp != NULL)
        {
            if (lastBed != temp->number || !trig) {   
                empiler(pile, temp->number, temp->timestamp);
                lastBed = temp->number;
                lastTimestamp = temp->timestamp;

                trig = 1;         
     
            }  
            if(trig && mathAbs(temp->timestamp - lastTimestamp) > sensibility) {
                // Serial.println(lastTimestamp);
                lastBed = 0;
                trig = 0;
            }

            
            temp = temp->next;
        }
        //fill tabRooms
        for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB; i++){
          lastTabRoomsIsUpdate[i] = tabRoomsIsUpdate[i];
        }
        for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB; i++){
          tabRooms[i] = depiler(pile);
        }
        if (!isTabUpdated()) {
          tabRoomsIsUpdate[0] = 0;

          for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB-1; i++){
            if(lastTabRoomsIsUpdate[i] == 0){
              tabRoomsIsUpdate[i+1] = 0;
            }
            else{
              tabRoomsIsUpdate[i+1] = 1;
            }
          }
        }

        while (pile != NULL && pile->premier != NULL)
        {
          depiler(pile);
        }
        free(pile);
    }
}


void buzzer(){
  digitalWrite(BUZZER, HIGH); 
  delay(50);
  digitalWrite(BUZZER, LOW); 
}

void initScreen(){
  tft.reset();
  uint16_t identifier = tft.readID();
  Serial.println();
  Serial.println("------");
  Serial.print("ID = 0x");
  Serial.println(identifier, HEX);
  if (identifier == 0xEFEF) identifier = 0x9486;
  tft.begin(identifier);
  tft.fillScreen(BLACK);
  tft.setRotation(2);
  // tft.setCursor(80, 2);
  // tft.setTextSize(2);
  // tft.setTextColor(WHITE);
  // tft.println("Cama");

  tft.fillRect(0,90,240,1,WHITE);
  tft.fillRect(0,150,240,1,WHITE);
  tft.fillRect(0,210,240,1,WHITE);
  tft.fillRect(0,270,240,1,WHITE);
}

int mathAbs(int a) {
    if(a < 0) a=-a;
    return a;
}

// Functions for pile management
void empiler(Pile *pile, int nvNumber, int timestamp)
{
    Element *nouveau = malloc(sizeof(*nouveau));
    if (pile == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }

    nouveau->number = nvNumber;
    nouveau->timestamp = timestamp;
    nouveau->next = pile->premier;
    pile->premier = nouveau;
}

int depiler(Pile *pile)
{
    if (pile == NULL)
    {
        exit(EXIT_FAILURE);
    }

    int numberDepile = 0;
    Element *elementDepile = pile->premier;

    if (pile != NULL && pile->premier != NULL)
    {
        numberDepile = elementDepile->number;
        pile->premier = elementDepile->next;
        free(elementDepile);
    }

    return numberDepile;
}

void displayPile(Pile *pile)
{
    if (pile == NULL)
    {
        exit(EXIT_FAILURE);
    }
    Element *actuel = pile->premier;

    while (actuel != NULL)
    {
        printf("%d\n", actuel->number);
        actuel = actuel->next;
    }

    printf("\n");
}
int isInPile(Pile *pile, int val){
    if (pile == NULL)
    {
        exit(EXIT_FAILURE);
    }
    Element *actuel = pile->premier;

    while (actuel != NULL)
    {
        if (actuel->number == val){
            return 1;
        }
        actuel = actuel->next;
    }
    return 0;
}

// Functions for file management
void displayFile(File *file){
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if (file->premier != NULL) /* La file n'est pas vide */
    {
        /* On se positionne à la fin de la file */
        Element *elementActuel = file->premier;
        while (elementActuel != NULL)
        {
            Serial.print(elementActuel->number);
            elementActuel = elementActuel->next;
        }
    }
}

int defiler(File *file)
{
    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    // int ret[2] = {-1,-1};
    int nb = -1;

    /* On vérifie s'il y a quelque chose à défiler */
    if (file->premier != NULL)
    {
        Element *elementDefile = file->premier;

        nb = elementDefile->number;

        file->premier = elementDefile->next;
        file->size--;
        free(elementDefile);
    }
    return nb;
}

void enfiler(File *file, int nvNumber, int timestamp)
{
    Element *nouveau = malloc(sizeof(*nouveau));
    if (file == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }

    nouveau->number = nvNumber;
    nouveau->timestamp = timestamp;
    nouveau->next = NULL;
    // Serial.print(**(file->premier));

    if (file->premier != NULL) /* La file n'est pas vide */
    {

        /* On se positionne à la fin de la file */
        Element *elementActuel = file->premier;
        while (elementActuel->next != NULL)
        {
            elementActuel = elementActuel->next;
        }
        elementActuel->next = nouveau;
        file->size++;

        // defile if the file size exceeds MAX_ELEMENTS_FILE
        if(file->size > MAX_ELEMENTS_FILE) {
            defiler(file);
        }
    }
    else /* La file est vide, notre élément est le premier */
    {
      file->premier = nouveau;
      file->size = 1;
    }
}

// Functions for tab management
void displayTab(int tab[]){
    for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB; i++)
    {
        Serial.print(tab[i]);
    }
}

