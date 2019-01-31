/* main.c --- 
 * 
 * Filename: main.c
 * Description: 
 * Author: Jill Pena
 * Maintainer: 
/* Code: */

#include <stm32f30x.h>  // Pull in include files for F30x standard drivers 
#include <f3d_led.h>     // Pull in include file for the local drivers
#include <f3d_uart.h>
#include <f3d_gyro.h>
#include <f3d_lcd_sd.h>
#include <f3d_i2c.h>
#include <f3d_i2c2.h>
#include <f3d_accel.h>
#include <f3d_mag.h>
#include <f3d_nunchuk.h>
#include <f3d_nunchuk2.h>
#include <f3d_rtc.h>
#include <f3d_systick.h>
#include <ff.h>
#include <diskio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <f3d_lcd_sd.h>
#include <f3d_timer2.h>
#include <f3d_dac.h>
#include <f3d_delay.h>
#include <players.h>
#define TIMER 20000
#define AUDIOBUFSIZE 128

extern uint8_t Audiobuf[AUDIOBUFSIZE];
extern int audioplayerHalf;
extern int audioplayerWhole;
int playAudio(char *);
struct Question{
  char question[100];
  char answer[25];
};
int askQ(int, int, int, int);

/**************Catagory 1 = Cities*************************
***************Category 2 = Countries********************/

//Category 1 for 100 points
struct Question c1_100[2]={{"This is the largest city in Illinois.", "chicago"},
                 {"This is the most populated city in America.", "new york city"}};

//Category 2 for 100 points
struct Question c2_100[2]={{"this is the largest country in the world", "russia"},
                            {"this is the country with the highest population density", "china"}};

//Category 1 for 200 points
struct Question c1_200[2]={{"This is the largest city in the midwest.", "chicago"},
                {"This city has a space needle.", "seattle"}};

//Category 2 for 200 points
struct Question c2_200[2]={{"By area, this is the fourth largest country in the world.", "china"},
                           {"By area, this is the second largest country in the world.","canada"}};

//Category 1 for 300 points
struct Question c1_300[2]={{"This city has the CNN World Head Quarters.", "atlanta"},
               {"This city has the headquarters of AT&T and Southwest Airlines.", "dallas"}};

//Category 2 for 300 points
struct Question c2_300[2]={{"The Yen is the standard currency of this country.", "japan"},
                {"The Rupee is the standard currency of this country.", "india" }};

//Category 1 for 400 points
struct Question c1_400[2]={{"By population, this is currently the fourth largest city in the United States.", "houston"},
               {"This city is the home of the Alamo.", "san antonio"}};

//Category 2 for 400 points
struct Question c2_400[2]={{"In 2015 this was the most common country of orgin for new immigrants coming into to the U.S.", "india"},
               {"This is the smallest country in the world.","vatican city"}};

//Category 1 for 500 points
struct Question c1_500[2]={{"Unless you count Indianapolis, this is the only state capital actually named after its state.", "oklahoma city"},
                {"This city is where Coca Cola was invented.", "atlanta"}};

//Category 2 for 500 points
struct Question c2_500[2]={{"This is the country with the most languages spoken.", "papa new guinea"},
               {"The third largest city in the world is located in this country.", "india"}};

//Category 1 for 600 points
struct Question c1_600[2]={{"This town has recently earned the nickname,  \"Where young people go to retire\"", "portland"},
               {"This city has the nickname Rip City", .answer= "portland"}};

//Category 2 for 600 points
struct Question c2_600[2]={{"This is the most land locked country.", "uzbekistan"},
               {"This country has the highest proportion of seats held by women in its national parliament.", "rwanda"}};

//**************************FINAL JEOPARDY**************************************************
struct Question final[2]={{"This city is named after a British Prime Minister.", "Pittsburgh"},
               {"This country has ten sheep to every one person", "New Zealand"}};
char *finalWrongCity[2] = {"Churchtown","Thatchland"};
char *finalWrongCountry[2] = {"Wales","Sri Lanka"};

FATFS Fatfs;		/* File system object */
FIL fid;		/* File object */
BYTE Buff[512];		/* File read buffer */
int ret;
int displayQ(int, int, int);

struct ckhd {
  uint32_t ckID;
  uint32_t cksize;
};

int updateScore(player_t *, int);

struct fmtck {
  uint16_t wFormatTag;      
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
};

void readckhd(FIL *fid, struct ckhd *hd, uint32_t ckID) {
  f_read(fid, hd, sizeof(struct ckhd), &ret);
  if (ret != sizeof(struct ckhd))
    exit(-1);
  if (ckID && (ckID != hd->ckID))
    exit(-1);
}

int handleHeight(int);
int handleLR(int);


int disabled[3][2] = {{0, 0}, {0, 0}, {0, 0}};


void die (FRESULT rc) {
  printf("Failed with rc=%u.\n", rc);
  while (1);
}

int allDisabled();
int clearTable();

int highlight(int, int); // changes highlighted category and value

  FRESULT rc;			/* Result code */
  DIR dir;			/* Directory object */
  FILINFO fno;			/* File information object */
  UINT bw, br;
  unsigned int retval;
  int bytesread;
  player_t player1, player2;
  nunchuk2_t nunchuk2;
  nunchuk_t nunchuk1;
