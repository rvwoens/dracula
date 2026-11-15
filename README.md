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

## Rooms and objects

**Spoilers.** Click a heading to expand it.

<details>
<summary>Rooms (0–54)</summary>

Exits are not listed: some passages only open after you do something.

| # | Room |
|---|---|
| 0 | Your own house |
| 1 | Bedroom |
| 2 | Small hall |
| 3 | Attic |
| 4 | Damp cellar |
| 5 | Narrow sand tunnel |
| 6 | Behind the house |
| 7–10 | Dark forest |
| 11 | Village street |
| 12 | Inn "de zwarte hand", with the innkeeper |
| 13 | At a table in the inn |
| 14 | Attic of the inn |
| 15 | Clearing in the forest |
| 16 | Hilltop with a spring |
| 17 | Forest, oak tree with a hut |
| 18 | Tree hut |
| 19 | Path to the castle |
| 20 | Castle entrance |
| 21 | Castle hall (suit of armour) |
| 22 | Long corridor, door "SLAAPKAMER 2" |
| 23 | Bedroom 1 |
| 24 | Bedroom 2 |
| 25 | Balcony |
| 26 | Narrow ledge along the castle |
| 27 | Dining room |
| 28 | Tower roof |
| 29 | Castle tower |
| 30 | Spiral staircase |
| 31 | Spiral staircase, door "DIT IS SESAM" |
| 32 | Vault with an iron gate |
| 33 | Spy room |
| 34 | Cross chamber (spider, fire) |
| 35 | Treasure chamber |
| 36 | Courtyard with graveyard |
| 37 | Burial vault |
| 38 | Inside a coffin |
| 39 | Dug tunnel |
| 40 | Long corridor, door "SLAAPKAMER 1" |
| 41 | Narrow niche with a peephole |
| 42 | End of a narrow, high corridor |
| 43 | Hole in the ground |
| 44 | Impassable corridor |
| 45–50, 53 | Maze ("...?...") |
| 51 | Niche with a sign "TORENKAMER" |
| 52 | Ventilation room |
| 54 | Inside the suit of armour |

</details>

<details>
<summary>Objects (1–41)</summary>

"Words" are the nouns you can use for an object (`pak lamp`, `pak lant`); only the first 4 letters count. An object without words
can't be picked up: it is scenery, or a state of something else. Several objects are states of one thing: the bottle (17, 18, 19, 34),
the chests (22–24 and 25–27), the coffin (38, 39), the axe (11, 29), the rope (2, 13) and the spider (35, 41).
The game texts are in Dutch; the English is a translation.

| # | Words | In the game | English | Where / when |
|---|---|---|---|---|
| 1 | LANT BRAN LAMP | kleine brandende lantaren | small burning lantern | 1 bedroom |
| 2 | TOUW | stuk dik touw | piece of thick rope | 3 attic |
| 3 | WIG | houten wig | wooden wedge | 18 tree hut |
| 4 | KNOF STRE | streng knoflook | string of garlic | 12 inn (for sale) |
| 5 | BOEK | dik gehavend boek | thick, battered book | hidden in the dusty chest in 14 |
| 6 | KAPM MES | zwaar kapmes | heavy machete | 18 tree hut |
| 7 | LADD | lange ladder | long ladder | 36 graveyard |
| 8 | HOUT | stuk hout | piece of wood | by chopping down a tree |
| 9 | TAK | gesneden tak | carved branch | never appears |
| 10 | KRUI | houten kruis | wooden cross | carved from the wood |
| 11 | BIJL | vlijmsscherpe bijl | razor-sharp axe | 4 cellar |
| 12 | | Overbodig voorwerp | redundant object | not used |
| 13 | | stuk dik touw, vastgebonden aan het balkon | rope tied to the balcony | after tying the rope in 25 |
| 14 | SCHA KIST | schatkist, vol met gouden munten | treasure chest full of gold coins | 35 treasure chamber |
| 15 | BROO SNEE | sneetje brood | slice of bread | 2 hall |
| 16 | MUNT DUIV | gouden duivelaarsmunt | golden devil's coin | 2 hall |
| 17 | FLES MELK | fles melk (zonder statiegeld) | bottle of milk (no deposit) | 2 hall |
| 18 | FLES | lege fles | empty bottle | the bottle after drinking |
| 19 | FLES WATE | fles water | bottle of water | filled at the spring |
| 20 | HALS | gouden halsband | golden collar | 24 bedroom 2 |
| 21 | | zwaar eikenhouten bed | heavy oak bed | 1 bedroom |
| 22 | | zware gesloten kist | heavy closed chest | 29 castle tower |
| 23 | | zware geopende kist met een gesloten luik onderin | opened chest with a closed hatch in the bottom | after opening 22 |
| 24 | | zware geopende kist met een geopend luik onderin | opened chest with an open hatch in the bottom | after opening the hatch |
| 25 | | zware kist met een dikke laag stof op de bodem | heavy chest with a thick layer of dust in it | 14 attic of the inn |
| 26 | | zware kist met een boek onderin | heavy chest with a book at the bottom | after blowing the dust away |
| 27 | | zware lege kist | heavy empty chest | after taking the book |
| 28 | | scherven | shards | when a bottle breaks |
| 29 | BIJL | botte bijl | blunt axe | the axe after chopping |
| 30 | SCHE | schep | shovel | 6 behind the house |
| 31 | | ondiepe waterbron | shallow spring | 16 hilltop |
| 32 | DOOS | kleine houten doos | small wooden box | 4 cellar |
| 33 | HAME | houten hamer | wooden hammer | inside the box |
| 34 | FLES | fles gevuld met blauw,giftig water | bottle of blue, poisonous water | filled at the spring after it has been poisoned |
| 35 | | gevaarlijke kruisspin | dangerous cross spider | 34 cross chamber |
| 36 | BRIE | klein briefje | small note | 0 your own house |
| 37 | HARN IJZE GROO | groot ijzeren harnas | large iron suit of armour | 21 castle hall |
| 38 | | zware doodskist met in gothische letters de tekst 'D R A C U L A' | heavy coffin with 'D R A C U L A' in Gothic letters | 37 burial vault |
| 39 | KIST DOOD | zware geopende doodskist met op de zijkant de tekst 'DRACULA' | heavy opened coffin with 'DRACULA' on the side | after opening the coffin |
| 40 | | De vreemd gevormde deur is nu open | the strangely shaped door is now open | after the magic word |
| 41 | SPIN KRUI | dode kruisspin | dead cross spider | after the poison |

</details>

## License

Freeware, under an Artistic-style license: see [`dracula-322/LICENSE.TXT`](dracula-322/LICENSE.TXT).
