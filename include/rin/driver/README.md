# Public Driver headers

`abi_v3.h` is the fixed-width public NDRV v3 contract.  It is intentionally
independent of OS-Core headers.  Drivers receive opaque handles and
generation-bound resource descriptors; kernel-private structs and physical
pointers are not part of this include tree.

The NDRV v1/v2 compatibility headers are not placed under this directory.
They belong to the explicitly legacy runtime and are not an NDRV v3 include
dependency.
