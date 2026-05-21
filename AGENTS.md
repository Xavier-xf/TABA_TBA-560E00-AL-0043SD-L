# Repository Guidelines

## Project Structure & Module Organization
- `app_cu_datin/` contains the main hall-machine application. Source lives under `system/`, shared code under the `*_lib/` folders, and UI assets under `system/ui/`.
- `AK37E_SDK_V1.03/` is the vendor SDK and packaging tree. Kernel, U-Boot, and release images live here, with generated artifacts under `upgrade/` and `tools/burntool/platform/`.
- `switch/` holds the floor-branch MCU work and related notes.
- Root `README.md` and `app_cu_datin/README.md` are the primary build and deployment references.

## Build, Test, and Development Commands
- `./build.sh -a` runs the full SDK build and package flow.
- `./build.sh -u`, `-k`, `-r`, and `-cp` build U-Boot, kernel, rootfs/package, or copy release files.
- `cd app_cu_datin && ./autobuild.sh -all-sdk` builds the app, copies `ANYKA37E.BIN` and `system/ui/rom.bin`, then repacks the SDK.
- `cd app_cu_datin && make`, `make lib`, `make clean` build the app, build only `libleo_37e.a`, or remove local outputs.

## Coding Style & Naming Conventions
- Follow the existing embedded C/C++ style in each file. Use tabs in `Makefile` recipes and match nearby indentation in source files.
- Prefer `snake_case` for functions and filenames, `ALL_CAPS` for macros/constants, and keep module prefixes already used in the codebase.
- Do not reformat vendor or generated SDK files unless a build fix requires it.

## Testing Guidelines
- There is no standalone unit test suite in this repository. Validate changes by rebuilding the touched target and, when relevant, packaging an image.
- For application changes, run `app_cu_datin/autobuild.sh -all-sdk`. For SDK or packaging changes, run the narrowest `build.sh` target that exercises the change.
- Treat a successful image build plus device smoke test as the default acceptance check.

## Commit & Pull Request Guidelines
- Commit history uses short scope-prefixed messages, often in the form `fix(system): ...` or `refactor(user_main): ...`.
- PRs should state what changed, which build commands were run, and any hardware or packaging notes needed to reproduce the result.

## Security & Configuration Tips
- The app Makefile expects the cross-toolchain at `/opt/arm-anykav500-linux-uclibcgnueabi/bin/`. Update it only if the build environment changes.
- Generated outputs under `upgrade/`, `tools/burntool/platform/`, and `app_cu_datin/ANYKA37E.BIN` should not be committed unless you intentionally refreshed release artifacts.
