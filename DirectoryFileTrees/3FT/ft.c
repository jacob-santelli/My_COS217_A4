/*--------------------------------------------------------------------*/
/* ft.c                                                              */
/* Authors: Jacob Santelli and Joshua Yang                             */
/*--------------------------------------------------------------------*/

/*
  A File Tree is a representation of a hierarchy of directories and
  files: the File Tree is rooted at a directory, directories
  may be internal nodes or leaves, and files are always leaves.
*/

#include <stddef.h>
#include "a4def.h"

int FT_insertDir(const char *pcPath) { 
    return 0;
}


boolean FT_containsDir(const char *pcPath) {
    return FALSE;
}


int FT_rmDir(const char *pcPath) {
    return 0;
}

int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {
    return 0;
}

boolean FT_containsFile(const char *pcPath) {
    return FALSE;
}

int FT_rmFile(const char *pcPath) {
    return 0;
}

void *FT_getFileContents(const char *pcPath) {
}

void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {

}

int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
    return 0;
}

int FT_init(void) {
    return 0;
}

int FT_destroy(void) {
    return 0;
}

char *FT_toString(void) {
    return NULL;
}