int user = 0, prevUser = 0;
  int selection = 0;
int repeat = 0, change = 0, lr = 0, height = 0, sameQ = 0;
int rnd = 0;
int prevVal = 0, j;
int correct = 0;
int main(void) { 
  int endGame();

  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  f3d_uart_init();
  delay(10);
  printf("what\n");
  f3d_timer2_init();
  delay(10);
  f3d_dac_init();
    printf("dac\n");
  delay(10);
  f3d_delay_init();
  printf("delay\n");
  delay(10);
  f3d_rtc_init();
  printf("rtc\n");
  delay(10);
  f3d_systick_init();
  printf("tick\n");
  delay(10);
  f3d_i2c1_init();
  printf("i2c1\n");
  delay(10);
  f3d_i2c2_init();
  printf("i2c2 inint\n");
  delay(10);
  f3d_lcd_init();
  delay(10);
  f3d_nunchuk_init();
  printf("nunchuk1 inint\n");
  delay(10);
  f3d_nunchuk2_init();
  delay(10);
  f3d_systick_init();
  printf("nunchuk2 inint\n");
  printf("Reset\n");
  f3d_lcd_fillScreen(WHITE);
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_fillScreen(WHITE);
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_fillScreen(WHITE);
  delay(10);
  rc = f_mount(0, &Fatfs);/* Register volume work area */
  if(rc) {
    printf("problem at mount\n");
  }
  f3d_lcd_fillScreen(WHITE);  
  

  char *startMessage = "Press \'C\' on the nunchuk to buzz in, press \'Z\' on  the nunchuk to select a category. For the first round, the first person to buzz in will select the category.";
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_drawString(0,20, startMessage, YELLOW, BLUE);
  delay(10000);
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_drawString(15, 50, "Player 1, please", YELLOW, BLUE);
  f3d_lcd_drawString(40, 60, "enter a", YELLOW, BLUE);
  f3d_lcd_drawString(5, 70, "3-character nickname", YELLOW, BLUE);
  add_name(&player1);
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_drawString(5, 10, "P1: ", YELLOW, BLUE);
  f3d_lcd_drawString(25, 10, player1.name, YELLOW, BLUE);
  f3d_lcd_drawString(5, 20, "Score1:0",YELLOW, BLUE);
  f3d_lcd_drawString(15, 50, "Player 2, please",YELLOW, BLUE);
  f3d_lcd_drawString(40, 60, "enter a", YELLOW, BLUE);
  f3d_lcd_drawString(5, 70, "3-character nickname", YELLOW, BLUE);
  add_name(&player2);
  f3d_lcd_drawString(65, 10, "P2: ", YELLOW, BLUE);
  f3d_lcd_drawString(85, 10, player2.name, YELLOW, BLUE);
  f3d_lcd_drawString(5, 30, "Score2:0",YELLOW, BLUE);
  f3d_lcd_drawString(15, 50, "Player 2, please",BLUE, BLUE);
  f3d_lcd_drawString(40, 60, "enter a", BLUE, BLUE);
  f3d_lcd_drawString(5, 70, "3-character nickname", BLUE, BLUE);
  f3d_lcd_drawString(15, 50, "Cities", YELLOW, BLUE);
  f3d_lcd_drawString(65, 50, "Countries", YELLOW, BLUE);
  f3d_lcd_drawString(20, 65, "100", BLACK, BLUE);
  f3d_lcd_drawString(75, 65, "100", YELLOW, BLUE);
  f3d_lcd_drawString(20, 90, "200", YELLOW, BLUE);
  f3d_lcd_drawString(75, 90, "200", YELLOW, BLUE);
  f3d_lcd_drawString(20, 115, "300", YELLOW, BLUE);
  f3d_lcd_drawString(75, 115, "300", YELLOW, BLUE);
  while(1) {
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk_read(&nunchuk1);
    if(nunchuk1.c && !nunchuk2.c) {
      user = 1;
      printf("Player 1 buzzed in first\n");
      playAudio("Buzzer.wav");
    }
    if(nunchuk2.c && !nunchuk1.c) {
      user = 2;
      printf("Player 2 buzzed in first\n");
      playAudio("Buzzer.wav");
    }
    if(user)
      break;
  }
  prevUser = user;
  while (1) {
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk_read(&nunchuk1); // maybe condtional checking for rnd == 2 to avoid following same steps as other rounds
    if(rnd == 2) {
      finalJeopardy();
    }
    else {
      if(prevUser == 1) {
	if(nunchuk1.jx >= 170) {
	  sameQ = 0;
	  lr = handleLR(1);
	}
	if(nunchuk1.jy >= 170) {
	  sameQ = 0;
	  height = handleHeight(1);
	}
	if(nunchuk1.jy <= 76) {
	  sameQ = 0;
	  height = handleHeight(0);
	}
	if(nunchuk1.jx <= 76) {
	  sameQ = 0;
	  lr = handleLR(0);
	}
	highlight(lr, height);
	if(nunchuk1.z) {
	  displayQ(lr, height, rnd);
	}
      }
      if(prevUser == 2) {
	if(nunchuk2.jx >= 170) {
	  sameQ = 0;
	  lr = handleLR(1);
	}
	if(nunchuk2.jy >= 170) {
	  sameQ = 0;
	  height = handleHeight(1);
	}
	if(nunchuk2.jy <= 76) {
	  sameQ = 0;
	  height = handleHeight(0);
	}
	if(nunchuk2.jx <= 76) {
	  sameQ = 0;
	  lr = handleLR(0);
	}
	highlight(lr, height);
	if(nunchuk2.z) {
	  displayQ(lr, height, rnd);
	}
      }
    }
  }
}

