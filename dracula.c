/*
 * dracula.c -- Dracula Avontuur v3.22
 * (c) 1982 Incore Automatisering / R.van Woensel produkties
 *
 * Vertaling naar C van DRACULA.BAS (de uit DRACULA.EXE gereconstrueerde GW-BASIC-bron).
 * Gestructureerd in plaats van GOTO-spaghetti, maar met hetzelfde gedrag: elke functie
 * noemt het BASIC-regelnummer waar ze vandaan komt. De variabelen heten zoals in de BASIC
 * (R% -> g.r, OL%(i) -> g.ol[i], ...). Die namen zijn bij de reconstructie bedacht.
 *
 * Bouwen:  make            (of: cc -O2 -o dracula dracula.c)
 * Starten: ./dracula       (in de map met DRACULA.TXT; PLAYER voor de autoplay)
 * Titelscherm: willekeurige toets = spelen, '!' = autoplay uit PLAYER, 'T' = testermodus.
 *
 * Sprongen: elke handler geeft een `enum vervolg` terug, de hoofdlus (speel) voert die uit:
 *   GOTO 170 = melding M% tonen en beurt   GOTO 340/350 = kamer beschrijven en beurt
 *   GOTO 360 = beurt (Dracula, spin, ...)  GOTO 390 = volgende invoer zonder beurt
 *   GOTO 7710 = dood                        GOTO 560 = CM$ opnieuw ontleden
 *
 * Bewust gelijk aan pcbasic (zodat de autoplay hetzelfde spel speelt):
 *   - RND/RANDOMIZE gebruiken dezelfde generator als GW-BASIC/pcbasic. BASIC rekent alle termen
 *     van AND/OR uit, dus waar RND in zo'n voorwaarde staat wordt hij hier altijd getrokken.
 *   - CALL ISIBM/ISZENITH geven 0: eenvoudig titelscherm, geen schermwissen, "msBASCOM".
 * Bewust anders dan DRACULA.BAS onder pcbasic:
 *   - CLEAR bij reincarnatie sluit ook de bestanden en wist alles. (In pcbasic stopt
 *     reincarnatie met "File already open in 11550", omdat pcbasic CLEAR de bestanden open laat.)
 *   - CALL FINDFILE is echt geimplementeerd: LAAD SPEL zonder DRACULA.SAV geeft melding 189
 *     (pcbasic maakt een leeg bestand aan en stopt met "Laadfout.").
 *   - Geen afbreken op 80 kolommen; spaties/nullen aan het eind van tekstregels worden weggelaten.
 *   - C:DRACULA.BUG wordt DRACULA.BUG in de werkmap, direct aangevuld (in plaats van kopieren
 *     naar DRACULA.$$$ en hernoemen). TESTER wordt aangevuld; LPRINT (printer) vervalt.
 *   - "/" zonder testermodus schrijft niets (BASIC: "Bad file number").
 *   - Wachtlussen: TIME$-wachten -> time() + nanosleep, FOR T%=0 TO 29000 -> 1 seconde.
 *   - Einde van de invoer (stdin) beeindigt het programma.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define NO_MAX 50     /* DIM OL%(50), OBJ$(50) */
#define SLEN   1024   /* ruim boven de 255 tekens van een BASIC-string */

#define KOPIEER(d, s) snprintf((d), sizeof(d), "%s", (s))

enum vervolg {
    NA_MELDING,     /* GOTO 170 */
    NA_KIJK,        /* GOTO 340 */
    NA_BESCHRIJF,   /* GOTO 350 */
    NA_BEURT,       /* GOTO 360 */
    NA_INVOER,      /* GOTO 390 */
    NA_DOOD,        /* GOTO 7710 */
    NA_ONTLEED      /* GOTO 560 */
};

/* ------------------------------------------------------------------------------------------
 * Alle BASIC-variabelen. CLEAR (reincarnatie) zet ze in een keer op 0 / "".
 * ------------------------------------------------------------------------------------------ */
static struct {
    char mach[16], vers[40];
    char in[SLEN];     /* IN$: invoer, met nog uit te voeren commando's (gescheiden door ".") */
    char cm[SLEN];     /* CM$: huidig commando */
    char v[SLEN];      /* V$: werkwoord, eerste 5 letters */
    char n[SLEN];      /* N$: naamwoord, eerste 4 letters */
    char vv[SLEN];     /* VV$: werkwoord volledig */
    char nn[SLEN];     /* NN$: naamwoord volledig */
    char nl[SLEN];     /* NL$: naamwoord in kleine letters */
    char land[SLEN];   /* LAND$: toverwoord */
    char obj[NO_MAX + 1][80];   /* OBJ$(i) */
    int ol[NO_MAX + 1];         /* OL%(i): kamer, -1 = bij speler, 99 = weg */
    int ex[11];                 /* EX%(1..7): uitgangen N,Z,O,W,H,L,E */
    int ty[18];                 /* TY%(1..16): titelanimatie */
    int no, nc, nv, n9;
    int r, m, dr, dv, ln, dag, d23, afsp, dood;
    int raam, lad0, lad30, deur22, sesam, steen, hek, kist, trn, dxd, da, spin;
    int r28, waar, hb, munt, bed, luik, doos, tw, slaap, luis, xx;
    int wm, wk, wh, wc, wi;     /* resultaat van zoek_voorwerp (6700) */
    int seed, ibm, zen;
    int rb, mb, ib, h4, h5;     /* header van DRACULA.TXT */
} g;

/* Volgorde van de vlaggen in DRACULA.SAV, na OL%(1..NO) (8270-8820). Niet wijzigen. */
static int *const bewaar_vlaggen[28] = {
    &g.r, &g.dag, &g.xx, &g.lad30, &g.steen, &g.hek, &g.munt, &g.r28, &g.waar, &g.dxd,
    &g.dr, &g.trn, &g.hb, &g.bed, &g.luik, &g.doos, &g.tw, &g.slaap, &g.luis, &g.d23,
    &g.da, &g.raam, &g.kist, &g.deur22, &g.dv, &g.sesam, &g.spin, &g.ln
};

static FILE *tf;   /* #TF%: DRACULA.TXT */
static FILE *pf;   /* #PF%: PLAYER (autoplay) */
static FILE *t1;   /* #T1%: TESTER */

/* 10-11: toverwoorden, elke letter +1 versleuteld */
static const char *const land_data[6] = {
    "WBNQJSBOJB", "GMBVXFLVMB", "ESBLFMTUFJOJB", "NJDSPTPGUFOTUFJO", "JODPSPOJVN", "USBOTTZMWBOJVN"
};

/* ------------------------------------------------------------------------------------------
 * RND en RANDOMIZE zoals GW-BASIC (pcbasic, values/randomiser.py)
 * ------------------------------------------------------------------------------------------ */
#define RND_PERIODE 16777216L
#define RND_START   5228370L
static int64_t rnd_zaad = RND_START;

static void rnd_stap(void)
{
    rnd_zaad = (rnd_zaad * 214013 + 2531011) % RND_PERIODE;
}

static float rnd(void)
{
    rnd_stap();
    return (float)rnd_zaad / (float)RND_PERIODE;
}

static void randomize(int n)   /* RANDOMIZE n% */
{
    rnd_zaad &= 0xff;
    rnd_stap();
    rnd_zaad = (rnd_zaad + (int64_t)n * 4455680) % RND_PERIODE;
    if (rnd_zaad < 0)
        rnd_zaad += RND_PERIODE;
}

/* ------------------------------------------------------------------------------------------
 * Toetsenbord en scherm
 * ------------------------------------------------------------------------------------------ */
static int is_tty, skip_lf;
static struct termios tty_oud;

static void sluit_bestanden(void)
{
    if (tf) fclose(tf);
    if (pf) fclose(pf);
    if (t1) fclose(t1);
    tf = pf = t1 = NULL;
}

static void herstel_terminal(void)
{
    if (is_tty)
        tcsetattr(STDIN_FILENO, TCSANOW, &tty_oud);
}

static void bij_signaal(int sig)
{
    herstel_terminal();
    signal(sig, SIG_DFL);
    raise(sig);
}

static void einde(void)   /* END */
{
    fflush(stdout);
    sluit_bestanden();
    herstel_terminal();
    exit(0);
}

static void ruwe_modus(int vmin)
{
    struct termios t = tty_oud;
    t.c_lflag &= ~(tcflag_t)(ICANON | ECHO);
    t.c_cc[VMIN] = (cc_t)vmin;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

static int lees_byte(void)   /* -1 = niets (meer) te lezen */
{
    unsigned char c;
    for (;;) {
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == 1)
            return c;
        if (n < 0 && errno == EINTR)
            continue;
        return -1;
    }
}

static int volgende_byte(void)   /* slaat de LF van een CR-LF na LINE INPUT over */
{
    int c = lees_byte();
    if (skip_lf) {
        skip_lf = 0;
        if (c == '\n')
            c = lees_byte();
    }
    return c;
}

static void invoer_op(void)
{
    putchar('\n');
    einde();
}

