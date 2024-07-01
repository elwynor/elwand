/*****************************************************************************
 *                                                                           *
 *   ANDROIDS.C                                                              *
 *                                                                           *
 *   Copyright (c) 1989-1995 GALACTICOMM, Inc.    All Rights Reserved.       *
 *                                                                           *
 *   This is a little robot action game, to demonstrate a few of the         *
 *   possibilities inherent in The Major BBS.                                *
 *                                                                           *
 *                                            - T. Stryker 2/7/88            *
 *                                                                           *
 *   Conversion from WG3 DOS to BBSV10        - R. Hadsall 5/1/23            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as published  *
 * by the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *                                                                           *
 * Additional Terms for Contributors:                                        *
 * 1. By contributing to this project, you agree to assign all right, title, *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies.                              *
 * 2. You grant Rick Hadsall and Elwynor Technologies a non-exclusive,       *
 *    royalty-free, worldwide license to use, reproduce, prepare derivative  *
 *    works of, publicly display, publicly perform, sublicense, and          *
 *    distribute your contributions                                          *
 * 3. You represent that you have the legal right to make your contributions *
 *    and that the contributions do not infringe any third-party rights.     *
 * 4. Rick Hadsall and Elwynor Technologies are not obligated to incorporate *
 *    any contributions into the project.                                    *
 * 5. This project is licensed under the AGPL v3, and any derivative works   *
 *    must also be licensed under the AGPL v3.                               *
 * 6. If you create an entirely new project (a fork) based on this work, it  *
 *    must also be licensed under the AGPL v3, you assign all right, title,  *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies, and you must include these   *
 *    additional terms in your project's LICENSE file(s).                    *
 *                                                                           *
 * By contributing to this project, you agree to these terms.                *
 *                                                                           *
 *****************************************************************************/

#include "gcomm.h"
#include "brkthu.h"
#include "majorbbs.h"
#include "elwand.h"

#define FILREV "$Revision: 1.4 $"
#define VERSION "1.5"



typedef
struct droid {                     /* per-player dynamic info              */
    INT row, col;                  /*   coordinates of android             */
    LONG points;                  /*   how many points so far             */
    LONG ptsdsp;                  /*   points most recently displayed     */
    LONG secs;                    /*   how long so far in seconds         */
    INT ptsrow;                   /*   display row-number of points       */
    CHAR chr;                     /*   display symbol                     */
    CHAR scodun;                  /*   flag - score done in last UPDIVL   */
    CHAR needrc;                  /*   flag - need to restore cursor      */
} droid;

static droid *drolst, *droptr;     /* dynam alloc'd array of droids, & ptr */
static droid *dpafol[26];          /* ptrs to droid structs, by alpha char */

static
INT unafol[26];                    /* user numbers as function of letters  */

#define MAXDRO   6                 /* max number of droids in game         */
#define UPDIVL   5                 /* update interval for pts/hr, in secs  */
#define PLAYING -1                 /* substate code when actually playing  */
#define BDTSIZ   7                 /* baud rate table size                 */
#define CHCRIT 170                 /* start ignoring keys if buffer > this */

static
INT chbufd,                        /* characters buffered in slowest chan  */
    chperi;                        /* chars per 1/TPERSC interval (slowest)*/

static
struct {                           /* # of players allowed, by baud rate   */
     LONG baud;                    /*   (set from config parameters)       */
     INT allowd;
} bdrtbl[BDTSIZ]={{300,0},{1200,0},{2400,0},{4800,0},{9600,0},{19200,0},
                  {38400L,0}};

static
INT rampts,                        /* number of points transferred by ram  */
    bonpts,                        /* number of points for super-bonus prz */
    ptsded;                        /* number of points deducted for crash  */

//static
typedef 
struct t10stf {                    /* top-ten listings data                */
     CHAR userid[UIDSIZ];          /*   User-ID                            */
     USHORT date;                  /*   date of game in packed format      */
     LONG pts;                     /*   number of points, or points per hr */
} t10stf;

static t10stf ttpts[3], ttpph[3];  /* top-three pts & top-three pts/hour   */

static
CHAR *mapp,*omapp,*oomapp;         /* copies of playfield                  */
static
UINT hsize,vsize,bdsize;           /* horiz & vert playfield size, & bytes */

static
INT playrs[MAXDRO],nplyrs=0;       /* usrnum's of actual players, & count  */

#define    map(row,col)   (*(mapp+(row)*hsize+(col)))

static
CHAR firsdg[80]={                            /* 1st digit of num, 1 to 80  */
     '0','0','0','0','0','0','0','0','0','1',
     '1','1','1','1','1','1','1','1','1','2',
     '2','2','2','2','2','2','2','2','2','3',
     '3','3','3','3','3','3','3','3','3','4',
     '4','4','4','4','4','4','4','4','4','5',
     '5','5','5','5','5','5','5','5','5','6',
     '6','6','6','6','6','6','6','6','6','7',
     '7','7','7','7','7','7','7','7','7','8'
};
static
CHAR secndg[80]={                            /* 2nd digit of num, 1 to 80  */
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0',
     '1','2','3','4','5','6','7','8','9','0'
};

