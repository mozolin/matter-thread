#include <ctype.h> //-- toupper, tolower...
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "OSAL.h"

#if USE_LCD
	#include "hal_oled.h"
#endif
#include "cc2530_io_ports.h"

uint8 DEBUG = 1;

/***********************************
 * convert String to Integer #1    *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~    *
 * example:                        *
 *   char* str1 = "-5678";         * 
 *   num1 = str2int(str1);         *
 *   printf("num1 = %d\n", num1);  *
 *   | num1 = -5678                *
 ***********************************/
int str2int(char* str)
{
  int num;
  sscanf(str, "%d", &num);
  return num;
}
double str2float(char* str)
{
  return atof(str);
}
/***********************************
 * convert String to Integer #2    *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~    *
 * example:                        *
 *   char* str2 = "-5678";         *
 *   num2 = str2int2(str2);        *
 *   printf("num2 = %d\n", num2);  *
 *   | num2 = -5678                *
 ***********************************/
int str2int2(char* str)
{
  return atoi(str);
}

/*******************************************
 * convert Integer to String #1            *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~            *
 * example:                                *
 *   int num1 = -5678;                     *
 *   char buffer1[sizeof(int) * 8 + 1];    *
 *   char* str1 = int2str(num1, buffer1);  *
 *   printf("str1 = %s\n", str1);          *
 *   | str1 = -5678                        *
 *******************************************/
char* int2str(int num, char str[])
{
  sprintf(str, "%d", num);
  return str;
}
char* long2str(int32 num, char str[])
{
  sprintf(str, "%ld", num);
  return str;
}
/****************************************
 * convert Integer to String #2         *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~         *
 * example:                             *
 *   int num2 = -5678;                  *
 *   char buffer2[sizeof(int) * 8 + 1]; *
 *   char* str2 = itoa(num2, buffer2);  *
 *   printf("str2 = %s\n", str2);       *
 *   | str2 = -5678                     *
 ****************************************/
char* itoa(int num, char str[])
{
  char const digit[] = "0123456789";
  char* p = str;
  if(num < 0){
    *p++ = '-';
    num *= -1;
  }
  int shifter = num;
  //-- Move to where representation ends
  do {
    ++p;
    shifter = shifter / 10;
  } while(shifter);
  *p = '\0';
  //-- Move back, inserting digits as u go
  do {
    *--p = digit[num%10];
    num = num / 10;
  } while(num);
  return str;
}

void osal_releaseMemory(char* var)
{
	if(var) {
		osal_mem_free(var);
  	var = NULL;
  }
}
void osal_releaseMemoryPtr(char** var)
{
	if(var) {
		osal_mem_free(var);
  	var = NULL;
  }
}

#if DEBUG_PRINT_UART
  void printNumber(long double number, int decimals)
  {
    int powDec = 1;
    
    int i;
    for(i = 0; i < decimals; i++) {
      powDec *= 10;
    }
    
    //-- integer part of number
    int n0 = (int)number;
    //-- integer part * powDec
    int n1 = n0 * powDec;
    //-- the whole number * powDec
    int n2 = (long)(number * powDec);
    //-- decimal part of the number
    int n3 = abs(n2 - n1);
    
    char piece[10];
    char str[100];
    
    //-- add integer part
    sprintf(piece, "%d", n0);
    strcpy(str, piece);
    //-- if 0, show as an integer
    if(decimals > 0) {
      //-- add dot
      strcat(str, ".");
      //-- add decimal part
      if(n3 == 0) {
        //-- add extra "0"
        for(i = 0; i < decimals; i++) {
          strcat(str, "0");
        }
      } else {
        sprintf(piece, "%d", n3);
        strcat(str, piece);
      }
    }
    printf("%s", str);
    //halOLEDShowX16(0, 0, str);
  }
  
  void osal_printf(char str[])
  {
    char *memStr = osal_mem_alloc(sizeof(char)*sizeof(str));
    if(memStr) {
      osal_memset(memStr, 0, sizeof(char)*sizeof(str));
      osal_memcpy(memStr, str, osal_strlen(str));
      
      printf("%s", memStr);
      
      //osal_mem_free(memStr);
    }
    osal_releaseMemory(memStr);
  }
#endif