static int toets(void)   /* INPUT$(1): wacht op een toets, zonder echo */
{
    int c;
    fflush(stdout);
    if (is_tty)
        ruwe_modus(1);
    c = volgende_byte();
    if (is_tty)
        tcsetattr(STDIN_FILENO, TCSANOW, &tty_oud);
    if (c < 0)
        invoer_op();
    return c;
}

static int inkey(void)   /* INKEY$: -1 als er geen toets klaarstaat */
{
    int c = -1;
    fflush(stdout);
    if (is_tty) {
        ruwe_modus(0);
        c = lees_byte();
        tcsetattr(STDIN_FILENO, TCSANOW, &tty_oud);
    } else {
        struct pollfd p = { STDIN_FILENO, POLLIN, 0 };
        if (poll(&p, 1, 0) > 0 && (p.revents & POLLIN))
            c = lees_byte();
    }
    return c;
}

static void line_input(const char *prompt, char *buf, size_t grootte)   /* LINE INPUT "p"; X$ */
{
    size_t n = 0;
    int c;
    fputs(prompt, stdout);
    fflush(stdout);
    for (;;) {
        c = volgende_byte();
        if (c < 0) {
            if (n == 0)
                invoer_op();
            break;
        }
        if (c == '\r') {
            skip_lf = 1;
            break;
        }
        if (c == '\n')
            break;
        if (n < 255 && n + 1 < grootte)
            buf[n++] = (char)c;
    }
    buf[n] = '\0';
    if (!is_tty)   /* geen terminal die meeschrijft: zelf tonen, zoals het BASIC-scherm */
        printf("%s\n", buf);
}

static void wacht_sec(int ws)   /* 10290: wacht tot TIME$ ws seconden verder is */
{
    time_t t0;
    struct timespec kort = { 0, 50000000L };
    fflush(stdout);
    t0 = time(NULL);
    while (time(NULL) < t0 + ws)
        nanosleep(&kort, NULL);
}

static void wis_scherm(void)   /* 11940 */
{
    if (g.ibm != 0) fputs("\033[2J\033[H", stdout);   /* CALL CLRSCR */
    if (g.zen != 0) fputs("\033E", stdout);
}

static void cursor(int x, int y)   /* 11970 */
{
    if (g.ibm != 0) printf("\033[%d;%dH", y, x);       /* CALL CURSOR(X%,Y%) */
    if (g.zen != 0) printf("\033Y%c%c", y + 32, x + 32);
}

/* ------------------------------------------------------------------------------------------
 * BASIC-stringfuncties
 * ------------------------------------------------------------------------------------------ */
static int isV(const char *s) { return strcmp(g.v, s) == 0; }
static int isN(const char *s) { return strcmp(g.n, s) == 0; }

static int instr(int start, const char *s, const char *pat)   /* INSTR(start, s$, pat$) */
{
    const char *p;
    if ((size_t)start > strlen(s))
        return 0;
    p = strstr(s + start - 1, pat);
    return p ? (int)(p - s) + 1 : 0;
}

static void mid(char *d, size_t dsz, const char *s, int start, int len)   /* MID$(s$, start, len) */
{
    if (start < 1 || (size_t)start > strlen(s) || len <= 0)
        d[0] = '\0';
    else
        snprintf(d, dsz, "%.*s", len, s + start - 1);
}

static int mid_is(const char *s, int start, const char *pat)   /* MID$(s$, start, LEN(pat$)) = pat$ */
{
    size_t l = strlen(s), n = strlen(pat);
    if (n == 0)
        return 1;
    if ((size_t)start > l || l - (size_t)(start - 1) < n)
        return 0;
    return memcmp(s + start - 1, pat, n) == 0;
}

/* ------------------------------------------------------------------------------------------
 * DRACULA.TXT: records van 80 bytes (11530-11910)
 * ------------------------------------------------------------------------------------------ */
static int cvi(const unsigned char *p)
{
    return (int16_t)(p[0] | p[1] << 8);
}

static void lees_record(int n, unsigned char *b)   /* GET #TF%, n */
{
    memset(b, 0, 80);
    if (tf && n >= 1 && fseek(tf, (long)(n - 1) * 80, SEEK_SET) == 0)
        (void)fread(b, 1, 80, tf);
}

static void druk_tekst(int rec)   /* 11720: PRINT TX$ en volg de keten */
{
    unsigned char b[80];
    char s[79];
    int i, len, nx;
    for (;;) {
        lees_record(rec, b);
        for (i = 0; i < 78; i++)
            s[i] = b[i] ? (char)b[i] : ' ';
        len = 78;
        while (len > 0 && s[len - 1] == ' ')
            len--;
        printf("%.*s\n", len, s);
        nx = cvi(b + 78);
        if (nx <= 0)
            break;
        rec = nx;
    }
}

static void melding(int m)   /* M% = m : GOSUB 11920 */
{
    g.m = m;
    druk_tekst(g.m + g.mb);
}

static void kamertekst(int rr)   /* 11700 */
{
    if (rr < 0) {
        puts("Err.51210");
        puts("Break in 11710");
        einde();
    }
    druk_tekst(g.rb + rr + 1);
}

static void lees_uitgangen(int rr)   /* 11600 */
{
    unsigned char b[80];
    int d;
    lees_record(rr + 1 + 1, b);
    for (d = 1; d <= 7; d++)
        g.ex[d] = cvi(b + 2 * (d - 1));
}

static void lees_voorwerp(int i)   /* 11760 */
{
    unsigned char b[80];
    int lg;
    if (i < 1) {
        puts("Illigal item number");
        einde();
    }
    lees_record(g.ib + i, b);
    lg = cvi(b + 76);
    if (lg < 0 || lg > 76)
        lg = 0;
    memcpy(g.obj[i], b, (size_t)lg);
    g.obj[i][lg] = '\0';
    g.ol[i] = cvi(b + 78);
}

static void open_tekstbestand(void)   /* 10810-10820, 11530-11590, 11840-11910 */
{
    unsigned char b[80];
    tf = fopen("DRACULA.TXT", "rb");
    lees_record(1, b);
    g.rb = cvi(b);
    g.mb = cvi(b + 2);
    g.ib = cvi(b + 4);
    g.h4 = cvi(b + 6);
    g.h5 = cvi(b + 8);
    if (g.rb == 0) {
        puts("Err 51440 -- Bestand DRACULA.TXT niet gevonden of verminkt.");
        einde();
    }
}

/* ------------------------------------------------------------------------------------------
 * Subroutines (GOSUB)
 * ------------------------------------------------------------------------------------------ */
static const char *objnaam(int i)   /* 4580: OB$ = OBJ$(I) zonder de /XXXX-sleutels */
{
    const char *s = g.obj[i];
    while (s[0] == '/')
        s = strlen(s) >= 5 ? s + 5 : "";
    return s;
}

static void tel_gedragen(void)   /* 3240 */
{
    int i;
    g.nc = 0;
    for (i = 1; i <= g.no; i++)
        if (g.ol[i] == -1)
            g.nc++;
}

static void zoek_voorwerp(void)   /* 6700: object bij N$ zoeken */
{
    int j;
    g.wm = g.wk = g.wh = g.wc = 0;
    for (j = 1; j <= g.no; j++) {
        const char *o = g.obj[j];
        if (!mid_is(o, 2, g.n) && !mid_is(o, 7, g.n) && !mid_is(o, 12, g.n))
            continue;
        g.wk = -1;
        if (g.ol[j] == g.r) { g.wh = -1; g.wi = j; }
        if (g.ol[j] == -1) { g.wc = -1; g.wi = j; }
        if (g.wh || g.wc) {
            g.wm = 1;
            return;
        }
    }
}

static void decodeer_toverwoord(void)   /* 10730 */
{
    char *c;
    if (g.ln > 6) {
        puts("Out of DATA in 10750");
        einde();
    }
    if (g.ln >= 1)
        KOPIEER(g.land, land_data[g.ln - 1]);
    for (c = g.land; *c; c++)
        *c = (char)(*c - 1);
}

static void wachten(void)   /* 10210 */
{
    int k;
    wacht_sec(2);
    for (k = 1; k <= 12; k++) {
        fputs("** BIEP ** ", stdout);   /* het origineel: PRINT CHR$(7); */
        wacht_sec(2);
    }
}

static void dracula_vergaat(void)   /* 10360 */
{
    melding(265);
    puts("De stof waait door elkaar en lijkt op een bepaald moment");
    printf("letters te vormen. Vaag kun je de tekst %s lezen.\n", g.land);
    puts("Dan verwaait het stof totaal en blijft er geen spoor van deze");
    puts("toch eens zeer indrukwekkende dracula over.");
}

static void schrik(void)   /* 10430: van de schrik iets laten vallen */
{
    int i;
    for (i = 1; i <= g.no; i++) {
        float x = rnd();   /* wordt voor elk voorwerp getrokken (AND rekent alles uit) */
        if (g.ol[i] == -1 && x > 0.7f && i != 34) {
            printf("Van de schrik laat je een %s vallen.\n", objnaam(i));
            g.ol[i] = g.r;
            return;
        }
    }
}