static
CHAR wrtchr[]={"\33[yy;xxHz"};               /* template write-char seq    */

CHAR rstrcur[] = { "\33[u" };                /* restore cursor escape code */

static
UINT psernd;                                 /* pseudo-random accumulator  */

static                   /* game rule-variable controls                    */
INT showro,                   /* show run-over prizes in new locations     */
    showam,                   /* show auto-moved prizes in new locations   */
    amshos,                   /*   auto-moved prizes, show spaces not '?'  */
    showpy,                   /* show players                              */
    pyshoq,                   /*   players, show question marks not ' '    */
    nquick,                   /* controls how violently prizes auto-move   */
    bonon,                    /* bonus on-time in 1/TPERSCths of a second  */
    bonoff,                   /* bonus off-time in 1/TPERSCths of a second */
    bonnum;                   /* number of bonuses strewn at on-time       */

static
CHAR vacpos;                  /* vacated position disp code (32 or 219)    */

static
CHAR nquicka[]={5,0,2,5,10,0,10,1,2,5,0};
static
CHAR bonona[]={50,60,70,10,30,50,70,30,70,90,90,30,30,50,50,50,50};
static
CHAR bonoffa[]={30,20,10,10,120,70,50};
static
CHAR bonnuma[]={1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,3,1,1,1,1,1,1,1,1,1,3,1,1,1};

static VOID idcata(INT which);
static VOID drotck(VOID);
static VOID dromin(VOID);
static GBOOL androi(VOID);
static VOID g2play(VOID);
static VOID prfttl(VOID);
static VOID ntrgam(VOID);
static INT finsco(VOID);
static VOID writ10(VOID);
static INT intop10(LONG pts, t10stf* ttpts);
static VOID rmvguy(INT guy);
static INT fndprw(INT ptsrow);
static INT fndlet(CHAR chr);
static VOID drosta(VOID);
static VOID go2slo(VOID);
static VOID asetup(VOID);
static VOID allsco(VOID);
static VOID in1sco(droid* dp, CHAR* userid);
static VOID rf1sco(droid* droptr);
static VOID outpys(INT uncond);
static VOID drwbrd(VOID);
static CHAR drochi(INT chan, INT ch);
static VOID drorti(VOID);
static VOID drohup(VOID);
static VOID clsdro(VOID);

static
INT drostt;                        /* Androids game playing state          */

struct module androidm = {           /* module interface block               */
     "",                           /*    name used to refer to this module */
     NULL,                         /*    user logon supplemental routine   */
     androi,                       /*    input routine if selected         */
     drosta,                       /*    status-input routine if selected  */
     NULL,                         /*    "injoth" routine for this module  */
     NULL,                         /*    user logoff supplemental routine  */
     drohup,                       /*    hangup (lost carrier) routine     */
     NULL,                         /*    midnight cleanup routine          */
     NULL,                         /*    delete-account routine            */
     clsdro                        /*    finish-up (sys shutdown) routine  */
};

static
HMCVFILE dromb;                    /* Androids message file block pointer  */


void EXPORT
init__elwand(VOID)                 /* Androids initialization function     */
{
     INT i,j;
     CHAR *mapptr,*mp,c;
     FILE *fp;

     dromb=opnmsg("elwand.mcv");
     shocst(spr("ELW Androids v%s", VERSION), "(C) Copyright 2023-2024 Elwynor Technologies - www.elwynor.com");

     stzcpy(androidm.descrp,gmdnam("ELWAND.MDF"),MNMSIZ);
     drostt=register_module(&androidm);

     rampts=numopt(RAMPTS,0,1000);
     bonpts=numopt(BONPTS,0,1000);
     ptsded=numopt(PTSDED,0,1000);
     for (i=0 ; i < BDTSIZ ; i++) {
          bdrtbl[i].allowd=numopt(BDRTB1+i,0,MAXDRO);
     }
     drolst=(struct droid *)alczer(nterms*sizeof(struct droid));
     mapptr=rawmsg(BCKGND4);
     stpans(mapptr);
     for (mp=mapptr+1 ; *mp != '\r' ; mp++) {
          if (*mp == '\0') {
               idcata(1);
          }
     }
     hsize=(INT)(mp-(mapptr+1));
     vsize=1;
     while (*++mp != '\0') {
          if (*mp == '\r') {
               vsize++;
          }
     }
     if (hsize < 5 || vsize < 5) {
          idcata(2);
     }
     mapp=alcmem(bdsize=hsize*vsize);
     omapp=alcmem(bdsize);
     oomapp=alcmem(bdsize);
     mp=mapptr+1;
     for (i=0 ; i < (INT)vsize ; i++) {
          for (j=0 ; j < (INT)hsize ; j++) {
               map(i,j)=((c=*mp++) == ' ' && (UINT)i == vsize-1 ? 219 : c);
          }
          if (*mp++ != '\r') {
               idcata(3);
          }
     }
     if (*mp != '\0') {
          idcata(4);
     }
     nplyrs=0;
     if ((fp=fopen("elwand.t10",FOPRB)) != NULL) {
          fread(ttpts,3,sizeof(struct t10stf),fp);
          fread(ttpph,3,sizeof(struct t10stf),fp);
          fclose(fp);
     }
     vsize--;
     rtihdlr(drorti);
     drotck();
     dromin();
}