int8 encodeU8to8(uint8 byte)
{
  //-- implicit conversion from unsigned to signed
  return byte;
}

/****************************************
 * implode two UINT8 to UINT16          *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~          *
 * example:                             *
 *   uint16 big;                        *
 *   uint8 bH = 0xEF, bL = 0x7D;        *
 *   big = implodeU8toU16(bH, bL);      *
 *   printf("0x%x+0x%x=0x%x\n", bH, bL, big); *
 *   => 125+239=-4227                   *
 ****************************************/
uint16 implodeU8toU16(uint8 byte1, uint8 byte2)
{
  return ((uint16)byte1 << 8) + byte2;
}

/******************************************************
 * explode UINT16 to array of UINT8                   *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                   *
 * example:                                           *
 *   uint8* arr8x2;                                   *
 *   uint16 big = 0xEF7D;                             *
 *   arr8x2 = explodeU16toU8(big);                    *
 *   printf("0x%x+0x%x=0x%x\n", arr8x2[0], arr8x2[1], big); *
 *   => 239+125=-4227                                 *
 *****************************************************/
uint8* explodeU16toU8(uint16 byte)
{
  static uint8 arr8x2[2];
  
  //-- low byte
  arr8x2[0] = byte & 0xFF;
  //-- high byte
  arr8x2[1] = (byte >> 8);
  return arr8x2;
}

uint16 convert16toU16(int16 in)
{
  return (uint16)32768 + (uint16)in;
}


/****************************************
 * convert Int to Hex                   *
 * ~~~~~~~~~~~~~~~~~~                   *
 * example:                             *
 *   char* r1 = int2hex(0xEF7D,1,1);    *
 *   printf("%s", r1);                  *
 *   => 0xEF7D (uppercase, with prefix) *
 *                                      *
 *   char* r1 = int2hex(0xEF7D,0,0);    *
 *   printf("%s", r1);                  *
 *   => ef7d (lowercase, no prefix)     *
 ****************************************/
char* int2hex(int32 value, uint8 upperCase, uint8 prefix)
{
  static char result[36];
  char str1[32], str2[32];
  
  //-- low 2 bytes
  uint16 l = (uint16)(value & 0xFFFF);
  //-- high 2 bytes
  uint16 h = (uint16)(value >> 16);
  
  //-- format int to hex
  sprintf(str1, "%x", l);
  sprintf(str2, "%x", h);
  if(upperCase) {
    //-- convert to uppercase
    sprintf(str1, "%s", str2upper(str1, 32));
    sprintf(str2, "%s", str2upper(str2, 32));
  }
  //-- add "0x" prefix, if necessary
  strcpy(result, prefix ? "0x" : "");
  strcat(result, str2);
  strcat(result, str1);
  
  return result;
}

/**************************************************
 * convert string to UpperCase                    *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~                    *
 * example:                                       *
 *   char srcStr[5];                              *
 *   sprintf(srcStr, "%s", "hello");              *
 *   sprintf(srcStr, "%s", str2upper(srcStr, 5)); *
 *   printf("%s", srcStr);                        *
 *   => HELLO                                     *
 **************************************************/
char* str2upper(char* str, uint8 length)
{
  for(uint8 i=0;i<length;i++) {
    str[i] = toupper(str[i]);
  }
  return str;
}

/*********************************************
 * convert milliseconds to string            *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~            *
 * example:                                  *
 *   uint32 pTime32 = osal_GetSystemClock(); *
 *   char* t1 = ms2str(pTime32, 1);          *
 *   printf("time: %s", t1);                 *
 *   => time: 1y 1m 2w 20d 02:15:13          *
 *                                           *
 *   char* t1 = ms2str(pTime32, 0);          *
 *   printf("time: %s", t1);                 *
 *   => time: 1y 1m 2w 20d 2h 15m 13.426s    *
 *********************************************/
