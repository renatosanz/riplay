# AGENTS.md

Context for AI agents working on this repository. Read before editing code.

## What this is

Riplay: a music player built on GTK4. Features: synchronized/unsynchronized lyrics,
extra metadata view, recents list.

## Stack

- Language: C++ (gtkmm-4.0 bindings, GTK4 under the hood)
- Build: Meson + Ninja, `c_name: 'resources'` gresource bundle
- UI: Blueprint (`.blp`) compiled to `.ui` at build time, loaded via `Gtk::Builder`
- Metadata: TagLib (linked static, v2.1.1+ required)
- Audio: libmpg123; GStreamer/libav/SDL2 blocks are commented out in `meson.build`
- License: GPL-3.0-or-later

## Layout

| Path | Contents |
| --- | --- |
| `src/main.cpp` | entry point, creates `AppState` and runs it |
| `src/models/` | `models.h` holds `AppState`, `SongInstance`, `Home/Recents/PlayerInstance`; `state.cpp` wires views |
| `src/views/` | per-view logic: `home.cpp`, `player.cpp`, `recents.cpp` (`actions`, `equalizer`, `visuals` are not compiled) |
| `src/metadata/` | TagLib parsing, album art, lyrics parser and `LyricsManager` |
| `src/logic/` | file history / recents persistence |
| `src/utils/` | Builder and memory helpers |
| `src/graphics/` | standby drawing, not compiled |
| `src/types.h` | plain C structs: `FileMetadata`, `LyricBar`, `AudioProps` |
| `data/ui/*.blp` | UI definitions (generated `.ui` land in `builddir/data/ui/`) |
| `data/resources.gresource.xml` | lists every resource to bundle |
| `include/` | legacy headers, mostly stale; prefer `src/` |
| `docs/DEVNOTES.md` | editor/LSP setup notes |

## Build and verify

```bash
meson setup --wipe builddir      # first time only
meson compile -C builddir        # this is the only verification step
./builddir/riplay <file.mp3>     # manual smoke test
```

There is no test suite, linter, or `.clang-format` config. `meson.build` declares
`test('basic', exe)` only. Compiler is `warning_level=2`, `werror=false`, so
warnings do not fail the build: read the warning output and do not leave new ones.

## Rules that break the build silently

- Adding a `.cpp` without adding it to the `meson.build` of its directory means it is
  never compiled and there is no error.
- Adding a `.blp` needs no meson change (globbed by `find`), but any new resource
  file referenced at runtime must be added to `data/resources.gresource.xml`.
- Include paths are resolved through `include_directories()` for `src`, `src/utils`,
  `src/views`, `src/graphics`, `src/logic`.

## Conventions

- 2-space indent, ~80 column limit, LLVM-style formatting.
- Views are classes taking `AppState *state` in the constructor; widgets held as
  `Glib::RefPtr`, ownership via `std::unique_ptr` / `std::shared_ptr`.
- Signals use sigc; async work uses `Glib::AsyncResult`; audio data guarded by `GMutex`.
- No comments unless explicitly requested.
- Never commit: `builddir/`, `data/ui/*.ui` (generated), `*.mp3`, `compile_commands.json`
  (a symlink into `builddir`).
- Commits: short imperative subject, e.g. `add cli open file support`.
  Branches off `dev`, merged into `master` via PR. Do not commit or push unless asked.

## Before you finish

Run `meson compile -C builddir`, confirm it links, and launch the binary once if the
change touches views, metadata, or audio.
