# RinOS SDK consumer examples

`CMakeLists.txt` is the host-side public-header check. It compiles every SDK
header in its C11 and C++20 modes and compiles the consumer source files as
separate objects. It does not link for RinOS.

`build.py` cross-compiles, links, validates, and packages the five C examples
(`hello`, `process`, `channel`, `file_portal`, and `window`) for either SDK
target. Pass one installed target prefix produced by `package_rinsdk_v1.py`;
the helper verifies its manifest, public headers, runtime-library hashes, and
signatures before producing application packages. The prefix contains the
declared RinGPU and RinResource public-header dependencies.

Example PowerShell invocation:

```powershell
python build.py `
  --sdk-root E:/RinOS/build/sdk/x86_64-unknown-rinos `
  --target x86_64-unknown-rinos `
  --output-dir E:/RinOS/build/sdk-consumer-packages `
  --rcc E:/RinOS/public-base/toolchain/RinCompiler/rcc.exe `
  --rld E:/RinOS/public-base/toolchain/RinCompiler/rld.exe `
  --rinsign E:/RinOS/public-base/toolchain/rinsign/rinsign.exe `
  --rinpack E:/RinOS/build/tools/rinpack.exe `
  --validate-with E:/RinOS/build/tools/rin-image-v3-validate.exe `
  --rin-sign-key E:/RinOS/build/keys/rin-update-development.pem `
  --rin-public-key E:/RinOS/build/keys/rin-update-trust.der `
  --publisher-key E:/RinOS/build/keys/rin-app-package-development.pem `
  --publisher-public-key E:/RinOS/build/keys/rin-app-package-development.der
```

The image-signing key and application-publisher key have different roles. The
public keys supplied to the command must match the identities trusted by the
target. The generated packages declare the runtime-library package version
selected with `--runtime-version` (default `1.0.0`). `file_portal` accepts a
durable grant ID from the trusted portal handoff; it requests read-only access
and never turns the grant into a pathname.
