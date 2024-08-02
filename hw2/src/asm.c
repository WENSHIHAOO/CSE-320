#include <stdlib.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <getopt.h>

#define NSYM	400
#define LSIZE	129
#define HSHSIZ	211
#define DOT	(*dot)
#define SINST	01
#define SREG	02
#define SIDENT	04
#define SDIRECT	010
#define SDEF	020
#define SDOT	040
#define EOFF	1
#define GARBG	2
#define NEWLN	3
#define SPACE	4
#define EXCLA	5
#define DQUOTE	6
#define SHARP	7
#define DOL	8
#define PCNT	9
#define AND	10
#define SQUOTE	11
#define LPARN	12
#define RPARN	13
#define STAR	14
#define PLUS	15
#define COMMA	16
#define MINUS	17
#define LETTER	18
#define SLASH	19
#define	DIGIT	20
#define COLON	21
#define SEMI	22
#define LESS	23
#define EQUAL	24
#define GREAT	25
#define QUES	26
#define AT	27
#define LBRACK	28
#define BSLASH	29
#define RBRACK	30
#define UPARR	31
#define UNDER	32
#define GRAVE	33
#define LCURL	34
#define VBAR	35
#define RCURL	36
#define TILDE	37

#define TNEWLN	1
#define TEOFF	2
#define TEXCLA	3
#define TDQUO	4
#define TSHARP	5
#define TDOL	6
#define TPCNT	7
#define TAND	8
#define TCONST	9
#define TLPARN	10
#define TRPARN	11
#define TSTAR	12
#define TPLUS	13
#define TCOMMA	14
#define TMINUS	15
#define TSLASH	16
#define TCOLON	17
#define TLESS	18
#define TEQUAL	19
#define TGREAT	20
#define TQUES	21
#define TAT	22
#define TIDENT	23
#define TLBRAK	24
#define TBSLA	25
#define TRBRAK	26
#define TUPARR	27
#define TUNDER	28
#define TGRAVE	29
#define TLCURL	30
#define TVBAR	31
#define TRCURL	32
#define TTILDE	33

#define REGMOD	01
#define IMMMOD	02
#define DEFIMM	04
#define DEFREG	010

#define IHALT	0
#define IEIS	2
#define IDIS	3
#define ITCI	5
#define ICLRC	6
#define ISETC	7
#define ISWAP	8
#define ISLL	9
#define IRLC	10
#define ISLLC	11
#define ISLR	12
#define ISAR	13
#define IRRC	14
#define ISARC	15
#define IJ	16
#define IJE	17
#define IJD	18
#define IJSR	20
#define IJSRE	21
#define IJSRD	22
#define IB	32
#define INOP2	40
#define IBC	33
#define IBNC	41
#define IBOV	34
#define IBNOV	42
#define IBZE	36
#define IBNZE	44
#define IBLGE	33
#define IBLLT	41
#define IBPL	35
#define IBMI	43
#define IBEQ	36
#define IBNEQ	44
#define IBLT	37
#define IBGE	45
#define IBLE	38
#define IBGT	46
#define IBUSC	39
#define IBESC	47
#define IBEXT	64
#define IMOV	66
#define IADD	67
#define ISUB	68
#define ICMP	69
#define IAND	70
#define IXOR	71
#define IINC	73
#define IDEC	74
#define ICOM	75
#define INEG	76
#define IADC	77
#define IGSWD	78
#define IRSWD	79
#define INOP	80
#define ISIN	81
#define IJMP	82
#define ITST	83
#define ICLR	84


struct symtab {
	char s_name[9];
	int s_flags;
	int s_hash;
	struct symtab *s_link;
	int s_val;
} symtab[NSYM];

struct symtab *sorteds[NSYM];

struct symtab *hshtab[HSHSIZ];
struct insops {
	int i_mode;
	int i_val;
} insops[2];

int ninsops;

int nodata;
int odata;
int isline;
int lineno;
char *fname;
int lastaddr=-1;
int insaddr;

int nsym;
struct symtab *csym;
int cval;
int ctoke;
char lbuf[LSIZE];
char *lp=lbuf;
FILE *ibuf0;
FILE *ibuf1;
int wchbuf=0;
int pass1;
char peekc;
int ispc;
int peekt;
int ispt;
int *dot;
int nerr;
int oradix=8;
int nowds=2;
int obuf[19];
int oaddr;
int ofile;
int anyhalf;
jmp_buf env;
struct halfw{
	char lobyte;
	char hibyte;
}halfw;

int token(), peektoke(), rtoken(), eqs(char *s1, char *s2), line(), expr(int t,int lev), undefined(), less(char *s1,char *s2);
char getch(), peekch(), rgetc();
void untoke(), lflush(FILE *f), lookup(char *st), label(), assign(), cexpr(), num(char ch), error(char *s), bopen(char *s);
void prsym(), syminit(), putwd(int w), iflush(int a), bflush(), puthex(int n), prtwd(int n), putoct(int n), rwordb();
void cinstr(), getops(), ins0(struct symtab *sp), ins1(struct symtab *sp), ins2(struct symtab *sp), jump1(int v), sortsym();
void branch(int v), single(int v), newline(int a), shift(int v), dbl(int v), bext(), jsr(int v), exchg(int i,int j);
void dhex(),doctal(),dascii(),dasciz(),dwordb(),dword(),dbyte(),dblk(),dincld();

