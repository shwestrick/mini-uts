#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "uts.h"

void impl_abort(int err) {
  exit(err);
}

const char *impl_getName() {
  return "Sam's mini UTS";
}

int impl_paramsToStr(char *strBuf, int ind) {
  ind += sprintf(strBuf+ind, "Execution strategy:  %s\n", impl_getName());
  return ind;
}

// Not using UTS command line params, return non-success
int impl_parseParam(char *param, char *value) {
  return 1;
}

void impl_helpMessage() {
  printf("   none.\n");
}

// ==========================================================================

typedef struct {
  counter_t maxdepth, size, leaves;
} Result;

Result treeSearch(UTSConfig *config, int depth, Node *parent) {
  int numChildren, childType;
  counter_t parentHeight = parent->height;

  Result r;
  r.maxdepth = depth;
  r.size = 1;
  r.leaves = 0;

  numChildren = uts_numChildren(config, parent);
  childType   = uts_childType(config, parent);

  // record number of children in parent
  parent->numChildren = numChildren;

  // Recurse on the children
  if (numChildren > 0) {
    int i, j;

    for (i = 0; i < numChildren; i++) {
      Node child;
      child.type = childType;
      child.height = parentHeight + 1;
      child.numChildren = -1;    // not yet determined
      for (j = 0; j < config->computeGranularity; j++) {
        rng_spawn(parent->state.state, child.state.state, i);
      }
      Result c = treeSearch(config, depth+1, &child);

      if (c.maxdepth > r.maxdepth) r.maxdepth = c.maxdepth;
      r.size += c.size;
      r.leaves += c.leaves;
    }

  } else {
    r.leaves = 1;
  }

  return r;
}

// ===========================================================================

int main(int argc, char *argv[]) {
  UTSConfig config;
  Node root;
  double t1, t2;

  uts_parseParams(&config, argc, argv);
  uts_printParams(&config);
  uts_initRoot(&config, &root);

  t1 = uts_wctime();

  Result r = treeSearch(&config, 0, &root);

  t2 = uts_wctime();

  uts_showStats(&config, 1, 0, t2-t1, r.size, r.leaves, r.maxdepth);

  return 0;
}
