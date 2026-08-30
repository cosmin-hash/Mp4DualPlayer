# C++ Coding Standard

The shared standard for these public projects:

| Project | Kind | Sources |
|---|---|---|
| `Threading` | Qt Widgets, 4 sub-projects (`concurrency-01..04`) | 44 files |
| `QtOpenGLQuadPlayers` | Qt + OpenGL video | 14 files |
| `Mp4DualPlayer` | Qt Multimedia | 8 files |
| `PBRViewer` | Qt + OpenGL renderer | 9 files |
| `ConcurrentInferenceServer` | Non-Qt, llama.cpp + CUDA | 7 files |

It was **derived from the existing code**, not imported from a stock preset. Four
of the five projects already agreed on almost everything; this writes it down and
makes it enforceable.

---

## 1. Mechanics — identical in all five projects

Enforced by a byte-identical `.clang-format` in every repo.

| Rule | Value |
|---|---|
| Indent | 4 spaces, never tabs |
| Column limit | 110 |
| Braces | Attached (`if (x) {`) |
| Pointers / references | Bound left: `QWidget* parent`, `const Mesh& m` |
| Include guards | `#pragma once` — never `#ifndef` |
| Include order | Preserved, never auto-sorted |
| Namespace bodies | Not indented |
| Trailing comments | 3 spaces before `//`, aligned in runs |
| C++ standard | **C++20** |
| Line endings | LF in the repo (`.gitattributes` + `.editorconfig`) |

Short constructs stay on one line — accessors, guard clauses, `if (x) a; else b;` —
because all five codebases use that deliberately for compact math and state logic.

## 2. Naming — two dialects, on purpose

**Qt/UI cluster** (`Threading`, `QtOpenGLQuadPlayers`, `Mp4DualPlayer`, `PBRViewer`):

| Kind | Style | Example |
|---|---|---|
| Types | `CamelCase` | `GLViewport`, `DecodedFrame` |
| Methods / functions | `camelBack` | `computeBounds`, `popLatest` |
| Members | `m_` + `camelBack` | `m_capacity`, `m_showAxes` |
| Enums | `enum class`, `CamelCase` | `RenderMode::Shaded` |
| Global constants | `k` prefix | `kReaderPt`, `kRadius` |
| Namespaces | `lower_case` | `pbr` |

**`ConcurrentInferenceServer`** keeps STL/library-style naming — `lower_case`
methods, `trailing_` members, `.hpp` headers. This is **deliberate and not a
deviation to fix**: it is a non-Qt, header-library-shaped codebase whose public
surface (`BoundedQueue::push` / `try_push` / `pop`) intentionally mirrors the
standard library. Renaming it would churn the API for no benefit.

Both dialects are encoded in each repo's `.clang-tidy` via
`readability-identifier-naming`.

## 3. Warnings

Every project defines a `project_warnings` INTERFACE target:

```cmake
option(${PROJECT_NAME}_WERROR "Treat compiler warnings as errors" OFF)
add_library(project_warnings INTERFACE)   # linked PRIVATE into each real target
```

- **INTERFACE target, not `add_compile_options`** — directory-scoped flags leak
  into vendored dependencies. `ConcurrentInferenceServer` builds llama.cpp via
  `FetchContent`; global flags would bury its own warnings under thousands from
  vendored code.
- **Scoped to `$<COMPILE_LANGUAGE:CXX>`** — nvcc treats `/W4` as an input file
  and fails with *"A single input file is required for a non-link phase"*.
- **GCC/Clang**: `-Wall -Wextra -Wpedantic` plus `-Wnon-virtual-dtor`,
  `-Woverloaded-virtual`, `-Wnull-dereference`, `-Wformat=2`, `-Wcast-align`,
  `-Wunused`. **MSVC**: `/W4 /permissive-`.
- **`-Wshadow` is deliberately excluded.** Measured: 11 hits in PBRViewer alone,
  all Qt signal/slot lambdas naming a parameter after an enclosing local. All
  benign, and keeping the flag would put `_WERROR` permanently out of reach.
- `_WERROR` defaults **OFF**. Turn it on per project once that repo is clean.

### Exception: ConcurrentInferenceServer

Its pre-existing global `add_compile_options` block is **intentionally left in
place**. `/Zc:char8_t-` must reach the vendored llama.cpp sources — the pinned
tag uses `u8"..."` literals that MSVC treats as `char8_t[]` under C++20 — so it
cannot be scoped to our own target.

## 4. Tooling

```
pip install clang-format==23.1.0 clang-tidy==22.1.8
```

Installs to `%APPDATA%\Python\Python314\Scripts`, which is **not** on PATH by
default — add it. Config keys were validated against these exact versions; note
LLVM 22/23 renamed several (`Attach` not `Attached`, `BreakTemplateDeclarations`,
`KeepEmptyLines`), so configs written for older LLVM will not parse.