struct psym {
    char *p_name;
    int p_flags;
    int p_val;
}psym[]={
    {"tst",  SINST,  ITST},
    {"clr",  SINST,  ICLR},
    {"com",  SINST,  ICOM},
    {"mov",  SINST,  IMOV},
    {"add",  SINST,  IADD},
    {"sub",  SINST,  ISUB},
    {"cmp",  SINST,  ICMP},
    {"and",  SINST,  IAND},
    {"xor",  SINST,  IXOR},
    {"inc",  SINST,  IINC},
    {"dec",  SINST,  IDEC},
    {"neg",  SINST,  INEG},
    {"adc",  SINST,  IADC},
    {"gswd", SINST,  IGSWD},
    {"nop",  SINST,  INOP},
    {"sin",  SINST,  ISIN},
    {"rswd", SINST,  IRSWD},
    {"swap", SINST,  ISWAP},
    {"sll",  SINST,  ISLL},
    {"rlc",  SINST,  IRLC},
    {"sllc", SINST,  ISLLC},
    {"slr",  SINST,  ISLR},
    {"sar",  SINST,  ISAR},
    {"rrc",  SINST,  IRRC},
    {"sarc", SINST,  ISARC},
    {"halt", SINST,  IHALT},
    {"eis",  SINST,  IEIS},
    {"dis",  SINST,  IDIS},
    {"tci",  SINST,  ITCI},
    {"clrc", SINST,  ICLRC},
    {"setc", SINST,  ISETC},
    {"jmp",  SINST,  IJMP},
    {"j",    SINST,  IJ},
    {"je",   SINST,  IJE},
    {"jd",   SINST,  IJD},
    {"jsr",  SINST,  IJSR},
    {"jsre", SINST,  IJSRE},
    {"jsrd", SINST,  IJSRD},
    {"b",    SINST,  IB},
    {"nop2", SINST,  INOP2},
    {"bc",   SINST,  IBC},
    {"bnc",  SINST,  IBNC},
    {"bov",  SINST,  IBOV},
    {"bnov", SINST,  IBNOV},
    {"bze",  SINST,  IBZE},
    {"bnze", SINST,  IBNZE},
    {"blge", SINST,  IBLGE},
    {"bllt", SINST,  IBLLT},
    {"bpl",  SINST,  IBPL},
    {"bmi",  SINST,  IBMI},
    {"beq",  SINST,  IBEQ},
    {"bneq", SINST,  IBNEQ},
    {"blt",  SINST,  IBLT},
    {"bge",  SINST,  IBGE},
    {"ble",  SINST,  IBLE},
    {"bgt",  SINST,  IBGT},
    {"busc", SINST,  IBUSC},
    {"besc", SINST,  IBESC},
    {"bext", SINST,  IBEXT},
    {"r0",   SREG,   0},
    {"r1",   SREG,   1},
    {"r2",   SREG,   2},
    {"r3",   SREG,   3},
    {"r4",   SREG,   4},
    {"r5",   SREG,   5},
    {"r6",   SREG,   6},
    {"r7",   SREG,   7},
    {"sp",   SREG,   6},
    {"pc",   SREG,   7},
    {".hex", SDIRECT,    0},
    {".octal",   SDIRECT,    0},
    {".ascii",   SDIRECT,    0},
    {".asciz",   SDIRECT,    0},
    {".wordb",   SDIRECT,    0},
    {".word",    SDIRECT,    0},
    {".byte",    SDIRECT,    0},
    {".blk", SDIRECT,    0},
    {".incld",   SDIRECT,    0},
    {0,0,0}
};

char mapchr[]={
EOFF,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
GARBG,	SPACE,	NEWLN,	GARBG,	NEWLN,	GARBG,	GARBG,	GARBG,
GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,	GARBG,
SPACE,	EXCLA,	DQUOTE,	SHARP,	DOL,	PCNT,	AND,	SQUOTE,
LPARN,	RPARN,	STAR,	PLUS,	COMMA,	MINUS,	LETTER,	SLASH,
DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,
DIGIT,	DIGIT,	COLON,	SEMI,	LESS,	EQUAL,	GREAT,	QUES,
AT,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LBRACK,	BSLASH,	RBRACK,	UPARR,	UNDER,
GRAVE,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,	LETTER,
LETTER,	LETTER,	LETTER,	LCURL,	VBAR,	RCURL,	TILDE,	GARBG
};

