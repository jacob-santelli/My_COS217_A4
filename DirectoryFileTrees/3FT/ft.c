/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Authors: Jacob Santelli and Joshua Yang                            */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "path.h"
#include "nodeFT.h"
#include "ft.h"
#include "a4def.h"

/*
  A File Tree is a representation of a hierarchy of directories and
  files: the File Tree is rooted at a directory, directories
  may be internal nodes or leaves, and files are always leaves.
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;

/* --------------------------------------------------------------------

  Traverses the DT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poNFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poNFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
  * NOT_A_DIRECTORY if path contains a file anywhere instead of a directory 
*/
static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest, enum bool checkFilesInPath) {
   int iStatus;
   Path_T oPPrefix = NULL;
   Node_T oNCurr;
   Node_T oNChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;

   assert(oPPath != NULL);
   assert(poNFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oNRoot == NULL) {
      *poNFurthest = NULL;
      return SUCCESS;
   }
 
   /* set oPPrefix to the value of the root node, make sure that it returns success */
   iStatus = Path_prefix(oPPath, 1, &oPPrefix);

   /* checks that there is a value in the root node */
   if(iStatus != SUCCESS) {
      *poNFurthest = NULL;
      return iStatus;
   }

   if(Path_comparePath(Node_getPath(oNRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oNCurr = oNRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poNFurthest = NULL;
         return iStatus;
      }
      if(Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = Node_getChild(oNCurr, ulChildID, &oNChild);
         if(iStatus != SUCCESS) {
            *poNFurthest = NULL;
            return iStatus;
         }
         if (checkFilesInPath == TRUE && Node_getState(oNChild) != DIRECTORY) {
            return NOT_A_DIRECTORY;
         }
         oNCurr = oNChild;
      }
      else {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   Path_free(oPPrefix);
   *poNFurthest = oNCurr;
   return SUCCESS;
}

int FT_insertDir(const char *pcPath) { 
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oNCurr, TRUE);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   /* only possible if root is in fact NULL, in which case the first
   Node is being added to the tree */
   if(oNCurr == NULL)
      ulIndex = 1;

   /* checks to see if the whole path being inserted is in the tree already */
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

      /* oNCurr is the node with the longest shared prefix with the
      path that we are trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /* _isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, DIRECTORY);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   return SUCCESS;
}

boolean FT_containsDir(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return FALSE;

   if (oNRoot == NULL)
      return FALSE;

   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);
   if (iStatus != SUCCESS)
      return FALSE;

   if (Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
      return FALSE;

   if (Node_getState(oNFound) == DIRECTORY) {
      return (boolean) (iStatus == SUCCESS);
   }
   return FALSE;
}


int FT_rmDir(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return INITIALIZATION_ERROR;

   /* call traverse path to find the last node in pcPath, and then check if that
   is actually a directory */
   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);

   if(iStatus != SUCCESS)
       return iStatus;

   if (oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
      return NO_SUCH_PATH;

   if (Node_getState(oNFound) != DIRECTORY) {
      return NOT_A_DIRECTORY;
   }

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;

   return SUCCESS;
}

int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oNCurr, TRUE);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   /* only possible if root is in fact NULL, in which case we do
   not insert a file into the root */
   if(oNCurr == NULL)
      return CONFLICTING_PATH;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

      /* oNCurr is the node with the longest shared prefix with the
      path that we are trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* insert the new node for this level, depending on whether it is the final file node */
      if (ulIndex < ulDepth) {
         iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, DIRECTORY);
      }
      else {
         iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode, A_FILE);
         Node_setFile(oNNewNode, pvContents);
         Node_setFileLength(oNNewNode, ulLength);
      }
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   return SUCCESS;
}

boolean FT_containsFile(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return FALSE;

   if (oNRoot == NULL)
      return FALSE;

   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);
   if (iStatus != SUCCESS)
      return FALSE;

   if (oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
      return FALSE;

   if (Node_getState(oNFound) == A_FILE) {
      return (boolean) (iStatus == SUCCESS);
   }
   return FALSE;
}

