# mkvnote

A CLI and GUI tool for embedding key:value tags in Matroska (MKV) files, built for the Smithsonian National Museum of African American History and Culture (NMAAHC) archival standards.

## Overview

mkvnote provides tools for viewing and modifying global tags embedded in Matroska video files. It supports single-file GUI editing, batch tagging via CSV, tag export to CSV, and tag inspection from the command line. Tags are organized into read-only technical fields, NMAAHC standard tags, and any extra tags already present in the file.

mkvnote has two components:

| Component | Description |
|-----------|-------------|
| `mkvnote` | Bash CLI for batch tagging, CSV export, tag inspection, and setting or removing individual tags. Launches the GUI when given a single MKV file. |
| `mkvnote-gui` | Qt 6 graphical editor for viewing and editing tags on a single MKV file. |

Developed by Smithsonian NMAAHC in collaboration with Dave Rice.

## Features

- **GUI metadata editor** — Qt window for viewing and editing tags on a single MKV file
- **CSV batch tagging** — embed metadata from a CSV into multiple MKV files at once
- **CSV export** — extract tags from one or more MKV files into CSV format
- **Tag inspection** — formatted table output (`-i`) and literal/debug output (`-ii`)
- **Single-tag operations** — set (`-s`) or remove (`-r`) one tag across multiple files from the command line
- **Read-only protection** — technical tags (`ENCODER`, `VIDEO_STREAM_HASH`, `AUDIO_STREAM_HASH`) and attachment info are displayed but not editable
- **Multi-line field support** — `DESCRIPTION`, `ENCODER_SETTINGS`, `_PRE_TRANSFER_NOTES`, and `_TRANSFER_NOTES` render as multi-line text areas
- **Logging** — every write operation creates a `<filename>_mkvnote_tags.log` alongside the MKV with timestamps, tag values, XML content, and mkvpropedit results

## NMAAHC Standard Tag Set

Tags in **bold** are mandatory. `_ORIGINAL_FPS` is required if motion picture film was the source material for the digital file.

