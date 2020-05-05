#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include "parallel.h"
#include "utilities.h"
#include "uts.h"

void impl_abort(int err) {
  exit(err);
}

const char *impl_getName() {
  return "mini-uts parallel";
}

int impl_paramsToStr(char *strBuf, int ind) {
  ind += sprintf(strBuf+ind, "Execution strategy:  %s\n", impl_getName());
  ind += sprintf(strBuf+ind, "Scheduler:           %s\n", scheduler_name().c_str());
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
  if (numChildren == 0) {
    r.leaves = 1;
    return r;
  }

  long granularity = (depth > 100) ? numChildren : 1;

  parallel_for(0, numChildren, [&] (long i) {
    Node child;
    child.type = childType;
    child.height = parentHeight + 1;
    child.numChildren = -1;    // not yet determined
    for (int j = 0; j < config->computeGranularity; j++) {
      rng_spawn(parent->state.state, child.state.state, i);
    }
    Result c = treeSearch(config, depth+1, &child);

    pbbs::write_max(&r.maxdepth, c.maxdepth, std::less<int>());
    pbbs::write_add(&r.size, c.size);
    pbbs::write_add(&r.leaves, c.leaves);
  }, granularity);

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