int playAudio(char *target) {
  printf("\nOpen %s\n", target);
  rc = f_open(&fid, target, FA_READ);
  if (rc) die(rc);
  
  if (!rc) {
    struct ckhd hd;
    uint32_t  waveid;
    struct fmtck fck;
    
    readckhd(&fid, &hd, 'FFIR');
    
    f_read(&fid, &waveid, sizeof(waveid), &ret);
    if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
      return -1;
    
    readckhd(&fid, &hd, ' tmf');
    
    f_read(&fid, &fck, sizeof(fck), &ret);
    
    // skip over extra info
    
    if (hd.cksize != 16) {
      printf("extra header info %d\n", hd.cksize - 16);
      f_lseek(&fid, hd.cksize - 16);
    }
    
    printf("audio format 0x%x\n", fck.wFormatTag);
    printf("channels %d\n", fck.nChannels);
    printf("sample rate %d\n", fck.nSamplesPerSec);
    printf("data rate %d\n", fck.nAvgBytesPerSec);
    printf("block alignment %d\n", fck.nBlockAlign);
    printf("bits per sample %d\n", fck.wBitsPerSample);
    
    // now skip all non-data chunks !
    
    while(1){
      readckhd(&fid, &hd, 0);
      if (hd.ckID == 'atad')
	break;
      f_lseek(&fid, hd.cksize);
    }
    
    printf("Samples %d\n", hd.cksize);
    
    // Play it !
    
    //    audioplayerInit(fck.nSamplesPerSec);
    
    f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
    hd.cksize -= ret;
    audioplayerStart();
    while (hd.cksize) {
      int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
      if (audioplayerHalf) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(Audiobuf, AUDIOBUFSIZE/2);
	f_read(&fid, Audiobuf, next, &ret);
	hd.cksize -= ret;
	audioplayerHalf = 0;
      }
      if (audioplayerWhole) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	hd.cksize -= ret;
	audioplayerWhole = 0;
      }
    }
    audioplayerStop();
  }
  
  printf("\nClose the file.\n"); 
  rc = f_close(&fid);
  
  if (rc) die(rc);
}



 int displayQ(int lr, int height, int r) {
   int num = (rand() & 1);
   int i;
   uint16_t blues[128];
   for(i = 0; i < 128; i++)
     blues[i] = BLUE;
   printf("in displayQ\n");
   switch(rnd) {
   case 0:
     if(!lr) {
       switch(height) {
       case 0:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_100[num]).question, YELLOW, BLUE);
	 break;
       case 1:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_200[num]).question, YELLOW, BLUE);
	 break;
	 break;
       case 2:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_300[num]).question, YELLOW, BLUE);
	 break;
	 break;
       }
     }
     else {
       switch(height) {
       case 0:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_100[num]).question, YELLOW, BLUE);
	 break;
       case 1:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_200[num]).question, YELLOW, BLUE);
	 break;
	 break;
       case 2:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_300[num]).question, YELLOW, BLUE);
	 break;
	 break;
       }
     }
     break;
   case 1:
     if(!lr) {
       switch(height) {
       case 0:
	 printf("should be doing stuff now\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_400[num]).question, YELLOW, BLUE);
	 break;
	 break;
       case 1:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_500[num]).question, YELLOW, BLUE);
	 break;
	 break;
       case 2:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c1_600[num]).question, YELLOW, BLUE);
	 break;
	 break;
       }
     }
     else {
       switch(height) {
       case 0:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_400[num]).question, YELLOW, BLUE);
	 break;
       case 1:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_500[num]).question, YELLOW, BLUE);
	 break;
	 break;
       case 2:
	 printf("should be doing stuff nbow\n");
	 // f3d_lcd_setAddrWindow(0, 20, 128, 160, MADCTLGRAPHICS);
	 for(i = 0; i < 120; i++) {
	   f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	   f3d_lcd_pushColor(blues, 128);
	 }
	 f3d_lcd_drawString(0, 50, (c2_600[num]).question, YELLOW, BLUE);
	 break;
	 break;
       }
     }
     break;
   case 2: //************Initial Final Jeopardy question/request for each player to enter wager points**************
	break;
   }
   askQ(lr, height, rnd, num);
 }