| Tag | Description |
|-----|-------------|
| **`COLLECTION`** | The name of the collection that the content or object is from. *Ex: Johnson Publishing Company Archive / Pearl Bowser Collection* |
| **`TITLE`** | Leave blank. Or, if already in ASpace or TMS, the same as there. *Ex: Ebony/Jet Showcase #1001 / A Pinch of Soul* |
| **`CATALOG_NUMBER`** | Often the file name. *Ex: JPC_AV_12345 / 2012.79.2.54.1a* |
| **`DESCRIPTION`** | Please be brief! Ok to leave blank! *Ex: In the 1980s a young journalist moves to New York City / Broadcast of the 1978 American Black Achievement Awards* |
| **`DATE_DIGITIZED`** | yyyy-mm-dd. *Ex: 2024-10-10* |
| **`ENCODER_SETTINGS`** | Source VTR: model name, serial number, video signal type (Composite, SDI, etc.), audio signal type (analog balanced, analog unbalanced, embedded, etc.) ; TBC/Framesync: model name, serial number, video signal type, audio signal type ; ADC: model name, serial number, video signal type, audio signal type (if audio is embedded at this point simply say "embedded") ; Capture Device: model name, serial number, data connection type (Thunderbolt/PCIe/SATA/etc) ; Computer: model name, serial number, computer os version, capture software (including version), encoding software (ffmpeg version not required) |
| **`ENCODED_BY`** | Entity, name, country. *Ex: Smithsonian NMAAHC, James Smithson, US* (use ISO 3166-1 alpha-2 codes) |
| **`ORIGINAL_MEDIA_TYPE`** | Format (from PB Core), manufacturer, model. *Ex: U-matic, Sony, KSP-30* (don't guess or estimate the manufacturer or model — the format is plenty sufficient if the others are ambiguous) |
| **`TERMS_OF_USE`** | Not definitive, nor authoritative. *Ex: Some or all of this video may be subject to copyright or other intellectual property rights. Proper usage is the responsibility of the user.* |
| **`DATE_TAGGED`** | yyyy-mm-dd, this is the date the mkv tags were embedded or updated. *Ex: 2024-10-18* |
| **`_TAGGED_BY`** | Entity, name, country. *Ex: Smithsonian NMAAHC, James Smithson, US* (use ISO 3166-1 alpha-2 codes) |
| **`_PRE_TRANSFER_NOTES`** | Free text for capturing anything concerning the inspection and any physical conservation or preparation of the tape. *Ex: tape was baked for 20 hrs at 130°F, sticky leader was removed, treated for mold.* |
| **`_TRANSFER_NOTES`** | Free text for capturing any technical notes about the transfer of the tape. *Ex: drop out throughout picture, loss of tracking at tc 123, audio in left channel only.* |
| `_ORIGINAL_FPS` | For motion picture film, the frames per second at which the film was meant to be projected. Most common: 16, 18, 24. *Ex: 24* |

### Example: JPCA Record

| Tag | Value |
|-----|-------|
| `COLLECTION` | Johnson Publishing Company Archive |
| `TITLE` | *(leave blank)* |
| `CATALOG_NUMBER` | JPC_AV_12345 |
| `DESCRIPTION` | Robert Townsend interviewed by Darryl Dennard. Includes Robert conducting "man on street" outside the John Hancock Center. David Brenner makes an unplanned appearance. |
| `DATE_DIGITIZED` | 2024-10-10 |
| `ENCODER_SETTINGS` | Source VTR: Sony BVH3100, SN 10525, composite, analog balanced ; TBC/Framesync: Sony BVH3100, SN 10525, composite, analog balanced ; ADC: Leitch DPS575 with flash firmware h2.16, SN 15230, SDI, embedded ; Capture Device: Blackmagic Design UltraStudio 4K Extreme, SN B022159, Thunderbolt ; Computer: 2023 Mac Mini, Apple M2 Pro chip, SN H9HDW53JMV, OS 14.5, vrecord v2023-08-07, ffmpeg |
| `ENCODED_BY` | Smithsonian NMAAHC, David Sohl, US |
| `ORIGINAL_MEDIA_TYPE` | 1-inch type C, Sony, V1-K |
| `TERMS_OF_USE` | Some or all of this video may be subject to copyright or other intellectual property rights. Proper usage is the responsibility of the user. |
| `DATE_TAGGED` | 2024-10-18 |
| `_TAGGED_BY` | Smithsonian NMAAHC, Emily Nabasny, US |
| `_PRE_TRANSFER_NOTES` | Beginning of tape was trimmed to remove gluey residue. Flange is slightly bent. |
| `_TRANSFER_NOTES` | Signal errors occur during color bars. ProcAmp set to bars after 00:00:11. Some dropouts between 00:08:00 and 00:12:00. These are replicable during tape playback after cleaning heads, and are most likely permanent part of tape. |

## Installation

### Runtime dependencies (CLI)

| Tool | Purpose |
|------|---------|
| `mkvtoolnix` | `mkvpropedit`, `mkvextract` |
| `xmlstarlet` | XML processing |
| `csvprintf` | CSV-to-XML conversion (also provides `xml2csv` for XML-to-CSV) |
| `cowsay` | Terminal output |

### Build dependencies (GUI)

| Tool | Purpose |
|------|---------|
| `cmake` | Build system (3.16+) |
| `qt` | Qt 6 (Core, Widgets, Xml) |

### macOS

```bash
# CLI dependencies
brew install mkvtoolnix xmlstarlet csvprintf cowsay

# GUI build dependencies
brew install cmake qt
```

> **Note:** mkvnote requires **bash 4.0+** for associative arrays. macOS ships with bash 3.2. Install a newer bash via Homebrew (`brew install bash`) and ensure it appears in your `PATH` before the system version.

### Linux

Install the dependencies via your distribution's package manager (e.g., `apt`, `dnf`, `pacman`).

### Building the GUI

From the repository root:

```bash
./build
```

The `build` script checks for cmake and Qt 6, then runs the cmake configure and build steps. Run `./build clean` to remove the build directory first. Equivalently, you can run cmake directly:

```bash
cmake -B _build
cmake --build _build
```

Either way produces `_build/mkvnote-gui`. The `mkvnote` script looks for `mkvnote-gui` in its own directory first, then in `_build/`, then on your `PATH`.

To install both the CLI script and the GUI system-wide:

```bash
sudo cmake --install _build
```

### Releases

Source archives are available from the [releases page](https://github.com/NMAAHC/mkvnote/releases). The latest release is [v1.0.0](https://github.com/NMAAHC/mkvnote/releases/tag/v1.0.0).

## Usage

### GUI mode (single file)

```bash
mkvnote video.mkv
```

Opens a Qt window with all global tags organized into sections (Technical and Hashes, NMAAHC Tags, Empty NMAAHC Tags, Extra Existing Tags). Edit values and click **Tag-On!** to write, **Revert** to restore the loaded values, or **Cancel** to discard.

`mkvnote-gui` can also be launched directly, with or without a file argument — without one, use **Open** to choose an MKV file.

### Batch tagging from CSV

```bash
mkvnote metadata.csv
```

The CSV's first column must be `filename` containing paths to MKV files. All other columns are treated as tag names. Non-empty values are written into the corresponding files.

### Export tags to CSV

```bash
mkvnote -c file1.mkv file2.mkv file3.mkv > tags.csv
```

Produces a CSV with a `filename` column and one column per unique tag found across all input files.

### Inspect tags

```bash
# Formatted table
mkvnote -i file1.mkv file2.mkv

# Literal output with TAG=(value) for debugging
mkvnote -ii file1.mkv file2.mkv
```

The `-ii` flag makes value boundaries visible, useful for spotting trailing whitespace, empty strings, or line breaks that are hard to see in the formatted table.

### Set or remove a single tag

```bash
# Set a tag (overwrites existing value by default)
mkvnote -s KEY=VALUE file1.mkv file2.mkv

# Set a tag, but fail if the key already has a value
mkvnote -s KEY=VALUE -n file1.mkv

# Remove a tag (no effect if the key doesn't exist)
mkvnote -r KEY file1.mkv file2.mkv
```

### Delete global tags

```bash
mkvnote -d file1.mkv file2.mkv
```

Removes all global tags except `ENCODER`. Prompts for confirmation. This is destructive and irreversible.

### Options

| Flag | Description |
|------|-------------|
| `-h`, `--help` | Show detailed help |
| `-v`, `--version` | Show version |
| `-c`, `--csv` | Export tags from MKV files to CSV (stdout) |
| `-i`, `--info` | Print tags in a formatted table |
| `-ii` | Print tags as literal `TAG=(value)` for debugging |
| `-d`, `--drop` | Delete all global tags (except ENCODER) |
| `-s`, `--set` | Set a global tag (`-s KEY=VALUE`) |
| `-n`, `--no-overwrite` | With `-s`, fail if the key already has a value |
| `-r`, `--remove` | Remove a global tag by key |

## Log Files

When tags are written (GUI or CSV mode), mkvnote creates a log file in the same directory as the MKV file, named after it with the `.mkv` extension replaced by `_mkvnote_tags.log` (e.g., `video.mkv` → `video_mkvnote_tags.log`). The log includes:

- Session timestamp and version
- Each tag name and value being set
- Full XML content passed to mkvpropedit
- mkvpropedit exit status and output

## License

[GPL-3.0](LICENSE)

## Links

- [NMAAHC on GitHub](https://github.com/NMAAHC/nmaahcmm)
- [Matroska Tagging Specification](https://www.matroska.org/technical/tagging.html)
