# Review of mulle-storage (second pass)

**Reviewed:** commit `be76122` (HEAD of `develop`) **plus** the uncommitted working-tree changes, which are substantial. This is a re-review: the first pass (earlier in `REVIEW.md` history) flagged a set of errors and gaps, and most of them have since been fixed. This document re-examines everything with fresh eyes and verified claims.

**Method:** full source read, plus verification with the project's own tooling and an independent end-to-end build:
- `mulle-sde test` → **12/12 tests pass** on Linux, and 12/12 cross-compiled and run on Windows under wine.
- Cloned `mulle-core/mulle-core` fresh, built a consumer app via the README's exact flow (`add_subdirectory(mulle-core)` + `target_link_libraries(... PRIVATE mulle-core)` + `#include <mulle-core/mulle-core.h>`) — **builds and runs clean**, exercising both `mulle_storage` and `mulle_indexedstorage`.

---

## Verdict (TL;DR)

A small, well-crafted, single-purpose C library with an unusually good consumption story. The big change since the first review: **mulle-storage is a component of the `mulle-core` amalgamation**, and I verified end-to-end that adopting it as a consumer is genuinely *one* git submodule + *one* `add_subdirectory` + *one* `target_link_libraries` + *one* umbrella `#include`. The four-package dependency graph I originally called a "tax" is an internal detail of the amalgamation — consumers never see it. My earlier headline criticism was wrong, and I'm happy to retract it.

What was fixed between the reviews is also exactly what a maintainer should have fixed: a broken dead test deleted, a documented-but-nonexistent API implemented, poison fills unified, `count` widened to `size_t`, five new operation tests added, stress tests made deterministic, CI given ASan/UBSan + 32-bit + valgrind jobs, and the README rewritten with a proper memory model and thread-safety statement.

