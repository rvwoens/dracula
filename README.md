# Dracula Avontuur

*(c) 1980–2000 R. van Woensel · "(c) 1982 Incore Automatisering / R.van Woensel produkties" · versie 3.22 · freeware*

Homepage (uit de uitgave van 2001): <http://www.cosninix.com/dracula>

Dracula Avontuur is een Nederlandstalig tekstadventure. Het is in 1980 geschreven voor de Commodore PET 2001
(de voorloper van de VIC-20 en de C64) en was waarschijnlijk het allereerste Nederlandstalige adventurespel. Die eerste versie
paste in 8 KB, met heel beknopte teksten. Daarna is het spel naar CP/M overgezet en zijn de teksten uitgebreid. De MS-DOS-versie
is een hercompilatie daarvan met Microsoft BASCOM. In 1985 heeft CYCO Automatisering het spel als nieuwjaarswens gratis verspreid,
in een oplage van zo'n 200 stuks, en daarna werd het in Nederland enorm populair. In 2001, twintig jaar na het begin, is het als freeware
opnieuw uitgebracht.

## Het spel

Je begint in je eigen huis, met een briefje. In het dorp staat herberg "de zwarte hand", in de verte ligt het kasteel van
Lord Dracula. Aan jou de taak om Dracula onschadelijk te maken en levend terug te komen.

Je speelt door Nederlandse commando's te typen, een werkwoord met een zelfstandig naamwoord:

```
->lees brief
->pak lamp
->ga deur
->n                  (ook: z, o, w, h(oog), l(aag), e(ruit))
->pak alles.o.i      (meerdere commando's achter elkaar, gescheiden door een punt)
```

Handig: `kijk`, `i` (wat draag je), `help`, `bewaar spel`, `laad spel` (in `DRACULA.SAV`) en `quit`.

Toetsen op het titelscherm:

| Toets | Wat |
|---|---|
| willekeurige toets | Normaal spel. |
| `!` | Autoplay: speelt de oplossing uit het bestand `PLAYER` af. Druk na elk commando op een toets; `S` stopt de autoplay. **Spoiler.** |
| `T` | Testermodus: met het commando `/` schrijf je opmerkingen naar het bestand `TESTER`. |

## Wat zit er in deze map

- **`DRACULA.BAS`**: de broncode in GW-BASIC. Let op: dit is **niet** de originele broncode. Die is verloren gegaan; deze bron
  is gereconstrueerd uit `DRACULA.EXE`. Variabelenamen, regelnummers en commentaar zijn bij de reconstructie bedacht.
- **`dracula.c`**: een vertaling van `DRACULA.BAS` naar C, die hetzelfde spel speelt.
- **`dracula-322/`**: de originele uitgave van 2001 voor MS-DOS (`DRACULA.EXE`, `BASRUN.EXE`, `LEESMIJ.TXT`, `LICENSE.TXT`).
- **`DRACULA.TXT`**: alle teksten, kamers en voorwerpen. Dit is een **binair** bestand, ondanks de naam: niet openen of opslaan
  met een teksteditor. Alle versies hebben het nodig.

## Spelen

### Met PC-BASIC (de BASIC-bron)

[PC-BASIC](https://pc-basic.org) ([GitHub](https://github.com/robhagemans/pcbasic)) is een gratis emulator van GW-BASIC,
geschreven in Python. Hij draait op macOS, Linux en Windows.

**Installeren** (nodig: Python 3; getest met Python 3.13 en PC-BASIC 2.0.8):

```sh
python3 -m venv ~/.venvs/pcbasic
~/.venvs/pcbasic/bin/pip install pcbasic
~/.venvs/pcbasic/bin/pcbasic --version      # controle: "PC-BASIC 2.0.8"
```

`pip` installeert ook `pysdl2-dll` (voor het grafische venster) en `pyserial`. De startscripts verwachten PC-BASIC in
precies deze venv, `~/.venvs/pcbasic`. Wil je hem ergens anders, pas dan het pad in `run` en `run-nowindow` aan.

**Starten:**

```sh
./run              # in een PC-BASIC-venster
./run-nowindow     # in de terminal zelf, zonder venster
```

De scripts zijn zsh-scripts (standaard op macOS) en werken vanuit elke map. Ze starten PC-BASIC via `pcbasic_fix.py`, dat eerst
twee fouten in PC-BASIC 2.0.8 repareert: een geheugenlek dat het spel kan laten crashen, en het niet-scrollen van de
terminalversie. Start PC-BASIC dus altijd via dat script. Op een systeem zonder zsh:

```sh
~/.venvs/pcbasic/bin/python pcbasic_fix.py --quit DRACULA.BAS
```

Bekende beperking onder PC-BASIC: reïncarneren na de dood werkt niet (het programma stopt met "File already open").
Bewaar dus af en toe je spel, of speel de C-versie.

### C-versie

Nodig: een C-compiler (`cc`/`gcc`/`clang`) en `make`; werkt op macOS en Linux.

```sh
./run-c            # bouwt zo nodig en start het spel
# of:
make && ./dracula
```

Start het in deze map, want het spel zoekt `DRACULA.TXT` (en `PLAYER` voor de autoplay) in de huidige map.

### Origineel onder DOSBox

De originele `DRACULA.EXE` uit `dracula-322/` draait in principe in [DOSBox](https://www.dosbox.com). Zet `DRACULA.TXT` in dezelfde
map; de EXE zoekt `BASRUN.EXE` op schijf `A:`, dus mount de map als `A:`. Dit is niet getest.

## Licentie

Freeware, onder een Artistic-achtige licentie: zie [`dracula-322/LICENSE.TXT`](dracula-322/LICENSE.TXT).