uintptr_t pAddr = 0;
void orig_main(ac,av)
int ac;
char **av;
{
	int option_index = 0;
    static struct option long_options[] = {
        {"help",             no_argument,       0,  'h' },
        {"output-directory", required_argument, 0,  'O' },
        {"no-second-pass",   no_argument,       0,  '2' },
        {0,                  0,                 0,   0  }
    };
    int oc=getopt_long(ac, av, "hO:2", long_options, &option_index);
    if(oc == 'h'){
    	fprintf(stderr, "USAGE: %s\n", \
		"[-h|--help]   [-O DIR/|--output-directory DIR/]   [-2|--no-second-pass]\n" \
		"	-h       Help: -h (short form) or --help (long form):"
		"Print a \"Usage\" message, ignore any other arguments, and perform no other processing.\n" \
		"   -O       Output: -O DIR/ (short form) or --output-directory DIR/ (long form):\n"\
		"Specify the /-terminated pathname of a directory into which the output binary is to be placed.\n" \
		"   -2       NO Second: -2 (short form) or --no-second-pass (long form):\n"\
		"Perform only first-pass processing and skip the second pass.Errors should be reported, but no output produced.\n" \
		); \
		exit(0);
	}
	pAddr=(uintptr_t)(&dhex);
	pAddr=pAddr>>32<<32;
	psym[69].p_val=(uintptr_t)(&dhex);
	psym[70].p_val=(uintptr_t)(&doctal);
	psym[71].p_val=(uintptr_t)(&dascii);
	psym[72].p_val=(uintptr_t)(&dasciz);
	psym[73].p_val=(uintptr_t)(&dwordb);
	psym[74].p_val=(uintptr_t)(&dword);
	psym[75].p_val=(uintptr_t)(&dbyte);
	psym[76].p_val=(uintptr_t)(&dblk);
	psym[77].p_val=(uintptr_t)(&dincld);

	int i;
	char **a;
	i = 2;
	av++;
	a = av;
	//////////////////////////
	//putenv("ASM_INPUT_PATH=tests/rsrc/");
	////////////////////////////
	int ai=0;
	int haveArg = 0;
	while(a[ai] != NULL && *a[ai] != 0){
		int aj=0;
		while(*(a[ai]+aj) != 0){
			aj++;
		}
		if((*(a[ai]+aj-1) == 54) && (*(a[ai]+aj-2) == 49)){
			haveArg=1;
			break;
		}
		ai++;
	}
	if(!haveArg){
		exit(1);
	}
	syminit();
	lp = lbuf;
	*lp = -1;
	char* genv=getenv("ASM_INPUT_PATH");
	char f[32];
	int gj=0;
	FILE *temp;
	while(--i) {
		char* af=a[ai];
		int dif = 1;
		while(dif && genv!=NULL){
			dif=0;
			while(!((*(genv+gj)>64 && *(genv+gj)<91) || (*(genv+gj)>96 && *(genv+gj)<123))){
				gj++;
			}

			int fi=0;
			int fj=0;
			while(af[fj]!=0){
				fj++;
				fi++;
			}
			fj=gj;
			while(*(genv+fj)!=47 && *(genv+fj)!=0){
				fj++;
				fi++;
			}
			if(*(genv+fj+1)!=0){
				dif=1;
			}
			fi=0;
			fj=gj;
			while(*(genv+fj)!=47 && *(genv+fj)!=0){
				*(f+fi)=*(genv+fj);
				fj++;
				fi++;
				gj++;
			}
			*(f+fi)='/';
			fi++;
			fj=0;
			while(af[fj]!=0){
				*(f+fi)=af[fj];
				fj++;
				fi++;
			}
			*(f+fi)=0;
			a[ai]=f;
			if ((temp = fopen(a[ai], "r")) != NULL) {
				fclose(temp);
				break;
			}
		}
		lineno = 0;
		fname = a[ai];
		if ((ibuf0 = fopen(a[ai], "r")) == NULL) {
			nerr++;
			fprintf(stderr,"Can't open %s\n",a[ai]);
			a++;
			continue;
		}
		setjmp(env);
		while(line() == 0) iflush(0);
		iflush(0);
		fclose(ibuf0);
		a++;
	}
	if(undefined() || nerr) exit(1);
	pass1++;
	DOT = 0;
	nowds = 2;
	oaddr = 0;
	i = 2;
	a = av;
	lp = lbuf;
	*lp = -1;
	anyhalf = 0;
	if(oc == 'O'){
		char oString[32];
		int oi=0;
		while(*(optarg+oi) != 0){
			oString[oi]=*(optarg+oi);
			oi++;
		}

		int MAXSIZE = 0xFFF;
	    char proclnk[0xFFF];
	    char filename[0xFFF];
	    int fno;
	    ssize_t r;
		int fi=0;
		fno = fileno(stdout);
        sprintf(proclnk, "/proc/self/fd/%d", fno);
        r = readlink(proclnk, filename, MAXSIZE);
        if (r >= 0){
            filename[r] = '\0';
        	while(*(filename+fi)!=46 && *(filename+fi)!=0){
        		fi++;
        	}
        	while(*(filename+fi)!=47 && *(filename+fi)!=0){
        		fi--;
        	}
        	fi++;
        	while(*(filename+fi)!=46 && *(filename+fi)!=0){
        		oString[oi]=*(filename+fi);
				oi++;
				fi++;
        	}
        }

        fi=0;
		while(*(fname+fi) != 47){
			fi++;
		}
		while(*(fname+fi) != 0){
			oString[oi]=*(fname+fi);
			oi++;
			fi++;
		}
		oString[oi]=0;
		bopen(oString);
		bflush();
	}
	if(oc != '2'){
		while(--i) {
			lineno = 0;
			//fname = a[0];
			ibuf0 = fopen(fname, "r");
			a++;
			setjmp(env);
			while(line() == 0) iflush(0);
			iflush(0);
			fclose(ibuf0);
		}
		prsym();
		fflush(stdout);
	}
	fflush(stderr);
	exit(0);
}

int token()
{
	if(ispt) {
		ispt = 0;
		ctoke = peekt;
		return(peekt);
	}
	ctoke = rtoken();
	return(ctoke);
}

