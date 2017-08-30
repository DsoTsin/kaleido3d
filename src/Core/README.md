This directory includes:

* **Project config headers** (`Config`) :

  defines COMPILER, PLATFORM, INTRINSICS

* **Kaleido3D template library** (`KTL`) :

  implementation of **Allocator**, **Dynamic Array**, **SharedPtr**, etc. (Why don't use STL? Coz it cannot be compiled across Compilers/Platforms, if we only provide headers and libraries.)

* **Math library** (`Math`) :

  vector and matrix operations with **SIMD(SSE, NEON) acceleration**.

* Interfaces