char* ms2str(uint32 pTime32, uint8 hisFormat)
{
  static char result[100];
  char s[10];
  
  //-- in seconds
  uint32 pTime = pTime32 / 1000;
  int mseconds = pTime32 - (pTime * 1000);
  int seconds = pTime % 60;
  int minutes = (pTime % 3600) / 60;
  int hours = (pTime % 86400) / 3600;
  int days = (pTime % (86400 * 30)) / 86400;

  //-- total days
  int d = pTime / 86400;
  int weeks = (d / 7) % 7;
  //-- approx. 30 day per month
  int months = (d / 30) % 12;
   //-- approx. 365 day per year
  int years = d / 365;

  strcpy(result, "");

  if(years > 0) {
    sprintf(s, "%dy ", years);
    strcat(result, s);
  }
  if(months > 0) {
    sprintf(s, "%dm ", months);
    strcat(result, s);
  }
  if(weeks > 0) {
    sprintf(s, "%dw ", weeks);
    strcat(result, s);
  }
  if(days > 0) {
    sprintf(s, "%dd ", days);
    strcat(result, s);
  }
  if(hisFormat) {
    //-- hours
    if(hours < 10) {
      strcat(result, "0");
    }
    sprintf(s, "%d:", hours);
    strcat(result, s);
    //-- minutes
    if(minutes < 10) {
      strcat(result, "0");
    }
    sprintf(s, "%d:", minutes);
    strcat(result, s);
    //-- seconds
    if(seconds < 10) {
      strcat(result, "0");
    }
    sprintf(s, "%d", seconds);
    strcat(result, s);
  } else {
    if(hours > 0) {
      sprintf(s, "%dh ", hours);
      strcat(result, s);
    }
    if(minutes > 0) {
      sprintf(s, "%dm ", minutes);
      strcat(result, s);
    }
    if(mseconds > 0) {
      sprintf(s, "%d.%ds ", seconds, mseconds);
    } else {
      sprintf(s, "%ds ", seconds);
    }
    strcat(result, s);
  }

  //char* r1 = int2hex(pTime32,1,1);

  return result;
}

/*************************************************
 * get part of a string (substring)              *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~              *
 * example:                                      *
 *   char str[100];                              *
 *   sprintf(str, "1021184523");                 *
 *       |'length':      ^^                      *
 *       |'offset':0123456789                    *
 *   printf("substr = %s\n", substr(str, 6, 2)); *
 *   => substr = 45                              *
 *************************************************/
//char* substr(char str[], int offset, int length)
char* substr(char *str, int offset, int length)
{
  int i = 0, memSize = 0;
  
  memSize = sizeof(char) * strlen(str);
  printf(">>ss:%d,%d,%d\n", memSize, offset, length);

  char *result = osal_mem_alloc(memSize);
  if(!result) {
    if(DEBUG) {
      printf("!substr\n");
    }
    return NULL;
  }
  //printf("L%d\n", strlen(str));

  while(i < length) {
    result[i] = str[offset + i];
    i++;
  }

  result[i] = '\0';

  return result;
}

/**************************************************************
 * split string into pieces and get an item by index          *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~          *
 * example:                                                   *
 *   char s[100];                                             *
 *   sprintf(s, "1021184523");                                *
 *   int numParts = 5, idx = 3;                               *
 *   printf("s[%d] = %s\n", idx, getPiece(s, numParts, idx)); *
 *   => s[3] = 45                                             *
 **************************************************************/
