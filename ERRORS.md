# Error Log

### FluidSynth PipeWire driver fails to start — 2026-11-07

- **Severity:** Medium
- **Category:** Configuration
- **File(s):** `main.cpp`
- **Pattern:** Requesting `audio.driver = "pipewire"` from libfluidsynth 2.3.x in a host app that never calls `pw_init()`; the driver errors with "Failed to allocate PipeWire loop" and no audio driver is created.
- **Root cause:** FluidSynth 2.3.x's PipeWire driver assumes the host already initialised libpipewire.
- **Fix applied:** Dropped "pipewire" from the driver fallback list; use "pulseaudio" first (served by pipewire-pulse), then "alsa".
- **Prevention rule:** On FluidSynth < 2.4 use the PulseAudio driver on PipeWire systems; always check `new_fluid_audio_driver` for NULL and fall back to another driver.

### General MIDI layers clip when stacked — 2026-11-07

- **Severity:** High
- **Category:** Logic
- **File(s):** `engine.cpp`, `song.cpp`
- **Pattern:** Stacking several General MIDI presets at equal channel volume with a fixed master gain; GM leads/arps are ~5x louder than GM drums, so the full mix clips hard while each track alone sounds fine.
- **Root cause:** Master gain (0.8) was picked for one instrument, and default track volumes were all 100.
- **Fix applied:** Rendered each track solo, measured peak/RMS, balanced default CC7 volumes per track and lowered master gain to 0.4 (full drop now peaks ~22000/32767).
- **Prevention rule:** After changing instruments, gain or reverb defaults, render the full arrangement and check its peak stays below ~0.8 full scale before shipping.

### QJsonObject index chained on a non-const object fails to compile — 2026-11-07

- **Severity:** Medium
- **Category:** API Misuse
- **File(s):** `song.cpp`
- **Pattern:** `obj["key"][i]` or `QString::value(i)` on Qt 6.4; non-const `QJsonObject::operator[]` returns `QJsonValueRef`, and `QString` has no `value()`.
- **Root cause:** Assumed newer Qt convenience API.
- **Fix applied:** Read parsed JSON through `const QJsonObject` (operator[] returns `QJsonValue`, which supports `[i]`) and used `mid(c, 1)`.
- **Prevention rule:** Hold parsed JSON in `const` objects when only reading; check APIs against Qt 6.4 docs (Ubuntu 24.04's version).

### assert-based self-check silently passes in Release — 2026-11-07

- **Severity:** Medium
- **Category:** Build
- **File(s):** `song_test.cpp`
- **Pattern:** An `assert()` test compiled in a `CMAKE_BUILD_TYPE=Release` tree, where `NDEBUG` removes every assert.
- **Root cause:** The build directory was configured as Release earlier.
- **Fix applied:** `#undef NDEBUG` before the includes in the test file.
- **Prevention rule:** Every assert-based test file starts with `#undef NDEBUG`.

### Endless MPK knobs read as positions jump 0 <-> 127 — 2026-11-07

- **Severity:** High
- **Category:** Logic
- **File(s):** `engine.cpp`, `knobreader.h`, `panels.cpp`
- **Pattern:** Treating every MIDI CC value as an absolute 0-127 position. Endless encoders in relative mode send steps (1 = +1, 127 = -1), so a tiny back-and-forth turn flips the setting between the extremes. Separately, `QDial`'s circular mouse drag jumps min <-> max when the pointer slips across the bottom gap.
- **Root cause:** Assumed the MPK knobs were absolute pots; the earlier log showed `CC 73 = 1` from a tiny turn, which was the clue.
- **Fix applied:** `KnobReader` decodes two's-complement relative steps with auto-detection (repeated small values or a 1 <-> 127 flip), plus manual mode and speed settings; the on-screen dials use vertical drag instead.
- **Prevention rule:** Never assume CC values are absolute for hardware knobs: support relative encoders (auto-detect or setting), and test with the real controller's raw output (`aseqdump`).

### FluidSynth file renderer cannot write MP3 — 2026-11-07

- **Severity:** Medium
- **Category:** Configuration
- **File(s):** `engine.cpp`
- **Pattern:** Using `new_fluid_file_renderer` with `audio.file.type = "m1a"` for MP3; FluidSynth 2.3 lists the type but sets no MPEG subtype, so the renderer returns NULL.
- **Root cause:** FluidSynth only maps PCM sample formats (and Vorbis for Ogg) onto libsndfile subtypes.
- **Fix applied:** Render with `fluid_synth_write_float` and write the samples through libsndfile directly (`SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_III`, Vorbis, FLAC, WAV, AIFF).
- **Prevention rule:** For compressed export formats, drive libsndfile yourself rather than FluidSynth's file renderer, and test-write each format with `file` to check the result.

### Tutorial highlight names silently unmapped — 2026-11-07

- **Severity:** Low
- **Category:** Logic
- **File(s):** `mainwindow.cpp`, `guide.cpp`
- **Pattern:** A `GuideStep::show` name ("save", "export") with no entry in `MainWindow::named`; `QHash::value` returns an empty list, so nothing is outlined and nothing complains.
- **Root cause:** The buttons are locals in `buildTopBar()` and were never registered.
- **Fix applied:** Registered them in `buildTopBar()`.
- **Prevention rule:** When adding a name to a guide step's `show` list, grep `named["<name>"]` to confirm it is registered.

### Record switched itself off when starting playback — 2026-11-07

- **Severity:** High
- **Category:** Logic
- **File(s):** `melody.cpp`, `engine.cpp`
- **Pattern:** Calling a "start" function that internally calls "stop" first, while a listener reacts to the stop signal by undoing the state that triggered the start (re-entrant signal feedback).
- **Root cause:** `Engine::play()` calls `stop()`, which synchronously emits `position(-1)`; the melody maker's handler unchecks Record on that signal, so pressing Record started the loop but immediately turned recording off.
- **Fix applied:** Block the engine's signals with `QSignalBlocker` around `eng.play()` inside the Record handler.
- **Prevention rule:** When a handler starts playback, check whether `play()` emits a synchronous "stopped" signal that other handlers react to; block signals or set state after the call.