static void kamer_gebeurtenissen(void)   /* 3590 */
{
    float x;
    if (g.r == 0) {
        if (g.lad0 == 0) {
            melding(40);
            return;
        }
        if (g.lad0 == 1 && g.r == 0)
            melding(41);
    }
    if (g.r == 20 || g.r == 21)
        melding(g.r + (1 - g.dag) * 2 + 22);
    x = rnd();
    if (g.r > 6 && g.r < 12 && x > 0.8f)
        melding(46);
    if (g.r == 28)
        g.r28 = 1;
    if (g.r == 30 && g.lad30 == 1)
        melding(224);
    if (g.r == 11 && g.r28 == 1)
        melding(47);
    if (g.r == 12 && g.waar == 1)
        melding(48);
    if (g.r == 20 && g.dxd == 1 && g.dag != 0) {   /* Dracula verjaagd: de nacht valt */
        melding(49);
        g.dag = 0;
        if (g.ol[30] > 20)
            g.ol[30] = (int)(rnd() * 4.0f) + 7;
    }
    /* 3740: overdag in het kasteel duikt Dracula op (30%) */
    x = rnd();
    if (!(g.r < 21 || g.r == 26 || g.r == 39 || g.r == g.dr || g.dxd == 1 || g.dag == 0 ||
          g.trn == 0 || x > 0.3f)) {
        g.dr = g.r;
        g.da = 2;
    }
    if (g.r == g.dr && g.r == 24 && g.kist == 1)
        melding(50);
    if (g.r == g.dr && g.kist == 0)
        melding(50);
    if (g.r == 32)
        melding(g.hek == 1 ? 51 : 52);
    if (g.r == 37) {
        melding(53);
        if (g.hek != 1)
            melding(54);
    }
    if (g.r == 23)
        melding(g.d23 == 1 ? 55 : 56);
    if (g.r == 40)
        g.d23 = 1;
}

static void beschrijf_kamer(void)   /* 10610 */
{
    int i;
    kamertekst(g.r);
    kamer_gebeurtenissen();
    for (i = 1; i <= g.no; i++) {
        const char *ob;
        if (g.ol[i] != g.r)
            continue;
        ob = objnaam(i);
        if (ob[0] == '@')
            printf("Er zijn %s hier.\n", ob + 1);
        else if (ob[0] == '~')
            printf("%s\n", ob + 1);
        else
            printf("Er is een %s hier.\n", ob);
    }
}

static void per_beurt(void)   /* 3910: Dracula, spin en winst, elke beurt */
{
    g.dood = 0;
    if (g.r == g.dr && g.dxd != 1) {   /* Dracula is hier en valt aan */
        if (g.da < 0) {                /* verjaagd met knoflook + lamp */
            g.dxd = 1;
            g.dr = 255;
            melding(150);
        } else if (g.da == 6) {
            melding((int)(rnd() * 2.0f) + 152);
            g.dood = -1;
            return;
        } else {
            if (g.da == 5)
                melding((int)(rnd() * 2.0f) + 148);
            g.da++;
        }
    }
    if (g.kist != 0 && g.dr != 24) {   /* 4070: Dracula loopt van zijn kist naar kamer 24 */
        int dz = (g.r == g.dr) ? -1 : 0;
        if (g.dr == 22) { g.dr = 24; g.m = 250; g.da = 2; g.deur22 = -1; g.dv = 22; }
        if (g.dr == 30) { g.dr = 22; g.m = 251; g.dv = 30; }
        if (g.dr == 31) { g.dr = 30; g.m = 252; g.dv = 31; }
        if (g.dr == 32) { g.dr = 31; g.m = 253; g.dv = 32; }
        if (g.dr == 37) { g.dr = 32; g.m = 254; g.hek = 1; g.dv = 37; }
        if (dz != 0)
            melding(g.m);
    } else if (g.kist != 0 && g.dr == 24 && g.r == 24) {   /* 4250: in zijn slaapkamer */
        if (g.da < 0) {
            g.dr = 257;
            g.kist = 0;
            dracula_vergaat();
        } else if (g.da == 7) {
            melding(257);
            g.dood = -1;
        } else {
            if (g.da == 3) {
                melding(255);
            } else if (g.da == 4) {
                melding(256);
                schrik();
            }
            g.da++;
            g.dv = 255;
        }
    }
    if (g.r == 34 && g.r == g.ol[35]) {   /* 4440: de kruisspin */
        if (g.spin > 3) {
            melding(270);
            g.dood = -1;
        } else {
            if (g.spin == 2)
                melding(271);
            g.spin++;
        }
    }
    if (g.r == 11 && g.ol[14] == -1) {   /* 4520: met de schatkist in het dorp */
        melding(282);
        fputs("--- Druk een toets ---", stdout);
        putchar(toets());
        putchar('\n');
        einde();
    }
}

/* ------------------------------------------------------------------------------------------
 * Bestanden: bewaren, laden, fouten melden, tester
 * ------------------------------------------------------------------------------------------ */
static void schrijf_int16(FILE *f, int rec, int w)   /* LSET S$ = MKI$(w) : PUT #SF%, rec */
{
    unsigned char b[2];
    b[0] = (unsigned char)(w & 0xff);
    b[1] = (unsigned char)((w >> 8) & 0xff);
    if (fseek(f, (long)(rec - 1) * 2, SEEK_SET) == 0)
        (void)fwrite(b, 1, 2, f);
}

static int lees_int16(FILE *f, int rec)   /* GET #SF%, rec : CVI(S$) */
{
    unsigned char b[2] = { 0, 0 };
    if (fseek(f, (long)(rec - 1) * 2, SEEK_SET) == 0)
        (void)fread(b, 1, 2, f);
    return cvi(b);
}

static void bewaar_spel(void)   /* 8170 */
{
    FILE *f;
    int k;
    melding(186);
    f = fopen("DRACULA.SAV", "r+b");   /* OPEN "R": bestaand bestand niet afkappen */
    if (!f)
        f = fopen("DRACULA.SAV", "w+b");
    if (!f) {
        puts("Disk not ready");
        return;
    }
    for (k = 1; k <= g.no; k++)
        schrijf_int16(f, k, g.ol[k]);
    for (k = 0; k < 28; k++)
        schrijf_int16(f, g.no + 1 + k, *bewaar_vlaggen[k]);
    fclose(f);
}

static enum vervolg laad_spel(void)   /* 8860 */
{
    FILE *f;
    int k;
    melding(187);
    f = fopen("DRACULA.SAV", "rb");   /* 8890: CALL FINDFILE */
    if (!f) {
        g.m = 189;
        return NA_MELDING;
    }
    for (k = 1; k <= g.no; k++)
        g.ol[k] = lees_int16(f, k);
    for (k = 0; k < 28; k++)
        *bewaar_vlaggen[k] = lees_int16(f, g.no + 1 + k);
    decodeer_toverwoord();
    fclose(f);
    if ((g.dr != 257 && g.sesam != 0) || g.dr == 0) {
        puts("Laadfout.");
        einde();
    }
    return NA_BEURT;
}

static enum vervolg fout_melden(void)   /* 7930: FOUT/BUG -> DRACULA.BUG */
{
    char naam[SLEN], l[SLEN] = "";
    FILE *f;
    int i;
    melding(184);
    line_input("==>", naam, sizeof naam);
    f = fopen("DRACULA.BUG", "ab");
    if (f) {
        for (i = 0; i < 79; i++)
            fputc('*', f);
        fprintf(f, "\r\n\r\n Speler: %s\r\n", naam);
    }
    melding(185);
    while (strcmp(l, ".") != 0) {   /* de punt wordt ook weggeschreven */
        line_input("==>", l, sizeof l);
        if (f)
            fprintf(f, "%s\r\n", l);
    }
    if (f)
        fclose(f);
    return NA_BEURT;
}

static void tester_openen(void)   /* 12190: TESTER aanvullen */
{
    t1 = fopen("TESTER", "ab");
}

static void tester_opmerking(void)   /* 12080 */
{
    char l[SLEN];
    puts("");
    puts("Hallo beste tester.");
    puts("Typ nu jouw opmerking in, als je klaar bent typ dan");
    puts("een punt en dan return");
    for (;;) {
        line_input("commentaar -->", l, sizeof l);
        if (strcmp(l, ".") == 0)
            break;
        if (t1)
            fprintf(t1, "%s\r\n", l);
    }
    if (t1) {
        fputs("\r\n", t1);
        fflush(t1);
    }
    puts("");
}

static void lees_player_regel(char *buf, size_t grootte)   /* 420: LINE INPUT #PF%, IN$ */
{
    size_t n = 0;
    int c = fgetc(pf);
    if (c == EOF || c == 0x1a) {
        puts("");
        puts("Input past end in 420");
        einde();
    }
    while (c != EOF && c != '\r' && c != '\n' && c != 0x1a) {
        if (n < 255 && n + 1 < grootte)
            buf[n++] = (char)c;
        c = fgetc(pf);
    }
    if (c == '\r') {
        int d = fgetc(pf);
        if (d != '\n' && d != EOF)
            ungetc(d, pf);
    }
    buf[n] = '\0';
}