static
VOID
idcata(INT which)                 /* identify catastrophe in playfield         */
{
     catastro("ELWAND: error %d in playfield",which);
}

static
VOID
drotck(VOID)                  /* once-per-second real-time checker         */
{
     INT i;
     droid *dp;
     static INT secnds=0;

     for (i=0 ; i < nplyrs ; i++) {
          drolst[playrs[i]].secs++;
     }
     if (++secnds == UPDIVL) {     /* && fdentval()) { */
          secnds=0;
          for (i=0 ; i < nplyrs ; i++) {
               dp=&drolst[playrs[i]];
               if (!dp->scodun) {
                    rf1sco(dp);
               }
               dp->scodun=0;
          }
     }
     rtkick(1,drotck);
}

static
VOID
dromin(VOID)                  /* once-per-minute game rules changer        */
{
     static INT minctr=2;

     showro=minctr%      5 ;
     showam=minctr%      3 ;
     amshos=(minctr+2)%  5 ;
     showpy=minctr%     13 ;
     pyshoq=minctr%      7 ;
     vacpos=(minctr%79 > 1 ? ' ' : 219);
     nquick=nquicka[minctr%sizeof(nquicka)];
     bonon=bonona[minctr%sizeof(bonona)];
     bonoff=bonoffa[minctr%sizeof(bonoffa)];
     bonnum=bonnuma[minctr%sizeof(bonnuma)];
     minctr++;
     rtkick(60,dromin);
}

static GBOOL
androi(VOID)                  /* Androids master <CR>-string entry point   */
{
     UINT i,ninstt,nallow,autott;
     LONG minbdr;

     setmbk(dromb);
     droptr=&drolst[usrnum];
     do {
          autott=0;
          bgncnc();
          switch (usrptr->substt) {
          case 0:
               cncchr();
               prfmsg(INTRO3,usaptr->scnwid/2-9);
               if (!(usaptr->ansifl&ANSON)) {
                    prfmsg(NOANSI);
                    return(0);
               }
               if (usaptr->scnwid < hsize+2) {
                    prfmsg(S2SMAL,hsize+2);
                    return(0);
               }
               if (usaptr->scnbrk < 24 && usaptr->scnbrk != CTNUOS) {
                    prfmsg(S2SHRT);
                    return(0);
               }
               for (i=ninstt=0,minbdr=38400L ; i < (UINT)nterms ; i++) {
                    if (usroff(i)->state == drostt) {
                         ninstt++;
                         minbdr=min(minbdr,usroff(i)->baud);
                    }
               }
               for (i=0 ; i < BDTSIZ ; i++) {
                    if (minbdr <= bdrtbl[i].baud) {
                         break;
                    }
               }
               if (ninstt > (nallow=bdrtbl[i].allowd)) {
                    if (nallow == 0) {
                         prfmsg(NOGOOD,minbdr);
                    }
                    else {
                         prfmsg(TOMANY,nallow,minbdr);
                    }
                    return(0);
               }
               chperi=(INT)(minbdr/(11*TPERSC));
               prfmsg(INSTRS);
               prfmsg(usrptr->substt=P2PLAY2);
               break;
          case P2PLAY2:
               if (margc > 0) {
                    switch (cncchr()) {
                    case 'X':
                         return(0);
                    case 'D':
                         prfmsg(DETAILS3,rampts,ptsded,bonpts);
                         break;
                    case 'T':
                         prfttl();
                         break;
                    case 'P':
                         ntrgam();
                         break;
                    default:
                         cncall();
                         prfmsg(P2PLAY2);
                    }
               }
               else {
                    prfmsg(P2PLAY2);
               }
               break;
          case PLAYING:
               autott=finsco();
               rmvguy(usrnum);
          case BCKGND4:
               g2play();
               if (autott) {
                    prfttl();
               }
               else {
                    prfmsg(JIRPTD);
               }
               cncall();
               break;
          default:
               prf("");
               cncall();
               break;
          }
     } while (!endcnc());
     outprf(usrnum);
     return(1);
}

