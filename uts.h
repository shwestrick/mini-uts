#ifndef _UTS_H
#define _UTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rng/rng.h"

#define UTS_VERSION "2.1"

/***********************************************************
 *  Tree node descriptor and statistics                    *
 ***********************************************************/

#define MAXNUMCHILDREN    100  // cap on children (BIN root is exempt)

struct node_t {
  int type;          // distribution governing number of children
  int height;        // depth of this node in the tree
  int numChildren;   // number of children, -1 => not yet determined

  /* for statistics (if configured via UTS_STAT) */
#ifdef UTS_STAT
  struct node_t *pp;          // parent pointer
  int sizeChildren;           // sum of children sizes
  int maxSizeChildren;        // max of children sizes
  int ind;
  int size[MAXNUMCHILDREN];   // children sizes
  double unb[MAXNUMCHILDREN]; // imbalance of each child 0 <= unb_i <= 1
#endif

  /* for RNG state associated with this node */
  struct state_t state;
};

typedef struct node_t Node;

/* Tree type
 *   Trees are generated using a Galton-Watson process, in
 *   which the branching factor of each node is a random
 *   variable.
 *
 *   The random variable can follow a binomial distribution
 *   or a geometric distribution.  Hybrid tree are
 *   generated with geometric distributions near the
 *   root and binomial distributions towards the leaves.
 */
enum   uts_trees_e    { BIN = 0, GEO, HYBRID, BALANCED };
enum   uts_geoshape_e { LINEAR = 0, EXPDEC, CYCLIC, FIXED };

typedef enum uts_trees_e    tree_t;
typedef enum uts_geoshape_e geoshape_t;

/* Strings for the above enums */
extern const char * uts_trees_str[];
extern const char * uts_geoshapes_str[];

struct uts_config {
  /* Tree type
   *   Trees are generated using a Galton-Watson process, in
   *   which the branching factor of each node is a random
   *   variable.
   *
   *   The random variable can follow a binomial distribution
   *   or a geometric distribution.  Hybrid tree are
   *   generated with geometric distributions near the
   *   root and binomial distributions towards the leaves.
   */
  tree_t type  = GEO; // Default tree type
  double b_0   = 4.0; // default branching factor at the root
  int   rootId = 0;   // default seed for RNG state at root

  /*  Tree type BIN (BINOMIAL)
   *  The branching factor at the root is specified by b_0.
   *  The branching factor below the root follows an
   *     identical binomial distribution at all nodes.
   *  A node has m children with prob q, or no children with
   *     prob (1-q).  The expected branching factor is q * m.
   *
   *  Default parameter values
   */
  int    nonLeafBF   = 4;            // m
  double nonLeafProb = 15.0 / 64.0;  // q

  /*  Tree type GEO (GEOMETRIC)
   *  The branching factor follows a geometric distribution with
   *     expected value b.
   *  The probability that a node has 0 <= n children is p(1-p)^n for
   *     0 < p <= 1. The distribution is truncated at MAXNUMCHILDREN.
   *  The expected number of children b = (1-p)/p.  Given b (the
   *     target branching factor) we can solve for p.
   *
   *  A shape function computes a target branching factor b_i
   *     for nodes at depth i as a function of the root branching
   *     factor b_0 and a maximum depth gen_mx.
   *
   *  Default parameter values
   */
  int        gen_mx   = 6;      // default depth of tree
  geoshape_t shape_fn = LINEAR; // default shape function (b_i decr linearly)

  /*  In type HYBRID trees, each node is either type BIN or type
   *  GEO, with the generation strategy changing from GEO to BIN
   *  at a fixed depth, expressed as a fraction of gen_mx
   */
  double shiftDepth = 0.5;

  /* compute granularity - number of rng evaluations per tree node */
  int computeGranularity = 1;

  /* display parameters */
  int debug    = 0;
  int verbose  = 1;
};

typedef struct uts_config UTSConfig;

/* For stats generation: */
typedef unsigned long long counter_t;

/* Utility Functions */
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void   uts_error(char *str);
void   uts_parseParams(UTSConfig *c, int argc, char **argv);
int    uts_paramsToStr(UTSConfig *c, char *strBuf, int ind);
void   uts_printParams(UTSConfig *c);
void   uts_helpMessage();

void   uts_showStats(UTSConfig *c, int nPes, int chunkSize, double walltime, counter_t nNodes, counter_t nLeaves, counter_t maxDepth);
double uts_wctime();

double rng_toProb(int n);

/* Common tree routines */
void   uts_initRoot(UTSConfig *c, Node * root);
int    uts_numChildren(UTSConfig *c, Node *parent);
int    uts_numChildren_bin(UTSConfig *c, Node * parent);
int    uts_numChildren_geo(UTSConfig *c, Node * parent);
int    uts_childType(UTSConfig *c, Node *parent);

/* Implementation Specific Functions */
const char * impl_getName();
int    impl_paramsToStr(char *strBuf, int ind);
int    impl_parseParam(char *param, char *value);
void   impl_helpMessage();
void   impl_abort(int err);


#ifdef __cplusplus
}
#endif

#endif /* _UTS_H */