/* ------------------------------------------------------------------------------------------
 * Werkwoorden
 * ------------------------------------------------------------------------------------------ */
static enum vervolg meld(int m)    /* M% = m : GOTO 170 */
{
    g.m = m;
    return NA_MELDING;
}

static enum vervolg naar(int kamer)   /* R% = kamer : GOTO 340 */
{
    g.r = kamer;
    return NA_KIJK;
}

static enum vervolg bekijk(void);
static enum vervolg leg(void);
static enum vervolg pak(void);
static enum vervolg slaap(void);
static enum vervolg toon(void);

static enum vervolg zie_geen(void)   /* 10050 */
{
    printf("Ik zie geen %s hier.\n", g.nl);
    return NA_BEURT;
}

static enum vervolg in_harnas(void)   /* 10090 */
{
    if (g.r != g.ol[37])
        return meld(277);
    return naar(54);
}

static enum vervolg ladder_op(void)   /* 3370 */
{
    if (g.lad0 == 0)
        return meld(35);
    return naar(3);
}

static enum vervolg zolder_herberg(void)   /* 4750 */
{
    if (g.waar == 0)
        return meld(61);
    if (g.hb > 0)
        return meld(62);
    return naar(14);
}

static enum vervolg naar_bron(void)   /* 6870 */
{
    if (g.r28 == 1)
        return naar(15);
    g.r = (int)(rnd() * 4.0f) + 7;
    melding(127);
    return NA_BESCHRIJF;
}

static enum vervolg luik_kelder(void)   /* 6920 */
{
    if (g.bed == 0)
        return meld(128);
    return naar(4);
}

static enum vervolg touw_klimmen(void)   /* 7140 */
{
    if (g.ol[13] != 25)
        return meld(140);
    return naar(g.r == 25 ? 26 : 25);
}

static enum vervolg toren_luik(void)   /* 7810 */
{
    if (g.r == g.ol[24])
        return naar(24);
    if (g.r == g.ol[23])
        return meld(169);
    return meld(170);
}

static enum vervolg ga(void)   /* 1260: GA <richting/plaats> */
{
    int d = 0;
    int deur = isN("DEUR") || isN("SLAA");
    if (g.r == g.dr)
        return meld(229);
    if (isN("NOOR") || isN("N")) d = 1;
    if (isN("ZUID") || isN("Z")) d = 2;
    if (isN("OOST") || isN("O")) d = 3;
    if (isN("WEST") || isN("W")) d = 4;
    if (isN("HOOG") || isN("OMHO") || isN("H")) d = 5;
    if (isN("LAAG") || isN("OMLA") || isN("L")) d = 6;
    if (isN("ERUI") || isN("E") || isN("UIT")) d = 7;
    if (d != 0) {
        if (d == 7 && g.r == 54)
            return naar(g.ol[37]);
        if (d == 7 && g.r == 38)
            return naar(g.ol[39]);
        lees_uitgangen(g.r);
        if (g.ex[d] == 255)
            return meld(11);
        g.r = g.ex[d];
        if (g.r == 99)
            g.r = 0;   /* 1420: R% = R9, maar R9 wordt nergens gezet */
        return NA_KIJK;
    }
    /* 1440 */
    if (g.r == 0) {
        if (isN("HAL")) return naar(2);
        if (isN("RAAM")) {
            if (g.raam != 0) return meld(236);
            return naar(6);
        }
        if (isN("GAT") && g.lad0 == 0) return meld(12);
        if (isN("SLAA")) return naar(1);
        if (isN("DEUR")) return naar(11);
        if (isN("LADD") || isN("GAT")) return ladder_op();
    }
    if (g.r == 11) {
        if (isN("HUIS") || isN("LINK")) return naar(0);
        if (isN("HERB")) return naar(12);
        if (isN("BRON")) return naar_bron();
    }
    if (g.r == 12) {
        if (isN("ZITT")) return naar(13);
        if (isN("ZOLD") || isN("TRAP")) return zolder_herberg();
        if (isN("DEUR")) return naar(11);
    }
    if (g.r == 13 && isN("STAA")) return naar(12);
    if (g.r == 2 && (isN("UITG") || isN("RUIM"))) return naar(0);
    if (g.r == 1 && isN("LUIK")) return luik_kelder();
    if (isN("SLAP") || isN("BED")) return slaap();
    if (isN("HARN")) return in_harnas();
    if (g.r == 4 && isN("LUIK")) return naar(1);
    if (g.r == 5 && isN("GANG")) return naar(4);
    if (g.r == 6 && isN("RAAM")) {
        if (g.raam != 0) return meld(236);
        return naar(0);
    }
    if (g.r > 6 && g.r < 11 && isN("BOOM")) return meld(13);
    if (g.r == 17 && isN("BOOM")) return naar(18);
    if ((g.r == 26 || g.r == 25) && isN("TOUW")) return touw_klimmen();
    if (g.r == 3 && isN("LADD")) return naar(0);
    if (g.r == 19 && (isN("PAD") || isN("KAST"))) return naar(20);
    if ((g.r == 20 && (isN("DEUR") || isN("KAST"))) || (g.r == 21 && isN("DEUR"))) {   /* 1790 */
        if (g.dag == 1) return naar(g.r == 20 ? 21 : 20);
        return meld(14);
    }
    if (g.r == 27 && isN("DEUR")) return naar(21);
    if (g.r == 21 && isN("TRAP")) return naar(22);
    if (g.r == 30 && isN("LADD")) {
        if (g.lad30 == 0) return meld(15);
        return naar(33);
    }
    if (g.r == 30 && isN("GAT")) {
        if (g.lad30 == 1) return naar(33);
        return meld(16);
    }
    if (g.r == 33 && isN("GAT")) return naar(30);
    if (g.r == 29 && isN("LUIK")) return toren_luik();
    if (g.r == 29 && isN("RAAM")) return naar(28);
    if (g.r == 32 && (isN("POOR") || isN("KERK") || isN("BINN"))) return naar(36);
    if (g.r == 39 && (isN("GAT") || isN("STEE")) && g.steen == 1) {
        g.r = 37;
        return meld(17);
    }
    if (g.r == 37 && (isN("GAT") || isN("STEE"))) return meld(18);
    if (g.r == 32 && (isN("HEK") || isN("TRAP")) && g.hek == 1) return naar(37);
    if (g.r == 37 && (isN("HEK") || isN("TRAP")) && g.hek == 1) return naar(32);
    if (g.r == 1 && isN("RAAM")) return meld(204);
    if (g.r == 14 && (isN("TRAP") || isN("ZOLD"))) return naar(12);
    if (g.r == 40 && deur) return naar(23);
    if (g.r == 23 && deur) return naar(40);
    if (g.r == 22 && deur) {
        if (g.deur22 != 0) return naar(24);
        return meld(258);
    }
    if (g.r == 24 && deur) {
        if (g.deur22 != 0) return naar(22);
        return meld(258);
    }
    if (g.dv == g.r && (isN("DRAC") || isN("MAN"))) return naar(g.dr);
    if (g.r == 31 && (isN("DEUR") || isN("SESA"))) {
        if (g.sesam != 0) return naar(34);
        return meld(269);
    }
    if (g.r == 54 && g.ol[37] == 34 && isN("VUUR") && g.r != g.ol[35]) {
        g.ol[37] = 35;
        return meld(278);
    }
    if (g.r == 54 && g.ol[37] == 35 && isN("VUUR")) {
        g.ol[37] = 34;
        return meld(279);
    }
    if ((g.r == 34 || g.r == 35) && isN("VUUR")) {
        melding(281);
        return NA_DOOD;
    }
    if (isN("KIST") && g.r == g.ol[39] && g.r != 26) return naar(38);
    if (isN("KIST") && g.r == g.ol[39] && g.r == 26) {   /* met de doodskist als slee naar het dorp */
        g.ol[39] = 11;
        melding(280);
        g.r = 38;
        return NA_BESCHRIJF;
    }
    return meld(19);
}

static enum vervolg blaas(void)   /* 2310 */
{
    if ((!isN("STOF") && !isN("KIST")) || g.r != 14)
        return meld(212);
    if (g.ol[25] != 14)
        return meld(20);
    g.ol[25] = 99;
    g.ol[26] = 14;
    return meld(21);
}

static enum vervolg pak_alles(void)   /* 4610: zet ") XXXX." per voorwerp in de wachtrij */
{
    char sleutel[8], nieuw[SLEN];
    int i;
    for (i = 1; i <= g.no; i++) {
        if (g.obj[i][0] == '/' && g.ol[i] == g.r) {
            mid(sleutel, sizeof sleutel, g.obj[i], 2, 4);
            snprintf(nieuw, sizeof nieuw, ") %s.%s", sleutel, g.in);
            KOPIEER(g.in, nieuw);
        }
    }
    return NA_BEURT;
}