static
VOID
g2play(VOID)                  /* go to the P2PLAY2 substate                */
{
     usrptr->flags&=~NOINJO;
     btutsw(usrnum,usaptr->scnwid);
     rstrxf();
     btuoes(usrnum,0);
     usrptr->substt=P2PLAY2;
}

static
VOID
prfttl(VOID)                  /* output top-three listings                 */
{
     INT i;
     CHAR *fmtstg;
     CHAR *ptsdat,*ptspts,*pphdat,*pphpts;

     prfmsg(TOPTHR1);
     fmtstg=rawmsg(TTFMT2);
     for (i=0 ; i < 3 ; i++) {
          if (ttpts[i].userid[0] == '\0') {
               ptsdat="";
               ptspts="";
          }
          else {
               ptsdat=spr("%s",ncedat(ttpts[i].date));
               ptspts=l2as(ttpts[i].pts);
          }
          prf(fmtstg,i+1,ttpts[i].userid,ptsdat,ptspts);
     }
     prfmsg(TOPTHR2);
     fmtstg=rawmsg(TTFMT2);
     for (i=0 ; i < 3 ; i++) {
          if (ttpph[i].userid[0] == '\0') {
               pphdat="";
               pphpts="";
          }
          else {
               pphdat=spr("%s",ncedat(ttpph[i].date));
               pphpts=l2as(ttpph[i].pts);
          }
          prf(fmtstg,i+1,ttpph[i].userid,pphdat,pphpts);
     }
     prfmsg(PTTPMT2);
}

static
VOID
ntrgam(VOID)                  /* enter game if possible                    */
{
     CHAR chr;
     INT i,j;
     CHAR *ptr,*optr;
     struct user *uptr;

     for (i=0 ; i < nterms ; i++) {
          uptr=usroff(i);
          if (uptr->state == drostt && uptr->substt == BCKGND4) {
               prfmsg(HOLDON2);
               usrptr->substt=P2PLAY2;
               return;
          }
     }
     drwbrd();
     allsco();
     prf("\33[%d;10H",vsize+2);
     prfmsg(ONEMOM);
     btuoes(usrnum,1);
     droptr->row=1;
     if (droptr->chr == '\0') {
          for (chr=usaptr->userid[0] ; fndlet(chr) ; chr=(chr+1-'A')%26+'A') {
          }
          droptr->chr=chr;
          dpafol[chr-'A']=droptr;
          unafol[chr-'A']=usrnum;
     }
     for (i=vsize+5 ; fndprw(i) ; i++) {
     }
     droptr->ptsrow=i;
     ptr=mapp+hsize;
     optr=omapp+hsize;
     for (i=1 ; i < (INT)vsize ; i++) {
          for (j=0 ; j < (INT)hsize ; j++) {
               *optr=(*ptr == '\4' || isdigit(*ptr) || isalpha(*ptr)
                      ? ' ' : *ptr);
               ptr++;
               optr++;
          }
     }
     usrptr->flags|=NOINJO;
     usrptr->substt=BCKGND4;
}

static
INT
finsco(VOID)                  /* output "final" score upon interruption    */
{
     INT i,n;
     droid *dp;
     LONG pts,pph;
     CHAR *prepts,*pstpts,*prepph,*pstpph;
     INT autott=0;

     for (i=0 ; i < nplyrs ; i++) {
          dp=&drolst[playrs[i]];
          pts=dp->points;
          pph=(pts*3600L)/dp->secs;
          prepts=pstpts=prepph=pstpph="";
          if (playrs[i] == usrnum) {
               if ((n=intop10(pts,ttpts)) != 0) {
                    prepts=spr("\33[s\33[A\33[5;45;33m#%-2dEVER!\33[u\33[44m",n);
                    pstpts="\33[0;1;44;37m";
                    autott=1;
               }
               if ((n=intop10(pph,ttpph)) != 0) {
                    prepph=spr("\33[s\33[A\33[5;45;33m#%-2dEVER!\33[u\33[44m",n);
                    pstpph="\33[0;1;44;37m";
                    autott=1;
               }
          }
          prf("\33[%d;41H%s %7s%s%s %7s%s",dp->ptsrow,
               prepts,l2as(pts),pstpts,prepph,l2as(pph),pstpph);
     }
     outprf(usrnum);
     if (autott) {
          writ10();
     }
     return(autott);
}

static
VOID
writ10(VOID)                  /* write top-three data to disk              */
{
     FILE *fp;

     if ((fp=fopen("elwand.t10",FOPWB)) == NULL) {
          catastro("ELWAND: CAN'T OPEN ELWAND.T10");
     }
     else {
         fwrite(ttpts, 3, sizeof(struct t10stf), fp);
         fwrite(ttpph, 3, sizeof(struct t10stf), fp);
         fclose(fp);
     }
}