int enable1 = 0, enable2 = 0, done = 0, k, m; 
int timeUp;
int askQ(int lr, int height, int r, int num){
  printf("In askQ now\n");
  int i;
  uint16_t blues[128];
  for(i = 0; i < 128; i++)
    blues[i] = BLUE;
  char ans[100];
  i = 0;
  char *cor;
  int score = 0;
  switch(height) {
  case 0: score = 100; break;
  case 1: score = 200; break;
  case 2: score = 300; break;
  }
  if(rnd == 1)
    score = score + 300;
  switch(rnd) {
  case 0:
    if(!lr)
      switch(height) {
      case 0:
	cor = &c1_100[num].answer;
	break;
      case 1:
	cor = &c1_200[num].answer;
	break;
      case 2:
	cor = &c1_300[num].answer;
	break;
      }
    else {
      switch(height) {
      case 0:
	cor = &c2_100[num].answer;
	break;
      case 1:
	cor = &c2_200[num].answer;
	break;
      case 2:
	cor = &c2_300[num].answer;
	break;
      }
    }
    break;
  case 1: 
    if(!lr)
      switch(height) {
      case 0:
	cor = &c1_400[num].answer;
	break;
      case 1:
	cor = &c1_500[num].answer;
	break;
      case 2:
	cor = &c1_600[num].answer;
	break;
      }
    else {
      switch(height) {
      case 0:
	cor = &c2_400[num].answer;
	break;
      case 1:
	cor = &c2_500[num].answer;
	break;
      case 2:
	cor = &c2_600[num].answer;
	break;
      }
    }
    break;
  case 2: // determine what value is correct for final jeopardy, and also how to avoid doing same protocol as previous rounds
    break;
  }
  int prevVal = 0;
  SysTick_Config(SystemCoreClock / 1);
  timeUp = 0;
  while(1) {
    if(timeUp == 100) {
      playAudio("Wrong.wav");
      if(!prevVal || user == 0) {
	done = 1;
	for(i = 0; i < 120; i++) {
	  f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	  f3d_lcd_pushColor(blues, 128);
	}
	f3d_lcd_drawString(0, 50, "Correct Answer:", YELLOW, BLUE);
	f3d_lcd_drawString(0, 60, cor, YELLOW, BLUE);
	disabled[height][lr] = 1;
	if(allDisabled()) {
	  rnd++;
	  clearTable();
	} 
	delay(1000);
	resetScreen();
	enable1 = 0;
	enable2 = 0;
	user = prevUser;
      }	
    }
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    if(nunchuk1.c && !prevVal && !nunchuk2.c && !enable1) {
      SysTick_Config(SystemCoreClock / 1);
      timeUp = 0;
      user = 1;
      prevVal = 1;
      playAudio("Buzzer.wav");
      i = 0;
      printf("Nunchuk one pressed first\n");
    }
    else if(nunchuk2.c && !nunchuk1.c && !prevVal && !enable2) {
      SysTick_Config(SystemCoreClock / 1);
      timeUp == 0;
      user = 2;
      prevVal = 1;
      playAudio("Buzzer.wav");
      printf("Nunchuk two was first\n");
      i = 0;
    }
    if(prevVal) {
      ans[i] = getchar();
      if(ans[i] >= 65 && ans[i] <= 90) {
	ans[i] = ans[i] + 32;
      }
      printf("\nInput char: ");
      putchar(ans[i]);
      if(ans[i] == '\n' || i > 99) {
	timeUp = 0;
	for(i = 0; i < 100; i++) {
	  if(ans[i] == '\n' && cor[i] == 0x00) {
	    prevVal = 0;
	    enable1 = 0;
	    enable2 = 0;
	    playAudio("Correct.wav");
	    for(i = 0; i < 120; i++) {
	      f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
	      f3d_lcd_pushColor(blues, 128);
	    }
	    f3d_lcd_drawString(0, 50, "Correct!", YELLOW, BLUE);
	    f3d_lcd_drawString(0, 60, cor, YELLOW, BLUE);
	    disabled[height][lr] = 1;
	    for(k = 0; k < 3; k++) {
	      for(m = 0; m < 2; m++) {
		if(!disabled[k][m]) {
		  height = k;
		  lr = m;
		}
	      }
	    }
	    if(user == 1) {
	      player1.score+= score;
	      printf("%d\n", player1.score);
	      updateScore(&player1, 1);
	    }
	    else if(user == 2){
	      if(!enable2)
		player2.score+= score;
	      updateScore(&player2, 2);
	    }
	    prevUser = user;
	    if(allDisabled()) {
	      rnd++;
	      clearTable();
	    }
	    delay(1000);
	    resetScreen();
	    done = 1;
	    break;
	  }  
	  else if(cor[i] != ans[i]) {
	    prevVal = 0;
	    if(user == 1) {
	      player1.score-=score;
	      updateScore(&player1, 1);
	      enable1 = 1;
	      playAudio("Wrong.wav");
	      user = 0;
	      if(enable2) {
		for(i = 0; i < 120; i++) {
		  f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
		  f3d_lcd_pushColor(blues, 128);
		}
		f3d_lcd_drawString(0, 50, "Correct Answer:", YELLOW, BLUE);
		f3d_lcd_drawString(0, 60, cor, YELLOW, BLUE);
		disabled[height][lr] = 1;
	      	if(allDisabled()) {
		  rnd++;
		  clearTable();
		  } 
		delay(1000);
		resetScreen();
		enable1 = 0;
		enable2 = 0;
		user = prevUser;
		done = 1;
	      }
	    }
	    else {
	      player2.score-=score;
	      enable2 = 1;
	      updateScore(&player2, 2);
	      playAudio("Wrong.wav");
	      user = 0;
	      if(enable1) {
		for(i = 0; i < 120; i++) {
		  f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
		  f3d_lcd_pushColor(blues, 128);
		}
		f3d_lcd_drawString(0, 50, "Correct Answer:", YELLOW, BLUE);
		f3d_lcd_drawString(0, 60, cor, YELLOW, BLUE);
		disabled[height][lr] = 1;
		if(allDisabled()) {
		  rnd++;
		  clearTable();
		  } 
		delay(1000);
		resetScreen();
		enable1 = 0;
		enable2 = 0;
		user = prevUser;
		done = 1;
	      }
	    }
	    break;
	  }
	}
      }
      else {
      }
      i++;
 }
    if(done) {
      done = 0;
      break;
    }
  }
}

