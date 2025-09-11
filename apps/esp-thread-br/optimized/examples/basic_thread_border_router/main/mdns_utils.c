#include <stdio.h>
#include <ctype.h>
#include <string.h>

void hostname_optimized(const char* input, char* output)
{
  if(input == NULL || output == NULL) {
    *output = '\0';
    return;
  }
  
  char* dest = output;
  const char* src = input;
  char last_char = '\0';
  
  //-- Skip leading spaces and dashes
  while(*src != '\0' && (isspace((unsigned char)*src) || *src == '-')) {
    src++;
  }
  
  while(*src != '\0') {
    if(isalnum((unsigned char)*src)) {
      //-- Alphanumeric characters
      *dest = tolower((unsigned char)*src);
      dest++;
      last_char = *(dest-1);
    } else if (isspace((unsigned char)*src) || *src == '-') {
      //-- Spaces and dashes - handle both cases
      if(last_char != '-') {
        *dest = '-';
        dest++;
        last_char = '-';
      }
    }
    src++;
  }
  
  //-- Remove trailing dash if present
  if(dest > output && *(dest-1) == '-') {
    dest--;
  }
  
  *dest = '\0';
}