static
INT
intop10(LONG pts,t10stf *ttpts_local)            /* find out if score is in top-three (where) */
{
     INT j,k;
     t10stf *ttptsk;

     for (j=0 ; j < 3 ; j++) {
          if (pts > ttpts_local->pts) {
               for (k=j,ttptsk= ttpts_local; k < 2 ; k++,ttptsk++) {
                    if (sameas(ttptsk->userid,usaptr->userid)) {
                         break;
                    }
               }
               if (j < k) {
                    movmem(ttpts_local, ttpts_local +1,(k-j)*sizeof(struct t10stf));
               }
               strcpy(ttpts_local->userid,usaptr->userid);
               ttpts_local->date=today();
               ttpts_local->pts=pts;
               return(j+1);
          }
          else if (sameas(ttpts_local->userid,usaptr->userid)) {
               break;
          }
          ttpts_local++;
     }
     return(0);
}

static
VOID
rmvguy(INT guy)                   /* remove a guy from the game                */
{
     INT i;

     for (i=0 ; i < nplyrs ; i++) {
          if (playrs[i] == guy) {
               dsairp();
               drochi(guy,' ');
               btuchi(guy,NULL);
               echonu(guy);
               playrs[i]=playrs[--nplyrs];
               enairp();
               prf("\33[%d;4H%33s\33[u",drolst[guy].ptsrow,"");
               for (i=0 ; i < nplyrs ; i++) {
                    outprf(playrs[i]);
               }
               clrprf();
               return;
          }
     }
     catastro("ELWAND: RMVGUY ERROR");
}

static
INT
fndprw(INT ptsrow)                /* find if a given points-row is taken yet   */
{
     INT i;

     for (i=0 ; i < nplyrs ; i++) {
          if (drolst[playrs[i]].ptsrow == ptsrow) {
               return(1);
          }
     }
     return(0);
}

static
INT
fndlet(CHAR chr)                   /* find if a given android letter is taken   */
{
     INT i;

     for (i=0 ; i < nterms ; i++) {
          if (drolst[i].chr == chr) {
               return(1);
          }
     }
     return(0);
}

static
VOID
drosta(VOID)                  /* Androids master status-input entry point  */
{
     setmbk(dromb);
     droptr=&drolst[usrnum];
     switch (usrptr->substt) {
     case BCKGND4:
          if (status == OUTMT) {
               asetup();
          }
          else if (status == 252 || status == 253) {
               go2slo();
          }
          else {
               dfsthn();
          }
          break;
     case TOOSLO:
          if (status == OUTMT) {
               btuoes(usrnum,0);
               btulok(usrnum,0);
               g2play();
          }
          else {
               dfsthn();
          }
          break;
     case PLAYING:
          if (status == CYCLE) {
               if (droptr->points != droptr->ptsdsp) {
                    rf1sco(droptr);
                    droptr->scodun=1;
               }
               else if (btuoba(usrnum) == OUTSIZ-1 && droptr->needrc) {
                    droptr->needrc=0;
                    btuxmt(usrnum, rstrcur);
               }
               btuinj(usrnum,CYCLE);
          }
          else if (status == 252) {
               rmvguy(usrnum);
               go2slo();
          }
          else if (status == 253) {
               btuclo(usrnum);
          }
          else {
               dfsthn();
          }
          break;
     default:
          dfsthn();
     }
}

static
VOID
go2slo(VOID)                  /* shut a player down due to overflow        */
{
     btuclo(usrnum);
     btulok(usrnum,1);
     btuoes(usrnum,1);
     btucli(usrnum);
     prfmsg(usrptr->substt=TOOSLO);
     outprf(usrnum);
}

static
VOID
asetup(VOID)                  /* attempt to set up a new android, or cycle */
{                             /* (the tricky part is synchronizing this    */
                              /* user's CRT with everybody else's that is  */
     INT i,j;                 /* already in the game, and pounding away on */
     CHAR *optr,*ooptr;       /* their keyboards while he tries to get in) */
     droid *dp;

     dsairp();
     if (memcmp(mapp,omapp,bdsize) == 0) {
          playrs[nplyrs++]=usrnum;
          btuchi(usrnum,drochi);
          btuech(usrnum,2);
          for (i=1 ; i < (INT)(hsize-2) ; i++) {
               if (mapp[hsize+i] == ' ') {
                    break;
               }
          }
          droptr->col=i-1;
          drochi(usrnum,'L');
          enairp();
          in1sco(droptr,usaptr->userid);
          outpys(1);
          droptr->secs++;
          btuoes(usrnum,0);
          for (i=0 ; i < nplyrs ; i++) {
               if (playrs[i] != usrnum) {
                    dp=&drolst[playrs[i]];
                    prf("\33[%d;42H%7s %7s",dp->ptsrow,
                       l2as(dp->points),l2as((dp->points*3600L)/dp->secs));
               }
          }
          prf("\33[0m\33[%d;1H\33[K\33[u\33[1;44;37m",vsize+2);
          usrptr->substt=PLAYING;
          btuinj(usrnum,CYCLE);
     }
     else {
          enairp();
          movmem(omapp,oomapp,bdsize);
          dsairp();
          movmem(mapp,omapp,bdsize);
          enairp();
          optr=omapp+hsize;
          ooptr=oomapp+hsize;
          for (i=2 ; i < (INT)(vsize+1) ; i++) {
               for (j=0 ; j < (INT)hsize ; j++) {
                    if (*optr != *ooptr) {
                         if (*optr == '\4') {
                              prf("\33[%d;%dH\33[5;35m\4\33[0;1;37;44m",i,j+2);
                         }
                         else {
                              prf("\33[%d;%dH%c",i,j+2,*optr);
                         }
                    }
                    optr++;
                    ooptr++;
               }
          }
     }
     outprf(usrnum);
}