int peektoke()
{
	if(ispt) return(peekt);
	ispt = 1;
	peekt = rtoken();
	return(peekt);
}

void untoke()
{
	ispt = 1;
	peekt = ctoke;
}

int rtoken()
{
	char c;
	char tbuf[9], *tp = NULL;
loop:
	switch(mapchr[(int)(c = getch())]) {

	case SPACE:
		goto loop;
	case NEWLN:
		return(TNEWLN);
	case EOFF:
		return(TEOFF);
	case EXCLA:
		return(TEXCLA);
	case DQUOTE:
		return(TDQUO);
	case SHARP:
		return(TSHARP);
	case DOL:
		return(TDOL);
	case PCNT:
		return(TPCNT);
	case AND:
		return(TAND);
	case SQUOTE:
		if(mapchr[(int)(c = getch())] == EOFF) error("x");
		cval = c;
		return(TCONST);
	case LPARN:
		return(TLPARN);
	case RPARN:
		return(TRPARN);
	case STAR:
		return(TSTAR);
	case PLUS:
		return(TPLUS);
	case COMMA:
		return(TCOMMA);
	case MINUS:
		return(TMINUS);
	case SLASH:
		return(TSLASH);
	case DIGIT:
		num(c);
		return(TCONST);
	case COLON:
		return(TCOLON);
	case SEMI:
		while(mapchr[(int)(c = getch())] != NEWLN) {
			if(mapchr[(int)c] == EOFF) error("x");
		}
		return(TNEWLN);
	case LESS:
		return(TLESS);
	case EQUAL:
		return(TEQUAL);
	case GREAT:
		return(TGREAT);
	case QUES:
		return(TQUES);
	case AT:
		return(TAT);
	case LETTER:
		tp=tbuf;
		*tp++ = c;
		while(((c = mapchr[(int)peekch()]) == LETTER) || (c == DIGIT)) {
			c = getch();
			if(tp == &tbuf[8]) *tp = 0;
			else *tp++ = c;
		}
		*tp = 0;
		lookup(tbuf);
		return(TIDENT);
	case LBRACK:
		return(TLBRAK);
	case BSLASH:
		return(TBSLA);
	case RBRACK:
		return(TRBRAK);
	case UPARR:
		return(TUPARR);
	case UNDER:
		return(TUNDER);
	case GRAVE:
		return(TGRAVE);
	case LCURL:
		return(TLCURL);
	case VBAR:
		return(TVBAR);
	case RCURL:
		return(TRCURL);
	case TILDE:
		return(TTILDE);
	case GARBG:
		error("g");
	}
	//////////////////////////
	exit(1);
}

char getch()
{
	if(ispc) {
		ispc = 0;
		return(peekc);
	}
	else return(rgetc());
}

char peekch()
{
	if(ispc) {
		return(peekc);
	}
	else {
		ispc = 1;
		return(peekc = rgetc());
	}
}

char rgetc()
{
	char c;
	if(*lp != -1) return(*lp++);
	else {
		lineno++;
		lp = lbuf;
		isline++;
		do {
			if(lp == &lbuf[LSIZE-1]) {
				lp = lbuf;
				error("l");
			}
			c = getc(wchbuf?ibuf1:ibuf0);
			if(c < 0){//123
				if(wchbuf) {
					fclose(ibuf1);
					wchbuf = 0;
					c = getc(ibuf0);
					if(c < 0) c = 0;
				} else c = 0;
			}
			*lp++ = c;
		} while((c != 0) && (c != '\n'));
		*lp = -1;
		lp = lbuf;
		//////////////////////////////
		/////////////////////////////
		////////////////////////////
		//fprintf(stderr,"[%s]", lp);
		return(*lp++);
	}
}

void lflush(f)
FILE *f;
{
	char *rlp;
	isline = 0;
	rlp = lbuf;
	//fprintf(f, "|%d", *rlp);
	//fprintf(f, "|%s\n", lbuf);
	while(*rlp != -1 && *rlp != 0) fprintf(f, "%c", *rlp++);
}

void lookup(st)
char *st;
{
	struct symtab *sp,*ssp;
	int hash,hsh;
	char *cp;
	hash = 0;
	cp = st;
	while(*cp) hash += *cp++;
	hsh = hash%HSHSIZ;
	if(hshtab[hsh] == 0) {
		if(++nsym >= NSYM) error("o");
		sp = &symtab[nsym-1];
		hshtab[hsh] = sp;
		goto gotone;
	}
	sp = hshtab[hsh];
	do {
		ssp = sp;
		if(hash != sp->s_hash) {
			continue;
		}
		if(eqs(st,sp->s_name)) {
			csym = sp;
			return;
		}
	} while((sp = sp->s_link));
	if(++nsym >= NSYM) error("o");
	sp = &symtab[nsym-1];
	ssp->s_link = sp;
gotone:	sp->s_link = 0;
	cp = sp->s_name;
	while(*st) *cp++ = *st++;
	csym = sp;
	sp->s_flags |= SIDENT;
	sp->s_hash = hash;
	return;
}

int eqs(s1, s2)
char *s1,*s2;
{
	while(*s1 == *s2++) if(*s1++ == 0) return(1);
	return(0);
}