int updateScore(player_t * play, int u) {
  char score[5];
  sprintf(score, "%d", play->score);
  if(u == 1) {
    f3d_lcd_drawString(5, 20, "Score1:00000",BLUE, BLUE);
    f3d_lcd_drawString(5, 20, "Score1:",YELLOW, BLUE);
    f3d_lcd_drawString(50, 20, score,YELLOW, BLUE);
  }
  else {
    f3d_lcd_drawString(5, 30, "Score2:00000",BLUE, BLUE);
    f3d_lcd_drawString(5, 30, "Score2:",YELLOW, BLUE);
    f3d_lcd_drawString(50, 30, score,YELLOW, BLUE);
  }
}

int resetScreen() {
  int i;
  uint16_t blues[128];
  for(i = 0; i < 128; i++)
    blues[i] = BLUE;
  for(i = 0; i < 120; i++) {
    f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
    f3d_lcd_pushColor(blues, 128);
  }
  if(rnd == 0) {
    f3d_lcd_drawString(15, 50, "Cities", YELLOW, BLUE);
    f3d_lcd_drawString(65, 50, "Countries", YELLOW, BLUE);
    if(!disabled[0][0])
      f3d_lcd_drawString(20, 65, "100", YELLOW, BLUE);
    if(!disabled[0][1])
      f3d_lcd_drawString(75, 65, "100", YELLOW, BLUE);
    if(!disabled[1][0])
      f3d_lcd_drawString(20, 90, "200", YELLOW, BLUE);
    if(!disabled[1][1])
      f3d_lcd_drawString(75, 90, "200", YELLOW, BLUE);
    if(!disabled[2][0])
      f3d_lcd_drawString(20, 115, "300", YELLOW, BLUE);
    if(!disabled[2][1])
      f3d_lcd_drawString(75, 115, "300", YELLOW, BLUE);
  }
  if(rnd == 1) {
    f3d_lcd_drawString(15, 50, "Cities", YELLOW, BLUE);
    f3d_lcd_drawString(65, 50, "Countries", YELLOW, BLUE);
    if(!disabled[0][0])
      f3d_lcd_drawString(20, 65, "400", YELLOW, BLUE);
    if(!disabled[0][1])
      f3d_lcd_drawString(75, 65, "400", YELLOW, BLUE);
    if(!disabled[1][0])
      f3d_lcd_drawString(20, 90, "500", YELLOW, BLUE);
    if(!disabled[1][1])
      f3d_lcd_drawString(75, 90, "500", YELLOW, BLUE);
    if(!disabled[2][0])
      f3d_lcd_drawString(20, 115, "600", YELLOW, BLUE);
    if(!disabled[2][1])
      f3d_lcd_drawString(75, 115, "600", YELLOW, BLUE);
  }
  highlight(lr, height);
}

int handleLR(int left_right) { 
  int tmp = lr;
  if(lr) {
    if(!disabled[height][0]) { // if the object to the left is not disabled, move left
      return 0;
    }
    else {
      lr = 0;
      if( !disabled[handleHeight(1)][0]) {
	height = handleHeight(1);
	return 0;
      }
      else if( !disabled[handleHeight(0)][0]) {
	height = handleHeight(0);
	return 0;
      }
    }
  }
  else {
    if(!disabled[height][1]) { // repeat same for to the right
      return 1;
    }
    else {
      lr = 1;
      if( !disabled[handleHeight(1)][1]) {
	height = handleHeight(1);
	return 1;
      }
      else if( !disabled[handleHeight(0)][1]) {
	height = handleHeight(0);
	return 1;
      }
    }
  }
  lr = tmp;
  if(left_right == lr && disabled[height][lr]) {
    for(k = 0; k < 3; k++) {
      for(m = 0; m < 2; m++) {
	if(!disabled[k][m]) {
	  height = k;
	  lr = m;
	}
      }
    }
  }
  return lr; 
}

