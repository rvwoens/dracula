# Dracula Avontuur

*English version: [README.md](README.md)*

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

## Kamers en voorwerpen

**Spoilers.** Klik op een kop om hem open te klappen.

<details>
<summary>Kamers (0–54)</summary>

De uitgangen staan er niet bij: sommige doorgangen gaan pas open als je iets gedaan hebt.

| # | Kamer |
|---|---|
| 0 | Je eigen huis |
| 1 | Slaapkamer |
| 2 | Kleine hal |
| 3 | Zolder |
| 4 | Vochtige kelder |
| 5 | Nauwe zandtunnel |
| 6 | Achter het huis |
| 7–10 | Donker woud |
| 11 | Dorpsstraat |
| 12 | Herberg "de zwarte hand", met de waard |
| 13 | Aan tafel in de herberg |
| 14 | Zolder van de herberg |
| 15 | Open plek in het bos |
| 16 | Heuveltop met een bron |
| 17 | Woud, eikenboom met een hut |
| 18 | Boomhut |
| 19 | Pad naar het kasteel |
| 20 | Kasteelingang |
| 21 | Kasteelhal (harnas) |
| 22 | Lange gang, deur "SLAAPKAMER 2" |
| 23 | Slaapkamer 1 |
| 24 | Slaapkamer 2 |
| 25 | Balkon |
| 26 | Smalle rand langs het kasteel |
| 27 | Eetkamer |
| 28 | Torendak |
| 29 | Kasteeltoren |
| 30 | Wenteltrap |
| 31 | Wenteltrap, deur "DIT IS SESAM" |
| 32 | Gewelf met een ijzeren hek |
| 33 | Spionageruimte |
| 34 | Kruiskamer (spin, vuur) |
| 35 | Schatkamer |
| 36 | Binnenplaats met kerkhof |
| 37 | Graftombe |
| 38 | In een doodskist |
| 39 | Gegraven gang |
| 40 | Lange gang, deur "SLAAPKAMER 1" |
| 41 | Smalle nis met een kijkgat |
| 42 | Einde van een nauwe, hoge gang |
| 43 | Gat in de grond |
| 44 | Onbegaanbare gang |
| 45–50, 53 | Doolhof ("...?...") |
| 51 | Nis met een bordje "TORENKAMER" |
| 52 | Ventilatieruimte |
| 54 | In het harnas |

</details>

<details>
<summary>Voorwerpen (1–41)</summary>

"Woorden" zijn de zelfstandige naamwoorden waarmee je een voorwerp aanspreekt (`pak lamp`, `pak lant`); alleen de eerste 4 letters tellen.
Een voorwerp zonder woorden kun je niet pakken: het is decor, of een stand van iets anders. Een aantal voorwerpen zijn standen van
hetzelfde ding: de fles (17, 18, 19, 34), de kisten (22–24 en 25–27), de doodskist (38, 39), de bijl (11, 29), het touw (2, 13) en de spin (35, 41).

| # | Woorden | Voorwerp | Waar / wanneer |
|---|---|---|---|
| 1 | LANT BRAN LAMP | kleine brandende lantaren | 1 slaapkamer |
| 2 | TOUW | stuk dik touw | 3 zolder |
| 3 | WIG | houten wig | 18 boomhut |
| 4 | KNOF STRE | streng knoflook | 12 herberg (te koop) |
| 5 | BOEK | dik gehavend boek | verstopt in de stoffige kist op 14 |
| 6 | KAPM MES | zwaar kapmes | 18 boomhut |
| 7 | LADD | lange ladder | 36 kerkhof |
| 8 | HOUT | stuk hout | door een boom om te hakken |
| 9 | TAK | gesneden tak | komt nooit in het spel |
| 10 | KRUI | houten kruis | gesneden uit het hout |
| 11 | BIJL | vlijmsscherpe bijl | 4 kelder |
| 12 | | Overbodig voorwerp | wordt niet gebruikt |
| 13 | | stuk dik touw, vastgebonden aan het balkon | als je het touw vastknoopt op 25 |
| 14 | SCHA KIST | schatkist, vol met gouden munten | 35 schatkamer |
| 15 | BROO SNEE | sneetje brood | 2 hal |
| 16 | MUNT DUIV | gouden duivelaarsmunt | 2 hal |
| 17 | FLES MELK | fles melk (zonder statiegeld) | 2 hal |
| 18 | FLES | lege fles | de fles na het drinken |
| 19 | FLES WATE | fles water | gevuld bij de bron |
| 20 | HALS | gouden halsband | 24 slaapkamer 2 |
| 21 | | zwaar eikenhouten bed | 1 slaapkamer |
| 22 | | zware gesloten kist | 29 kasteeltoren |
| 23 | | zware geopende kist met een gesloten luik onderin | na het openen van 22 |
| 24 | | zware geopende kist met een geopend luik onderin | na het openen van het luik |
| 25 | | zware kist met een dikke laag stof op de bodem | 14 zolder van de herberg |
| 26 | | zware kist met een boek onderin | na het wegblazen van het stof |
| 27 | | zware lege kist | na het pakken van het boek |
| 28 | | scherven | als er een fles kapotvalt |
| 29 | BIJL | botte bijl | de bijl na het hakken |
| 30 | SCHE | schep | 6 achter het huis |
| 31 | | ondiepe waterbron | 16 heuveltop |
| 32 | DOOS | kleine houten doos | 4 kelder |
| 33 | HAME | houten hamer | in de doos |
| 34 | FLES | fles gevuld met blauw,giftig water | gevuld bij de bron, nadat die vergiftigd is |
| 35 | | gevaarlijke kruisspin | 34 kruiskamer |
| 36 | BRIE | klein briefje | 0 je eigen huis |
| 37 | HARN IJZE GROO | groot ijzeren harnas | 21 kasteelhal |
| 38 | | zware doodskist met in gothische letters de tekst 'D R A C U L A' | 37 graftombe |
| 39 | KIST DOOD | zware geopende doodskist met op de zijkant de tekst 'DRACULA' | na het openen van de doodskist |
| 40 | | De vreemd gevormde deur is nu open | na het toverwoord |
| 41 | SPIN KRUI | dode kruisspin | na het gif |

</details>

## Licentie

Freeware, onder een Artistic-achtige licentie: zie [`dracula-322/LICENSE.TXT`](dracula-322/LICENSE.TXT).