int line()
{
	struct symtab *sp;
	int (*dirp)();
	insaddr = DOT;
loop:	switch(token()) {

	case TIDENT:
		sp = csym;
		if(sp->s_flags & SIDENT) {
			switch(peektoke()) {

			case TCOLON:
				token();
				label();
				goto loop;
			case TEQUAL:
				token();
				assign();
				return(0);
			default:
				//fprintf(stderr, "(%s, %o, %d)\n", sp->s_name, sp->s_val, sp->s_flags);
				cexpr();
				return(0);
			}
		} else if(sp->s_flags & SDIRECT) {
			dirp = (int(*)())(((uintptr_t)(sp->s_val)&4294967295)+pAddr);
			(*dirp)();
			return(0);
		} else if(sp->s_flags & SINST) {
			cinstr();
			return(0);
		}
	case TCONST:
	case TMINUS:
		cexpr();
		return(0);
	case TEOFF:
		return(1);
	case TNEWLN:
		return(0);
	default:
		untoke();
		error("g");
	}
	////////////////////////
	exit(1);
}

void label()
{
	struct symtab *sp;
	sp = csym;
	//////////////////////////////////
	if((!pass1) & (sp->s_flags & SDEF)) error("m");
	else {
		sp->s_flags |= SDEF;
		sp->s_val = DOT;
	}
}

void assign()
{
	struct symtab *sp;
	sp = csym;
	sp->s_val = expr(0,0);
	//printf("[%s-%d]", csym->s_name, sp->s_val);
	sp->s_flags |= SDEF;
	if(sp->s_flags & SDOT) bflush();
	newline(0);
}

void cexpr()
{
	putwd(expr(1,0));
	newline(0);
}

int expr(t,lev)
int t,lev;
{
	int toke;
	struct symtab *sp;
	int arg, val, nargs, nops, op;
	nargs = nops = 0;
	if(t == 0) toke = token();
	else toke = ctoke;
loop:
	switch(toke) {

	case TLPARN:
		val = expr(0,1);
		if(token() != TRPARN) error("e");
		goto caseident;
	case TCONST:
		val = cval;
		goto caseident;
	case TIDENT:
		sp = csym;
		if((sp->s_flags & SIDENT) == 0)
			error("e");
		val = sp->s_val;
caseident:	if(nops == 0) {
			if(nargs != 0)
				error("e");
			arg = val;
			nargs++;
			toke = token();
			goto loop;
		}
		if(nargs == 0) {
			switch(op) {

			case TMINUS:
				arg = -val;//123
				break;
			case TTILDE:
				arg = ~val;
				break;
			default:
				error("e");
			}
		} else {
			switch(op) {

			case TPLUS:
				arg = arg+val;
				break;

			case TMINUS:
				arg = arg-val;
				break;

			case TSTAR:
				arg = arg*val;
				break;

			case TSLASH:
				arg = arg/val;
				break;

			case TPCNT:
				arg = arg%val;
				break;

			case TAND:
				arg = arg&val;
				break;

			case TVBAR:
				arg = arg|val;
				break;

			case TGREAT:
				arg = arg>>val;
				break;

			case TLESS:
				arg = arg<<val;
				break;

			default:
				error("e");
			}
		}
		nops = 0;
		nargs = 1;
		toke = token();
		goto loop;
	case TMINUS:
	case TTILDE:
	case TPLUS:
	case TSTAR:
	case TSLASH:
	case TPCNT:
	case TAND:
	case TVBAR:
	case TGREAT:
	case TLESS:
		if(nops != 0) error("e");
		if(nargs == 0) {
			if((toke != TTILDE) && (toke != TMINUS)) error("e");
		}
		op = toke;
		nops++;
		toke = token();
		goto loop;
	default:
		if((nops != 0) || (nargs == 0))
			error("e");
		untoke();
		return(arg);
	}
}

void num(ch)
char ch;
{
	int val;
	char buf[16]="";
	char *bp;
	int radix;
	radix = 8;
	bp = buf;
	*bp++ = ch;
	//fprintf(stderr, "[%d]", *bp-1);
	while(mapchr[(int)peekch()] == DIGIT) {
		if(bp == &buf[15]) error("c");
		*bp++ = getch();
		//fprintf(stderr, "[%d]", *bp-1);
	}
	char c;
	if((c=peekch()) == '.') {
		getch();
		radix = 10;
	}
	//fprintf(stderr, "[%c]", c);

	bp = buf;
	val = 0;
	while(*bp) {//123
		//fprintf(stderr, "(%d)", *bp);
		val *= radix;
		val += *bp++ - '0';
		//fprintf(stderr, "(%o)", val);
	}
	cval = val;
}

void error(s)
char *s;
{
	nerr++;
	fprintf(stderr,"%s:%d: *** %s\n",fname,lineno,s);
	newline(1);
	iflush(1);
	longjmp(env, 1);
}

void prsym()
{
	struct symtab *sp;
	int i=0,j;
	sortsym();
	printf("\n\nSymbol Table\n\n");
	j = 0;
	while(j < nsym) {
		sp = sorteds[j];
		if(((sp->s_flags & SIDENT) == 0) || (sp->s_flags & SDOT)) {
			j++;
			continue;
		}
		printf("%-8s =\t", sp->s_name);
		prtwd(sp->s_val);
		if(!(sp->s_flags & SDEF)) putchar('U');
		putchar('\t');
		if((++i % 3) == 0) putchar('\n');
		j++;
	}
	putchar('\n');
}

