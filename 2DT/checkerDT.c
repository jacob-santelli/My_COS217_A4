/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author: Jacob Santelli and Joshua Yang                                                           */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"

/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;
   size_t length;
   Node_T oNChild = NULL;
   size_t i;

   oNParent = Node_getParent(oNNode);

   /* Sample check: a NULL pointer is not a valid node */
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

   else {
      length = Path_getDepth(oPNPath);
      if (length == 0) {
         fprintf(stderr, "Path is not NULL, but length is 0\n");
         return FALSE;
      }
      if (length == 1 && oNParent != NULL) {
         fprintf(stderr, "Path length is 1, but oNParent of oNNode is not NULL\n");
         return FALSE;
      }
      else if (length > 1 && oNParent == NULL) {
         fprintf(stderr, "Path length is more than 1, but oNParent of oNNode is NULL\n");
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

   else {
      oPNPath = Node_getPath(oNNode);

      /* path must have length of exactly 1 */
      if (Path_getDepth(oPNPath) != 1) {
         fprintf(stderr, "Root node does not have path length 1\n");
         return FALSE;
      }
   }

   if (Node_getNumChildren(oNNode) > 0) {
      for (i = 1; i < Node_getNumChildren(oNNode); i++) {
         if (Node_getChild(oNNode, i - 1, &oNChild) != SUCCESS) {
            fprintf(stderr, "something\n");
            return FALSE;
         }
      }
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode, boolean bIsInitialized, Node_T oNRoot, size_t ulCount) {
   size_t ulIndex;
   size_t i;
   size_t j;

   DynArray_T nodeChildren = DynArray_new(0);

   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* check: does current node have duplicate children? */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         if (Path_getDepth(Node_getPath(oNChild)) != Path_getDepth(Node_getPath(oNNode)) + 1) {
            fprintf(stderr, "Children have path length not equal to parent path length + 1\n");
            return FALSE;
         }

         assert(DynArray_add(nodeChildren, oNChild));
      }

      for (i = 0; i < DynArray_getLength(nodeChildren); i++) {
         for (j = i + 1; j < DynArray_getLength(nodeChildren); j++) {

            if (Path_getDepth(Node_getPath(DynArray_get(nodeChildren, i))) !=
             Path_getDepth(Node_getPath(DynArray_get(nodeChildren, j)))) {
               fprintf(stderr, "Children have path length not equal to each other\n");
               return FALSE;
            }

            if (Node_compare(DynArray_get(nodeChildren, i), DynArray_get(nodeChildren, j)) == 0) {
               fprintf(stderr, "Current node has duplicate children\n");
               return FALSE;
            }
         }
      }

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, bIsInitialized, oNRoot, ulCount))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
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
         fprintf(stderr, "Count is greater than 0, but tree is not initialized\n");
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
         fprintf(stderr, "Root is not NULL, but tree is not initialized\n");
         return FALSE;
      }
   }

   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(oNRoot, bIsInitialized, oNRoot, ulCount);
}
