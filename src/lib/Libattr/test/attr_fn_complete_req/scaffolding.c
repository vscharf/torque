#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "utils.h"
#include "attribute.h"

std::string encoded;

int is_whitespace(

  char c)

  {
  if ((c == ' ')  ||
      (c == '\n') ||
      (c == '\t') ||
      (c == '\r') ||
      (c == '\f'))
    return(TRUE);
  else
    return(FALSE);
  } /* END is_whitespace */

void move_past_whitespace(

  char **str)

  {
  if ((str == NULL) ||
      (*str == NULL))
    return;

  char *current = *str;

  while (is_whitespace(*current) == TRUE)
    current++;

  *str = current;
  } // END move_past_whitespace()


extern "C"
{
char *pbse_to_txt(

  int err)

  {
  return(NULL);
  }
}

int encode_str(
    
  pbs_attribute *pattr,
  tlist_head    *phead,
  const char    *name,
  const char    *rsname,
  int            mode,
  int            perm)

  {
  encoded = pattr->at_val.at_str;
  return(PBSE_NONE);
  }