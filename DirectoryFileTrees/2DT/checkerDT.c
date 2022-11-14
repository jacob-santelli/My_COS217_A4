/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author: Jacob Santelli and Joshua Yang                             */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"

/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   size_t length;
   size_t i;
   size_t j;
   size_t ulIndex;

   int comp;

   DynArray_T nodeChildren = DynArray_new(0);

   oNParent = Node_getParent(oNNode);

   /* a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* a Node with a NULL path is not a valid Node */
   oPNPath = Node_getPath(oNNode);
   if (oPNPath == NULL) {
      fprintf(stderr, "Path is NULL\n");
      return FALSE;
   }

   /* if oPNPath is non-NULL */
   else {
      length = Path_getDepth(oPNPath);
      if (length == 0) {
         fprintf(stderr, "Path is not NULL, but length is 0\n");
         return FALSE;
      }
      if (length == 1 && oNParent != NULL) {
         fprintf(stderr,
                 "Path length is 1, but oNParent is not NULL\n");
         return FALSE;
      }
      else if (length > 1 && oNParent == NULL) {
         fprintf(stderr,
                 "Path length is more than 1, but oNParent is NULL\n");
         return FALSE;
      }
   }

   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      /* parent node must have children */
      if (Node_getNumChildren(oNParent) < 1) {
         fprintf(stderr, "Parent node does not have any children\n");
         return FALSE;
      }

      /* parent's path must be the longest possible
      proper prefix of the node's path */
      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }

      /* path must have length greater than 1 */
      if (Path_getDepth(oPNPath) < 2) {
         fprintf(stderr, "Path length is less than 2\n");
         return FALSE;
      }
   }

   /* if no parent node, this must be the root node */
   else {
      oPNPath = Node_getPath(oNNode);

      /* path must have length of exactly 1 */
      if (Path_getDepth(oPNPath) != 1) {
         fprintf(stderr, "Node does not have path length 1\n");
         return FALSE;
      }
   }

   /* conduct checks on children of the node */
   /* populate a DynArray with children of node */
   for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
      Node_T oNChild = NULL;

      /* getNumChildren should equal number of children the node has */
      if(Node_getChild(oNNode, ulIndex, &oNChild) != SUCCESS) {
         fprintf(stderr,
                 "getNumChildren not equal to what getChild returns\n");
         return FALSE;
      }

      /* child path length should equal parent path length + 1 */
      if (Path_getDepth(Node_getPath(oNChild)) !=
          Path_getDepth(Node_getPath(oNNode)) + 1) {
         fprintf(stderr,
               "Child path length not equal to parent path length+1\n");
         return FALSE;
      }

      assert(DynArray_add(nodeChildren, oNChild));
   }
   
   if (DynArray_getLength(nodeChildren) > 1) {
      for (i = 0; i <  DynArray_getLength(nodeChildren) - 1; i++) {
         j = i + 1;
         comp = Path_comparePath(
            Node_getPath(DynArray_get(nodeChildren, i)), 
                  Node_getPath(DynArray_get(nodeChildren, j)));

         /* checks if current node children in lexicographic order */
         if (comp > 0) {
            fprintf(stderr,
                    "Children of node not in lexicographic order\n");
            return FALSE;
         }

         /* checks if current node has duplicate children */
         else if (comp == 0) {
            fprintf(stderr, "Node contains duplicate children\n");
            return FALSE;
         }
      }
   }
   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise. Performs checks on each individual
   node; independently counts the number of nodes reachable from 
   root node and stores result in ulCountCheck, comparing it
   to ulCount to verify. 
*/
static boolean CheckerDT_treeCheck(
   Node_T oNNode, size_t ulCount, size_t* ulCountCheck) {
   size_t ulIndex;

   assert(ulCountCheck);

   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr,
                 "getNumChildren not equal to what getChild returns\n");
            return FALSE;
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, ulCount, ulCountCheck))
            return FALSE;
      }

      /* ulCountCheck independently counts number of nodes reachable 
         from root to verify ulCount is accurate */
      (*ulCountCheck)++;
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   /* ulCountCheck independently counts number of nodes reachable from
      root to verify ulCount is accurate */
   size_t* ulCountCheck = malloc(sizeof(size_t*));
   assert(ulCountCheck);
   *ulCountCheck = 0;

   /* bIsIntialized checks */
   if(!bIsInitialized) {
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
      if(oNRoot != NULL) {
         fprintf(stderr, "Not initialized, but root is not NULL\n");
         return FALSE;
      }
   }

   /* ulCount checks */
   if (ulCount > 0) {
      if (!bIsInitialized) {
         fprintf(stderr,
                 "Count greater than 0, but tree is not initialized\n");
         return FALSE;
      }
      if (oNRoot == NULL) {
         fprintf(stderr, "Count is greater than 0, but root is NULL\n");
         return FALSE;
      }
   }
   else {
      if (oNRoot != NULL) {
         fprintf(stderr, "Count is 0, but root is not NULL\n");
         return FALSE;
      }
   }

   /* oNRoot checks */
   if (oNRoot == NULL) {
      if (ulCount > 0) {
         fprintf(stderr, "Root is NULL, but root is greater than 0\n");
         return FALSE;
      }
   }

   else {
      if (ulCount == 0) {
         fprintf(stderr, "Root is not NULL, but count is 0\n");
         return FALSE;
      }
      if (!bIsInitialized) {
         fprintf(stderr,
                 "Root is not NULL, but tree is not initialized\n");
         return FALSE;
      }
   }

   /* Now checks invariants recursively at each node from the root. */
   assert(CheckerDT_treeCheck(oNRoot, ulCount, ulCountCheck));

   /* verify ulCountCheck == ulCount */
   if (ulCount != *ulCountCheck) {
      fprintf(stderr,
              "ulCount doesn't match # of nodes reachable from root\n");
      free(ulCountCheck);
      return FALSE;
   }
   free(ulCountCheck);
   return TRUE;
}