```
clang-format --dry-run --Werror <files>   # report drift (emits error:, not warning:)
clang-format -i <files>                   # apply
clang-tidy -p build src/*.cpp             # needs compile_commands.json
```

Every repo's CI has a non-blocking `format` job. `CMAKE_EXPORT_COMPILE_COMMANDS`
is ON everywhere so clang-tidy and editors find the flags.

---

## 5. Adoption status — APPLIED

The reformat has been **run across all five repos** (82 files). Residual drift is
**0** everywhere, so the CI `format` job passes as-is.

Applied diff, source files only:

| Project | Files | Diff |
|---|---|---|
| `Threading` | 40 | +1003 / -1117 |
| `QtOpenGLQuadPlayers` | 12 | +148 / -174 |
| `Mp4DualPlayer` | 8 | +435 / -649 |
| `PBRViewer` | 8 | +508 / -248 |
| `ConcurrentInferenceServer` | 7 | +70 / -88 |

The churn was dominated by trailing-comment spacing and by column alignment of
declarations that clang-format re-flows. Tuning was attempted and hit diminishing
returns: `AlignConsecutive*: Consecutive` made it **worse** (it adds alignment
where the code had none). A ~25-50% one-time diff is simply what this costs.

Commit the reformat as its own isolated commit, then record it so `git blame`
skips it:

```
echo "<commit-sha>" >> .git-blame-ignore-revs
git config blame.ignoreRevsFile .git-blame-ignore-revs
```

## 6. Status

**Warnings: all fixed.** Every project now builds with **zero warnings** under
the shared flag set.

| Location | Was | Fix |
|---|---|---|
| `PBRViewer/src/MainWindow.cpp` | `-Wmisleading-indentation` | Split the two `while` loops onto separate lines. |
| `Threading/concurrency-02/src/Token.h` | `-Wcomment` (x4) | Redrew the ASCII diagram with `+` junctions instead of trailing `\`. |
| `QtOpenGLQuadPlayers/src/GLViewport.cpp` | `-Wunused-parameter` | Unnamed the vestigial `img` parameter; `uploadToTexture` is what consumes the pixels. |

**`<algorithm>`: fixed.** `PBRViewer/src/Camera.h` and `GLViewport.cpp` now
include it directly instead of relying on Qt to pull it in transitively.

**Verified running.** All seven Qt executables launch and stay up; PBRViewer
renders a frame via `--shot`; `geometry_tests` passes; `inference_server` runs a
real 4-request / 2-worker concurrent inference against a local GGUF model.

### Still open

**Remaining include hygiene (27).** Every direct gap is fixed -- `<algorithm>`
plus `<utility>`, `<memory>`, `<string>`, `<functional>`, `<optional>`,
`<vector>` and `<cstdint>` are now included where used. What remains are 27
findings that each file's *paired header* already satisfies, so they are
correct today and only a tidiness question, not a fragility one.

**`_WERROR` is still OFF** in all five repos. Every project is warning-free, so
it can be turned on whenever you want CI to enforce it.

**CI `format` jobs are still `continue-on-error: true`.** Drift is 0, so removing
that line in each of the five workflows makes formatting a hard gate.

**C++ standard drift** — `Threading/concurrency-01..03` are C++17 and
`concurrency-04` is C++23, against a C++20 baseline everywhere else. Raising
01-03 to C++20 is safe; dropping 04 from 23 to 20 needs a check for C++23-only
usage first.

**Local toolchain — use Qt's own MinGW, not msys2.** Qt 6.9.2 `mingw_64` was
built with **GCC 13.1.0** (`C:/Qt/Tools/mingw1310_64`). The msys2 UCRT64 compiler
on this machine is **GCC 15.2.0**. Linking Qt libraries with the msys2 compiler
fails at the link step with a silent `collect2.exe: error: ld returned 1 exit
status` and no diagnostic. It only bites executables that pull in
`libQt6EntryPoint.a` (i.e. `WIN32`/`-mwindows` GUI targets), which is why
PBRViewer linked while Threading and QtOpenGLQuadPlayers did not.

Configure with Qt's matching toolchain:

```
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64" ^
  -DCMAKE_C_COMPILER="C:/Qt/Tools/mingw1310_64/bin/gcc.exe" ^
  -DCMAKE_CXX_COMPILER="C:/Qt/Tools/mingw1310_64/bin/g++.exe" ^
  -DCMAKE_MAKE_PROGRAM="C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe"
```

All seven Qt targets build and link cleanly this way, with zero warnings.

Do **not** put `C:/Qt/6.9.2/mingw_64/bin` on `PATH` while configuring or
building — its runtime DLLs break the compiler probe. Add it (plus
`C:/Qt/Tools/mingw1310_64/bin`) only when *running* binaries or `ctest`;
without it the binaries exit with `0xc0000135` (DLL not found).