static enum vervolg leg_alles(void)   /* 4650 */
{
    char sleutel[8], nieuw[SLEN];
    int i;
    for (i = 1; i <= g.no; i++) {
        if (g.ol[i] < 0) {
            mid(sleutel, sizeof sleutel, g.obj[i], 2, 4);
            snprintf(nieuw, sizeof nieuw, "DROP %s.%s", sleutel, g.in);
            KOPIEER(g.in, nieuw);
        }
    }
    return NA_BEURT;
}

static enum vervolg pak_voorwerp(void)   /* 2640 */
{
    zoek_voorwerp();
    if (g.wk == 0)
        return meld(2);
    if (g.wc != 0)
        return meld(248);
    if (g.wh == 0)
        return zie_geen();
    if (g.nc > 7)
        return meld(23);
    fputs("Ok", stdout);
    if (isV(")"))
        printf(", %s : gepakt.", objnaam(g.wi));
    putchar('\n');
    g.ol[g.wi] = -1;
    return NA_BEURT;
}

static enum vervolg pak_knoflook(void)   /* 6950 */
{
    if (g.r != g.ol[4])
        return meld(129);
    if (g.r != 12)
        return pak_voorwerp();
    if (g.hb > 1) {
        melding(143);
        return NA_DOOD;
    }
    g.m = 130;
    g.hb++;
    if (g.hb > 1) {
        melding(g.m);
        g.m = 131;
    }
    return NA_MELDING;
}

static enum vervolg pak(void)   /* 2370: PAK / NEEM */
{
    tel_gedragen();
    if (g.n[0] == '\0')
        return meld(200);
    if (isN("ALLE"))
        return pak_alles();
    if (isN("BOEK") && g.r == g.ol[26]) {
        puts("Ok.");
        g.ol[26] = 99;
        g.ol[27] = g.r;
        g.ol[5] = -1;
        return NA_BEURT;
    }
    if (isN("BED") && g.r == 1)
        return meld(205);
    if (strncmp(g.nn, "SCHERV", 6) == 0 && g.r == g.ol[28])
        return meld(249);
    if (isN("LADD")) {
        if (g.r == 0 && g.lad0 == 1) {
            g.lad0 = 0;
            g.ol[7] = -1;
            puts("Ok.");
            return NA_BEURT;
        }
        if (g.r == 30 && g.lad30 == 1) {
            g.lad30 = 0;
            g.ol[7] = -1;
            puts("Ok.");
            return NA_BEURT;
        }
    }
    if ((isN("ZAND") || isN("STOF")) && g.r == 0)
        return meld(230);
    if (isN("KNOF"))
        return pak_knoflook();
    if (isN("KIST") && (g.r == 2 || g.r == 14 || g.r == 29))
        return meld(22);
    if ((isN("KIST") || isN("DOOD")) && g.r == g.ol[39] && g.nc > 0)
        return meld(238);
    if ((isN("KIST") || isN("DOOD")) && g.r == g.ol[38])
        return meld(237);
    return pak_voorwerp();
}

static enum vervolg inventaris(void)   /* 2740 */
{
    int i, f = 0;
    for (i = 1; i <= g.no; i++) {
        const char *ob;
        if (g.ol[i] != -1)
            continue;
        if (f == 0) {
            puts("Je draagt:");
            f = 1;
        }
        ob = objnaam(i);
        if (ob[0] == '@')
            printf("%s.\n", ob + 1);
        else
            printf("Een %s.\n", ob);
    }
    if (f == 0)
        puts("Je hebt niets bij je.");
    return NA_BEURT;
}

static enum vervolg leg(void)   /* 2840: LEG / DROP */
{
    int i;
    if (g.n[0] == '\0')
        return meld(201);
    if (isN("ALLE"))
        return leg_alles();
    zoek_voorwerp();
    i = g.wi;
    if (g.wc == 0)
        return meld(24);
    if (isN("FLES")) {   /* de fles valt kapot */
        g.m = 25;
        g.ol[i] = 99;
        g.ol[28] = g.r;
        if (i == 34 && g.r == g.ol[35]) {   /* gif op de spin */
            melding(g.m);
            g.m = 272;
            g.ol[35] = 99;
            g.ol[41] = g.r;
        }
        return NA_MELDING;
    }
    if (isN("LADD") && g.r == 0) {
        g.m = 26;
        g.lad0 = 1;
        g.ol[i] = 99;
        return NA_MELDING;
    }
    if (isN("MUNT") && g.r == 16) {
        melding(27);
        g.munt = 1;
    }
    if (isN("LADD") && g.r == 30) {
        g.m = 28;
        g.lad30 = 1;
        g.ol[i] = 99;
        return NA_MELDING;
    }
    printf("%s : laten vallen.\n", objnaam(i));
    g.ol[i] = g.r;
    return NA_BEURT;
}

static enum vervolg hak(void)   /* 3150 */
{
    if (!isN("BOOM"))
        return meld(2);
    if (g.r != 15 && (g.r < 7 || g.r > 10) && g.r != 17 && g.r != 18)
        return meld(29);
    if (g.ol[29] < 0)
        return meld(30);
    if (g.ol[11] > -1)
        return meld(3);
    g.ol[8] = g.r;
    g.ol[11] = 99;
    g.ol[29] = -1;
    return meld(31);
}

static enum vervolg graaf(void)   /* 3290 */
{
    if (g.ol[30] > -1)
        return meld(3);
    if (g.r == 36) {
        g.ol[31] = g.r;
        return meld(32);
    }
    if ((g.r < 4 || g.r > 5) && g.r != 39)
        return meld(33);
    if (isN("GRON") || isN("GAT"))
        return meld(34);
    if (g.r == 4)
        return naar(5);
    if (g.n[0] == 'N')
        return naar(39);
    return naar((int)(rnd() * 4.0f) + 7);
}

static enum vervolg eet(void)   /* 3400: EET / DRINK */
{
    if (isN("BROO")) {   /* 3540 */
        if (g.ol[15] > -1)
            return meld(4);
        g.m = 39;
        g.ol[15] = 99;
        g.nc--;
        return NA_MELDING;
    }
    if (isN("WATE")) {   /* 3480 */
        if (g.ol[19] > -1 && g.ol[34] > 1)
            return meld(4);
        if (g.ol[34] == -1) {
            g.dood = -1;   /* 3910 zet DOOD% meteen weer op 0: het gif doodt niet */
            return meld(37);
        }
        g.m = 38;
        g.ol[19] = 99;
        g.ol[18] = -1;
        return NA_MELDING;
    }
    if (!isN("MELK"))
        return meld(1);
    if (g.ol[17] > -1)
        return meld(4);
    g.m = 36;
    g.ol[17] = 99;
    g.ol[18] = -1;
    return NA_MELDING;
}

static enum vervolg vraag(void)   /* 4690 */
{
    if (!isN("WAAR") || g.r != 12)
        return meld(57);
    if (g.waar != 0)
        return meld(58);
    if (g.hb > 0)
        return meld(59);
    g.waar = 1;
    return meld(60);
}

static enum vervolg spring(void)   /* 4790 */
{
    if (g.r == 18) {
        melding(63);
        g.r = 17;
        schrik();
        return NA_BESCHRIJF;
    }
    if (g.r == 25 || g.r == 28) {
        melding(219);
        return NA_DOOD;
    }
    if (g.r == 33) {
        g.m = 225;
        g.r = 30;
        melding(g.m);
        schrik();
        return NA_BESCHRIJF;
    }
    return meld(64);
}

static enum vervolg duw(void)   /* 4940 */
{
    if (isN("WAAR") && g.r == 12)
        return meld(65);
    if (!isN("STEE") || g.r != 39)
        return meld(68);
    if (g.steen == 1)
        return meld(66);
    g.steen = 1;
    return meld(67);
}

static enum vervolg open_(void)   /* 5000 */
{
    if (isN("RAAM") && (g.r == 0 || g.r == 6)) {
        if (g.raam == 0)
            return meld(235);
        g.raam = 0;
        return meld(233);
    }
    if (isN("DEUR") && g.r == 0)
        return meld(283);
    if (isN("LUIK") && g.r == 1) {
        if (g.bed == 0)
            return meld(70);
        if (g.luik == 1)
            return meld(71);
        g.luik = 1;
        return meld(72);
    }
    if (isN("HEK") && (g.r == 32 || g.r == 37)) {
        if (g.hek == 1)
            return meld(73);
        if (g.r == 32)
            return meld(74);
        g.hek = 1;
        return meld(75);
    }
    if (g.r == 14 && isN("KIST"))
        return meld(76);
    if (g.r == g.ol[38] && (isN("KIST") || isN("DOOD"))) {   /* 5190: de doodskist */
        melding(243);
        wachten();
        g.m = 244;
        g.kist = 1;
        g.dr = 37;
        g.ol[38] = 99;
        g.ol[39] = g.r;
        return NA_MELDING;
    }
    if ((g.r == g.ol[39] || g.ol[39] == -1) && (isN("KIST") || isN("DOOD")))
        return meld(245);
    if (g.r == 29 && isN("KIST")) {
        g.m = 77;
        g.ol[22] = 99;
        g.ol[23] = g.r;
        return NA_MELDING;
    }
    if (g.r == 2 && isN("KIST"))
        return meld(207);
    if ((g.r == g.ol[5] || g.ol[5] == -1) && isN("BOEK"))
        return meld(214);
    if (g.r == 29 && isN("LUIK")) {
        if (g.r != g.ol[23])
            return meld(78);
        if (g.trn == 1) {
            g.m = 79;
            g.ol[23] = 99;
            g.ol[24] = g.r;
            return NA_MELDING;
        }
        return meld(80);
    }
    zoek_voorwerp();
    if (g.wm == 1 && isN("DOOS")) {
        if (g.doos == 1)
            return meld(81);
        g.doos = 1;
        g.m = 82;
        g.ol[33] = g.r;
        return NA_MELDING;
    }
    return meld(83);
}

