# Dracula Avontuur (v3.22)

Nederlandstalig tekstadventure van R. van Woensel, "(c) 1982 Incore Automatisering / R.van Woensel produkties".
Volgens `dracula-322/LEESMIJ.TXT` ontstaan in 1980 op de Commodore PET 2001 (8 KB, beknopte teksten), daarna
overgezet naar CP/M met langere teksten, en de MS-DOS-versie is een platte hercompilatie daarvan (Microsoft
BASCOM 1.0 + BASRUN). In 1985 gratis verspreid door CYCO Automatisering als nieuwjaarswens (~200 stuks).
In 2001 opnieuw uitgebracht als freeware (map `dracula-322/`).

**De BASIC-broncode is niet origineel**: `DRACULA.BAS` is gereconstrueerd uit `DRACULA.EXE`. Variabelenamen,
regelnummers en alle REM-regels zijn bij de reconstructie bedacht. Variabelenamen dus niet als "historisch" opvatten.

## Bestanden

| Bestand | Wat |
|---|---|
| `DRACULA.BAS` | Werkversie van de gereconstrueerde bron (GW-BASIC, ASCII, CRLF, 1261 regels). Hier wijzigen. Afwijkend van de reconstructie: regel 10240 `PRINT CHR$(7)` vervangen door 10245 `PRINT "** BIEP ** "`, en regel 10850 `WIDTH 255` weggezet als `REM`. |
| `DRACULA.TXT` | **Binair** databestand met alle teksten, kamers, uitgangen en objecten (zie onder). Nooit met een teksteditor openen of opslaan. |
| `PLAYER` | Complete oplossing als commandoreeks, voor de autoplay-modus. Gebaseerd op `dracula-322/PLAYER`, plus `vampirania` (regel 67); regel 40 is op 14-11 van `k.k.k.k` naar `k.k.k.k.k.k.k.k` gegaan (zie Bekende problemen, Autoplay). |
| `run` | Start het spel in een pcbasic-venster (via `pcbasic_fix.py`, met `--quit`). |
| `run-nowindow` | Idem, maar in de terminal zelf, zonder venster (`--interface=cli`). |
| `README.md` / `LEESME.md` | Voor bezoekers, in het Engels / Nederlands (met dezelfde inhoud; ze verwijzen bovenaan naar elkaar): wat het spel is, homepage, PC-BASIC installeren, spelen (pcbasic, C, DOSBox), en inklapbare spoilertabellen van kamers en voorwerpen. Beide bijwerken als scripts, paden, kamers of objecten veranderen. |
| `dracula.c` | Vertaling van `DRACULA.BAS` naar C (C99 + POSIX), zie "C-versie". `Makefile` bouwt `dracula`; `run-c` bouwt zo nodig en start het in deze map. |
| `pcbasic_fix.py` | Opstartscript dat twee pcbasic-bugs repareert (de temp-string-lek en het niet-scrollen van de cli-interface, zie Bekende problemen) en daarna pcbasic start met dezelfde argumenten. |
| `dracula-322/` | Originele uitgave van 2001: `DRACULA.EXE`, `BASRUN.EXE` (1983), `LEESMIJ.TXT`, `LICENSE.TXT` (Artistic-achtige freeware-licentie), `SPEEL.BAT`, lege `DRACULA.BUG`. `DRACULA.TXT` ontbreekt daar; die staat in de hoofdmap. Niet wijzigen. |

## Draaien

- `./run` → `python pcbasic_fix.py --quit DRACULA.BAS` in de venv `~/.venvs/pcbasic` (PC-BASIC 2.0.8, Python 3.13, grafisch venster).
  `./run-nowindow` doet hetzelfde met `--interface=cli`, in de terminal. Beide gaan eerst naar hun eigen map, dus ze zijn overal vandaan te starten.
  `--quit` sluit pcbasic als het programma stopt; anders blijft het op de `Ok`-prompt staan.
  Start pcbasic altijd via `pcbasic_fix.py`, niet direct.
