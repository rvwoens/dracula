# Dracula Avontuur

*Nederlandse versie: [LEESME.md](LEESME.md)*

*(c) 1980–2000 R. van Woensel · "(c) 1982 Incore Automatisering / R.van Woensel produkties" · version 3.22 · freeware*

Homepage (from the 2001 release): <http://www.cosninix.com/dracula>

Dracula Avontuur ("Dracula Adventure") is a Dutch-language text adventure. It was written in 1980 for the Commodore PET 2001
(the predecessor of the VIC-20 and the C64) and was probably the very first adventure game in Dutch. That first version
fit in 8 KB, with very terse texts. The game was then ported to CP/M and the texts were expanded. The MS-DOS version is a
recompilation of that with Microsoft BASCOM. In 1985 CYCO Automatisering gave the game away as a New Year's gift, about 200 copies,
and it became hugely popular in the Netherlands. In 2001, twenty years after it started, it was re-released as freeware.

## The game

**The game itself is in Dutch:** all texts are Dutch and you type your commands in Dutch.

You start in your own house, with a note. The village has an inn called "de zwarte hand" (the black hand), and in the distance
stands the castle of Lord Dracula. Your task: put an end to Dracula and get back alive.

You play by typing commands, a verb with a noun:

```
->lees brief         (read note)
->pak lamp           (take lamp)
->ga deur            (go door)
->n                  (also: z, o, w = south, east, west; h(oog) = up, l(aag) = down, e(ruit) = out)
->pak alles.o.i      (several commands in a row, separated by a full stop)
```

Useful: `kijk` (look), `i` (what you carry), `help`, `bewaar spel` / `laad spel` (save / load game, in `DRACULA.SAV`) and `quit`.

Keys on the title screen:

| Key | What |
|---|---|
| any key | Normal game. |
| `!` | Autoplay: plays the solution from the file `PLAYER`. Press a key after each command; `S` stops the autoplay. **Spoiler.** |
| `T` | Tester mode: the command `/` writes comments to the file `TESTER`. |

## What's in this folder

- **`DRACULA.BAS`**: the source code in GW-BASIC. Note: this is **not** the original source code. That was lost; this source
  was reconstructed from `DRACULA.EXE`. Variable names, line numbers and comments were made up during the reconstruction.
- **`dracula.c`**: a translation of `DRACULA.BAS` into C that plays the same game.
- **`dracula-322/`**: the original 2001 release for MS-DOS (`DRACULA.EXE`, `BASRUN.EXE`, `LEESMIJ.TXT`, `LICENSE.TXT`).
- **`DRACULA.TXT`**: all texts, rooms and objects. Despite its name this is a **binary** file: do not open or save it
  in a text editor. Every version needs it.

## Playing

### With PC-BASIC (the BASIC source)

[PC-BASIC](https://pc-basic.org) ([GitHub](https://github.com/robhagemans/pcbasic)) is a free GW-BASIC emulator
written in Python. It runs on macOS, Linux and Windows.

**Installing** (requires Python 3; tested with Python 3.13 and PC-BASIC 2.0.8):

```sh
python3 -m venv ~/.venvs/pcbasic
~/.venvs/pcbasic/bin/pip install pcbasic
~/.venvs/pcbasic/bin/pcbasic --version      # check: "PC-BASIC 2.0.8"
```

`pip` also installs `pysdl2-dll` (for the graphical window) and `pyserial`. The start scripts expect PC-BASIC in exactly
this venv, `~/.venvs/pcbasic`. If you want it somewhere else, change the path in `run` and `run-nowindow`.

**Starting:**

```sh
./run              # in a PC-BASIC window
./run-nowindow     # in the terminal itself, without a window
```

The scripts are zsh scripts (the default shell on macOS) and work from any folder. They start PC-BASIC through `pcbasic_fix.py`,
which first fixes two bugs in PC-BASIC 2.0.8: a memory leak that can make the game crash, and the terminal interface not scrolling.
So always start PC-BASIC through that script. On a system without zsh:

```sh
~/.venvs/pcbasic/bin/python pcbasic_fix.py --quit DRACULA.BAS
```

Known limitation under PC-BASIC: reincarnation after dying does not work (the program stops with "File already open").
So save your game now and then, or play the C version.

### C version

Requires a C compiler (`cc`/`gcc`/`clang`) and `make`; works on macOS and Linux.

```sh
./run-c            # builds if needed and starts the game
# or:
make && ./dracula
```

Start it in this folder, because the game looks for `DRACULA.TXT` (and `PLAYER` for the autoplay) in the current folder.

### The original under DOSBox

The original `DRACULA.EXE` from `dracula-322/` should run in [DOSBox](https://www.dosbox.com). Put `DRACULA.TXT` in the same
folder; the EXE looks for `BASRUN.EXE` on drive `A:`, so mount the folder as `A:`. This has not been tested.

## License

Freeware, under an Artistic-style license: see [`dracula-322/LICENSE.TXT`](dracula-322/LICENSE.TXT).