static
VOID
allsco(VOID)                  /* initialize all scores for new player      */
{
     INT i;

     for (i=0 ; i < nplyrs ; i++) {
          in1sco(&drolst[playrs[i]],uacoff(playrs[i])->userid);
     }
     prf("\33[u");
}

static
VOID
in1sco(droid *dp,CHAR *userid)             /* initialize one score utility              */
{
     prf("\33[%d;4H   %c    %-29s %7s %7s",
         dp->ptsrow,dp->chr,userid,l2as(dp->points),
        (dp->secs ? spr("%ld",(dp->points*3600L)/dp->secs) : ""));
}

static
VOID
rf1sco(droid *droptr_local)                /* refresh one score utility                 */
{
     prf("\33[%d;42H%7s %7s", droptr_local->ptsrow,l2as(droptr_local->points),
        l2as((droptr_local->points*3600L)/ droptr_local->secs));
     outpys(0);
     droptr_local->ptsdsp= droptr_local->points;
}

static
VOID
outpys(INT uncond)                /* output prfbuf contents to all players     */
{
     INT i;

     for (i=0 ; i < nplyrs ; i++) {
          if (uncond || btuoba(playrs[i]) > 1024) {
               drolst[playrs[i]].needrc=1;
               outprf(playrs[i]);
          }
     }
}

static
VOID
drwbrd(VOID)                  /* draw board (playfield) on user's CRT      */
{
     CHAR mchar;
     INT i,j,ls219r=0,ls219c=0;
     //INT color=0;

     btutsw(usrnum,0);
     prf("\33[0m\33[2J\33[H\33[1;36;44m");
     for (i=0 ; i < (INT)(vsize+1) ; i++) {
          prf(" ");
          for (j=0 ; j < (INT)hsize ; j++) {
               mchar=map(i,j);
               if (mchar == ' ' || isdigit(mchar) || mchar == '\4'
                 || (isalpha(mchar) && i < (INT)vsize)) {
                    mchar=' ';
               }
               else if (mchar == 176) {
                    if (color != 1) {
                         prf("\33[36m");
                         color=1;
                    }
               }
               else if (mchar == 177) {
                    if (color != 2) {
                         prf("\33[32m");
                         color=2;
                    }
               }
               else if (mchar == 178) {
                    if (color != 3) {
                         prf("\33[31m");
                         color=3;
                    }
               }
               else if (mchar == 219) {
                    ls219r=i+1;
                    ls219c=j+2;
                    if (color != 4) {
                         prf(i < (INT)vsize ? "\33[33m" : "\33[0m");
                         color=4;
                    }
               }
               else if (mchar < 128) {
                    if (usaptr->systyp == 1 && color != 5) {
                         color=5;
                         prf("\33[7m");
                    }
               }
               else if (color != 6) {
                    prf("\33[1;33;44m");
                    color=6;
               }
               if (usaptr->systyp != 1) {
                    if (mchar == 219 && (UINT)i == vsize) {
                         mchar='-';
                    }
                    else if (mchar > 127) {
                         mchar='@';
                    }
               }
               prf("%c",mchar);
          }
          prf(" ");
          if (usaptr->scnwid != hsize+2) {
               prf("\r");
          }
     }
     prfmsg(PYLIST2);
     prf("\33[%d;%dH\33[s\33[44;37m",ls219r,ls219c);
}

