/*--------------------------------------------------------------------*/
/* ft_client.c                                                        */
/* Author: Jacob Santelli and Joshua Yang                             */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ft.h"

/* Tests the FT implementation with an assortment of checks.
   Prints the status of the data structure along the way to stderr.
   Returns 0. */
int main(void) {
  enum {ARRLEN = 1000};
  char* temp;
  boolean bIsFile;
  size_t l;
  char arr[ARRLEN];
  arr[0] = '\0';

  /* After initialization, the data structure is empty, so
     contains* should still return FALSE for any non-NULL string,
     and toString should return the empty string.
  */
  assert(FT_init() == SUCCESS);

  /* A valid path must not:
     * be the empty string
     * start with a '/'
     * end with a '/'
     * have consecutive '/' delimiters.
  */
  assert(FT_insertDir("") == BAD_PATH);
  assert(FT_insertDir("/1root/2child") == BAD_PATH);
  assert(FT_insertDir("1root/2child/") == BAD_PATH);
  assert(FT_insertDir("1root//2child") == BAD_PATH);
  assert(FT_insertFile("", NULL, 0) == BAD_PATH);
  assert(FT_insertFile("/1root/2child", NULL, 0) == BAD_PATH);
  assert(FT_insertFile("1root/2child/", NULL, 0) == BAD_PATH);
  assert(FT_insertFile("1root//2child", NULL, 0) == BAD_PATH);

  /* putting a file at the root is illegal */
  assert(FT_insertFile("A",NULL,0) == CONFLICTING_PATH);

  /* After insertion, the data structure should contain every prefix
     of the inserted path, toString should return a string with these
     prefixes, trying to insert it again should return
     ALREADY_IN_TREE, and trying to insert some other root should
     return CONFLICTING_PATH.
  */
  assert(FT_insertDir("1root/2child/3gkid") == SUCCESS);
  assert(FT_containsDir("1root/2child") == TRUE);
  assert(FT_insertFile("1root/2second/3gfile", NULL, 0) == SUCCESS);
  assert(FT_containsFile("1root/2child/3gfile") == TRUE);
  assert(FT_insertDir("1root/2child/3gkid") == ALREADY_IN_TREE);
  assert(FT_insertFile("1root/2child/3gkid", NULL, 0) ==
         ALREADY_IN_TREE);
  assert(FT_insertDir("1otherroot") == CONFLICTING_PATH);
  assert(FT_insertDir("1otherroot/2d") == CONFLICTING_PATH);
  assert(FT_insertFile("1otherroot/2f", NULL, 0) == CONFLICTING_PATH);

  /* Trying to insert a third child should succeed, unlike in BDT */
  assert(FT_insertFile("1root/2third", NULL, 0) == SUCCESS);
  assert(FT_insertDir("1root/2ok/3yes/4indeed") == SUCCESS);

  /* Children must be unique, but individual directories or files
     in different paths needn't be
  */
  assert(FT_insertFile("1root/2child/3gkid", NULL, 0) ==
         ALREADY_IN_TREE);
  assert(FT_insertDir("1root/2child/3gkid") == ALREADY_IN_TREE);
  assert(FT_insertDir("1root/2child/3gk2/4ggk") == SUCCESS);
  assert(FT_insertDir("1root/2child/2child/2child") == SUCCESS);
  assert(FT_insertFile("1root/2child/2child/2child/2child", NULL, 0) ==
         SUCCESS);

  /* Attempting to insert a child of a file is illegal */
  assert(FT_insertDir("1root/2third/3nopeD") == NOT_A_DIRECTORY);
  assert(FT_insertFile("1root/2third/3nopeF", NULL, 0) ==
         NOT_A_DIRECTORY);
  return 0;
}