static enum vervolg knoop(void)   /* 5550 */
{
    if (!isN("TOUW"))
        return meld(84);
    if (g.ol[2] != -1)
        return meld(85);
    if (g.r != 25)
        return meld(86);
    g.ol[2] = 99;
    g.ol[13] = g.r;
    g.n9--;
    g.tw = 1;
    return meld(87);
}

static enum vervolg help(void)   /* 5640 */
{
    if (g.r > 20 && g.r < 39 && g.dag == 0)
        return meld(88);
    if (g.r < 21 && g.dag == 0)
        return meld(89);
    return meld(90);
}

static enum vervolg lees(void)   /* 5680 */
{
    if (isN("BOEK")) {
        if (g.ol[5] > -1)
            return meld(91);
        return meld(92);
    }
    if (isN("TEKS")) {
        if (g.r == 23 && g.d23 == 0)
            return meld(95);
        if (g.r == 20)
            return meld(217);
        if (g.ol[5] == -1 || g.r == g.ol[5])
            return meld(92);
        return meld(93);
    }
    if (isN("BRIE"))
        return meld((g.ol[36] == -1 || g.r == g.ol[36]) ? 202 : 203);
    if (!isN("INSC"))
        return bekijk();
    if (g.r == 2)
        return meld(210);
    return zie_geen();
}

static enum vervolg vul(void)   /* 5850 */
{
    if (!isN("FLES"))
        return meld(96);
    if (g.r != 16)
        return meld(97);
    if (g.ol[17] == -1)
        return meld(98);
    if (g.ol[19] == -1)
        return meld(99);
    if (g.ol[18] > -1)
        return meld(100);
    melding(101);
    g.ol[18] = 99;
    if (g.munt == 1) {
        g.ol[34] = -1;
        return NA_BEURT;
    }
    g.ol[19] = -1;
    return NA_BEURT;
}

static enum vervolg snij_hout(void)   /* 6040 */
{
    if (g.ol[8] > -1)
        return meld(104);
    g.ol[8] = 99;
    g.ol[10] = -1;
    return meld(105);
}

static enum vervolg snij(void)   /* 5960 */
{
    if (g.ol[6] > -1)
        return meld(3);
    if (isN("TAK")) {
        if (g.r == g.ol[9])
            return meld(4);
        if (g.ol[9] > -1)
            return meld(102);
        return meld(103);
    }
    if (isN("HOUT")) {
        if (g.r == g.ol[8])
            return meld(4);
        return snij_hout();
    }
    if (isN("KRUI")) {
        if (g.r == g.ol[8])
            return meld(106);
        return snij_hout();
    }
    if (isN("BOOM"))
        return meld(107);
    return meld(108);
}

static enum vervolg bekijk(void)   /* 6150: KIJK / BEKIJK */
{
    zoek_voorwerp();
    if (g.wm && isN("WIG")) return meld(109);
    if (g.wm && isN("BOEK")) return meld(110);
    if ((isN("BLOE") || isN("VLEK")) && g.r == 14) return meld(213);
    if (g.wm && isN("KAPM")) return meld(111);
    if (g.wm && isN("HARN")) return meld(112);
    if ((isN("HARN") || isN("VIZI")) && g.r == 54) {   /* 6210: door het vizier van het harnas */
        if (g.ol[37] == 21)
            return meld((int)(rnd() * 3.0f + 226.0f));
        if (g.ol[37] == -1)
            return meld(276);
        if (g.ol[37] != 34 && g.ol[37] != 35) {
            int rs = g.r;
            g.r = g.ol[37];
            beschrijf_kamer();
            g.r = rs;
            return NA_BEURT;
        }
        return meld(g.ol[37] == 34 ? 274 : 275);
    }
    if (g.wm && isN("TOUW")) return meld(113);
    if (isN("RAAM") && g.r == 29) return meld((int)(rnd() * 3.0f + 220.0f));
    if (isN("STEE") && g.r == 14) return meld(215);
    if ((isN("VOET") || isN("SPOR")) && g.r == 17) return meld(218);
    if (isN("CIRK") && g.r == 2) return meld(209);
    if (g.wm && (isN("SCHA") || isN("KIST")) && g.wi == 14) return meld(114);
    if (isN("KIST") && g.r == 2) return meld(208);
    if (isN("BED")) {
        if (g.r != 1)
            return meld(115);
        if (g.slaap == 0)
            return meld(116);
        g.m = 117;
        g.bed = 1;
        return NA_MELDING;
    }
    if ((isN("KIST") || isN("DOOD")) && g.r == g.ol[38]) return meld(246);
    if (g.wm && (isN("KIST") || isN("DOOD")) && g.wi == 39) return meld(247);
    if ((isN("DEUR") || isN("VREE") || isN("SESA")) && g.r == 31) return meld(284);
    if (isN("LUIK")) {
        if (g.r == 1 && g.bed == 1)
            return meld(118);
        if (g.r == 29 && (g.r == g.ol[23] || g.r == g.ol[24]))
            return meld(119);
    }
    if (g.r == 51 && (isN("TORE") || isN("GAT"))) {   /* 6520: de toren opent zich */
        if (g.trn == 0 && g.dxd == 0) {
            g.m = 120;
            g.ol[22] = 99;
            g.ol[23] = 99;
            g.ol[24] = 29;
            g.luik = 1;
            g.trn = 1;
            return NA_MELDING;
        }
        return meld(177);
    }
    if (g.r == 52 && (isN("EETK") || isN("ROOS") || isN("GAT"))) return meld(121);
    zoek_voorwerp();
    if (g.wm == 1)
        return meld(122);
    return meld(123);
}

static enum vervolg slaap(void)   /* 6820 */
{
    if (g.r != 1)
        return meld(124);
    if (g.dag == 1 || g.slaap == 1)
        return meld(125);
    g.slaap = 1;
    return meld(126);
}

static enum vervolg koop(void)   /* 7020 */
{
    if (g.r != g.ol[4])
        return meld(132);
    if (g.r != 12 || !isN("KNOF"))
        return meld(133);
    if (g.hb > 0) {
        g.hb = 0;
        return meld(134);
    }
    melding(135);
    return pak_voorwerp();
}

static enum vervolg luister(void)   /* 7080 */
{
    if (g.r == 31)
        return meld(223);
    if (g.r != 13)
        return meld(136);
    g.luis++;
    if (g.luis > 3)
        g.luis = 3;
    return meld(g.luis + 136);
}

static enum vervolg dood_7310(void)   /* 7310: DOOD WAARD / DRACULA */
{
    if (isN("WAAR") && g.r == 12) {
        float x;
        g.hb++;
        g.m = (int)(rnd() * 5.0f) + 141;
        if (g.m == 143) {
            melding(g.m);
            return NA_DOOD;
        }
        x = rnd();
        if (x > 0.8f) {
            melding(g.m);
            schrik();
            return NA_BEURT;
        }
        return NA_MELDING;
    }
    if (isN("DRAC")) {
        if (g.r != g.dr)
            return meld(146);
        return meld((int)(rnd() * 2.0f) + 172);
    }
    return meld(1);
}

static enum vervolg sla(void)   /* 7170: SLA / SCHOP */
{
    if ((isN("SPIN") || strncmp(g.nn, "KRUISS", 6) == 0) && g.r == g.ol[35])
        return meld(273);
    if (isN("WAAR"))
        return dood_7310();
    if (isN("DRAC") && g.r == g.dr && g.dxd != 1)
        return meld(171);
    if (isN("PIN") || isN("WIG")) {   /* de wig in Dracula slaan */
        if (g.ol[3] != -1)
            return meld(24);
        if (g.ol[33] != -1)
            return meld(262);
        if (g.r != g.dr || g.r != 24)
            return meld(263);
        if (g.da > 1)
            return meld(264);
        g.da = -1;
        g.ol[3] = 99;
        return NA_BEURT;
    }
    return meld(1);
}

static enum vervolg dood_maken(void)   /* 7300: DOOD / VERMOORD */
{
    if ((isN("SPIN") || strncmp(g.nn, "KRUISS", 6) == 0) && g.r == g.ol[35])
        return meld(273);
    return dood_7310();
}