static
CHAR
drochi(INT chan,INT ch)               /* INTERRUPT-LEVEL input-keystroke handler   */
{
     static INT updn[26]={
          1,-1,0,0,1,0,0,0,1,0,0,0,-1,-1,1,1,1,1,0,1,1,-1,1,-1,1,-1
     };
     static INT lfrt[26]={
          0,0,1,-1,0,1,-1,-1,0,-1,1,1,0,0,0,0,0,0,-1,0,0,0,0,0,0,0
     };
     static CHAR mvcode[26]={
          'A','B','C','D','A','C','D','D','A','D','C','C','B',
          'B','A','A','A','A','D','A','A','B','A','B','A','B'
     };
     droid *dp,*odp;
     CHAR locdat;
     UINT oldloc,newloc,przloc,ptexch;
     UINT oldrow,oldcol,newrow,newcol,przrow,przcol,i;
     CHAR outstg[40] = {0}, * osptr;
     CHAR c;

     dp=&drolst[chan];
     c=((CHAR)ch)&eurmsk;
     if (isalpha(c) && chbufd < CHCRIT) {
          c=(c&~('a'-'A'))-'A';
          oldrow=dp->row;
          oldcol=dp->col;
          newrow=oldrow-updn[c];
          newcol=oldcol+lfrt[c];
          osptr=outstg;
          if ((locdat=mapp[newloc=newrow*hsize+newcol]) != ' ') {
               if (isdigit(locdat)) {
                    dp->points+=locdat-'0';
                    do {
                         przrow=(newrow+(psernd+=12163))%vsize;
                         przcol=(newcol+(psernd+= 7457))%hsize;
                    } while (mapp[przloc=przrow*hsize+przcol] > ' ');
                    mapp[przloc]=locdat;
                    *osptr++='\33';
                    *osptr++='[';
                    if (przrow >= 9) {
                         *osptr++=firsdg[przrow];
                    }
                    *osptr++=secndg[przrow];
                    *osptr++=';';
                    if (++przcol >= 9) {
                         *osptr++=firsdg[przcol];
                    }
                    *osptr++=secndg[przcol];
                    *osptr++='H';
                    *osptr++=(showro ? locdat : ' ');
               }
               else if (isalpha(locdat) && newrow < vsize) {
                    odp=dpafol[locdat-'A'];
                    ptexch=(UINT)min(odp->points,rampts);
                    dp->points+=ptexch;
                    odp->points-=ptexch;
                    chiout(unafol[locdat-'A'],'\7');
                    chbufd++;
                    przrow=newrow-updn[c];
                    przcol=newcol+lfrt[c];
                    while (mapp[przloc=przrow*hsize+przcol] > ' ') {
                         przrow=(przrow+(psernd+=12163))%vsize;
                         przcol=(przcol+(psernd+= 7457))%hsize;
                    }
                    odp->row=przrow;
                    odp->col=przcol;
                    mapp[przloc]=locdat;
                    *osptr++='\33';
                    *osptr++='[';
                    if (przrow >= 9) {
                         *osptr++=firsdg[przrow];
                    }
                    *osptr++=secndg[przrow];
                    *osptr++=';';
                    if (++przcol >= 9) {
                         *osptr++=firsdg[przcol];
                    }
                    *osptr++=secndg[przcol];
                    *osptr++='H';
                    *osptr++=locdat;
               }
               else if (locdat == '\4') {
                    dp->points+=bonpts;
                    chiout(chan,'\7');
                    chbufd++;
               }
               else {
                    if ((dp->points-=ptsded) < 0) {
                         dp->points=0;
                    }
                    chiout(chan,'\7');
                    chbufd++;
                    return(0);
               }
          }
          mapp[newloc]=dp->chr;
          dp->row=newrow;
          dp->col=newcol;
          *osptr++='\33';
          *osptr++='[';
          if (oldrow >= 9) {
               *osptr++=firsdg[oldrow];
          }
          *osptr++=secndg[oldrow];
          *osptr++=';';
          if (++oldcol >= 9) {
               *osptr++=firsdg[oldcol];
          }
          *osptr++=secndg[oldcol];
          *osptr++='H';
          if (mapp[oldloc=oldrow*hsize+oldcol-1] == dp->chr) {
               mapp[oldloc]=' ';
               *osptr++=(CHAR)vacpos;
               if (vacpos == ' ') {
                    *osptr++='\b';
               }
               else {
                    *osptr++='\33';
                    *osptr++='[';
                    *osptr++='D';
               }
          }
          *osptr++='\33';
          *osptr++='[';
          *osptr++=mvcode[c];
          *osptr++=(showpy ? dp->chr : (pyshoq ? '?' : vacpos));
          *osptr='\0';
          chbufd+=(INT)(osptr-outstg);
          for (i=0 ; i < (UINT)nplyrs ; i++) {
               drolst[playrs[i]].needrc=1;
               chious(playrs[i],outstg);
          }
     }
     else if (c == ' ') {
          oldrow=dp->row;
          oldcol=dp->col;
          if (mapp[oldloc=oldrow*hsize+oldcol] == dp->chr) {
               mapp[oldloc]=' ';
               wrtchr[2]=firsdg[oldrow];
               wrtchr[3]=secndg[oldrow];
               wrtchr[5]=firsdg[++oldcol];
               wrtchr[6]=secndg[oldcol];
               wrtchr[8]=vacpos;
               chbufd+=(sizeof(wrtchr)-1);
               for (i=0 ; i < (UINT)nplyrs ; i++) {
                    drolst[playrs[i]].needrc=1;
                    chious(playrs[i],wrtchr);
               }
          }
     }
     else if (c == '\r') {
          chiinj(chan,CRSTG);
     }
     return(0);
}