void syminit()
{
	struct symtab *sp;
	struct psym *pp;
	lookup(".");
	sp = csym;
	sp->s_flags |= SDOT;
	dot = &(sp->s_val);
	pp = &psym[0];
	while(pp->p_name) {
		lookup(pp->p_name);
		sp = csym;
		sp->s_flags = pp->p_flags;
		sp->s_val = pp->p_val;
		pp++;
	}
}

void putwd(w)
int w;
{
	if(anyhalf) error("h");
	obuf[nowds++] = w;
	odata = w;
	nodata++;
	DOT++;
	if(nowds == 18) bflush();
	iflush(0);
}

void iflush(a)
int a;
{
	if(pass1 == 0) {
		if(a) {
			lflush(stderr);
		}
		nodata = 0;
		return;
	}
	if((nodata == 0) && (isline == 0))
		return;
	if(insaddr != lastaddr) {
		lastaddr = insaddr;
		prtwd(insaddr);
	}
	putchar('\t');
	if(nodata == 0) {
		putchar('\t');
		lflush(stdout);
	} else {
		prtwd(odata);
		putchar('\t');
		if(isline) lflush(stdout);
		else putchar('\n');
		nodata = 0;
	}
}

void bflush()
{
	int i,chksum;
	if((nowds == 2) || (pass1 == 0)) {
		nowds = 2;
		oaddr = DOT;
		return;
	}
	obuf[0] = oaddr;
	obuf[1] = nowds-1;
	chksum = 0;
	for(i = 0; i < nowds; i++) chksum += obuf[i];
	obuf[nowds] = -chksum;//123
	write(ofile,obuf,(nowds+1)*2);
	oaddr = DOT;
	nowds = 2;
}

void puthex(n)
int n;
{
	int count;
	int i;
	count = 4;
	while(count--) {
		i = ((n & 0170000) >> 12);
		i &= 017;
		if(i > 9) putchar(i - 10 + 'A');
		else putchar(i + '0');
		n <<= 4;
	}
}

void prtwd(n)
int n;
{
	if(oradix == 16) puthex(n);
	else putoct(n);
}

void putoct(n)
int n;
{
	int count;
	count = 5;
	putchar(n > 32767 ? '1' : '0');
	while(count--) {
		putchar(((n & 070000) >> 12) + '0');
		n <<= 3;
	}
}
//no
void cinstr()
{
	struct symtab *sp;
	sp = csym;
	getops();
	if(ninsops == 0) ins0(sp);
	else if(ninsops == 1) ins1(sp);
	else if(ninsops == 2) ins2(sp);
	else error("i");
	newline(0);
}

void getops()
{
	struct insops *ip;
	struct symtab *sp;
	ninsops = 0;
loop:
	ip = &insops[ninsops > 1 ? 1 : ninsops];
	switch(token()) {

	case TIDENT:
		sp = csym;
		if(sp->s_flags & SREG) {
			ip->i_mode = REGMOD;
			ip->i_val = sp->s_val;
			break;
		}
		ip->i_mode = IMMMOD;
		ip->i_val = expr(1,0);
		break;
	case TAT:
		switch(token()) {

		case TIDENT:
			sp = csym;
			if(sp->s_flags & SREG) {
				ip->i_mode = DEFREG;
				ip->i_val = sp->s_val;
				if(peektoke() == TPLUS) {
					token();
					if((ip->i_val < 4) || (ip->i_val > 6))
						error("r");
				}
				break;
			}
			ip->i_mode = DEFIMM;
			ip->i_val = expr(1,0);
			break;
		case TMINUS:
			token();
			sp = csym;
			if((ctoke == TCONST) || ((ctoke == TIDENT) && (sp->s_flags != SREG)))
				goto caseconst;
			ip->i_mode = DEFREG;
			ip->i_val = sp->s_val;
			if(ip->i_val != 6) error("r");
			break;
		case TCONST:
		case TTILDE:
caseconst:		ip->i_mode = DEFIMM;
			ip->i_val = expr(1,0);
			break;
		default:
			untoke();
			error("i");
		}
		break;
	case TCONST:
	case TMINUS:
	case TTILDE:
		ip->i_mode = IMMMOD;
		ip->i_val = expr(1,0);
		break;
	case TNEWLN:
		untoke();
		return;
	default:
		error("i");
	}
	ninsops++;
	switch(peektoke()) {

	case TCOMMA:
		token();
		goto loop;
	case TNEWLN:
		return;
	default:
		error("i");
	}
}

void ins0(sp)
struct symtab *sp;
{
	if((sp->s_val == INOP) || (sp->s_val == ISIN)) {
		putwd(064 | (sp->s_val == INOP ? 0 : 02));
		return;
	} else
	if(sp->s_val > 7) error("i");
	putwd(sp->s_val&07);
}