static enum vervolg roep(void)   /* 7430 */
{
    if (g.n[0] == '\0')
        return meld(154);
    printf("..%s..%s....\n", g.nn, g.nn);
    return NA_BEURT;
}

static enum vervolg sluit(void)   /* 7490 */
{
    if (isN("KIST")) {
        if (g.r == 2)
            return meld(207);
        if (g.r == 14)
            return meld(160);
        if (g.r == g.ol[38]) {
            if (g.kist == 0)
                return meld(241);
            return meld(242);
        }
        return meld(161);
    }
    if (isN("DEUR")) {
        if (g.r == 20 || g.r == 21)
            return meld(162);
        if (g.r == 11 || g.r == 0)
            return meld(163);
        if (g.r == 23) {
            g.d23 = 0;
            return meld(164);
        }
        if (g.r == 40)
            return meld(165);
    }
    if (isN("RAAM") && (g.r == 0 || g.r == 6)) {
        if (g.raam != 0)
            return meld(234);
        g.raam = -1;
        return meld(232);
    }
    return meld(166);
}

static enum vervolg geef(void)   /* 7860 */
{
    if (g.n[0] == '\0')
        return meld(180);
    if (!isN("MUNT") || g.r != 12)
        return meld(181);
    if (g.ol[16] != -1)
        return meld(182);
    g.m = 183;
    g.ol[16] = 99;
    if (g.hb > 0) {
        melding(g.m);
        g.m = 190;
        g.hb = 0;
    }
    return NA_MELDING;
}

static void stoppen(void)   /* 9600 */
{
    int s;
    melding(188);
    s = toets();
    if (s == 'J' || s == 'j')
        bewaar_spel();
    einde();
}

static enum vervolg gooi(void)   /* 9660: GOOI / WERP */
{
    int ti = 0;
    if (isN("KRUI") && g.r == 24 && g.dr == 24 && g.ol[10] == -1)
        return toon();
    if (!isN("KAPM") && !isN("BIJL") && !isN("KNOF"))
        return leg();
    if (isN("KAPM")) {
        ti = 6;
        if (g.ol[6] > -1)
            return leg();
    }
    if (isN("BIJL")) {
        ti = 11;
        if (g.ol[11] > -1) {
            ti = 29;
            if (g.ol[29] > -1)
                return leg();
        }
    }
    if (isN("KNOF")) {
        ti = 4;
        if (g.ol[ti] > -1)
            return meld(85);
    }
    if (g.r == 12 && ti != 4) {   /* 9800: naar de waard */
        if (g.hb == 0) {
            g.hb = 1;
            g.ol[ti] = g.r;
            return meld(ti == 6 ? 192 : 191);
        }
        g.m = 193 + (ti == 6 ? 3 : 0) + (int)(rnd() * 3.0f);
        if (g.m != 193 && g.m != 196)
            g.ol[ti] = g.r;
        return NA_MELDING;
    }
    if (g.r != g.dr)
        return leg();
    g.ol[ti] = g.r;   /* 9890: naar Dracula */
    if (g.kist && ti == 4)
        return meld(259);
    if (ti == 4 && g.dxd != 1) {
        if (g.da != 1) {
            g.da = 0;
            return meld(174);
        }
        g.da = -1;
        return NA_BEURT;
    }
    return meld(151);
}

static enum vervolg til(void)   /* 9970: TIL / TREK */
{
    if (isN("BED")) {
        if (g.r != 1)
            return zie_geen();
        return meld(206);
    }
    if (isN("KIST") && g.r == g.ol[38])
        return meld(240);
    if ((isN("UIT") || isN("HARN")) && g.r == 54)
        return naar(g.ol[37]);
    if (isN("HARN") && g.r == g.ol[37])
        return in_harnas();
    return pak();
}

static enum vervolg beschijn(void)   /* 10120 */
{
    if (g.ol[1] > -1)
        return meld(176);
    if (!isN("DRAC"))
        return bekijk();
    if (g.r != g.dr)
        return meld(2);
    if (g.dxd != 0)
        return meld(259);
    if (g.da == 1) {
        g.da = -1;
        return NA_BEURT;
    }
    g.da = 0;
    return meld(175);
}

static enum vervolg toon(void)   /* 10510: TOON KRUIS */
{
    if (!isN("KRUI") || g.r != g.dr || g.r != 24)
        return meld(260);
    if (g.ol[10] != -1)
        return meld(24);
    g.da = 0;
    return meld(261);
}

static enum vervolg zeg_toverwoord(void)   /* 10560 */
{
    if (g.dr != 257)
        return meld(266);
    g.sesam = 1;
    g.ol[40] = 31;
    return meld(267);
}

static void debug_kamer(void)   /* 750: $RVW */
{
    char s[SLEN], *e;
    long w;
    printf("Room%s%d \n", g.r < 0 ? "" : " ", g.r);
    for (;;) {
        line_input("Room? ", s, sizeof s);
        w = strtol(s, &e, 10);
        while (*e == ' ')
            e++;
        if (*e == '\0' && w >= -32768 && w <= 32767)
            break;
        puts("?Redo from start");
    }
    g.r = (int)w;
}

/* ------------------------------------------------------------------------------------------
 * Invoer ontleden en werkwoord kiezen
 * ------------------------------------------------------------------------------------------ */
static void ontleed(void)   /* 560-730 */
{
    char *c;
    int p = instr(2, g.cm, " ");
    if (p > 0) {
        snprintf(g.v, sizeof g.v, "%.*s", p - 1, g.cm);
        KOPIEER(g.n, g.cm + p);
    } else {
        KOPIEER(g.v, g.cm);
        g.n[0] = '\0';
    }
    for (c = g.n; *c == ' '; c++)
        ;
    memmove(g.n, c, strlen(c) + 1);
    p = instr(1, g.n, " ");
    if (p > 0 && p != (int)strlen(g.n))
        printf("Ik neem aan dat je '%s %.*s' bedoelt.\n", g.v, p - 1, g.n);
    KOPIEER(g.vv, g.v);
    g.v[5] = '\0';    /* LEFT$(V$, 5); g.v is lang genoeg */
    KOPIEER(g.nn, g.n);
    KOPIEER(g.nl, g.n);
    g.n[4] = '\0';    /* LEFT$(N$, 4) */
    for (c = g.nl; *c; c++)
        if (*c >= 'A' && *c <= 'Z')
            *c = (char)(*c - 'A' + 'a');
    if (g.v[0] != '\0' && g.v[1] == '\0' && strchr("NZOWHLE", g.v[0])) {   /* 710: losse richting */
        KOPIEER(g.n, g.v);
        KOPIEER(g.v, "GA");
    }
}

static enum vervolg voer_uit(void)   /* 740-1250 */
{
    if (isV("GA") || isV("BETRE") || isV("KRUIP") || isV("LOOP") || isV("KLIM") || isV("VOLG"))
        return ga();
    if (isV("$RVW")) {
        debug_kamer();
        return NA_KIJK;
    }
    if ((isV("KIJK") || isV("K")) && g.n[0] == '\0')
        return NA_KIJK;
    if (isV("BEKIJ") || isV("KIJK") || strcmp(g.vv, "BESCHRIJF") == 0 || isV("ONDER"))
        return bekijk();
    if (isV(")") || isV("PAK") || isV("GRIJP") || isV("NEEM") || isV("RAAP"))
        return pak();
    if (isV("LEG") || isV("ZET") || isV("DROP"))
        return leg();
    if (isV("GOOI") || isV("WERP"))
        return gooi();
    if (isV("SHOW") || isV("TOON") || isV("HOUDT"))
        return toon();
    if (isV("GEEF"))
        return geef();
    if (strcmp(g.vv, g.land) == 0 && g.r == 31)
        return zeg_toverwoord();
    if (isV("SCHIJ") || strcmp(g.vv, "BESCHIJN") == 0)
        return beschijn();
    if (isV("DOOD") || isV("VERMO") || isV("LIQUI"))
        return dood_maken();
    if (isV("SLA") || isV("STOMP") || isV("SCHOP") || isV("TRAP"))
        return sla();
    if (isV("/")) {
        tester_opmerking();
        return NA_BEURT;
    }
    if (isV("VRAAG"))
        return vraag();
    if (isV("PAS") || isV("DRAAG"))
        return in_harnas();
    if (isV("HAK") || isV("KAP"))
        return hak();
    if (isV("FOUT") || isV("BUG") || isV("COMMA") || isV("KOMMA"))
        return fout_melden();
    if (isV("BREEK") || isV("SCHEU") || isV("VERNI"))
        return meld(5);
    if (isV("GRAAF") || isV("SCHEP"))
        return graaf();
    if (isV("SPRIN"))
        return spring();
    if (isV("DRUK") || isV("DUW"))
        return duw();
    if (isV("BLAAS"))
        return blaas();
    if (isV("GIL") || isV("ROEP") || isV("SCHRE") || isV("BRUL"))
        return roep();
    if (isV("GODVE") || isV("SHIT") || isV("KUT") || isV("KLOOT") || isV("KANKE") || isV("GOD") ||
        isV("FUCK") || isV("GEDVE"))
        return meld((int)(rnd() * 5.0f) + 155);   /* 7470 */
    if (isV("SLUIT"))
        return sluit();
    if (isV("TIL") || isV("TREK"))
        return til();
    if (isV("OPEN"))
        return open_();
    if ((isV("SAVE") || isV("BEWAA")) && isN("SPEL")) {
        bewaar_spel();
        return NA_BEURT;
    }
    if ((isV("LOAD") || isV("LAAD")) && isN("SPEL"))
        return laad_spel();
    if (isV("QUIT") || isV("EIND") || isV("STOP") || (isV("HOU") && isN("OP")))
        stoppen();
    if (isV("WACHT") || isV("RUST"))
        return meld(216);
    if (isV("SLAAP"))
        return slaap();
    if (isV("BEVES") || isV("HANG") || isV("KNOOP"))
        return knoop();
    if (isV("ZEG"))
        return meld(6);
    if (isV("HELP") || isV("HULP"))
        return help();
    if (isV("LIJST") || isV("INVEN") || isV("I"))
        return inventaris();
    if (isV("LEES"))
        return lees();
    if (isV("SESAM") || isV("HOKUS") || isV("HOCUS"))
        return meld(211);
    if (isV("VUL"))
        return vul();
    if (isV("EET") || isV("DRINK"))
        return eet();
    if (isV("SNIJ"))
        return snij();
    if (isV("KOOP"))
        return koop();
    if (isV("LUIST"))
        return luister();
    if (isV("STA") && isN("OP")) {
        KOPIEER(g.cm, "GA STAAN");
        return NA_ONTLEED;
    }
    /* 1190: onbekend werkwoord, er gaat geen beurt voorbij */
    if (g.r == 31) {
        g.m = 268;
    } else {
        g.nv++;
        if (g.r == 0 && g.nv > 3) {
            g.m = 231;
            g.nv = 0;
        } else if (rnd() > 0.1f) {
            g.m = (int)(rnd() * 3.0f) + 8;
        } else {
            g.m = 7;
        }
    }
    melding(g.m);
    return NA_INVOER;
}