- Toetsen op het titelscherm:
  - willekeurige toets: normaal spel; `RANDOMIZE` met een teller uit de wachtlus, dus een willekeurig toverwoord.
  - `!`: **autoplay**. Leest commando's uit `PLAYER`; na elk commando een toets, `S` stopt de autoplay. Geen `RANDOMIZE`, dus deterministisch.
  - `T`: testermodus. Commando `/` schrijft opmerkingen naar bestand `TESTER` (en `LPRINT`).
- Headless (zonder venster), bijvoorbeeld om wijzigingen te testen. Werkt; draai in een kopie in de scratchpad, want het spel kan `DRACULA.SAV`/`TESTER` schrijven:
  ```sh
  (sleep 2; printf 'x'; sleep 1; printf 'lees brief.z.i\r\n'; sleep 1; printf 'quit\r\n'; sleep 1; printf 'n') \
    | ~/.venvs/pcbasic/bin/python pcbasic_fix.py --run=DRACULA.BAS --interface=none 2>/dev/null | tr -d '\r'
  ```
  Eerst ~2 s wachten: regel 12060 doet bij de start al een `INKEY$`, die anders de eerste toets opeet.
- Origineel: `/Applications/dosbox.app` is aanwezig. De EXE zoekt `A:BASRUN.EXE`, dus de map als `A:` mounten en `DRACULA.TXT` erbij zetten (niet getest).

## Programmastructuur (`DRACULA.BAS`)

Klassieke GOTO-spaghetti met één grote IF-keten als werkwoord-dispatcher.

