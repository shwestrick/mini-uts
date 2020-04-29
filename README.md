# mini-uts
This is a minimal implementation of the Unbalanced Tree Search benchmark,
derived from the authors' original code, version 2.1. I deleted all the files
I didn't need, and made some changes to avoid compiler warnings/errors.

At the moment it just does a depth-first search, but I may add parallelism
later.

For more information, check out the [UTS sourceforge page][sf], read the
[paper][paperlink] [1], or download the full code:
```
git clone https://git.code.sf.net/p/uts-benchmark/code uts-benchmark-code
```

[1] S. Olivier, J. Huan, J. Liu, J. Prins, J. Dinan, P. Sadayappan, C.-W. Tseng,
"[UTS: An Unbalanced Tree Search Benchmark][paperlink]", 19th International
Workshop on Languages and Compilers for Parallel Computing (LCPC 2006), 2006. 

[sf]: https://sourceforge.net/p/uts-benchmark/wiki/Home/
[paperlink]: https://people.eecs.ku.edu/~jhuan/papers/lcpc06.pdf
