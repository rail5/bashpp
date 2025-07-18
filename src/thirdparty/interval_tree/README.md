This header-only implementation of [Interval Trees](https://en.wikipedia.org/wiki/Interval_tree) was written by [Tim Ebbeke](https://github.com/5cript/interval-tree) and is licensed under the [Creative Commons Zero 1.0 Universal](https://creativecommons.org/publicdomain/zero/1.0/deed.en) license.

An interval tree is a data structure that allows for efficient querying of intervals. Interval trees are particularly useful for problems involving ranges, such as finding overlapping intervals or searching for intervals that contain a specific point. We use them in Bash++ to query which code entity is active at a given arbitrary point in the source code.

The files authored by Tim Ebbeke are:

 - `interval_tree.hpp`
 - `interval_tree_fwd.hpp`
 - `interval_types.hpp`
 - `tree_hooks.hpp`
 - `feature_test.hpp`

The files `EntityNode.h` and `EntityMap.h` are GNU GPL v3 additions to give us a slightly higher-level abstraction for our use case.
