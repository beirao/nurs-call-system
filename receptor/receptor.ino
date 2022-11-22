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
#define MAX_ELEMENTS_ROOMS_TAB 5
#define NB_REMOTE 255
#define DELTA_ACQUISITION 3

// Global vars
typedef struct Element Element;
struct Element
{
    int nombre;
    int timestamp;
    Element *suivant;
};

typedef struct File File;
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

int boolBuzzer = false;
int tabRooms[MAX_ELEMENTS_ROOMS_TAB];
int tabRoomsIsUpdate[MAX_ELEMENTS_ROOMS_TAB];
int lastTabRooms[MAX_ELEMENTS_ROOMS_TAB];
File *fileResponse = malloc(sizeof(*fileResponse));
int triggerUpdate = 0;
int timestamp = 0;
int secondeTrigger = 0;
int millisecondeTrigger = 0;
int nbCamaUpdate = 0;

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

  Serial.println("");
  Serial.println("Rooms call order : ");
  displayTab(tabRooms);
  Serial.println("");
  Serial.println("TabRoomsIsUpdate : ");
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
    nbCamaUpdate = 0;
  }

  if (secondeTrigger > 250) {
    timestamp++;
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
    man.beginReceive(); //start listening for next message right after you retrieve the message
    enfiler(fileResponse,temp,timestamp);

    // DEBUG
    Serial.println("");
    Serial.println("fileResponse : ");
    displayFile(fileResponse);       
    Serial.println("");
    Serial.print("Seconde : ");
    Serial.println(timestamp);
  }
}

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
      boolBuzzer = true;
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
    int lastTimestamp = 0;
    int lastBed = 0;
    int nbIteration = 0;
    int trig = 1;

    Pile *pile = malloc(sizeof(*pile));
    pile->premier = NULL;

    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    if (file->premier != NULL)
    {
        Element *temp = file->premier;
        while (temp->suivant != NULL)
        {
            if (temp->nombre == lastBed && mathAbs(temp->timestamp - lastTimestamp) <= sensibility) {
                nbIteration++;
                if(nbIteration >= 2 && trig) {
                    empiler(pile, temp->nombre, temp->timestamp);
                    nbIteration = 0;
                    trig = 0;
                }
            }
            else {
                nbIteration = 0;
                lastTimestamp = temp->timestamp;
                lastBed = temp->nombre;
                trig = 1;
            }
            temp = temp->suivant;
        }
        //fill tabRooms
        for (int i = 0; i < MAX_ELEMENTS_ROOMS_TAB; i++){
          tabRooms[i] = depiler(pile);
        }
        if (!isTabUpdated() && nbCamaUpdate < 5) {
          nbCamaUpdate++;

          for (int i = 0; i < nbCamaUpdate; i++){
            tabRoomsIsUpdate[i] = 0;
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
  delay(100);
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
void empiler(Pile *pile, int nvNombre, int timestamp)
{
    Element *nouveau = malloc(sizeof(*nouveau));
    if (pile == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }

    nouveau->nombre = nvNombre;
    nouveau->timestamp = timestamp;
    nouveau->suivant = pile->premier;
    pile->premier = nouveau;
}

int depiler(Pile *pile)
{
    if (pile == NULL)
    {
        exit(EXIT_FAILURE);
    }

    int nombreDepile = 0;
    Element *elementDepile = pile->premier;

    if (pile != NULL && pile->premier != NULL)
    {
        nombreDepile = elementDepile->nombre;
        pile->premier = elementDepile->suivant;
        free(elementDepile);
    }

    return nombreDepile;
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
        printf("%d\n", actuel->nombre);
        actuel = actuel->suivant;
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
        if (actuel->nombre == val){
            return 1;
        }
        actuel = actuel->suivant;
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
            Serial.print(elementActuel->nombre);
            elementActuel = elementActuel->suivant;
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

        nb = elementDefile->nombre;

        file->premier = elementDefile->suivant;
        file->size--;
        free(elementDefile);
    }
    return nb;
}

void enfiler(File *file, int nvNombre, int timestamp)
{
    Element *nouveau = malloc(sizeof(*nouveau));
    if (file == NULL || nouveau == NULL)
    {
        exit(EXIT_FAILURE);
    }

    nouveau->nombre = nvNombre;
    nouveau->timestamp = timestamp;
    nouveau->suivant = NULL;
    // Serial.print(**(file->premier));

    if (file->premier != NULL) /* La file n'est pas vide */
    {

        /* On se positionne à la fin de la file */
        Element *elementActuel = file->premier;
        while (elementActuel->suivant != NULL)
        {
            elementActuel = elementActuel->suivant;
        }
        elementActuel->suivant = nouveau;
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