- **Start** 100–350: machinedetectie (12010), titel (10810), `LN%` = toverwoord 1..6, objecten laden uit `DRACULA.TXT`, kamer tonen.
- **Hoofdlus** 360–730: `GOSUB 3910` (Dracula/spin/winst, elke beurt), invoer lezen (of uit `PLAYER`), naar hoofdletters,
  splitsen op `.` (meerdere commando's per regel; rest blijft in `IN$` als wachtrij), werkwoord/naamwoord splitsen op eerste spatie.
  - `V$` = eerste **5** letters werkwoord, `N$` = eerste **4** letters naamwoord. `VV$`/`NN$` = volledig, `NL$` = naamwoord in kleine letters.
  - Losse `N Z O W H L E` worden `GA <richting>`.
- **Werkwoorden** 740–1180 → handler-regel. Belangrijkste: GA/BETRE/KRUIP/LOOP/KLIM/VOLG 1260 · KIJK/BEKIJ/ONDER 6150 · PAK/NEEM/GRIJP/RAAP 2370 ·
  LEG/ZET/DROP 2840 · GOOI 9660 · TOON 10510 · BESCHIJN 10120 · DOOD 7300 · SLA/SCHOP 7170 · OPEN 5000 · SLUIT 7490 · LEES 5680 · EET/DRINK 3400 ·
  GRAAF 3290 · VUL 5850 · SNIJ 5960 · HAK 3150 · KOOP 7020 · GEEF 7860 · VRAAG 4690 · LUISTER 7080 · KNOOP/HANG 5550 · SLAAP 6820 ·
  BEWAAR/LAAD SPEL 8170/8860 · QUIT 9600 · I/INVENTARIS 2740 · BUG/FOUT 7930 · HELP 5640. Onbekend: random "begrijp niet" (berichten 7–10).
  - Debug-cheat: `$RVW` vraagt een kamernummer en teleporteert (750).
  - Toverwoord: `VV$ = LAND$` in kamer 31 → 10560.
- **Conventies in de code**:
  - `M% = n : GOTO 170` = bericht n tonen en terug naar de hoofdlus. `GOSUB 11920` = bericht n tonen zonder sprong.
  - `GOTO 340` = scherm wissen + kamer opnieuw beschrijven. `GOTO 7710` = dood, met reïncarnatie (`CLEAR`, `GOTO 100`).
  - `OL%(i)` = locatie van object i: kamernummer, `-1` = bij speler, `99` = weg/bestaat (nog) niet. Max. 8 objecten dragen (`NC > 7`).
  - Spelvlaggen zijn losse integers (`LAD0%`, `LAD30%`, `HEK%`, `STEEN%`, `RAAM%`, `LUIK%`, `BED%`, `SLAAP%`, `TRN%`, `KIST%`, `SESAM%`, `SPIN%`, ...).
    `DAG%` 1=dag, 0=nacht. `DR%` = kamer van Dracula (255 = afwezig, 257 = vernietigd), `DA%` = aanvalsteller, `DXD%` = 1 als hij verjaagd is.
- **Subroutines**: 3240 tel gedragen (`NC`) · 3590 kamer-gebeurtenissen/sfeertekst · 3910 per-beurt-logica · 4580 objectnaam (`OB$`) ·
  4610/4650 PAK/DROP ALLES (zet commando's in `IN$`) · 6700 object zoeken op `N$` (zet `WK%` bekend, `WH%` hier, `WC%` bij speler, `WI%` index) ·
  10210 wachten (piepjes + `TIME$`-busy-wait) · 10360 Dracula vergaat, toont toverwoord · 10430 schrik: random iets laten vallen ·
  10610 kamer beschrijven · 10730 toverwoord decoderen · 11470/11440 bestandsnummer claimen/vrijgeven · 11530–11910 `DRACULA.TXT` lezen ·
  11940/11970 scherm wissen/cursor (alleen IBM/Zenith).
- **Toverwoord**: DATA regel 10–11, elke letter +1 versleuteld (Caesar). 1 VAMPIRANIA, 2 FLAUWEKULA, 3 DRAKELSTEINIA, 4 MICROSOFTENSTEIN, 5 INCORONIUM, 6 TRANSSYLVANIUM.
- **CALL ISIBM/ISZENITH/CURSOR/CLRSCR/FINDFILE**: assemblerroutines die in de EXE meegelinkt waren. pcbasic negeert `CALL`
  ("CALL/CALLS statement not implemented"), dus `IBM% = ZEN% = 0`: eenvoudig titelscherm, geen schermwissen, `MACH$ = "msBASCOM"`.

## C-versie (`dracula.c`)

- `make` (of `./run-c`), daarna `./dracula` in de map met `DRACULA.TXT` (en `PLAYER` voor `!`). Zelfde toetsen als de BASIC-versie.
- Gestructureerd: elk werkwoord is een functie die een `enum vervolg` teruggeeft (`NA_MELDING` = `GOTO 170`, `NA_KIJK` = 340,
  `NA_BESCHRIJF` = 350, `NA_BEURT` = 360, `NA_INVOER` = 390, `NA_DOOD` = 7710, `NA_ONTLEED` = 560); de hoofdlus `speel()` voert die uit.
  Elke functie noemt haar BASIC-regelnummer. Alle BASIC-variabelen staan in `struct g` (`R%` → `g.r`, `OL%(i)` → `g.ol[i]`); `CLEAR` = `memset`.
  `meld(n)` = `M% = n : GOTO 170`, `melding(n)` = `M% = n : GOSUB 11920`, `naar(k)` = `R% = k : GOTO 340`.
- RND/RANDOMIZE zijn een exacte kopie van de generator van GW-BASIC/pcbasic (LCG 214013/2531011 mod 2^24, start 5228370). BASIC rekent alle
  termen van `AND`/`OR` uit, dus waar `RND` in zo'n voorwaarde staat (3630, 3740, 10440) wordt hij in C vooraf getrokken. Anders loopt de reeks uit de pas.
- Bewust anders dan `DRACULA.BAS` onder pcbasic (staat ook bovenaan `dracula.c`): `CLEAR` sluit bestanden (reïncarnatie werkt), `FINDFILE`
  echt geïmplementeerd (melding 189 zonder save), geen afbreken op 80 kolommen, `C:DRACULA.BUG` → `DRACULA.BUG` in de werkmap (aanvullen),
  `TESTER` aanvullen zonder `LPRINT`, `/` zonder testermodus crasht niet, wachtlussen via de klok, einde van stdin = einde programma.
- Zonder terminal (stdin is een pipe) echoot `LINE INPUT` zelf de regel, zodat de uitvoer op die van `pcbasic --interface=none` lijkt.
  De `INKEY$` bij de start (12060) eet net als in BASIC een toets die al klaarstaat: bij piped tests eerst ~1,5 s wachten.
- **Getest (14-11-2026)** tegen pcbasic, uitvoer genormaliseerd (`tr -d '\377'`, `tr '\0' ' '`, CR en spaties aan regeleinde weg):
  autoplay tot de winst (met `k.k.k.k` op regel 40) is regel voor regel gelijk. Verschillen: pcbasic breekt de doodskistregel en de `** BIEP **`-rij
  op 80 kolommen af, en zet na `END` nog `Ok` neer. Toetsenbordmodus (`!` en dan `S`, dus geen RANDOMIZE, met 71 commando's waaronder `$RVW`,
  bewaren en laden) is gelijk en `DRACULA.SAV` byte-voor-byte gelijk. Verder alleen in C getest: laden zonder save, `T` + `/`, `FOUT`,
  dood + `N`/`J` (reïncarnatie), via een pty (ruwe toetsen, terminal hersteld) en een ASan/UBSan-build zonder meldingen.

## `DRACULA.TXT` (binair, random access)

Records van 80 bytes, 1-based (`OPEN "R",#f,"DRACULA.TXT",80`), int16 little-endian (`CVI`). 1289 records + 48 nulbytes
(aangevuld tot een veelvoud van 128 bytes, een overblijfsel van CP/M). Alleen ASCII.

| Record | Inhoud |
|---|---|
| 1 | Header: `RB`=101, `MB`=201, `IB`=701, `H4`=801 (begin overloopgebied), `H5`=1289 (aantal records) |
| `kamer + 2` | Uitgangen van kamer: 7 × int16 N,Z,O,W,H(oog),L(aag),E(ruit); 255 = geen uitgang |
| `RB + kamer + 1` | Kamerbeschrijving |
| `MB + n` | Bericht n (1–284; 178/179 leeg) |
| `IB + i` | Object i (1–41): tekst, int16 @76 = lengte, int16 @78 = startlocatie |

Tekstrecords: 78 tekens + int16 @78 = volgend record (vervolgketen in 802–1288), 0 = einde. De offsets verschillen per soort (`+2`, `+RB+1`, `+MB`);
zo werkt het origineel, niet "rechttrekken".

**Objectnamen** (`OBJ$`): `/LANT/BRAN/LAMPkleine brandende lantaren`. Maximaal 3 sleutels van 4 tekens (met spatie aangevuld, bv. `/MES `) op posities 2, 7 en 12;
het naamwoord wordt als prefix vergeleken. Zonder `/` = decor (niet te pakken; PAK ALLES neemt alleen `/`-objecten). Prefix `@` = meervoud
("Er zijn ... hier"), `~` = letterlijke zin.

## Wereld (kamers 0–54)

0 eigen huis · 1 slaapkamer · 2 hal · 3 zolder · 4 kelder · 5 zandtunnel · 6 achter het huis · 7–10 woud · 11 dorpsstraat ·
12 herberg "de zwarte hand" (waard) · 13 aan tafel · 14 zolder herberg · 15 open plek (bron in het oosten) · 16 heuveltop met bron (`VUL FLES`, munt) · 17 boom met hut · 18 boomhut ·
19 pad · 20 kasteelingang · 21 kasteelhal (harnas) · 22 gang (SLAAPKAMER 2) · 23 slaapkamer 1 · 24 slaapkamer 2 (Dracula) · 25 balkon ·
26 smalle rand · 27 eetkamer · 28 torendak · 29 toren · 30–31 wenteltrap (31: deur "DIT IS SESAM") · 32 gewelf met ijzeren hek ·
33 spionageruimte · 34 kruiskamer (spin, vuur) · 35 schatkamer · 36 kerkhof · 37 graftombe · 38 in de doodskist · 39 gegraven gang ·
40 gang (SLAAPKAMER 1) · 41–44, 51, 52 geheime gangen/nissen · 45–50, 53 doolhof ("...?...") · 54 in het harnas.

## Objecten (1–41)

Uit `DRACULA.TXT` (record `IB + i`); "Start" = beginlocatie (`OL%`), 99 = (nog) niet in het spel. Sleutels zijn de 4-letterwoorden waarmee
je het object aanspreekt; zonder sleutel = decor of toestand. Meerdere objecten zijn standen van hetzelfde ding (fles 17/18/19/34,
torenkist 22/23/24, herbergkist 25/26/27, doodskist 38/39, bijl 11/29, touw 2/13, spin 35/41): de code zet dan de ene op 99 en de andere op de plek.

| # | Sleutels | Tekst | Start | Hoe het verandert (regel) |
|---|---|---|---|---|
| 1 | LANT BRAN LAMP | kleine brandende lantaren | 1 slaapkamer | BESCHIJN (10120), o.a. Dracula verjagen |
| 2 | TOUW | stuk dik touw | 3 zolder | KNOOP TOUW in 25 → 13 (5580) |
| 3 | WIG | houten wig | 18 boomhut | SLA in 24 met hamer 33, Dracula daar → weg (7260), Dracula vergaat |
| 4 | KNOF STRE | streng knoflook | 12 herberg | KOOP KNOF (7020); pakken zonder betalen telt `HB%` op. GOOI KNOF → `DA%=0` |
| 5 | BOEK | dik gehavend boek | 99 | PAK BOEK uit kist 26 → bij speler (2440); LEES BOEK |
| 6 | KAPM MES | zwaar kapmes | 18 boomhut | nodig voor SNIJ (5960) |
| 7 | LADD | lange ladder | 36 kerkhof | LEG in 0 of 30 → 99 + `LAD0%`/`LAD30%` (3000–3090); PAK daar → terug (2510/2560) |
| 8 | HOUT | stuk hout | 99 | HAK BOOM met bijl 11 → in de kamer (3190) |
| 9 | TAK | gesneden tak | 99 | wordt nooit gezet; SNIJ TAK (5970) kan dus niet |
| 10 | KRUI | houten kruis | 99 | SNIJ HOUT → bij speler (6070); TOON KRUIS in 24 |
| 11 | BIJL | vlijmsscherpe bijl | 4 kelder | HAK BOOM → weg, speler krijgt 29 (3200) |
| 12 | | Overbodig voorwerp | 99 | nergens gebruikt |
| 13 | | stuk dik touw, vastgebonden aan het balkon | 99 | na KNOOP TOUW in 25 |
| 14 | SCHA KIST | schatkist, vol met gouden munten | 35 schatkamer | winst: ermee in kamer 11 (4520) |
| 15 | BROO SNEE | sneetje brood | 2 hal | EET → weg (3560) |
| 16 | MUNT DUIV | gouden duivelaarsmunt | 2 hal | laten vallen in 16 (de bron) → `MUNT%=1`, bronwater blauw (3050); GEEF MUNT in 12 → weg (7900) |
| 17 | FLES MELK | fles melk (zonder statiegeld) | 2 hal | DRINK MELK → 18 (3450) |
| 18 | FLES | lege fles | 99 | VUL FLES in 16 → 19, of 34 als `MUNT%=1` (5920–5940) |
| 19 | FLES WATE | fles water | 99 | DRINK WATER → 18 (3510) |
| 20 | HALS | gouden halsband | 24 slaapkamer 2 | geen functie in de code |
| 21 | | zwaar eikenhouten bed | 1 slaapkamer | decor |
| 22 | | zware gesloten kist | 29 toren | OPEN KIST → 23 (5320) |
| 23 | | zware geopende kist met een gesloten luik onderin | 99 | OPEN LUIK (als `TRN%=1`) → 24 (5410) |
| 24 | | zware geopende kist met een geopend luik onderin | 99 | ook via KIJK TORE/GAT in 51 (6560–6600, zet `TRN%=1`) |
| 25 | | zware kist met een dikke laag stof op de bodem | 14 zolder herberg | BLAAS STOF → 26 (2340) |
| 26 | | zware kist met een boek onderin | 99 | PAK BOEK → 27 (2420) |
| 27 | | zware lege kist | 99 | |
| 28 | | @scherven | 99 | een fles laten vallen → fles weg, scherven in de kamer (2910) |
| 29 | BIJL | botte bijl | 99 | na HAK BOOM |
| 30 | SCHE | schep | 6 achter het huis | nodig voor GRAAF; ligt hij bij het vallen van de nacht in een kamer > 20, dan verhuist hij naar woud 7–10 (3730) |
| 31 | | ondiepe waterbron | 16 heuvel | VUL FLES, munt erin. GRAAF in 36 zet hem op 36 (3300, vreemd) |
| 32 | DOOS | kleine houten doos | 4 kelder | OPEN DOOS → hamer 33 valt eruit (5510) |
| 33 | HAME | houten hamer | 99 | uit de doos; nodig voor SLA (wig) |
| 34 | FLES | fles gevuld met blauw,giftig water | 99 | laten vallen bij de spin → spin dood: 35 weg, 41 erbij (2930–2970). Valt nooit van schrik (10440). Drinken doodt niet (zie Notities) |
| 35 | | gevaarlijke kruisspin | 34 kruiskamer | zie 34 |
| 36 | BRIE | klein briefje | 0 eigen huis | LEES BRIEF |
| 37 | HARN IJZE GROO | groot ijzeren harnas | 21 kasteelhal | GA HARN → kamer 54; GA VUUR in 54 brengt het van 34 naar 35 en terug (2190/2200) |
| 38 | | zware doodskist met in gothische letters de tekst 'D R A C U L A' | 37 graftombe | OPEN KIST → 39, Dracula wordt wakker (5260) |
| 39 | KIST DOOD | zware geopende doodskist met op de zijkant de tekst 'DRACULA' | 99 | GA KIST → kamer 38; in 26 → slee naar 11 (2220–2240) |
| 40 | | ~De vreemd gevormde deur is nu open | 99 | toverwoord in 31 → op 31 (10590) |
| 41 | SPIN KRUI | dode kruisspin | 99 | zie 34 |

## Spelverloop (spoiler; exacte commando's in `PLAYER`)

1. Huis/dorp/bos: brief, lamp, knoflook kopen, waard, boek, luisteren in de herberg, bijl/hout/kruis snijden, ladder, toren openen (`TRN%`).
2. Overdag duikt Dracula op in het kasteel (30%/beurt, regel 3740). Blijf je bij hem, dan loopt `DA%` op van 2 naar 6: bij 5 een waarschuwing, bij 6 ben je dood. Verjagen: knoflook gooien (`DA%=0`) en de beurt erna beschijnen met de lamp (`DA%=-1`), dan `DXD%=1`.
3. Bij de kasteelingang valt daarna de nacht. Thuis slapen → luik onder het bed → kelder → graven naar het noorden → steen wegduwen → graftombe 37.
4. Doodskist openen: Dracula loopt elke beurt 37→32→31→30→22→24 (opent onderweg hek en deur). Volgen, in 24 kruis tonen en de wig slaan (hamer nodig). Hij vergaat en het stof toont het toverwoord.
5. Toverwoord zeggen in 31 → SESAM-deur open → kruiskamer: spin doden door de fles gif te laten vallen. Door het vuur in het harnas (`GA HARN`, `GA VUUR`), schatkist halen.
6. Ontsnappen: touw aan het balkon knopen, via de rand met de doodskist als slee naar het dorp. **Winst** = in kamer 11 met de schatkist (`OL%(14)=-1`), bericht 282.

## Bekende problemen en valkuilen

- **pcbasic-crash** `KeyError: Dereferencing detached string` in de string-GC, twee keer gezien bij "OPEN KIST" in de graftombe
  (regel 10330 `T$ = TIME$`, autoplay, grafisch venster; logs `~/Library/Application Support/pcbasic-2.0/crash-20261114-*.log`).
  Waarschijnlijke oorzaak (14-11-2026 onderzocht; de crash zelf is niet opnieuw gereproduceerd): **een bug in pcbasic**, niet in Dracula.
  `StringFunctions.instr_/left_/right_/mid_` (`pcbasic/basic/values/values.py`) zetten hun stringargumenten in `memory.temp_values`, maar slaan
  het opruimen over bij een vroege `return` (`INSTR` niets gevonden, lege string, start voorbij het einde, lengte 0). Ook upstream master
  heeft dit nog. Dracula lekt zo bij elk commando (`INSTR` in 530/560/610), na één keer uitspelen ~830–950 tijdelijke waarden. De string-ruimte
  blijft dan vol (1–6 bytes vrij) en de GC draait om de paar statements, vooral in de `TIME$`-wachtlus. Uiteindelijk wijst een gelekte waarde
  naar een al vrijgegeven string. Een `TIMER`-wachtlus is dus géén oplossing, alleen minder GC op die plek.
  Oplossing: `pcbasic_fix.py` (gebruikt door `run` en `run-nowindow`). Het omwikkelt de vier functies en ruimt na elke aanroep achtergebleven
  waarden op (getest: 900 gelekte waarden → 0, zelfde uitkomsten). Overbodig zodra pcbasic dit zelf oplost; tot die tijd onschadelijk.
- **Autoplay**: pcbasic geeft zonder `RANDOMIZE` als eerste `RND` .1213501, dus `LN%=1` = VAMPIRANIA. `PLAYER` probeert INCORONIUM en
  FLAUWEKULA (vermoedelijk van de originele EXE). Daarom is `vampirania` toegevoegd; met `k.k.k.k` op regel 40 speelt de autoplay het spel headless helemaal uit.
  Met `k.k.k.k.k.k.k.k` (regel 40 sinds 14-11) gaat de speler dood in kamer 51: Dracula duikt tijdens het kijken op en valt aan voordat
  regel 41 (`gooi knof.beschijn drac`) aan de beurt is. Dat gebeurt in pcbasic en in de C-versie precies hetzelfde.
- **Reïncarnatie werkt niet in pcbasic**: na de dood en `J` stopt het met "File already open in 11550". pcbasic laat bij `CLEAR` de bestanden
  open (`_clear_all(close_files=False)`), maar `FU%()` wordt gewist, dus `DRACULA.TXT` wordt opnieuw als #1 geopend. Bovendien wist `CLEAR` de arrays,
  terwijl `GOTO 100` de `DIM` op regel 30 overslaat. In de gecompileerde EXE (statische arrays) speelde dat waarschijnlijk niet. De C-versie reïncarneert wel.
- **cli-interface scrollt niet** (pcbasic 2.0.8, `interface/video_cli.py`). `VideoCLI` schrijft alleen een `\n` als de cursor naar een lager rijnummer gaat.
  Op de onderste regel scrollt het virtuele 80×25-scherm, maar de cursor blijft op rij 25, dus elke nieuwe regel komt met `\r` over dezelfde
  terminalregel. pcbasic herstelt dat alleen als de cursor zichtbaar wordt (`LINE INPUT`), en dan alleen voor één scherm. Bij autoplay
  (`!`, wachten met `INPUT$(1)`) gebeurt dat nooit. Oplossing in `pcbasic_fix.py`: na elke scroll-up meteen `_update_position` naar de cursorrij
  (getest via pty: autoplay en normaal spel scrollen correct).
- De cli-interface (`run-nowindow`) behandelt alles wat in één keer binnenkomt als één toets. Bij geautomatiseerd testen via een pty
  (`script -q /dev/null ./run-nowindow`) moet je de toetsen dus één voor één sturen, met Enter als losse `\r`, en `TERM` zetten.
  Voor geautomatiseerde tests is `--interface=none` (zie Draaien) eenvoudiger.
- **Bewaren**: `DRACULA.SAV` = `OL%(1..41)` + 28 vlaggen in vaste volgorde (8170–8820). `NO` of de volgorde wijzigen maakt oude saves onbruikbaar.
  `XX%` wordt bewaard maar nergens gebruikt, en `LAD0%` (ladder in huis) wordt níet bewaard. Vermoedelijk een tikfout in het origineel.
- `LAAD SPEL` zonder `DRACULA.SAV`: `FINDFILE` doet niets in pcbasic, dus `OPEN "R"` maakt een leeg bestand → "Laadfout." en `END` (verwacht, niet getest).
- `BUG`/`FOUT` schrijft naar `C:DRACULA.BUG`. pcbasic op macOS mount alleen de werkmap (als `Z:`), dus dat faalt waarschijnlijk (niet getest).
- Regel 1420 gebruikt `R9`, dat nooit gezet wordt; onschuldig, want de enige uitgang 99 (kamer 38) wordt al in 1370 afgehandeld.
- Nieuwe woorden moeten uniek zijn in de eerste 5 (werkwoord) of 4 (naamwoord) letters.
- Teksten en spelfouten ("Illigal item number", "lantaren", "kompjoetertje") zijn origineel: alleen aanpassen op verzoek.

## Werkafspraken

- Wijzigingen in `DRACULA.BAS`; `dracula-322/` als referentie laten staan.
- Blijf binnen GW-BASIC (pcbasic emuleert GW-BASIC 3.23). Behoud CRLF, ASCII en regelnummering in stappen van 10.
- `DRACULA.TXT` alleen via een script lezen of wijzigen (struct, 80-byte records), nooit als tekst opslaan. Maak eerst een backup.

## Notities

- 14-11-2026: project onderzocht en deze CLAUDE.md aangemaakt. Geen git-repo.
- 14-11-2026: pcbasic-crash onderzocht (zie Bekende problemen). De autoplay is 4× zonder crash doorgelopen (3× met het oude `CHR$(7)`, waarvan 1× in het grafische venster via `--keys`), maar telkens met ~830+ gelekte tijdelijke strings.
- 14-11-2026: `pcbasic_fix.py` en `run-nowindow` toegevoegd en `run` omgezet (het oude `run` deed `pcbasic DRACULA.BAS` zonder fix en zonder `--quit`).
- 14-11-2026: scroll-reparatie voor de cli-interface toegevoegd aan `pcbasic_fix.py` (autoplay in `run-nowindow` overschreef regels).
  Hulpscript om een opgenomen terminalstroom weer te geven (`\r`/`\n` toepassen): `termrender.py` in de scratchpad van die sessie.
- 14-11-2026: `DRACULA.BAS` naar C vertaald (`dracula.c`, `Makefile`, `run-c`) en vergeleken met pcbasic (zie "C-versie"). Daarbij gevonden:
  reïncarnatie faalt in pcbasic ("File already open in 11550"), en het gif drinken (3490) doodt niet, omdat 3910 `DOOD%` meteen weer op 0 zet (origineel gedrag, in C overgenomen).