static enum vervolg verwerk_invoer(void)   /* 390-730 */
{
    enum vervolg v;
    char *c;
    int p;
    while (g.in[0] == '\0') {
        if (g.afsp) {   /* 400: autoplay */
            int k;
            fputs("player ->", stdout);
            lees_player_regel(g.in, sizeof g.in);
            puts(g.in);
            k = toets();
            g.afsp = (k == 'S') ? 0 : -1;
            break;   /* 460 GOTO 490, ook bij een lege regel */
        }
        line_input("->", g.in, sizeof g.in);   /* 470 */
    }
    for (c = g.in; *c; c++)   /* 490: naar hoofdletters */
        if (*c >= 'a' && *c <= 'z')
            *c = (char)(*c - 'a' + 'A');
    p = instr(2, g.in, ".");   /* 530: het eerste commando eraf halen */
    if (p > 0) {
        snprintf(g.cm, sizeof g.cm, "%.*s", p - 1, g.in);
        memmove(g.in, g.in + p, strlen(g.in + p) + 1);
    } else {
        KOPIEER(g.cm, g.in);
        g.in[0] = '\0';
    }
    do {
        ontleed();
        v = voer_uit();
    } while (v == NA_ONTLEED);
    return v;
}

/* ------------------------------------------------------------------------------------------
 * Start, titel, dood en de hoofdlus
 * ------------------------------------------------------------------------------------------ */
static void machine_detectie(void)   /* 12010 */
{
    g.ibm = 0;   /* CALL ISIBM(IBM%): niet beschikbaar */
    g.zen = 0;   /* CALL ISZENITH(ZEN%): niet beschikbaar */
    KOPIEER(g.mach, "msBASCOM");
    if (g.ibm != 0) KOPIEER(g.mach, "IBM PC");
    if (g.zen != 0) KOPIEER(g.mach, "Zenith Z100");
    if (inkey() == '!') {
        g.zen = 0;
        g.ibm = 0;
        puts(" Geen cursoradressing.");
    }
}

static void lege_regels(void)   /* 11390 */
{
    int k;
    for (k = 0; k <= 10; k++)
        puts("");
}

static void titel(void)   /* 10810 */
{
    static const char titeltekst[] = "DRACULA AVONTUUR";
    struct timespec nu;
    int a;
    open_tekstbestand();
    wis_scherm();
    if (g.ibm == 0 && g.zen == 0) {   /* 11310 */
        lege_regels();
        puts("                   D R A C U L A   A V O N T U U R");
        puts("");
        puts("       (c) 1982 Incore Automatisering / R.van Woensel produkties");
        puts("");
        puts("                    Druk een toets om te beginnen");
        lege_regels();
    } else {   /* 10870: de letters vallen op hun plaats */
        int klaar = 0, j;
        while (!klaar) {
            int l = (int)(rnd() * 16.0f + 1.0f);
            int x = l * 2 + 20;
            if (g.ty[l] <= 14) {
                if (g.ty[l] > 0) {
                    cursor(x, g.ty[l]);
                    putchar(' ');
                }
                g.ty[l]++;
                cursor(x, g.ty[l]);
                putchar(titeltekst[l - 1]);
            }
            klaar = 1;
            for (j = 1; j <= 16; j++)
                if (g.ty[j] < 15)
                    klaar = 0;
        }
        cursor(9, 19);
        puts("(c)1982 Incore Automatisering / R.van Woensel produkties");
        cursor(22, 21);
        fputs("Druk een toets om te beginnen", stdout);
    }
    /* 11090: wachten op een toets. In BASIC telt SEED% door tot de toets komt; hier de klok. */
    a = toets();
    clock_gettime(CLOCK_REALTIME, &nu);
    g.seed = (int)((nu.tv_nsec / 1000 + (long)(nu.tv_sec % 1000) * 7919) % 60001) - 30000;
    if (a == '!') {   /* 11140: autoplay */
        g.afsp = -1;
        pf = fopen("PLAYER", "rb");
        if (!pf) {
            puts("File not found in 11180");
            einde();
        }
    }
    if (a == 'T' || a == 't') {
        puts("De tester...");
        tester_openen();
    }
    if (g.afsp == 0)
        randomize(g.seed);
    if (g.ibm || g.zen) {
        wis_scherm();
    } else {
        lege_regels();
        lege_regels();
        lege_regels();
    }
    puts("Dracula Avontuur. (c) 1982 Incore Automatisering / Rotterdam        ");
    printf("Versie %s op MS-DOS.\n", g.vers);
    puts("Serienummer:A000002-MSD");
    puts("");
    puts("");
}

static void start(void)   /* 100-330 */
{
    int i;
    machine_detectie();
    snprintf(g.vers, sizeof g.vers, "%s - 3.22", g.mach);
    titel();
    g.no = 41;
    g.dr = 255;
    g.dv = 255;
    g.ln = (int)(rnd() * 6.0f + 1.0f);
    decodeer_toverwoord();
    for (i = 1; i <= g.no; i++)
        lees_voorwerp(i);
    g.dag = 1;
    g.d23 = 1;
}

static void dood(void)   /* 7710: dood; keert alleen terug om te reincarneren */
{
    struct timespec poef = { 1, 0 };
    int a;
    melding(167);
    a = toets();
    if (a == 'N') {
        wis_scherm();
        einde();
    }
    melding(168);
    fflush(stdout);
    nanosleep(&poef, NULL);   /* 7770: FOR T% = 0 TO 29000 : NEXT */
}

static void speel(void)   /* 100 ... ; keert terug bij reincarnatie */
{
    enum vervolg v;
    start();
    v = NA_BESCHRIJF;
    for (;;) {
        switch (v) {
        case NA_MELDING:   /* 170 */
            melding(g.m);
            v = NA_BEURT;
            break;
        case NA_KIJK:      /* 340 */
            if (!g.afsp)
                wis_scherm();
            /* fall through */
        case NA_BESCHRIJF: /* 350 */
            beschrijf_kamer();
            v = NA_BEURT;
            break;
        case NA_BEURT:     /* 360 */
            g.nv = 0;
            per_beurt();
            v = g.dood ? NA_DOOD : verwerk_invoer();
            break;
        case NA_INVOER:    /* 390 */
        case NA_ONTLEED:
            v = verwerk_invoer();
            break;
        case NA_DOOD:      /* 7710 */
            dood();
            return;
        }
    }
}

static void clear(void)   /* CLEAR */
{
    sluit_bestanden();
    memset(&g, 0, sizeof g);
    rnd_zaad = RND_START;
}

int main(void)
{
    is_tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &tty_oud) == 0;
    signal(SIGINT, bij_signaal);
    signal(SIGTERM, bij_signaal);
    signal(SIGHUP, bij_signaal);
    for (;;) {   /* 7790-7800: CLEAR : GOTO 100 */
        clear();
        speel();
    }
}