void ins1(sp)
struct symtab *sp;
{
	int v;
	int addr;
	v = sp->s_val;
	if(v < 8) error("i");
	else if((v >= 16) && (v <= 19)) jump1(v);
	else if((v >= 32) && (v <= 47)) branch(v);
	else if((v >= 73) && (v <= 79)) single(v);
	else if((v == ITST) || (v == ICLR)) {
		if((insops[0].i_mode & REGMOD) == 0)
			error("i");
		addr = insops[0].i_val;
		addr = (addr << 3) | addr;
		if(v == ITST) putwd(0200 | addr);
		else putwd(0700 | addr);
		return;
	}
	else if(v == IJMP) {
		if((insops[0].i_mode & REGMOD) == 0) {
			branch(IB);
			return;
		}
		addr = insops[0].i_val;
		putwd(0200 | (addr << 3) | 07);
	}
	else error("i");
}
void ins2(sp)
struct symtab *sp;
{
	int v;
	v = sp->s_val;
	if((v >= 8) && (v <= 15)) shift(v);
	else if((v >= 66) && (v <= 71)) dbl(v);
	else if(v == IBEXT) bext();
	else if((v >= 20) && (v <= 22)) jsr(v);
	else error("i");
}

void jump1(v)
int v;
{
	int addr;
	if(insops[0].i_mode != IMMMOD) error("i");
	addr = insops[0].i_val;
	putwd(04);
	putwd(01400 | (((addr & 0176000) >> 8)
			& 0377) | (v & 03));
	putwd(addr & 01777);
}

void branch(int v)
{
	int pc,addr,sign;
	pc = DOT+2;
	if(insops[0].i_mode != IMMMOD) error("i");
	addr = insops[0].i_val;
	sign = addr > pc ? 0 : 040;
	putwd(01000 | sign | (v & 017));
	//fprintf(stderr,"(%d)", addr); //123
	if(sign)
		putwd(~(addr - pc));//123
	else
		putwd(addr - pc);
}
void single(v)
int v;
{
	int mode, reg;
	reg = insops[0].i_val;
	mode = insops[0].i_mode;
	if(mode == REGMOD) {
		switch(v) {

		case IINC:
		case IDEC:
		case ICOM:
		case INEG:
		case IADC:
			putwd(((v & 07) << 3) | reg);
			return;
		case IGSWD:
			if(reg > 3) error("r");
			putwd(060 | reg);
			return;
		case IRSWD:
			putwd(070 | reg);
			return;
		default:
			error("i");
		}
	} else error("i");
}

void newline(a)
int a;
{
	char c;
	if((!a) && (peektoke() != TNEWLN))
		error("n");
	if(ispt && (peekt == TNEWLN)) {
		token();
		return;
	}
	while((mapchr[(int)(c = getch())] != NEWLN) && (mapchr[(int)c] != EOFF));
	return;
}

void shift(v)
int v;
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != IMMMOD) || (val1 <= 0) || (val1 >= 3))
		error("i");
	if((mode2 != REGMOD) || (val2 > 3))
		error("i");
	putwd(0100 | ((v & 07) << 3) | ((val1 - 1) << 2) | val2);
}

void dbl(v)
int v;
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 == REGMOD) && (mode2 == REGMOD)) {
		putwd(((v & 07) << 6) | (val1 << 3) | val2);
		return;
	}
	if(mode1 == REGMOD) {
		if(v != IMOV) error("i");
		if(mode2 == IMMMOD) mode1 = 7;
		else if(mode2 == DEFIMM) mode1 = 0;
		else {
			if((val2 == 7) || (val2 == 0))
				error("r");
			mode1 = val2;
		}
		putwd(01100 | (mode1 << 3) | val1);
		if((mode1 == 0) || (mode1 == 7))
			putwd(val2);
		return;
	}
	if(mode2 != REGMOD) error("i");
	if(mode1 == IMMMOD) mode2 = 7;
	else if(mode1 == DEFIMM) mode2 = 0;
	else {
		if((val1 == 0) || (val1 == 7))
			error("r");
		mode2 = val1;
	}
	putwd(01000 | ((v & 07) << 6) | (mode2 << 3) | val2);
	if((mode2 == 0) || (mode2 == 7))
		putwd(val1);
}

void bext()
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != IMMMOD) || (val1 < 0) || (val1 > 15) || (mode2 != IMMMOD))
		error("i");
	mode1 = DOT+2;
	mode2 = val2 > mode1 ? 0 : 040;
	putwd(01020 | mode2 | val1);
	if(mode2)
		putwd(~(val2 - mode1));
	else
		putwd(val2 - mode1);
}

void jsr(v)
int v;
{
	int mode1, val1, mode2, val2;
	mode1 = insops[0].i_mode;
	val1 = insops[0].i_val;
	mode2 = insops[1].i_mode;
	val2 = insops[1].i_val;
	if((mode1 != REGMOD) || (mode2 != IMMMOD) || (val1 < 4) || (val1 == 7))
		error("i");
	putwd(04);
	////////////////////////////////////////123
	putwd((((val1 - 4) << 8) | (((val2 & 0176000) >> 8) & 0377)) | (v & 03));
	putwd(val2 & 01777);
}

int undefined()
{
	struct symtab *sp;
	int anyund=0;
	sp = &symtab[0];
	while(sp < &symtab[nsym]) {
		if((sp->s_flags & SIDENT) &&
		  !(sp->s_flags & SDOT) &&
		  !(sp->s_flags & SDEF)) {
			if(!anyund) fprintf(stderr,"Undefined Symbols\n");
			fprintf(stderr,"%-8s\n",sp->s_name);
			anyund++;
		}
		sp++;
	}
	//fprintf(stderr, "%d",anyund);
	return(anyund);
}