int handleHeight(int up_down) { // 1 = up, 0 = down
  if(up_down) {
    if(height == 0) {
      if(!disabled[2][lr])
	return 2;
      else if(!disabled[1][lr])
	return 1;
    }
    else {
      if(height == 2){
	if(!disabled[1][lr])
	  return 1;
	else if(!disabled[0][lr])
	  return 0;
      }
      if(height == 1) {
	if(!disabled[0][lr])
	  return 0;
	else if(!disabled[2][lr])
	  return 2;
      }
    }
  }
  else {
    if(height == 0) {
      if(!disabled[1][lr])
	return 1;
      else if(!disabled[2][lr])
	return 2;
    }
    else {
      if(height == 2){
	if(!disabled[0][lr])
	  return 0;
	else if(!disabled[1][lr])
	  return 1;
      }
      if(height == 1) {
	if(!disabled[2][lr])
	  return 2;
	else if(!disabled[1][lr])
	  return 1;
      }
    }
  }
  if(up_down == height && disabled[height][lr]) {
    for(k = 0; k < 3; k++) {
      for(m = 0; m < 2; m++) {
	if(!disabled[k][m]) {
	  height = k;
	  lr = m;
	}
      }
    }
  }
  return height;
}

int highlight(int l, int h) {
  if(!sameQ) {
    sameQ++;
  if(!rnd) {
      f3d_lcd_drawString(15, 50, "Cities", YELLOW, BLUE);
      f3d_lcd_drawString(65, 50, "Countries", YELLOW, BLUE);
      if(!disabled[0][0])
	f3d_lcd_drawString(20, 65, "100", YELLOW, BLUE);
      if(!disabled[0][1])
	f3d_lcd_drawString(75, 65, "100", YELLOW, BLUE);
      if(!disabled[1][0])
	f3d_lcd_drawString(20, 90, "200", YELLOW, BLUE);
      if(!disabled[1][1]) 
	f3d_lcd_drawString(75, 90, "200", YELLOW, BLUE);
      if(!disabled[2][0]) 
      f3d_lcd_drawString(20, 115, "300", YELLOW, BLUE);
      if(!disabled[2][1]) 
      f3d_lcd_drawString(75, 115, "300", YELLOW, BLUE);
      switch(h) {
      case 0:
	if(!l)
	  f3d_lcd_drawString(20, 65, "100", BLACK, BLUE);
	else
	  f3d_lcd_drawString(75, 65, "100", BLACK, BLUE);
	break;
      case 1:
	if(!l)
	  f3d_lcd_drawString(20, 90, "200", BLACK, BLUE);
	else
	  f3d_lcd_drawString(75, 90, "200", BLACK, BLUE);
	break;
      case 2:
	if(!l)
	  f3d_lcd_drawString(20, 115, "300", BLACK, BLUE);
	else
	  f3d_lcd_drawString(75, 115, "300", BLACK, BLUE);
	break;
      }
  }
  if(rnd == 1) {
    f3d_lcd_drawString(15, 50, "Cities", YELLOW, BLUE);
    f3d_lcd_drawString(65, 50, "Countries", YELLOW, BLUE);
    if(!disabled[0][0])
      f3d_lcd_drawString(20, 65, "400", YELLOW, BLUE);
    if(!disabled[0][1])
      f3d_lcd_drawString(75, 65, "400", YELLOW, BLUE);
    if(!disabled[1][0])
      f3d_lcd_drawString(20, 90, "500", YELLOW, BLUE);
    if(!disabled[1][1])
      f3d_lcd_drawString(75, 90, "500", YELLOW, BLUE);
    if(!disabled[2][0])
      f3d_lcd_drawString(20, 115, "600", YELLOW, BLUE);
    if(!disabled[2][1])
      f3d_lcd_drawString(75, 115, "600", YELLOW, BLUE);
    switch(h) {
    case 0:
      if(!l)
	f3d_lcd_drawString(20, 65, "400", BLACK, BLUE);
      else
	f3d_lcd_drawString(75, 65, "400", BLACK, BLUE);
      break;
    case 1:
      if(!l)
	f3d_lcd_drawString(20, 90, "500", BLACK, BLUE);
      else
	f3d_lcd_drawString(75, 90, "500", BLACK, BLUE);
      break;
    case 2:
      if(!l)
	f3d_lcd_drawString(20, 115, "600", BLACK, BLUE);
      else
	f3d_lcd_drawString(75, 115, "600", BLACK, BLUE);
      break;
    }
  }
  else { //**********For Final Jeopardy*******************
    }
  }
}

int allDisabled() {
  int i, j;
  for(i = 0; i < 3; i++) {
    for(j = 0; j < 2; j++)
      if(!disabled[i][j])
	return 0;
  }
  return 1;
}

int clearTable() {
  int i, j;
  for(i = 0; i < 3; i++) {
    for(j = 0; j < 2; j++)
      disabled[i][j] = 0;
  }
}

int powTen(int power) {
  if(power == 0)
    return 1;
  else
    return 10 * powTen(power - 1);
}

