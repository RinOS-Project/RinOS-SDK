# RinOS SDK consumer examples

`CMakeLists.txt` is the host-side public-header check. It compiles every SDK
header in its C11 and C++20 modes and compiles the consumer source files as
separate objects. It does not link for RinOS.

`build.py` cross-compiles, links, validates, and packages the C examples
(`hello`, `process`, `channel`, `file_portal`, `window`, `media`, `keyring`,
and `explorer`) for
either SDK target. Pass one installed target prefix produced by
`package_rinsdk_v1.py`;
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

`explorer` is a developer-only text catalog. It lists the sample package IDs,
public entry points, and each sample's declared capability. Use `--run` with
an installed sample's executable path to start it and wait for its exit. The
Explorer requests `process-control`, and it starts children with an empty
capability mask; each sample package still declares and requests its own
capabilities. It is not included in the standard application set.

`media` opens a self-contained 1x1 PNG through the public media API using
`RIN_MEDIA_TYPE_IMAGE`. It handles `RIN_ERROR_NOT_SUPPORTED` explicitly and
requests no filesystem, camera, or microphone capability. The example only
demonstrates typed media-object open/close; it does not claim that a target
provides an image decoder or that media decoding has been accepted.

`keyring` demonstrates the public Keyring v1 wire ABI with a read-only service
status request. It resolves the system Keyring slot, connects to the documented
Unix socket, and checks the kernel-reported service identity against that slot
before sending the request. It declares no capabilities and never sends a
secret name or secret payload; this sample therefore exercises service
identity and availability without granting application access to credentials.
The bounded socket I/O and response validation are local to this small sample;
applications performing secret operations should use the authenticated
RinRuntime Keyring client rather than extending this status-only example.
