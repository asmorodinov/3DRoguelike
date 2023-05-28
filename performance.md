# Testing performance

## Pathfind algorithm with different persistent hash set implementations

Two levels were tested:
- Level with size $50\times 20 \times 50$, $10$ rooms and $17$ corridors.
- Level with size $100\times 40 \times 100$, $10$ rooms and $10$ corridors.

Seed was set to 1236 in both cases.

For each level, time to generate all corridors were measured (in seconds). Room generation was not tested, because it does not depend on persistent hash set data structure.

Different persistent hash set implementations were tested (they can be selected in the [PersistentHashSet.h](3DRoguelike/3DRoguelike/Game\Algorithms\PersistentHashSet.h) header file).

## Contenders

1. `immer::set` from [arximboldi/immer](https://github.com/arximboldi/immer) library - `ImmerPersistentHashSet`.
2. `patricia::IntSet` from [library](https://github.com/asmorodinov/PersistentSet/tree/master) with 64-bit bitmaps in leaves and `TwoPoolsAllocator` ($2^{23}\approx 8.4 \cdot 10^6$ elements in each pool). Time to allocate pools were also counted.
3. `std::unordered_set` from STL.
4. `PersistentLinkedList` - which is implemented in [PersistentHashSet.h](3DRoguelike/3DRoguelike/Game\Algorithms\PersistentHashSet.h) file.
5. `ikos::core::PatriciaTreeSet<ikos::core::Index>` from [NASA-SW-VnV/ikos](https://github.com/NASA-SW-VnV/ikos/tree/master/core/include/ikos/core/adt/patricia_tree).
6. `sparta::PatriciaTreeSet<T>` from [facebook/sparta](https://github.com/facebook/SPARTA/blob/main/include/PatriciaTreeCore.h).
7. `HAMT::Set` from [HAMT.h](3DRoguelike/3DRoguelike/Game/Utility/HAMT.h).
8. `AlwaysEmptyHashSet` from [PersistentHashSet.h](3DRoguelike/3DRoguelike/Game\Algorithms\PersistentHashSet.h) - always returns `false` from `contains` call. Corridors generated with this data structure can have self-intersections, this data structure is included just for testing purposes, it is **not actually a valid implementation**.
9. `SimplePersistentHashSet` from [PersistentHashSet.h](3DRoguelike/3DRoguelike/Game\Algorithms\PersistentHashSet.h) - never has false-negatives in `contains` method, but does have false-positives (about $37.5\%$). This is also **not a valid implementation**, since it can have false-positives. Corridors generated with this data structure can fail to be generated (even when path exists) or can be non-shortest paths, however, they never have self-intersections. In many cases, corridors that are generated look reasonable, and it's OK to use this data-structure in practice (since paths do not necessarily need to be shortest).

## Results
|                           |   1   |   2   |   3    |   4   |   5   |   6   |   7   |   8   |   9   |
| :-----------------------: | :---: | :---: | :----: | :---: | :---: | :---: | :---: | :---: | :---: |
|  $50\times 20\times 50$   | 4.64  | 8.59  |  9.28  | 18.96 | 14.31 | 11.05 | 22.14 | 1.64  | 2.91  |
| $100\times 40 \times 100$ | 16.01 | 22.44 | 235.25 |   -   | 46.28 | 35.60 | 65.96 | 4.58  |  8.9  |

## Statistics
How many `contains`, `insert`, `copy`, `clear` calls were issued in total (per level)? 

Here, `copy` refers to copy assignment operator.

- $50\times 20\times 50$
  - contains - $14,787,285$
  - insert - $5,147,955$
  - copy - $691,125$
  - clear - $1,836,000$
- $100\times 40\times 100$
  - contains - $38,401,614$
  - insert - $12,330,712$
  - copy - $1,976,322$
  - clear - $6,050,000$

How much time was spent on each operation in total (per level)?

Here, statistics are given only for `immer::set`.
- $50\times 20\times 50$
  - contains - $0.055256$
  - insert - $0.598413$
  - copy - $0.114428$
  - clear - $0.546782$
  - total time to generate corridors - $7.426401$ (measuring statistics slows down generation)
- $100\times 40\times 100$
  - contains - $0.070307$
  - insert - $1.246427$
  - copy - $0.396569$
  - clear - $2.320085$
  - total time to generate corridors - $19.933064$

## Analytical complexity
Here are worst-case complexities for set operations.
Worst-case `clear` operation always has $O(n)$ complexity, since it needs to delete all elements from set.

|            |         1         |        2        |   3    |   4    |        5        |        6        |         7         |   8    |   9    |
| :--------: | :---------------: | :-------------: | :----: | :----: | :-------------: | :-------------: | :---------------: | :----: | :----: |
| `contains` | $O(\log_{32}(n))$ | $O(\min(n, W))$ | $O(1)$ | $O(n)$ | $O(\min(n, W))$ | $O(\min(n, W))$ | $O(\log_{32}(n))$ | $O(1)$ | $O(1)$ |
|  `insert`  | $O(\log_{32}(n))$ | $O(\min(n, W))$ | $O(1)$ | $O(1)$ | $O(\min(n, W))$ | $O(\min(n, W))$ | $O(\log_{32}(n))$ | $O(1)$ | $O(1)$ |
|   `copy`   |      $O(1)$       |     $O(1)$      | $O(n)$ | $O(1)$ |     $O(1)$      |     $O(1)$      |      $O(1)$       | $O(1)$ | $O(1)$ |

where $n$ is number of elements in the set, $W$ - size of $T$ in bits, where $T$ is a type of integer keys that set stores.
