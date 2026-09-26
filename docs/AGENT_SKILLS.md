# Skills an AI agent needs to contribute to Riplay

Ordered by importance. Levels: **core** (cannot work without it), **solid** (needed for
most tasks), **extra** (needed for specific areas).

## Core

1. **C++17** - classes, RAII, `std::unique_ptr` / `std::shared_ptr`, references over raw
   pointers, no manual `new`/`delete`. Read `src/models/models.h` for house style.
2. **GTK4 / gtkmm-4.0** - widget hierarchy, `sigc::signal` connections, `Gtk::Builder`
   from gresource, `Glib::RefPtr` ownership, `Gtk::Application` lifecycle and actions.
   This is the single largest learning curve in the repo.
3. **Meson + Ninja** - `meson.build` files, `files()`, `custom_target()`, `dependency()`.
   Knowing that new sources must be registered is mandatory, not optional.
4. **Reading a build error** - the only feedback loop is the compiler, so skill at mapping
   template, incomplete-type, and missing-header errors to the right header.

## Solid

5. **Blueprint** (`.blp` syntax) - the entire UI is declarative; changing layout means
   editing `data/ui/*.blp`, not C++.
6. **GResource** - how `data/resources.gresource.xml` becomes the binary bundle and why a
   new file must be listed there.
7. **TagLib** - reading ID3/FLAC/Vorbis tags, embedded album art, synchronized lyrics
   (`SYLT`/LRC) and unsynchronized lyrics frames.
8. **Shell and git** - `meson setup`/`compile`, `ninja`, `blueprint-compiler`, and the
   repo's commit/branch conventions (`dev` to `master` via PR).
9. **Markdown** - `README.md`, `docs/DEVNOTES.md`, and updating docs when behavior changes.

## Extra (area specific)

10. **Debugging** - `gdb` breakpoints on signal handlers, `G_DEBUG=fatal-warnings`,
    `G_MESSAGES_DEBUG=all`, valgrind or ASAN for leaks in metadata and album art.
11. **Concurrency** - `GMutex`, audio callback vs UI thread, and the rules for touching
    widgets only from the main loop.
12. **Audio internals** - libmpg123 decoding, sample rate/channels/bitrate, and GStreamer
    pipelines (present but commented out in `meson.build`).
13. **Testing with Meson** - the repo has no real tests; knowing how to add a
    `test()` target is valuable for metadata and lyrics parsing, which are pure logic.
14. **Packaging and install** - Meson install rules, desktop entries, GApplication CLI
    (`on_open`, file arguments).
15. **Code navigation at scale** - grep across `src/` plus `compile_commands.json` for
    clangd/LSP context; see `docs/DEVNOTES.md` for the required include flags.