static VOID
drorti(VOID)                  /* Androids real-time INTERRUPT handler      */
{
     UINT przrow,przcol,przloc,i;
     static CHAR przchr=' ';
     static UINT opzrow,opzcol;
     static INT spcctr=1;
     static INT numctr;
     static INT spcrow,spccol,spcloc;
     static CHAR wrtspc[]={"\33[yy;xxH\33[5;35m\4\33[0;1;37;44m"};

     if ((chbufd-=chperi) <= 0) {
          chbufd=0;
          if (nplyrs != 0) {
               if (przchr != ' ') {
                    do {
                         przrow=(opzrow+(psernd+=12163))%vsize;
                         przcol=(opzcol+(psernd+= 7457))%hsize;
                    } while (mapp[przloc=przrow*hsize+przcol] > ' ');
                    mapp[przloc]=przchr;
                    wrtchr[2]=firsdg[przrow];
                    wrtchr[3]=secndg[przrow];
                    wrtchr[5]=firsdg[++przcol];
                    wrtchr[6]=secndg[przcol];
                    wrtchr[8]=(showam ? przchr : (amshos ? ' ' : '?'));
                    chbufd=sizeof(wrtchr)-1;
                    for (i=0 ; i < (UINT)nplyrs ; i++) {
                         drolst[playrs[i]].needrc=1;
                         chious(playrs[i],wrtchr);
                    }
                    przchr=' ';
               }
               else {
                    przrow=(opzrow+(psernd+=12163))%vsize;
                    przcol=(opzcol+(psernd+= 7457))%(hsize-nquick);
                    przloc=przrow*hsize+przcol;
                    for (i=0 ; i < (UINT)nquick ; i++,przloc++) {
                         if (isdigit(mapp[przloc])) {
                              przcol+=i;
                              przchr=mapp[przloc];
                              mapp[przloc]=' ';
                              opzrow=przrow;
                              opzcol=przcol;
                              wrtchr[2]=firsdg[przrow];
                              wrtchr[3]=secndg[przrow];
                              wrtchr[5]=firsdg[++przcol];
                              wrtchr[6]=secndg[przcol];
                              wrtchr[8]=vacpos;
                              chbufd=sizeof(wrtchr)-1;
                              for (i=0 ; i < (UINT)nplyrs ; i++) {
                                   drolst[playrs[i]].needrc=1;
                                   chious(playrs[i],wrtchr);
                              }
                              break;
                         }
                    }
               }
          }
     }
     if (nplyrs != 0) {
          if (spcctr < 0) {
               if (spcctr == -1) {
                    if (chbufd < CHCRIT) {
                         if (mapp[spcloc] == '\4') {
                              mapp[spcloc]=' ';
                              wrtchr[2]=firsdg[spcrow];
                              wrtchr[3]=secndg[spcrow];
                              wrtchr[5]=firsdg[spccol];
                              wrtchr[6]=secndg[spccol];
                              wrtchr[8]=vacpos;
                              chbufd+=sizeof(wrtchr)-1;
                              for (i=0 ; i < (UINT)nplyrs ; i++) {
                                   drolst[playrs[i]].needrc=1;
                                   chious(playrs[i],wrtchr);
                              }
                         }
                         spcctr=bonoff;
                    }
               }
               else {
                    spcctr++;
               }
          }
          else if (spcctr == 1) {
               if (chbufd < CHCRIT-sizeof(wrtspc)) {
                    do {
                         spcrow=(spcrow+(psernd+=12163))%vsize;
                         spccol=(spccol+(psernd+= 7457))%hsize;
                    } while (mapp[spcloc=spcrow*hsize+spccol] > ' ');
                    mapp[spcloc]='\4';
                    wrtspc[2]=firsdg[spcrow];
                    wrtspc[3]=secndg[spcrow];
                    wrtspc[5]=firsdg[++spccol];
                    wrtspc[6]=secndg[spccol];
                    chbufd+=(sizeof(wrtspc)-1);
                    for (i=0 ; i < (UINT)nplyrs ; i++) {
                         drolst[playrs[i]].needrc=1;
                         chious(playrs[i],wrtspc);
                    }
                    if (++numctr >= bonnum) {
                         numctr=0;
                         spcctr=-bonon;
                    }
               }
          }
          else {
               spcctr--;
          }
     }
}

static
VOID
drohup(VOID)                  /* Androids master lost-carrier entry point  */
{
     droptr=&drolst[usrnum];
     if (usrptr->state == drostt && usrptr->substt == PLAYING) {
          rmvguy(usrnum);
     }
     setmem(droptr,sizeof(struct droid),0);
}

static
VOID
clsdro()                      /* Androids system-shutdown entry point      */
{
     clsmsg(dromb);
}