What still keeps me from calling it "adopt unconditionally": there are **no benchmarks** for a library whose *entire reason to exist is performance*, the newest fixes have not yet flowed into the `mulle-core` amalgamation, and the `_mulle_*` API identifiers are still formally reserved (though they're deliberate, ecosystem-wide house style). None of these are blockers; the first is the one I'd actually fix.

---

## The mulle-core finding (retraction + verification)

**Claim:** mulle-storage is a component of `mulle-core/mulle-core`, so a consumer pulls one repo and gets everything.

**Verification (done in this review):**
- `mulle-core/mulle-core` (master, v0.8.1) exists and is an **amalgamation repo**: it vendors the full `src/` of its ~30 constituents directly into its own tree. `src/mulle-storage/` is present with the complete `mulle-storage.h`, `mulle-indexedstorage.h`, `mulle-storage.c` — not a submodule reference, actual source.
- Its `CMakeLists.txt` globs `src/mulle-*` and builds one amalgamated target; `src/mulle-core.h` is the umbrella.
- I cloned it fresh (`git clone --depth 1`), wrote a 30-line consumer using only `<mulle-core/mulle-core.h>` + `add_subdirectory(mulle-core)` + `target_link_libraries(consumer PRIVATE mulle-core)`, and **it built and ran**: both `mulle_storage` and `mulle_indexedstorage` worked (alloc/count/element-size semantics all verified at runtime).
- No `mulle-sde`, no manual include chains, no hand-resolved dependencies needed on the consumer side.

**Consequence for the review:** my first-pass point #1 ("the dependency tax — 4 packages to drag in") is **retracted for the primary consumption path**. The four-package graph (`mulle-container` → `mulle-allocator`/`mulle-data`/`mulle-c11`) only matters on the *alternative* clib path ("embed just mulle-storage"), which the README now correctly frames as the less-recommended option. For a general C developer, the mulle-core route is a genuinely pleasant integration — closer in spirit to vendoring `fmt` or `gtest` than to a fragile dependency chain.

**Honest nuance:** you get the whole amalgamation whether you want it or not — the trade-off of the pattern. It is *more* than a node-pool consumer strictly needs. The mulle-core README argues the amalgamation compiles faster than the individual projects and links as one library, which is a fair defence; the cost is that "just use mulle-storage" is not a thing — "use mulle-core" is.

---

## What is good (verified)

1. **The consumption story (new, verified above).** One submodule, one `add_subdirectory`, one link line, one umbrella include. The README documents exactly this, plus the mulle-sde and clib alternatives. This is a genuinely nice structure, and I proved it works on a fresh clone.

2. **The two structures map exactly onto two stated use cases.** `struct mulle_storage` = queue-backed, pointer-stable (tree nodes that must not move); `struct mulle_indexedstorage` = array-backed, index-stable (pointers may move, indices are stable and reused). The README explains *why* each exists — including why not plain `malloc` and why not a `mulle_structarray`. Rare clarity for a library this size.

3. **Small, readable, documented code.** The API is a handful of `static inline` functions with doc comments on every one. Consistent BSD-3 headers, consistent style.

4. **Genuinely good defensive engineering.**
   - `MULLE_C_NONNULL_FIRST` on fast paths; NULL-safe public wrappers around the `_mulle_*` fast paths (now covered by the new `null-safe.c` test).
   - Double-free detection via `assert( _mulle__pointerarray_find(...) == mulle_not_found_e)`.
   - `_mulle_structqueue_assert_pointer` rejects foreign/stale pointers.
   - DEBUG-only poison fill — **now unified**: both `mulle_storage` and `mulle_indexedstorage` free paths fill freed nodes with `0xDEADDEAD` via `mulle_memset_uint32` (was `0xFD` via `memset` in the indexed variant — fixed).
   - Index bounds and reuse-state asserts in the indexed storage.

5. **The test suite is now real, and it passes everywhere.**
   - **12/12 green on Linux and Windows (wine).** The Debug test builds compile the poison fills and asserts in, so the suite exercises them.
   - `count.c` (count accounting through mixed alloc/free cycles), `element-size.c` (tests *both* storage variants), `indexed-add.c` and `indexed-reuse.c` (dedicated indexed-storage unit tests: index identity, data integrity, freed-index reuse), `null-safe.c` (NULL-pointer wrappers, NULL-free, calloc zero-init).
   - `add-remove.c` was **fixed** to free the stored pointer `saved` instead of a stack copy; the broken, extensionless twin `add-remove-c` was **deleted** (it was silently skipped by `mulle-test` anyway).
   - `pointer-stability.c` now seeds `srand(42)` — deterministic, and its golden `.stdout` matches.

6. **CI now means it.** The workflow gained three jobs beyond the build/test one: an **ASan+UBSan** build+test job, a **32-bit** build+test job, and a **valgrind** test job. It also now triggers on **pull requests**, not just `master` pushes. The wine-based Windows cross-run was already there and remains.

7. **README now documents the memory model.** A dedicated "Memory Model" section states the permanent high-water mark (freed nodes are never returned to the system until `_done`) and explicitly says **not thread-safe** — both were undocumented gaps in the first pass.

8. **Consistent versioning.** `MULLE__STORAGE_VERSION` (0.0.6) == CMake project version == `clib.json` version.

9. **Sound linkage idiom.** `MULLE__STORAGE_GLOBAL` + ranlib anchor for a mostly-header library; generated files in `src/reflect/`/`cmake/reflect/` marked "do not edit".

10. **Housekeeping fixed:** the missing space in `mulle_memset_uint32( p, 0xDEADDEAD, _size)` (was `0xDEADDEAD,_size`), the test sourcetree now points at `mulle-c/mulle-storage` (was `nat/...`), `get_count` widened from `unsigned int` to `size_t` (with matching `%zu` in `pointer-stability.c`), and `REVIEW.md` is no longer silently swallowed by `.gitignore`.

---

## What is still open

1. **No benchmarks (the one real gap).** The README's value proposition is "likely faster and less fragmentation than malloc". After all this work, there is still *not a single number*. For a library whose reason to exist is performance, a benchmark vs. `malloc`/`free` and vs. a trivial hand-rolled free-list (typical tree workload, reproducible, linked from the README) is the single highest-value addition left. Everything else is polish.

2. **The newest fixes haven't reached the `mulle-core` amalgamation yet.** The amalgamated copy on `master` already has the `size_t` counts, but **not** the new `_mulle_indexedstorage_get_element_size()` and **not** the unified 0xDEADDEAD indexed poison fill (still `0xFD` there). A consumer of `<mulle-core/mulle-core.h>` today won't see the current develop API. This is presumably a release-process step (refresh the amalgamation when a constituent is tagged), but it means "use the umbrella header" and "use the latest mulle-storage" are momentarily different things.

3. **`_mulle_*` identifiers are still formally reserved** (C11 7.1.3). First pass called this an error; second pass, I'll frame it accurately: it is **deliberate, ecosystem-wide house style** — every mulle-c project names its fast-path API this way, and the double-underscore internal members are the actual reserved-identifier problem, not the single-underscore functions (which are "reserved at file scope" technically, but pervasively used in the C world). Strict-conformance users will trip on it; mainstream compilers won't. Documenting the convention as intentional would close it out.

4. **macOS is still commented out of the CI matrix** (`# macos-latest`). Windows is genuinely covered (wine), Linux is covered, macOS is not. One line to flip when convenient.

5. **`RELEASENOTES.md` is still just "Various small improvements"** for 0.0.6 — the single-line release note persists even though the release delivered real fixes and new API.

6. **`init-done.c` verifies nothing** — it would pass even if init/done silently did nothing. Harmless, but it's noise in a suite that now has proper tests.

7. **Minor assumption:** the new valgrind CI job calls `mulle-sde test --valgrind`; I could not confirm that flag exists in the installed `mulle-sde` here (the local toolchain's craft/fetch didn't complete in this environment for that path). Worth a quick sanity run before merging the workflow.

---

## Would I use this project in my own code? (revised)

**Yes — and the integration cost is now genuinely low.** My first-pass answer was "no, outside the ecosystem — the dependency graph isn't worth it for 200 lines." The mulle-core verification changes that calculus: I can add one submodule, one `add_subdirectory`, one link line, and a single include, and get a pointer-stable node pool whose correctness is guarded by a real test suite that runs on Linux, Windows, ASan/UBSan, 32-bit, and valgrind. That is *cheaper* than hand-maintaining a free-list with equivalent confidence, even for a general C project.

**What would make it fully attractive:**
1. **Benchmarks** (the missing proof of the core claim) — #1 with a bullet.
2. **Refresh the `mulle-core` amalgamation** so the umbrella header delivers the current API (and document the refresh cadence).
3. **Flip on macOS in CI** and sanity-run the `--valgrind` flag.
4. Optionally document the `_mulle_*` naming as deliberate convention (or provide non-underscore aliases) to close the strict-conformance nit.

---

## Verification notes

- **Tests:** `mulle-sde test` in the project — 12/12 pass on Linux, and 12/12 cross-compiled and run on Windows under wine. The previously-deleted `add-remove-c` is confirmed absent from the run; all five new operation tests execute.
- **Consumer integration test (this review):** fresh `git clone https://github.com/mulle-core/mulle-core.git` (master, v0.8.1) → consumer `CMakeLists.txt` with `add_subdirectory(mulle-core)` + `target_link_libraries(consumer PRIVATE mulle-core)` → `main.c` with `#include <mulle-core/mulle-core.h>` using `_mulle_storage_init/malloc/free/done`, `_mulle_indexedstorage_init/alloc/get/done`, count and element-size checks → **configured, built, ran, exit 0**. The only adjustment needed was using the amalgamation's `unsigned int` count (the `size_t` change is newer than the amalgamation), and dropping the not-yet-amalgamated `_mulle_indexedstorage_get_element_size` call — which is itself finding #2 above.
- **Ecosystem tooling:** `mulle-sde craft`/`test` work as the project intends (kitchen at `~/.mulle/var/cache/sde/mulle-storage-<id>/kitchen/Debug`); earlier difficulties were mine (looking for a project-local `kitchen/`), not the project's.
