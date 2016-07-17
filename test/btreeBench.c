 
#include <stdio.h>
#include <stdlib.h>

#if !defined(USE_MALLOC)
#  define USE_MALLOC 0
#endif

#if !USE_MALLOC
#  include "akmalloc/malloc.h"
#endif
 
typedef struct _treenode
{
    struct _treenode* left;
    struct _treenode* right;
    int item;
} treenode;
 
static treenode *newtreenode(treenode (*left),treenode (*right),int item);
static int itemcheck(treenode (*tree));
static treenode *bottomuptree(int item,int depth);
static void deletetree(treenode (*tree));
 
static int nallocs=0;
 
static treenode *newtreenode(treenode (*left),treenode (*right),int item)
{
treenode (*t);
    t=(treenode (*))malloc(sizeof(treenode));
    ++nallocs;
    (t)->left=left;
    (t)->right=right;
    (t)->item=item;
    return t;
}
 
static int itemcheck(treenode (*tree))
{
    if ((tree)->left==0)
    {
        return (tree)->item;
    }
    else
    {
        return (((tree)->item+itemcheck((tree)->left))-itemcheck((tree)->right));
    }
}
 
static treenode *bottomuptree(int item,int depth)
{
    if ((depth>0))
    {
        return newtreenode(bottomuptree(((2*item)-1),(depth-1)),bottomuptree((2*item),(depth-1)),item);
    }
    else
    {
        return newtreenode(0,0,item);
    }
}
 
static void deletetree(treenode (*tree))
{
    if ((!!(tree)->left))
    {
        deletetree((tree)->left);
        deletetree((tree)->right);
    }
    free((void (*))tree);
}
 
int main(void)
{
treenode (*stretchtree);
treenode (*longlivedtree);
treenode (*temptree);
int n;
int depth;
int mindepth;
int maxdepth;
int stretchdepth;
int check;
int iterations;
int i;
    n=15;
    mindepth=4;
    if (((mindepth+2)>n))
    {
        maxdepth=(mindepth+1);
    }
    else
    {
        maxdepth=n;
    }
    stretchdepth=(maxdepth+1);
    stretchtree=bottomuptree(0,stretchdepth);
    printf("%s %d %s %d\n","Stretch tree of depth",stretchdepth,"\tcheck:",itemcheck(stretchtree));
    deletetree(stretchtree);
    longlivedtree=bottomuptree(0,maxdepth);
    depth=mindepth;
    while ((depth<=maxdepth))
    {
        iterations=(1<<((maxdepth-depth)+mindepth));
        check=0;
        for (i=1; i<=iterations; ++i)
        {
            temptree=bottomuptree(i,depth);
            check+=itemcheck(temptree);
            deletetree(temptree);
            temptree=bottomuptree(-i,depth);
            check+=itemcheck(temptree);
            deletetree(temptree);
        }
        printf("%d %s %d %s %d\n",(iterations*2),"\tTrees of depth",depth," check:",check);
        depth+=2;
    }
    printf("%s %d %s %d\n","long lived tree of depth",maxdepth,"\tcheck:",itemcheck(longlivedtree));
    printf("NALLOCS = %d\n",nallocs);
}