int FT_rmFile(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return INITIALIZATION_ERROR;

   /* call traverse path to find the last node in pcPath, and then check if that
   is actually a directory */
   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);

   if(iStatus != SUCCESS)
       return iStatus;

   if (oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
      return NO_SUCH_PATH;

   if (Node_getState(oNFound) != A_FILE) {
      return NOT_A_FILE;
   }

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;

   return SUCCESS;
}

void *FT_getFileContents(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return NULL;

   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);

   if(iStatus != SUCCESS)
       return NULL;

   if (oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0)
      return NULL;

   if (Node_getState(oNFound) != A_FILE) {
      return NULL;
   }
   return Node_getFile(oNFound);
}

void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {
   int iStatus;
   Node_T oNFound = NULL;
   Path_T oPPath = NULL;
   void *pvTempOne;

   assert(pcPath != NULL);
   assert(Path_new(pcPath, &oPPath) == SUCCESS);

   if (!bIsInitialized)
      return NULL;

   iStatus = FT_traversePath(oPPath, &oNFound, FALSE);
   if (iStatus != SUCCESS) {
      Path_free(oPPath);
      return NULL;
   }
   if(oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      return NULL;
   }

   if (Node_getState(oNFound) != A_FILE) {
      Path_free(oPPath);
      return NULL;
   }

   pvTempOne = Node_getFile(oNFound);
   Node_setFile(oNFound, pvNewContents);
   Node_setFileLength(oNFound, ulNewLength);

   return pvTempOne;
}

int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;

   assert(pcPath != NULL);

   /* checks for initialization error */
   if(!bIsInitialized) return INITIALIZATION_ERROR;

   /* checks for memory error and bad path */
   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      Path_free(oPPath);
      return iStatus;
   }

   /* checks for no such path and conflicting path */
   iStatus = FT_traversePath(oPPath, &oNFound, 0);
   if (iStatus == CONFLICTING_PATH) return CONFLICTING_PATH;
   if(oNFound == NULL || Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      return NO_SUCH_PATH;
   }

   /* change booleans depending on if node is directory or Node */
   if (Node_getState(oNFound) == DIRECTORY) {
      *pbIsFile = FALSE;
   }
   else {
      *pbIsFile = TRUE;
      *pulSize = Node_getFileLength(oNFound);
   }

   Path_free(oPPath);
   return SUCCESS;
}

int FT_init(void) {
   /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */

   if(bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oNRoot = NULL;
   ulCount = 0;

   /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
   return SUCCESS;
}

int FT_destroy(void) {
   /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */

   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   if(oNRoot) {
      ulCount -= Node_free(oNRoot);
      oNRoot = NULL;
   }

   bIsInitialized = FALSE;

   /* assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount)); */
   return SUCCESS;
}

/* --------------------------------------------------------------------

  The following auxiliary functions are used for generating the
  string representation of the DT.
*/

/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/
static void FT_preOrderTraversal(DynArray_T d, Node_T n) {
   size_t c;
   size_t j;
   DynArray_T temp;

   assert(d != NULL);

   temp = DynArray_new(1);

   if(n != NULL) {
      (void) DynArray_add(d, n);
      for(c = 0; c < Node_getNumChildren(n); c++) {
         Node_T oNChild = NULL;
         assert(!Node_getChild(n,c, &oNChild));

         if (Node_getState(oNChild) == A_FILE) {
            (void) DynArray_add(d, oNChild);
         }
         else {
            (void) DynArray_add(temp, oNChild);
         }
         /* assert(iStatus == SUCCESS); */
      }
      for (j = 0; j< DynArray_getLength(temp); j++) {
         (void) FT_preOrderTraversal(d, DynArray_get(temp, j));
      }
   }
}

/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc) {
   assert(pulAcc != NULL);

   if(oNNode != NULL)
      *pulAcc += (Path_getStrLength(Node_getPath(oNNode)) + 1);
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc) {
   assert(pcAcc != NULL);

   if(oNNode != NULL) {
      strcat(pcAcc, Path_getPathname(Node_getPath(oNNode)));
      strcat(pcAcc, "\n");
   }
}
/*--------------------------------------------------------------------*/

char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if(!bIsInitialized)
      return NULL;

   nodes = DynArray_new(ulCount);  
   (void) FT_preOrderTraversal(nodes, oNRoot);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);

   return result;
}