char* getPiece(char* str, int numParts, int idx)
{
  int memSize = 0, tmpIdx = 0;
  int strLength = strlen(str);
  
  //-- get string
  memSize = (sizeof(char*) * strLength) + 1;
  char *mainStr = osal_mem_alloc(memSize);
  if(!mainStr) {
    if(DEBUG) {
      printf("!getPiece1\n");
    }
    //-- there is no memory to free
    return NULL;
  }
  
  //-- make a copy of string
  sprintf(mainStr, str);

  /*
  //-- check if string can be divided in "numParts" equal parts
  if(strLength % numParts != 0) {
    if(DEBUG) {
      printf("1) Invalid Input: String size(%d) is not divisible by numParts(%d)\n", strLength, numParts);
    }
	  //-- free memory allocated for the main string
  	osal_releaseMemory(mainStr);
    return str;
  }
  */

  int partSize = strLength / numParts;

  memSize = (sizeof(char) * partSize) + 1;
  char *tmpStr = osal_mem_alloc(memSize);
  if(!tmpStr) {
    if(DEBUG) {
      printf("!getPiece2\n");
    }
    //-- free memory allocated for the main string
    osal_releaseMemory(mainStr);
    return NULL;
  }

  memSize = sizeof(char) * 2;
  char *tmpChar = osal_mem_alloc(memSize);
  if(!tmpChar) {
    if(DEBUG) {
      printf("!getPiece3\n");
    }
    //-- free memory allocated for the main string
    osal_releaseMemory(tmpStr);
    osal_releaseMemory(mainStr);
    return NULL;
  }

  int mainIdx = 0;

  strcpy(tmpStr, "");

  for(uint8 i = 0; i < strLength; i++) {
    tmpIdx++;
    //-- get char
    sprintf(tmpChar, "%c", mainStr[i]);
    //-- add char
    strcat(tmpStr, tmpChar);

    if(tmpIdx == partSize) {
      if(idx < numParts && idx == mainIdx) {
      	//-- found index
      	sprintf(mainStr, "%s", tmpStr);
      	//-- free temporary memory
  			osal_releaseMemory(tmpStr);
  			osal_releaseMemory(tmpChar);
    		return mainStr;
      }
      
      //-- go to the next item
      strcpy(tmpStr, "");
      tmpIdx = 0;
      mainIdx++;
    }
  }

  //-- the last item
  if(idx < numParts && idx == mainIdx) {
  	//-- found index
  	sprintf(mainStr, "%s", tmpStr);
  	//-- free temporary memory
  	osal_releaseMemory(tmpStr);
  	osal_releaseMemory(tmpChar);
  	return mainStr;
  }

  //-- free all allocated memory in case of failure!
  if(DEBUG) {
    printf("!getPiece4\n");
  }
  osal_releaseMemory(tmpStr);
  osal_releaseMemory(tmpChar);
  osal_releaseMemory(mainStr);
  return NULL;

}

/*******************************************************
 * split string into pieces and get an array of pieces *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
 * example:                                            *
 *   char s[100];                                      *
 *   sprintf(s, "1021184523");                         *
 *   int numParts = 5;                                 *
 *   char **arr = getPieces(s, numParts);              *
 *   for(uint8 i = 0; i <= sizeof(arr); i++) {         *
 *     printf("arr[%d] = %s\n", i, arr[i]);            *
 *   }                                                 *
 *   => arr[0] = 10                                    *
 *      arr[1] = 21                                    *
 *      arr[2] = 18                                    *
 *      arr[3] = 45                                    *
 *      arr[4] = 23                                    *
 *******************************************************/
char **getPieces(char* str, int numParts)
{
  char** arr;
  int memSize = 0;
  int strLength = strlen(str);
  int partSize = strLength / numParts;

  //-- get string
  memSize = (sizeof(char*) * strLength) + 1;
  char *mainStr = osal_mem_alloc(memSize);
  if(!mainStr) {
    if(DEBUG) {
      printf("!getPiece1\n");
    }
    //-- there is no memory to free
    return NULL;
  }
  sprintf(mainStr, str);
  
  //-- memory allocation to store pointers to strings
  memSize = numParts * sizeof(char*);
  arr = (char**)osal_mem_alloc(memSize);
  if(!arr) {
  	if(DEBUG) {
      printf("!getPieces1(%d)\n", memSize);
    }
    osal_releaseMemory(mainStr);
    return NULL;
  }
  
  //-- loop through strings
  for(uint8 i = 0; i < numParts; i++) {
    //-- memory allocation to store strings
    arr[i] = (char*)osal_mem_alloc((partSize + 1) * sizeof(char));
    if(!arr[i]) {
  		if(DEBUG) {
      	printf("!getPieces2(%d)\n", memSize);
    	}
    	for(uint8 k = 0; k < numParts; k++) {
    		osal_releaseMemory(arr[k]);
    	}
    	osal_releaseMemoryPtr(arr);
    	return NULL;
  	}

    char *piece = getPiece(mainStr, numParts, i);
    if(piece) {
    	//-- copy piece to array item
    	sprintf(arr[i], piece);
    }
    osal_releaseMemory(piece);
  }

  //-- release mainStr memory
  osal_releaseMemory(mainStr);
  
  return arr;
}

/*******************************************
 *
 *  int sprintf(char *,const char *,...)
 *  char *strcpy(char *,const char *)
 *  char *strcat(char *, const char *);
 *
 *******************************************/