int finalJeopardy(char * cor, int height){
  int timeUp;
  f3d_lcd_fillScreen(BLUE);
  int num = (rand() & 1);
  int pos = (rand() % 3);
  int i;
  uint16_t blues[128];
  for(i = 0; i < 128; i++)
    blues[i] = BLUE;
  //***********collect wager amount*****************
  int wager_player1[10];
  int enterPlayer1 = 0;
  int enterPlayer2 = 0;
  int wager_player2[10];
  char *correct;
  int finished = 0, enable1 = 0, enable2 = 0;
  updateScore(&player1, 1);
  updateScore(&player2, 2);
  
  f3d_lcd_drawString(0, 50, "Final Jeopardy!", YELLOW, BLUE);
  f3d_lcd_drawString(0, 60, "Click Nunchuk button C and then type wager.", YELLOW, BLUE);
  f3d_lcd_drawString(0, 70, "The default wager is 1000. You may not wager 0, or more than what you have.", YELLOW, BLUE);
  f3d_lcd_drawString(0, 110, "Any attempt to do so will result in a wager of 1000.", YELLOW, BLUE);
  char c;
  int wager1 = 0, wager2 = 0, j, l;
  while (1) {
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    if(nunchuk1.c && !nunchuk2.c && !enterPlayer1)
      {
	printf("Nunchuk one pressed\n");
	playAudio("Buzzer.wav");
	printf("Player One's wager\n");
	int i = 0;
	l = 0;
	c = ' ';
	enterPlayer1 = 1;
	while(i < 10 && c != '\n'){
	  c = getchar();
	  putchar(c);
	  if(c >= 48 && c <=57) {
	    wager_player1[i] = c;
	    i++;
	  }
	}
	l = i - 1;
	for(j = 0; j < i; j++) {
	  wager1+= powTen(j) * (wager_player1[l] - 48);
	  l--;
	}
	printf("Wager 1: %d\n", wager1);
	if((wager1 > 1000 && player1.score < wager1) || wager1 == 0) {
	  wager1 = 1000;
	}
	printf("Wager 1: %d\n", wager1);
      }
    else if(nunchuk2.c && !nunchuk1.c && !enterPlayer2) {
      printf("Nunchuk two was pressed\n");
      playAudio("Buzzer.wav");
      printf("Player Two's wager\n");
      int i = 0;
      l = 0;
      c = ' ';
      enterPlayer2 = 1;
	while(i < 10 && c != '\n'){
	  c = getchar();
	  putchar(c);
	  if(c >= 48 && c <=57) {
	    wager_player2[i] = c;
	    i++;
	  }
	}
	l = i - 1;
	for(j = 0; j < i; j++) {
	  wager2+= powTen(j) * (wager_player2[l] - 48);
	  l--;
	}
	if((wager2 > 1000 && player2.score < wager2) || wager2 == 0) {
	  wager2 = 1000;
	}
	printf("Wager 2: %d\n", wager2);
    }
      
      if (enterPlayer1 && enterPlayer2){
	break;
      }
  }
  
  //***********************Display Final Jeopardy Question******************
  
  /* for(i = 0; i < 120; i++) {
    f3d_lcd_setAddrWindow(0, 40 + i, 127, 20 + i + 1, MADCTLGRAPHICS);
    f3d_lcd_pushColor(blues, 128);
    } */
  f3d_lcd_fillScreen(BLUE);
  f3d_lcd_drawString(0, 10, (final[num]).question, YELLOW, BLUE);
  correct = &final[num].answer;
  
  //***********************Collect Answer**********************************
  int p1Level = 0, p1Prev = 0;
  int p2Level = 0, p2Prev = 0;
  char *fakeAns1, *fakeAns2;
  if(num) {
    fakeAns1 = finalWrongCountry[0];
    fakeAns2 = finalWrongCountry[1];
  }
  else {
    fakeAns1 = finalWrongCity[0];
    fakeAns2 = finalWrongCity[1];
  }
    
  f3d_lcd_drawString(0, 60,fakeAns1, YELLOW, BLUE);
  f3d_lcd_drawString(0, 80, correct, YELLOW, BLUE);
  f3d_lcd_drawString(0, 100, fakeAns2, YELLOW, BLUE);
  f3d_lcd_drawString(50, 70, fakeAns1, YELLOW, BLUE);
  f3d_lcd_drawString(50, 90, correct, YELLOW, BLUE);
  f3d_lcd_drawString(50, 110, fakeAns2, YELLOW, BLUE);
  SysTick_Config(SystemCoreClock/1);
  timeUp = 0;
  while(1) {
    printf("Looking for input to shift highlight\n");
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk_read(&nunchuk1);

    //display 2 columns of multiple choice question highlighting
    if(timeUp == 100) {
      if(!enable1){
	player1.score-=wager1;
      }
      if(!enable2) {
	player2.score-=wager2;
      }
    }
    if(nunchuk1.jy >= 170 && !enable1) {
      printf("Changing nunchuk1 highlight\n");
      p1Prev = 1;
      if(p1Level == 0)
	p1Level = 2;
      else
	p1Level--;
    }
    if(nunchuk1.jy <= 76 && !enable1) {
      printf("Changing nunchuk1 highlight\n");
      p1Prev = 1;
      if(p1Level == 2)
	p1Level = 0;
      else
	p1Level++;
    }
    if(nunchuk2.jy >= 170 && !enable2) {
     printf("Changing nunchuk2 highlight\n");
      p2Prev = 1;
      if(p2Level == 0)
	p2Level = 2;
      else
	p2Level--;
    }
    if(nunchuk2.jy <= 76 && !enable2) {
     printf("Changing nunchuk2 highlight\n");
      p2Prev = 1;
      if(p2Level == 2)
	p2Level = 0;
      else
	p2Level++;
    }
    if(p1Prev) {
      p1Prev = 0;
      switch(p1Level) {
      case 0:
	f3d_lcd_drawString(0, 60, fakeAns1, BLACK, BLUE);
	f3d_lcd_drawString(0, 80, correct, YELLOW, BLUE);
	f3d_lcd_drawString(0, 100, fakeAns2, YELLOW, BLUE);
	break;
      case 1:
	f3d_lcd_drawString(0, 60, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(0, 80, correct, BLACK, BLUE);
	f3d_lcd_drawString(0, 100, fakeAns2, YELLOW, BLUE);
	break;
      case 2:
	f3d_lcd_drawString(0, 60, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(0, 80, correct, YELLOW, BLUE);
	f3d_lcd_drawString(0, 100, fakeAns2, BLACK, BLUE);
	break;
      default:
	f3d_lcd_drawString(0, 60, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(0, 80, correct, YELLOW, BLUE);
	f3d_lcd_drawString(0, 100, fakeAns2, YELLOW, BLUE);
	break;
      }
    }
    if(p2Prev) {
      p2Prev = 0;
      switch(p2Level) {
        case 0:
	f3d_lcd_drawString(50, 70, fakeAns1, BLACK, BLUE);
	f3d_lcd_drawString(50, 90, correct, YELLOW, BLUE);
	f3d_lcd_drawString(50, 110, fakeAns2, YELLOW, BLUE);
	break;
      case 1:
	f3d_lcd_drawString(50, 70, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(50, 90, correct, BLACK, BLUE);
	f3d_lcd_drawString(50, 110, fakeAns2, YELLOW, BLUE);
	break;
      case 2:
	f3d_lcd_drawString(50, 70, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(50, 90, correct, YELLOW, BLUE);
	f3d_lcd_drawString(50, 110, fakeAns2, BLACK, BLUE);
	break;
      default:
	f3d_lcd_drawString(50, 70, fakeAns1, YELLOW, BLUE);
	f3d_lcd_drawString(50, 90, correct, YELLOW, BLUE);
	f3d_lcd_drawString(50, 110, fakeAns2, YELLOW, BLUE);
	break;
      }
    }
  
  
  
  
  
  //collect nunchuk data & see who selects correct question first
    if(nunchuk1.c && !finished && !enable1) {
      p1Prev = 1;
      if(p1Level == 1) {
	playAudio("Correct.wav");
	player1.score+=wager1;
	finished = 1;
	if(!enable2)
	  player2.score-=wager2;
      }
      else {
	player1.score-=wager1;
	enable1 = 1;
	p1Level = 3;
      }
    }
    if(nunchuk2.c && !finished && !enable2) {
      p2Prev = 1;
      if(p2Level == 1) {
	playAudio("Correct.wav");
	player2.score+=wager2;
	finished = 1;
	if(!enable1) {
	  player1.score-=wager1;
	}
      }
      else {
	player2.score-=wager2;
	enable2 = 1;
	p2Level = 2;
      }
    }
  //if person selects wrong question disable highlighting column and continue to collect nunchuk data
  
  
  //if player selects correct question or both players columns are disabled, end game
    if(finished || (enable1 && enable2)) {
      endGame();
      break;
    }
  }
}
 

int endGame(){
  //display scores
  int winner;
  f3d_lcd_fillScreen(BLUE);
  if(player1.score > player2.score) {
    winner = 1;
    f3d_lcd_drawString(0, 50, "The winner is: ", YELLOW, BLUE);
    f3d_lcd_drawString(80, 50, player1.name, YELLOW, BLUE);
    updateScore(&player1, 1);
    updateScore(&player2, 2);
  }
  else if(player2.score > player1.score) {
    winner = 2;
    f3d_lcd_drawString(0, 50, "The winner is: ", YELLOW, BLUE);
    f3d_lcd_drawString(80, 50, player2.name, YELLOW, BLUE); 
    updateScore(&player2, 2);
    updateScore(&player1, 1);
  }
  else {
    winner = prevUser;
    f3d_lcd_drawString(10, 50, "The game is a draw!", YELLOW, BLUE);
    updateScore(&player1, 1);
    updateScore(&player2, 2);
  }
  f3d_lcd_drawString(0, 80, "Would you like to play again? (C on the nunchuk if yes)", YELLOW, BLUE);
  while(1) {
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk_read(&nunchuk1);
    f3d_nunchuk2_read(&nunchuk2);
    f3d_nunchuk2_read(&nunchuk2);
    if(nunchuk1.c || nunchuk2.c) {
      rnd = 0;
      player1.score = 0;
      player2.score = 0;
      user = winner;
      resetScreen();
      updateScore(&player1, 1);
      updateScore(&player2, 2);
      f3d_lcd_drawString(5, 10, "P1: ", YELLOW, BLUE);
      f3d_lcd_drawString(25, 10, player1.name, YELLOW, BLUE);
      f3d_lcd_drawString(65, 10, "P2: ", YELLOW, BLUE);
      f3d_lcd_drawString(85, 10, player2.name, YELLOW, BLUE);
      break;
    }
  }
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  /* Infinite loop */
  /* Use GDB to find out why we're here */
  while (1);
}
#endif


/* main.c ends here */