void dhex()
{
	oradix = 16;
	newline(0);
}

void doctal()
{
	oradix = 8;
	newline(0);
}

void bopen(s)
char *s;
{
	char st[64]="";
	char *sp;
	sp = st;
	while(((*sp++ = *s) != '.') && *s) s++;
	if(!*s) *sp++ = '.';
	*sp++ = 'b';
	*sp++ = 'i';
	*sp++ = 'n';
	*sp++ = 0;
	ofile = creat(st,0666);
}

void dascii()
{
	char c;
	char sep;
	do {
		c = mapchr[(int)(sep = peekch())];
		if((c == NEWLN) || (c == EOFF))
			error("d");
		getch();
	} while(c == SPACE);
	while((c = getch()) != sep) {
		if((mapchr[(int)c] == NEWLN) || (mapchr[(int)c] == EOFF))
			error("d");
		if(anyhalf) {
			halfw.lobyte = c;
			anyhalf = 0;
			putwd((halfw.hibyte<<8) + halfw.lobyte);
		} else {
			halfw.hibyte = c;
			anyhalf++;
		}
	}
	newline(0);
}

void dasciz()
{
	dascii();
	if(anyhalf) {
		halfw.lobyte = 0;
		anyhalf = 0;
		putwd((halfw.hibyte<<8) + halfw.lobyte);
	} else {
		halfw.hibyte = 0;
		anyhalf++;
	}
}

void dwordb()
{
	rwordb();
	newline(0);
}

void dword()
{
	int toke;
	do {
		putwd(expr(0,0));
		toke = token();
	} while(toke == TCOMMA);
	untoke();
	newline(0);
}

void dbyte()
{
	int toke;
	do {
		toke = expr(0,0) & 0377;
		if(anyhalf) {
			halfw.lobyte = toke;
			anyhalf = 0;
			putwd((halfw.hibyte<<8) + halfw.lobyte);
		} else {
			halfw.hibyte = toke;
			anyhalf++;
		}
		toke = token();
	} while(toke == TCOMMA);
	untoke();
	newline(0);
}

void dblk()
{
	int n;
	n = expr(0,0);
	newline(0);
	while(n--) putwd(0);
}

void rwordb()
{
	if(anyhalf) {
		halfw.lobyte = 0;
		anyhalf = 0;
		putwd((halfw.hibyte<<8) + halfw.lobyte);//123
	}
}

void sortsym()
{
	register int i,j;
	for(i = 0; i < nsym; i++){
		sorteds[i] = &symtab[i];
	}
	for(i = 0; i < nsym-1; i++)
		for(j = i+1; j < nsym; j++) {
			if(less(sorteds[j]->s_name,
				sorteds[i]->s_name))
				exchg(i,j);
			}
}

int less(s1,s2)
char *s1,*s2;
{
	register char *rs1, *rs2;
	rs1 = s1;
	rs2 = s2;
	while(*rs1 == *rs2) {
		if(*rs1++ == 0) return(0);
		rs2++;
	}
	return(*rs1 < *rs2 ? 1 : 0);
}

void exchg(i,j)
int i,j;
{
	struct symtab *sp;
	struct symtab **si,**sj;
	si = &sorteds[i];
	sj = &sorteds[j];
	sp = *si;
	*si = *sj;
	*sj = sp;
}

void dincld()
{
	char fnbuf[32], *fn, c;
	do {
		c = getch();
	} while(mapchr[(int)c] == SPACE);
	fn = fnbuf;
	while(mapchr[(int)c] != EOFF && mapchr[(int)c] != NEWLN) {
		*fn++ = c;
		c = getch();
	}
	*fn = 0;
	char* genv=getenv("ASM_INPUT_PATH");
	char f[32];
	char* af=fnbuf;
	int gj=0;
	int dif = 1;
	FILE *temp;
	while(dif && genv!=NULL){
		dif=0;
		while(!((*(genv+gj)>64 && *(genv+gj)<91) || (*(genv+gj)>96 && *(genv+gj)<123))){
			gj++;
		}

		int fi=0;
		int fj=0;
		while(af[fj]!=0){
			fj++;
			fi++;
		}
		fj=gj;
		while(*(genv+fj)!=47 && *(genv+fj)!=0){
			fj++;
			fi++;
		}
		if(*(genv+fj+1)!=0){
			dif=1;
		}
		fi=0;
		fj=gj;
		while(*(genv+fj)!=47 && *(genv+fj)!=0){
			*(f+fi)=*(genv+fj);
			fj++;
			fi++;
			gj++;
		}
		*(f+fi)='/';
		fi++;
		fj=0;
		while(af[fj]!=0){
			*(f+fi)=af[fj];
			fj++;
			fi++;
		}
		*(f+fi)=0;
		if ((temp=fopen(f, "r")) != NULL) {
			fclose(temp);
			break;
		}
	}
	if(genv!=NULL){
		int fi=0;
		while(f[fi] != 0){
			fnbuf[fi]=f[fi];
			fi++;
		}
		fnbuf[fi]=0;
	}
	if((ibuf1 = fopen(fnbuf, "r")) == NULL) {
		nerr++;
		error("Can't include");
		return;
	}
	wchbuf = 1;
}
