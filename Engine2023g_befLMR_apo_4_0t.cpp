﻿/* Change History

20230408 - 2023g  use condvar to search mainthread bef other 7 helper-threads NCORE=8, thd_idx == 0 (mainthread) 
20230322 - 2023f5 7 helper-threads NCORE=8, thd_idx == 0 (mainthread) 
20230322 - 2023f4vote use both sf and sf15.1 votr best thread
20230321 - 2023f4 debug THREADPO loop (solved by use thd_ instead of spboard->
20230320 - 2023f3async vs eleeyea (4-0)
20230319 - 2023f4 use future[7],thd_completedDepth[8]
20230316 - 2023f3 no IsDraw(), quiesc not interrupt f-vs-e (4-0t)
20230312 - 2023f1 back to evalhash 6 bytes uint32_t hEkey; //lower 32 bits by cast (uint32_t) f-vs-e (2-2)
20230308 - 2023f1 chg evalhash from 6 bytes to 8 bytes, 50bit lock
                  hashstruct uint32_t   hkey:27, hDepth:5;  //20230308 use 27bit hkey
*/
#include <stdio.h>
//#define SYNCOUT
#undef SYNCOUT
//#define PRINTINFO
#undef PRINTINFO
#ifdef PRINTINFO
  FILE* infoout = fopen("eychessu2023ginfo.txt", "w+"); //w+");   //a+ use append
#endif
//#define ASYNC	
#undef ASYNC
#define THREADPO
//#undef THDINFO  
#ifdef  THREADPO
	#include <future>       // std::future
	#include <functional>   // std::bind
	#ifdef ASYNC
	#else
		#include "BS_thread_pool.hpp"
		#include <iostream> 
		//#include <fstream>
		#include <iomanip>
		extern BS::synced_stream sync_out;
		extern BS::thread_pool thread_pool; 
	#endif
	
	//#define TMR
	#undef  TMR
	#ifdef  TMR
		BS::timer tmr;
	#endif
	extern int NCORE; // logical core=8, thread 1-7 (thread 0=mainthread)
	int NCORE1 = NCORE - 1;
	//int NCORE=1; //20230327 test single thread
	//constexpr int NCORE1 = 7; //20230405 //7; //7; // NCORE - 1; // 7 = 8-1 //20230324 NCORE-1
	//#define NCORE1 7
  //int thd_bestmove[NCORE1+1]; //20230327 = {0,0,0,0,0,0,0,0};       //20230325
  //int thd_bestscore[NCORE1+1]; //20230401
  //int thd_compdepth[NCORE1+1]; //20230401
#endif
//#define THDINFO
#undef THDINFO  
#define EASYMOVE
//#undef EASYMOVE
//#define UNCHANGE
#undef UNCHANGE
#define TTSINGU
//#undef TTSINGU
//#define PREEVAL
#undef PREEVAL
#define QPH
//#undef QPH
//#define PVRED
#undef PVRED
//2892u - use stable_sort (stockfish-7)
#define ASPWIN
//#undef ASPWIN
#define ROOTLMR
//#undef ROOTLMR
#define HORSECK //1881e
//#undef HORSECK
//#define PERFT   //1880s
#undef PERFT
//#define DEBUG
#undef DEBUG

//#define PRTBOARD
#undef PRTBOARD
#undef PRINTEVA
#define HISTPRUN
//#undef HISTPRUN
#define HISTHIT
//#undef HISTHIT
#define QCHECK
//#undef QCHECK
#define QHASH
//#undef QHASH
#define EVALHASH
//#undef EVALHASH
#define DELTAN
//#undef DELTAN
#define FUTILITY
//#undef FUTILITY
#define EVALATTK
//#undef EVALATTK
#define ATTHREAT     //20230121
//#undef ATTHREAT


/* Eychessu -
a Chinese Chess Program Designed by Edward Yu, Version: 2023d

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* History
// 20230209 - 2023d taped eval
// 20160806 - 2892v time usage similiar to XQMS
// 20160724 - 2892u max(8, (UcciComm.nTime/25) to prevent timeout
// 20160721 - 2892t use stockfish-7 aspwin
// 20160717 - 2892s abort helper threads if main thread ends, using atomic_bool thdabort
//            2892s - use stable_sort (stockfish-7)
// 20160717 - 2892s implement lazy smp (use c++11 thread), work but seems to create/destroy 
//                  with more overhead. Using future/async seems to reuse threads  
// 20160715 - 2892r implement lazy smp
2006-02-13  v0.93 - initial copy from HIce 1.02
2006-02-16  v0.93a - rename "xor" by "Xor". Fix mate finding for FEN 2b1ka3/4a4/4b4/9/9/2B3B2/9/3p5/3pp4/5K3 w - - 0 1
		disable nullmove at endgame.
2006-02-21  v0.94 - add srchbook.cpp using JNI invocation to read EYBOOK.DAT
2006-02-23  v0.95 - use c++ file-io and string tokenizer to read EYBOOK.DAT
2006-03-18  v0.96 - use aggressive nullmove R=3,4(depth>5) and history pruning 50%
2006-03-24  v0.97 - fix recapture extended bug, history pruning 25%, add repval
2006-04-14  v0.98 - 2level hash, MoveStruct
2006-05-17  v0.99 - nullred(3/3, 2/2), histprun(1/3, 2/4)
2006-05-23  v1.00 - use bitrank, bitfile for rook/cannon capture and isincheck
2006-05-27  v1.01 - this_pv[2]
2006-05-31  v1.02 - nullred(2/4, 2/2)+Eval(), histred(2/3, 2/3)
2006-05-31  v1.03 - fix fpv=1 after if val>thisalpha
2006-06-01  v1.03a- use phase loop. nullred(3/6, 2/2)+Eval(), histred(1/2, 2/3), no histprun, no futility
2006-06-04  v1.03b- null comment out depth <=nullreduction+1
2006-06-04  v1.03c- put root last pv to the back
2006-06-09  v1.04 - node_type, pv[2], attack_rook, nullred(3/6, 2/2)+Eval(), histred(1/2, 2/3)
2006-06-13  v1.04c- zkey, no evalhash, node_type, pv[2], attack_rook, nullred(3/6, 2/2)+pointsum, histred(1/2, 2/3)
2006-06-14  v1.04d- fix legalkiller elephan, fix gennoncapture() missing break error: rook and cannon illegal move
2006-06-15  v1.05a- update killer/hist if phase==3 only
2006-06-15  v1.05b- add RecordHash when Nullmove beta cutoff
2006-06-16  v1.06 - rewrite makemove, add have rook better than no rook in Eval
2006-06-16  v1.06a- g_killer[4]
2006-06-16  v1.06d- back to g_killer[2], remove rook better but add 20 in rook basicvalues in endgame
2006-06-19  v1.07 - add rook better. rewrite banned. fix legalmv isincheck to use wbitrank/file
2006-06-20  v1.08 - correct bottom cannon face king. add generate_checks if cannon superchecking.
2006-06-22  v1.09 - skip small protcapture (tabval<0) in quiesce. add endgame pointtable chg for low pawns.
2006-06-22  v1.09a -add if (!incheck && tabval<0) to fix loseLHA.fen bug. rewrite to add MoveTabStruct for tab and tabval
2006-06-26  v1.09d -use null_allow in a-b. null_allow in Nullmove and IID, else all null_allow=1 even at root. see glaurung.
2006-06-28  v1.09e -searchroot return immed if move_count==1. Try king move evasion extension.
2006-06-29  v1.1  - bug fix knight in legalkiller. add NonCapCnt in PreGen for Eval() rook mobility.
2006-06-30  v1.1a - rewrite cannon case in Eval() using pregen
2006-06-30  v1.1b - condense rook and cannon eval() in n,k loops
2006-07-06  v1.1h - add pointsum <= alpha in history pruning (prodeo 1.1)
2006-07-08  v1.12 - fix fen position moves ? (IMakeMove). use hist reduct% to 50%+12.5% = 62.5% and exthistshift=25%
2006-07-08  v1.13 - rewrite GenCap, Gen etc using tabptr. win JUPITER
2006-07-12  v1.14 - add SideKnight. revise rook mobility. reduce side pawn value.
2006-07-13  v1.14a - HistoryMoveNb=6, no exthistprune. win JUP
2006-07-16  v1.15 - add delta pruning
2006-07-19  v1.15a- delta margin=25 win eleeye1.6 (10min)
2006-08-10  v1.16 - aspiration window=40 (see Beowulf), win anita 0.2 (10min)
2006-08-14  v1.16e- HistoryMax=16384 win anita 0.1 (10min)
2006-09-15  v1.17 - add mate killer
2006-09-20  v1.18 - add quieskiller (see coonyl)
2006-09-25  v1.18f- nullreductver=8, (endgame=4), bis-ele value endgame 50->55  win anita
2006-09-28  v1.19 - nullred use eval()
2006-10-09  v1.20 - use 4-level hash (see eleeye 1.61)
2006-10-11  v1.21 - use 2 killers with killerscore (see lime). use pointtable for ordering in root, use histval alone in gen()
2006-10-17  v1.21win do not reassign tabval in root. use pointtableABS + histval in root and gen(). nullredver=9, endgame=6
2006-10-19  v1.21draw xena use smash nullreduction formula
2006-10-20  v1.22win add futility margin1=35, 2*RAZOR at endgame &&!mate_threat. (glaurung 121). incr hash to 64M. bug in TT (mate_threat)
2006-10-27  v1.22Ewin nullmove use eval-tempo, futilty use pointsum+25+razor[45], history 45%. endgame same razor and history.
		    endgame==0 for qcheck and king_evasion ext.  skip small capture even end_game.
2006-12-28  v1.24win3DC futility 40 extfut 100 extHist >>2 ( >>3 at endgame)	exthist capture==0
2006-12-30  v1.24awin3DC histshift >>2 at endgame, RAZOR*2 at endgame
2006-12-30  v1.24b  loop HistorySize only at 0,1  4,5  8,9 etc (see mask29.txt)
//0 0 2 3   4 5 6 7   8 9 10 11  12 13 14 15  16 17 18 19   20 21 22 23   24 25 26 27  28 29 30 31  32 33
//&011101 (& 0x1D)  ( &29)
//0 0 0 1   4 5 4 5   8 9 8 9    12 13 12 13  16 17 16 17   20 21 20 21   24 25 24 25  28 29 28 29  0 1
    bprb bprbbprb  bprbbprb    bb rb bb rb
2006-12-31  v1.25win-draw3DC   sort root moves by tabval in iterations
2007-01-05  v1.26draw anita    fix rook sac bug (suppress HashRecord in pv,cap,killer phase). update killer if capture==0
                               history pruning && movetab[i].tabval < 0
2007-01-07 v1.26a win anita    history pruning <<3 instead of <<4   pending fortattack and ADVISOR bonus
2007-01-08 v1.27  win anita    fix rook sac bug by if dropdown value of 40, board.m_bestmove = prev_bestmove
2007-01-14 v1.28  win 3DC      add qcheck. tempo=6, futility=50/125 end=60/150   no qskiller  nullmove if depth>0 recordhash
2007-01-16 v1.29  win 3DC      hist prune && gencount>=8 //10
2007-01-18 v1.30  win 3DC/ANI  unloop Eval() poscolor=0,1  remove Eval_legalmv and move central knight/cannon check to cannon/rook eval
2007-01-22 v1.31  win 3DC      HistCutShift=3 gen_count>=9 capture==0   Histprun tabval < 3500
2007-01-24 v1.32  win 3DC      HistCutShift=3 gen_count>=9 capture==0 all. remove tabval < 3500. add singleext in search/QS.
2007-01-27 v1.33  win 3DC50    add kingdoor checking, add pawn-isattackedby, add mate_threat, histcutshift revert to >>4
2007-01-31 v1.34  win xen      update_pv, qcheck, check_depth -1, qhash, remove mate_threat, nullreduction=3,nullreductver=5
2007-02-10 v1.35  win anita    add FORTATTK, window=30 hist gencount>=10 delta=25
2007-03-11 v1.36               tabval=0 for protcapt
2007-03-13 v1.36a              clearhash() instead of aging(). fix river pawn. wincap <<10
2007-03-14 v1.37               GenChkEvasion()
2007-03-15 v1.37a              remove tempo_bonus. GenChkEvasion() incheck 1=pawn, 2=horse+p, 3=rook/cannon/horse/pawn
2007-03-16 v1.37b              incheck 1=rook/c/p file, 2=rook/c/p rank, 3=horse
2007-03-30 v1.38 win4          use bitpiece. quiesCheck() for ordering root nodes. use ponder move as root PV
2007-04-02 v1.38a              use prev_boardsq to derive p_feedback_move
2007-04-03 v1.38b              align prev_boardsq same piece type
2007-04-04 v1.39               sort root moves by board.m_nodes - start_nodes
2007-04-06 v1.39a              fix rookcapbug by call legalkiller for pv
2007-04-10 v1.40               add mate_threat in board.Eval() for nullmove. extend based on mate_threat no improvement
2007-04-12 v1.41               add back evalhash.
2007-04-17 v1.42               add pawnhash
2007-04-25 v1.43a		add banmove. suppress nobestmove. change evalscore back to pointsum in nullmove and futility
				add board.p_endgame==0 in futility and histcut. Great improvement!!! win eleeye 4-2-2 !!!
2007-04-26 v1.44		futility/history pruning before makemove, see toga
2007-05-30 v1.47    use 10x16 board for faster nFile, nRank
2007-07-14 v1.48    add attack_threat extension
2007-10-30 v1.52A2  dropdown_val 30/40, add attack_threat extension for only node_type==NodePV (win ufx 5-2-3)
2007-11-01 v1.52A7  dropdown_val 40/40, only cannon threat, board.p_endgame==0, (win ufx 5-1-4 short 1 min)
2007-11-01 v1.52A8  dropdown_val 40/40, full attack threat, board.p_endgame==0, (win ufx 4-3-3 short 1 min)
2007-11-02 v1.52AB  futility endgame fut_depth=0 (from 2), dropdown_val 40/40, full attack threat, board.p_endgame==0, (win ufx 4-3-3 short 1 min)
2007-11-02 v1.52AD  futility endgame depth-- instead of pruning (win ufx 4-3-3 long 10 min)
2007-11-02 v1.53    atthreat win ufx 3-5-2
2007-11-10 v1.53E   use quiescCheck for all. QHash if qdepth<=check_depth (draw ufx 5-0-5, win 5-3-2)
2007-11-20 v1.54B5/6  use eleeye 300 hashstruct and pointtable (win ufx 8-1-1)
2007-11-21 v1.55    use left-right attack-defend eval
2008-07-12 v1.187   DsqSq-- to use 0 as terminator to shorten rookmove to unsigned char
2008-07-31 v1.840   return +8 for incheck by rook-cann double-check so that chkevas excludes capture
2008-08-22 v1.848   in gencap/qs gen cannon, rook first   win 3dc(110) kin(110) ele(110) lil(200) ufx(200) 730
2008-08-22 v1.848a  separate gencapqs for m_side==0 / 1   win 3dc(110) kin(110) ele(110) lil(200) ufx(200) 730
2008-08-24 v1.848c  combine gencapqs,                     win 3dc(101) kin(110) ele(200) lil(101) ufx(200) 712
2008-08-25 v1.848e  shiftfile uint16_t kingevas[4]  win 3dc(110) kin(110) ele(101) lil(101) ufx(200) 622
2008-09-14 v1.850j  supercap val 12 advele val 48         win 3dc(202) kin(121) ele(220)
2008-11-10 v1.854n  use qphase and phase
2008-11-30 v1.855x  noevalBE,attkpval=2                   los 121 202 112 022 103                          5510
2008-12-05 v1.856f  relhis - val+=dxd, dxd/tot++, hit++   win 112	220	202	121	202                          857
2008-12-05 v1.856g  relhis - no Histval, use Hit++/Tot++  los 3dc(031) kin(022) ele(202) bhw(121) ufx(211) 587
2008-12-05 v1.856p  relhis - histcut>>4->>7, end>>5->>7   win 3dc(301) kin(301) ele(121) bhw(121) ufx(301) 1145
2009-01-07 v1.857f  rookdraw                              los 3dc(004) cyc(022) ele(022)
2010-05-15 v1.882x  fix srchbook and bookmain
2010-05-16 v1.882y  fix endgame knight vs pawn bug
2010-05-22 v1.883b  fix missing refuted move bug,add pinned ADVISOR bonus
2010-08-02 v1.885b  fix chase unprot bug - but need check chase one step before
//6A2 (no delta, no fut, no atthreat) -ufx (325)
//6A3 (no delta, no fut,    atthreat) -ufx (118)
//6A4 (no delta,    fut, no atthreat) -ufx (235)
//6A8 (no delta,    fut,    atthreat) -ufx (   )
//6A5 (   delta, no fut, no atthreat) -ufx (145)
//6A6 (   delta,    fut, no atthreat) -ufx (334)
//6A7 (   delta,    fut,    atthreat) -ufx (343)
//6A9 (   delta, no fut,    atthreat) -ufx (   )
//6AA (   delta,    fut,    atthreat) -ufx (325) combine rook eval()
//6B2 (   delta,    fut,    atthreat) -ufx (514) combine rook/pawn/cannon eval()
//6B4 -ufx (523) -ele (154)
//
*/

#include ".\engine.h"
//#include "EFen.h"
//#include "srchbook.h"
#include "srchboob.h"
#include "ucci.h"


//#include "locale.h"
//static const char PieceChar[34] = {'.', '.', 'p', 'P', 'p', 'P', 'p', 'P', 'p', 'P', 'p', 'P', 'a', 'A', 'a', 'A', 'b', 'B', 'b', 'B',
//                                   'h', 'H', 'h', 'H', 'c', 'C', 'c', 'C', 'r', 'R', 'r', 'R', 'k', 'K'
//                                  };
#ifdef PRTBOARD
static const wchar_t PieceChar[34] =
{L'。', L'。', L'卒', L'兵', L'卒', L'兵', L'卒', L'兵', L'卒', L'兵', L'卒', L'兵', L'士', L'仕', L'士', L'仕',
 L'象', L'相', L'象', L'相', L'馬', L'傌', L'馬', L'傌', L'包', L'炮', L'包', L'炮', L'車', L'俥', L'車', L'俥', L'將', L'帥'
};
int prtboard=0;
int before_score=0;
char charmove[5];
void Engine::print_board(int score) //for debug
{
//setlocale( LC_ALL, "chs" );
	if (prtboard<50)
	{
	  printf("\n");
    for (int i=0; i<10; i++)
    {	if (i==5) printf(" ----------------------------------\n");
    	for (int j=0; j<9; j++)
    	{
    	wprintf(L" %c ", PieceChar[board.piece[(i*16)+j]] );
    	}
    	printf("\n");
    	fflush(stdout);
    }
    prtboard++;
    printf("score=%d, ", score);
		before_score = score;
	}
}
#endif

#define NULL_NO  0
#define NULL_YES 1
#define THREAT_MARGIN 96 //80 //96 //128 //192(86j) //96
//int movenum=0;
//int board.ply = 0;
#ifdef TTSINGU
//int pv_singular_cnt, nonpv_singular_cnt;
#endif
int RCH_count = 0;
//int pointsum = 0;
int evalthreat[2] = {0, 0};
//uint32_t bitpiece = 0; //RrRrCcCc HhHhEeEe BbBbPpPp PpPpPp00
// piececnt[piece&61]
//int piececnt[34] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//bitpieceStruct bitp;
#ifdef EVALATTK
//uint32_t bitattk[10] = {0,0,0,0,0,0,0,0,0,0}; //[left-right][poscolor]  [4]=black central, [5]=red central
unsigned char ATTKAREA[BOARD_SIZE-7] = {

0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
6,6,0,0,4,2,2,8,8,  0,0,0,0,0,0,0,
6,6,6,6,4,8,8,8,8,  0,0,0,0,0,0,0,

7,7,7,7,5,9,9,9,9,  0,0,0,0,0,0,0,
7,7,1,1,5,3,3,9,9,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3};
/*
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
0,0,0,0,4,2,2,2,2,  0,0,0,0,0,0,0,
6,6,6,6,4,8,8,8,8,  0,0,0,0,0,0,0,

7,7,7,7,5,9,9,9,9,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3,  0,0,0,0,0,0,0,
1,1,1,1,5,3,3,3,3};
*/
//int board.p_endgame_7 = 7;
#endif

#ifdef QCHECK
//int QNodes = 0;
#endif
#define MAXPLY 256 //128
#define MAXEXT 64
#define MAXDEPTH 64
#define DRAWVALUE 1 //16 //30  //32; // 20;  // 藐視因子，即和棋時返回的分數(取負值)
//int DRAWVALUE = 16; //endgame value 1
int TEMPO_BONUS = 5;  //endgame = 0;
#define INF 2048
//#define NUM_KILLERS 2 //2  //
MoveStruct g_killer[16][2][256];  //20230511 max 16 threads 
MoveStruct g_matekiller[16][256]; //20230511 max 16 threads 
int thd_bestmove[16] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};       //20230511 max 16 threads 
int thd_bestscore[16] = {-INF,-INF,-INF,-INF,-INF,-INF,-INF,-INF, -INF,-INF,-INF,-INF,-INF,-INF,-INF,-INF};   //20230511 max 16 threads 
int thd_completedDepth[16] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0}; //20230511 max 16 threads 

uint64_t h_value[10][9][10];   //[piecetype][nFile}[nRank]
uint64_t h_rside;

// misc
//static const int NodeAll = -1;
//static const int NodePV  =  0;
//static const int NodeCut = +1;
#define NodeAll 1
#define NodePV 0
#define NodeCut 1
// macros
//#define NODE_OPP(type)     (-(type))
#define NODE_OPP(type) (type)
//for std::max min
#include <algorithm>   
//#define MAX(x,y) (std::max(x,y))
//#define MIN(x,y) (std::min(x,y))
//#define MAX(x,y) (((x)>(y))?(x):(y))
//#define MIN(x,y) (((x)<(y))?(x):(y))


//int root_qcheck_depth = -4; //0; //-2;	//end_game = -4
#define root_qcheck_depth 0 //-4
//int qcheck_depth = 0;
#define qcheck_depth 0



//{-19, -17, -11, -7, 7, 11, 17, 19};
//static const int knightchecknew[8] = {9,9,-1,1,-1,1,-9,-9};
//       6   7
//     4       5
//         +
//     2       3
//       0   1
/*
static unsigned char knightmoves[BOARD_SIZE-7][8] =
   {{255,33,255, 18,255,255,255, 255},{ 32, 34,255, 19,255,255,255,255},{ 33, 35, 16, 20,255,255,255,255},{ 34, 36, 17, 21,255,255,255,255},{ 35, 37, 18, 22,255,255,255,255},{ 36, 38, 19, 23,255,255,255,255},{ 37, 39, 20, 24,255,255,255,255},{ 38, 40, 21,255,255,255,255,255},{ 39,255, 22,255,255,255,255,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255, 49,255, 34,255,  2,255,255},{ 48, 50,255, 35,255,  3,255,255},{ 49, 51, 32, 36,0,  4,255,255},{ 50, 52, 33, 37,  1,  5,255,255},{ 51, 53, 34, 38,  2,  6,255,255},{ 52, 54, 35, 39,  3,  7,255,255},{ 53, 55, 36, 40,  4,  8,255,255},{ 54, 56, 37,255,  5,255,255,255},{ 55,255, 38,255,  6,255,255,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255, 65,255, 50,255, 18,255,  1},{ 64, 66,255, 51,255, 19,0,  2},{ 65, 67, 48, 52, 16, 20,  1,  3},{ 66, 68, 49, 53, 17, 21,  2,  4},{ 67, 69, 50, 54, 18, 22,  3,  5},{ 68, 70, 51, 55, 19, 23,  4,  6},{ 69, 71, 52, 56, 20, 24,  5,  7},{ 70, 72, 53,255, 21,255,  6,  8},{ 71,255, 54,255, 22,255,  7,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255, 81,255, 66,255, 34,255, 17},{ 80, 82,255, 67,255, 35, 16, 18},{ 81, 83, 64, 68, 32, 36, 17, 19},{ 82, 84, 65, 69, 33, 37, 18, 20},{ 83, 85, 66, 70, 34, 38, 19, 21},{ 84, 86, 67, 71, 35, 39, 20, 22},{ 85, 87, 68, 72, 36, 40, 21, 23},{ 86, 88, 69,255, 37,255, 22, 24},{ 87,255, 70,255, 38,255, 23,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255, 97,255, 82,255, 50,255, 33},{ 96, 98,255, 83,255, 51, 32, 34},{ 97, 99, 80, 84, 48, 52, 33, 35},{ 98,100, 81, 85, 49, 53, 34, 36},{ 99,101, 82, 86, 50, 54, 35, 37},{100,102, 83, 87, 51, 55, 36, 38},{101,103, 84, 88, 52, 56, 37, 39},{102,104, 85,255, 53,255, 38, 40},{103,255, 86,255, 54,255, 39,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255,113,255, 98,255, 66,255, 49},{112,114,255, 99,255, 67, 48, 50},{113,115, 96,100, 64, 68, 49, 51},{114,116, 97,101, 65, 69, 50, 52},{115,117, 98,102, 66, 70, 51, 53},{116,118, 99,103, 67, 71, 52, 54},{117,119,100,104, 68, 72, 53, 55},{118,120,101,255, 69,255, 54, 56},{119,255,102,255, 70,255, 55,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255,129,255,114,255, 82,255, 65},{128,130,255,115,255, 83, 64, 66},{129,131,112,116, 80, 84, 65, 67},{130,132,113,117, 81, 85, 66, 68},{131,133,114,118, 82, 86, 67, 69},{132,134,115,119, 83, 87, 68, 70},{133,135,116,120, 84, 88, 69, 71},{134,136,117,255, 85,255, 70, 72},{135,255,118,255, 86,255, 71,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255,145,255,130,255, 98,255, 81},{144,146,255,131,255, 99, 80, 82},{145,147,128,132, 96,100, 81, 83},{146,148,129,133, 97,101, 82, 84},{147,149,130,134, 98,102, 83, 85},{148,150,131,135, 99,103, 84, 86},{149,151,132,136,100,104, 85, 87},{150,152,133,255,101,255, 86, 88},{151,255,134,255,102,255, 87,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255,255,255,146,255,114,255, 97},{255,255,255,147,255,115, 96, 98},{255,255,144,148,112,116, 97, 99},{255,255,145,149,113,117, 98,100},{255,255,146,150,114,118, 99,101},{255,255,147,151,115,119,100,102},{255,255,148,152,116,120,101,103},{255,255,149,255,117,255,102,104},{255,255,150,255,118,255,103,255},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {255,255,255,255,255,130,255,113},{255,255,255,255,255,131,112,114},{255,255,255,255,128,132,113,115},{255,255,255,255,129,133,114,116},{255,255,255,255,130,134,115,117},{255,255,255,255,131,135,116,118},{255,255,255,255,132,136,117,119},{255,255,255,255,133,255,118,120},{255,255,255,255,134,255,119,255}
}; //,{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
*/


// 0 1
// 2 3
/*
unsigned char g_advelemoves[BOARD_SIZE-7][4] =
 {{0,0,0,0},{0,0,0,0},{32,36,0,0}, { 0,0,0,20},{0,0,0,0},{ 0,0,0,20},      { 36, 40,0,0},{0,0,0,0},{0,0,0,0},   {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0}, {0,0,0,0},{3,  5, 35, 37},{0,0,0,0},    {0,0,0,0},{0,0,0,0},{0,0,0,0},       {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
   {2,66,0,0},{0,0,0,0},{0,0,0,0}, { 0,0,0,20},{2,  6, 66, 70},{ 0,0,0,20},{0,0,0,0},{0,0,0,0},{  6, 70,0,0},   {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0},          {0,0,0,0},{0,0,0,0},{0,0,0,0},       {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
  {0,0,0,0},{0,0,0,0},{32,36,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0},          { 36, 40,0,0},{0,0,0,0},{0,0,0,0},   {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{112,116,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0},          {116,120,0,0},{0,0,0,0},{0,0,0,0},   {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0}, {0,0,0,0},{0,0,0,0},{0,0,0,0},          {0,0,0,0},{0,0,0,0},{0,0,0,0},       {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
 {82,146,0,0},{0,0,0,0},{0,0,0,0}, {0,0,0,132},{82,86,146,150},{0,0,0,132},{0,0,0,0},{0,0,0,0},{ 86,150,0,0},   {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
    {0,0,0,0},{0,0,0,0},{0,0,0,0}, {0,0,0,0},{115,117,147,149},{0,0,0,0},  {0,0,0,0},{0,0,0,0},{0,0,0,0},       {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{112,116,0,0}, {0,0,0,132},{0,0,0,0},{0,0,0,132},      {116,120,0,0},{0,0,0,0},{0,0,0,0}
}; //,{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
*/
unsigned char g_KnightMoves[BOARD_SIZE-7][16];
/*
= {
{ 34, 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 33, 35, 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 34, 36, 17, 21,  0,  0,  0,  0,  0, 32, 36,  0,  0,  0,  0,  0},{ 35, 37, 18, 22,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0},{ 36, 38, 19, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 37, 39, 20, 24,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0,  0},{ 38, 40, 21, 25,  0,  0,  0,  0,  0, 36, 40,  0,  0,  0,  0,  0},{ 39, 41, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 40, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{ 50, 35,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 49, 51, 36,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 50, 52, 33, 37,  1,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 51, 53, 34, 38,  2,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 52, 54, 35, 39,  3,  7,  0,  0,  0,  3,  5, 35, 37,  0,  0,  0},{ 53, 55, 36, 40,  4,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 54, 56, 37, 41,  5,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 55, 57, 38,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 56, 39,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{ 66, 51, 19,  2,  0,  0,  0,  0,  0,  2, 66,  0,  0,  0,  0,  0},{ 65, 67, 52, 20,  1,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 66, 68, 49, 53, 17, 21,  2,  4,  0,  0,  0,  0,  0,  0,  0,  0},{ 67, 69, 50, 54, 18, 22,  3,  5,  0, 20,  0,  0,  0,  0,  0,  0},{ 68, 70, 51, 55, 19, 23,  4,  6,  0,  2,  6, 66, 70,  0,  0,  0},{ 69, 71, 52, 56, 20, 24,  5,  7,  0, 20,  0,  0,  0,  0,  0,  0},{ 70, 72, 53, 57, 21, 25,  6,  8,  0,  0,  0,  0,  0,  0,  0,  0},{ 71, 73, 54, 22,  7,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 72, 55, 23,  8,  0,  0,  0,  0,  0,  6, 70,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{ 82, 67, 35, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 81, 83, 68, 36, 17, 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 82, 84, 65, 69, 33, 37, 18, 20,  0,  0,  0,  0,  0,  0,  0,  0},{ 83, 85, 66, 70, 34, 38, 19, 21,  0,  0,  0,  0,  0,  0,  0,  0},{ 84, 86, 67, 71, 35, 39, 20, 22,  0,  0,  0,  0,  0,  0,  0,  0},{ 85, 87, 68, 72, 36, 40, 21, 23,  0,  0,  0,  0,  0,  0,  0,  0},{ 86, 88, 69, 73, 37, 41, 22, 24,  0,  0,  0,  0,  0,  0,  0,  0},{ 87, 89, 70, 38, 23, 25,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 88, 71, 39, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{ 98, 83, 51, 34,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 97, 99, 84, 52, 33, 35,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{ 98,100, 81, 85, 49, 53, 34, 36,  0, 32, 36,  0,  0,  0,  0,  0},{ 99,101, 82, 86, 50, 54, 35, 37,  0,  0,  0,  0,  0,  0,  0,  0},{100,102, 83, 87, 51, 55, 36, 38,  0,  0,  0,  0,  0,  0,  0,  0},{101,103, 84, 88, 52, 56, 37, 39,  0,  0,  0,  0,  0,  0,  0,  0},{102,104, 85, 89, 53, 57, 38, 40,  0, 36, 40,  0,  0,  0,  0,  0},{103,105, 86, 54, 39, 41,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{104, 87, 55, 40,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{114, 99, 67, 50,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{113,115,100, 68, 49, 51,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{114,116, 97,101, 65, 69, 50, 52,  0,112,116,  0,  0,  0,  0,  0},{115,117, 98,102, 66, 70, 51, 53,  0,  0,  0,  0,  0,  0,  0,  0},{116,118, 99,103, 67, 71, 52, 54,  0,  0,  0,  0,  0,  0,  0,  0},{117,119,100,104, 68, 72, 53, 55,  0,  0,  0,  0,  0,  0,  0,  0},{118,120,101,105, 69, 73, 54, 56,  0, 116,120, 0,  0,  0,  0,  0},{119,121,102, 70, 55, 57,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{120,103, 71, 56,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{130,115, 83, 66,  0,  0,  0,  0,  0, 82,146,  0,  0,  0,  0,  0},{129,131,116, 84, 65, 67,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{130,132,113,117, 81, 85, 66, 68,  0,  0,  0,  0,  0,  0,  0,  0},{131,133,114,118, 82, 86, 67, 69,  0,  0,  0,  0,  0,  0,  0,  0},{132,134,115,119, 83, 87, 68, 70,  0,  0,  0,  0,  0,  0,  0,  0},{133,135,116,120, 84, 88, 69, 71,  0,  0,  0,  0,  0,  0,  0,  0},{134,136,117,121, 85, 89, 70, 72,  0,  0,  0,  0,  0,  0,  0,  0},{135,137,118, 86, 71, 73,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{136,119, 87, 72,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{146,131, 99, 82,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{145,147,132,100, 81, 83,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{146,148,129,133, 97,101, 82, 84,  0,  0,  0,  0,  0,  0,  0,  0},{147,149,130,134, 98,102, 83, 85,  0,132,  0,  0,  0,  0,  0,  0},{148,150,131,135, 99,103, 84, 86,  0,  82,86,146,150,  0,  0,  0},{149,151,132,136,100,104, 85, 87,  0,132,  0,  0,  0,  0,  0,  0},{150,152,133,137,101,105, 86, 88,  0,  0,  0,  0,  0,  0,  0,  0},{151,153,134,102, 87, 89,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{152,135,103, 88,  0,  0,  0,  0,  0, 86,150,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{147,115, 98,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{148,116, 97, 99,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{145,149,113,117, 98,100,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{146,150,114,118, 99,101,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{147,151,115,119,100,102,  0,  0,  0,  115,117,147,149,0,  0,  0},{148,152,116,120,101,103,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{149,153,117,121,102,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{150,118,103,105,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{151,119,104,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
{131,114,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{132,113,115,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{129,133,114,116,  0,  0,  0,  0,  0,112,116,  0,  0,  0,  0,  0},{130,134,115,117,  0,  0,  0,  0,  0,132,  0,  0,  0,  0,  0,  0},{131,135,116,118,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{132,136,117,119,  0,  0,  0,  0,  0,132,  0,  0,  0,  0,  0,  0},{133,137,118,120,  0,  0,  0,  0,  0, 116,120, 0,  0,  0,  0,  0},{134,119,121,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},{135,120,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}};
*/
static const unsigned char ele_pos[12]={2,6, 32,40, 66,70, 82,86, 112,120, 146,150};
static const unsigned char ele_moves[12][2]={{32, 36},{36, 40},{2, 66},{6, 70},{32, 36},{36, 40},
{112,116},{116,120},{82,146},{86,150},{112,116},{116,120}};
static const unsigned char advele_center_pos[4]={20,36,116,132};
static const unsigned char advele_center_moves[4][4]={{3,  5, 35, 37},{2,  6, 66, 70,},{82,86,146,150},{115,117,147,149}};
static const unsigned char bis_pos[8]={3,5,35,37, 115,117,147,149};
static const unsigned char bis_moves[8]={20,20,20,20, 132,132,132,132};

//unsigned char g_ElephantEyes[BOARD_SIZE][4];
unsigned char g_HorseLegs[BOARD_SIZE-7][8];
/*
 = {
{ 16,  1,  0,  0,  0,  0,  0,  0},{ 17, 17,  2,  0,  0,  0,  0,  0},{ 18, 18,  1,  3,  0,  0,  0,  0},{ 19, 19,  2,  4,  0,  0,  0,  0},{ 20, 20,  3,  5,  0,  0,  0,  0},{ 21, 21,  4,  6,  0,  0,  0,  0},{ 22, 22,  5,  7,  0,  0,  0,  0},{ 23, 23,  6,  0,  0,  0,  0,  0},{ 24,  7,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{ 32, 17, 17,  0,  0,  0,  0,  0},{ 33, 33, 18, 18,  0,  0,  0,  0},{ 34, 34, 17, 19, 17, 19,  0,  0},{ 35, 35, 18, 20, 18, 20,  0,  0},{ 36, 36, 19, 21, 19, 21,  0,  0},{ 37, 37, 20, 22, 20, 22,  0,  0},{ 38, 38, 21, 23, 21, 23,  0,  0},{ 39, 39, 22, 22,  0,  0,  0,  0},{ 40, 23, 23,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{ 48, 33, 33, 16,  0,  0,  0,  0},{ 49, 49, 34, 34, 17, 17,  0,  0},{ 50, 50, 33, 35, 33, 35, 18, 18},{ 51, 51, 34, 36, 34, 36, 19, 19},{ 52, 52, 35, 37, 35, 37, 20, 20},{ 53, 53, 36, 38, 36, 38, 21, 21},{ 54, 54, 37, 39, 37, 39, 22, 22},{ 55, 55, 38, 38, 23, 23,  0,  0},{ 56, 39, 39, 24,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{ 64, 49, 49, 32,  0,  0,  0,  0},{ 65, 65, 50, 50, 33, 33,  0,  0},{ 66, 66, 49, 51, 49, 51, 34, 34},{ 67, 67, 50, 52, 50, 52, 35, 35},{ 68, 68, 51, 53, 51, 53, 36, 36},{ 69, 69, 52, 54, 52, 54, 37, 37},{ 70, 70, 53, 55, 53, 55, 38, 38},{ 71, 71, 54, 54, 39, 39,  0,  0},{ 72, 55, 55, 40,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{ 80, 65, 65, 48,  0,  0,  0,  0},{ 81, 81, 66, 66, 49, 49,  0,  0},{ 82, 82, 65, 67, 65, 67, 50, 50},{ 83, 83, 66, 68, 66, 68, 51, 51},{ 84, 84, 67, 69, 67, 69, 52, 52},{ 85, 85, 68, 70, 68, 70, 53, 53},{ 86, 86, 69, 71, 69, 71, 54, 54},{ 87, 87, 70, 70, 55, 55,  0,  0},{ 88, 71, 71, 56,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{ 96, 81, 81, 64,  0,  0,  0,  0},{ 97, 97, 82, 82, 65, 65,  0,  0},{ 98, 98, 81, 83, 81, 83, 66, 66},{ 99, 99, 82, 84, 82, 84, 67, 67},{100,100, 83, 85, 83, 85, 68, 68},{101,101, 84, 86, 84, 86, 69, 69},{102,102, 85, 87, 85, 87, 70, 70},{103,103, 86, 86, 71, 71,  0,  0},{104, 87, 87, 72,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{112, 97, 97, 80,  0,  0,  0,  0},{113,113, 98, 98, 81, 81,  0,  0},{114,114, 97, 99, 97, 99, 82, 82},{115,115, 98,100, 98,100, 83, 83},{116,116, 99,101, 99,101, 84, 84},{117,117,100,102,100,102, 85, 85},{118,118,101,103,101,103, 86, 86},{119,119,102,102, 87, 87,  0,  0},{120,103,103, 88,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{128,113,113, 96,  0,  0,  0,  0},{129,129,114,114, 97, 97,  0,  0},{130,130,113,115,113,115, 98, 98},{131,131,114,116,114,116, 99, 99},{132,132,115,117,115,117,100,100},{133,133,116,118,116,118,101,101},{134,134,117,119,117,119,102,102},{135,135,118,118,103,103,  0,  0},{136,119,119,104,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{129,129,112,  0,  0,  0,  0,  0},{130,130,113,113,  0,  0,  0,  0},{129,131,129,131,114,114,  0,  0},{130,132,130,132,115,115,  0,  0},{131,133,131,133,116,116,  0,  0},{132,134,132,134,117,117,  0,  0},{133,135,133,135,118,118,  0,  0},{134,134,119,119,  0,  0,  0,  0},{135,135,120,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},{  0,  0,  0,  0,  0,  0,  0,  0},
{145,128,  0,  0,  0,  0,  0,  0},{146,129,129,  0,  0,  0,  0,  0},{145,147,130,130,  0,  0,  0,  0},{146,148,131,131,  0,  0,  0,  0},{147,149,132,132,  0,  0,  0,  0},{148,150,133,133,  0,  0,  0,  0},{149,151,134,134,  0,  0,  0,  0},{150,135,135,  0,  0,  0,  0,  0},{151,136,  0,  0,  0,  0,  0,  0}};
*/
#ifdef HORSECK
unsigned char g_KnightChecks[BOARD_SIZE-7][18][2];
#endif
/*
unsigned char g_RookMoves[BOARD_SIZE-7][4][16] =
{{{9, 18,27,36,45,54,63,72,81,255,0,0,0,0,0,0}, {1, 2, 3, 4, 5, 6, 7, 8,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}   },
    {{10,19,28,37,46,55,64,73,82,255,0,0,0,0,0,0}, {0,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}, {2, 3, 4, 5, 6, 7, 8,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{11,20,29,38,47,56,65,74,83,255,0,0,0,0,0,0}, {1, 0,255,255,255,255,255,255,255,255,0,0,0,0,0,0}, {3, 4, 5, 6, 7, 8,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{12,21,30,39,48,57,66,75,84,255,0,0,0,0,0,0}, {2, 1, 0,255,255,255,255,255,255,255,0,0,0,0,0,0}, {4, 5, 6, 7, 8,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{13,22,31,40,49,58,67,76,85,255,0,0,0,0,0,0}, {3, 2, 1, 0,255,255,255,255,255,255,0,0,0,0,0,0}, {5, 6, 7, 8,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{14,23,32,41,50,59,68,77,86,255,0,0,0,0,0,0}, {4, 3, 2, 1, 0,255,255,255,255,255,0,0,0,0,0,0}, {6, 7, 8,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{15,24,33,42,51,60,69,78,87,255,0,0,0,0,0,0}, {5, 4, 3, 2, 1, 0,255,255,255,255,0,0,0,0,0,0}, {7, 8,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{16,25,34,43,52,61,70,79,88,255,0,0,0,0,0,0}, {6, 5, 4, 3, 2, 1, 0,255,255,255,0,0,0,0,0,0}, {8,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{17,26,35,44,53,62,71,80,89,255,0,0,0,0,0,0}, {7, 6, 5, 4, 3, 2, 1, 0,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},



    {{  0,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{18,27,36,45,54,63,72,81,255,255,0,0,0,0,0,0},{10,11,12,13,14,15,16,17,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 1,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{19,28,37,46,55,64,73,82,255,255,0,0,0,0,0,0}, {9,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{11,12,13,14,15,16,17,255,255,255,0,0,0,0,0,0}},
    {{ 2,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{20,29,38,47,56,65,74,83,255,255,0,0,0,0,0,0}, {10,9,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{12,13,14,15,16,17,255,255,255,255,0,0,0,0,0,0}},
    {{ 3,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{21,30,39,48,57,66,75,84,255,255,0,0,0,0,0,0}, {11,10,9,255,255,255,255,255,255,255,0,0,0,0,0,0},{13,14,15,16,17,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 4,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{22,31,40,49,58,67,76,85,255,255,0,0,0,0,0,0}, {12,11,10,9,255,255,255,255,255,255,0,0,0,0,0,0},{14,15,16,17,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 5,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{23,32,41,50,59,68,77,86,255,255,0,0,0,0,0,0}, {13,12,11,10,9,255,255,255,255,255,0,0,0,0,0,0},{15,16,17,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 6,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{24,33,42,51,60,69,78,87,255,255,0,0,0,0,0,0}, {14,13,12,11,10,9,255,255,255,255,0,0,0,0,0,0},{16,17,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 7,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{25,34,43,52,61,70,79,88,255,255,0,0,0,0,0,0}, {15,14,13,12,11,10,9,255,255,255,0,0,0,0,0,0},{17,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{ 8,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{26,35,44,53,62,71,80,89,255,255,0,0,0,0,0,0}, {16,15,14,13,12,11,10,9,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{  9, 0,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{27,36,45,54,63,72,81,255,255,255,0,0,0,0,0,0},{19,20,21,22,23,24,25,26,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{10, 1,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{28,37,46,55,64,73,82,255,255,255,0,0,0,0,0,0},{18,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{20,21,22,23,24,25,26,255,255,255,0,0,0,0,0,0}},
    {{11, 2,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{29,38,47,56,65,74,83,255,255,255,0,0,0,0,0,0},{19,18,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{21,22,23,24,25,26,255,255,255,255,0,0,0,0,0,0}},
    {{12, 3,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{30,39,48,57,66,75,84,255,255,255,0,0,0,0,0,0},{20,19,18,255,255,255,255,255,255,255,0,0,0,0,0,0},{22,23,24,25,26,255,255,255,255,255,0,0,0,0,0,0}},
    {{13, 4,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{31,40,49,58,67,76,85,255,255,255,0,0,0,0,0,0},{21,20,19,18,255,255,255,255,255,255,0,0,0,0,0,0},{23,24,25,26,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{14, 5,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{32,41,50,59,68,77,86,255,255,255,0,0,0,0,0,0},{22,21,20,19,18,255,255,255,255,255,0,0,0,0,0,0},{24,25,26,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{15, 6,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{33,42,51,60,69,78,87,255,255,255,0,0,0,0,0,0},{23,22,21,20,19,18,255,255,255,255,0,0,0,0,0,0},{25,26,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{16, 7,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{34,43,52,61,70,79,88,255,255,255,0,0,0,0,0,0},{24,23,22,21,20,19,18,255,255,255,0,0,0,0,0,0},{26,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{17, 8,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{35,44,53,62,71,80,89,255,255,255,0,0,0,0,0,0},{25,24,23,22,21,20,19,18,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 18, 9, 0,255,255,255,255,255,255,255,0,0,0,0,0,0},{36,45,54,63,72,81,255,255,255,255,0,0,0,0,0,0},{28,29,30,31,32,33,34,35,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{19,10, 1,255,255,255,255,255,255,255,0,0,0,0,0,0},{37,46,55,64,73,82,255,255,255,255,0,0,0,0,0,0},{27,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{29,30,31,32,33,34,35,255,255,255,0,0,0,0,0,0}},
    {{20,11, 2,255,255,255,255,255,255,255,0,0,0,0,0,0},{38,47,56,65,74,83,255,255,255,255,0,0,0,0,0,0},{28,27,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{30,31,32,33,34,35,255,255,255,255,0,0,0,0,0,0}},
    {{21,12, 3,255,255,255,255,255,255,255,0,0,0,0,0,0},{39,48,57,66,75,84,255,255,255,255,0,0,0,0,0,0},{29,28,27,255,255,255,255,255,255,255,0,0,0,0,0,0},{31,32,33,34,35,255,255,255,255,255,0,0,0,0,0,0}},
    {{22,13, 4,255,255,255,255,255,255,255,0,0,0,0,0,0},{40,49,58,67,76,85,255,255,255,255,0,0,0,0,0,0},{30,29,28,27,255,255,255,255,255,255,0,0,0,0,0,0},{32,33,34,35,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{23,14, 5,255,255,255,255,255,255,255,0,0,0,0,0,0},{41,50,59,68,77,86,255,255,255,255,0,0,0,0,0,0},{31,30,29,28,27,255,255,255,255,255,0,0,0,0,0,0},{33,34,35,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{24,15, 6,255,255,255,255,255,255,255,0,0,0,0,0,0},{42,51,60,69,78,87,255,255,255,255,0,0,0,0,0,0},{32,31,30,29,28,27,255,255,255,255,0,0,0,0,0,0},{34,35,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{25,16, 7,255,255,255,255,255,255,255,0,0,0,0,0,0},{43,52,61,70,79,88,255,255,255,255,0,0,0,0,0,0},{33,32,31,30,29,28,27,255,255,255,0,0,0,0,0,0},{35,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{26,17, 8,255,255,255,255,255,255,255,0,0,0,0,0,0},{44,53,62,71,80,89,255,255,255,255,0,0,0,0,0,0},{34,33,32,31,30,29,28,27,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 27,18, 9, 0,255,255,255,255,255,255,0,0,0,0,0,0},{45,54,63,72,81,255,255,255,255,255,0,0,0,0,0,0},{37,38,39,40,41,42,43,44,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{28,19,10, 1,255,255,255,255,255,255,0,0,0,0,0,0},{46,55,64,73,82,255,255,255,255,255,0,0,0,0,0,0},{36,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{38,39,40,41,42,43,44,255,255,255,0,0,0,0,0,0}},
    {{29,20,11, 2,255,255,255,255,255,255,0,0,0,0,0,0},{47,56,65,74,83,255,255,255,255,255,0,0,0,0,0,0},{37,36,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{39,40,41,42,43,44,255,255,255,255,0,0,0,0,0,0}},
    {{30,21,12, 3,255,255,255,255,255,255,0,0,0,0,0,0},{48,57,66,75,84,255,255,255,255,255,0,0,0,0,0,0},{38,37,36,255,255,255,255,255,255,255,0,0,0,0,0,0},{40,41,42,43,44,255,255,255,255,255,0,0,0,0,0,0}},
    {{31,22,13, 4,255,255,255,255,255,255,0,0,0,0,0,0},{49,58,67,76,85,255,255,255,255,255,0,0,0,0,0,0},{39,38,37,36,255,255,255,255,255,255,0,0,0,0,0,0},{41,42,43,44,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{32,23,14, 5,255,255,255,255,255,255,0,0,0,0,0,0},{50,59,68,77,86,255,255,255,255,255,0,0,0,0,0,0},{40,39,38,37,36,255,255,255,255,255,0,0,0,0,0,0},{42,43,44,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{33,24,15, 6,255,255,255,255,255,255,0,0,0,0,0,0},{51,60,69,78,87,255,255,255,255,255,0,0,0,0,0,0},{41,40,39,38,37,36,255,255,255,255,0,0,0,0,0,0},{43,44,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{34,25,16, 7,255,255,255,255,255,255,0,0,0,0,0,0},{52,61,70,79,88,255,255,255,255,255,0,0,0,0,0,0},{42,41,40,39,38,37,36,255,255,255,0,0,0,0,0,0},{44,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{35,26,17, 8,255,255,255,255,255,255,0,0,0,0,0,0},{53,62,71,80,89,255,255,255,255,255,0,0,0,0,0,0},{43,42,41,40,39,38,37,36,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 36,27,18, 9, 0,255,255,255,255,255,0,0,0,0,0,0},{54,63,72,81,255,255,255,255,255,255,0,0,0,0,0,0},{46,47,48,49,50,51,52,53,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{37,28,19,10, 1,255,255,255,255,255,0,0,0,0,0,0},{55,64,73,82,255,255,255,255,255,255,0,0,0,0,0,0},{45,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{47,48,49,50,51,52,53,255,255,255,0,0,0,0,0,0}},
    {{38,29,20,11, 2,255,255,255,255,255,0,0,0,0,0,0},{56,65,74,83,255,255,255,255,255,255,0,0,0,0,0,0},{46,45,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{48,49,50,51,52,53,255,255,255,255,0,0,0,0,0,0}},
    {{39,30,21,12, 3,255,255,255,255,255,0,0,0,0,0,0},{57,66,75,84,255,255,255,255,255,255,0,0,0,0,0,0},{47,46,45,255,255,255,255,255,255,255,0,0,0,0,0,0},{49,50,51,52,53,255,255,255,255,255,0,0,0,0,0,0}},
    {{40,31,22,13, 4,255,255,255,255,255,0,0,0,0,0,0},{58,67,76,85,255,255,255,255,255,255,0,0,0,0,0,0},{48,47,46,45,255,255,255,255,255,255,0,0,0,0,0,0},{50,51,52,53,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{41,32,23,14, 5,255,255,255,255,255,0,0,0,0,0,0},{59,68,77,86,255,255,255,255,255,255,0,0,0,0,0,0},{49,48,47,46,45,255,255,255,255,255,0,0,0,0,0,0},{51,52,53,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{42,33,24,15, 6,255,255,255,255,255,0,0,0,0,0,0},{60,69,78,87,255,255,255,255,255,255,0,0,0,0,0,0},{50,49,48,47,46,45,255,255,255,255,0,0,0,0,0,0},{52,53,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{43,34,25,16, 7,255,255,255,255,255,0,0,0,0,0,0},{61,70,79,88,255,255,255,255,255,255,0,0,0,0,0,0},{51,50,49,48,47,46,45,255,255,255,0,0,0,0,0,0},{53,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{44,35,26,17, 8,255,255,255,255,255,0,0,0,0,0,0},{62,71,80,89,255,255,255,255,255,255,0,0,0,0,0,0},{52,51,50,49,48,47,46,45,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 45,36,27,18, 9, 0,255,255,255,255,0,0,0,0,0,0},{63,72,81,255,255,255,255,255,255,255,0,0,0,0,0,0},{55,56,57,58,59,60,61,62,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{46,37,28,19,10, 1,255,255,255,255,0,0,0,0,0,0},{64,73,82,255,255,255,255,255,255,255,0,0,0,0,0,0},{54,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{56,57,58,59,60,61,62,255,255,255,0,0,0,0,0,0}},
    {{47,38,29,20,11, 2,255,255,255,255,0,0,0,0,0,0},{65,74,83,255,255,255,255,255,255,255,0,0,0,0,0,0},{55,54,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{57,58,59,60,61,62,255,255,255,255,0,0,0,0,0,0}},
    {{48,39,30,21,12, 3,255,255,255,255,0,0,0,0,0,0},{66,75,84,255,255,255,255,255,255,255,0,0,0,0,0,0},{56,55,54,255,255,255,255,255,255,255,0,0,0,0,0,0},{58,59,60,61,62,255,255,255,255,255,0,0,0,0,0,0}},
    {{49,40,31,22,13, 4,255,255,255,255,0,0,0,0,0,0},{67,76,85,255,255,255,255,255,255,255,0,0,0,0,0,0},{57,56,55,54,255,255,255,255,255,255,0,0,0,0,0,0},{59,60,61,62,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{50,41,32,23,14, 5,255,255,255,255,0,0,0,0,0,0},{68,77,86,255,255,255,255,255,255,255,0,0,0,0,0,0},{58,57,56,55,54,255,255,255,255,255,0,0,0,0,0,0},{60,61,62,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{51,42,33,24,15, 6,255,255,255,255,0,0,0,0,0,0},{69,78,87,255,255,255,255,255,255,255,0,0,0,0,0,0},{59,58,57,56,55,54,255,255,255,255,0,0,0,0,0,0},{61,62,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{52,43,34,25,16, 7,255,255,255,255,0,0,0,0,0,0},{70,79,88,255,255,255,255,255,255,255,0,0,0,0,0,0},{60,59,58,57,56,55,54,255,255,255,0,0,0,0,0,0},{62,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{53,44,35,26,17, 8,255,255,255,255,0,0,0,0,0,0},{71,80,89,255,255,255,255,255,255,255,0,0,0,0,0,0},{61,60,59,58,57,56,55,54,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 54,45,36,27,18, 9, 0,255,255,255,0,0,0,0,0,0},{72,81,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{64,65,66,67,68,69,70,71,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{55,46,37,28,19,10, 1,255,255,255,0,0,0,0,0,0},{73,82,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{63,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{65,66,67,68,69,70,71,255,255,255,0,0,0,0,0,0}},
    {{56,47,38,29,20,11, 2,255,255,255,0,0,0,0,0,0},{74,83,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{64,63,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{66,67,68,69,70,71,255,255,255,255,0,0,0,0,0,0}},
    {{57,48,39,30,21,12, 3,255,255,255,0,0,0,0,0,0},{75,84,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{65,64,63,255,255,255,255,255,255,255,0,0,0,0,0,0},{67,68,69,70,71,255,255,255,255,255,0,0,0,0,0,0}},
    {{58,49,40,31,22,13, 4,255,255,255,0,0,0,0,0,0},{76,85,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{66,65,64,63,255,255,255,255,255,255,0,0,0,0,0,0},{68,69,70,71,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{59,50,41,32,23,14, 5,255,255,255,0,0,0,0,0,0},{77,86,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{67,66,65,64,63,255,255,255,255,255,0,0,0,0,0,0},{69,70,71,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{60,51,42,33,24,15, 6,255,255,255,0,0,0,0,0,0},{78,87,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{68,67,66,65,64,63,255,255,255,255,0,0,0,0,0,0},{70,71,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{61,52,43,34,25,16, 7,255,255,255,0,0,0,0,0,0},{79,88,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{69,68,67,66,65,64,63,255,255,255,0,0,0,0,0,0},{71,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{62,53,44,35,26,17, 8,255,255,255,0,0,0,0,0,0},{80,89,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{70,69,68,67,66,65,64,63,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 63,54,45,36,27,18, 9, 0,255,255,0,0,0,0,0,0},{81,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{73,74,75,76,77,78,79,80,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{64,55,46,37,28,19,10, 1,255,255,0,0,0,0,0,0},{82,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{72,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{74,75,76,77,78,79,80,255,255,255,0,0,0,0,0,0}},
    {{65,56,47,38,29,20,11, 2,255,255,0,0,0,0,0,0},{83,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{73,72,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{75,76,77,78,79,80,255,255,255,255,0,0,0,0,0,0}},
    {{66,57,48,39,30,21,12, 3,255,255,0,0,0,0,0,0},{84,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{74,73,72,255,255,255,255,255,255,255,0,0,0,0,0,0},{76,77,78,79,80,255,255,255,255,255,0,0,0,0,0,0}},
    {{67,58,49,40,31,22,13, 4,255,255,0,0,0,0,0,0},{85,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{75,74,73,72,255,255,255,255,255,255,0,0,0,0,0,0},{77,78,79,80,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{68,59,50,41,32,23,14, 5,255,255,0,0,0,0,0,0},{86,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{76,75,74,73,72,255,255,255,255,255,0,0,0,0,0,0},{78,79,80,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{69,60,51,42,33,24,15, 6,255,255,0,0,0,0,0,0},{87,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{77,76,75,74,73,72,255,255,255,255,0,0,0,0,0,0},{79,80,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{70,61,52,43,34,25,16, 7,255,255,0,0,0,0,0,0},{88,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{78,77,76,75,74,73,72,255,255,255,0,0,0,0,0,0},{80,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{71,62,53,44,35,26,17, 8,255,255,0,0,0,0,0,0},{89,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{79,78,77,76,75,74,73,72,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},


    {{ 72,63,54,45,36,27,18, 9, 0,255,0,0,0,0,0,0},{82,83,84,85,86,87,88,89,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{73,64,55,46,37,28,19,10, 1,255,0,0,0,0,0,0},{81,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{83,84,85,86,87,88,89,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{74,65,56,47,38,29,20,11, 2,255,0,0,0,0,0,0},{82,81,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{84,85,86,87,88,89,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{75,66,57,48,39,30,21,12, 3,255,0,0,0,0,0,0},{83,82,81,255,255,255,255,255,255,255,0,0,0,0,0,0},{85,86,87,88,89,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{76,67,58,49,40,31,22,13, 4,255,0,0,0,0,0,0},{84,83,82,81,255,255,255,255,255,255,0,0,0,0,0,0},{86,87,88,89,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{77,68,59,50,41,32,23,14, 5,255,0,0,0,0,0,0},{85,84,83,82,81,255,255,255,255,255,0,0,0,0,0,0},{87,88,89,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{78,69,60,51,42,33,24,15, 6,255,0,0,0,0,0,0},{86,85,84,83,82,81,255,255,255,255,0,0,0,0,0,0},{88,89,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{79,70,61,52,43,34,25,16, 7,255,0,0,0,0,0,0},{87,86,85,84,83,82,81,255,255,255,0,0,0,0,0,0},{89,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}},
    {{80,71,62,53,44,35,26,17, 8,255,0,0,0,0,0,0},{88,87,86,85,84,83,82,81,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0},{255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0}}
};
*/
/*
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}};
*/

/*
unsigned char g_PawnMoves[BOARD_SIZE-7][2][4] = {
    {{255,  0,  0,  0},{  1,255,  0,  0}},{{255,  0,  0,  0},{  0,  2,255,  0}},{{255,  0,  0,  0},{  1,  3,255,  0}},{{255,  0,  0,  0},{  2,  4,255,  0}},{{255,  0,  0,  0},{  3,  5,255,  0}},{{255,  0,  0,  0},{  4,  6,255,  0}},{{255,  0,  0,  0},{  5,  7,255,  0}},{{255,  0,  0,  0},{  6,  8,255,  0}},{{255,  0,  0,  0},{  7,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{255,  0,  0,  0},{  0, 17,255,  0}},{{255,  0,  0,  0},{  1, 16, 18,255}},{{255,  0,  0,  0},{  2, 17, 19,255}},{{255,  0,  0,  0},{  3, 18, 20,255}},{{255,  0,  0,  0},{  4, 19, 21,255}},{{255,  0,  0,  0},{  5, 20, 22,255}},{{255,  0,  0,  0},{  6, 21, 23,255}},{{255,  0,  0,  0},{  7, 22, 24,255}},{{255,  0,  0,  0},{  8, 23,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{255,  0,  0,  0},{ 16, 33,255,  0}},{{255,  0,  0,  0},{ 17, 32, 34,255}},{{255,  0,  0,  0},{ 18, 33, 35,255}},{{255,  0,  0,  0},{ 19, 34, 36,255}},{{255,  0,  0,  0},{ 20, 35, 37,255}},{{255,  0,  0,  0},{ 21, 36, 38,255}},{{255,  0,  0,  0},{ 22, 37, 39,255}},{{255,  0,  0,  0},{ 23, 38, 40,255}},{{255,  0,  0,  0},{ 24, 39,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 64,255,  0,  0},{ 32, 49,255,  0}},{{255,  0,  0,  0},{ 33, 48, 50,255}},{{ 66,255,  0,  0},{ 34, 49, 51,255}},{{255,  0,  0,  0},{ 35, 50, 52,255}},{{ 68,255,  0,  0},{ 36, 51, 53,255}},{{255,  0,  0,  0},{ 37, 52, 54,255}},{{ 70,255,  0,  0},{ 38, 53, 55,255}},{{255,  0,  0,  0},{ 39, 54, 56,255}},{{ 72,255,  0,  0},{ 40, 55,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 80,255,  0,  0},{ 48, 65,255,  0}},{{255,  0,  0,  0},{ 49, 64, 66,255}},{{ 82,255,  0,  0},{ 50, 65, 67,255}},{{255,  0,  0,  0},{ 51, 66, 68,255}},{{ 84,255,  0,  0},{ 52, 67, 69,255}},{{255,  0,  0,  0},{ 53, 68, 70,255}},{{ 86,255,  0,  0},{ 54, 69, 71,255}},{{255,  0,  0,  0},{ 55, 70, 72,255}},{{ 88,255,  0,  0},{ 56, 71,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 96, 81,255,  0},{ 64,255,  0,  0}},{{ 97, 80, 82,255},{255,  0,  0,  0}},{{ 98, 81, 83,255},{ 66,255,  0,  0}},{{ 99, 82, 84,255},{255,  0,  0,  0}},{{100, 83, 85,255},{ 68,255,  0,  0}},{{101, 84, 86,255},{255,  0,  0,  0}},{{102, 85, 87,255},{ 70,255,  0,  0}},{{103, 86, 88,255},{255,  0,  0,  0}},{{104, 87,255,  0},{ 72,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{112, 97,255,  0},{ 80,255,  0,  0}},{{113, 96, 98,255},{255,  0,  0,  0}},{{114, 97, 99,255},{ 82,255,  0,  0}},{{115, 98,100,255},{255,  0,  0,  0}},{{116, 99,101,255},{ 84,255,  0,  0}},{{117,100,102,255},{255,  0,  0,  0}},{{118,101,103,255},{ 86,255,  0,  0}},{{119,102,104,255},{255,  0,  0,  0}},{{120,103,255,  0},{ 88,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{128,113,255,  0},{255,  0,  0,  0}},{{129,112,114,255},{255,  0,  0,  0}},{{130,113,115,255},{255,  0,  0,  0}},{{131,114,116,255},{255,  0,  0,  0}},{{132,115,117,255},{255,  0,  0,  0}},{{133,116,118,255},{255,  0,  0,  0}},{{134,117,119,255},{255,  0,  0,  0}},{{135,118,120,255},{255,  0,  0,  0}},{{136,119,255,  0},{255,  0,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{144,129,255,  0},{255,  0,  0,  0}},{{145,128,130,255},{255,  0,  0,  0}},{{146,129,131,255},{255,  0,  0,  0}},{{147,130,132,255},{255,  0,  0,  0}},{{148,131,133,255},{255,  0,  0,  0}},{{149,132,134,255},{255,  0,  0,  0}},{{150,133,135,255},{255,  0,  0,  0}},{{151,134,136,255},{255,  0,  0,  0}},{{152,135,255,  0},{255,  0,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{145,255,  0,  0},{255,  0,  0,  0}},{{144,146,255,  0},{255,  0,  0,  0}},{{145,147,255,  0},{255,  0,  0,  0}},{{146,148,255,  0},{255,  0,  0,  0}},{{147,149,255,  0},{255,  0,  0,  0}},{{148,150,255,  0},{255,  0,  0,  0}},{{149,151,255,  0},{255,  0,  0,  0}},{{150,152,255,  0},{255,  0,  0,  0}},{{151,255,  0,  0},{255,  0,  0,  0}}
}; //,{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}}};
*/
unsigned char g_PawnMoves[BOARD_SIZE-7][2][4] = {
{{  0,  1,  1,  1},{  2,  0,  1,  1}},{{  0,  1,  1,  1},{  1,  3,  0,  1}},{{  0,  1,  1,  1},{  2,  4,  0,  1}},{{  0,  0,  4, 19},{  3,  5,  0,  1}},{{  0,  5,  3, 20},{  4,  6,  0,  1}},{{  0,  0,  4, 21},{  5,  7,  0,  1}},{{  0,  1,  1,  1},{  6,  8,  0,  1}},{{  0,  1,  1,  1},{  7,  9,  0,  1}},{{  0,  1,  1,  1},{  8,  0,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{  0,  1,  1,  1},{  1, 18,  0,  1}},{{  0,  1,  1,  1},{  2, 17, 19,  0}},{{  0,  1,  1,  1},{  3, 18, 20,  0}},{{  0, 20, 35,  3},{  4, 19, 21,  0}},{{ 21, 19, 36,  4},{  5, 20, 22,  0}},{{  0, 20, 37,  5},{  6, 21, 23,  0}},{{  0,  1,  1,  1},{  7, 22, 24,  0}},{{  0,  1,  1,  1},{  8, 23, 25,  0}},{{  0,  1,  1,  1},{  9, 24,  0,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{  0,  1,  1,  1},{ 17, 34,  0,  1}},{{  0,  1,  1,  1},{ 18, 33, 35,  0}},{{  0,  1,  1,  1},{ 19, 34, 36,  0}},{{  0,  0, 36, 19},{ 20, 35, 37,  0}},{{  0, 37, 35, 20},{ 21, 36, 38,  0}},{{  0,  0, 36, 21},{ 22, 37, 39,  0}},{{  0,  1,  1,  1},{ 23, 38, 40,  0}},{{  0,  1,  1,  1},{ 24, 39, 41,  0}},{{  0,  1,  1,  1},{ 25, 40,  0,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{ 65,  0,  1,  1},{ 33, 50,  0,  1}},{{  0,  1,  1,  1},{ 34, 49, 51,  0}},{{ 67,  0,  1,  1},{ 35, 50, 52,  0}},{{  0,  1,  1,  1},{ 36, 51, 53,  0}},{{ 69,  0,  1,  1},{ 37, 52, 54,  0}},{{  0,  1,  1,  1},{ 38, 53, 55,  0}},{{ 71,  0,  1,  1},{ 39, 54, 56,  0}},{{  0,  1,  1,  1},{ 40, 55, 57,  0}},{{ 73,  0,  1,  1},{ 41, 56,  0,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{ 81,  0,  1,  1},{ 49, 66,  0,  1}},{{  0,  1,  1,  1},{ 50, 65, 67,  0}},{{ 83,  0,  1,  1},{ 51, 66, 68,  0}},{{  0,  1,  1,  1},{ 52, 67, 69,  0}},{{ 85,  0,  1,  1},{ 53, 68, 70,  0}},{{  0,  1,  1,  1},{ 54, 69, 71,  0}},{{ 87,  0,  1,  1},{ 55, 70, 72,  0}},{{  0,  1,  1,  1},{ 56, 71, 73,  0}},{{ 89,  0,  1,  1},{ 57, 72,  0,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{ 97, 82,  0,  1},{ 65,  0,  1,  1}},{{ 98, 81, 83,  0},{  0,  1,  1,  1}},{{ 99, 82, 84,  0},{ 67,  0,  1,  1}},{{100, 83, 85,  0},{  0,  1,  1,  1}},{{101, 84, 86,  0},{ 69,  0,  1,  1}},{{102, 85, 87,  0},{  0,  1,  1,  1}},{{103, 86, 88,  0},{ 71,  0,  1,  1}},{{104, 87, 89,  0},{  0,  1,  1,  1}},{{105, 88,  0,  1},{ 73,  0,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{113, 98,  0,  1},{ 81,  0,  1,  1}},{{114, 97, 99,  0},{  0,  1,  1,  1}},{{115, 98,100,  0},{ 83,  0,  1,  1}},{{116, 99,101,  0},{  0,  1,  1,  1}},{{117,100,102,  0},{ 85,  0,  1,  1}},{{118,101,103,  0},{  0,  1,  1,  1}},{{119,102,104,  0},{ 87,  0,  1,  1}},{{120,103,105,  0},{  0,  1,  1,  1}},{{121,104,  0,  1},{ 89,  0,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{129,114,  0,  1},{  0,  1,  1,  1}},{{130,113,115,  0},{  0,  1,  1,  1}},{{131,114,116,  0},{  0,  1,  1,  1}},{{132,115,117,  0},{  0,  0,116,131}},{{133,116,118,  0},{  0,117,115,132}},{{134,117,119,  0},{  0,  0,116,133}},{{135,118,120,  0},{  0,  1,  1,  1}},{{136,119,121,  0},{  0,  1,  1,  1}},{{137,120,  0,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{145,130,  0,  1},{  0,  1,  1,  1}},{{146,129,131,  0},{  0,  1,  1,  1}},{{147,130,132,  0},{  0,  1,  1,  1}},{{148,131,133,  0},{  0,132,147,115}},{{149,132,134,  0},{133,131,148,116}},{{150,133,135,  0},{  0,132,149,117}},{{151,134,136,  0},{  0,  1,  1,  1}},{{152,135,137,  0},{  0,  1,  1,  1}},{{153,136,  0,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},{{  0,  1,  1,  1},{  0,  1,  1,  1}},
{{146,  0,  1,  1},{  0,  1,  1,  1}},{{145,147,  0,  1},{  0,  1,  1,  1}},{{146,148,  0,  1},{  0,  1,  1,  1}},{{147,149,  0,  1},{  0,  0,148,131}},{{148,150,  0,  1},{  0,149,147,132}},{{149,151,  0,  1},{  0,  0,148,133}},{{150,152,  0,  1},{  0,  1,  1,  1}},{{151,153,  0,  1},{  0,  1,  1,  1}},{{152,  0,  1,  1},{  0,  1,  1,  1}}};

/*
    {{255,  0,  0,  0},{  1,255,  0,  0}},{{255,  0,  0,  0},{  0,  2,255,  0}},{{255,  0,  0,  0},{  1,  3,255,  0}},  {{255,255, 3,  18},{  2,  4,255,  0}},{{255,  4,  2, 19},{  3,  5,255,  0}},{{255,255,  3, 20},{  4,  6,255,  0}},   {{255,  0,  0,  0},{  5,  7,255,  0}},{{255,  0,  0,  0},{  6,  8,255,  0}},{{255,  0,  0,  0},{  7,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{255,  0,  0,  0},{  0, 17,255,  0}},{{255,  0,  0,  0},{  1, 16, 18,255}},{{255,  0,  0,  0},{  2, 17, 19,255}},  {{255, 19, 34,  2},{  3, 18, 20,255}},{{20,  18, 35,  3},{  4, 19, 21,255}},{{255, 19, 36,  4},{  5, 20, 22,255}},   {{255,  0,  0,  0},{  6, 21, 23,255}},{{255,  0,  0,  0},{  7, 22, 24,255}},{{255,  0,  0,  0},{  8, 23,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{255,  0,  0,  0},{ 16, 33,255,  0}},{{255,  0,  0,  0},{ 17, 32, 34,255}},{{255,  0,  0,  0},{ 18, 33, 35,255}},  {{255,255, 35, 18},{ 19, 34, 36,255}},{{255, 36, 34, 19},{ 20, 35, 37,255}},{{255,255, 35, 20},{ 21, 36, 38,255}},   {{255,  0,  0,  0},{ 22, 37, 39,255}},{{255,  0,  0,  0},{ 23, 38, 40,255}},{{255,  0,  0,  0},{ 24, 39,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 64,255,  0,  0},{ 32, 49,255,  0}},{{255,  0,  0,  0},{ 33, 48, 50,255}},{{ 66,255,  0,  0},{ 34, 49, 51,255}},  {{255,  0,  0,  0},{ 35, 50, 52,255}},{{ 68,255,  0,  0},{ 36, 51, 53,255}},{{255,  0,  0,  0},{ 37, 52, 54,255}},   {{ 70,255,  0,  0},{ 38, 53, 55,255}},{{255,  0,  0,  0},{ 39, 54, 56,255}},{{ 72,255,  0,  0},{ 40, 55,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 80,255,  0,  0},{ 48, 65,255,  0}},{{255,  0,  0,  0},{ 49, 64, 66,255}},{{ 82,255,  0,  0},{ 50, 65, 67,255}},  {{255,  0,  0,  0},{ 51, 66, 68,255}},{{ 84,255,  0,  0},{ 52, 67, 69,255}},{{255,  0,  0,  0},{ 53, 68, 70,255}},   {{ 86,255,  0,  0},{ 54, 69, 71,255}},{{255,  0,  0,  0},{ 55, 70, 72,255}},{{ 88,255,  0,  0},{ 56, 71,255,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{ 96, 81,255,  0},{ 64,255,  0,  0}},{{ 97, 80, 82,255},{255,  0,  0,  0}},{{ 98, 81, 83,255},{ 66,255,  0,  0}},  {{ 99, 82, 84,255},{255,  0,  0,  0}},{{100, 83, 85,255},{ 68,255,  0,  0}},{{101, 84, 86,255},{255,  0,  0,  0}},   {{102, 85, 87,255},{ 70,255,  0,  0}},{{103, 86, 88,255},{255,  0,  0,  0}},{{104, 87,255,  0},{ 72,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{112, 97,255,  0},{ 80,255,  0,  0}},{{113, 96, 98,255},{255,  0,  0,  0}},{{114, 97, 99,255},{ 82,255,  0,  0}},  {{115, 98,100,255},{255,  0,  0,  0}},{{116, 99,101,255},{ 84,255,  0,  0}},{{117,100,102,255},{255,  0,  0,  0}},   {{118,101,103,255},{ 86,255,  0,  0}},{{119,102,104,255},{255,  0,  0,  0}},{{120,103,255,  0},{ 88,255,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{128,113,255,  0},{255,  0,  0,  0}},{{129,112,114,255},{255,  0,  0,  0}},{{130,113,115,255},{255,  0,  0,  0}},  {{131,114,116,255},{255,255,115,130}},{{132,115,117,255},{255,116,114,131}},{{133,116,118,255},{255,255,115,132}},   {{134,117,119,255},{255,  0,  0,  0}},{{135,118,120,255},{255,  0,  0,  0}},{{136,119,255,  0},{255,  0,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{144,129,255,  0},{255,  0,  0,  0}},{{145,128,130,255},{255,  0,  0,  0}},{{146,129,131,255},{255,  0,  0,  0}},  {{147,130,132,255},{255,131,146,114}},{{148,131,133,255},{132,130,147,115}},{{149,132,134,255},{255,131,148,116}},   {{150,133,135,255},{255,  0,  0,  0}},{{151,134,136,255},{255,  0,  0,  0}},{{152,135,255,  0},{255,  0,  0,  0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},{{255,0,0,0},{255,0,0,0}},
    {{145,255,  0,  0},{255,  0,  0,  0}},{{144,146,255,  0},{255,  0,  0,  0}},{{145,147,255,  0},{255,  0,  0,  0}},  {{146,148,255,  0},{255,255,147,130}},{{147,149,255,  0},{255,148,146,131}},{{148,150,255,  0},{255,255,147,132}},   {{149,151,255,  0},{255,  0,  0,  0}},{{150,152,255,  0},{255,  0,  0,  0}},{{151,255,  0,  0},{255,  0,  0,  0}}
}; //,{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}},{{-1,0,0,0},{-1,0,0,0}}};
*/
/*
int g_KingMoves[BOARD_SIZE-7][8] =
   {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{ 19,  4,0,0,0,0,0,0},{ 20,  3,  5,0,0,0,0,0},{ 21,  4,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{  3, 35, 20,0,0,0,0,0},{  4, 36, 19, 21,0,0,0,0},{  5, 37, 20,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{ 19, 36,0,0,0,0,0,0},{ 20, 35, 37,0,0,0,0,0},{ 21, 36,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{131,116,0,0,0,0,0,0},{132,115,117,0,0,0,0,0},{133,116,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{115,147,132,0,0,0,0,0},{116,148,131,133,0,0,0,0},{117,149,132,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{131,148,0,0,0,0,0,0},{132,147,149,0,0,0,0,0},{133,148,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}
}; //,{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
*/

char horsdiff_[(BOARD_SIZE-7) * 2] =
//{        0,0,0,0,0, 0,0,
    {	0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,-17, 0,-15, 0,0,0, 0,0,0,0,0,0,0,
      0,0,-17,0, 0, 0,-15,0,0, 0,0,0,0,0,0,0,
      0,0,  0,0, 0, 0, 0,0,0, 0,0,0,0,0,0,0,
      0,0, 15,0, 0, 0,17,0,0, 0,0,0,0,0,0,0,
      0,0, 0,15, 0,17, 0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0, 0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0, 0,0,0,0, //0,0,0,
// 0,0,0,0
    };
char *horsdiff = horsdiff_ + BOARD_SIZE-7;


//const int HistorySize = BOARD_SIZE*32;

HistStruct m_his_table[8][10][BOARD_SIZE];  //20230309 per board.thd_id [8]
//HistStruct m_his_table[10][9][10]; //[piecetype][nFile][nRank];
//HistRecord m_hisrecord[512]; //[384] [256]; //[MAX_MOVE_NUM];
//int  m_hisindex=0;
UcciCommStruct UcciComm;
UcciCommEnum BusyComm;

inline char * MoveStr(char *str, int from, int dest)
{
    sprintf (str, "%c%d%c%d",
             //(from % 9) + 'a', 9 - (from / 9),  (dest % 9) + 'a', 9 - (dest / 9)
             (nFile(from)) + 'a', 9 - (nRank(from)),  (nFile(dest)) + 'a', 9 - (nRank(dest))
            );

    return str;
}
/* usage 
int bestmv;
printf("bestmv: %s", MOVE_COORD_C(bestmv);
*/

inline char * MOVE_COORD_C(int mv) {      // 把著法轉換成字符串
  char str[5];
  MoveStruct tempmove;
	tempmove.move = mv;
	return MoveStr(str, tempmove.from, tempmove.dest);
}

// 這四個數組用來判斷棋子的走子方向，以馬為例就是：sqDst = sqSrc + cKnightMoveTab[i]
//static const int cKingMoveTab[4]    = {-9, -1, 1, 9};      //{-0x10, -0x01, +0x01, +0x10};
//static const int cADVISORMoveTab[4] = {-10, -8, 8, 10};    //{-0x11, -0x0f, +0x0f, +0x11};
//static const int cElephanMoveTab[4] = {-20, -16, 16, 20};; //{-0x22, -0x1e, +0x1e, +0x22};
//{-19, -17, -11, -7, 7, 11, 17, 19};
//static const int knightchecknew[8] = {9,9,-1,1,-1,1,-9,-9};
//       6   7
//     4       5
//         +
//     2       3
//       0   1
//static const int cKnightMoveTab[8]  = {-19, -17, -11, -7, 7, 11, 17, 19}; //
//static const int cKnightMoveTab[8]  = {-0x21, -0x1f, -0x12, -0x0e, +0x0e, +0x12, +0x1f, +0x21};
  static const int cKnightMoveTab[8]  = {+0x1f, +0x21, +0x0e, +0x12, -0x12, -0x0e, -0x21, -0x1f};

//#define INF ((1<<13) - 1) //20230308 2^^13 -1 for max short 14bits in hVal of HashStruct in engine.h
#define VP 24 //18 //24 //21 //24
//#define VPC 30 //26 //30	//central pawn val
#define VPR 95 //95(87v) //95(73b) //VP*2 //95 //95 //83	//cross-river pawn val
//3dc092 13,25,26,144,152,340
//#define ENDPAWNBONUS 32 //32(1891u) //25(87v) //27(77b) //28 //32(73k) //32(73b) //3dc92=16
// 1892q - real optimize VPE,VPER,VBE,VEE
/* //20230210 chg to compile init
int VPE=56;  //24+32=56; 
int VPER=127; //95+32=127; 
int VBE=54;  // 54
int VEE=53;  // 53
//extern int VPE,VPER,VBE,VEE;
*/
#define VPE 56
#define VPER 127
#define VBE 54
#define VEE 53
//#define VPE  (VP+ENDPAWNBONUS) //56(73b) //50(72k) //56(87w) //54 //56 //54 //52(58z) //50(86f) //48 //46 //42 //48	//endgame pawn val
//#define VPER (VPR+ENDPAWNBONUS)//127(73b) //117(72k) //123(87w) //119 //123 //114 //123 //121 //119 //110 //104	//endgame river pawn val
#define VB  54 //35 //VP*2-1 //47 //53 //53(72w) //54 //40 //37 //54(87w) //54 //56
//#define VBE VB //54 //60(72k) //46 //43 //60(87w) //100= //110 //60 //60
#define VE  53 //(VB+1) //53 //39 //36 //53(87w) //54 //56
//#define VEE VE //53 //58(72k) //44 //41 //58(87w) //58 //98= //106 //58 //58
#define VN 190 //195 //190 //200 //190(87w //216 //190
#define VNE 194 //193 //195 //192 //212 //192(87w
#define VC 202 //205 //202 //212 //202(87w //228
#define VCE 199 //203 //199 //200 //200(87w
#define VNC ((VN+VC)/2) //196 //AVG(VN+VC)=(190+204)/2
#define VNCE ((VNE+VCE)/2)
#define VR 460 //20230209 460 //510 //460
#define VRE 460 //20230209 460 //470(72k) //460
//int PIECE_VALUE[46];
//20230209 [2][46]
int PIECE_VALUE_side[2][46] = {
    {0,0,-VP,VP,-VP,VP,-VP,VP,-VP,VP,-VP,VP,-VB,VB,-VB,VB,-VE,VE,-VE,VE,-VNC,VNC,-VNC,VNC,-VNC,VNC,-VNC,VNC,
    -VR,VR,-VR,VR,-9999,9999, 0,0,-VPR,VPR,-VPR,VPR,-VPR,VPR,-VPR,VPR,-VPR,VPR}, 
  	{0,0,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VBE,VBE,-VBE,VBE,-VEE,VEE,-VEE,VEE,-VNCE,VNCE,-VNCE,VNCE,
  	-VNCE,VNCE,-VNCE,VNCE,-VRE,VRE,-VRE,VRE,-9999,9999, 0,0,-VPER,VPER,-VPER,VPER,-VPER,VPER,-VPER,VPER,-VPER,VPER}};
//int PIECE_VALUE_side_ENDGAME[46]; // =
//#define XX 0 //1891v 36
//static int PIECE_VALUE_NOROOK[46] =
//    {0,0,-VPE-XX,VPE+XX,-VPE-XX,VPE+XX,-VPE-XX,VPE+XX,-VPE-XX,VPE+XX,-VPE-XX,VPE+XX,-VBE,VBE,-VBE,VBE,-VEE,VEE,-VEE,VEE,-VNCE,VNCE,-VNCE,VNCE,-VNCE,VNCE,-VNCE,VNCE,-VRE,VRE,-VRE,VRE,-9999,9999,
//    0,0,-VPER-XX,VPER+XX,-VPER-XX,VPER+XX,-VPER-XX,VPER+XX,-VPER-XX,VPER+XX,-VPER-XX,VPER+XX};    
//static int BasicValues[2][8] = {{VP,VP,VP,VB,VE,VN,VC,VR}, //9999}, // 56-> 55 //centrepawn,sidepawn,pawn,ADVISOR,elephant,knight,cannon,rook,king eyu
//    {VPE,VPE,VPE,VBE,VEE,VNE,VCE,VRE} //,9999}
//};   // 69-> 65 -> 64
//static int BasicValues[2][5] = {{VP,VE,VN,VC,VR}, //9999}, // 56-> 55 //centrepawn,sidepawn,pawn,ADVISOR,elephant,knight,cannon,rook,king eyu
//    {VPE,VEE,VNE,VCE,VRE} //,9999}    
//};   // 69-> 65 -> 64
int BasicValues[2][5] = {{VP,VE,VN,VC,VR}, {VPE,VEE,VNE,VCE,VRE}}; //20230210 chg to compile init

//static char PosValues[2][5][BOARD_SIZE] =
static char PosValues[2][5][90] =
{{{
        -6, -5, -5, -3,  3, -3, -5, -5, -6,
        34, 54, 94,123,127,123, 94, 54, 34,
        34, 54, 84,104,104,104, 84, 54, 34,
        34, 48, 54, 59, 62, 59, 54, 48, 34,
        11, 29, 30, 48, 50, 48, 30, 29, 11,
       -10,  0,  3,  0,  13,  0,  3,  0,-10,	//7->13
       -10,  0,-10,  0,  12,  0,-10,  0,-10,  //6->12
         0,  0,  0,  -99,  -99,  -99,  0,  0,  0,
         0,  0,  0,  -96,  -96,  -96,  0,  0,  0,
         0,  0,  0,  -79,  -70, -79,  0,  0,  0}, //   0,0,0,0,0,0,0,      // PAWN KING  -20

         //0,  0,  0,  1,  1,  1,  0,  0,  0,
         //0,  0,  0,  4,  4,  4,  0,  0,  0,
         //0,  0,  0, 21, 30, 21,  0,  0,  0},

        {0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, -6,  0,  0,  0, -6,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
        -4,  0,  0, -1,  8, -1,  0,  0, -4,
         0,  0,  0,  0,  5,  0,  0,  0,  0,
         0,  0,  1,  1,  0,  1,  1,  0,  0},        // ADVISOR ELEPHAN black

        {-10, -9, -7,  3,-10,  3, -7, -9,-10,
         -10,  4, 18,  4, -1,  4, 18,  4,-10,
         -4,   6,  9, 18,  9, 18,  9,  6, -4,
         -4,  27, 13, 25, 14, 25, 13, 27, -4,
         -10, 11,  9, 18, 19, 18,  9, 11,-10,
         -10,  8, 13, 15, 16, 15, 13,  8,-10,
         -6,  -1,  5,  0,  6,  0,  5, -1, -6,
         -4,  -6, -1,  0, -6,  0, -1, -6, -4,
         -19,-10, -6, -4,-54, -4, -6,-10,-19,   //-50->-54
         -13,-20,-10,-13,-10,-13,-10,-20,-13}, 					// knight black
/*
			{10, 10, 2,-7,-10, -7, 2, 10, 10,
				6, 6, 2,-6,-13,-6, 2, 6, 6,
				3, 3, 2,-7, -7,-7, 2, 3, 3,
				2, 9, 9, 6, 10, 6, 9, 9, 2,
				1, 3, 1, 2,  9, 2, 1, 3, 1,
				-1,-1, 6, 2, 9, 2, 6, -1, -1,	    //was 2 at side
				2, 2, 2, 2, 3, 2, 2, 2, 2,
				4, 3,10, 9,13, 9,10, 3, 4,
			  1, 4, 6, 6, 6, 6, 6, 4, 1,
        1, 1, 4, 9, 9, 9, 4, 1, 1},       //CANNNON black  ey 154 ok
*/
{7,  7,  1,  -5,  -7,  -5,   1,   7,  7,
 4,  4,  1, -13,  -9, -13,   1,   4,  4, //-8->-9
 2,  2,  1, -16,  -4, -16,   1,   2,  2,
 1,  6,  6,   4,   7,   4,   6,   6,  1,
 1,  2,  1,   1,   6,   1,   1,   2,  1,
 -4, -2,  4,   1,   6,   1,   4,   -2,  -4, //-1->-2->-3 1->0  -1=>-2 2892w
 1,  -1,  1,   1,   1,   1,   1,   -1,  1,  //1->0
 3,  1,  7,   5,   9,   5,   7,   1,  3,
 1,  3,  4,   4,   4,   4,   4,   3,  1,
 1,  1,  3,   5,   5,   5,   3,   1,  1},

		   {16,18,14,23,24,23,14,18,16, //15->14
				16,22,19,26,53,26,19,22,16,
				12,18,17,24,26,24,17,18,12,	//16->14->13->12
				14,23,23,26,26,26,23,23,14, //14
				17,21,21,24,25,24,21,21,17,
				15,22,22,24,25,24,22,22,15,  //18->15
				14,19,14,22,24,22,14,19,14,
				-3, 18,14,22,22,22,14,18,-3,   // 6-> -3
				10,18,13,23, 3,23,13,18,10,		//12->11->10 //22->23 //14->13
       -24, 18,14,22, 5,22,14,18,-24}},	// ROOK black ey 320  = (200+120) late corner rook (from -8 to -10)

// for endgame
    {
    		{	-18,-18,-18,-15,-15,-15,-18,-18,-18,
    			35, 44, 80, 103, 114,103, 80, 44, 35, //103-> 100
    			35, 43, 68, 86, 87, 86, 68, 43, 35,
    			33, 38, 44, 59, 62, 59, 44, 38, 33, //62->80
    			13, 33, 35, 38, 40, 38, 35, 33, 13,	  //11->9   30->35
//    		-4, 0,  3,   0, 10,  0, 3,  0, -4,			//7->9(86f)->10    -7->-6->-5->-4
//           6,  0, 13,  0, 20,  0, 13,  0,  6,
           11,  0, 15,  0, 22,  0, 15,  0,  11,

//    			-6, 0,  -6,  0, 5,  0, -6,  0, -6,		//-8->-6
//    			4,  0,   4,  0, 15,  0,  4,  0,  4,		//-8->-6
    			8,  0,   8,  0, 17,  0,  8,  0,  8,		//-8->-6
    			0,  0,   0, -99,-96,-99,  0,  0,  0,	//-97 -> -99
    			0,  0,   0, -96,-94,-96,  0,  0,  0,
          0,  0,   0, -78,-70,-78,  0,  0,  0},          // PAWN KING  -20

          //0,  0,   0, 0,   2,  0,  0,  0,  0,
    			//0,  0,   0, 4,   6,  4,  0,  0,  0,
          //0,  0,   0, 22, 30, 22,  0,  0,  0},

				{0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
         0,  0, -6,  0,  0,  0, -6,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,  0,
        -4,  0,  0, -1,  8, -1,  0,  0, -4,
         0,  0,  0,  0,  4,  0,  0,  0,  0,  // 5->4
         0,  0,  1,  1,  0,  1,  1,  0,  0},        // ADVISOR ELEPHAN black
/*
       {-5, -1, 3,  2,  2,  2, 3, -1, -5,
         0,  2, 7,  5,  4,  5, 7,  2,  0,
         1,  5, 8,  8,  8,  8, 8,  5,  1,
         1,  6, 8,  8,  8,  8, 8,  6,  1,
         1,  5, 8,  8,  8,  8, 8,  5,  1,
         0,  4, 6,  6,  7,  6, 6,  4,  0,
         0,  2, 3,  5,  6,  5, 3,  2,  0,
        -5, -1, 1,  1,  1,  1, 1, -1, -5,
        -8, -5,-2, -5, -50, -5,-2, -5, -8,
        -11,-8,-5, -8, -9, -8,-5, -8, -11}, // HORSE black ey

        {4, 8,12,12,12,12,12, 8, 4,
         8,12,16,16,16,16,16,12, 8,
        12,16,20,20,20,20,20,16,12,
        12,16,20,20,20,20,20,16,12,
        12,16,20,20,20,20,20,16,12,

         8,12,16,16,16,16,16,12, 8,
         8,12,16,16,16,16,16,12, 8,
         4, 8,12,12,12,12,12, 8, 4,
         0, 4, 8, 4, 4, 4, 8, 4, 0,
        -4, 0, 4, 0, 0, 0, 4, 0,-4},   //knight
*/
          {5, 10,15,15,15,15,15, 10, 5,
         10,15,25,20,20,20,25,15, 10,
        15,20,25,25,25,25,25,20,15,
        14,25,24,25,25,25,24,25,14,
        14,20,24,25,25,25,24,20,14,

         10,15,20,20,20,20,20,15, 10,
         10,15,20,20,20,20,20,15, 10,
         5, 10,15,15,15,15,15,10, 5,    //15->12
         0, 5, 10, 5,-5, 5,10, 5, 0,
        -5, 0, 5, 0, 0, 0, 5, 0,-5},   //knight
      //    -4,  -6, -1,  0, -1,  0, -1, -6, -4,
      //   -39,-10, -6, -4,-54, -4, -6,-10,-39,  //-19->-29 -54->-64
      //   -44,-40,-20,-23,-10,-23,-20,-40,-44}, //-20->-30 -13->-23 -10->-20

      //  -5, -1, 1,  1,  1,  1, 1, -1, -5,
      //   -8, -5,-2, -5, -50, -5,-2, -5, -8,
      //  -11,-8,-5, -8, -9, -8,-5, -8, -11}, // HORSE black ey
//         4, 8,12,12,12,12,12, 8, 4,
//         0, 4, 8, 4, 4, 4, 8, 4, 0,
//        -4, 0, 4, 0, 0, 0, 4, 0,-4},

        {7, 8, 8, 8,  8,  8, 8, 8, 7,
         8, 8, 8, 8,  8,  8, 8, 8, 8,
         8, 8, 8, 8,  8,  8, 8, 8, 8,
         8, 10, 10, 13, 19,13, 10, 10, 8,
         8, 10, 10, 13, 19,13, 10, 10, 8,
         8, 10, 10, 14, 19,14, 10, 10, 8,	  // was 2 at side
         8, 10, 10, 14, 19,14, 10, 10, 8,
         8, 10, 10, 17, 22,17, 10, 10, 8,   //15-19-15 ->  17-22-17
        10, 10, 10, 19, 21,19, 10, 10, 10,
        10, 10, 10, 19, 22,19, 10, 10, 10},   // CANNON black  ey 154 ok
/*
         {16,18,15,23,24,23,15,18,16,
				16,22,19,26,53,26,19,22,16,
				14,18,17,24,26,24,17,18,14,	//16->14
				14,23,23,26,26,26,23,23,14,
				17,21,21,24,25,24,21,21,17,
				15,22,22,24,25,24,22,22,15,  //18->15
				14,19,14,22,24,22,14,19,14,
				-3, 18,14,22,22,22,14,18,-3,   // 6-> -3
				11,18,14,23, 3,23,14,18,11,		//12->11 //22->23
       -24, 18,14,22, 5,22,14,18,-24}},
*/
       {23,23,23,28,31,28,23,23,23,
        26,26,26,29,60,29,26,26,26,      //30->46->78->89->85->80
        23,23,23,26,29,26,23,23,23,
        20,20,20,24,28,24,20,20,20,
        20,20,20,24,26,24,20,20,20,
        20,20,20,24,26,24,20,20,20,
        18,19,19,24,26,24,19,19,18,
        18,19,19,24,26,24,19,19,18,
        18,19,19,24,26,24,19,19,18,
        18,19,19,24,26,24,19,19,18}}     // ROOK black ey 320 = (200 + 120)
/* (72k)
       {13,13,13,18,29,18,13,13,13,
        16,16,16,19,50,19,16,16,16,      //30->46->78->89->85->80
        13,13,13,16,19,16,13,13,13,
        10,10,10,14,18,14,10,10,10,
        10,10,10,14,16,14,10,10,10,
        10,10,10,14,16,14,10,10,10,
         8, 9, 9,14,16,14, 9, 9, 8,
         8, 9, 9,14,16,14, 9, 9, 8,
         8, 9, 9,14,16,14, 9, 9, 8,
         8, 9, 9,14,16,14, 9, 9, 8}}     // ROOK black ey 320 = (200 + 120)
*/
};  //0,0,0,0,0,0,0}}};

//unsigned char ATTKPVAL[34]   = {0,0,2,2,0,0,0,0,1,1,1,1,4,4,4,4,2,2,2,2, 7,7,7,7,7,7,7,7,9,9,9,9,1,1};
//20230209
//int ATTKPVAL[34]   = {0,0,2,2,0,0,0,0,1,1,1,1,4,4,4,4,3,3,3,3, 7,7,7,7,7,7,7,7,9,9,9,9,8,8};
//int ATTKPVAL_3[34] = {0,0,5,5,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,10,10,10,10,10,10,10,10,12,12,12,12,11,11};

int ATTKPVAL[2][34]  = {{0,0,2,2,0,0,0,0,1,1,1,1,4,4,4,4,3,3,3,3, 7,7,7,7,7,7,7,7,9,9,9,9,8,8},
  											{0,0,5,5,5,5,5,5,5,5,5,5,4,4,4,4,2,2,2,2, 7,7,7,7,7,7,7,7,9,9,9,9,8,8}};
int ATTKPVAL_3[2][34] ={{0,0,5,5,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,10,10,10,10,10,10,10,10,12,12,12,12,11,11},
  											{0,0,9,9,9,9,9,9,9,9,9,9,5,5,5,5,6,6,6,6,10,10,10,10,10,10,10,10,12,12,12,12,11,11}};

//char pointtableABS[BOARD_SIZE-7][16]; //[10]; //[32];  // for genNoncap() use
char abs_pointtable[2][10][10][9]; //20230209 [p_endgame][piecetype]{nRank][nFile]
//short pointtable[BOARD_SIZE-7][16][2]; //[10]; //[32]; //[34][90] [2]=board.p_endgame=0,1
short pointtable[2][10][10][9]; //20230209 [p_endgame][piecetype][nRank]{nFile]
//20230209 short eg_pointtable[10][10][9]; //end-game
//short abspointtable[10][10][9]; //[piecetype][nRank]{nFile]
//short eg_abspointtable[10][10][9]; //end-game
//short pointtableAdvEleB[(BOARD_SIZE/2)-7][2]; //preEval
//short pointtableAdvEleR[(BOARD_SIZE/2)-7][2]; //preEval
//short pointtablePawnB[(7*16)-7][2]; //preEval
//short pointtablePawnR[(7*16)-7][2]; //preEval
#ifdef PREEVAL
short pointtableAdvEleB[5][9]; //preEval
short pointtableAdvEleR[5][9]; //preEval
short pointtablePawnB[7][9]; //preEval
short pointtablePawnR[7][9]; //preEval
#endif
//short pointtableHorsB[5][9]; //preEval
//short pointtableHorsR[5][9]; //preEval
//short eg_pointtableAdvEleB[9][5]; //preEval
//short eg_pointtableAdvEleR[9][5]; //preEval
//short eg_pointtablePawnB[9][7]; //preEval
//short eg_pointtablePawnR[9][7]; //preEval
//int SCORE80_20;
//int SCORE6_2;
//20230209 chg to [2][9]
//int EMPTY_CANN_SCORE[9] = {0,0,34,40,48,60,90,90,90};
//int BOTTOM_CANN_SCORE[9] = {17,13,2,0,0,0,2,13,17};
int EMPTY_CANN_SCORE[2][9] ={{0,0,30,40,48,60,90,90,90},{0,0,15,20,24,30,45,45,45}};
int BOTTOM_CANN_SCORE[2][9] ={{17,13,2,0,0,0,2,13,17},
														   {8,6, 1,0,0,0,1,6,8}};
//unsigned char EMPTY_CANN_SCORE[3][16] =
//{{0,0,8,12,14,17,22,22,22, 0,0,0,0,0,0,0},
// {0,0,27,33,39,48,66,66,66, 0,0,0,0,0,0,0},
// {0,0,38,44,52,64,90,90,90, 0,0,0,0,0,0,0}};
/* pre-84p
{{0,0,8,10,12,15,22,22,22, 0,0,0,0,0,0,0},
 {0,0,24,30,36,45,66,66,66, 0,0,0,0,0,0,0},
 {0,0,34,40,48,60,90,90,90, 0,0,0,0,0,0,0}};
*/
//int BOTTOM_CANN_SCORE[9] = {0,0,13,18,18,18,18,18,18};
//int BOTTOM_CANN_SCORE[2][9] = {{0,0,13,20,20,20,20,20,20},{0,0,13,20,20,20,20,20,20}};
//int BOTTOM_CANN_SCORE[2][9] = {{0,6,60,80,80,80,80,80,80},{0,6,60,80,80,80,80,80,80}};
//static const unsigned char BOTTOM_CANN_SCORE_ORI[9][2] = {{80,80},{60,60},{6,6},{0,0},{0,0},{0,0},{6,6},{60,60},{80,80}};
//static const unsigned char BOTTOM_CANN_SCORE_ORI[9][2] = {{40,40},{30,30},{3,3},{0,0},{0,0},{0,0},{3,3},{30,30},{40,40}};
//int BOTTOM_CANN_SCORE[9][2];
//int HORSE_MOBSCORE[9][2] = {{-14,14},{-10,10},{0,0},{6,-6}, {7,-7},{9,-9},{10,-10},{12,-12},{13,-13}};
//static const int HORSE_MOBSCORE_ENDGAME[9][2] = {{-20,20},{-15,15},{0,0},{8,-8}, {13,-13},{18,-18},{23,-23},{28,-28},{33,-33}};
//20230209 int HORSE_MOBSCORE[9] = {-30,-13,-9,-6,0,3,6,8,8};
//static const int HORSE_MOBSCORE_ENDGAME[9] = {-60,-26,-15,-6,0,3,6,8,8};
int HORSE_MOBSCORE[2][9] = {{-30,-13,-9,-6,0,3,6,8,8},
													{-60,-26,-15,-6,0,3,6,8,8}};
//int QCHECKDEPTH = 0;
#ifdef HORSECK
unsigned char kingidxpos[18]={3,4,5, 19,20,21, 35,36,37, 115,116,117, 131,132,133, 147,148,149};
#endif
char kingindex[BOARD_SIZE-7] =
    {-1,-1,-1,0,1,2,-1,-1,-1,      0,0,0,0,0,0,0,
     -1,-1,-1,3,4,5,-1,-1,-1,      0,0,0,0,0,0,0,
     -1,-1,-1,6,7,8,-1,-1,-1,      0,0,0,0,0,0,0,
     -1,-1,-1,-1,-1,-1,-1,-1,-1,   0,0,0,0,0,0,0,
     -1,-1,-1,-1,-1,-1,-1,-1,-1,      0,0,0,0,0,0,0,
     -1,-1,-1,-1,-1,-1,-1,-1,-1,      0,0,0,0,0,0,0,
     -1,-1,-1,-1,-1,-1,-1,-1,-1,     0,0,0,0,0,0,0,
     -1,-1,-1,9,10,11,-1,-1,-1,   0,0,0,0,0,0,0,
     -1,-1,-1,12,13,14,-1,-1,-1,   0,0,0,0,0,0,0,
     -1,-1,-1,15,16,17,-1,-1,-1
    }; //,   0,0,0,0,0,0,0};
unsigned char kingattk_incl_horse[18][BOARD_SIZE-7] =
{{
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0
 },
{1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0
 },
{1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0
 },

{0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0
 },
{0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0
 },
{0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0
 },

{0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0
 },
{0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0
 },
{0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0
 },

{0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0
 },
{0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0
 },
{0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0
 },

{0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0
 },
{0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0
 },
{0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0
 },

{0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,1,1,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1
 },
{0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,1,1,1,1,1,0,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1
 },
{0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,
 0,0,0,0,1,1,1,0,0, 0,0,0,0,0,0,0,
 0,0,0,1,1,1,1,1,0, 0,0,0,0,0,0,0,
 1,1,1,1,1,1,1,1,1
 }};
//#define FUTPAWN
//#define ENDFUTPAWN 28 //30 //31(1880u) //26(1880s) //33 //36
// razoring
//int TordRazorDepth =  3;
//int TordRazorMargin = 180; //150; //150; //125; //135; //300
// pawn val 24, futmargin1=1.50 * 24 = 36, margin2=3.00 * 24 = 72
//int FUT_MARGIN[8] = {0,36,72,195,240,285,325,370};
//int FUT_MARGIN[8] = {0,45,125,195,240,285,325,370};
//  int FUT_MARGIN[8] = {0,FUTPAWN, FUTPAWN*2, FUTPAWN*(32+5)/16, FUTPAWN*(32+10)/16, FUTPAWN*(48+4)/16, FUTPAWN*(48+10)/16, FUTPAWN*8}; //195}; //36 72;
// 1877f
//int FUT_MARGIN[6] = {0,FUTPAWN*3/2, FUTPAWN*3, FUTPAWN*6, FUTPAWN*15/2, FUTPAWN*15/2}; //36 72 144;
// 1880o 1880t
//int FUT_MARGIN[6] = {0,FUTPAWN, FUTPAWN*3, FUTPAWN*5, FUTPAWN*15/2, FUTPAWN*15/2}; //36 72 144;
// 1880u
//int FUT_MARGIN[4] = {0,FUTPAWN, FUTPAWN*3, FUTPAWN*5};
//if (depth >= 2) futility_margin = 200 + (depth % 2) * 100;  //toga 1.4.2SE
// 1878n
//int FUT_MARGIN[6] = {0,FUTPAWN, FUTPAWN*3, FUTPAWN*5, FUTPAWN*15/2, FUTPAWN*15/2};   //1880c
//int FUT_MARGIN[6] = {0,FUTPAWN, FUTPAWN*3, FUTPAWN*3, FUTPAWN*4, FUTPAWN*4}; //1880g
int fut_depth=6; //3; //3; //3; //5; //5=stockfish //2; //endgame=1 //2; //3; //toga1.4.1se=5 //3; //2; //endgame=0
//int DELTA_MARGIN=FUTPAWN; //*5/4; //VE; //FUTPAWN; // / 2; //12; //25; //30; //25; //30; //30; //25; //30; //25; //30; //25;
//int SINGULAR_MARGIN=FUTPAWN/2; //*3/4;   //*2; // /8; //32/4; end=28/4 //64-1889n  // for Singular ext
int FUTPAWN=23; //=32;
int ENDFUTPAWN=26; //=28; //30 //31(1880u) //26(1880s) //33 //36
int DELTA_MARGIN=34; //=32;
int SINGULAR_MARGIN=14; //=16;
int DELTA_MARGIN_EG=22; //=28;
//extern int SINGULAR_MARGIN_EG; //=14; 
//int delta_piecedest = B_ROOK1;
//int nullreduction=3;  //3; //2; //3;
#define nullreduction 3
//int nullreductver=5; //5; //5; //6; //gluarung=6 toga=5; //9; //8; //endgame=6; //4 //6; //5; //6;
#define nullreductver 5
#ifdef HISTPRUN

//int HistCutShift=3; //4;
int ExtHistPrunShift=2; //3(60p);
int ExtHistDepth=6; //7(60p); //6; //endgame=9
//int ExtHistCount=9;
int histcut_depth=7; //7=stockfish 1.3 //6=toga1.4.1se //4; //endgame=0 (mean no cut at endgame)
//int histcut_count=5; //9;
#define HistoryDepth  3 //2=stockfish //3=toga //2 //3 //3
int HistoryMoveNb=7; //6; //endgame=9 //=6; //5 //6 // 6 //7 //6 //5  //3   // >=5 crafty does not reduce first 4 history moves
//static /* const */ int HistoryValue = 9830; // 60%
//static /* const */ int HistoryBound = 2458; // * 16384 + 50) / 100 10%=1638 15%=2458 20%=3277
//#define HistPrunShift 1  // 歷史表裁剪的比例閾值，1=50%，2=25%，3=12.5%  4=6.25%，等等
//#define ExtHistPrunShift 2
#endif
extern int ext_IMaxTime;
int PollNodes = 32767;  // number of nodes between poll for checkstop/timeout. Depends on time remain
//int DROPDOWN_VAL=45;



void Engine::Setpointtable(int endgame) // 0=start-midgame, 1=endgame
{	//printf("info Setpointtable start\n");
    int m,n,posval;

//    //king/pawn, bis/ele, hors, cann, rook
//		for (i=0; i<16; i+=2)
    for (int p=0; p<10; p+=2)
    {
        for (n=0; n<9;  n++)
        for (m=0; m<10; m++)
            {
                //posval = pointtableABS[p][n][m];
                posval = PosValues[endgame][p/2][((9-m)*9)+n];
                
                //abspointtable[p][m][n]= posval + 100;
                abs_pointtable[endgame][p][m][n]= posval;       //20230209
                abs_pointtable[endgame][p+1][9-m][n] = posval;  //20230209
                if (p==0 && posval <= -70) //king posval      //20230209
                	posval += 9999;	//inc king val
                //abspointtable[p+1][9-m][n] =
                //abspointtable[p][m][n]=
                //pointtable[p][m][n]= posval + BasicValues[endgame][p/2];
                //pointtable[endgame][p][m][n]= posval + BasicValues[endgame*5 +(p/2)];  //20230209
                pointtable[endgame][p][m][n]= posval + BasicValues[endgame][(p/2)];  //20230210
                pointtable[endgame][p+1][9-m][n] = -pointtable[endgame][p][m][n];	// [89-j]    //20230209
                //abspointtable[p+1][9-m][n] = abspointtable[p][m][n];	// [89-j]
            }
    }

#ifdef PREEVAL
//save for preEval
/*
for (i=0; i<9; i++)
{
for (j=0; j<5; j++)
{
	pointtableAdvEleB[i][j] = pointtable[2][i][j];  //[2]
	pointtableAdvEleR[i][j] = pointtable[3][i][9-j]; //[3]
}

for (j=0; j<7; j++)
{
	pointtablePawnB[i][j] = pointtable[0][i][j+3];
	pointtablePawnR[i][j] = pointtable[1][i][9-(j+3)];
}
}
*/
memcpy(pointtableAdvEleB, &(pointtable[2][0][0]), sizeof(pointtableAdvEleB));
memcpy(pointtableAdvEleR, &(pointtable[3][5][0]), sizeof(pointtableAdvEleR));
//memcpy(pointtableHorsB, &(pointtable[4][0][0]), sizeof(pointtableHorsB));
//memcpy(pointtableHorsR, &(pointtable[5][5][0]), sizeof(pointtableHorsR));
memcpy(pointtablePawnB, &(pointtable[0][3][0]), sizeof(pointtablePawnB));
memcpy(pointtablePawnR, &(pointtable[1][0][0]), sizeof(pointtablePawnR));
#endif

//if (endgame) 
//{
	  //20230209 memcpy(eg_pointtable, pointtable, sizeof(pointtable));
	  
	//memcpy(eg_abspointtable, abspointtable, sizeof(abspointtable));
//	memcpy(eg_pointtableAdvEleB, pointtableAdvEleB, sizeof(pointtableAdvEleB));
//	memcpy(eg_pointtableAdvEleR, pointtableAdvEleR, sizeof(pointtableAdvEleR));
//	memcpy(eg_pointtablePawnB, pointtablePawnB, sizeof(pointtablePawnB));
//	memcpy(eg_pointtablePawnR, pointtablePawnR, sizeof(pointtablePawnR));
//}
//debug
/*
    printf("info abspointtable: endgame=%d\n", endgame);
    //for (i=0; i<16; i++)
    for (int i=0; i<10; i++)
    {	printf("i=%d\n", i);
    	for (m=0; m<10; m++)
      {      for (n=0; n<9;  n++)
            {
                //j=(m*16) + n;
    						//printf("%5d", pointtableABS[j][i]);
    						printf("%5d", abspointtable[i][m][n]);
    				}
    				printf("\n");
  		}
    }

    printf("info eg_abspointtable: endgame=%d\n", endgame);
    //for (i=0; i<16; i++)
    for (int i=0; i<10; i++)
    {	printf("i=%d\n", i);
    	for (m=0; m<10; m++)
      {      for (n=0; n<9;  n++)
            {
                //j=(m*16) + n;
    						//printf("%5d", pointtable[j][i][endgame]);
    						printf("%5d", eg_abspointtable[i][m][n]);
    				}
    				printf("\n");
  		}
    }

    printf("info pointtable: endgame=%d\n", endgame);
    //for (i=0; i<16; i++)
    for (int i=0; i<10; i++)
    {	printf("i=%d\n", i);
    	for (m=0; m<10; m++)
      {      for (n=0; n<9;  n++)
            {
                //j=(m*16) + n;
    						//printf("%5d", pointtable[j][i][endgame]);
    						printf("%5d", pointtable[i][m][n]);
    				}
    				printf("\n");
  		}
    }

    printf("info eg_pointtable: endgame=%d\n", endgame);
    //for (i=0; i<16; i++)
    for (int i=0; i<10; i++)
    {	printf("i=%d\n", i);
    	for (m=0; m<10; m++)
      {      for (n=0; n<9;  n++)
            {
                //j=(m*16) + n;
    						//printf("%5d", pointtable[j][i][endgame]);
    						printf("%5d", eg_pointtable[i][m][n]);
    				}
    				printf("\n");
  		}
    }
*/
//endebug

}

void Engine::AdjustEndgame(Board &board)
{
//adjust pointtable depending on endgame reached
    if (board.p_endgame==0)
    {
				// endgame if attacking pieces (R,C,H) <=2 or no nooks
        if (PCbitCountMSB(board.bitpiece & 0x55500000) <=2 || PCbitCountMSB(board.bitpiece & 0xaaa00000) <=2   
        || (board.bitpiece & 0xf0000000) ==0 ) // no rooks    //20230216         
        {
  						//20230209 if ((board.bitpiece & 0xf0000000) ==0) // no rooks
  						//{
  							board.p_endgame=1;
	  						FUTPAWN = ENDFUTPAWN;
	  						//DELTA_MARGIN = ENDFUTPAWN;
	  						//SINGULAR_MARGIN = ENDFUTPAWN/2; //*3/4; //*2; ///8;
    						DELTA_MARGIN = DELTA_MARGIN_EG;
							//}
  							//DRAWVALUE = 1; //endgame value
                TEMPO_BONUS = 0; //endgame value
   							//20230209 memcpy(PIECE_VALUE_side, PIECE_VALUE_side_ENDGAME, sizeof(PIECE_VALUE_side)); 
                //20230209 memcpy(pointtable, eg_pointtable, sizeof(pointtable));

                //20230209 memcpy(HORSE_MOBSCORE, HORSE_MOBSCORE_ENDGAME, sizeof(HORSE_MOBSCORE));
								//20230209 chg to [2][9]
                //for (int i=0; i<9; i++)
                //{
                //	EMPTY_CANN_SCORE[i] /= 2; //4;
                //	BOTTOM_CANN_SCORE[i] /= 2; //4;
                //}

								//20230209 chg to [2][34] increase pawn ATTKPVAL at endgame
								//for (int i=2; i<12; i++)
								//{
								//	ATTKPVAL[i]=5; //5;   //6; 87e
								//	ATTKPVAL_3[i]=9; //9; 87eed
								//}		
#ifdef PREEVAL
                memcpy(pointtableAdvEleB, &(pointtable[2][0][0]), sizeof(pointtableAdvEleB));
								memcpy(pointtableAdvEleR, &(pointtable[3][5][0]), sizeof(pointtableAdvEleR));
								//memcpy(pointtableHorsB, &(pointtable[4][0][0]), sizeof(pointtableHorsB));
								//memcpy(pointtableHorsR, &(pointtable[5][5][0]), sizeof(pointtableHorsR));
								memcpy(pointtablePawnB, &(pointtable[0][3][0]), sizeof(pointtablePawnB));
								memcpy(pointtablePawnR, &(pointtable[1][0][0]), sizeof(pointtablePawnR));
#endif
//20230209 #ifdef DEBUG
//                printf("endgame reached\n");
//#endif
        }
    }
}

void Clear_Killer()
{
    memset(g_killer, 0, sizeof(g_killer));
    memset(g_matekiller, 0, sizeof(g_matekiller)); // [0]) * MAXPLY);   
} 
  
void Clear_Hist(void)
{
		//for (int i=0; i<(BOARD_SIZE-7); i++)
		//for (int j=0; j<16; j++)
		for (int t=0; t<=NCORE1; ++t) //20230309 per board.thd_id
		for (int p=0; p<10; p++)
		for (int i=0; i<10; i++)
		for (int j=0; j<9; j++)
		{
#ifdef HISTHIT
        //m_his_table[p][i][j].HistHit = 1; //1;
        //m_his_table[p][i][j].HistTot = 1;
        m_his_table[t][p][(i*16)+j].HistHit = 1; //1;
        m_his_table[t][p][(i*16)+j].HistTot = 1;
#endif
        //m_his_table[p][i][j].HistVal = 0;
        m_his_table[t][p][(i*16)+j].HistVal = 0;
    }
}
#ifdef PVRED
// Reduction lookup tables (initialized at startup) and their getter functions
  unsigned char    PVRedMatrix[64][64]; // [depth][moveNumber]
  unsigned char NonPVRedMatrix[64][64]; // [depth][moveNumber]

  inline int    pvred(int d, int mn) { return    PVRedMatrix[std::min(d / 2, 63)][std::min(mn, 63)]; }
  inline int nonpvred(int d, int mn) { return NonPVRedMatrix[std::min(d / 2, 63)][std::min(mn, 63)]; }
#endif

void Engine::PreMoveGen()
{	//printf("info PreMoveGen start\n");

    int i, j,SrcSq, DstSq, Index;

    Clear_Killer();
    Clear_Hist();

//    for (i=0; i<46; i++)
//		{
//			PIECE_VALUE[i] = abs(PIECE_VALUE_side[i]);
//		}

    // adjust sq after change BOARD_SIZE from 90 to 160
    //for (SrcSq = 0; SrcSq < BOARD_SIZE-7; SrcSq ++)
    //{
        //for (i=0; i<8; i++)
        //{
            /*
            DstSq = knightmoves[SrcSq][i];
            if ((DstSq >8) && (DstSq <255))
            {	knightmoves[SrcSq][i] = ADJ9_16(DstSq);
            }


            */
       // }

       /*
        //for (i=0; i<5; i++)
        for (i=0; i<4; i++)
        {
            for (j=0; j<16; j++)
            {
                DstSq = g_RookMoves[SrcSq][i][j];
                if ( (DstSq <255))
                    g_RookMoves[SrcSq][i][j] = ADJ9_16(DstSq) + 1;
                else g_RookMoves[SrcSq][i][j] = 0; // -1

            }
        }
*/

    //} // end for SrcSq



    for (SrcSq = 0; SrcSq < BOARD_SIZE-7; SrcSq ++)
    {
        /*
        // 生成象眼數組
        Index = 0;
        for (i = 0; i<5; i++)
        {
        DstSq = g_advelemoves[SrcSq][i];
        if (DstSq == 0) break;
        //if (DstSq > 0)
        {
         g_ElephantEyes[SrcSq][Index] = (SrcSq + DstSq) >>1;
         Index ++;
        }
        }
        */

        // 生成馬的著法預生成數組，包括馬腿數組
      if (nFile(SrcSq) < 9)
      {
#ifdef HORSECK
        unsigned char knightchkidx[18];
        for (int k=0; k<18; k++)
        {
        	knightchkidx[k]=1;
        }
#endif
        Index = 0;
        for (i=0; i<8; i++)
        {
            //DstSq = knightmoves[SrcSq][i];
            //if (DstSq !=255) //>= 0)
            DstSq = SrcSq + cKnightMoveTab[i];
            //if (c_InBoard[DstSq])
            if (DstSq >=0 && DstSq <160 && nFile(DstSq) < 9)
            {
                g_KnightMoves[SrcSq][Index] = DstSq +1;
                g_HorseLegs[SrcSq][Index] = DstSq + horsdiff[SrcSq - DstSq];
                Index ++;
#ifdef HORSECK
                // generate g_KnightChecks
                for (int j=0; j<18; j++)
                {
                	if (DstSq != kingidxpos[j] && SrcSq != kingidxpos[j])
                	if (horsdiff[DstSq-kingidxpos[j]] !=0) //DstSq is knight check
                	{
                		g_KnightChecks[SrcSq][j][knightchkidx[j]] = DstSq;
                    knightchkidx[j]--;
                  }
                }
#endif
            }
        }
        g_KnightMoves[SrcSq][Index] = 0; //-1; //255; //-1; //0;

/*
				// 生成馬腿數組
        //Index = 0;
        for (i=0; i<8; i++)
        {
            DstSq = g_KnightMoves[SrcSq][i];
            if (DstSq !=0)
            {
                DstSq--;
                g_HorseLegs[SrcSq][i] = (DstSq) + horsdiff[SrcSq - DstSq];
                //Index ++;
            }
						else break;
        }
*/
        // 生成兵(卒)的著法預生成數組
/*
        for (i = 0; i < 2; i ++)
        {
            for (j = 0; j < 4; j++)
            {
              DstSq = g_PawnMoves[SrcSq][i][j];
              if (DstSq !=255) //>= 0)
              {
                g_PawnMoves[SrcSq][i][j] = DstSq+1;
              }
              else
              	g_PawnMoves[SrcSq][i][j] = 0;
            }
        }
*/
    } // end if (nFile(SrcSq) < 9)
    }	// end for SrcSq

    //put advele moves in KnightMoves
    for (i=0; i<12; i++)
    {
    	for (j=0; j<2; j++)
    		g_KnightMoves[ele_pos[i]][j+9] = ele_moves[i][j];
    }

    for (i=0; i<4; i++)
    {
    	for (j=0; j<4; j++)
    		g_KnightMoves[advele_center_pos[i]][j+9] = advele_center_moves[i][j];
    }

		for (i=0; i<8; i++)
    {
    		g_KnightMoves[bis_pos[i]][9] = bis_moves[i];
    }
    /*
    //print g_PawnMoves for debug
    FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
    fprintf(traceout, "g_PawnMoves\n");

    for (SrcSq=0; SrcSq<BOARD_SIZE-7; SrcSq++)
    {	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");

    		fprintf(traceout, "{");
    		for (i=0; i<2; i++)
    	 {

    	 	fprintf(traceout, "{");
    		for (Index=0; Index<4; Index++)
    		{	fprintf(traceout, "%3d", g_PawnMoves[SrcSq][i][Index]);
    			if (Index<3)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "}");

    		if (i<1)
    				fprintf(traceout, ",");
    	  }
    	 fprintf(traceout, "},");

    }
    fprintf(traceout, "\n");

    fclose(traceout);
    */


    /*
    //print g_KingMoves, g_advelemoves for debug
    FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append

    	for (SrcSq=0; SrcSq<BOARD_SIZE-7; SrcSq++)
    	{	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");
    		fprintf(traceout, "{");
    		for (Index=0; Index<8; Index++)
    		{	fprintf(traceout, "%3d", g_KingMoves[SrcSq][Index]);
    			if (Index<7)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "},");
    	}
    	fprintf(traceout, "\n");
    	fclose(traceout);
    */
    /*
    	for (SrcSq=0; SrcSq<BOARD_SIZE; SrcSq++)
    	{	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");
    		fprintf(traceout, "{");
    		for (Index=0; Index<8; Index++)
    		{	fprintf(traceout, "%3d", g_advelemoves[SrcSq][Index]);
    			if (Index<7)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "},");
    	}
    	fprintf(traceout, "\n");

    fclose(traceout);
    */
/*
    FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
			fprintf(traceout, "g_KnightMoves\n");
    	for (SrcSq=0; SrcSq<BOARD_SIZE-7; SrcSq++)
    	{	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");
    		fprintf(traceout, "{");
    		for (Index=0; Index<16; Index++)
    		{	fprintf(traceout, "%3d", g_KnightMoves[SrcSq][Index]);
    			if (Index<15)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "},");
    	}
    	fprintf(traceout, "\n");

    	fprintf(traceout, "g_HorseLegs\n");
    	for (SrcSq=0; SrcSq<BOARD_SIZE-7; SrcSq++)
    	{	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");
    		fprintf(traceout, "{");
    		for (Index=0; Index<8; Index++)
    		{	fprintf(traceout, "%3d", g_HorseLegs[SrcSq][Index]);
    			if (Index<7)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "},");
    	}
    	fprintf(traceout, "\n");
    fclose(traceout);
*/
/*
FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
			fprintf(traceout, "g_KnightChecks\n");
			for (int j=0; j<18; j++)
			{
				fprintf(traceout, "kingposidx = %d\n", j);
    	for (SrcSq=0; SrcSq<BOARD_SIZE-7; SrcSq++)
    	{	if ((SrcSq &15)==0)
    			fprintf(traceout, "\n");
    		fprintf(traceout, "{");
    		for (Index=0; Index<2; Index++)
    		{	fprintf(traceout, "%3d", g_KnightChecks[SrcSq][j][Index]);
    			if (Index<1)
    				fprintf(traceout, ",");
    		}
    		fprintf(traceout, "},");
    	}
    	fprintf(traceout, "\n");
    }
    fclose(traceout);
*/
#ifdef PVRED
  // Init our reduction lookup tables
  for (int i = 1; i < 64; i++) // i == depth (OnePly = 1)
      for (int j = 1; j < 64; j++) // j == moveNumber
      {
          double    pvRed = 0.5 + log(double(i)) * log(double(j)) / 6.0;
          double nonPVRed = 0.5 + log(double(i)) * log(double(j)) / 3.0;
          PVRedMatrix[i][j]    = (unsigned char) (   pvRed >= 1.0 ? floor(   pvRed ) : 0);
          NonPVRedMatrix[i][j] = (unsigned char) (nonPVRed >= 1.0 ? floor(nonPVRed ) : 0);
      }
#endif
/*
  printf("PV reduction table\n");
  for (int i=1; i<64; i++)
  {
  	for (int j=1; j<64; j++)
  	{
  		printf("%3d", PVRedMatrix[i][j]);
  	}
  	printf("\n");
 }

 printf("NonPV reduction table\n");
  for (int i=1; i<64; i++)
  {
  	for (int j=1; j<64; j++)
  	{
  		printf("%3d", NonPVRedMatrix[i][j]);
  	}
  	printf("\n");
 }
*/

}



//const int HistoryMax = 16384; // 32765; //65534;    // 用於裁剪的歷史表的最大值﹔
//const int HistValMax = 16384; // 32000; //64000;    // 用於啟發的歷史表的最大值﹔
#define HistoryMax 32000 //16384
#define HistValMax 32000 //16384



// 歷史表啟發的深度相關的增加值，採用Crafty、Fruit等程序的方案，即深度的平方
//static const uint32_t HistInc[64] = {
//    0,   1,   4,   9,  16,  25,  36,  49,  64,  81, 100, 121, 144, 169, 196, 225,
//  256, 289, 324, 361, 400, 441, 484, 529, 576, 625, 676, 729, 784, 841, 900, 961,
//  1024,1089,1156,1225,1296,1369,1444,1521,1600,1681,1764,1849,1936,2025,2116,2209,
//  2304,2401,2500,2601,2704,2809,2916,3025,3136,3249,3364,3481,3600,3721,3844,3969
//};

// for ABDADA, hFlag (2bits) definition
//#define HASH_EXCLUSIVE 1   //use 1 bit of hval instead
//#define HASH_ALPHA  2
//#define HASH_BETA  1
//#define HASH_PV  3        // HASH_ALPHA | HASH_BETA;

#define HASH_BETA  -1  //befABDADA
#define HASH_ALPHA  0
#define HASH_PV     1        

//const int HASH_LAYERS = 4;   // 置換表的層數
//const int MAX_FRESH = 15;    // 最大新鮮度
//int NULL_DEPTH = 2; //2; //3; //2;    // 空著裁剪的深度
#define HASH_LAYERS 4 //8 //4 //5 //6 //6x10byte //4 //2  //4   // 置換表的層數
//const int MAX_FRESH = 15;    // 最大新鮮度
//#define NULL_DEPTH 3 //= 2; //2; //3; //2;    // 空著裁剪的深度

// 沒有命中置換表而返回值的失敗值，必須在-MATE_VALUE和MATE_VALUE以外
const int UNKNOWN_VALUE = INF + 1; //MATE_VALUE + 1;

#define BAN_VALUE (INF-100) //20230308 1948 //(INF-100) //1900;
#define WIN_VALUE (INF-150) //20230308 1848 //1792 //(INF-256) //1800;

#ifdef DEBUG
FILE *out = fopen("search.txt", "a+"); //w+");   //use append
#endif



bool operator<(const MoveTabStruct& a, const MoveTabStruct& b)  // for std::sort
{
//    return a.score < b.score;
return a.tabval > b.tabval;   //descending
}



#ifdef __GNUC__
inline
#else  
__forceinline
#endif
static int GetNextMove(int n, int size, MoveTabStruct movetab[])
{
short nval = movetab[n].tabval;
int q = n;
for (int i=n+1; i<size; i++)
	{
		if (movetab[i].tabval > nval)
		{
			nval = movetab[i].tabval;
			q = i;
		}
	}
		int tmp = movetab[q].tabentry;
		if (q != n)
		{
			movetab[q].tabentry = movetab[n].tabentry;
			movetab[n].tabentry = tmp;
		}

	return tmp;
}

//int printed=0;

/*
// 過濾禁止著法
int Engine::IsBanMove(int mv) {
  int i;
  for (i = 0; i < nBanMoves; i ++) {
    if (mv == wmvBanList[i]) {
      return 1;
    }
  }
  return 0;
}
*/
//static int FUTILITY_MARGIN = 50; //40;  //40

// pawn value=25, margin1=4p, margin2=12p
// toga 1.2.1a margin1=1p, margin2=3p, margin3(not used)=9.5p
// glaurung    margin1=2p, margin2=6p
// eychessu    margin1=1b, margin2=3b, margin3(not used)=7b where b=ADVISOR=50
// crafty 20.14 margin1=(ADVISOR+1)/2 margin2=(queen+1)/2


//uint32_t nEvalHits=0;
//uint32_t nEvalRecs=0;
//uint32_t nPawnHits=0;
//uint32_t nPawnMiss=0;
//uint32_t nThreat[12]={0,0,0,0,0,0,0,0,0,0,0,0}; //, {0,0,0,0,0,0,0,0,0,0,0,0}};
//uint32_t nThreat_justendgame=0;
//uint32_t nThreat_deependgame=0;
//uint32_t nHistCuts=0;

extern int ext_p_endgame;
extern int p_movenum;
extern int p_bookfound;
extern int p_feedback_move;
static int ponder_move=0;
static int respmove_to_ponder=0;
//static int prev_boardsq[34]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0};
//static unsigned char prev_piece[BOARD_SIZE-7];
static unsigned char prev_boardsq[34];
extern int p_nBanmove;
extern int p_banmove[16];
//static int prev_ponder_depth=2;
//static const int ADVISOR_POS[5]={3,5,13,21,23};
//static const int ELEPHAN_POS[7]={2,6,18,22,26,38,42};
// move to lazy smp id loop
//#ifdef ASPWIN
//int ASP_WINDOW; //=16; //16; //32; //30; //16 //30 //30 //30 //40 //48 //24 //35 //70 //65 //40 //70 //20 //30 //40 //30 //40
//int ValueByIteration[MAXDEPTH];
//#else
//#define ASP_WINDOW 0
//#endif
//static const int WINDOW[64]=   //60; //30 //40 //30 //40
//{110,105,100,95,90,85,80,75,70,65,60, 55,50,45,40,35,35,35,35,35,35, 35,35,35,35,35,35,35,35,35,
// 25,25,25,25,25,25,25,25,25,25, 25,25,25,25,25,25,25,25,25,25, 25,25,25,25,25,25,25,25,25,25, 25,25,25,25};
//{80,70,70,65,60,50,45,45,40,40, 35,35,30,30,25,25,25,25,25,25, 25,25,25,25,25,25,25,25,25,25,
//25,25,25,25,25,25,25,25,25,25, 25,25,25,25,25,25,25,25,25,25, 25,25,25,25,25,25,25,25,25,25, 25,25,25,25};

//static int DROPDOWN_VAL=45; //40; //45; //45; //35; //35; //40; //[2] = {40,40};
static const int CENTRAL_PAWN[2] = {52, 100}; //159-52-7

void Engine::printf_info_nps(int best, Board &board)
{
//printf("info time %d nodes %d\n", clock() - board.m_startime, board.m_nodes);
                        int TimeSpan = (int)(GetTime() - board.m_startime); //(clock() - board.m_startime) ;
                        if (TimeSpan==0) TimeSpan=1;
                        //uint32_t u32board.m_nodes = board.m_nodes;
                        int nps = int((double)board.m_nodes * 1000 / (double)TimeSpan);
                        //printf("info time %d nodes %d nps %d\n", TimeSpan, board.m_nodes, board.m_nodes/(TimeSpan+1));
                        //printf("info nps %d time %d nodes %d \n", board.m_nodes/(TimeSpan+1), TimeSpan, board.m_nodes);
                        char str[5];
    MoveStruct tempmove;
    tempmove.move=board.m_bestmove;
                    
    //printf("info depth %d score %d time %d nodes %d nps %d pv %s\n", m_depth-1, best, TimeSpan, u32board.m_nodes, nps,
//    printf("info depth %d score %d time %d nodes %llu nps %d pv %s\n", m_depth-1, best, TimeSpan, board.m_nodes, nps,
	printf("info depth %d score %d time %d nodes %llu nps %d pv %s\n", 0, best, TimeSpan, board.m_nodes, nps,
    MoveStr(str, tempmove.from, tempmove.dest) );
                        fflush(stdout);
}



int Engine::searchRoot()
{

// copy from ext_p in eychessu.cpp to board.p_
    board.IMaxTime = ext_IMaxTime;
    board.p_endgame = ext_p_endgame;
 	  
//smp    int best; //, size; //, j; //RootAlpha, RootBeta, size, j; //, piecefrom; //j,banned;
//	int alpha, beta;
 //smp   MoveStruct tempmove;
//    char charmove[5];


		board.Compress_index();

		board.bitpiece=0;
		memset(board.piececnt,0,sizeof(board.piececnt));
#ifdef EVALATTK
		memset(board.bitattk,0,sizeof(board.bitattk));
#endif
    //for (i = 0; i < 10; i ++) {
    //	board.wBitRanks[i] = 0;
    //}
    //for (i = 0; i < 9; i ++) {
    //	board.wBitFiles[i] = 0;
    //}
    memset(board.wBitRanks,0,sizeof(board.wBitRanks));
    memset(board.wBitFiles,0,sizeof(board.wBitFiles));


for (int i=2; i<34; i++)
    {
        int sq = board.boardsq[i];
        //if (sq >=0 )
        if (NOTSQEMPTY(sq))
        {
            //if (i>=B_HORSE && i<=R_ROOK)
            //if (i>=20 && i<=31)
            //    side_power_piece[(i&1)]++;
            //bitpiece, board.p_endgame not yet determine
            //pointsum += pointtable[sq][PIECE_IDX(i)][board.p_endgame]; //[i &29];
            board.piececnt[i &61]++;
            if (i<B_KING) // not king
            {
                board.bitpiece ^= (1 << i); //BITPIECE_MASK[i];
#ifdef EVALATTK
                board.bitattk[ATTKAREA[sq]] ^= (1 << i);
#endif
                //bitpiece ^= BITPIECE_MASK[i];
                //_bittestandcomplement(&bitpiece, i);
            }
            board.wBitRanks[nRank(sq)] ^= PreGen.wBitRankMask[sq];
            board.wBitFiles[nFile(sq)] ^= PreGen.wBitFileMask[sq];

        }
    }



#ifdef PERFT
#else
    //int bookmove,book_boardsq[34];
    unsigned char book_boardsq[34];   //20190912 int
    int bookmove;

		//int old_IMaxTime = board.IMaxTime;
    //don't know why, but has to set m_side to black meaning computer's turn
    //board.m_side = IComSide;
int val=0;
    // search book using board.boardsq
    //1892g if (board.IMaxTime >= 5000)  //1891c - search book only if enough time
    if (!board.p_endgame)  //20230216 no need to set p_endgame if already set in engine.Iread
    	AdjustEndgame(board);  //set board.p_endgame for pointsum recal //1892k move to here
    if (board.p_endgame == 0 && board.IMaxTime >= 1000) //1892k	
    if (p_bookfound<20)
    {
        board.m_startime=GetTime();
        for (int i=0; i<34; i++)
        {
            val=board.boardsq[i];
            if (NOTSQEMPTY(val))
            	book_boardsq[i] = (nRank(val) * 9) + nFile(val);
            else
            	book_boardsq[i] = SQ_EMPTY;   //20190912 -1
        }
        //bookmove = srchboob(board.boardsq, 1 - IComSide); //IComSide);   //IComSide=0 COMPUTER SIDE=BLACK
        //bookmove = srchboob(book_boardsq, 1 - IComSide);
        bookmove = srchboob(book_boardsq, BLACK); //1 - board.m_side);
        if (bookmove != 0)
        {
			MoveStruct tempmove;
			//board.m_bestmove = ((bookmove>>7) <<8) + (bookmove&127);
            tempmove.from = bookmove>>7;
            tempmove.dest = bookmove&127;

            tempmove.from = ADJ9_16(tempmove.from);
            //verify bookmove - must be same side to move (may be diff due to transposition)
            if ((board.piece[tempmove.from]&1) == board.m_side)
            //printf("from=%d, piece=%d, m_side=%d\n", tempmove.from, board.piece[tempmove.from], board.m_side);
            {
            	tempmove.dest = ADJ9_16(tempmove.dest);
            	//verify bookmove
            	if (board.LegalKiller(tempmove))
            	{
           				board.m_bestmove = tempmove.move;
            //FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
            //fprintf(traceout, "%s%d %s%d\n", "board.m_bestmove.from=", board.m_bestmove.from, "board.m_bestmove.dest=", board.m_bestmove.dest);
            //fflush(traceout);
            //fclose(traceout);

            //printf("book move found\n");
            			p_bookfound++;
            			//printf("info book move found\n");
            			//fflush(stdout);
//            			printf_info_nps(0);
            			return board.m_bestmove;
          		}
          		//printf("info bookmove error\n");
          	}
        }

        //fprintf(traceout, "%s\n", "bookfound=0");
        //fflush(traceout);

        //printf("info Leave book\n");
        //fflush(stdout);
//        printf_info_nps(0);
        //
        // inc board.IMaxTime when leaving book
        if (p_bookfound>0)
        {	p_bookfound=20; //0; 
        	board.IMaxTime += (board.IMaxTime>>2);
        	//printf("info inc board.IMaxTime to %d when leave book\n", board.IMaxTime);
          //fflush(stdout);
      }

       
    }
    
     if (p_movenum >=20)
        	p_bookfound=20;
#endif //end PERFT



    board.m_startime=GetTime(); //clock();
    board.m_nodes=0;
    board.m_timeout=0;

    board.ply=0;


  	//AdjustEndgame(board);  //set board.p_endgame for pointsum recal
  	//88c - ClearHash if endgame
  	//if (board.p_endgame)
  	//	ClearHash();

#ifdef PREEVAL
//preEval pointtable to adjust bis/ele val
//  	memcpy(pointtable[board.p_endgame][2], pointtableAdvEleB[board.p_endgame], sizeof(pointtableAdvEleB[board.p_endgame]));
//  	memcpy(&pointtable[board.p_endgame][3][BOARD_SIZE/2], pointtableAdvEleR[board.p_endgame], sizeof(pointtableAdvEleR[board.p_endgame]));
/*
for (int i=0; i<9; i++)
{
for (int j=0; j<5; j++)
{
	 pointtable[2][i][j] = pointtableAdvEleB[i][j];
	 pointtable[3][i][9-j] = pointtableAdvEleR[i][j];
}

for (int j=0; j<7; j++)
{
	 pointtable[0][i][j+3] = pointtablePawnB[i][j];
	 pointtable[1][i][9-(j+3)] = pointtablePawnR[i][j];
}
}
*/
memcpy(&(pointtable[2][0][0]), pointtableAdvEleB, sizeof(pointtableAdvEleB));
memcpy(&(pointtable[3][5][0]), pointtableAdvEleR, sizeof(pointtableAdvEleR));
memcpy(&(pointtable[0][3][0]), pointtablePawnB, sizeof(pointtablePawnB));
memcpy(&(pointtable[1][0][0]), pointtablePawnR, sizeof(pointtablePawnR));

  	uint32_t bitattkB = (bitattk[0] | bitattk[2] | bitattk[4]);
  	int nattkR = bitCount(bitattkB & 0xAAA00AA8)
  	//+ bitCount(bitattkB & 0xA0A00000)	//RH=2, CP=1
  	//+ bitCountLSB((bitattkB & 0xA0A00000)>>16)	//RH=2, CP=1
  	+ bitCountMSB((bitattkB & 0xA0A00000))	//RH=2, CP=1
  	+ bitCountRookR(bitattk[6] | bitattk[8]);
  	uint32_t bitattkR = (bitattk[1] | bitattk[3] | bitattk[5]);
  	int nattkB = bitCount(bitattkR & 0x55500554)
  	//+ bitCount(bitattkR & 0x50500000)	//RH=2, CP=1
  	//+ bitCountLSB((bitattkR & 0x50500000)>>16)	//RH=2, CP=1
  	+ bitCountMSB((bitattkR & 0x50500000))	//RH=2, CP=1
  	+ bitCountRookB(bitattk[7] | bitattk[9]);
/*
  	// 如果本方輕子數比對方多，那麼每多一個輕子(車算2個輕子)威脅值加2。威脅值最多不超過8。
  nWhiteSimpleValue = PopCnt16(lppos->wBitPiece[0] & ROOK_BITPIECE) * 2 + PopCnt16(lppos->wBitPiece[0] & (HORSE_BITPIECE | CANNON_BITPIECE));
  nBlackSimpleValue = PopCnt16(lppos->wBitPiece[1] & ROOK_BITPIECE) * 2 + PopCnt16(lppos->wBitPiece[1] & (HORSE_BITPIECE | CANNON_BITPIECE));
  if (nWhiteSimpleValue > nBlackSimpleValue) {
    nWhiteAttacks += (nWhiteSimpleValue - nBlackSimpleValue) * 2;
  } else {
    nBlackAttacks += (nBlackSimpleValue - nWhiteSimpleValue) * 2;
  }
*/
  	int nSimpleB = bitCountRookB(bitpiece) *2 + bitCountCanHorB(bitpiece);
  	int nSimpleR = bitCountRookR(bitpiece) *2 + bitCountCanHorR(bitpiece);
  	if (nSimpleB > nSimpleR)
  		nattkB += (nSimpleB - nSimpleR) * 2;
  	else
  		nattkR += (nSimpleR - nSimpleB) * 2;
  	if (nattkR >=8) nattkR=8;
  	if (nattkB >=8) nattkB=8;

  	int p;
  	//if ( ((bitattk[0] | bitattk[2] | bitattk[4] ) & 0xAAA00AA8) ==0)
  	{
  		for (int n=0; n<9; n++)
  		for (int m=0; m<5; m++)
  		{
  			p=pointtable[2][m][n];
  			p = p*(8 + nattkR)/8;
  			pointtable[2][m][n]=p;

  			p=pointtable[3][9-m][n];
  			p = p*(8 + nattkB)/8;
  			pointtable[3][9-m][n]=p;
  		}
  	}

//adjust pointtablepawn
for (int n=0; n<9; n++)
  		for (int m=0; m<7; m++)
  		{
  			p=pointtable[0][m+3][n];
  			p = p*(8 + nattkB)/8;
  			pointtable[0][m+3][n]=p;

  			p=pointtable[1][6-m][n]; //9-(m+3)
  			p = p*(8 + nattkR)/8;
  			pointtable[1][6-m][n]=p;
  		}
#endif

//if only pawns and opp has cannon, adjust pointtablepawn not to move forward
if ((board.bitpiece & 0x55500000)==0 && (board.bitpiece & 0x0A000000)!=0)
{

	for (int j=7; j<10; j++)
  		for (int k=0; k<9; k++)
  		{
  			//pointtable[j*16+k][0][board.p_endgame] /=2;
  			pointtable[ext_p_endgame][0][j][k] /=2;   //20230209
  		}


}
if ((board.bitpiece & 0xAAA00000)==0 && (board.bitpiece & 0x05000000)!=0)
{
	for (int j=0; j<3; j++)
  		for (int k=0; k<9; k++)
  		{
  			//pointtable[j*16+k][1][board.p_endgame] /=2;
  			pointtable[ext_p_endgame][1][j][k] /=2;   //20230209
  		}
}
// 調整不受威脅方少掉的仕(士)相(象)分值
#ifdef PREEVAL
board.pointsum = (nattkB - nattkR) * 15;
#else
board.pointsum=0;
#endif

for (int i=2; i<34; i++) //i+=2)
    {
        int sq = board.boardsq[i];
        if (NOTSQEMPTY(sq))
        {
            board.pointsum += pointtable[ext_p_endgame][PIECE_IDX(i)][nRank(sq)][nFile(sq)];  //20230209
        }
    }
/*
for (i=3; i<34; i+=2)
    {
        int sq = board.boardsq[i];
        if (NOTSQEMPTY(sq))
        {
            pointsum -= pointtable[PIECE_IDX(i)][nRank(sq)][nFile(sq)];
        }
    }
*/
/*
//debug
    printf("info nattkB=%d\n", nattkB);
    printf("info nattkR=%d\n", nattkR);
    printf("info pointtable: endgame=%d\n", board.p_endgame);
    for (int i=0; i<4; i++)
    {	printf("i=%d\n", i);
    	for (int m=0; m<10; m++)
      {      for (int n=0; n<9;  n++)
            {
                int j=(m*16) + n;
    						printf("%5d", pointtable[j][i][board.p_endgame]);
    				}
    				printf("\n");
  		}
    }
printf("info pointsum=%d\n", pointsum);
*/

/* move to adjustendgame
if ((board.bitpiece & 0xf0000000) ==0) // no rooks
  {
	  FUTPAWN = ENDFUTPAWN;
	  DELTA_MARGIN = ENDFUTPAWN;
#ifdef TTSINGU
	  SINGULAR_MARGIN = ENDFUTPAWN/2; //*3/4; //*2; ///8;
//	  SINGULAR_MARGIN_PV = ENDFUTPAWN/4; // /8;
#endif
	  //FUT_MARGIN[1]=ENDFUTPAWN;
    //FUT_MARGIN[2]=ENDFUTPAWN*2;
    //FUT_MARGIN[3]=ENDFUTPAWN*4;

		//memcpy(EMPTY_CANN_SCORE, EMPTY_CANN_SCORE_endgame, sizeof(EMPTY_CANN_SCORE));
  }
*/

//七、初始化一些有用的變量
// 這些變量用於測試搜索樹性能的各種參數
//    int old_IMaxTime = board.IMaxTime;

//	nHashMoves = nHashCuts = 0;

//	nTreeHashHit = nLeafHashHit = 0;
#ifdef TTSINGU
//    pv_singular_cnt = nonpv_singular_cnt = 0;
#endif
//debug
#ifdef DEBUG
    nTreeNodes = nLeafNodes = 0;
//	nBetaNodes = 0;
//	nHashCuts = 0;
//	nHashMoves = 0;
    nQuiescNodes = 0;
    nFutility = nExtFutility = 0;
    nHistPrunNodes = 0;
    nExtHistPrunNodes = 0;
//	nHistPrunVers = 0;
#endif
//	nEvalHits = nEvalRecs = 0;
//	nPawnHits = nPawnMiss = 0;
//

//	for (int j=0; j<12; j++)
//		nThreat[j]=0;
// nThreat_justendgame=0;
// nThreat_deependgame=0;
//
//	nHistCuts=0;
//	lazya = lazyb = 0;
//	nBetaMoveSum = nBetaCutAt1 = 0;
//	nNullMoveNodes = nNullMoveCuts = 0;

    //search_stack_t sstack[MAXPLY];
    //Clear_Killer(); //1891c - same with Clear_Hist
    
#ifdef PERFT
for (int d=1; d<=5; d++)   //d<=6 20230217
{
  board.m_startime = GetTime();
	printf("depth=%5d  ",d);
	fflush(stdout);
	//uint64_t nPerft = Perft(d);
	printf("Perft(%d)=%16llu  Std Perft=%16llu  ", d, Perft(d), StdPerft(d)); //STD_PERFT[d]);
  printf("time=%d \n", (int)(GetTime() - board.m_startime));
  fflush(stdout);
}
return 0;
#endif //end PERFT

    // init_node

//debug

		//printf("m_hisindex=%d\n", m_hisindex);
		//for (int i=0; i<=m_hisindex; i++)
		//{	printf("i=%d Chk=%d\n", i, m_hisrecord[i].htab.Chk);
// printed=1;}

    //nCheckEvasions = 0;
    //nZugzwang = 0;
    //int incheck=board.IsInCheck(board.m_side );
    //m_hisrecord[m_hisindex].Chk =incheck;
    //int incheck=0;
    if (board.m_hisindex > 0)
        board.incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;
    else
        //incheck=board.IsInCheck(board.m_side, 0);	//not singlechk
        board.incheck=board.IsInCheck<0>(board.m_side);	//not singlechk
        //m_hisrecord[0].htab.Chk = incheck;

    //printf("incheck=%d\n", incheck);
 //   int newdepth; //depth
    // 單方最多合理的走法111種： 將與單士(2+4=6), 雙象(4+2＝6), 雙車(17×2＝34), 雙炮(17*2=34), 雙馬(8*2=16), 五個兵(3*5=15)
    //MoveStruct table[111];
    //int  tabval[111];
    //MoveTabStruct movetab[111];  //smp
    MoveTabStruct ncapmovetab[64];
//    int board.root_pv[256]; //smp keep in board.h
//    memset(board.root_pv, 0, sizeof(board.root_pv));

    long ncapsize; //=0;
    //int max_root_nodes;
    board.nBanMoves=0;
    if (board.incheck)
    {
    	for (int k=0; k<p_nBanmove; k++)
        p_banmove[k] = 0;	//if incheck, not consider banmove
        //printf("info Before GenChkEvasion\n");
        //fflush(stdout);
        //size=board.GenChkEvasion(movetab, incheck);
        //if (incheck < 8)
        	board.size=board.GenChkEvasCap(&board.movetab[0], board.incheck);
        //else
        //	size=board.GenChkEvasRookCann(&movetab[0], incheck &7);


        //memcpy(&movetab[size], &ncapmovetab[0], ncapsize * 4);
        //size += ncapsize;
        ncapsize=board.GenChkEvasNCap(&board.movetab[board.size], board.incheck);
        //memcpy(&movetab[size], &ncapmovetab[0], ncapsize * 4);
        board.size += ncapsize;
        //printf("info Root GenChkEvasion OK\n");
        //fflush(stdout);
    }
    else
    {	//printf("info Before Gen\n");
        //fflush(stdout);

        //size=(board.m_side ? board.GenCap<1>(&movetab[0], &ncapmovetab[0], ncapsize)
        //: board.GenCap<0>(&movetab[0], &ncapmovetab[0], ncapsize));
        board.size=board.GenCap(&board.movetab[0], &ncapmovetab[0], ncapsize);
        memcpy(&board.movetab[board.size], &ncapmovetab[0], ncapsize * 4);
        board.size += ncapsize;

//        size=board.GenCapQS(&movetab[0]);
        ncapsize=board.GenNonCap(&board.movetab[board.size], 0);
        board.size += ncapsize;

        //ncapsize=board.GenNonCapPBEK(&movetab[size]);
        //size += ncapsize;

        //printf("info Root Gen OK\n");
        //fflush(stdout);
    }
    //derive p_feeback_move from prev_piece
    
//    std::sort(movetab, movetab+size); //for sort root_seq

    if (p_feedback_move==0 && respmove_to_ponder !=0 && ponder_move !=0)
    {
		MoveStruct tempmove;
    	/*
    	printf("            ");
    	for (int i=0; i<BOARD_SIZE-7; i++)
    	  printf("%3d ",i);
    	printf("\n");
    	printf("prev_piece : ");
      for (int i=0; i<BOARD_SIZE-7; i++)
      	printf("%3d ",prev_piece[i]);
      printf("\nboard.piece: ");
      for (int i=0; i<BOARD_SIZE-7; i++)
      	printf("%3d ",board.piece[i]);
      printf("\n");

      printf("prev_boardsq : ");
      for (int i=0; i<34; i++)
      	printf("%3d ",prev_boardsq[i]);
      printf("\nboard.boardsq: ");
      for (int i=0; i<34; i++)
      	printf("%3d ",board.boardsq[i]);
      printf("\n");
      //printf("board.m_side: %d\n", board.m_side);
      */
			/*
        for (int j=0; j<BOARD_SIZE-7; j+=16)
        for (int k=0; k<9; k++)
        {
        	int i=j+k;
            if (PIECE_IDX(board.piece[i]) != PIECE_IDX(prev_piece[i]))
            {
            	if (board.piece[i])
            	 tempmove.dest = i;
              else
              	tempmove.from = i;
             }
        }
      */
;     for (int p=32+board.m_side; p>1; p-=2)
      {
      	if (NOTSQEMPTY(prev_boardsq[p]))
      	{
      		if (PIECE_IDX(p) != PIECE_IDX(board.piece[prev_boardsq[p]]) )
      		{
              		tempmove.dest = prev_boardsq[p];
              		break;
          }
      	}
      }
      for (int p=33-board.m_side; p>1; p-=2)
      {
      	if (NOTSQEMPTY(prev_boardsq[p]))
      	{
      		if (PIECE_IDX(p) != PIECE_IDX(board.piece[prev_boardsq[p]]) )
      		{
             tempmove.from = prev_boardsq[p];
             break;
          }
      	}
      }
      /*
      for (int p=2+board.m_side; p<34; p+=2)
      {
      	if (NOTSQEMPTY(prev_boardsq[p]))
      	{
      		if (PIECE_IDX(p) != PIECE_IDX(board.piece[prev_boardsq[p]]) )
      		{
          	//if (board.piece[prev_boardsq[p]])
              		tempmove.dest = prev_boardsq[p];

              		break;
          }
      	}
      }


      for (int p=3-board.m_side; p<34; p+=2)
      {
      	if (NOTSQEMPTY(prev_boardsq[p]))
      	{
      		if (PIECE_IDX(p) != PIECE_IDX(board.piece[prev_boardsq[p]]) )
      		{
             tempmove.from = prev_boardsq[p];
             break;
          }
      	}
      }
      */
      p_feedback_move = tempmove.move;
    //com2char(charmove, tempmove.from, tempmove.dest );
    //printf("derived lastmove %s from=%d dest=%d\n", charmove, tempmove.from, tempmove.dest);
    }


    if (p_feedback_move==0 || p_feedback_move != ponder_move)
    {    respmove_to_ponder = 0;	//clear respmove_to_ponder if feedback not match
    	   //ClearHash();
    }

    //leave more time for stopping

//	  if (old_IMaxTime < 10)  // 2892q not worked, diabled
//	  {
//	  	board.IMaxTime /= 2;
//		  old_IMaxTime = board.IMaxTime;
//		  printf("     ***leave more time for stopping to avoid timeout\n");
//		  fflush(stdout);	
//	  }
//-----------------------------

    MoveStruct tempmove;
    int best=-INF;
    for (int i=0;i<board.size;++i)
    {    	
    		tempmove.move = board.movetab[i].table.move;
    		int isBanmove = 0;
    		for (int k=0; k<p_nBanmove; k++)
    		{
        if (tempmove.move == p_banmove[k]
        //	 && !incheck && board.piece[tempmove.dest] < B_KING
        	)
          {

            board.movetab[i].tabval = -BIGVAL;
            board.nBanMoves ++;
            p_banmove[k] = 0;	//reset banmove
            isBanmove = 1;
            break;
          }
        }
        
        if (isBanmove)
        	continue;  // next i
        	
                //for (i=0;i<size;++i)
            {
                //if ( makemove(tempmove) < 0 )
                if ( board.makemove(tempmove, 1) < 0) //, 1) < 0 )
                {
                    //wmvBanList[nBanMoves] = table[i].move;
                    board.movetab[i].tabval = -BIGVAL;
                    board.nBanMoves ++;
                }
                else if ( !board.incheck && board.piece[tempmove.dest] < B_KING
                	&& (abs(board.checkloop(3)) > DRAWVALUE) // ||  checkloop(3) != 0)
                	//&& (abs(checkloop(3)) > DRAWVALUE) // ||  checkloop(3) != 0)
                	)
                {
                    board.movetab[i].tabval = -BIGVAL;
                    board.nBanMoves ++;
                    board.unmakemove();
                }

            		else if (tempmove.move == respmove_to_ponder)
            		{	//return respmove_to_ponder;
                		board.movetab[i].tabval = BIGVAL;
                    board.unmakemove();
                    best = BIGVAL;
                    board.m_bestmove = tempmove.move;
                //start_depth = prev_ponder_depth - 1;
                //printf("info ponder move matched\n");
                //fflush(stdout);

            		}
                else
                {
                    board.movetab[i].tabval = -quiesCheckPV(board, -INF, INF, 0, root_qcheck_depth);                    
                    board.unmakemove();
                    if (board.m_timeout)
                    {                    	
                    	break;
                    }
                    if (board.movetab[i].tabval > best)
                    {
                    	best = board.movetab[i].tabval;
                    	board.m_bestmove = board.movetab[i].table.move;
                    }
                }
            }
/* lazy smp - disable time management         
      if (board.IMaxTime < 1000)
          {
            BusyComm = BusyLine(UcciComm, false);
            if (BusyComm == UCCI_COMM_STOP)
            //if (BusyLine(UcciComm, false) == UCCI_COMM_STOP )
            {
            	// "stop"指令發送中止信號
            board.m_timeout= 1;	//stop
            //printBestmove(board.m_bestmove);
            printf("     ***searchRoot QS stopped by GUI pv\n");
            fflush(stdout);            
            	break;            	
            }

            else if (BusyComm == UCCI_COMM_QUIT)
            {
            	p_feedback_move = -1; //pass to Eychessu.cpp to quit the engine
            	//printf("info UCCI_COMM_QUIT p_feedback_move = %d\n", p_feedback_move);
			        //fflush(stdout);
            }
           
          	  
    		
    		long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);
    				if (t_remain <= 0)
            {                        
                  	 board.m_timeout= 1; //stop
    printf("     *****searchRoot QS timout\n"); // %d board.ply %d board.IMaxTime %d\n", (int)(GetTime() - board.m_startime), board.ply, board.IMaxTime);
    fflush(stdout);            	    
                  //return UNKNOWN_VALUE;
                  break;
            }
    		  }   
*/    		  
//        root_seq[i] = i;    
    } //end for (i=


		if (board.m_timeout)
		{
#ifdef THDINFO			
			printf("     *** panic! stopping at searchroot QS\n");
			fflush(stdout);
#endif			
			return board.m_bestmove;
		}

    //Quicksort(0, size-1, movetab);

    std::sort(board.movetab, board.movetab + board.size);
    //Quicksortroot(0, size-1, movetab, root_seq);	
		//Insertsortroot(size, movetab, root_nodes);


    /*
    // debug after pregen g_HorseLegs
    FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
    fprintf(traceout, "\n");
    //printf("\n");
    //fflush(stdout);
    for (int i=0; i<10; i++)
    {	for (int j=0; j<9; j++)
    	{
    		for (int k=0; k<8; k++)
    		{
    	fprintf(traceout, " %2d(%2d) ", g_KnightMoves[(i*16)+j][k], g_HorseLegs[(i*16)+j][k] );
    	//printf(" %c ", PieceChar[board.piece[(i*16)+j]] );
    		}
    		fprintf(traceout, " %2d() ", g_KnightMoves[(i*16)+j][8] );
    		fprintf(traceout, "|");
    	}
    	fprintf(traceout, "\n");
    	//printf("\n");
    	//fflush(stdout);
    }
    fflush(traceout);
    //fflush(stdout);
      fclose(traceout);
    */

    /*
    // debug after fen position moves
    //    FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
    //fprintf(traceout, "\n");
    printf("\n");
    fflush(stdout);
    for (int i=0; i<10; i++)
    {	for (int j=0; j<9; j++)
    	{
    	//fprintf(traceout, " %c ", PieceChar[board.piece[(i*9)+j]] );
    	printf(" %c ", PieceChar[board.piece[(i*16)+j]] );
    	}
    	//fprintf(traceout, "\n");
    	printf("\n");
    	fflush(stdout);
    }
    //fflush(traceout);
    //fclose(traceout);
    */


    //
    // debug after fen position moves
//	FILE *traceout = fopen("trace.txt", "a+"); //w+");   //use append
//	fprintf(traceout, "\n Root moves: ");
/*
#ifdef PRTBOARD
    print_board();
    printf("\ninfo Root moves: ");
    for (int i=0; i<size; i++)
    {
        com2char(charmove, movetab[i].table.from, movetab[i].table.dest );
        //fprintf(traceout, " %s", charmove);
        printf("  %s", charmove);
    }
    //fprintf(traceout, "\n");
    printf("\n");
    //fflush(traceout);
    fflush(stdout);
    //fclose(traceout);
    //

    printf("info Root tbval: ");
    for (int i=0; i<size; i++)
    {
        printf("%6d", movetab[i].tabval);
        //printf("%6d", root_nodes[i]);
    }
    //fprintf(traceout, "\n");
    printf("\n");

    printf("nBanMoves=%d, incheck=%d\n", nBanMoves, incheck);
    //fflush(traceout);
    fflush(stdout);
    //fclose(traceout);
    //

#endif
*/
    /*
    printf("info pointtable: m_side=%d", board.m_side);
    for (int i=0; i<size; i++)
    {	piecefrom = board.piece[movetab[i].table.from];
    	printf("%5d%5d%5d|", piecefrom, pointtable[piecefrom][movetab[i].table.from],
    	pointtable[piecefrom][movetab[i].table.dest]);
    }
    //fprintf(traceout, "\n");
    printf("\n");
    //fflush(traceout);
    fflush(stdout);
    //fclose(traceout);
    */

    board.size = board.size - board.nBanMoves;
    //board.m_bestmove=0;
    board.m_bestmove = board.movetab[0].table.move;
//lazy smp    m_depth = 1; //0 (1892g) 
    //single move at root, just play it immediately
    //if (size==1)
    //	return board.m_bestmove;
    
// lazy smp - disable shallow move    
    //if (board.IMaxTime <= 1000) //1892g
//    if (board.IMaxTime <= 300)	//300 1892q
//    {	
    	//printf("     ***board.IMaxTime < 300, move immediate to avoid stop\n");
    	//fflush(stdout);
//    	printf_info_nps(movetab[0].tabval, board);
//    	return board.m_bestmove;
//    }	

// move inside lasy smp id loop
//    int EasyMove = 0;
    // Is one move significantly better than others after initial scoring ?
    //if ( rml.get_move_score(0) > rml.get_move_score(1) + EasyMoveMargin)
//    if ( size==1 || movetab[0].tabval > movetab[1].tabval + 96)	//EasyMoveMargin 96 = FUTPAWN*4 //0x200 sf1.8
//        EasyMove = board.m_bestmove; //rml.get_move(0);

//	int bestscore=-INF;
    board.m_nodes=0;
    board.root_depth=1;  //init for lazy smp 
//    m_time_check=0;

//    unsigned long long start_nodes;
//	int fpv=0;
//    int prev_best[256]; //=-INF;
//    int prev_bestmove[256];

//move to lazy smp id loop
//    int lastbest = INF;
//#ifdef ASPWIN
//    ValueByIteration[0] = movetab[0].tabval;
//#endif
    //int lastpv = movetab[0].table.move;
//	int move_count;
//    int print_depth=0;
//	for (int i=0; i<32; i++)
//	{	prev_best[i] = -INF;
//		prev_bestmove[i] = 0;
//	}
#ifdef DEBUG
    searchdepth=0;
#endif



// 2892q - lazy smp disable time management
//    if (board.p_endgame && old_IMaxTime >=4000)
//        board.IMaxTime += (board.IMaxTime>>2); //2); //1) ; //+ (board.IMaxTime>>3);  // *24 / 20

//    int inc_IMaxTime = board.IMaxTime + (board.IMaxTime >>2); // + (board.IMaxTime>>3);
//    int dec_IMaxTime = (board.IMaxTime >>1) + (board.IMaxTime >>3);  // - (board.IMaxTime >>2) - (board.IMaxTime>>3);

    //int TimeSpan;

    //search_stack_t *ss = sstack;
    //HistRecord *hisptr;
    //int from, dest, piecefrom, piecedest;
//lazy smp - init from i=2, leave i=0 in lazy smp loop after setting valuebyiteration[0] and easymove    
    for (int i=2;i<board.size;++i)
    {
//        	root_nodes[i] = 0;
        	board.movetab[i].tabval = -BIGVAL;
    }

//-------------------- 20230330------------------


	        Board* spboard;     
	        
//	         spboard = &board; 
	               
          //Board spboardref[7];  
					//spboardref = board;
					
//					A myClass02( tempClass ); //copy constructor
           //Board spboardref(board);  // copy board to spboardref

					
					//spboard = &spboardref; 
/* //20230121
if (board.IMaxTime < 1000)
{
	IMaxDepth = 8;	
	NCORE = 1; //do not start smp if short in time
}
else
*/
  
	IMaxDepth = 64;
std::function<int()> bind_search[15]; //20230511 max 15 threads excl mainthread
std::future<int> f1[15];	
//NCORE = 2;  // lazy smp --- testing stockfish thd skipping pattern
//#ifdef PRINTINFO
	printf("Info num of threads = %d\n", NCORE1);
  fflush(stdout);  
//#endif
Board spboardref[15]; //20230511 max 15 threads excl mainthread	// use assignment operator


//tmr.start();

//***************************************************
for (int k=0; k<NCORE1; k++)  //0-6 (7 threads)
{ 
	spboardref[k] = board;         // use assignment operator 	
	spboard = &spboardref[k]; 
#ifdef TMR	
	tmr.start();	
#endif	
	
#ifdef THREADPO
//  f1[k] = thread_pool.submit(std::launch::async, bind_search[k]);
//20230123 f1[k] = thread_pool.submit(bind_search[k]);
//20230320 try submit bind  thread_pool.push_task(&Engine::Lazy_smp_ID_loop, this, k+1, spboard);  //20230124 
	bind_search[k] = std::bind( &Engine:: Lazy_smp_ID_loop, this,	k+1, spboard ); //thd_idx = 1-7
	f1[k] = thread_pool.submit(bind_search[k]);	
//2023f4 fallback to push_task  f1[k] = thread_pool.submit(&Engine::Lazy_smp_ID_loop, this, k+1, spboard);  //2023f4 

#else
	bind_search[k] = std::bind( &Engine:: Lazy_smp_ID_loop, this,	k+1, spboard ); //thd_idx = 1-7
  f1[k] = std::async(std::launch::async, bind_search[k]);
  //f1[k] = std::async(std::launch::async, &Engine::Lazy_smp_ID_loop, this, k+1, spboard);  //20230124
#endif
} //end for k=0 to 6 (thread 1-7)

//---------------------------------------------------------------------------

  //Iterative deepening at searchroot
  
spboard = &board; //main thread 0 points to &board

#ifdef TMR
tmr.start();	
#endif

//2023f4 start mainthread thd-id = 0 after thread_pool.push_task or f1[k] = std::async(std::launch::async
board.m_bestmove = Lazy_smp_ID_loop(0, spboard);  

//2023f4 thd_bestmove[0] = board.m_bestmove;  
//2023f4 thd_bestscore[0] = board.bestscore;       //2023f4

int completedDepth = board.root_depth;
//2023f4 debug loop thd_completedDepth[0] = board.root_depth; //2023f4
//board.root_depth = -1;  // signal end of mainthread to helper threads 1-7
board.m_timeout = 1; // signal end of mainthread to helper threads 1-7
#ifdef TMR
	tmr.stop();
  printf("info thd %d elapsed time : %d ms.\n", 0, tmr.ms());
#endif

//20230320 try submit bind #ifdef THREADPO
//	thread_pool.wait_for_tasks();  //2023f4 use f1[k].get() loop bug fallback to push_task()  //20230124 push_task()
//	board.m_bestmove = spboardref[0].m_bestmove;
//#else
  //for (int k = 0; k < NCORE - 1; k++)
  for (int k = 0; k < NCORE1; k++)     //20230325
	{ 
  	thd_bestmove[k+1] = f1[k].get();   //20230325
  	//f1[k].join();
// 2023f4 ref sf15.1 instead of below  
    // sf: Check if there are threads with a better score than main thread
    // for (Thread* th : Threads)
    //      if (   th->completedDepth > bestThread->completedDepth
    //          && th->rootMoves[0].score > bestThread->rootMoves[0].score)
    //          bestThread = th;
//20230322 ---------   
    if (spboardref[k].root_depth > completedDepth
     && spboardref[k].movetab[0].tabval > board.movetab[0].tabval )
    {
    	completedDepth = spboardref[k].root_depth;
    	board.movetab[0].tabval = spboardref[k].movetab[0].tabval;
    	board.m_bestmove = spboardref[k].m_bestmove;
    }	
//20230322 --- end ---

	} 
//20230320 #endif

//2023f4 sf15.1 after get all threads, vote for the best thread based on bestscore, completedDepth to retrun bestmove
/*
Thread* ThreadPool::get_best_thread() const {

    Thread* bestThread = front();
    std::map<Move, int64_t> votes;
    Value minScore = VALUE_NONE;

    // Find minimum score of all threads
    for (Thread* th: *this)
        minScore = std::min(minScore, th->rootMoves[0].score);

    // Vote according to score and depth, and select the best thread
    auto thread_value = [minScore](Thread* th) {
            return (th->rootMoves[0].score - minScore + 14) * int(th->completedDepth);
        };

    for (Thread* th : *this)
        votes[th->rootMoves[0].pv[0]] += thread_value(th);

    for (Thread* th : *this)
        if (abs(bestThread->rootMoves[0].score) >= VALUE_TB_WIN_IN_MAX_PLY)
        {
            // Make sure we pick the shortest mate / TB conversion or stave off mate the longest
            if (th->rootMoves[0].score > bestThread->rootMoves[0].score)
                bestThread = th;
        }
        else if (   th->rootMoves[0].score >= VALUE_TB_WIN_IN_MAX_PLY
                 || (   th->rootMoves[0].score > VALUE_TB_LOSS_IN_MAX_PLY
                     && (   votes[th->rootMoves[0].pv[0]] > votes[bestThread->rootMoves[0].pv[0]]
                         || (   votes[th->rootMoves[0].pv[0]] == votes[bestThread->rootMoves[0].pv[0]]
                             && thread_value(th) > thread_value(bestThread)))))
            bestThread = th;

    return bestThread;
}
*/

// 20230408 suppress voting
//20230322 ---------  
		int minScore = thd_bestscore[0];
    // Find minimum score of all threads
    for (int t=1; t<=NCORE1; ++t) 
    { //if ((thd_bestmove[t] !=0) && (thd_completedDepth[t] !=0) && (thd_bestscore[t]!= -INF) )
    	if ( (thd_bestscore[t]!= -INF) )
        minScore = std::min(minScore, thd_bestscore[t]);
		}
    int best_thread_value = (thd_bestscore[0] - minScore ) * thd_completedDepth[0];  //minScore + 14)
    
    int best_thd_idx = 0;
    // Vote according to score and depth, and select the best thread which have a bestmove
    for (int t=1; t<=NCORE1; ++t) 
    { 
    	if ((thd_bestmove[t] != thd_bestmove[0]) && (thd_bestmove[t] !=0) )
    	{
    		int thread_value = (thd_bestscore[t] - minScore ) * thd_completedDepth[t];  //minScore + 14)
    		if (thread_value > best_thread_value)
    		{ 
    			best_thread_value = thread_value;
    			best_thd_idx = t; 
    		}
    	}
    }    

#ifdef PRINTINFO
// 2023f4 debug loop
		if (best_thd_idx !=0) //20230321 return voting bestmove
		{
			for (int t=0; t<=NCORE1; ++t)  //2023f4 debug lazy smp
			{
				fprintf(infoout, "thd_id: %d thd_bestmove=%d, thd_bestscore=%d, thd_completedDepth=%d\n",
			                   t, thd_bestmove[t], thd_bestscore[t], thd_completedDepth[t]);
				fflush(infoout);
			}
		
			fprintf(infoout, "*** best_thd_idx=%d, bestmove=%d, score=%d, completedDepth=%d, best_thread_value=%d\n",
					best_thd_idx, thd_bestmove[best_thd_idx], thd_bestscore[best_thd_idx],thd_completedDepth[best_thd_idx], best_thread_value); 
			fflush(infoout);
			board.m_bestmove = thd_bestmove[best_thd_idx]; //20230321
		}
		//fclose(infoout);
#endif
//20230322 --- end ---
//20230408 suppress voting end
#ifdef DEBUG
    //searchdepth=depth - 1;
    PrintLog("Search.txt");
#endif	

   	return board.m_bestmove;
   
   
 } //end of searchroot  
//----------------------- 20230408 starts ---------------------------------------------------------------
constexpr int SkipSize[]  = { 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
constexpr int SkipPhase[] = { 0, 1, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7 };
/* 20230406 fix thd 7 bug
>>> entry_thd 0 ended. skip pattern: s_s_s_s_s_s_s_s_s_s_s_s_s_s_s_s_ compdepth: 32 vlLast: 0 thd_bestmv: 51883
>>> entry_thd 1 ended. skip pattern: _s_s_s_s_s_s_s_s_s_s_s_s_s_s_s_s compdepth: 31 vlLast: 0 thd_bestmv: 51883
>>> entry_thd 2 ended. skip pattern: _ss__ss__ss__ss__ss__ss__ss__ss_ compdepth: 32 vlLast: 0 thd_bestmv: 51883
>>> entry_thd 3 ended. skip pattern: ss__ss__ss__ss__ss__ss__ss__ss__ compdepth: 32 vlLast: 0 thd_bestmv: 51883
>>> entry_thd 4 ended. skip pattern: s__ss__ss__ss__ss__ss__ss__ss__s compdepth: 31 vlLast: 0 thd_bestmv: 51883
>>> entry_thd 5 ended. skip pattern: __ss__ss__ss__ss__ss__ss__ss__ss compdepth: 30 vlLast: 0 thd_bestmv: 51883
>>> (mainthrd 6 ended. skip pattern: ________________________________ compdepth: 8 vlLast: 7 thd_bestmv: 51883
>>> entry_thd 7 ended. skip pattern: __sss___sss___sss___sss___sss___ compdepth: 32 vlLast: 0 thd_bestmv: 51883
*/
/* 20230420
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
std::mutex mtx;
std::condition_variable cv;
//bool ready = false;
//20230410 std::atomic<bool> ready{false}; // atomic flag for the condition variable
static int static_counter = 0; //20230410 -1; //0; // static counter variable outside the function  //20230408 
std::mutex counter_mutex; // mutex to synchronize access to the counter variable      //20230408  
*/
//----------------------- 20230408 ends -----------
int Engine::Lazy_smp_ID_loop(uint32_t idx, Board* spboard)
{
	  int entry_thread_id = idx; //20230420
	  // copy idx to board.thd_id for tracing //20230309 per thread killers
	  spboard->thd_id = idx;
	  
	  // 20230321 init thd_bestmove etc.
	  thd_bestmove[idx] = 0;
		thd_bestscore[idx] = -INF;
		thd_completedDepth[idx] = 0;      
    
    //int m_depth; // move to board.h for smp
    int m_depth, alpha, beta, val, best, newdepth;
    MoveStruct tempmove;
    char charmove[5];
    unsigned long long start_nodes;
    
    int old_IMaxTime = spboard->IMaxTime;    
    int lastbest = INF;
    

#ifdef EASYMOVE
    unsigned long long root_nodes[111];
if (idx==0)
{	       
    for (int i=0;i<spboard->size;++i)
    {
        	root_nodes[i] = 0;        	
    }
}    
#endif    
   	        

    int root_pv[256]; //smp keep in board.h, now local in thd 
    memset(root_pv, 0, sizeof(root_pv));

#ifdef ASPWIN
int ASP_WINDOW = 16; //8; //4; //0; //=16; //16; //32; //30; //16 //30 //30 //30 //40 //48 //24 //35 //70 //65 //40 //70 //20 //30 //40 //30 //40
//int ValueByIteration[MAXDEPTH];
//    ValueByIteration[0] = spboard->movetab[0].tabval;
int asp_save_ab;
#endif  

#ifdef EASYMOVE
    int EasyMove = 0;
if (idx==0)
{	        
    // Is one move significantly better than others after initial scoring ?
    //if ( rml.get_move_score(0) > rml.get_move_score(1) + EasyMoveMargin)
    if ( spboard->size==1 || spboard->movetab[0].tabval > spboard->movetab[1].tabval + 96)	//EasyMoveMargin 96 = FUTPAWN*4 //0x200 sf1.8
        EasyMove = spboard->m_bestmove; //rml.get_move(0);
}        
#endif

//leave i=0 in lazy smp loop after setting valuebyiteration[0] and easymove 
spboard->movetab[0].tabval = -BIGVAL;
spboard->movetab[1].tabval = -BIGVAL;
#ifdef UNCHANGE  
        int nUnchanged = 0;
#endif    
    //Iterative deepening at searchroot
    //for(depth=start_depth; depth<IMaxDepth; ++depth) //depth=1 has been qsearch??

//lazy smp - limit to IMAXDepth to 3 if IMaxTime < 10 
//    if (board.IMaxTime < 10) IMaxDepth = 3;
//    printf("     **thd %d IMaxDepth=%d\n", idx, IMaxDepth);
//    fflush(stdout);	 


//----------------------- 20230408 starts -----------
	char skipchar[33] = "                                ";
  skipchar[32] = '\0';
/* 20230420  
	thread_local static int entry_thread_id; // define thread-local variable inside function
  {      std::lock_guard<std::mutex> lock(counter_mutex); // lock the mutex to access the counter variable
        //20230410 static_counter++; // increment the counter and assign the value to the entry_thread_id variable
        entry_thread_id = static_counter++; //20230410 =0 then ++  %(NCORE1+1); 
  } //end mutex lock  
*/  
  if (idx == 0) 
  {
/* 202304010
  		{std::unique_lock<std::mutex> lck(mtx);
  		ready = true;
  		cv.notify_all();
  	  } //end mutex lock  
*/
  	#ifdef SYNCOUT
  		sync_out.println(">>> entry_thd ", entry_thread_id, " (mainthread ", idx, ") cv.notify-all and starts ...");
		#endif
	}	
	else 
	{
/* 20230410
		{std::unique_lock<std::mutex> lck(mtx);
		 #ifdef PRINTINFO
		 sync_out.println(">>> entry_thd ", entry_thread_id, " (thd ", idx, ") cv.wait ...");
		 #endif
  	 while (!ready) cv.wait(lck);
  	} //end mutex lock  
*/		
  	//if (idx <= 4 )  //20230402 delay thd 1-3
  		//std::this_thread::sleep_for(std::chrono::milliseconds(entry_thread_id)); //20230404 wait 1ms for the mainthread to start
  		//std::this_thread::sleep_for(std::chrono::microseconds(1)); //20230404 wait 1micros for the mainthread to start
  		//std::this_thread::sleep_for(std::chrono::nanoseconds(1)); //20230404 wait 1ns for the mainthread to start
  	#ifdef SYNCOUT
  			sync_out.println(">>> entry_thd ", entry_thread_id, " (thd ", idx, ") starts ...");
  	#endif
  	#ifdef PRINTINFO
			//fprintf(infoout, "thd %d, entry_thd %d starts ...\n", idx, entry_thread_id);
			fprintf(infoout, "entry_thd %d starts ...\n", entry_thread_id);
  		fflush(infoout);
		#endif  
		
		
  }    	
//----------------------- 20230408 ends ---------------------------------------------------------------    

/* 
//20230110 - stockfish-7 lazy smp (search.cpp)
// Iterative deepening loop until requested to stop or target depth reached
  while (++rootDepth < DEPTH_MAX && !Signals.stop && (!Limits.depth || rootDepth <= Limits.depth))
  {
      // Set up the new depth for the helper threads skipping in average each
      // 2nd ply (using a half density map similar to a Hadamard matrix).
      if (!mainThread)
      {
          int d = rootDepth + rootPos.game_ply();

          if (idx <= 6 || idx > 24)
          {
              if (((d + idx) >> (msb(idx + 1) - 1)) % 2)
                  continue;
          }
          else
          {
              // Table of values of 6 bits with 3 of them set
              static const int HalfDensityMap[] = {
                0x07, 0x0b, 0x0d, 0x0e, 0x13, 0x16, 0x19, 0x1a, 0x1c,
                0x23, 0x25, 0x26, 0x29, 0x2c, 0x31, 0x32, 0x34, 0x38
              };

              if ((HalfDensityMap[idx - 7] >> (d % 6)) & 1)
                  continue;
          }
      }

//20230112 - stockfish-10 lazy smp (search.cpp)
  // Sizes and phases of the skip-blocks, used for distributing search depths across the threads
  constexpr int SkipSize[]  = { 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
  constexpr int SkipPhase[] = { 0, 1, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7 };
// Iterative deepening loop until requested to stop or the target depth is reached
  while (   (rootDepth += ONE_PLY) < DEPTH_MAX
         && !Threads.stop
         && !(Limits.depth && mainThread && rootDepth / ONE_PLY > Limits.depth))
  {
      // Distribute search depths across the helper threads
      if (idx > 0)
      {
          int i = (idx - 1) % 20;
          if (((rootDepth / ONE_PLY + SkipPhase[i]) / SkipSize[i]) % 2)
              continue;  // Retry with an incremented rootDepth
      }      
*/ 
          	
      // 20230112 - ref: sf 10 search.cpp line 64
			// Sizes and phases of the skip-blocks, used for distributing search depths across the threads
			//constexpr int SkipSize[]  = { 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
			//constexpr int SkipPhase[] = { 0, 1, 0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7 };
			// 20230112 - ref: sf 10 search.cpp line 353
			// Iterative deepening loop until requested to stop or the target depth is reached
			// while (   (rootDepth += ONE_PLY) < DEPTH_MAX
      //   && !Threads.stop
      //   && !(Limits.depth && mainThread && rootDepth / ONE_PLY > Limits.depth))
      //{
 for (m_depth=1; m_depth<IMaxDepth; ++m_depth) //depth=1 has been qsearch??
 {      
  		// Distribute search depths across the helper threads
      if (idx !=0)  // not mainThread
      {      	  
					//20230408 int i = (idx - 1) % 20;
       		//20230408 if (((m_depth + SkipPhase[i]) / SkipSize[i]) % 2)
          //20230408  		continue;  // Skip this depth and move on to the next
           	
          //20230408 ---- starts -----
					int ph;
					//20230420 if (entry_thread_id == idx) //2023040 if thd starts after mainthread, ph is as original
						ph = (entry_thread_id - 1) % 20;
					//20230420 else ph = (entry_thread_id ) % 20;  //20230410 fix skippattern at entry_thd 0
						//20230410 ph = (entry_thread_id %(NCORE1+1) ) % 20;  //20230405 fix skippattern at entry_thd 0
					
       		if (((m_depth + SkipPhase[ph]) / SkipSize[ph]) % 2)   //20230325 i equ m_depth
       		{   
						#ifdef SYNCOUT
						skipchar[m_depth-1] = 's';  //20230401 s=skip this depth
						#endif        			
           	continue;  // Skip this depth and move on to the next
          }
          else 
          { 
          	#ifdef SYNCOUT
          	skipchar[m_depth-1] = '_'; 
          	#endif
          }	 	
           		
		    /*
					//int d = rootDepth + rootPos.game_ply();
          // Convert from fullmove starting from 1 to ply starting from 0,
          // handle also common incorrect FEN with fullmove = 0.
          //gamePly = std::max(2 * (gamePly - 1), 0) + (sideToMove == BLACK);
          
					int d = m_depth; // + board.ply; 
          if (idx <= 6 || idx > 24)  
          {   //                             depth:  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
          	  // stockfish skipping pattern: thd 1:  s s     s s     s s     s s     s s
          	  //                             thd 2:  s     s s     s s     s s     s s
          	  //                             thd 3:  s s s s         s s s s
          	  //                             thd 4:  s s s         s s s s
          	  //                             thd 5:  s s         s s s s    
          	  //                             thd 6:  s
          	  //                             thd 7:  s s   
          	  //int idx2 = idx + 2 * (idx % 2) - 1; // idx=1 => idx2=2, idx=2 => idx2=1
              //if (((d + idx2) >> (msb32(idx2 + 1) - 1)) % 2)
              
              if (((d + idx) >> (msb32(idx + 1) - 1)) % 2)  // original sf depth policy	
              	
              	
              //if (d != int(3*log(1+idx)))
              	
              //if ((d==1) || ((d % 2) == (idx % 2)) )  //skipping d=1,3,5,..for thd=1(odd), skipping d=1,2,4,6 for thd=2(even)     
              //if (d <= idx) // skipping depth <= thread#, in effect starting from root_depth +1, or +2          
              { 
              		
              	
#ifdef THDINFO              	  
              	  for (int s=1; s<=idx; s++) printf("     ");              	
                  printf("helper thd %d depth %d ...skipping, mainthd completed depth = %d\n", idx, d, board.root_depth);  
                  fflush(stdout);    
#endif              
                            
                  continue;
                  
              }    
          }
          else
          {
              // Table of values of 6 bits with 3 of them set
              static const int HalfDensityMap[] = {
                0x07, 0x0b, 0x0d, 0x0e, 0x13, 0x16, 0x19, 0x1a, 0x1c,
                0x23, 0x25, 0x26, 0x29, 0x2c, 0x31, 0x32, 0x34, 0x38
              };

              if ((HalfDensityMap[idx - 7] >> (d % 6)) & 1)
              {	
#ifdef THDINFO            
              	  for (int s=1; s<=idx; s++) printf("     "); 
              	  printf("helper thd %d depth %d ...skipping\n", idx, d); 
              	  fflush(stdout);  
#endif              	    
                  
                  continue;
              }    
          } 
          */       
        // if helper thd m_depth overrun root_depth by > +2, decrease m_depth to avoid timeout  
        if ((m_depth > 7) && (m_depth > board.root_depth + 2))
        { int thd_m_depth =  m_depth;
        	m_depth--;
        	if (m_depth == spboard->root_depth)
        		m_depth--;
#ifdef THDINFO            
         for (int s=1; s<=idx; s++) printf("     "); 
         printf("helper thd %d depth %d overrun root_depth %d, reducing to depth %d instead\n", idx, thd_m_depth, board.root_depth, m_depth); 
         fflush(stdout);  
#endif         		
        }		  
      }
//20230420   else skipchar[m_depth-1] = '_'; 
          	
        /* 根結點搜索例程，和完全搜索的區別有以下幾點：
         *
         * 1. 省略無害裁剪(也不獲取置換表著法)﹔
         * 2. 不使用空著裁剪﹔
         * 3. 選擇性延伸隻使用將軍延伸﹔
         * 4. 過濾掉禁止著法﹔
         * 5. 搜索到最佳著法時要做很多處理(包括記錄主要變例、結點排序等)﹔
         * 6. 不更新歷史表和殺手著法表。
         */
          
#ifdef ASPWIN
// use aspiration window
  if (m_depth < 7 //5 //stcokfish-7 use asspwin for depth >=5, i.e 6 //7  
//  	|| (idx !=0)  //20160719 test noaspwin for helper
  	)
	{
		alpha = -INF;
		beta = INF;
	}
	else
	{
		//if ((lastbest) == -DRAWVALUE)
		//{
		//	alpha = lastbest - 1;
		//	beta = lastbest + 1;
		//}
		//else
		{
			// Calculate dynamic aspiration window based on previous iterations
        //if (abs(ValueByIteration[m_depth - 1]) < WIN_VALUE)
        if (abs(lastbest) < WIN_VALUE)
        {
            
            //int prevDelta1 = lastbest                      - ValueByIteration[m_depth - 2];
            //int prevDelta2 = ValueByIteration[m_depth - 2] - ValueByIteration[m_depth - 3];
            //ASP_WINDOW = std::max((abs(prevDelta1) + abs(prevDelta2))/2, 16); //m_depth); //1889l 10.5-20.5-MAX,16
            ASP_WINDOW = 16; //4;           
            alpha = std::max(lastbest - ASP_WINDOW, -INF);
            beta  = std::min(lastbest + ASP_WINDOW,  INF);
//            printf("     **m_depth-2=%d, ValueByIteration[%d]=%d\n", m_depth-2, m_depth-2, ValueByIteration[m_depth-2]);
//        fflush(stdout); 
//                printf("     **m_depth-3=%d, ValueByIteration[%d]=%d\n", m_depth-3, m_depth-3, ValueByIteration[m_depth-3]);
//        fflush(stdout); 
//                printf("     **m_depth=%d, ASP_WINDOW=%d\n", m_depth, ASP_WINDOW);
//        fflush(stdout);             
            
          
            
        }
        else
        {
        	// no aspiration window if >= WIN_VALUE
        	alpha=-INF;
        	beta=INF;
        }

			//alpha = lastbest - ASP_WINDOW;
			//beta  = lastbest + ASP_WINDOW;
		}
	}
#else
// no aspiration window
        alpha=-INF;
        beta=INF;

#endif
        spboard->ply=0;
//		fpv=0;
// 20160804 - replaced by sf root move ordering 
//        pv_i = 0; //-1;

//        alpha=-INF;
//        beta=INF;
        best=-INF;
        int old_alpha = alpha;

//		move_count=0;
//		bestscore=-INF;

//		max_root_nodes = 0;
        for (int i=0;i<spboard->size;++i)
        {
// 2892v - check if main thread run ends, abort helper threads
if (idx !=0)
{	
  //if (board.root_depth == -1)  // signal mainthread ended
  if (board.m_timeout==1)  // signal mainthread ended
  {
#ifdef PRINTINFO	
	printf("     **thd %d aborted due to mainthread ended, board.root_depth=%d\n", idx, board.root_depth);
	fflush(stdout); 
#endif
  spboard->m_timeout = 1; 	
  break;  //break i  //20230408 fix loop bug???
  //20230408 loop bug??? return spboard->m_bestmove;
  }	

// 20230408 suppress chk m_depth <= board.root_depth
  if (m_depth <= board.root_depth)
  {
#ifdef PRINTINFO	
	printf("     **thd %d abort depth %d due to main thread depth=%d completed\n", idx, m_depth, board.root_depth);
	fflush(stdout); 
#endif		
  continue;  //20230408 continue instead of break i
  }
//
}



            start_nodes=spboard->m_nodes;
            //if (IsBanMove(table[i].move))
            //	continue;

            //if (makemove(movetab[i].table) < 0)
            //{	continue;
            //}
            tempmove.move = spboard->movetab[i].table.move;
            //makemoverootNochk(tempmove);
            int capture=spboard->makemove(tempmove, 0); //, 0);
    	
       	
       	
            //move_count++;
            newdepth=m_depth-1;
            if (spboard->incheck)
            {
            	newdepth++;
            	//singleext at root
            	//if (size==1)
            	//	newdepth++;
            }
// LMR at root

int reduced=0;
#ifdef ROOTLMR
		if (m_depth>= HistoryDepth && !spboard->incheck && newdepth < m_depth && i >= 5 //8 //5  //gen_count >= 6 //3 //HistoryMoveNb
			&& (capture==0) // || tempmove.tabval < 0) //non capture or bad capture
			&& spboard->m_hisrecord[spboard->m_hisindex-1].htab.Chk==0
			 )
				{
#ifdef PVRED
					reduced=pvred(m_depth, i+1);
					newdepth -= reduced;
#else
	      //  if (m_depth==3 && i < 15)
				//;
        //    else
        if (spboard->p_endgame || ((spboard->EvalSide<1>() < THREAT_MARGIN)
      	&&  (spboard->EvalSide<0>() < THREAT_MARGIN))
					 )
          {
	        	reduced=1;
	        	newdepth--;
	        }
#endif
					//{
						//newdepth -= reduced;
					//}
				}


#endif
            if (newdepth <=0)
                val = -quiesCheckPV(*spboard, -INF, INF, newdepth, qcheck_depth); //newdepth, -2);
            else
           {
            if (best == -INF) // || m_depth==1) //first move or searchShort
            {    
            	int faillow_cnt = 0;
            	int failhigh_cnt = 0;  
            	while (true) //sf-7
              {          
                val = -searchPV(*spboard, -beta, -alpha, newdepth); //, NULL_YES);
                if (spboard->m_timeout)
                {
#ifdef THDINFO            
              	  for (int s=1; s<=idx; s++) printf("     "); 
              	  printf("thd %d depth %d timeout at fail low/high\n", idx, m_depth); 
              	  fflush(stdout);  
#endif                 	
                	break;
                }		
                // 1888v 1888y                                
                if (val <= alpha)
                {
                	//sf-7: In case of failing low/high increase aspiration window and re-search              
                	asp_save_ab = beta;
                	beta = (alpha + beta) / 2;              //sf-7
                  alpha = std::max(val - ASP_WINDOW, -INF);   //sf-7
                  faillow_cnt++; 
                  	
                  //val = -searchPV(*spboard, -beta, -alpha, newdepth); //sf-7
                  	
                	//val = -searchPV(*spboard, -beta, INF, newdepth); //, NULL_YES);
                	//old_alpha = alpha = -INF;      
#ifdef THDINFO                	          	               	
                	printf("     **thd %d fail low depth=%d, ASP_WINDOW=%d, re-search root with -beta=%d, -alpha=%d\n", idx, m_depth, ASP_WINDOW, -beta, -alpha);
#endif
                  if (alpha == -INF || faillow_cnt > 1)
                  {
                  	old_alpha = alpha = -INF; 
                  	beta = asp_save_ab;
                  	val = -searchPV(*spboard, -beta, INF, newdepth);                   	
                  	break;
                  }	
                  
                  		
                	//if (val >= beta)
                	//{
                	//printf("info ASP_WINDOW=%d, re-search root with -beta,INF and val>=beta\n", ASP_WINDOW);
                	//beta = INF;
                	//val = -searchPV(sstack, -INF, INF, newdepth);
                	//}
                	
                	
                }
                else if (val >= beta)
                {
                	asp_save_ab = alpha;
                	alpha = (alpha + beta) / 2;            //sf-7 
                  beta = std::min(val + ASP_WINDOW, INF);    //sf-7
                  failhigh_cnt++;	
                  //val = -searchPV(*spboard, -beta, -alpha, newdepth); //sf-7	
#ifdef THDINFO                  
                  printf("     **thd %d fail high depth=%d, ASP_WINDOW=%d, re-search root with -beta=%d, -alpha=%d\n", idx, m_depth, ASP_WINDOW, -beta, -alpha);
#endif                	
                	if (beta == INF || failhigh_cnt > 1)
                	{
						        alpha = asp_save_ab;
                		beta = INF;
                		val = -searchPV(*spboard, -INF, -alpha, newdepth); //, NULL_YES);                	  
                	  break;
                	}		
                	
                	//val = -searchPV(*spboard, -INF, -alpha, newdepth); //, NULL_YES);
                	//beta = INF;
                	//printf("info depth=%d, ASP_WINDOWS=%d, re-search root with -INF,-alpha\n", m_depth, ASP_WINDOW);
                	//if (val <= alpha)
                  //{
                  	//printf("info ASP_WINDOWS=%d, re-search root with -INF,-alpha and val<=alpha\n", ASP_WINDOW);
                	//old_alpha = alpha = -INF;
                	//val = -searchPV(sstack, -INF, INF, newdepth);
                  //}
                  
                } 
                else
                	break; //while true
                	
                ASP_WINDOW += ASP_WINDOW / 4 + 5;  //sf-7
                
                assert(alpha >= -INF && beta <= INF);
              } //while true  	               
            }
            else
            {
                //val = -AlphaBeta(sstack, -best - 1, -best,newdepth, NodeCut, NULL_YES);
                //if (val > best)
                //    val = -AlphaBeta(sstack, -INF, -best,newdepth, NodePV, NULL_YES);
                //val = -AlphaBeta(sstack, -alpha-1, -alpha, newdepth, NULL_YES);

//jmp_ok = true;
//if (!setjmp(setjmpbuf))
{
                val = -AlphaBeta(*spboard, -alpha, newdepth, NULL_YES);
}
//jmp_ok = false;

                        // history-pruning re-search
if (spboard->m_timeout==0)
{
#ifdef ROOTLMR
        			if (reduced && val >alpha) //>=beta)	//was >alpha
        			{
            		newdepth+=reduced;
                //val = -AlphaBeta(sstack, -beta,-alpha, newdepth, NodePV, NULL_YES);
                val = -AlphaBeta(*spboard, -alpha, newdepth, NULL_YES);   //avoid double re-search
        			}
#endif
        if (spboard->m_timeout==0)
        {
                if (val > alpha)
                {
                    // If we are above alpha then re-search at same depth but as PV
                        // to get a correct score or eventually a fail high above beta.
                    //val = -AlphaBeta(sstack, -INF, -alpha, newdepth, NodePV, NULL_YES);
                    val = -searchPV(*spboard, -INF, -alpha, newdepth); //, NULL_YES);
                    if (val >= beta)
                    {
                    	beta = INF;
                    //	//1888v
                    //	val = -searchPV(sstack, -INF, INF, newdepth);
                    }
                }
        }
} //endif board.m_timeout

          }
        }
        
        //if (nHashExclusive == 1)
        //RecordHashExcl(*spboard, 0, 0);  //clear exclusive  ABDADA    
        
            spboard->unmakemove();
                        
            if (spboard->m_timeout) //stop
            	break;
//stockfish way of root move ordering
/*            	
if (RootNode)
      {
          RootMove& rm = *std::find(thisThread->rootMoves.begin(),
                                    thisThread->rootMoves.end(), move);

          // PV move or new best move ?
          if (moveCount == 1 || value > alpha)
          {
              rm.score = value;
              rm.pv.resize(1);

              assert((ss+1)->pv);

              for (Move* m = (ss+1)->pv; *m != MOVE_NONE; ++m)
                  rm.pv.push_back(*m);

              // We record how often the best move has been changed in each
              // iteration. This information is used for time management: When
              // the best move changes frequently, we allocate some more time.
              if (moveCount > 1 && thisThread == Threads.main())
                  ++static_cast<MainThread*>(thisThread)->bestMoveChanges;
          }
          else
              // All other moves but the PV are set to the lowest value: this is
              // not a problem when sorting because the sort is stable and the
              // move position in the list is preserved - just the PV is pushed up.
              rm.score = -VALUE_INFINITE;
      }
*/
//20160804 - replaced by the above stockfish root move ordering
 // PV move or new best move ?
          if (i == 0 || val > alpha)
          {
              spboard->movetab[i].tabval = val; 
          }
          else     
            	spboard->movetab[i].tabval = -INF; 
            	


#ifdef EASYMOVE
if (idx==0 && 
 (i==0) )
{	
      root_nodes[i] += (spboard->m_nodes - start_nodes);
}      
#endif


            if (val>best)
            {
#ifdef UNCHANGE            	
                // 6. 如果搜索到第一著法，那麼"未改變最佳著法"的計數器加1，否則清零
                nUnchanged = (best == -INF ? nUnchanged + 1 : 0);
#endif
                best=val;
                if (val > alpha)
                {
                	//if (m_depth > 1)  //search_type not searchShort
                	alpha = val;
                	if (val >= beta)
                	{
					          //ASSERT( false );
					          break;
				          }
				    		}

//				bestscore=val;
// 20160804 - replaced by sf root move ordering
                //spboard->movetab[i].tabval = val; //replaced by sf root move ordering
                
                spboard->m_bestmove=tempmove.move; //movetab[i].table.move;
                //update_pv(sstack, 0, board.m_bestmove);
                //if (best != -INF)
                //	tabval[i] = BIGVAL;
                // Drop the easy move if differs from the new best move
        				//if (ss->pv[0] != EasyMove)
#ifdef EASYMOVE   
if (idx==0)
{	     				
        				if (spboard->m_bestmove != EasyMove)
            			EasyMove = 0;
}            			
#endif
// 20160804 - replaced by sf root move ordering
//                pv_i = i;
                //	fpv=1;
                //if (m_depth > 1) //searchshort
                RecordHash(HASH_PV, m_depth, best, spboard->m_bestmove, *spboard); //, 0);
//                recordhashpvcnt++;
//print hash move for debug lazy smp    
           
//    char c_bestmove[5];  //, c_rootpv[5];
//  	MoveStruct hash_tempmove;  	
//  	hash_tempmove.move = spboard->m_bestmove;
//  	com2char(c_bestmove, hash_tempmove.from, hash_tempmove.dest );  	
//  	printf("     **Recordhash Pv at depth=%d best move=%s\n", m_depth, c_bestmove);
//  	fflush(stdout);

    }
//long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);
/* 
//lazy smp disable time management
    				if (board.IMaxTime - (GetTime()-board.m_startime) <= 200)
            {  
            	board.m_timeout = 1;
            	printf("     **next root move timeout\n");
            	fflush(stdout);
            	break; 
            }
*/


        } //next i
              
        if (spboard->m_timeout)
           	break; 
           	
        // sf211 - Write PV back to transposition table in case the relevant entries
            // have been overwritten during the search.
            //for (int i = 0; i < Min(MultiPV, (int)Rml.size()); i++)
            //    Rml[i].insert_pv_in_tt(pos); 
//lazy smp bug - disable putpvline            
           // if (spboard->root_pv[0] != 0)     
          //  if (spboard->root_pv[0] == spboard->m_bestmove) 	      
          //  	PutPvLine(*spboard, spboard->root_pv, m_depth, best);
//lazy smp -- stockfish-7 still write back pv for helper threads

          	
            int hashmove = 0;
  hashmove = ProbeMoveQ(*spboard);
  if (hashmove == 0 || hashmove != spboard->m_bestmove) //tempmove.move)
  {
  	/*
  	char c_hashmove[5], c_bestmove[5]; //, c_rootpv[5];
  	MoveStruct hash_tempmove;  	
  	hash_tempmove.move = spboard->m_bestmove;
  	com2char(c_bestmove, hash_tempmove.from, hash_tempmove.dest );  
  	printf("     **thd %d depth=%d PutPv back hashmove=%s best move=%s\n", m_depth, spboard->thd_id, c_hashmove, c_bestmove);
  	fflush(stdout);
  	*/
  	RecordHash(HASH_PV, m_depth, best, spboard->m_bestmove, *spboard); //tempmove.move);
//  	recordhashpvcnt++;
  }		
 	
//            	
	    //save main and helper thread completed root_depth
      spboard->root_depth = m_depth; 
//      spboard->bestscore = best;  //2023f4      
//if (m_depth>=7) //7) //7)  //6)
//lazy smp - print pvline only for mainthread idx=0
int TimeSpan = (int)(GetTime() - spboard->m_startime); //(clock() - m_startime) ;
if (TimeSpan==0) TimeSpan=1;
int nps = int((double)spboard->m_nodes * 1000 / (double)TimeSpan);	
if (idx != 0)
{	 
#ifdef THDINFO	
  for (int s=1; s<=idx; s++) printf("     "); 
	printf("helper thd %d depth %d score %d time %d nodes %llu nps %d \n", idx, m_depth, best, TimeSpan, spboard->m_nodes, nps);
  fflush(stdout);
#endif  
}
else 
                {
                    //int TimeSpan = (int)(GetTime() - spboard->m_startime); //(clock() - board.m_startime) ;
                    //if (TimeSpan==0) TimeSpan=1;                    
                    //int nps = int((double)spboard->m_nodes * 1000 / (double)TimeSpan);
                    printf("info depth %d score %d time %d nodes %llu nps %d pv", m_depth, best, TimeSpan, spboard->m_nodes, nps);
                    //fflush(stdout);

//int m_pvline[256];
//m_pvline[0] = board.m_bestmove;
//m_pvline[1] = 0;                    
                    //GetPvLine(m_pvline, board.m_bestmove);
                    GetPvLine(*spboard, root_pv, spboard->m_bestmove);
                    ponder_move = 0;
                    respmove_to_ponder = 0;
                    for (int j=0; j<256; ++j)
                    {
                        //tempmove.move = m_pvline[j];
                        tempmove.move = root_pv[j];
                        //tempmove.move = ss->pv[j];                        
                        if (tempmove.move == 0)
                            break;

                        com2char(charmove, tempmove.from, tempmove.dest );
                        printf(" %s", charmove); //fflush(stdout);
                        if (j==0)
                        {
                            int piecedest=spboard->piece[tempmove.dest];
                            if (piecedest) spboard->boardsq[piecedest]=SQ_EMPTY;
                            spboard->boardsq[spboard->piece[tempmove.from]] = tempmove.dest;
                            //memcpy(prev_piece, board.piece, sizeof(prev_piece));
                            if (piecedest) spboard->boardsq[piecedest] = tempmove.dest;
                            spboard->boardsq[spboard->piece[tempmove.from]] = tempmove.from;
                            //prev_ponder_depth=depth;
                        }
                        else if (j==1) ponder_move = tempmove.move;
                        else if (j==2) respmove_to_ponder = tempmove.move;
                    }
                    printf("\n"); fflush(stdout);
                }
//                prev_bestmove[print_depth] = board.m_bestmove;
//                prev_best[print_depth] = best;
#ifdef DEBUG
                searchdepth=m_depth;
#endif
#ifdef UNCHANGE
                // b. 如果最佳著法連續多層沒有變化，那麼適當時限減半
                if (IMaxDepth == 64) //for gotime, not for go depth
                if ( m_depth >=9) //9) //8) //7) //9) //9)
                {

                   if (nUnchanged == 4)
                    {

//                        if (board.IMaxTime != inc_IMaxTime)
                        {
                            spboard->IMaxTime = (spboard->IMaxTime >>1) + (spboard->IMaxTime >>3);  //dec_IMaxTime;
                            printf("     **best move unchanged, decrease board.IMaxTime=%d\n", spboard->IMaxTime);
                            fflush(stdout);
                        }
                    }
                    else if (nUnchanged == 0) // && board.IMaxTime != inc_IMaxTime)
                    {	  printf("     **board.IMaxTime=%d will be reset to %d\n", spboard->IMaxTime, old_IMaxTime);
                    	  fflush(stdout);
                        spboard->IMaxTime = old_IMaxTime;
                    }

                }
#endif                
//                print_depth++;
        // Stop search early if there is only a single legal move,
            // we search up to Iteration 6 anyway to get a proper score.
            /*
            if (m_depth >= 6 && size==1)
                break; //board.m_timeout = 1;
        // Stop search early when the last two iterations returned a mate score
            if (m_depth >= 6 && abs(best) > WIN_VALUE && abs(lastbest) > WIN_VALUE)
            	  break; //board.m_timeout = 1;
            */
            //board.IMaxTime <= 4000 1892g
            //old_IMaxTime <= 1000
            if (spboard->size==1 || (abs(best) > WIN_VALUE && abs(lastbest) > WIN_VALUE))
            	if (m_depth >= 6) //4 6
            	{ //board.m_timeout = 1; //20230321
            	  break; 	  
            	}  
        // Stop search early if one move seems to be much better than the others
        /*    int64_t nodes = TM.nodes_searched();
            if (   Iteration >= 8
                && EasyMove == pv[0]
                && (  (   rml.get_move_cumulative_nodes(0) > (nodes * 85) / 100
                       && current_search_time() > MaxSearchTime / 16)
                    ||(   rml.get_move_cumulative_nodes(0) > (nodes * 98) / 100
                       && current_search_time() > MaxSearchTime / 32)))
                stopSearch = true;
                */
            //int TimeSpan = (int)(GetTime() - board.m_startime); //(clock() - board.m_startime) ;
          
        //if(board.m_timeout) // don't break early, find faster mate // ||bestscore<=-INF+9999||bestscore>=INF-9999)
        //if ( (size==1) || board.m_timeout
        if ( spboard->m_timeout
        	//|| ( (best > WIN_VALUE+128) || (best < -WIN_VALUE-128)
        	//|| ( abs(best) > WIN_VALUE && abs(lastbest) > WIN_VALUE)

        //	|| (old_IMaxTime < 2000 && m_depth >=9)
        //	|| (old_IMaxTime < 1000 && m_depth >=5)

        	)
        {	//searchdepth=depth; // - 1;

            break;
        }
#ifdef EASYMOVE   
if (idx==0)
{	        
            if (m_depth >= 8
            	 && EasyMove == spboard->m_bestmove //ss->pv[0]
            	 && (  (   root_nodes[0] > (spboard->m_nodes * 85) / 100
                       && (int)(GetTime() - spboard->m_startime) > old_IMaxTime / 16)
                    ||(  root_nodes[0] > (spboard->m_nodes * 98) / 100
                       && (int)(GetTime() - spboard->m_startime) > old_IMaxTime / 32)))
            {
                printf("     ** thd %d easymove\n", idx);
                fflush(stdout);
                //board.m_timeout = 1; //20230321
                break; //board.m_timeout = 1;
            }
}            
#endif          
/*
// lazy smp, disable time management
        //check to see if we have enough time left to search for next depth, see scorpio
        //1892g if (m_depth >=11 || (old_IMaxTime < 5000))	//11 //>=13
        if (m_depth >=13 || (old_IMaxTime < 5000))
        {	  int TimeRemain = old_IMaxTime - (int)(GetTime() - spboard->m_startime); //(clock() - board.m_startime) ;
            if ((TimeRemain <=200) || (TimeRemain <= (old_IMaxTime>>2) + (old_IMaxTime>>3)))	//0.75 //0.625
          //if (TimeSpan > old_IMaxTime * 79 / 128) //0.625
            {
#ifdef THDINFO            	
                printf("     thd %d not enough time for next depth. m_depth=%d, TimeRemain=%d, old_IMaxTime=%d\n", idx, m_depth, TimeRemain, old_IMaxTime);
                fflush(stdout);
#endif
                break;
            }
        }
*/        
/*
// 2892q - try different from original 1892q -not successful, disabled now
				// don't start new iteration if most of the time is consumed
    		if ((GetTime() - spboard->m_startime) > old_IMaxTime * 79 / 128)   //0.617 //0.625
{
#ifdef THDINFO            	
                printf("     thd %1 not enough time for next depth. m_depth=%d, old_IMaxTime=%d\n", idx, m_depth, old_IMaxTime);
                fflush(stdout);
#endif	
        	break;
}        	
*/
// 20160804 - replaced by sf root move ordering
/*
        //put this pv to the front
        //if (pv_i >=0)
        {
            pv_tabval = spboard->movetab[pv_i].tabval;
            spboard->movetab[pv_i].tabval = 0x7fff;

        }
*/        

        
        // Bring the best move to the front. It is critical that sorting
              // is done with a stable algorithm because all the values but the
              // first and eventually the new best one are set to -VALUE_INFINITE
              // and we want to keep the same order for all the moves except the
              // new PV that goes to the front. Note that in case of MultiPV
              // search the already searched PV lines are preserved.	
        std::stable_sort(spboard->movetab, spboard->movetab + spboard->size);	
        

//      put back pv_tabval after sort 
// 20160804 - replaced by sf root move ordering
//        spboard->movetab[0].tabval = pv_tabval;	
        	
#ifdef DEBUG
        printf("info Root moves: ");
        for (int i=0; i<size; i++)
        {
            com2char(charmove, movetab[i].table.from, movetab[i].table.dest );
            //fprintf(traceout, " %s", charmove);
            printf("%10s", charmove);
        }
        //fprintf(traceout, "\n");
        printf("\n");
        //fflush(traceout);
        fflush(stdout);
        //fclose(traceout);
        //

        printf("info Root tbval: ");
        for (int i=0; i<size; i++)
        {
            printf("%10d", movetab[i].tabval);
            //printf("%10d", root_nodes[i]);
        }
        //fprintf(traceout, "\n");
        printf("\n");
        //fflush(traceout);
        fflush(stdout);
        //fclose(traceout);
        
        printf("info Root nodes: ");
        for (int i=0; i<size; i++)
        {
            //printf("%5d", movetab[i].tabval);
            printf("%10d", (unsigned int)root_nodes[i]);
        }
        //fprintf(traceout, "\n");
        printf("\n");
        //fflush(traceout);
        fflush(stdout);
        //fclose(traceout);
        //
#endif

        lastbest = best;
//#ifdef ASPWIN
// 	if (idx ==0)  //20160719 test noaspwin for helper
//        ValueByIteration[m_depth] = best;
//        printf("     **m_depth=%d, ValueByIteration[%d]=%d\n", m_depth, m_depth, ValueByIteration[m_depth]);
//        fflush(stdout); 
//#endif
        //lastpv = movetab[0].table.move;
//	if (HCheckStop()) break;

		thd_bestmove[idx] = spboard->m_bestmove;
	thd_bestscore[idx] = best; //spboard->bestscore;
	thd_completedDepth[idx] = m_depth; //spboard->root_depth;

    } // next depth


	
	//if (idx == 0)
  {
  		//board.root_depth = -1;  // signal end of mainthread
  		//board.timeout=1;           //signal end of mainthread
  }
  
  //thd_bestmove[idx] = spboard->m_bestmove;
	//thd_bestscore[idx] = spboard->bestscore;
	//thd_completedDepth[idx] = spboard->root_depth;

//20230408 --- start -----	
#ifdef PRINTINFO 
  fprintf(infoout, "thd %d ended. completed depth %d score %d mv %d, skip pattern: %s\n", idx, thd_completedDepth[idx], lastbest,
  	thd_bestmove[idx] , (char*)skipchar );
  fflush(infoout);
#endif	
	
	
		if (idx == 0)
		{ 
			#ifdef SYNCOUT
			sync_out.println(">>> (mainthrd ", entry_thread_id, " ended. skip pattern: ", (char*)skipchar, 
				" compdepth: ", thd_completedDepth[0], " vlLast: ", lastbest, " thd_bestmv: ",
				MOVE_COORD_C(thd_bestmove[0])); 
				//20230405 !!! ucci bestmove keyword not allowed in stdout
			#endif
//20230410	ready = false; //20230408 reset cv when mainthread ends, for next cycle
//20230420			static_counter = 0; //20230410 reset for next cycle
		}
		else 
		{
			#ifdef SYNCOUT
			sync_out.println(">>> entry_thd ", entry_thread_id, " ended. skip pattern: ", (char*)skipchar, 
				" compdepth: ", thd_completedDepth[idx], " vlLast: ", lastbest, " thd_bestmv: ", 
				MOVE_COORD_C(thd_bestmove[idx]));
			#endif
    }	
//20230408 --- end -----		
	
	return spboard->m_bestmove;
}  // end of Lazy_smp_ID_loop


//------------------------------------------------------------------------------------------------------------------
//#define EVAL_MARGIN1 345 //256
#ifdef __GNUC__
inline
#else  
__forceinline
#endif
int Engine::Evalscore(Board &board) //, int beta) //(int alpha,int beta)
{
    //if (board.IsInCheck(oppside(board.m_side)))
    //	return (board.m_side ? board.ply-INF : INF-ply);

    int val;
/*
    //lazy eval
    val=(board.m_side ? -pointsum : pointsum);


    if (val - EVAL_MARGIN1 >= beta)  {
        return val - EVAL_MARGIN1;
    }
    if (val + EVAL_MARGIN1 <= alpha) {
        return val + EVAL_MARGIN1;
    }
*/
#ifdef EVALHASH
    //val=ProbeEvalHash();
    EvalHash evalhsh;
	uint64 offset = (board.hkey & nEvalHashMask);
    evalhsh = evalhashItems[offset];
    //if ((evalhsh.hkey64 & 0x0000FFFFFFFFFFFF) == (hkey & 0x0000FFFFFFFFFFFF))
        //if ((evalhsh.hkey64 ) == (hkey ))
    //if (evalhsh.hkey32 == ((board.hkey )>>32))  //8byte evalhash 20230308
    //if ( (evalhsh.hkey16_2 <<16)+(evalhsh.hkey16_1) == (hkey >>32 ) )  //6byte evalhash
    //20230308 if (evalhsh.hkey32 == ((board.hkey )>>32))  //6byte evalhash
    //if (evalhsh.hkey32 == ((board.hkey ) & 0x0000FFFFFFFFFFFF))  //20230308 8byte evalhash lower 50bits as lock
    
    //20230312 if (evalhsh.hkey50 == (board.hkey >>14) ) //20230308 upper 50bitkey
    if (evalhsh.hEkey == (uint32_t)board.hkey ) //20230312
    {
        val=evalhsh.value; //20230312  - INF;  //20230308
        //val=evalhsh.value - INF;  //-2048  4byte evalhash
//      		nEvalHits++;
    }
    else
    {	/*lazy=0*/

#endif
        val= board.Eval(); //(alpha,beta); //+ 5; //+ pointsum;
#ifdef EVALHASH
        //if (val >= beta)	//stockfish 1.4
        {	//RecordEvalHash(val);
            //evalhsh.hkey64 = hkey;
            //evalhsh.hkey16_2 = ((hkey )>>32)>>16;
            //evalhsh.hkey16_1 = ((hkey )>>32)&0x00FF;  
            //evalhsh.hkey32 = ((board.hkey )>>32);   //20230308  
               
            //evalhsh.hkey50 = (board.hkey >>14) ;   //20230308  upper 50bit key   
            evalhsh.hEkey = (uint32_t)board.hkey;  //20230312
            evalhsh.value = val; //20230312 + INF;    //20230308
            
            //evalhsh.hkey32 = ((hkey )>>32); 
            //evalhsh.hkey = hkey; //8byte evalhash
            //evalhsh.hkey32 = uint32(hkey ); //4byte evalhash            
            //evalhsh.value = val + INF; //+2048 4byte evalhash
						offset = (board.hkey & nEvalHashMask);
            evalhashItems[offset] = evalhsh;
//  			nEvalRecs++;
        }
    }
#endif
    return val;
}

/*
inline
void Update_HistVal(HistStruct *hisvptr, int depth)
{
#ifdef HISTPRUN
#ifdef HISTHIT
            hisvptr->HistHit++;
#endif
#endif
            hisvptr->HistVal += depth * depth; //HistInc[depth]; //

            if (hisvptr->HistVal >= HistValMax)
            {
				      for (int i=0; i<(BOARD_SIZE-7); i++)
				      //for (int j=0; j<16; j++)
				      for (int j=0; j<10; j++)
							{
								 m_his_table[i][j].HistVal = (m_his_table[i][j].HistVal + 1) / 2;

							}
            }

}
*/
  // update_history(board.thd_id, ) registers a good move that produced a beta-cutoff
  // in history and marks as failures all the other moves of that board.ply.
#ifdef __GNUC__
inline
#else  
__forceinline
#endif
static void update_HistVal(int t, HistStruct *hisvptr, int depth) //20230309 per thd_id
{

    hisvptr->HistVal += depth * depth; //HistInc[depth]; //
    if (hisvptr->HistVal >= HistValMax)
    {
			//for (int i=0; i<(BOARD_SIZE-7); i++)
			//for (int j=0; j<16; j++)
			for (int p=0; p<10; p++)
		  for (int i=0; i<10; i++)
			for (int j=0; j<9; j++)
			{
				//m_his_table[p][i][j].HistVal /= 2; // (m_his_table[p][i][j].HistVal + 1) / 2;
        m_his_table[t][p][(i*16)+j].HistVal /= 2;
			}
    }
}

#ifdef __GNUC__
inline
#else  
__forceinline
#endif
static void update_history(int t, HistStruct *hisvptr, int depth,  //20230309 per thd_id
    MoveTabStruct movesSearched[], int noncap_gen_count) //20230309 per thd_id
{
		//H.success(pos.piece_on(move_from(m)), m, depth);
		hisvptr->HistHit++;
    for (int icnt = 0; icnt < noncap_gen_count - 1; icnt++)
    {
//        assert(m != movesSearched[i]);
        //if (ok_to_history(pos, movesSearched[i]))
        //    H.failure(pos.piece_on(move_from(movesSearched[i])), movesSearched[i]);
        //if (movesSearched[icnt].tabval != 0) //capture == 0 and tabval = piecefrom
        {
        		HistStruct *hisvptri;
        		//hisvptri = &(m_his_table[movesSearched[icnt].dest][PIECE_IDX16(movesSearched[icnt].tabval)]);
        		//hisvptri = &(m_his_table[PIECE_IDX(movesSearched[icnt].tabval)][movesSearched[icnt].dest]);
        		hisvptri = &(m_his_table[t][PIECE_IDX(movesSearched[icnt].tabval)][movesSearched[icnt].dest]);
        		hisvptri->HistTot++;
        		if (hisvptri->HistTot >= HistoryMax)
            {
            	//for (int i=0; i<(BOARD_SIZE-7); i++)
							//for (int j=0; j<16; j++)
							for (int p=0; p<10; p++)
							for (int i=0; i<10; i++)
							for (int j=0; j<9; j++)
							{
								m_his_table[t][p][(i*16)+j].HistHit /= 2; // (m_his_table[p][i][j].HistHit + 1) / 2;
        				m_his_table[t][p][(i*16)+j].HistTot /= 2; // (m_his_table[p][i][j].HistTot + 1) / 2;
    					}
  					}

  					hisvptri->HistVal -= depth * depth;
            if (hisvptri->HistVal <= -HistValMax)
            {
    	        for (int p=0; p<10; p++)
		          for (int i=0; i<10; i++)
			        for (int j=0; j<9; j++)
			        {
				        m_his_table[t][p][(i*16)+j].HistVal /= 2; // (m_his_table[p][i][j].HistVal + 1) / 2;
			        }
            }


        }
    }
  }


//int seemovecnt=0; //for see()
//static const int IIDDepth = 3;
//static const int IIDReduction = 2;
#define IIDDepth 3
#define IIDReduction 2
#define IIDMargin 48 //FUTPAWN*3/2  //32*2=64  //48
#define NullMoveMargin IIDMargin*3/2
int Engine::searchPV(Board &board, int alpha, int beta, int depth) //, int null_allow)
{
    int val,incheck,newdepth, best, nHashFlag;
    
    MoveStruct mvBest;
    MoveTabStruct movesSearched[128];
    HistStruct *hisvptr;
#ifdef TTSINGU
//int best2nd;
//best2nd =
#endif
best = -INF;


//debug
#ifdef DEBUG
    if (depth>0)
        nTreeNodes ++;	// 統計樹枝節點
    else
        nLeafNodes ++;
//debug
#endif

// 6. 中斷調用﹔
        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;
        board.m_nodes++;
      //m_time_check++;
        //if ((m_time_check &8191)==0)  //4095)==0)

        if ((board.m_nodes      & PollNodes)==0) //4095)==0) //8191)==0)  //4095)==0)
         {
            //if (board.IMaxTime <4000)
          {
            BusyComm = BusyLine(UcciComm, false);
            if (BusyComm == UCCI_COMM_STOP)
            //if (BusyLine(UcciComm, false) == UCCI_COMM_STOP )
            {
            	// "stop"指令發送中止信號
            board.m_timeout= 1;	//stop
            //printBestmove(board.m_bestmove);
            printf("info searchPV stopped by GUI pv\n");
            fflush(stdout);
            	//return (board.m_side ? -INF-1 : INF+1);
            	return UNKNOWN_VALUE;
            	//longjmp(setjmpbuf,1);
            }

            else if (BusyComm == UCCI_COMM_QUIT)
            {
            	p_feedback_move = -1; //pass to Eychessu.cpp to quit the engine
            	//printf("info UCCI_COMM_QUIT p_feedback_move = %d\n", p_feedback_move);
			        //fflush(stdout);
            }
          }
            //if ((clock()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            //if ((GetTime()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);

    				if (t_remain <= 0)
            {


//    printf("info Search timout %d board.ply %d board.IMaxTime %d\n", (int)(GetTime() - board.m_startime), board.ply, board.IMaxTime);

            	    board.m_timeout= 1; //stop
            	    //printBestmove(board.m_bestmove);
                    return UNKNOWN_VALUE;
                    //longjmp(setjmpbuf,1);
            }
            if (t_remain < 4000)    				
    					PollNodes = 1023;
          
        }

// 2. 和棋裁剪﹔
    //if (pos.IsDraw())
//20230316 no IsDraw()   if ((board.bitpiece & 0xfff00ffc) == 0) return 0;

// mate-distance pruning
int old_alpha = alpha;
        alpha = std::max(-INF + board.ply, alpha);
        beta = std::min(INF - 1 - board.ply, beta);
        if (alpha >= beta)
            return (alpha);


    // 3. 重復裁剪﹔
    val=board.checkloop(1);
    if (val)
    {
     		return val;
    }

    // 5. 置換裁剪﹔
    HashStruct *hsho;
    int this_pv_move = 0;
    /*
    int nBetaDepth = 0;
    int nBetaVal = 0;
//    int nSingular = 0;
      int nAlphaDepth = 0;
    */
    // Step 4. Transposition table lookup sf 1.7.1
    // At PV nodes, we don't use the TT for pruning, but only for move ordering.
    // This is to avoid problems in the following areas:
    //
    // * Repetition draw detection
    // * Fifty move rule detection
    // * Searching for a mate
    // * Printing of full PV line
    //if (node_type==NodePV)	//strelka
    //{
      	//this_pv_move = ProbeMove(nBetaDepth, nBetaVal, nAlphaDepth); //, nSingular);
      	hsho = ProbeMove(board);
		//}
		if (hsho) this_pv_move = get_hmv(hsho->hmvBest);

    incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;

// Step 6. Razoring (is omitted in PV nodes)
    // Step 7. Static null move pruning (is omitted in PV nodes)
    // Step 8. Null move search with verification search (is omitted in PV nodes)
		// nullmove pruning (not for SearchPV)
    mvBest.move = 0;


    //八、內部深度迭代： Internal Iterative Deepening if no HashMove found
    // 如果Hash表未命中，使用depth-2進行內部深度迭代，目的是為了獲取HashMove
    // 感覺速度快10∼15%，有的局面並不顯得快，但也不會慢。
    // 殘局能夠增加Hash表的命中率，加快解體速度。即提前發現將軍的局面。

    if ( depth >= 5 //5  //IIDDepth  5(sf 1.7.1) //84q 3->5 fix hor_bis3 but others may be slower
    	&& this_pv_move == 0
         //&& node_type==NodePV
         //&& null_allow
		 )
    {        
        val = searchPV(board, alpha, beta, depth-2); //, NULL_NO);        
        hsho = ProbeMove(board);
        if (hsho) this_pv_move = hsho->hmvBest;
    }

    MoveTabStruct movetab[64], ncapmovetab[111]; //capmovetab[111],

    nHashFlag = HASH_ALPHA;
    int gen_count=0;
		int noncap_gen_count=0;
    int CheckExt=0;
//    int CaptureExt;

    if (board.ply < MAXEXT) //60) //MAXPLY - 4) //60 //30) //40)
    {
        if (incheck
        	)
        {
            CheckExt=1;
        }
    }

    MoveTabStruct tempmove;
    int size=0;
    long ncapsize=0;
    int capture=0;
	//int kingidx,
	//int opt_value;
    //for (int phase=6; phase--;)
    for (int phase=5; phase--;)
    {
        //if (phase==0 && ncapsize==0)
        //	break; // break for phase

        int SingleExt=0;	//singleext in PV
        switch (phase)
        {

 case 0: //1:	// nocap

    if (incheck)
    {
        //size=board.GenChkEvasNCap(&movetab[0], incheck);
        size=board.GenChkEvasNCap(&ncapmovetab[0], incheck);
    }
    else
    {
    			for (int j=0; j<ncapsize; j++)
        {
            MoveTabStruct *tabptr;
            tabptr = &ncapmovetab[j];
            //MoveTabStruct tabptr = ncapmovetab[j];
            // cal tabval for hors/pawn/bis/ele/king noncap
            if (tabptr->tabval >=0)
            {
                int piecefromidx = PIECE_IDX(board.piece[tabptr->from]);
            		//tabptr->tabval = m_his_table[piecefromidx][nFile(tabptr->dest)][nRank(tabptr->dest)].HistVal
                //int hs = his_table(piecefromidx, tabptr->dest);

            		// Ensure history has always highest priority
                //if (hs > 0)
                //	hs += 512;
                // if horse/pawn attk opp king (potential checking) sort to front
                //int piecefromtype = board.piece[tabptr->from]>>2;
                //if (piecefromtype == HORSE || piecefromtype <= PAWN)
                //	hs +=	kingattk_incl_horse[kingindex[board.boardsq[33-board.m_side]]][tabptr->dest]<<13; //+8192	
            		tabptr->tabval = 
                  (abs_pointtable[ext_p_endgame][piecefromidx][nRank(tabptr->dest)][nFile(tabptr->dest)]
                 - abs_pointtable[ext_p_endgame][piecefromidx][nRank(tabptr->from)][nFile(tabptr->from)]) ;
                if (depth > 1)
                	 //20230309 tabptr->tabval += his_table(piecefromidx, tabptr->dest);
                	 //#define his_table(p, sq) (m_his_table[thd_id][p][sq].HistVal)
                	 tabptr->tabval += m_his_table[board.thd_id][piecefromidx][tabptr->dest].HistVal;
            }
        }
    	//size=board.GenNonCap(&movetab[0]);
    	size=board.GenNonCap(&ncapmovetab[ncapsize], depth);
    	size += ncapsize;
    }


#ifdef FUTILITY
//    opt_value = INF;   //for futility
#endif
        		//sort noncap
        		//Quicksort(0, size-1, movetab);
        		//std::sort(movetab, movetab+size);
        		std::sort(ncapmovetab, ncapmovetab+size);
        		break;

         case 1: //2: //killer
            size=2; //NUM_KILLERS; //3; //4; //2;
            break;

          case 2: //3: //capture
        {

            if (incheck)
            {
            		size=board.GenChkEvasCap(&movetab[0], incheck);

								//if (node_type==NodePV && board.ply < MAXEXT) //MAXPLY - 4) //50)
					if (board.ply < MAXEXT)
								{
								if (size <= 1)
									  SingleExt=1;
          			//single-reply extension
                else //if (size > 1 && board.ply < MAXEXT) //MAXPLY - 4) //50)
    						{

        					int gcount=0;
        					for (int j=0; j<size; j++)
        					{
        						//if (makemovechk(movetab[j].table) ==0)
            				if (board.makemovenoeval(movetab[j].table, 1) >=0) //, 1) >=0)
            				{
                			board.unmakemovenoeval();
                			gcount++;
                			if (gcount > 1)
                    		break;
            				}
        					}
        					if (gcount <= 1)
            				SingleExt=1;
       					}
       					//if (capsize <=1 && board.ply < 50)
       					//	Extended=2;
       				} // end if board.ply

            }
            else
            {
                size=board.GenCap(&movetab[0], &ncapmovetab[0], ncapsize);
                //size=(board.m_side ? board.GenCap<1>(&movetab[0], &ncapmovetab[0], ncapsize)
                //                   : board.GenCap<0>(&movetab[0], &ncapmovetab[0], ncapsize));
                //size=board.GenCapQS(&movetab[0]);
            }
             //sort capture
             //Quicksort(0, size-1, movetab);
        }
        break;
case 3: //4:  //mate-killer
        //default:  // phase 4=mate-killer, 5=hash pv
        		size=1;
        		break;
case 4: //5:  //hash pv
        //default:  // phase 4=mate-killer, 5=hash pv
        		size=1;
        		break;


        } // end switch

//        if (phase==0 && size==0)
//        	break; // break for phase

				//if (incheck==0)
			 	//				kingidx=kingindex[board.boardsq[32+board.m_side]];

        for (int i=0; i<size; i++)
        {
//           CaptureExt=0;

           switch (phase)
            {
            case 0: //1:	//noncap
            {
            	  //tempmove.tabentry = movetab[i].tabentry; //after std::sort noncap
            	  tempmove.tabentry = ncapmovetab[i].tabentry; //after std::sort noncap
        				if (tempmove.table.move==this_pv_move
                ||
                tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                ||
                tempmove.table.move==g_killer[board.thd_id][0][board.ply].move
                || tempmove.table.move==g_killer[board.thd_id][1][board.ply].move

           			)
            		continue;
            }
            break;

           case 1: //2:	//killer
           	{
                tempmove.table.move=g_killer[board.thd_id][i][board.ply].move;
                if (tempmove.table.move==0
                        ||
                        tempmove.table.move==this_pv_move
                        ||
                        tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                        || board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
           case 2: //3:	//capture
            {
                tempmove.tabentry = GetNextMove(i, size, movetab);
                if (
                        tempmove.table.move==this_pv_move
                        ||
                        tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                   )
                    continue;
//                seemovecnt = 0;
//                capture = board.piece[tempmove.dest];
                if (incheck==0 && (board.piece[tempmove.dest] < B_HORSE1
        	//|| (board.piece[tempmove.from]>=B_ROOK1
        	|| ( (board.piece[tempmove.from]>>2)==ROOK  //exclude king capture for SEE()
        	&& board.piece[tempmove.dest] < B_ROOK1)
			)
			)
        {
        	val = board.see(tempmove.from, tempmove.dest);

        				if (val == -9999) //1882bug  ??
        					continue;
        			if (val < 0)
              {
              	ncapmovetab[ncapsize].table.move = tempmove.table.move;
                ncapmovetab[ncapsize].tabval = val - HistValMax; // Be sure are at the bottom
                ncapsize++;
        				continue;
        			}

        }

            }
            break;

						case 3: //4:	//matekiller
            {
                tempmove.table.move=g_matekiller[board.thd_id][board.ply].move;
                if (tempmove.table.move==0
                        ||
                        tempmove.table.move==this_pv_move
                        || board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
            case 4: //5:	//hashmove
            {
                tempmove.table.move=this_pv_move;
                if (tempmove.table.move==0
                        ||
                        board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
           } // end switch

            int piecefrom = board.piece[tempmove.from];
            //hisvptr = &(m_his_table[PIECE_IDX(board.piece[tempmove.from])][tempmove.dest]);
            //hisvptr = &(m_his_table[PIECE_IDX(piecefrom)][tempmove.dest]);
            hisvptr = &(m_his_table[board.thd_id][PIECE_IDX(piecefrom)][tempmove.dest]);  //20230309

            newdepth = depth - 1 + CheckExt + SingleExt;
/* sf 2.0
// Singular extension search. If all moves but one fail low on a search of (alpha-s, beta-s),
      // and just one fails high on (alpha, beta), then that move is singular and should be extended.
      // To verify this we do a reduced search on all the other moves but the ttMove, if result is
      // lower then ttValue minus a margin then we extend ttMove.
      if (   singularExtensionNode
          && move == tte->move()
          && ext < ONE_PLY)
      {
          Value ttValue = value_from_tt(tte->value(), board.ply);

          if (abs(ttValue) < VALUE_KNOWN_WIN)
          {
              Value b = ttValue - SingularExtensionMargin;
              ss->excludedMove = move;
              ss->skipNullMove = true;
              Value v = search<NonPV>(pos, ss, b - 1, b, depth / 2, board.ply);
              ss->skipNullMove = false;
              ss->excludedMove = MOVE_NONE;
              ss->bestMove = MOVE_NONE;
              if (v < b)
                  ext = ONE_PLY;
          }
      }
*/
#ifdef TTSINGU
//TT Singular extension
if (phase==4 //hashmove
&& depth >= 6  //pv singular ext depth
//&& tempmove.table.move==this_pv_move // this is hashmove
//&& nAlphaDepth == 0 //beta TT  //&& nSingular == 1  	//1889r allow pv-tt for pv tt-sing ext
&& (hsho)  //hashmove may be from IID
&& hsho->hFlag==HASH_BETA
//&& depth - (hsho->hDepth5*4+hsho->hDepth2) <= 3 // recent hash entry
&& depth - (hsho->hDepth) <= 3 // recent hash entry
// cross river
&& OPPHALF(tempmove.dest, board.m_side)
&& newdepth < depth // not yet Ext
//&& kingattk_incl_horse[kingindex[board.boardsq[33-board.m_side]]][tempmove.dest]  //move attack oppking
//&& abs(hsho->hBetaVal) < WIN_VALUE
&& (hsho->hVal > -WIN_VALUE && hsho->hVal < WIN_VALUE)
)
{
/*	
	capture = makemove<1>(tempmove.table);
	if (capture < 0)
			 goto end_singpv; // skip illegal move
	unmakemove();
	if (m_hisrecord[m_hisindex].htab.Chk)
		goto end_singpv;	// skip checking hashmove
*/		
		
#ifdef PRTBOARD
	printf("\nPV TT-singular extension, score=hBetaVal");
	print_board(hsho->hBetaVal);
  com2char(charmove, tempmove.from, tempmove.dest );
  printf("  %s\n", charmove);
  fflush(stdout);
#endif	
int sing_size,  sing_val, sing_beta, sing_best;
long sing_badcapsize;
//if (board.piece[tempmove.dest])
//if (capture)	
//{	
#ifdef PRTBOARD		
	sing_best = -INF;
	sing_beta = hsho->hBetaVal - SINGULAR_MARGIN; //_PV;
#endif	
//	goto cap_singpv;
//}	
	
	MoveTabStruct sing_tempmove;
	MoveTabStruct sing_movetab[111], sing_badcapmovetab[64];
	sing_badcapsize = 0;
	sing_best = -INF;
	sing_beta = hsho->hVal - SINGULAR_MARGIN; //_PV; 89y PV=npv=futp/2=16

	for (int sing_phase=2; sing_phase--;)
	{
		switch(sing_phase)
		{
			case 0:
			{
  			sing_size = board.GenNonCap(&sing_movetab[0], depth);
  			memcpy(&sing_movetab[sing_size], &sing_badcapmovetab[0], sing_badcapsize * 4);
  			sing_size += sing_badcapsize;
				break;
			}
			case 1:
			{
				sing_size = board.GenCap(&sing_movetab[0], &sing_badcapmovetab[0], sing_badcapsize);
				std::sort(sing_movetab, sing_movetab+size);
  			break;
			}
		} // end switch

	for (int i = 0; i < sing_size; i++)
	{
		sing_tempmove.tabentry = sing_movetab[i].tabentry;
		if (sing_tempmove.table.move == this_pv_move)
			 continue;	// skip hashmove
		if (board.makemove(sing_tempmove.table, 1) < 0)
			 continue; // skip illegal move
		//sing_val = -quiesCheck<0>(-sing_beta, -(sing_beta-1), 0, qcheck_depth);
		sing_val = -AlphaBeta(board, 1-sing_beta, depth/2, NULL_NO);
		
#ifdef PRTBOARD	
		com2char(charmove, sing_tempmove.from, sing_tempmove.dest );
  printf("makemove %s sing_val=%d\n", charmove, sing_val);
  fflush(stdout);
#endif
  
		board.unmakemove();
		if (sing_val > sing_best)
		{
			sing_best = sing_val;
			if (sing_best >= sing_beta)
				//break;
				goto end_singpv;
		}
	}	//next i

} // next sing_phase

	if (sing_best != -INF && sing_best < sing_beta)
	{
//cap_singpv:
#ifdef PRTBOARD	
	printf("info TT-singular extension at pv, sing_best=%d, sing_beta=%d, newdepth=%d, depth=%d, hBetaDepth=%d\n", sing_best, sing_beta, newdepth, depth, hsho->hBetaDepth);
	fflush(stdout);
#endif	
//	pv_singular_cnt ++;
	newdepth = depth; //TT singleext 1 board.ply
  }

}
end_singpv:

#endif

/*  1888c - no recapture extension
            // recapture   //only at node_type==NodePV
            if (phase==2    //SEE+ capture phase
            && CheckExt==0
            //&& seemovecnt >0 //1
            //88a
            //&&   piecefrom < B_ROOK1    //87z
            //&& ((board.piece[tempmove.dest] <= R_ELEPHAN2
            //&&   board.piece[tempmove.dest] >= B_ADVISOR1

            //)
            //|| (piecefrom>>2) == (board.piece[tempmove.dest]>>2)
            //)
            //

            && m_hisindex >0 //>=1
            && m_hisrecord[m_hisindex-1].htab.dest==tempmove.dest
            && board.piece[tempmove.dest] < B_ROOK1 //sac cann/horse/pawn for adv/elep
            && m_hisrecord[m_hisindex-1].htab.capture <= R_ELEPHAN2
            && m_hisrecord[m_hisindex-1].htab.capture >= B_ADVISOR1

						//		|| (board.piece[tempmove.dest]>>2) == (m_hisrecord[m_hisindex-1].capture >>2) )

            )
            	{


#ifdef PRTBOARD
printf("\nrecapture extension, score=prev capture");
	print_board(m_hisrecord[m_hisindex-1].htab.capture);
        com2char(charmove, tempmove.from, tempmove.dest );
        printf("  %s\n", charmove);
    fflush(stdout);
#endif

            	newdepth=depth;
              }
*/



// History pruning and Futility (not for SearchPV)
//if (phase<=1)	// noncap + badcap
//{
//}	//end if phase<=1

        if (incheck
					|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
					|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
        		capture = board.makemove(tempmove.table, 1);
        else
		        capture = board.makemove(tempmove.table, 0);

        if (capture < 0)
        	continue;

        if (capture==0)
        {
        	movesSearched[noncap_gen_count].table.move = tempmove.table.move;
        	movesSearched[noncap_gen_count].tabval = piecefrom;
        	noncap_gen_count++;
        }



        gen_count++;
        int reduced=0;



   			//if ((node_type==NodePV) && (board.ply < MAXEXT))
				if (board.ply < MAXEXT)
        {
                		if (capture && board.p_endgame && board.m_hisrecord[board.m_hisindex-1].endgame == 0)
                			newdepth=depth;	// extend if just enter board.p_endgame
      } // end if node_type

            if (newdepth <= 0)

                val = -quiesCheckPV(board, -beta, -alpha, newdepth, qcheck_depth);

            else

            {
            //if (node_type !=NodePV || best == -INF)	// first move //toga
            if (best == -INF)	// first move //toga
						{                
                val = -searchPV(board, -beta, -alpha, newdepth); //, NULL_YES);
            }
            else	// other moves
            {
if (phase==0) //<=1)
{
#ifdef HISTPRUN
		if (depth>= HistoryDepth && !incheck && newdepth < depth && gen_count >= 6 //9 //6
			//&& (capture==0)  //diff from stockfish
			&& board.m_hisrecord[board.m_hisindex-1].htab.Chk==0)
		{
#ifdef PVRED
						reduced=pvred(depth, gen_count);
						newdepth -= reduced;
#else
            //if (gen_count >= 18 && depth >=23)	//PV LMR
            //{
            //	reduced=2;
            //	newdepth -= 2;
            //}
            //else
        //    if (depth==3 && gen_count < 16)
				//;
        //    else
        if (board.p_endgame || ((board.EvalSide<1>() < THREAT_MARGIN) //== 0
      	&&  (board.EvalSide<0>() < THREAT_MARGIN)) //== 0
      	//if ((board.m_side ? board.EvalSide<1>() : board.EvalSide<0>()) < THREAT_MARGIN
					 )
						{	reduced=1;
							newdepth--;
						}
#endif
					//newdepth -= reduced;
		}
#endif
} // end if phase<=1

            if (newdepth <= 0)
                //val = -quiesCheck<0>(-(alpha+1), -alpha, newdepth, qcheck_depth);
                val = -quiesCheck(board, 1-beta, newdepth, qcheck_depth);
            else
                
                val = -AlphaBeta(board, -alpha, newdepth , NULL_YES);
                        // history-pruning re-search
#ifdef HISTPRUN
                if (reduced && val >alpha)  //>=beta)	//was >alpha
        				{
            			newdepth+=reduced;                
                  val = -AlphaBeta(board, -alpha, newdepth, NULL_YES); // avoid double re-search
        				}
#endif
                // Step extra. pv search (only in PV nodes)
                if ((val > alpha) && (val < beta))
                {
            if (newdepth <= 0)
                val = -quiesCheckPV(board, -beta, -alpha, newdepth, qcheck_depth);
            else                    
                    val = -searchPV(board, -beta, -alpha, newdepth); //, NULL_YES);  //toga
                }
            }
						}


        board.unmakemove();

        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;



                    // 12. Alpha-Beta邊界判定
/* sf191
// Step 17. Check for new best move
      if (value > bestValue)
      {
          bestValue = value;
          if (value > alpha)
          {
              if (PvNode && value < beta) // We want always alpha < beta
                  alpha = value;

              if (value == value_mate_in(board.ply + 1))
                  ss->mateKiller = move;

              ss->bestMove = move;
          }
      }
*/
        //  val >= best >= val >= best2nd >= val
        if (val <= best)
        {
#ifdef TTSINGU
//        		if (val > best2nd)
//        			best2nd = val;
#endif
        }
        else
        //if (val > best)
        {
            best = val;
            if (val >= beta)
            {
                RecordHash(HASH_BETA, depth, val, tempmove.table.move, board); //, nSingular);
                if (capture==0  //update killer and history for non-captures
                    && phase==0 //<=1
                    && !incheck
                    && !board.m_timeout  //sf191
                   )
                {
                    if (val >= WIN_VALUE)
                        g_matekiller[board.thd_id][board.ply].move = tempmove.table.move;
                    else
                    {
                        if (g_killer[board.thd_id][0][board.ply].move != tempmove.table.move)
                        {

                            g_killer[board.thd_id][1][board.ply].move = g_killer[board.thd_id][0][board.ply].move;
                            g_killer[board.thd_id][0][board.ply].move = tempmove.table.move;

                        }
                    }
                    update_HistVal(board.thd_id, hisvptr, depth);
                    update_history(board.thd_id, hisvptr, depth, movesSearched, noncap_gen_count);
                }

                return val;
            }


            if (val > alpha)
            {
            	  //critter
            	  /*
            	  //RecordHash(HASH_ALPHA, depth, val, tempmove.table.move, board);
            	  if (capture==0  //update history for non-captures
                    && phase==0 //<=1
                    && !incheck
                    && !board.m_timeout  //sf191
                   )
                {
            	  update_HistVal(hisvptr, depth);
                update_history(board.thd_id, hisvptr, depth, movesSearched, noncap_gen_count);
              }
               */          	  
            	  //if (PvNode && //sf191
            	  if (val < beta) // We want always alpha < beta  //sf191
                	alpha = val;
                mvBest.move = tempmove.table.move;
                //update_pv(sstack, board.ply, mvBest.move); //tempmove.table.move);
                //mvBest = mv;
                nHashFlag = HASH_PV;
            }
        }

        //no longer diff NodeCut and NodeAll
        //if (node_type == NodeCut) node_type = NodeAll;

        } // end for i
    }	// end for phase

//-----------------------------------------------

    if (gen_count==0)
    //if (best == -INF)
    {	//bestmove = 0;
        return board.ply-INF;	// No moves in this pos, lost by checkmate or stalemate
    }
    // 13. 更新置換表、歷史表和殺手著法表
//cut:
    if (board.m_timeout) //sf191
    	return best;

    if (mvBest.move)
    {
        RecordHash(nHashFlag, depth, best, mvBest.move, board); //, 0);

        //if (capture==0 && node_type==NodePV && !incheck)
        if (capture==0 && !incheck)
        {
            if (best >= WIN_VALUE)
                g_matekiller[board.thd_id][board.ply].move = mvBest.move;
            else
            {
                if (g_killer[board.thd_id][0][board.ply].move != mvBest.move)
                {
                    g_killer[board.thd_id][1][board.ply].move = g_killer[board.thd_id][0][board.ply].move;
                    g_killer[board.thd_id][0][board.ply].move = mvBest.move;
                    //hisvptr = &(m_his_table[mvBest.dest][PIECE_IDX16(board.piece[mvBest.from])]);
                    //hisvptr = &(m_his_table[PIECE_IDX(board.piece[mvBest.from])][mvBest.dest]);
                    hisvptr = &(m_his_table[board.thd_id][PIECE_IDX(board.piece[mvBest.from])][mvBest.dest]);
                    update_HistVal(board.thd_id, hisvptr, depth);
                }
            }
            //hisvptr = &(m_his_table[mvBest.dest][PIECE_IDX16(board.piece[mvBest.from])]);
            //update_HistVal(hisvptr, depth);
        }

    }
    else if (best <= old_alpha)
    	RecordHash(nHashFlag, depth, best, mvBest.move, board); //, 0);

    return best; //thisalpha;

}

int Engine::AlphaBeta(Board &board, int beta,int depth, int null_allow)
{
    int val,capture,incheck,newdepth, best, nHashFlag;
    MoveStruct mvBest;
    MoveTabStruct movesSearched[128];
    HistStruct *hisvptr;
    int alpha = beta - 1;
#ifdef TTSINGU
//    int best2nd;
//    best2nd =
#endif
    best = -INF;
//debug
#ifdef DEBUG
    if (depth>0)
        nTreeNodes ++;	// 統計樹枝節點
    else
        nLeafNodes ++;
//debug
#endif


// 6. 中斷調用﹔
        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;
        board.m_nodes++;
      //m_time_check++;
        //if ((m_time_check &8191)==0)  //4095)==0)
        if ((board.m_nodes      & PollNodes)==0) //4095)==0) //8191)==0)  //4095)==0)
         {
            //if (board.IMaxTime <4000)
          {
            BusyComm = BusyLine(UcciComm, false);
            if (BusyComm == UCCI_COMM_STOP)
            //if (BusyLine(UcciComm, false) == UCCI_COMM_STOP )
            {
            	// "stop"指令發送中止信號
            board.m_timeout= 1;	//stop
            //printBestmove(board.m_bestmove);
            printf("info searchAB stopped by GUI pv\n");
            fflush(stdout);
            	//return (board.m_side ? -INF-1 : INF+1);
            	return UNKNOWN_VALUE;
            	//longjmp(setjmpbuf,1);
            }

            else if (BusyComm == UCCI_COMM_QUIT)
            {
            	p_feedback_move = -1; //pass to Eychessu.cpp to quit the engine
            	//printf("info UCCI_COMM_QUIT p_feedback_move = %d\n", p_feedback_move);
			        //fflush(stdout);
            }
          }
          
            //if ((clock()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            //if ((GetTime()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);
    				if (t_remain <= 0)
            {                        
                  	 board.m_timeout= 1; //stop
    printf("info timout\n"); // %d board.ply %d board.IMaxTime %d\n", (int)(GetTime() - board.m_startime), board.ply, board.IMaxTime);
    fflush(stdout);
            	    //printBestmove(board.m_bestmove);
                  //if (jmp_ok)
                  //  longjmp(setjmpbuf,1);
                  return UNKNOWN_VALUE;
            }
            
          if (t_remain < 4000)    			
    					PollNodes = 1023;
          
          
          
        }

// 2. 和棋裁剪﹔
    //if (pos.IsDraw())
//20230316    if ((board.bitpiece & 0xfff00ffc) == 0) return 0;

 // Step 3. Mate distance pruning
 //   if (value_mated_in(board.ply) >= beta)
      if (-INF+board.ply >= beta)
        return beta;

    //if (value_mate_in(board.ply + 1) < beta)
    if (INF-(board.ply + 1) < beta)
        return beta - 1;

// mate-distance pruning

int old_alpha = alpha;
//        alpha = MAX(-INF + board.ply, alpha);
//        beta = MIN(INF - 1 - board.ply, beta);
//        if (alpha >= beta)
//            return (alpha);


    // 3. 重復裁剪﹔

    val=board.checkloop(1);
    if (val)
    {
     		return val;

    }

    // 5. 置換裁剪﹔
    HashStruct *hsho;
    //hsho = NULL;
    /*
    int this_pv_move = 0;
    int nBetaDepth = 0;
    int nBetaVal = 0;
//    int nSingular = 0;
    int nAlphaDepth = 0;
    */
    //if (node_type==NodePV)	//strelka
    //{
    //  	this_pv_move = ProbeMove();
		//}
		//else
		//{
    //val = ProbeHash(depth, beta, null_allow, this_pv_move, sstack[0], nBetaDepth, nBetaVal, nAlphaDepth); //, nSingular);
    val = ProbeHash(depth, beta, null_allow, hsho, board);
    if (val != UNKNOWN_VALUE)
        return val;
    //}
    int this_pv_move = 0;
    if (hsho) this_pv_move = hsho->hmvBest;


    incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;


/* sf 2.0
    // Step 6. Razoring (is omitted in PV nodes)
*/
    int evalscore, futpawn_x2depth_1; 
    
	  if (depth <= 3 && !incheck) 
	  {
	  	evalscore = Evalscore(board);
	  	futpawn_x2depth_1 = (depth*2-1)*FUTPAWN;	
	  	if (this_pv_move == 0
	  	&& evalscore < beta - futpawn_x2depth_1 *3 
	  	//&& abs(beta) < WIN_VALUE
	  	&& (beta > -WIN_VALUE && beta < WIN_VALUE)
	  	)
	  	{
	  		int rbeta = beta - futpawn_x2depth_1 *3; //(16 + depth*4);
	  		val = quiesCheck(board, rbeta, 0, qcheck_depth);	
	  		if (val < rbeta)
	  			return val;
	  	}		
/* sf 2.0
    // Step 7. Static null move pruning (is omitted in PV nodes)
    // We're betting that the opponent doesn't have a move that will reduce
    // the score by more than futility_margin(depth) if we do a null move.
    if (   !PvNode
        && !ss->skipNullMove
        &&  depth < RazorDepth
        && !isCheck
        &&  refinedValue >= beta + futility_margin(depth, 0)
        && !value_is_mate(beta)
        &&  pos.non_pawn_material(pos.side_to_move()))
        return refinedValue - futility_margin(depth, 0);
*/
		if (null_allow //&& depth <=3 && !incheck
			&& evalscore >= beta + futpawn_x2depth_1 //(depth*2-1)*FUTPAWN
			//&& abs(beta) < WIN_VALUE
			&& (beta > -WIN_VALUE && beta < WIN_VALUE)
			)
		{
			return evalscore - futpawn_x2depth_1; //(depth*2-1)*FUTPAWN;
		}		
	}


		// nullmove pruning
    mvBest.move = 0;
    if (null_allow && depth>1 && !incheck //&& node_type != NodePV
    	//probe TT to filter out nullmove 
    	// If a plain reduced search without nullmove won't produce a cutoff... 
    	//if (tt_depth >= depth - R - 1 && tt_score < beta && tt_type & UPPER) skip_null();     		
    	//slower //&& !(hsho && hsho->hAlphaDepth >= depth - nullreduction - 1 && hsho->hAlphaVal < beta)
    	
            //&& (beta <= ValueEvalInf)
            //&& (beta > -WIN_VALUE && beta < WIN_VALUE)
            //&& (abs(beta) < WIN_VALUE)
            //&& bitCountMSB(bitpiece & (0x55500000<<board.m_side))>=2  //major>=2
       )
    {
        if ( (depth <=4
            || (
            !(hsho && hsho->hFlag >= HASH_ALPHA && (hsho->hDepth) >= depth - nullreduction - 1 && hsho->hVal < beta)
             &&
              Evalscore(board) >= beta //- NullMoveMargin //sf 1.8
            // stockfish 1.3 approximateEval >= beta - NullMoveMargin
            //|| (board.m_side ? -pointsum : pointsum) >= beta - VN             
               )
              )
              && (beta > -WIN_VALUE && beta < WIN_VALUE)
            
        )
        {
//debug		nNullMoveNodes ++;
            //makenull();


    /*
            HistRecord *hisptr;
    hisptr = m_hisrecord + m_hisindex;
    //hisptr->hkey =hkey;  //no need since nullmove is irrevisible and not draw
    hisptr->htab.tabentry = 0; //move,capture,chk=0;
    //hisptr->pointsum =pointsum;
		hisptr->mvpiece = 0;
		//histptr->endgame = 0;
		*/
		        board.m_hisrecord[board.m_hisindex].htab.tabentry = 0; //move,capture,chk=0;
		        board.m_hisrecord[board.m_hisindex].mvpiece = 0;
            board.m_hisindex++;
            board.ply++;
            chgside(board.m_side);
            board.Xor(board.hkey,h_rside);


            //newdepth=depth - (nullreduction+1); // - 1;  // -4  R=3
            //nullreduction = (depth >= 5 ? 4 : 3); // Null move dynamic reduction
            newdepth=depth - nullreduction - 1; // - 1;  // -4  R=3
            //if (depth>6) newdepth++;  //>5
						if (newdepth <= 0)
      			  val = -quiesCheck(board, 1-beta, newdepth, qcheck_depth);
      			else            	
            	val = -AlphaBeta(board, 1-beta, newdepth, NULL_NO);
            //unmakenull();

            board.Xor(board.hkey,h_rside);
            chgside(board.m_side);
            board.ply--;
            board.m_hisindex--;
            if (board.m_timeout)
						   return UNKNOWN_VALUE;

						if ((val >= WIN_VALUE) || (val <= -WIN_VALUE))
            {        	//val = WIN_VALUE; // do not return unproven mates
            }
            else

            if (val >= beta)
            {
                //if ((val >= WIN_VALUE) || (val <= -WIN_VALUE))
                //    	val = beta; // do not return unproven mates //sf 1.7.1
                if (
                	(depth <= nullreductver)
             //    || bitCountMSB(bitpiece & (0x55500000<<board.m_side))>=4  //major>=4
               ||               
               AlphaBeta(board, beta, depth - nullreductver, NULL_NO) >= beta
                   )
                    {
                        if ((newdepth > 0) && !board.m_timeout)
                            RecordHash(HASH_BETA, std::max(depth, nullreduction), val, 0, board); //, 0); //mvBest.move);  //mvBest=0
                        return val; //beta
                    }
            }

        }
    }


// IID for nonPV
    if ( depth >= 8  //IIDDepthNonPV  8(sf 1.7.1)
    	&& this_pv_move == 0
    	&& !incheck
        && Evalscore(board) >= beta - IIDMargin
		 )
    {        
        val = AlphaBeta(board, beta, depth/2, NULL_NO);	// crafty NULL_YES // 不使用帶風險的裁剪
        
        //this_pv_move = sstack[ply].pv[ply];
        
        hsho = ProbeMove(board);
        if (hsho) this_pv_move = (hsho->hmvBest);
    }

    MoveTabStruct movetab[64], ncapmovetab[111]; //64]; //capmovetab[111],

    nHashFlag = HASH_ALPHA;
    int gen_count=0;
		int noncap_gen_count=0;
    int CheckExt=0;

    //bool rook_horse_threat = false;
    if (board.ply < MAXEXT) //60) //MAXPLY - 4) //60 //30) //40)
    {
        if (incheck
        	)
        {
            CheckExt=1;
        }
    }

    MoveTabStruct tempmove;
    int size=0;
    long ncapsize=0;
	//int kingidx,
	int opt_value=INF;
//int phase;

    for (int phase=5; phase--;)
    {

        switch (phase)
        {

 case 0: // nocap

    if (incheck)
    {
        //size=board.GenChkEvasNCap(&movetab[0], incheck);
        size=board.GenChkEvasNCap(&ncapmovetab[0], incheck);
    }
    else
    {
    	//size=board.GenNonCap(&movetab[0]);
    	//append pawn/bis/ele/king noncap
    		for (int j=0; j<ncapsize; j++)
        {
            MoveTabStruct *tabptr;
            tabptr = &ncapmovetab[j];
            //MoveTabStruct tabptr = ncapmovetab[j];
            // cal tabval for pawn/bis/ele/king noncap
            if (tabptr->tabval >=0)
            {
                int piecefromidx = PIECE_IDX(board.piece[tabptr->from]);
            		//tabptr->tabval = m_his_table[piecefromidx][nFile(tabptr->dest)][nRank(tabptr->dest)].HistVal
                //int hs = his_table(piecefromidx, tabptr->dest);

            		// Ensure history has always highest priority
                //if (hs > 0)
                //	hs += 512;
                // if horse/pawn attk opp king (potential checking) sort to front
                //int piecefromtype = board.piece[tabptr->from]>>2;
                //if (piecefromtype == HORSE || piecefromtype <= PAWN)
                //	hs +=	kingattk_incl_horse[kingindex[board.boardsq[33-board.m_side]]][tabptr->dest]<<13; //+8192
            		tabptr->tabval = 
                  (abs_pointtable[ext_p_endgame][piecefromidx][nRank(tabptr->dest)][nFile(tabptr->dest)]
                 - abs_pointtable[ext_p_endgame][piecefromidx][nRank(tabptr->from)][nFile(tabptr->from)]) ;
                if (depth > 1)
                	 //20230309 tabptr->tabval += his_table(piecefromidx, tabptr->dest);    
                	 //#define his_table(p, sq) (m_his_table[thd_id][p][sq].HistVal)
                	 tabptr->tabval += m_his_table[board.thd_id][piecefromidx][tabptr->dest].HistVal;         
            }
            //movetab[size] = ncapmovetab[j];
            //size++;
        }
        //memcpy(&movetab[size], &ncapmovetab[0], sizeof(ncapmovetab[0]) * ncapsize);
        //size += ncapsize;
        size=board.GenNonCap(&ncapmovetab[ncapsize], depth);
        size += ncapsize;
    }


#ifdef FUTILITY
//    opt_value = INF;   //for futility
#endif
        		//sort noncap
        		//Quicksort(0, size-1, movetab);
        		//std::sort(movetab, movetab+size);
        		std::sort(ncapmovetab, ncapmovetab+size);
        		break;

         case 1: //2: //killer
            size=2; //NUM_KILLERS; //3; //4; //2;
            break;

          case 2:  //capture
        {

            if (incheck)
            {
            		size=board.GenChkEvasCap(&movetab[0], incheck);
          			// no single-reply extension for nonPV
            }
            else
            {
                size=board.GenCap(&movetab[0], &ncapmovetab[0], ncapsize);
            }
             //sort capture
             //Quicksort(0, size-1, movetab);
        }
        break;
case 3:   //mate-killer

        		size=1;
        		break;
case 4:   //hash pv

        		size=1;
        		break;


        } // end switch

//        if (phase==0 && size==0)
//        	break; // break for phase

				//if (incheck==0)
			 	//				kingidx=kingindex[board.boardsq[32+board.m_side]];

        for (int i=0; i<size; i++)
        {
           switch (phase)
            {

            case 0: 	//noncap
            {
            	  //tempmove.tabentry = movetab[i].tabentry; //after std::sort noncap
            	  tempmove.tabentry = ncapmovetab[i].tabentry; //after std::sort noncap
        				if (tempmove.table.move==this_pv_move
                ||
                tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                ||
                tempmove.table.move==g_killer[board.thd_id][0][board.ply].move
                || tempmove.table.move==g_killer[board.thd_id][1][board.ply].move

           			)
            		continue;
            }
            break;

           case 1: 	//killer
           	{
                tempmove.table.move=g_killer[board.thd_id][i][board.ply].move;
                if (tempmove.table.move==0
                        ||
                        tempmove.table.move==this_pv_move
                        ||
                        tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                        || board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
           case 2: 	//capture
            {
                tempmove.tabentry = GetNextMove(i, size, movetab);
                if (
                        tempmove.table.move==this_pv_move
                        ||
                        tempmove.table.move==g_matekiller[board.thd_id][board.ply].move
                   )
                    continue;

                if (incheck==0 && (board.piece[tempmove.dest] < B_HORSE1
        	//|| (board.piece[tempmove.from]>=B_ROOK1
        	|| ((board.piece[tempmove.from]>>2)==ROOK  //exclude king capture for SEE()
        	&& board.piece[tempmove.dest] < B_ROOK1)))
        {
        	val = board.see(tempmove.from, tempmove.dest);

        				if (val == -9999) //1882bug ??
        					continue;
        				if (val < 0)
                {
                                ncapmovetab[ncapsize].table.move = tempmove.table.move;
                                ncapmovetab[ncapsize].tabval = val - HistValMax; // Be sure are at the bottom
                                ncapsize++;

        				continue;
        			}

        }

            }
            break;

						case 3: 	//matekiller
            {
                tempmove.table.move=g_matekiller[board.thd_id][board.ply].move;
                if (tempmove.table.move==0
                        ||
                        tempmove.table.move==this_pv_move
                        || board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
            case 4: 	//hashmove
            {
                tempmove.table.move=this_pv_move;
                if (tempmove.table.move==0
                        ||
                        board.LegalKiller(tempmove.table)==0
                   )
                    continue;
            }
            break;
           } // end switch

            int piecefrom = board.piece[tempmove.from];
            //capture = board.piece[tempmove.dest];
            //hisvptr = &(m_his_table[tempmove.dest][PIECE_IDX16(board.piece[tempmove.from])]);
            //hisvptr = &(m_his_table[PIECE_IDX(piecefrom)][tempmove.dest]);
						hisvptr = &(m_his_table[board.thd_id][PIECE_IDX(piecefrom)][tempmove.dest]);  //20230309

            	newdepth = depth + CheckExt -1; // + SingleExt;

#ifdef TTSINGU
//TT Singular extension sf 2.0
if (phase==4 //hashmove
&& depth >= 8  //non-pv singular ext depth
//&& tempmove.table.move==this_pv_move // this is hashmove
&& (hsho) //hashmove may be from IID
&& hsho->hFlag == HASH_BETA
//&& nAlphaDepth == 0 //beta TT  //&& nSingular == 1
&& depth - (hsho->hDepth) <= 3  //recent hash entry
// cross river
&& OPPHALF(tempmove.dest, board.m_side)
//&& kingattk_incl_horse[kingindex[board.boardsq[33-board.m_side]]][tempmove.dest]  //move attack oppking
&& newdepth < depth // not yet Ext
//&& abs(hsho->hBetaVal) < WIN_VALUE
&& (hsho->hVal > -WIN_VALUE && hsho->hVal < WIN_VALUE)
)
{
//
//	if (makemove<1>(tempmove.table) < 0)
//			 goto end_singnonpv; // skip illegal move
//	unmakemove();
//	if (m_hisrecord[m_hisindex].htab.Chk)
//		goto end_singnonpv;	// skip checking hashmove
//	
#ifdef PRTBOARD
	printf("\nnonPV TT-singular extension, score=hBetaVal");
	print_board(hsho->hBetaVal);
  com2char(charmove, tempmove.from, tempmove.dest );
  printf("  %s\n", charmove);
  fflush(stdout);
#endif	

	int sing_size,  sing_val, sing_beta, sing_best;
	long sing_badcapsize;
//if (board.piece[tempmove.dest])
// {
//		sing_best = -INF;
//	  sing_beta = hsho->hBetaVal - SINGULAR_MARGIN; //2; //wider window for nonpv
//	goto cap_singnonpv;
//}
	MoveTabStruct sing_tempmove;
	MoveTabStruct sing_movetab[111], sing_badcapmovetab[64];
	sing_badcapsize = 0;
	sing_best = -INF;
	sing_beta = hsho->hVal - SINGULAR_MARGIN; //2; //wider window for nonpv

	for (int sing_phase=2; sing_phase--;)
	{
		switch(sing_phase)
		{
			case 0:
			{
  			sing_size = board.GenNonCap(&sing_movetab[0], depth);
  			memcpy(&sing_movetab[sing_size], &sing_badcapmovetab[0], sing_badcapsize * 4);
  			sing_size += sing_badcapsize;
				break;
			}
			case 1:
			{
				sing_size = board.GenCap(&sing_movetab[0], &sing_badcapmovetab[0], sing_badcapsize);
				std::sort(sing_movetab, sing_movetab+size);
  			break;
			}
		} // end switch

	for (int i = 0; i < sing_size; i++)
	{
		sing_tempmove.tabentry = sing_movetab[i].tabentry;
		if (sing_tempmove.table.move == this_pv_move)
			 continue;	// skip hashmove
		if (board.makemove(sing_tempmove.table, 1) < 0)
			 continue; // skip illegal move
		//sing_val = -quiesCheck<0>(-sing_beta, -(sing_beta-1), 0, qcheck_depth);
		sing_val = -AlphaBeta(board, 1-sing_beta, depth/2, NULL_NO);
#ifdef PRTBOARD	
		com2char(charmove, sing_tempmove.from, sing_tempmove.dest );
  printf("makemove %s sing_val=%d\n", charmove, sing_val);
  fflush(stdout);
#endif
		board.unmakemove();
		if (sing_val > sing_best)
		{
			sing_best = sing_val;
			if (sing_best >= sing_beta)
				//break;
				goto end_singnonpv;
		}
	}	//next i

} // next sing_phase

	if (sing_best != -INF && sing_best < sing_beta)
	{
//cap_singnonpv:
#ifdef PRTBOARD	
	printf("info TT-singular extension at nonPV, sing_best=%d, sing_beta=%d, newdepth=%d, depth=%d, hBetaDepth=%d\n", sing_best, sing_beta, newdepth, depth, hsho->hBetaDepth);
	fflush(stdout);
#endif	
//	nonpv_singular_cnt ++;
	newdepth = depth; //TT singleext 1 board.ply
  }
}
end_singnonpv:
#endif


/*
            	// recapture
                if (capture  && node_type==NodePV &&
                    CheckExt==0 &&
                   m_hisindex >=1 &&
                	 (m_hisrecord[m_hisindex-1].htab.capture && m_hisrecord[m_hisindex-1].htab.dest==tempmove.dest)
								//&& (capture>>2) == (m_hisrecord[m_hisindex-2].capture >>2)
                  	)
                  	newdepth=depth;
*/

if (phase==0) //<=1)	// noncap + badcap
{
        //evalthreat[0]=evalthreat[1]=0;
        if ( depth <= histcut_depth //7
              //&& depth>= HistoryDepth //2 //3
        && incheck == 0
        //&& node_type != NodePV
        && newdepth < depth
        && board.piece[tempmove.dest]==0
        && piecefrom < B_KING
                 // not moves threatening the King area
		//&& ((piecefrom < B_HORSE1 || piecefrom > R_ROOK2)
    && (kingattk_incl_horse[kingindex[board.boardsq[33-board.m_side]]][tempmove.dest] ==0)
    )
    {
          if (depth>= 3 //HistoryDepth=3
            	&& gen_count >= depth + 1 // + 1
            	&& (depth * hisvptr->HistHit) < hisvptr->HistTot

#ifdef ATTHREAT
						//20230121
           	//&& board.Eval_attack_threat(oppside(board.m_side))==0	//not in threat
            //  && (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15
            //20230127  	&& (board.p_endgame || (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15 )
              	                            && ( (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15 )	
#endif
/*
            	&& (( (bitpiece & (0x00005000<<board.m_side)) == (0x00005000<<board.m_side))  // if own ADVISORs ==2
|| (	(bitpiece & (0xA0000000>>board.m_side)) != (0xA0000000>>board.m_side)))  // if opp rooks != 2
*/
//     				&& (bitCount((bitattk[board.m_side] | bitattk[2+board.m_side] | bitattk[4+board.m_side] ) & (0xAAA00AA8 >>board.m_side)) <3) //<4)


           )
            	continue;



        //futility pruning
        // Check to see if this node might be futile.
        // This is true if;
        // (1) We are not in check, nor is this a checking move
        // (3) We are not capturing or promoting on this move
        // (5) We are not on the first two board.ply (board.ply>1), so that the search has had a chance to go bad.
        // If this is well below alpha then prune by a whole board.ply


#ifdef FUTILITY
        if ( depth<= fut_depth //3 //2  //2
        	&& alpha > -WIN_VALUE && beta < WIN_VALUE
//        	&& (board.m_side ? -pointsum : pointsum) < beta
           )
        {
                    // Move count based pruning
//- at depth 1, prune once you have searched 7 moves
//- at depth 2, prune once you have searched 11 moves
//- at depth 3, prune once you have searched 23 moves
//- at depth 4, prune once you have searched 35 moves
// not good 86m - 15.5
//        if (gen_count >= depth * 8)		// 7, 15, 23 moves
//        	continue;

            //fprintf(stderr,"%s%d %d %d \n","futility pruning, depth, newdepth, board.ply=",depth,newdepth,ply);
            //		newdepth --;
            //

            // optimistic evaluation

            if (opt_value == INF)
            {                
                //opt_value = Evalscore() + (depth*2-1)*FUTPAWN; // x1, x3, x5
                if (depth > 3)
                {
                	evalscore = Evalscore(board);
                	futpawn_x2depth_1 = (depth*2)*FUTPAWN; //(depth*2-1)*FUTPAWN;
                }	
                opt_value = evalscore + futpawn_x2depth_1; //(depth*2-1)*FUTPAWN;
            }

            //val = opt_value;

            // pruning
            if ( opt_value <= alpha
            //stockfish 1.62
            //if (opt_value < beta
            //	&& alpha > -WIN_VALUE && beta < WIN_VALUE

//              && evalthreat[1] < THREAT_MARGIN  //96 //== 0
//              && evalthreat[0] < THREAT_MARGIN //96 //== 0
#ifdef ATTHREAT
						//20230121
            //20230127  	&& (board.p_endgame || (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15 )
              		&& ( (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15 )
#endif
//            	&& (board.p_endgame || (board.m_side ? board.Evalattk_R() : board.Evalattk_B()) <15 )
            	//&& board.Evalattk_B()<15
            	//&& board.Evalattk_R()<15

//              && board.Eval_attack_threat(0)==0	//not in threat
//              && board.Eval_attack_threat(1)==0	//not threating moves

//            	&& board.Eval_attack_threat(oppside(board.m_side))==0	//not in threat

//              && mate_threat==0
/*
                                	&& (( (bitpiece & (0x00005000<<board.m_side)) == (0x00005000<<board.m_side))  // if own ADVISORs ==2
|| (	(bitpiece & (0xA0000000>>board.m_side)) != (0xA0000000>>board.m_side)))  // if opp rooks != 2
*/
//&& (bitCount((bitattk[board.m_side] | bitattk[2+board.m_side] | bitattk[4+board.m_side] ) & (0xAAA00AA8 >>board.m_side)) <4)
//     				&& (bitCount((bitattk[board.m_side] | bitattk[2+board.m_side] | bitattk[4+board.m_side] ) & (0xAAA00AA8 >>board.m_side))
//              + bitCountLSB((( bitattk[5-board.m_side] ) & (0x0A000000 >>board.m_side)) >>24)
//               <3) //<4)
               )
            {
#ifdef DEBUG
                if (depth >1) nExtFutility++;
                else nFutility++;
#endif
                //
                //if (board.p_endgame==0)
                //if (depth<=2)

                    //if (val > best)
                    if (opt_value > best)
                    {
                        //best = val;
                        best = opt_value;
                        //mvBest.move = tempmove.move;
                        
                    }
                    //unmakemove(); //tempmove);
                    continue;
            }
        }

        //
#endif
}
		}	//end if phase<=1

       // capture = makemove(tempmove.table,
       //  (incheck
			 //		|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
			//		|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
		//);
        if (incheck
					|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
					|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
        		capture = board.makemove(tempmove.table, 1);
        else
		        capture = board.makemove(tempmove.table, 0);


        if (capture < 0)
        	continue;

        if (capture==0)
        {
        	movesSearched[noncap_gen_count].table.move = tempmove.table.move;
        	movesSearched[noncap_gen_count].tabval = piecefrom;
        	noncap_gen_count++;
        }
        gen_count++;




        int reduced=0;
if (phase==0) //<=1)
{

#ifdef HISTPRUN
		if (depth>= HistoryDepth && !incheck && newdepth < depth && gen_count >= 3 //3 //HistoryMoveNb
			&& (capture==0)
			&& board.m_hisrecord[board.m_hisindex-1].htab.Chk==0
			 )
				{
#ifdef PVRED
						reduced=nonpvred(depth, gen_count);
						newdepth -= reduced;
#else

						//if ((board.EvalSide<0>() < THREAT_MARGIN) //== 0
      		 	//	 && (board.EvalSide<1>() < THREAT_MARGIN) //== 0
					 //)
      		 	{
						//if (gen_count >= 11 && depth >= 7)  //NonPV LMR 86L
						if (gen_count >= 11 && depth >= 6)  //NonPV LMR 86m
						{	reduced=2;
					  	newdepth -= 2;
						}
						else
						{	reduced=1;
							newdepth --;
						}
						//newdepth -= reduced;
					}
#endif


				}


#endif
} // end if phase<=1

            if (newdepth <= 0)
                val = -quiesCheck(board, 1-beta, newdepth, qcheck_depth);
            else
            {                
                val = -AlphaBeta(board, 1-beta, newdepth, NULL_YES); //for nonpv, alpha=beta-1
                        // history-pruning re-search
#ifdef HISTPRUN
        if (reduced && val >=beta)	//was >alpha
        {
            	newdepth+=reduced;                
                val = -AlphaBeta(board, 1-beta, newdepth, NULL_YES);

        }
#endif

						}

        board.unmakemove();

        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;



                    // 12. Alpha-Beta邊界判定
        //  val >= best >= val >= best2nd >= val
        if (val <= best)
        {
        }
        else
        //if (val > best)
        {
            best = val;
            if (val >= beta)
            {
                RecordHash(HASH_BETA, depth, val, tempmove.table.move, board); //, nSingular);
                if (capture==0  //update killer and history for non-captures
                    && phase==0 //<=1
                    && !incheck
                    && !board.m_timeout //sf191
                   )
                {
                    if (val >= WIN_VALUE)
                        g_matekiller[board.thd_id][board.ply].move = tempmove.table.move;
                    else
                    {
                        if (g_killer[board.thd_id][0][board.ply].move != tempmove.table.move)
                        {

                            g_killer[board.thd_id][1][board.ply].move = g_killer[board.thd_id][0][board.ply].move;
                            g_killer[board.thd_id][0][board.ply].move = tempmove.table.move;

                        }
                    }
                    update_HistVal(board.thd_id, hisvptr, depth);
                    update_history(board.thd_id, hisvptr, depth, movesSearched, noncap_gen_count);
                }

                return val;
            }

            if (val > alpha)
            {
//critter
/*
                //RecordHash(HASH_ALPHA, depth, val, tempmove.table.move);
            	  if (capture==0  //update history for non-captures
                    && phase==0 //<=1
                    && !incheck
                    && !board.m_timeout  //sf191
                   )
                {
            	  update_HistVal(hisvptr, depth);
                update_history(board.thd_id, hisvptr, depth, movesSearched, noncap_gen_count);
              } 
*/                         	
                //if (PvNode && value < beta) // We want always alpha < beta  //sf191
                // alpha = val; //sf191 not update alpha in nonPV
                mvBest.move = tempmove.table.move;
                //update_pv(sstack, board.ply, mvBest.move); //tempmove.table.move);
                //mvBest = mv;
                nHashFlag = HASH_PV;
            }
        }

        //no longer diff NodeCut and NodeAll
        //if (node_type == NodeCut) node_type = NodeAll;

        } // end for i
    }	// end for phase

//-----------------------------------------------

    if (gen_count==0)
    //if (best == -INF)
    {	//bestmove = 0;
        return board.ply-INF;	// No moves in this pos, lost by checkmate or stalemate
    }
    // 13. 更新置換表、歷史表和殺手著法表
//cut:
    if (board.m_timeout) //sf191
    	return best;

    if (mvBest.move)
    {
        RecordHash(nHashFlag, depth, best, mvBest.move, board); //, 0);
        //  stockfish 1.3 does not update killer and hist
        /*
        if (capture==0 && node_type==NodePV && !incheck)
        {
            if (best >= WIN_VALUE)
                g_matekiller[board.thd_id].move = mvBest.move;
            else
            {
                if (g_killer[board.thd_id][0].move != mvBest.move)
                {
                    g_killer[board.thd_id][1].move = g_killer[board.thd_id][0].move;
                    g_killer[board.thd_id][0].move = mvBest.move;
                    //hisvptr = &(m_his_table[mvBest.dest][PIECE_IDX16(board.piece[mvBest.from])]);
                    hisvptr = &(m_his_table[PIECE_IDX(board.piece[mvBest.from])][nFile(mvBest.dest)][nRank(mvBest.dest)]);
                    update_HistVal(hisvptr, depth);
                }
            }
            //hisvptr = &(m_his_table[mvBest.dest][PIECE_IDX16(board.piece[mvBest.from])]);
            //update_HistVal(hisvptr, depth);
        }
       */
    }
    else if (best <= old_alpha)
    	RecordHash(nHashFlag, depth, best, mvBest.move, board); //, 0);

    return best; //thisalpha;

}

//template <int PVNode>
int Engine::quiesCheckPV(Board &board, int alpha, int beta, int qdepth, int check_depth)
{
    int val, incheck;
    int best, capture, bestmove; //hashflag,  //, kingidx;

		// 統計寂靜搜索的次數

#ifdef DEBUG
    nQuiescNodes++;
#endif
#ifdef QCHECK
//		QNodes++;

#endif


// sf191 not poll input in qsearch
	//if (board.m_timeout) //stop
  //      	return UNKNOWN_VALUE;

  //board.m_nodes++;



// 6. 中斷調用﹔
        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;
        board.m_nodes++;
      //m_time_check++;
        //if ((m_time_check &8191)==0)  //4095)==0)
/* 20230316 quiescPV not interupt
        if ((board.m_nodes      & PollNodes)==0) //4095)==0) //8191)==0)  //4095)==0)
         {
            //if (board.IMaxTime <4000)            
          {
            BusyComm = BusyLine(UcciComm, false);
            if (BusyComm == UCCI_COMM_STOP)
            //if (BusyLine(UcciComm, false) == UCCI_COMM_STOP )
            {
            	// "stop"指令發送中止信號
            board.m_timeout= 1;	//stop
            //printBestmove(board.m_bestmove);
#ifdef THDINFO            
            printf("     ***quiesCheckPV stopped by GUI pv\n");
            fflush(stdout);
#endif            
            	//return (board.m_side ? -INF-1 : INF+1);
            	return UNKNOWN_VALUE;
            	//longjmp(setjmpbuf,1);
            }

            else if (BusyComm == UCCI_COMM_QUIT)
            {
            	p_feedback_move = -1; //pass to Eychessu.cpp to quit the engine
            	//printf("info UCCI_COMM_QUIT p_feedback_move = %d\n", p_feedback_move);
			        //fflush(stdout);
            }
          }
            //if ((clock()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            //if ((GetTime()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);

    				if (t_remain <= 0) 
            {


//    printf("info Search timout %d board.ply %d board.IMaxTime %d\n", (int)(GetTime() - board.m_startime), board.ply, board.IMaxTime);

            	    board.m_timeout= 1; //stop
            	    //printBestmove(board.m_bestmove);
#ifdef THDINFO            	    
            	    printf("     ***quiescePV timeout\n");
                  fflush(stdout);
#endif                  
                    return UNKNOWN_VALUE;
                    //longjmp(setjmpbuf,1);
            }
            if (t_remain < 4000)    				
    					PollNodes = 1023;
          
        }
*/
// 2. 和棋裁剪﹔
    //if (pos.IsDraw())
//20230316    if ((board.bitpiece & 0xfff00ffc) == 0) return 0;

    // 1. Return if there is a mate line bad enough
    if (board.ply-INF >= beta)
    	return (board.ply-INF);

    val=board.checkloop(1);
    if (val)
    	return val;

incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;
//if (incheck) printf("m_hisindex-1=%d, incheck=%d\n", m_hisindex-1, incheck);
//bool nonPV = (beta - alpha == 1);
#ifdef QHASH
int    hashmove = 0;

    // Transposition table lookup, only when not in PV
    //if ( (qdepth >=check_depth) ) // && !incheck)   //sf191 88i
    //{
    	//if (PVNode)
        hashmove = ProbeMoveQ(board);
      /*  
      else
    	{
        val = ProbeHashQ(beta, hashmove);  //0
        if (val != UNKNOWN_VALUE)
        {
        	    //printf("probehashQ val=%d\n", val);
        	    return val;
        }
      }
      */
    //}

#endif

    int old_alpha = alpha;
//    int opt_value;
    best = -INF;
    //int ncapsize;
    MoveTabStruct movetab[111]; //, ncapmovetab[64];
    MoveTabStruct tempmove;
/*
#ifdef DELTAN
    if (!PVNode)
    opt_value = INF;  // for delta pruning

#endif
*/
    //incheck=m_hisrecord[m_hisindex-1].htab.Chk;
    if (!incheck)
    {
        //val=Evalscore(alpha, beta);
        val=Evalscore(board);
// If this is too deep then just return the stand-pat score to avoid long
        // quiescence searches.
        if (board.ply>= MAXEXT) //MAXPLY - 4) //60) //30 //60) // > 17)  //qdepth > 60)
        {
            //val = board.Eval();
            return val;
        }

        if (val >= beta)
        {
            if (hashmove==0)  //90i
            //RecordHashQ(HASH_BETA, val, 0); //sf191 88e
            RecordHash(HASH_BETA, 0, val, 0, board);
            return val; //beta;
        }
        if (val > best)
        {
            best = val;            
            //if (PVNode && val > alpha) //sf191 //88e
            if (val > alpha) //sf191 //88e	
                alpha = val;
        }

/*
#ifdef DELTAN
        if (!PVNode)
        opt_value = val + DELTA_MARGIN; //25; //25; //25; //25; //40;  //25; //25; //40; //25; // DeltaMargin 50 centipawn, eyc=40
#endif
*/
        //num = (board.m_side ? board.GenCapQS<1>(&movetab[0]) : board.GenCapQS<0>(&movetab[0]));
        //num=board.GenCapQS(&movetab[0]);
        //num=board.GenCapQS(&movetab[0], &ncapmovetab[0], ncapsize);
        //kingidx=kingindex[board.boardsq[32+board.m_side]];
    }
    else
    {
    	 // If this is too deep then just return the stand-pat score to avoid long
        // quiescence searches.
        if (board.ply>= MAXEXT) //MAXPLY - 4) //60) //30 //60) // > 17)  //qdepth > 60)
        {
            //val = board.Eval();
            //return (board.m_side ? -pointsum : pointsum);
            return Evalscore(board);
        }
    } // end of if(!incheck)



bestmove=0;
int num=0;
//int gen_count=0;
#ifdef QPH
for (int qphase=3; qphase--;)	// 2=hashmove, 1=capture, 0=genchk
#else
for (int qphase=2; qphase--;)	// 1=capture, 0=genchk
#endif
{
    switch (qphase)
    {
    	case 0:	//genchk
    		if (incheck==0)
    		{
        	if (qdepth >=check_depth)
            //&& alpha < beta
            num=board.generate_checks(&movetab[0]);
        	else
            num=0;
        }
    		else
    		{
        		num=board.GenChkEvasNCap(&movetab[0], incheck);
    		}
    		break;
    	case 1:	//capture
    		if (incheck==0)
    		{
    				num = (board.m_side ? board.GenCapQS<1>(&movetab[0]) : board.GenCapQS<0>(&movetab[0]));
    	  }
    	  else
    	  {
    	  	  num=board.GenChkEvasCap(&movetab[0], incheck);

        //single-reply extension
        //if (PVNode && board.ply < MAXEXT) //MAXPLY - 4) //32)
        if (board.ply < MAXEXT)	
        {
        if (num <= 1)
        	check_depth -= 2;
        else
        {
        	int gcount = 0;
        	for (int j=0; j<num; j++)
        	{
            //if (makemovechk(movetab[j].table) ==0)
            if (board.makemovenoeval(movetab[j].table, 1) >=0) //, 1) >=0)
            {
                board.unmakemovenoeval();
                gcount++;
                if (gcount > 1)
                    break;
            }
        	}
        	if (gcount <= 1)
        	{
            //single_ext=1;
						check_depth -= 2;
        	}
       	}
      }
    	  }
    		break;
#ifdef QPH
    	case 2:	//hashmove
    		if (hashmove==0)
    			num=0;
    		else
    			num=1;
    		break;
#endif
    }

#ifdef QPH
#else
    if (hashmove)
    {
        for (int j=0; j<num; j++)
        {
            if (movetab[j].table.move == hashmove)
            {
                movetab[j].tabval = BIGVAL;
                break;
            }
        }
    }
#endif
    //Quicksort(0, num-1, movetab); //table, tabval);
    //Insertsort(num, movetab);

    for (int i=0;i<num;++i)
    {
      switch (qphase)
    {
    	case 0:	//genchk
      	tempmove.tabentry = GetNextMove(i, num, movetab);
#ifdef QPH
      	if (tempmove.table.move==hashmove)
        		continue;
#endif
      break;

      case 1:	//capture
      	tempmove.tabentry = GetNextMove(i, num, movetab);
#ifdef QPH
      	if (tempmove.table.move==hashmove)
        		continue;
#endif
/*
      if (incheck==0 	//delta pruning for capture phase only
      //&& nonPV
      //&& tempmove.table.move != hashmove
      )
      {
      	int piecedest = board.piece[tempmove.dest];
        //
// delta pruning
#ifdef DELTAN
//if (nonPV) //(beta==old_alpha+1)
if (!PVNode)
{
						val = opt_value +	abs(pointtable[PIECE_IDX(piecedest)][nRank(tempmove.dest)][nFile(tempmove.dest)]);
						//MAX(abs(pointtable[tempmove.dest][PIECE_IDX(piecedest)][0]),abs(pointtable[tempmove.dest][PIECE_IDX(piecedest)][1]));
						//if (val <= alpha)
						if (val < alpha)		//sf191 //88e
						{
							if (val > best)
							{
									best = val;

    					}
							continue;
						}
}
#endif
        if (!PVNode) //sf191 88L
        if (piecedest < B_HORSE1
        	//|| (board.piece[tempmove.from]>=B_ROOK1
        	|| ((board.piece[tempmove.from]>>2)==ROOK  //exclude king capture for SEE()
        	&& piecedest < B_ROOK1))
        {
        	if (board.see(tempmove.from, tempmove.dest) < 0)
        				continue;
        }       
        
			}	// end if incheck==0
*/

		  break;
#ifdef QPH
      case 2:	//hashmove
      	tempmove.table.move = hashmove;
      	//if (tempmove.table.move==0
        //   ||
        if (board.LegalKiller(tempmove.table)==0
           )
             continue;
      break;
#endif
     } // end switch qphase

        capture = board.makemove(tempmove.table,
        	 (incheck
						|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
						|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
						);
				/*
				if (incheck
					|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
					|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
        		capture = board.makemove(tempmove.table, 1);
        else
		        capture = board.makemove(tempmove.table, 0);
        */
				if (capture < 0)
        	continue;
				//gen_count++;

        //if (capture >=B_KING)
        //	val = INF-ply;
        //else
        val = -quiesCheckPV(board, -beta, -alpha, qdepth-1, check_depth);
        board.unmakemove();

        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;

        if (val >= beta)
        {

#ifdef QHASH

					//if ((	qdepth >=check_depth) && !incheck)
            //RecordHashQ(HASH_BETA, val, tempmove.table.move);
            RecordHash(HASH_BETA, 0, val, tempmove.table.move, board);
#endif
            //printf("beta cutoff val=%d\n", val);
            return val; // beta;
        }
        if (val > best)
        {
            best = val;
            if (val > alpha)
            {
                alpha = val;
                bestmove = tempmove.table.move;
            }
        }

    }	// end for i (< num)

} // end for qphase


// 8. 處理
    if ((best == -INF))
//			if (gen_count==0 && incheck)	//mate!
			{
        //printf("ply-INF val=%d\n", board.ply-INF);
        return board.ply - INF;
    	}

#ifdef QHASH

    //sf191 if ((qdepth >=check_depth) ) //&& !incheck)
    if (qdepth >=check_depth)
    {

        //int hashflag; //, hashmv;
        //if (alpha == old_alpha)
        if (best <= old_alpha) //sf191	//88e
        {
            //RecordHashQ(HASH_ALPHA, best, 0);
            RecordHash(HASH_ALPHA, 0, best, 0, board);
            //hashflag = HASH_ALPHA;
            //bestmove = 0;
        }
        else
        {
            //hashflag = HASH_PV;
            //hashmove = ss->pv[ply];
            //RecordHashQ(HASH_PV, best, bestmove, board);
            RecordHash(HASH_PV, 0, best, bestmove, board);
        }

//RecHashQ:
        //RecordHashQ(hashflag, best, bestmove, board);
				//RecordHash(hashflag, 0, best, bestmove, board);

    }
#endif

    // 8. 處理
    //if (best == -INF) {
    //    return board.ply - INF;
    //}
    //printf("Q best val=%d\n", best);
    return best;
}
int Engine::quiesCheck(Board &board, int beta,  int qdepth, int check_depth)
{
    int val, incheck;
    int best, capture, bestmove; //hashflag,  //, kingidx;

		// 統計寂靜搜索的次數

#ifdef DEBUG
    nQuiescNodes++;
#endif
#ifdef QCHECK
//		QNodes++;

#endif

// sf191 not poll input in qsearch
/*
	if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;
*/
//  board.m_nodes++;


// 6. 中斷調用﹔
        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;
        board.m_nodes++;
/* 20230316 quiesc not interupt
      //m_time_check++;
        //if ((m_time_check &8191)==0)  //4095)==0)

        if ((board.m_nodes      & PollNodes)==0) //4095)==0) //8191)==0)  //4095)==0)
         {
            //if (board.IMaxTime <4000)
          {
            BusyComm = BusyLine(UcciComm, false);
            if (BusyComm == UCCI_COMM_STOP)
            //if (BusyLine(UcciComm, false) == UCCI_COMM_STOP )
            {
            	// "stop"指令發送中止信號
            board.m_timeout= 1;	//stop
            //printBestmove(board.m_bestmove);
            printf("info quiesCheck stopped by GUI pv\n");
            fflush(stdout);
            	//return (board.m_side ? -INF-1 : INF+1);
            	return UNKNOWN_VALUE;
            	//longjmp(setjmpbuf,1);
            }

            else if (BusyComm == UCCI_COMM_QUIT)
            {
            	p_feedback_move = -1; //pass to Eychessu.cpp to quit the engine
            	//printf("info UCCI_COMM_QUIT p_feedback_move = %d\n", p_feedback_move);
			        //fflush(stdout);
            }
          }
            //if ((clock()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            //if ((GetTime()-board.m_startime>=board.IMaxTime))  //m_depth>=14 &&
            long long t_remain = board.IMaxTime - (GetTime()-board.m_startime);

    				if (t_remain <= 0)
            {


//    printf("info Search timout %d board.ply %d board.IMaxTime %d\n", (int)(GetTime() - board.m_startime), board.ply, board.IMaxTime);

            	    board.m_timeout= 1; //stop
            	    //printBestmove(board.m_bestmove);
            	    printf("info quiesce timeout\n");
                  fflush(stdout);
                    return UNKNOWN_VALUE;
                    //longjmp(setjmpbuf,1);
            }
            if (t_remain < 4000)    				
    					PollNodes = 1023;
          
        }
*/

// 2. 和棋裁剪﹔
    //if (pos.IsDraw())
//20230316    if ((board.bitpiece & 0xfff00ffc) == 0) return 0;

    // 1. Return if there is a mate line bad enough
    if (board.ply-INF >= beta)
    	return (board.ply-INF);

    val=board.checkloop(1);
    if (val)
    	return val;

incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;
//if (incheck) printf("m_hisindex-1=%d, incheck=%d\n", m_hisindex-1, incheck);
//bool nonPV = (beta - alpha == 1);
#ifdef QHASH
int    hashmove = 0;

    // Transposition table lookup, only when not in PV
    //if ( (qdepth >=check_depth) ) // && !incheck)   //sf191 88i
    //{
    	/*
    	if (PVNode)
        hashmove = ProbeMoveQ(board);
      else
    	{
    	*/
        val = ProbeHashQ(beta, hashmove, board);  //0
        if (val != UNKNOWN_VALUE)
        {
        	    //printf("probehashQ val=%d\n", val);
        	    return val;
        }
      //}

    //}

#endif

    int old_alpha = beta - 1; //alpha;
    int opt_value;
    best = -INF;
    //int ncapsize;
    MoveTabStruct movetab[111]; //, ncapmovetab[64];
    MoveTabStruct tempmove;

#ifdef DELTAN
    //if (!PVNode)
    opt_value = INF;  // for delta pruning

#endif

    //incheck=m_hisrecord[m_hisindex-1].htab.Chk;
    if (!incheck)
    {
        //val=Evalscore(alpha);
        val=Evalscore(board);
// If this is too deep then just return the stand-pat score to avoid long
        // quiescence searches.
        if (board.ply>= MAXEXT) //MAXPLY - 4) //60) //30 //60) // > 17)  //qdepth > 60)
        {
            //val = board.Eval();
            return val;
        }

        if (val >= beta)
        {
            if (hashmove==0)  //
            //RecordHashQ(HASH_BETA, val, 0); //sf191 88e
            RecordHash(HASH_BETA, 0, val, 0, board);
            return val; //beta;
        }
        if (val > best)
        {
            best = val;
            //if (val > alpha)
            /*
            if (PVNode && val > alpha) //sf191 //88e
                alpha = val;
            */    
        }


#ifdef DELTAN
        //if (!PVNode)
        opt_value = val + DELTA_MARGIN; //25; //25; //25; //25; //40;  //25; //25; //40; //25; // DeltaMargin 50 centipawn, eyc=40
#endif

        //num = (board.m_side ? board.GenCapQS<1>(&movetab[0]) : board.GenCapQS<0>(&movetab[0]));
        //num=board.GenCapQS(&movetab[0]);
        //num=board.GenCapQS(&movetab[0], &ncapmovetab[0], ncapsize);
        //kingidx=kingindex[board.boardsq[32+board.m_side]];
    }
    else
    {
    	// If this is too deep then just return the stand-pat score to avoid long quiescence searches
        if (board.ply>= MAXEXT) //MAXPLY - 4) //60) //30 //60) // > 17)  //qdepth > 60)
        {
            //val = board.Eval();
            //return (board.m_side ? -pointsum : pointsum);
            return Evalscore(board);
        }
    } // end of if(!incheck)



bestmove=0;
int num=0;
//int gen_count=0;
#ifdef QPH
for (int qphase=3; qphase--;)	// 2=hashmove, 1=capture, 0=genchk
#else
for (int qphase=2; qphase--;)	// 1=capture, 0=genchk
#endif
{
    switch (qphase)
    {
    	case 0:	//genchk
    		if (incheck==0)
    		{
        	if (qdepth >=check_depth)
            //&& alpha < beta
            num=board.generate_checks(&movetab[0]);
        	else
            num=0;
        }
    		else
    		{
        		num=board.GenChkEvasNCap(&movetab[0], incheck);
    		}
    		break;
    	case 1:	//capture
    		if (incheck==0)
    		{
    				num = (board.m_side ? board.GenCapQS<1>(&movetab[0]) : board.GenCapQS<0>(&movetab[0]));
    	  }
    	  else
    	  {
    	  	  num=board.GenChkEvasCap(&movetab[0], incheck);
/*
        //single-reply extension
        if (PVNode && board.ply < MAXEXT) //MAXPLY - 4) //32)
        {
        if (num <= 1)
        	check_depth -= 2;
        else
        {
        	int gcount = 0;
        	for (int j=0; j<num; j++)
        	{
            //if (makemovechk(movetab[j].table) ==0)
            if (board.makemovenoeval(movetab[j].table, 1) >=0) //, 1) >=0)
            {
                board.unmakemovenoeval();
                gcount++;
                if (gcount > 1)
                    break;
            }
        	}
        	if (gcount <= 1)
        	{
            //single_ext=1;
						check_depth -= 2;
        	}
       	}
      }
*/      
    	  }
    		break;
#ifdef QPH
    	case 2:	//hashmove
    		if (hashmove==0)
    			num=0;
    		else
    			num=1;
    		break;
#endif
    }

#ifdef QPH
#else
    if (hashmove)
    {
        for (int j=0; j<num; j++)
        {
            if (movetab[j].table.move == hashmove)
            {
                movetab[j].tabval = BIGVAL;
                break;
            }
        }
    }
#endif
    //Quicksort(0, num-1, movetab); //table, tabval);
    //Insertsort(num, movetab);

    for (int i=0;i<num;++i)
    {
      switch (qphase)
    {
    	case 0:	//genchk
      	tempmove.tabentry = GetNextMove(i, num, movetab);
#ifdef QPH
      	if (tempmove.table.move==hashmove)
        		continue;
#endif
      break;

      case 1:	//capture
      	tempmove.tabentry = GetNextMove(i, num, movetab);
#ifdef QPH
      	if (tempmove.table.move==hashmove)
        		continue;
#endif
      if (incheck==0 	//delta pruning for capture phase only
      //&& nonPV
      //&& tempmove.table.move != hashmove
      )
      {
      	int piecedest = board.piece[tempmove.dest];
        //
// delta pruning
#ifdef DELTAN
//if (nonPV) //(beta==old_alpha+1)
//if (!PVNode)
{           //20230209
						val = opt_value +	abs(pointtable[ext_p_endgame][PIECE_IDX(piecedest)][nRank(tempmove.dest)][nFile(tempmove.dest)]);
						//MAX(abs(pointtable[tempmove.dest][PIECE_IDX(piecedest)][0]),abs(pointtable[tempmove.dest][PIECE_IDX(piecedest)][1]));
						//if (val <= alpha)
						if (val < beta-1) //alpha)		//sf191 //88e
						{
							if (val > best)
							{
									best = val;

    					}
							continue;
						}
}
#endif
        //if (!PVNode) //sf191 88L
        if (piecedest < B_HORSE1
        	//|| (board.piece[tempmove.from]>=B_ROOK1
        	|| ((board.piece[tempmove.from]>>2)==ROOK  //exclude king capture for SEE()
        	&& piecedest < B_ROOK1))
        {
        	if (board.see(tempmove.from, tempmove.dest) < 0)
        				continue;
        }
			}	// end if incheck==0


		  break;
#ifdef QPH
      case 2:	//hashmove
      	tempmove.table.move = hashmove;
      	//if (tempmove.table.move==0
        //   ||
        if (board.LegalKiller(tempmove.table)==0
           )
             continue;
      break;
#endif
     } // end switch qphase

        capture = board.makemove(tempmove.table,
        	 (incheck
						|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
						|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
						);
				/*
				if (incheck
					|| (kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.dest]
					|| kingattk_incl_horse[kingindex[board.boardsq[32+board.m_side]]][tempmove.from]))
        		capture = board.makemove(tempmove.table, 1);
        else
		        capture = board.makemove(tempmove.table, 0);
        */
				if (capture < 0)
        	continue;
				//gen_count++;

        //if (capture >=B_KING)
        //	val = INF-ply;
        //else
        //val = -quiesCheck(-beta, -alpha, qdepth-1, check_depth);
        val = -quiesCheck(board,   1-beta, qdepth-1, check_depth);
        board.unmakemove();

        if (board.m_timeout) //stop
        	return UNKNOWN_VALUE;

        if (val >= beta)
        {

#ifdef QHASH

					//if ((	qdepth >=check_depth) && !incheck)
            //RecordHashQ(HASH_BETA, val, tempmove.table.move, board);
            RecordHash(HASH_BETA, 0, val, tempmove.table.move, board);
#endif
            //printf("beta cutoff val=%d\n", val);
            return val; // beta;
        }
        if (val > best)
        {
            best = val;
            if (val > beta-1) //alpha)
            {
                //alpha = val;
				//beta = val + 1;
                bestmove = tempmove.table.move;
            }
        }

    }	// end for i (< num)

} // end for qphase


// 8. 處理
    if ((best == -INF))
//			if (gen_count==0 && incheck)	//mate!
			{
        //printf("ply-INF val=%d\n", board.ply-INF);
        return board.ply - INF;
    	}

#ifdef QHASH

    //sf191 if ((qdepth >=check_depth) ) //&& !incheck)
    if (qdepth >=check_depth)
    {

        //int hashflag; //, hashmv;
        //if (alpha == old_alpha)
        if (best <= old_alpha) //sf191	//88e
        {
            //RecordHashQ(HASH_ALPHA, best, 0);
            RecordHash(HASH_ALPHA, 0, best, 0, board);
            //hashflag = HASH_ALPHA;
            //bestmove = 0;
        }
        else
        {
            //hashflag = HASH_PV;
            //hashmove = ss->pv[ply];
            //RecordHashQ(HASH_PV, best, bestmove);
            RecordHash(HASH_PV, 0, best, bestmove, board);
        }

//RecHashQ:
        //RecordHashQ(hashflag, best, bestmove);
				//RecordHash(hashflag, 0, best, bestmove);

    }
#endif

    // 8. 處理
    //if (best == -INF) {
    //    return board.ply - INF;
    //}
    //printf("Q best val=%d\n", best);
    return best;
}

////////////////////////////////////////////////////////////////////////////
//界面接口
//void  Engine::IMakeMove(int from,int dest)

void  Engine::IMakeMove(Board &board, MoveStruct &moveS)
{
    //makemove(moveS);
    board.makemove(moveS, 0); // , 0);
}

void  Engine::printBestmove(int bestmove)
{
    char str[5];
    MoveStruct tempmove;
    tempmove.move=bestmove;
    int from=tempmove.from;
    int dest=tempmove.dest;
    p_movenum++;
#ifdef THDINFO    
    printf("     **p_movenum= #%d:\n", p_movenum);
    fflush(stdout);
#endif    
    printf("bestmove %s\n",MoveStr(str,from,dest) );
    fflush(stdout);
}

// 主搜索例程 //20230323 init search after Eychessu.cpp : UCCI_GO_TIME_INCREMENT
void  Engine::ISearch()   //20230323 equiv to eleeye search.cpp : SearchMain(int nDepth)
{
    //char str[5];
    MoveStruct tempmove;
    tempmove.move = searchRoot();  //20230323 equiv to eleeye search.cpp : SearchMain(int nDepth) // 6. 做迭代加深搜索
    int from=tempmove.from;
    int dest=tempmove.dest;

    //if (board.m_timeout==0)
    	printBestmove(tempmove.move);
    /*
    p_movenum++;
    printf("info #%d:\n", p_movenum);
    fflush(stdout);
    printf("bestmove %s\n",MoveStr(str,from,dest) );
    fflush(stdout);
    */
		Clear_Killer(); //1891c - same with Clear_Hist
		Clear_Hist();
//    HashAging();
//    ClearHash();
//makemove bestmove
if (board.piece[dest])
    board.boardsq[board.piece[dest]]=SQ_EMPTY;
board.boardsq[board.piece[from]] = dest;
//board.piece[dest]=board.piece[from];
//board.piece[from]=EMPTY;

//memcpy(prev_piece, board.piece, sizeof(prev_piece));
memcpy(prev_boardsq, board.boardsq, sizeof(prev_boardsq));
//    return bestmove;
}
//////////////////////////////////////////////////////////////////////////

/* //20230210 - chg to compile time init since clop optimization not uesed
void Engine::init_parms()
{	//1892q - debug init VPE
	printf("info VPE=%d, VPER=%d, VBE=%d, VEE=%d\n", VPE, VPER, VBE, VEE);	
	
		// re-init BasicValues, etc. will not compile, as you cannot assign a complete array like that in C++
	//BasicValues = {VP,VE,VN,VC,VR, VPE,VEE,VNE,VCE,VRE};
	//20230210
    int values[10] = {VP, VE, VN, VC, VR, VPE, VEE, VNE, VCE, VRE};
    std::copy(std::begin(values), std::end(values), std::begin(BasicValues));


	BasicValues[0]=VP; 
	BasicValues[1]=VE;
	BasicValues[2]=VN;
	BasicValues[3]=VC;
	BasicValues[4]=VR;
	

	BasicValues[5]=VPE;
	BasicValues[6]=VEE;
	BasicValues[7]=VNE;
	BasicValues[8]=VCE;
	BasicValues[9]=VRE;
	//PIECE_VALUE_side =
  //  {0,0,-VP,VP,-VP,VP,-VP,VP,-VP,VP,-VP,VP,-VB,VB,-VB,VB,-VE,VE,-VE,VE,-VNC,VNC,-VNC,VNC,-VNC,VNC,-VNC,VNC,-VR,VR,-VR,VR,-9999,9999,
  //  0,0,-VPR,VPR,-VPR,VPR,-VPR,VPR,-VPR,VPR,-VPR,VPR};

 PIECE_VALUE_side[0][0]=0;
 PIECE_VALUE_side[0][1]=0;
 PIECE_VALUE_side[0][2]= -VP;
 PIECE_VALUE_side[0][3]=VP;
 PIECE_VALUE_side[0][4]= -VP;
 PIECE_VALUE_side[0][5]=VP;
 PIECE_VALUE_side[0][6]= -VP;
 PIECE_VALUE_side[0][7]=VP; 
 PIECE_VALUE_side[0][8]= -VP;
 PIECE_VALUE_side[0][9]=VP;
 PIECE_VALUE_side[0][10]= -VP;
 PIECE_VALUE_side[0][11]=VP;
 
 PIECE_VALUE_side[0][12]= -VB;
 PIECE_VALUE_side[0][13]=VB;
 PIECE_VALUE_side[0][14]= -VB;
 PIECE_VALUE_side[0][15]=VB;
 PIECE_VALUE_side[0][16]= -VE;
 PIECE_VALUE_side[0][17]=VE;
 PIECE_VALUE_side[0][18]= -VE;
 PIECE_VALUE_side[0][19]=VE;


 PIECE_VALUE_side[0][20]= -VNC;
 PIECE_VALUE_side[0][21]=VNC;
 PIECE_VALUE_side[0][22]= -VNC;
 PIECE_VALUE_side[0][23]=VNC;
 PIECE_VALUE_side[0][24]= -VNC;
 PIECE_VALUE_side[0][25]=VNC;
 PIECE_VALUE_side[0][26]= -VNC;
 PIECE_VALUE_side[0][27]=VNC;
 
 PIECE_VALUE_side[0][28]= -VR;
 PIECE_VALUE_side[0][29]=VR;
 PIECE_VALUE_side[0][30]= -VR;
 PIECE_VALUE_side[0][31]=VR;

 PIECE_VALUE_side[0][32]= -9999;
 PIECE_VALUE_side[0][33]=9999;
 PIECE_VALUE_side[0][34]=0;
 PIECE_VALUE_side[0][35]=0;

 PIECE_VALUE_side[0][36]= -VPR;
 PIECE_VALUE_side[0][37]=VPR;
 PIECE_VALUE_side[0][38]= -VPR;
 PIECE_VALUE_side[0][39]=VPR;
 PIECE_VALUE_side[0][40]= -VPR;
 PIECE_VALUE_side[0][41]=VPR; 
 PIECE_VALUE_side[0][42]= -VPR;
 PIECE_VALUE_side[0][43]=VPR;
 PIECE_VALUE_side[0][44]= -VPR;
 PIECE_VALUE_side[0][45]=VPR;
 
 //PIECE_VALUE_side_ENDGAME =
 //   {0,0,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VPE,VPE,-VBE,VBE,-VBE,VBE,-VEE,VEE,-VEE,VEE,-VNCE,VNCE,-VNCE,VNCE,-VNCE,VNCE,-VNCE,VNCE,-VRE,VRE,-VRE,VRE,-9999,9999,
 //   0,0,-VPER,VPER,-VPER,VPER,-VPER,VPER,-VPER,VPER,-VPER,VPER};
 PIECE_VALUE_side[1][0]=0;
 PIECE_VALUE_side[1][1]=0;
 PIECE_VALUE_side[1][2]= -VPE;
 PIECE_VALUE_side[1][3]=VPE;
 PIECE_VALUE_side[1][4]= -VPE;
 PIECE_VALUE_side[1][5]=VPE;
 PIECE_VALUE_side[1][6]= -VPE;
 PIECE_VALUE_side[1][7]=VPE; 
 PIECE_VALUE_side[1][8]= -VPE;
 PIECE_VALUE_side[1][9]=VPE;
 PIECE_VALUE_side[1][10]= -VPE;
 PIECE_VALUE_side[1][11]=VPE;
 
 PIECE_VALUE_side[1][12]= -VBE;
 PIECE_VALUE_side[1][13]=VBE;
 PIECE_VALUE_side[1][14]= -VBE;
 PIECE_VALUE_side[1][15]=VBE;
 PIECE_VALUE_side[1][16]= -VEE;
 PIECE_VALUE_side[1][17]=VEE;
 PIECE_VALUE_side[1][18]= -VEE;
 PIECE_VALUE_side[1][19]=VEE;


 PIECE_VALUE_side[1][20]= -VNCE;
 PIECE_VALUE_side[1][21]=VNCE;
 PIECE_VALUE_side[1][22]= -VNCE;
 PIECE_VALUE_side[1][23]=VNCE;
 PIECE_VALUE_side[1][24]= -VNCE;
 PIECE_VALUE_side[1][25]=VNCE;
 PIECE_VALUE_side[1][26]= -VNCE;
 PIECE_VALUE_side[1][27]=VNCE;
 
 PIECE_VALUE_side[1][28]= -VRE;
 PIECE_VALUE_side[1][29]=VRE;
 PIECE_VALUE_side[1][30]= -VRE;
 PIECE_VALUE_side[1][31]=VRE;

 PIECE_VALUE_side[1][32]= -9999;
 PIECE_VALUE_side[1][33]=9999;
 PIECE_VALUE_side[1][34]=0;
 PIECE_VALUE_side[1][35]=0;

 PIECE_VALUE_side[1][36]= -VPER;
 PIECE_VALUE_side[1][37]=VPER;
 PIECE_VALUE_side[1][38]= -VPER;
 PIECE_VALUE_side[1][39]=VPER;
 PIECE_VALUE_side[1][40]= -VPER;
 PIECE_VALUE_side[1][41]=VPER; 
 PIECE_VALUE_side[1][42]= -VPER;
 PIECE_VALUE_side[1][43]=VPER;
 PIECE_VALUE_side[1][44]= -VPER;
 PIECE_VALUE_side[1][45]=VPER;


for (int i=0; i<10; i++)
	{
		printf("info BasicValues[%d]=%d\n", i, BasicValues[i]);		
	}
for (int i=0; i<46; i++)
	{
		printf("info PIECE_VALUE_side[%d]=%d  PIECE_VALUE_side_ENDGAME[%d]=%d\n", i, PIECE_VALUE_side[i], i, PIECE_VALUE_side_ENDGAME[i]);		
	}	
	fflush(stdout);

}	
*/

//置換表
void Engine::init_hvalue()
{	//printf("info init_hvalue start\n");
    //int i,j;
    //srand((unsigned)time(0));
//    2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15, 16,17,18,19, 20,21,22,23, 24,25,26,27, 28,29,30,31, 32,33
//    p p  p p p p  p p p  p   b  b  b  b   e  e  e  e   n  n  n  n   c  c  c  c   r  r  r  r   k  k
//&011101 (& 0x1D)  ( &29)
//0 1 0 1   4 5 4 5   8 9 8 9    12 13 12 13  16 17 16 17   20 21 20 21   24 25 24 25  28 29 28 29  0 1
    /*
    for (i=0; i<BOARD_SIZE-7; i++)
    //for (j=4; j<16; j++)
    for (j=0; j<10; j++)
    {
    		h_value[i][j]=genrand_int64();
    }
    */
    for (int p=0; p<10; p++)
    for (int i=0; i<9; i++)
    for (int j=0; j<10; j++)
    {
    		h_value[p][i][j]=genrand_int64();
    }
    /*
    // copy pawn h_value
		for (i=0; i<BOARD_SIZE-7; i++)
    for (j=0; j<4; j+=2)
    {
    		h_value[i][j] =h_value[i][4];
        h_value[i][j+1] =h_value[i][5];
    }
    */
    h_rside =genrand_int64();
//What I did was change from
//temp_hashkey = (wtm) ? HashKey : ~HashKey;
//to
//temp_hashkey = (wtm) ? HashKey : HashKey ^ ~hash_mask;
//20230312    uint64_t nHashMask64 = nHashMask;
//20230312    h_rside &= ~nHashMask64;
		h_rside &= ~nHashMask; //20230312 nHaskMask is uint64_t

    //printf("info init_hvalue end\n");
    //for (i=0; i<BOARD_SIZE; i++)
    //{
    //	if ((i&15)==0) printf("\n");
    //		printf("%u ",h_val32[i]);
    //}
}


uint64_t Engine::init_hkey(const Board &board) //, unsigned long &zkey)
{	//printf("info init_hkey start\n");
    uint64_t temp_hkey;
//	zkey = 0;
    //hkey.x1 =0;
    //hkey.x2 =0;
    temp_hkey = 0;


    for (int i=2;i<34;++i)
    {
        int Sq = board.boardsq[i];
        //if (board.boardsq[i]>=0) //* != SQ_EMPTY )
        	if (NOTSQEMPTY(Sq))
            //Xor(hkey,h_value[i][board.boardsq[i]]);
        {
            //board.Xor(temp_hkey,h_value[PIECE_IDX(i)][nFile(i)][nRank(i)]); //[((i&29)+(i&1))/2]);
			temp_hkey ^= h_value[PIECE_IDX(i)][nFile(i)][nRank(i)]; //[((i&29)+(i&1))/2]);
//			zkey  ^= z_value[board.boardsq[i]][i &29];
        }
    }

/*
// try to init hkey in the order of board for consistence between runs
for (int i=0; i<10; i++)
for (int j=0; j<9; j++)
    {

        int p = board.piece[(i*16)+j];
        if (p != EMPTY)
        {
            //Xor(hkey,h_value[Sq][PIECE_IDX(i)]); //[((i&29)+(i&1))/2]);
            Xor(hkey,h_value[PIECE_IDX(p)][i][j]);

        }
    }
*/

//	if(board.m_side!=BLACK)
    //board.Xor(temp_hkey,h_rside);
    temp_hkey ^= h_rside;

    //else
    //	Xor(hkey,h_bside);
//		zkey ^= z_rside;


    //printf("info init_hkey end\n");
    return temp_hkey;
}


// 以下是置換表和開局庫的管理過程

uint64_t nHashMask;
HashStruct *hshItems;
/*
void Engine::HashAging(void) {
  int i;
  for (i = 0; i < nHashMask + 1; i ++) {
//    if (hshItems[i].wFresh > 0) {
//      hshItems[i].wFresh --;
//    }
      if (hshItems[i].hAlphaDepth >=2)
      	hshItems[i].hAlphaDepth -=2;
      if (hshItems[i].hBetaDepth >=2)
      	hshItems[i].hBetaDepth -=2;
  }
}
*/
#ifdef EVALHASH
uint64_t nEvalHashMask;
EvalHash *evalhashItems;
#endif

// 清空置換表 - ClearHash() in Engine.h

// 存儲置換表局面信息
void Engine::RecordHash(int nFlag, int nDepth, int vl, int mv, Board &board) //, int nSingular)
	{
	  //nDepth += p_movenum;
	  //if (nDepth >255) nDepth=255;
	//nDepth = std::min(nDepth+p_movenum, 255);

    HashStruct *hsh;
    int i, nMinDepth, nMinLayer; //nFreshDepth,

    if (vl > INF) return; //do not record UNKNOWN_VALUE
    // 存儲置換表局面信息的過程包括以下幾個步驟：

    // 1. 對分值做殺棋步數調整﹔
    if (vl > WIN_VALUE)
	{
        if (mv == 0 && vl <= BAN_VALUE)
            return; // 導致長將的局面(不進行置換裁剪)如果連最佳著法也沒有，那麼沒有必要寫入置換表
        vl += board.ply; //pos.Distance();
    }
	else if (vl < -WIN_VALUE)
	{
        if (mv == 0 && vl >= -BAN_VALUE)
            return; // 同理
        vl -= board.ply; //pos.Distance();
    }
	else if (mv == 0
    	//  && vl == ((board.ply &1) <<5) - DRAWVALUE ) 	//drawvalue
    	&& vl == ((board.ply &1) * DRAWVALUE *2) - DRAWVALUE )
    	//&& vl == ((board.ply &1) ? DRAWVALUE : -DRAWVALUE))
        return;   // 同理

nDepth+=(p_movenum&31);
    // 2. 逐層試探置換表﹔
    //nMinDepth = (MAX_FRESH + 1) << 8;
    nMinDepth = 512;
    nMinLayer = 0;
    uint64 offset = ((board.hkey ) & nHashMask);
    hsh = hshItems + offset;
    
    //uint32_t poskey = (board.hkey>>37); //upper 27bits  20230308
    for (i = 0; i < HASH_LAYERS; i++, hsh++) 
    {
         //hsh = hshItems + ((hkey + i) & nHashMask);
        // 3. 如果試探到一樣的局面，那麼更新置換表信息即可﹔

        //if (hsh->hkey == poskey)  //upper 26bits 27bits 20230308   
        if (hsh->hkey == (board.hkey>>37))  //upper 27bits 20230311 no poskey       	
        {
        	// 注：如果深度更深，或者邊界縮小，都可更新置換表的值
        	/*
            switch (nFlag)
            {
            	case HASH_BETA: 
            	// Beta結點要注意：不要用Null-Move的結點覆蓋正常的結點
            	if ( (i==0 || hsh->hDepth <= nDepth || hsh->hVal <= vl) && (mv != 0 || hsh->hmvBest == 0)) 
            	{
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;  
                hsh->hFlag = nFlag;              
            	}
            	break;	
            
            	case HASH_ALPHA:               	
            	if ( (hsh->hDepth <= nDepth || hsh->hVal >= vl)) 
            	{
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;
                hsh->hFlag = nFlag;
            	}
            	break;
            	  
            	case HASH_PV:	
            	if ( (i==0 || hsh->hDepth <= nDepth || hsh->hVal >= vl)) 
            	{
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;
                hsh->hFlag = nFlag;
            	}
            	break;
          	} // end switch
          	*/
          	
          	/* 1890j
            if (nFlag==HASH_ALPHA)
            {
            	if ( (hsh->hDepth <= nDepth || hsh->hVal >= vl)) 
            	{
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;
                //hsh->hFlag = nFlag;
            	}
            }
            else //HASH_BETA or PV         		
          	{
          		if ( (i==0 || hsh->hDepth <= nDepth || hsh->hVal <= vl) && (mv != 0 || hsh->hmvBest == 0)) 
            	{
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;  
                hsh->hFlag = nFlag;              
            	}
            }            
            */
            // 1890k            
            if (mv != 0 || hsh->hmvBest == 0 
            	|| (nFlag==HASH_ALPHA && ( hsh->hVal >= vl) ) 
            	)
            {            	
                hsh->hDepth = nDepth;                 
                hsh->hVal = vl;
                hsh->hFlag = nFlag;            	
            }
            // 
            
                        
            // 最佳著法是始終覆蓋的，否則輸出主要變例時會出現問題
            if (mv != 0) {
                hsh->hmvBest = set_hmv(mv);
            }

            //hshItems[(hkey + i) & nHashMask] = hsh;
            return;
        }

        // 4. 如果不是一樣的局面，那麼獲得深度最小的置換表項﹔
        //nFreshDepth = (hsh.wFresh << 8) + MAX(hsh.hAlphaDepth, hsh.hBetaDepth);
       // nFreshDepth = MAX((hsh->hDepth == 0 ? 0 : hsh->hDepth + 256),
       //                   (hsh->hmvBest == 0 ? hsh->hDepth : hsh->hDepth + 256));
                       	
       // if (nFreshDepth < nMinDepth) {
       //     nMinDepth = nFreshDepth;
       //     nMinLayer = i;
       // }
        if (hsh->hDepth < nMinDepth) {
            nMinDepth = hsh->hDepth;
            nMinLayer = i;
        }
    }

    // 5. 記錄置換表。
    //hsh = hshItems + (uint32(hkey + nMinLayer) & nHashMask);
    offset = ((board.hkey + nMinLayer ) & nHashMask);
    hsh = hshItems + offset;
    
    //hsh->hkey = poskey;  //upper 26bits 27bits 20230308
    hsh->hkey = (board.hkey>>37);  //upper 27bits 20230311 no poskey
    hsh->hmvBest = set_hmv(mv);

    hsh->hFlag = nFlag;    
    hsh->hDepth = nDepth;
    //hsh->hDepth5 = (nDepth+p_movenum)>>2;
    //hsh->hDepth2 = (nDepth+p_movenum)&3;
    hsh->hVal = vl;   
    
    

    //hshItems[(hkey + nMinLayer) & nHashMask] = hsh;
}

/*
//inline
void Engine::RecordHashQ(int nFlag, int vl, int mv, Board &board) {

    HashStruct *hsh;
    int i, nMinDepth, nMinLayer; //nFreshDepth, 

    if (vl > INF) return; //do not record UNKNOWN_VALUE
    // 存儲置換表局面信息的過程包括以下幾個步驟：

    // 1. 對分值做殺棋步數調整﹔
    if (vl > WIN_VALUE)
	{
        if (mv == 0 && vl <= BAN_VALUE)
            return; // 導致長將的局面(不進行置換裁剪)如果連最佳著法也沒有，那麼沒有必要寫入置換表
        vl += board.ply; //pos.Distance();
    }
	else if (vl < -WIN_VALUE)
	{
        if (mv == 0 && vl >= -BAN_VALUE)
            return; // 同理
        vl -= board.ply; //pos.Distance();
    }
	else if (mv == 0
    	//  && vl == ((board.ply &1) <<5) - DRAWVALUE ) 	//drawvalue
    	&& vl == ((board.ply &1) * DRAWVALUE *2) - DRAWVALUE ) 
    	//&& vl == ((board.ply &1) ? DRAWVALUE : -DRAWVALUE))
        return;   // 同理

int nDepth = (p_movenum&31);
    // 2. 逐層試探置換表﹔
    //nMinDepth = (MAX_FRESH + 1) << 8;
    nMinDepth = 512;
    nMinLayer = 0;
    hsh = hshItems + (uint32(hkey ) & nHashMask);
    for (i = 0; i < HASH_LAYERS; i++, hsh++) 
    {
         //hsh = hshItems + ((hkey + i) & nHashMask);
        // 3. 如果試探到一樣的局面，那麼更新置換表信息即可﹔

        if (hsh->hkey == (hkey>>38))  //upper 26bits
        {
            // 注：如果深度更深，或者邊界縮小，都可更新置換表的值
            

            if (nFlag==HASH_BETA)
            {
            		if ( (i==0 || hsh->hDepth <= nDepth 
            			|| hsh->hVal <= vl) && (mv != 0 || hsh->hmvBest == 0) ) 
            		{
                	hsh->hDepth = nDepth;                	
                	hsh->hVal = vl;
                	//hsh->hFlag = nFlag;
            		}
            }
            else
            {
            		if ( hsh->hDepth <= nDepth 
            			|| hsh->hVal >= vl ) 
            		{
                	hsh->hDepth = nDepth;                	
                	hsh->hVal = vl; 
                	//hsh->hFlag = nFlag;               
            		}
            }				
            // 最佳著法是始終覆蓋的，否則輸出主要變例時會出現問題
            if (mv != 0) {
                hsh->hmvBest = set_hmv(mv);
            }

            return;
        }

        // 4. 如果不是一樣的局面，那麼獲得深度最小的置換表項﹔
        //nFreshDepth = (hsh.wFresh << 8) + MAX(hsh.hAlphaDepth, hsh.hBetaDepth);
        //nFreshDepth = MAX((hsh->hDepth == 0 ? 0 : hsh->hDepth + 256),
        //                  (hsh->hmvBest == 0 ? hsh->hDepth : hsh->hDepth + 256));
        
        //if (nFreshDepth < nMinDepth) {
        //    nMinDepth = nFreshDepth;
        //    nMinLayer = i;
        //}
        if (hsh->hDepth < nMinDepth) {
            nMinDepth = hsh->hDepth;
            nMinLayer = i;
        }
    }

    // 5. 記錄置換表。
    hsh = hshItems + (uint32(hkey + nMinLayer) & nHashMask);
    hsh->hkey = (hkey>>38);  //upper 26bits
    hsh->hmvBest = set_hmv(mv);

    hsh->hFlag = nFlag;    
    hsh->hDepth = nDepth;
    //hsh->hDepth5 = (p_movenum)>>2;
    //hsh->hDepth2 = (p_movenum)&3;
    hsh->hVal = vl;    
}
*/


// 獲取置換表著法
inline
//int Engine::ProbeMove(int &nBetaDepth, int &nBetaVal, int &nAlphaDepth) //, int &nSingular)
HashStruct* Engine::ProbeMove(Board &board)
{
  HashStruct *hsh;
  //hsh = hshItems + (uint32(hkey) & nHashMask);
  uint64 offset = ((board.hkey ) & nHashMask);
    hsh = hshItems + offset;
  for (int i = 0; i < HASH_LAYERS; i++, hsh++) {
    //if (hsh->hkey == (board.hkey>>38))  //upper 26bits 20230308
    if (hsh->hkey == (board.hkey>>37))  //upper 27bits  20230308
    {
    	return hsh;
    }
  }
  return NULL; //0;
}

inline
int Engine::ProbeMoveQ(Board &board)
{
  HashStruct *hsh;
  //hsh = hshItems + (uint32(hkey) & nHashMask);
  uint64 offset = ((board.hkey ) & nHashMask);
    hsh = hshItems + offset;
  for (int i = 0; i < HASH_LAYERS; i++, hsh++) {
    //if (hsh->hkey == (board.hkey>>38))  //upper 26bits 20230308
    if (hsh->hkey == (board.hkey>>37))  //upper 27bits   20230308
    {
      return get_hmv(hsh->hmvBest);
    }
  }
  return 0;
}


/* 判斷獲取置換表要符合哪些條件，置換表的分值針對四個不同的區間有不同的處理：
 * 一、如果分值在"WIN_VALUE"以內(即介於"-WIN_VALUE"到"WIN_VALUE"之間，下同)，則隻獲取滿足搜索深度要求的局面﹔
 * 二、如果分值在"WIN_VALUE"和"BAN_VALUE"之間，則不能獲取置換表中的值(隻能獲取最佳著法僅供參考)，目的是防止由於長將而導致的“置換表的不穩定性”﹔
 * 三、如果分值在"BAN_VALUE"以外，則獲取局面時不必考慮搜索深度要求，因為這些局面已經被證明是殺棋了﹔
 * 四、如果分值是"DrawValue()"(是第一種情況的特殊情況)，則不能獲取置換表中的值(原因與第二種情況相同)。
 * 注意：對於第三種情況，要對殺棋步數進行調整！
 */
inline int ValueAdjust(int &bBanNode, int &bMateNode, int vl, int ply) {
    bBanNode = bMateNode = 0;
    if (vl > WIN_VALUE) {
        if (vl <= BAN_VALUE) {
            bBanNode = 1;
        } else {
            bMateNode = 1;
            vl -= ply; //pos.nDistance;
        }
    } else if (vl < -WIN_VALUE) {
        if (vl >= -BAN_VALUE) {
            bBanNode = 1;
        } else {
            bMateNode = 1;
            vl += ply; //pos.nDistance;
        }
    }
    //else if (vl == ((board.ply &1) <<5) - DRAWVALUE) //pos.DrawValue())
    else if (vl == ((ply &1) * DRAWVALUE *2) - DRAWVALUE)
    //else if (vl == ((board.ply &1) ? DRAWVALUE : -DRAWVALUE))
    {
        bBanNode = 1;
    }
    return vl;
}

// 獲取置換表局面信息
//inline
int Engine::ProbeHash(int nDepth, int vlBeta, int null_allow, HashStruct* &hsho, Board &board)
{

    HashStruct *hsh; //, *lphsh;
    //int i,vl;
    int i,vl, bBanNode,bMateNode;
    //int w_pvline[256];
   
    // 獲取置換表局面信息的過程包括以下幾個步驟：

nDepth+=(p_movenum&31);
// 1. 逐層獲取置換表項

    //mv = 0;
    //hsh = hshItems + (uint32(hkey ) & nHashMask);
    uint64 offset = ((board.hkey ) & nHashMask);
    hsh = hshItems + offset;
    for (i = 0; i < HASH_LAYERS; i++, hsh++) {
        //hsh = hshItems + ((hkey + i) & nHashMask);
        //if (hsh->hkey == (board.hkey>>38))  //upper 26bits
        if (hsh->hkey == (board.hkey>>37))  //upper 27bits
        {
        	  hsho = hsh;        	  
            break;
        }
    }
    if (i == HASH_LAYERS) {
    	  hsho = NULL;
        return UNKNOWN_VALUE;
    }


       // 2. 判斷獲取置換表要符合哪些條件，置換表的分值針對三個不同的區間有不同的處理：
       // *    一、如果分值在"WIN_VALUE"以內(即介於"-WIN_VALUE"到"WIN_VALUE"之間，下同)，則隻獲取滿足搜索深度要求的局面﹔
       // *    二、如果分值在"WIN_VALUE"和"BAN_VALUE"之間，則不能獲取置換表中的值(隻能獲取最佳著法僅供參考)，目的是防止由於長將而導致的“置換表的不穩定性”﹔
       // *    三、如果分值在"BAN_VALUE"以外，則獲取局面時不必考慮搜索深度要求，因為這些局面已經被證明是殺棋了。
       // *    注意：對於第三種情況，要對殺棋步數進行調整！
       // *

         // * 3. 對於空著裁剪的局面，判斷局面判斷是否命中，需要符合以下三個條件：
         // *    一、允許使用空著裁剪﹔
         // *    二、深度達到要求(注：深度在"NULL_DEPTH"以下，即達到要求)﹔
         // *    三、不是因禁止長將策略而搜索到的殺棋﹔
         // *    四、符合邊界條件。
         // *

         // 4. 其他局面判斷是否命中，需要符合以下四個條件：
         // *    一、深度達到要求(殺棋局面可以不考慮深度)﹔
         // *    二、不是因禁止長將策略而搜索到的殺棋﹔
         // *    三、符合邊界條件﹔
         // *    四、不會產生循環的局面(目的是為進一步減輕“置換表的不穩定性”，和前面的相關措施有異曲同工之妙)。
         // *

    switch (hsh->hFlag)
    {
    	case HASH_BETA:  
    	case HASH_PV:		
// 2. 判斷是否符合Beta邊界

    //if (hsh->hDepth4*8+hsh->hDepth3 > 0) 
			{
        vl = ValueAdjust(bBanNode, bMateNode, hsh->hVal, board.ply);
        if (!bBanNode && (hsh->hmvBest || null_allow) && (hsh->hDepth >= nDepth || bMateNode) && vl >= vlBeta) {
            // __ASSERT_BOUND(1 - MATE_VALUE, vl, MATE_VALUE - 1);
            if (hsh->hmvBest == 0
            	 //|| GetPvStable(w_pvline, hsh->hmvBest)
            	 ) {
                return vl;
            }
        }
    }
       break;
       
       case HASH_ALPHA:       
    // 3. 判斷是否符合Alpha邊界
    //if (hsh->hDepth4*8+hsh->hDepth3 > 0) 
		   {
        vl = ValueAdjust(bBanNode, bMateNode, hsh->hVal, board.ply);
        if (!bBanNode && (hsh->hDepth >= nDepth || bMateNode) && vl < vlBeta)   //<= vlAlpha)
        {
            // __ASSERT_BOUND(1 - MATE_VALUE, vl, MATE_VALUE - 1);
            if (hsh->hmvBest == 0 
            	//|| GetPvStable(w_pvline, hsh->hmvBest)
            	) {
                return vl;
            }
        }
    }
        break;
    } // end switch    

    // 4. 如果達不到獲取局面的要求，那麼可以返回最佳著法以便做迭代加深。
    //*lpmv = hshTemp.mvBest;
    //mv = hsh.mvBest;  //done above in hsh.hkey == hkey
    return UNKNOWN_VALUE;
}

inline int ValueAdjQ(int vl, int ply) {

    if (vl > WIN_VALUE)

            vl -= ply; //pos.nDistance;

    else if (vl < -WIN_VALUE)

            vl += ply; //pos.nDistance;

    return vl;
}
#ifdef QHASH

// 獲取置換表局面信息
inline
int Engine::ProbeHashQ(int vlBeta, int &mv, Board &board)
{
    HashStruct *hsh; //, *lphsh;
    //int i,vl;
    int i,vl; //, bBanNode,bMateNode;
    //int wmvPvLine[MAX_MOVE_NUM];
    // 獲取置換表局面信息的過程包括以下幾個步驟：

// 1. 逐層獲取置換表項

    mv = 0;
    //hsh = hshItems + (uint32(hkey ) & nHashMask);
    uint64 offset = ((board.hkey ) & nHashMask);
    hsh = hshItems + offset;
    for (i = 0; i < HASH_LAYERS; i++, hsh++) {
        //hsh = hshItems + ((hkey + i) & nHashMask);
        //if (hsh->hkey == (board.hkey>>38))  //upper 26bits 20230308
        if (hsh->hkey == (board.hkey>>37))  //upper 27bits 20230308
        {
            mv = get_hmv(hsh->hmvBest);
            break;
        }
    }
    if (i == HASH_LAYERS) {
        return UNKNOWN_VALUE;
    }

//int nDepth = p_movenum;
//if (nDepth >255) nDepth=255;
//	int nDepth = std::min(p_movenum, 255);
switch (hsh->hFlag)
    {
    	case HASH_BETA:  
    	case HASH_PV:	
// 2. 判斷是否符合Beta邊界
    //if (hsh->hDepth4*8+hsh->hDepth3 > 0)
    {
        if ( (hsh->hDepth >= (p_movenum&31) ) 
        	&& (hsh->hmvBest ) )
        //if (hsh->hmvBest )	
        {   vl = ValueAdjQ(hsh->hVal, board.ply);
            if (vl >= vlBeta)
                return vl;
        }
    }
    break;

		case HASH_ALPHA:
    // 3. 判斷是否符合Alpha邊界
    //if (hsh->hDepth4*8+hsh->hDepth3 > 0)
    {
        if ( (hsh->hDepth >= (p_movenum&31) ) 
        	&& (hsh->hmvBest == 0 ) )
        //if (hsh->hmvBest == 0 )
        {   vl = ValueAdjQ(hsh->hVal, board.ply);
            if (vl < vlBeta)  //<= vlAlpha)
                return vl;
        }
    }
    break;
  } // end switch

    // 4. 如果達不到獲取局面的要求，那麼可以返回最佳著法以便做迭代加深。
    //*lpmv = hshTemp.mvBest;
    //mv = hsh.mvBest;  //done above in hsh.hkey == hkey
    return UNKNOWN_VALUE;
}
#endif
/* eleeye 3.15
// 獲取置換表局面信息(靜態搜索)
int ProbeHashQ(const PositionStruct &pos, int vlAlpha, int vlBeta) {
  volatile HashStruct *lphsh;
  int vlHashAlpha, vlHashBeta;

  lphsh = hshItemsQ + (pos.zobr.dwKey & nHashMask);
  if (lphsh->dwZobristLock0 == pos.zobr.dwLock0) {
    vlHashAlpha = lphsh->svlAlpha;
    vlHashBeta = lphsh->svlBeta;
    if (lphsh->dwZobristLock1 == pos.zobr.dwLock1) {
      if (vlHashBeta >= vlBeta) {
        __ASSERT(vlHashBeta > -WIN_VALUE && vlHashBeta < WIN_VALUE);
        return vlHashBeta;
      }
      if (vlHashAlpha <= vlAlpha) {
        __ASSERT(vlHashAlpha > -WIN_VALUE && vlHashAlpha < WIN_VALUE);
        return vlHashAlpha;
      }
    }
  }
  return -MATE_VALUE;
}
*/



/////////////////////////////////////////////////////////////////////////

void Engine::IRead(Board &board, const char *fen)
{
#ifdef PERFT
    printf("%s\n", fen);
#endif
    int piece_count[12]={0,0,0,0,0,0, 0,0,0,0,0,0};
    int i,len,sp;
    len=strlen(fen);
    sp=0;
    //for(i=0;i<BOARD_SIZE;++i)
    //{
    //	board.piece [i]=0;
    //}
    memset(board.piece,0,sizeof(board.piece));
    board.m_side=RED; //BLACK; //99; //0;
    for (i=0; i<len && sp<BOARD_SIZE-7; ++i)
    {
        if (fen[i]>='0'&&fen[i]<='9')
            sp+=fen[i]-'0';
        else if (fen[i]=='/')
            sp+=7;	//advance 7 spaces at end of rank
        else if (fen[i]>='a'&&fen[i]<='z')
        {
            switch (fen[i])
            {
            case 'k':
                board.piece [sp]=B_KING; //1;
                sp++;
                break;
            case 'a':
                //if (piece_count[1]==0)
                    board.piece [sp]=B_ADVISOR1; //2;
                //else board.piece [sp]=B_ADVISOR2;

                //piece_count[1]++;
                sp++;
                break;

            case 'b':
            case 'e':
                //if (piece_count[2]==0)
                    board.piece [sp]=B_ELEPHAN1; //3;
                //else board.piece [sp]=B_ELEPHAN2;

                piece_count[2]++;
                sp++;
                break;

            case 'n':
            case 'h':
                //if (piece_count[3]==0)
                    board.piece [sp]=B_HORSE1; //4;
                //else board.piece [sp]=B_HORSE2;

                piece_count[3]++;
                sp++;
                break;

            case 'r':
                //if (piece_count[5]==0)
                    board.piece [sp]=B_ROOK1; //5;
                //else board.piece [sp]=B_ROOK2;

                piece_count[5]++;
                sp++;
                break;

            case 'c':
                //if (piece_count[4]==0)
                    board.piece [sp]=B_CANNON1; //6;
                //else board.piece [sp]=B_CANNON2;

                piece_count[4]++;
                sp++;
                break;

            case 'p':
                //if (piece_count[0]==0)
                //    board.piece [sp]=B_PAWN2; //7;
                //else if (piece_count[0]==1) board.piece [sp]=B_PAWN4;
                //else if (piece_count[0]==2) board.piece [sp]=B_PAWN1;
                //else if (piece_count[0]==3) board.piece [sp]=B_PAWN5;
                //else 			    board.piece [sp]=B_PAWN3;

                //piece_count[0]++;
                board.piece [sp]=B_PAWN1;
                sp++;
                break;
            }
        }
        else if (fen[i]>='A'&&fen[i]<='Z')
        {
            switch (fen[i])
            {
            case 'K':
                board.piece [sp]=R_KING; //8;
                sp++;
                break;
            case 'A':

                //if (piece_count[7]==0)
                //    board.piece [sp]=R_ADVISOR2; //2;
                //else
                board.piece [sp]=R_ADVISOR1;

                //piece_count[7]++;
                sp++;
                break;

            case 'B':
            case 'E':

                //if (piece_count[8]==0)
                //    board.piece [sp]=R_ELEPHAN2; //3;
                //else
                board.piece [sp]=R_ELEPHAN1;

                piece_count[8]++;
                sp++;
                break;

            case 'N':
            case 'H':

                //if (piece_count[9]==0)
                //    board.piece [sp]=R_HORSE2; //4;
                //else
                board.piece [sp]=R_HORSE1;

                piece_count[9]++;
                sp++;
                break;

            case 'R':

                //if (piece_count[11]==0)
                //    board.piece [sp]=R_ROOK2; //5;
                //else
                board.piece [sp]=R_ROOK1;

                piece_count[11]++;
                sp++;
                break;

            case 'C':

                //if (piece_count[10]==0)
                //    board.piece [sp]=R_CANNON2; //6;
                //else
                board.piece [sp]=R_CANNON1;

                piece_count[10]++;
                sp++;
                break;

            case 'P':
                //if (piece_count[6]==0)
                //    board.piece [sp]=R_PAWN3; //7;
                //else if (piece_count[6]==1) board.piece [sp]=R_PAWN5;
                //else if (piece_count[6]==2) board.piece [sp]=R_PAWN1;
                //else if (piece_count[6]==3) board.piece [sp]=R_PAWN4;
                //else 			    board.piece [sp]=R_PAWN2;

                //piece_count[6]++;
                board.piece [sp]=R_PAWN1;
                sp++;
                break;
            }
        }
    }

    if (sp==BOARD_SIZE-7)
    {
        for (;i<len;i++)
        {
            if (fen[i]=='W'||fen[i]=='w'||fen[i]=='R'||fen[i]=='r')
            {
                board.m_side =RED;
                break;
            }
            else if (fen[i]=='B'||fen[i]=='b')
            {
                board.m_side=BLACK;
                break;
            }
        }
    }
    else
    {
        printf("Fen String Error!\n");
        fflush(stdout);
    }

    //if(board.m_side==99) //0)
    //	board.m_side=RED;
    for (i=0;i<34;++i)
        board.boardsq[i]=SQ_EMPTY;
/*
    for (i=0;i<BOARD_SIZE-7;++i)
    {
        if (board.piece[i] != 0)
            board.boardsq[board.piece[i]]=i;
    }
*/
		// vertical scan of board to set the correct boardsq for openbook searching
//		for (i=0; i<12; i++)
//			piece_count[i]=0;

		for (i=0; i<9; i++)
		for (int j=0; j<10; j++)
		{
			int k = j*16+i;
			int piecek = board.piece[k];
			if (piecek != 0)
			{
				switch (piecek)
				{
					case B_PAWN1:
					//case B_PAWN2:
					//case B_PAWN3:
					//case B_PAWN4:
					//case B_PAWN5:
						/*
						if (piece_count[0]==0)
                    board.boardsq[B_PAWN2]=k; //7;
                else if (piece_count[0]==1) board.boardsq[B_PAWN4]=k;
                else if (piece_count[0]==2) board.boardsq[B_PAWN1]=k;
                else if (piece_count[0]==3) board.boardsq[B_PAWN5]=k;
                else 			    board.boardsq[B_PAWN3]=k;
            piece_count[0]++;
            */
            // (2) 4 1 5 3
            if (i<=1)
            {	if (board.boardsq[B_PAWN2]==SQ_EMPTY)
            		board.boardsq[B_PAWN2]=k;
            	else if (board.boardsq[B_PAWN4]==SQ_EMPTY)
            		board.boardsq[B_PAWN4]=k;
            	else if (board.boardsq[B_PAWN1]==SQ_EMPTY)
            		board.boardsq[B_PAWN1]=k;
            	else if (board.boardsq[B_PAWN5]==SQ_EMPTY)
            		board.boardsq[B_PAWN5]=k;
            	else board.boardsq[B_PAWN3]=k;
            }
            // 2 (4) 1 5 3
            else if (i==2 || i==3)
            {	if (board.boardsq[B_PAWN4]==SQ_EMPTY)
            		board.boardsq[B_PAWN4]=k;
            	else if (board.boardsq[B_PAWN2]==SQ_EMPTY)
            		board.boardsq[B_PAWN2]=k;
            	else if (board.boardsq[B_PAWN1]==SQ_EMPTY)
            		board.boardsq[B_PAWN1]=k;
            	else if (board.boardsq[B_PAWN5]==SQ_EMPTY)
            		board.boardsq[B_PAWN5]=k;
            	else board.boardsq[B_PAWN3]=k;
            }
            // 2 4 (1) 5 3
            else if (i==4)
            {	if (board.boardsq[B_PAWN1]==SQ_EMPTY)
            		board.boardsq[B_PAWN1]=k;
            	else if (board.boardsq[B_PAWN4]==SQ_EMPTY)
            		board.boardsq[B_PAWN4]=k;
            	else if (board.boardsq[B_PAWN5]==SQ_EMPTY)
            		board.boardsq[B_PAWN5]=k;
            	else if (board.boardsq[B_PAWN2]==SQ_EMPTY)
            		board.boardsq[B_PAWN2]=k;
            	else board.boardsq[B_PAWN3]=k;
            }
            // 2 4 1 (5) 3
            else if (i==5 || i==6)
            {	if (board.boardsq[B_PAWN5]==SQ_EMPTY)
            		board.boardsq[B_PAWN5]=k;
            	else if (board.boardsq[B_PAWN3]==SQ_EMPTY)
            		board.boardsq[B_PAWN3]=k;
            	else if (board.boardsq[B_PAWN1]==SQ_EMPTY)
            		board.boardsq[B_PAWN1]=k;
            	else if (board.boardsq[B_PAWN4]==SQ_EMPTY)
            		board.boardsq[B_PAWN4]=k;
            	else board.boardsq[B_PAWN2]=k;
            }
            // 2 4 1 5 (3)
            else //if (i==7 || i==8)
            {	if (board.boardsq[B_PAWN3]==SQ_EMPTY)
            		board.boardsq[B_PAWN3]=k;
            	else if (board.boardsq[B_PAWN5]==SQ_EMPTY)
            		board.boardsq[B_PAWN5]=k;
            	else if (board.boardsq[B_PAWN1]==SQ_EMPTY)
            		board.boardsq[B_PAWN1]=k;
            	else if (board.boardsq[B_PAWN4]==SQ_EMPTY)
            		board.boardsq[B_PAWN4]=k;
            	else board.boardsq[B_PAWN2]=k;
            }

						break;

					case B_ADVISOR1:
					//case B_ADVISOR2:
						/*
						if (piece_count[1]==0)
                    board.boardsq[B_ADVISOR1]=k; //2;
                else board.boardsq[B_ADVISOR2]=k;
            piece_count[1]++;
            */
            if (i<=4)
            {	if (board.boardsq[B_ADVISOR1]==SQ_EMPTY)
            		board.boardsq[B_ADVISOR1]=k;
            	else board.boardsq[B_ADVISOR2]=k;
            }
            else
            {	if (board.boardsq[B_ADVISOR2]==SQ_EMPTY)
            		board.boardsq[B_ADVISOR2]=k;
            	else board.boardsq[B_ADVISOR1]=k;
            }
						break;

					case B_ELEPHAN1:
					//case B_ELEPHAN2:
					/*
						if (piece_count[2]==0)
                    board.boardsq[B_ELEPHAN1]=k; //2;
                else board.boardsq[B_ELEPHAN2]=k;
            piece_count[2]++;
          */
            if (i<=4)
            {	if (board.boardsq[B_ELEPHAN1]==SQ_EMPTY)
            		board.boardsq[B_ELEPHAN1]=k;
            	else board.boardsq[B_ELEPHAN2]=k;
            }
            else
            {	if ((board.boardsq[B_ELEPHAN1]==SQ_EMPTY) && piece_count[2]==2)
            		board.boardsq[B_ELEPHAN1]=k;
            	else board.boardsq[B_ELEPHAN2]=k;
            }
						break;

					case B_HORSE1:
					//case B_HORSE2:
						/*
						if (piece_count[3]==0)
                    board.boardsq[B_HORSE1]=k; //2;
                else board.boardsq[B_HORSE2]=k;
            piece_count[3]++;
            */
            if (i<=4)
            {	if (board.boardsq[B_HORSE1]==SQ_EMPTY)
            		board.boardsq[B_HORSE1]=k;
            	else board.boardsq[B_HORSE2]=k;
            }
            else
            {	if ((board.boardsq[B_HORSE1]==SQ_EMPTY) && piece_count[3]==2)
            		board.boardsq[B_HORSE1]=k;
            	else board.boardsq[B_HORSE2]=k;
            }
						break;

					case B_CANNON1:
					//case B_CANNON2:
						/*
						if (piece_count[4]==0)
                    board.boardsq[B_CANNON1]=k; //2;
                else board.boardsq[B_CANNON2]=k;
            piece_count[4]++;
            */
            if (i<=4)
            {	if (board.boardsq[B_CANNON1]==SQ_EMPTY)
            		board.boardsq[B_CANNON1]=k;
            	else board.boardsq[B_CANNON2]=k;
            }
            else
            {	if ((board.boardsq[B_CANNON1]==SQ_EMPTY) && piece_count[4]==2)
            		board.boardsq[B_CANNON1]=k;
            	else board.boardsq[B_CANNON2]=k;
            }
						break;

					case B_ROOK1:
					//case B_ROOK2:
						/*
						if (piece_count[5]==0)
                    board.boardsq[B_ROOK1]=k; //2;
                else board.boardsq[B_ROOK2]=k;
            piece_count[5]++;
            */
            // R R |
            if (i<=4)
            {	if (board.boardsq[B_ROOK1]==SQ_EMPTY)
            		board.boardsq[B_ROOK1]=k;
            	else board.boardsq[B_ROOK2]=k;
            }
            else //  | R R
            {	if ((board.boardsq[B_ROOK1]==SQ_EMPTY) && piece_count[5]==2)
            		board.boardsq[B_ROOK1]=k;
            	else board.boardsq[B_ROOK2]=k;
            }
						break;

					case B_KING:
						board.boardsq[B_KING]=k;
						break;

					case R_PAWN1:
					//case R_PAWN2:
					//case R_PAWN3:
					//case R_PAWN4:
					//case R_PAWN5:
						/*
						if (piece_count[6]==0)
                    board.boardsq[R_PAWN3]=k; //7;
                else if (piece_count[6]==1) board.boardsq[R_PAWN5]=k;
                else if (piece_count[6]==2) board.boardsq[R_PAWN1]=k;
                else if (piece_count[6]==3) board.boardsq[R_PAWN4]=k;
                else 			    board.boardsq[R_PAWN2]=k;
            piece_count[6]++;
            */
            // (3) 5 1 4 2
            if (i<=1)
            {	if (board.boardsq[R_PAWN3]==SQ_EMPTY)
            		board.boardsq[R_PAWN3]=k;
            	else if (board.boardsq[R_PAWN5]==SQ_EMPTY)
            		board.boardsq[R_PAWN5]=k;
            	else if (board.boardsq[R_PAWN1]==SQ_EMPTY)
            		board.boardsq[R_PAWN1]=k;
            	else if (board.boardsq[R_PAWN4]==SQ_EMPTY)
            		board.boardsq[R_PAWN4]=k;
            	else board.boardsq[R_PAWN2]=k;
            }
            // 3 (5) 1 4 2
            else if (i==2 || i==3)
            {	if (board.boardsq[R_PAWN5]==SQ_EMPTY)
            		board.boardsq[R_PAWN5]=k;
            	else if (board.boardsq[R_PAWN1]==SQ_EMPTY)
            		board.boardsq[R_PAWN1]=k;
            	else if (board.boardsq[R_PAWN3]==SQ_EMPTY)
            		board.boardsq[R_PAWN3]=k;
            	else if (board.boardsq[R_PAWN4]==SQ_EMPTY)
            		board.boardsq[R_PAWN4]=k;
            	else board.boardsq[R_PAWN2]=k;
            }
            // 3 5 (1) 4 2
            else if (i==4)
            {	if (board.boardsq[R_PAWN1]==SQ_EMPTY)
            		board.boardsq[R_PAWN1]=k;
            	else if (board.boardsq[R_PAWN4]==SQ_EMPTY)
            		board.boardsq[R_PAWN4]=k;
            	else if (board.boardsq[R_PAWN5]==SQ_EMPTY)
            		board.boardsq[R_PAWN5]=k;
            	else if (board.boardsq[R_PAWN2]==SQ_EMPTY)
            		board.boardsq[R_PAWN2]=k;
            	else board.boardsq[R_PAWN3]=k;
            }
            // 3 5 1 (4) 2
            else if (i==5 || i==6)
            {	if (board.boardsq[R_PAWN4]==SQ_EMPTY)
            		board.boardsq[R_PAWN4]=k;
            	else if (board.boardsq[R_PAWN2]==SQ_EMPTY)
            		board.boardsq[R_PAWN2]=k;
            	else if (board.boardsq[R_PAWN1]==SQ_EMPTY)
            		board.boardsq[R_PAWN1]=k;
            	else if (board.boardsq[R_PAWN5]==SQ_EMPTY)
            		board.boardsq[R_PAWN5]=k;
            	else board.boardsq[R_PAWN3]=k;
            }
            // 3 5 1 4 (2)
            else //if (i==7 || i==8)
            {	if (board.boardsq[R_PAWN2]==SQ_EMPTY)
            		board.boardsq[R_PAWN2]=k;
            	else if (board.boardsq[R_PAWN4]==SQ_EMPTY)
            		board.boardsq[R_PAWN4]=k;
            	else if (board.boardsq[R_PAWN1]==SQ_EMPTY)
            		board.boardsq[R_PAWN1]=k;
            	else if (board.boardsq[R_PAWN5]==SQ_EMPTY)
            		board.boardsq[R_PAWN5]=k;
            	else board.boardsq[R_PAWN3]=k;
            }
						break;

					case R_ADVISOR1:
					//case R_ADVISOR2:
						/*
						if (piece_count[7]==0)
                    board.boardsq[R_ADVISOR2]=k; //2;
                else board.boardsq[R_ADVISOR1]=k;
            piece_count[7]++;
            */
            if (i<=4)
            {	if (board.boardsq[R_ADVISOR2]==SQ_EMPTY)
            		board.boardsq[R_ADVISOR2]=k;
            	else board.boardsq[R_ADVISOR1]=k;
            }
            else
            {	if (board.boardsq[R_ADVISOR1]==SQ_EMPTY)
            		board.boardsq[R_ADVISOR1]=k;
            	else board.boardsq[R_ADVISOR2]=k;
            }
						break;

					case R_ELEPHAN1:
					//case R_ELEPHAN2:
						/*
						if (piece_count[8]==0)
                    board.boardsq[R_ELEPHAN2]=k; //2;
                else board.boardsq[R_ELEPHAN1]=k;
            piece_count[8]++;
            */
            if (i<=4)
            {	if (board.boardsq[R_ELEPHAN2]==SQ_EMPTY)
            		board.boardsq[R_ELEPHAN2]=k;
            	else board.boardsq[R_ELEPHAN1]=k;
            }
            else
            {	if ((board.boardsq[R_ELEPHAN2]==SQ_EMPTY) && piece_count[8]==2)
            		board.boardsq[R_ELEPHAN2]=k;
            	else board.boardsq[R_ELEPHAN1]=k;
            }
						break;

					case R_HORSE1:
					//case R_HORSE2:
						/*
						if (piece_count[9]==0)
                    board.boardsq[R_HORSE2]=k; //2;
                else board.boardsq[R_HORSE1]=k;
            piece_count[9]++;
            */
            // N N |
            if (i<=4)
            {	if (board.boardsq[R_HORSE2]==SQ_EMPTY)
            		board.boardsq[R_HORSE2]=k;
            	else board.boardsq[R_HORSE1]=k;
            }
            else // | N N
            {	if ((board.boardsq[R_HORSE2]==SQ_EMPTY) && piece_count[9]==2)
            		board.boardsq[R_HORSE2]=k;
            	else board.boardsq[R_HORSE1]=k;
            }
						break;

					case R_CANNON1:
					//case R_CANNON2:
						/*
						if (piece_count[10]==0)
                    board.boardsq[R_CANNON2]=k; //2;
                else board.boardsq[R_CANNON1]=k;
            piece_count[10]++;
            */
            if (i<5)
            {	if (board.boardsq[R_CANNON2]==SQ_EMPTY)
            		board.boardsq[R_CANNON2]=k;
            	else board.boardsq[R_CANNON1]=k;
            }
            else
            {	if ((board.boardsq[R_CANNON2]==SQ_EMPTY) && piece_count[10]==2)
            		board.boardsq[R_CANNON2]=k;
            	else board.boardsq[R_CANNON1]=k;
            }
						break;

					case R_ROOK1:
					//case R_ROOK2:
						/*
						if (piece_count[11]==0)
                    board.boardsq[R_ROOK2]=k; //2;
                else board.boardsq[R_ROOK1]=k;
            piece_count[11]++;
            */
            if (i<5)
            {	if (board.boardsq[R_ROOK2]==SQ_EMPTY)
            		board.boardsq[R_ROOK2]=k;
            	else board.boardsq[R_ROOK1]=k;
            }
            else
            {	if ((board.boardsq[R_ROOK2]==SQ_EMPTY) && piece_count[11]==2)
            		board.boardsq[R_ROOK2]=k;
            	else board.boardsq[R_ROOK1]=k;
            }
						break;

					case R_KING:
						board.boardsq[R_KING]=k;
						break;
					}
				}
			}



    memset(board.wBitRanks,0,sizeof(board.wBitRanks));
    memset(board.wBitFiles,0,sizeof(board.wBitFiles));
	board.bitpiece=0;
	memset(board.piececnt,0,sizeof(board.piececnt));
#ifdef EVALATTK
	  memset(board.bitattk,0,sizeof(board.bitattk));
#endif
    for (i=2; i<34; i++)
    {
        sp = board.boardsq[i];
        //if (sp >=0 /* != SQ_EMPTY */)
        	if (NOTSQEMPTY(sp))
        {
            board.wBitRanks[nRank(sp)] ^= PreGen.wBitRankMask[sp];
            board.wBitFiles[nFile(sp)] ^= PreGen.wBitFileMask[sp];
            board.piececnt[i &61]++;
            if (i<B_KING) // not king
            {
						board.bitpiece ^= (1 << i); //BITPIECE_MASK[i];
#ifdef EVALATTK
						board.bitattk[ATTKAREA[sp]] ^= (1 << i);
#endif
            }
						//update piece val after vertical scan of board
						board.piece[sp]=i;
        }
    }
board.hkey = init_hkey(board);
    //IHkey=init_hkey(board);
    

    //board=board;
	//board.hkey=IHkey;
    board.Init_index(); //compressed boardsq may affect booksearch
    board.m_timeout=0;

    board.m_hisindex=0;
    board.ply = 0;
    for (int i=0;i<34;++i)
    {        
        board.piececnt[i]=0;
     }

		for (int i=0; i<10; i++)
		{
			board.bitattk[i] = 0;
		}	
    board.pointsum = 0;

//    IComSide=board.m_side ;


    /*
    // print board after read FEN for debug
    printf("\n");
    fflush(stdout);
    for (int i=0; i<10; i++)
    {	for (int j=0; j<9; j++)
    	{
    	//fprintf(traceout, " %c ", PieceChar[board.piece[(i*9)+j]] );
    	printf(" %c ", PieceChar[board.piece[(i*16)+j]] );
    	}
    	//fprintf(traceout, "\n");
    	printf("\n");
    	fflush(stdout);
    }


		*/
		AdjustEndgame(board);
}

/* from eleeye 3.21 hash.cpp
// 檢測下一個著法是否穩定，有助於減少置換表的不穩定性
inline bool MoveStable(PositionStruct &pos, int mv) {
  // 判斷下一個著法是否穩定的依據是：
  // 1. 沒有後續著法，則假定是穩定的﹔
  if (mv == 0) {
    return true;
  }
  // 2. 吃子著法是穩定的﹔
  __ASSERT(pos.LegalMove(mv));
  if (pos.ucpcSquares[DST(mv)] != 0) {
    return true;
  }
  // 3. 可能因置換表引起路線遷移，使得路線超過"MAX_MOVE_NUM"，此時應立刻終止路線，並假定是穩定的。
  if (!pos.MakeMove(mv)) {
    return true;
  }
  return false;
}

// 檢測後續路線是否穩定(不是循環路線)，有助於減少置換表的不穩定性
static bool PosStable(const PositionStruct &pos, int mv) {
  HashStruct hsh;
  int i, nMoveNum;
  bool bStable;
  // pos會沿著路線變化，但最終會還原，所以被視為"const"，而讓"posMutable"承擔非"const"的角色
  PositionStruct &posMutable = (PositionStruct &) pos;

  __ASSERT(mv != 0);
  nMoveNum = 0;
  bStable = true;
  while (!MoveStable(posMutable, mv)) {
    nMoveNum ++; // "!MoveStable()"表明已經執行了一個著法，以後需要撤消
    // 執行這個著法，如果產生循環，那麼終止後續路線，並確認該路線不穩定
    if (posMutable.RepStatus() > 0) {
      bStable = false;
      break;
    }
    // 逐層獲取置換表項，方法同"ProbeHash()"
    for (i = 0; i < HASH_LAYERS; i ++) {
      hsh = HASH_ITEM(posMutable, i);
      if (HASH_POS_EQUAL(hsh, posMutable)) {
        break;
      }
    }
    mv = (i == HASH_LAYERS ? 0 : hsh.wmv);
  }
  // 撤消前面執行過的所有著法
  for (i = 0; i < nMoveNum; i ++) {
    posMutable.UndoMakeMove();
  }
  return bStable;
}
*/ 
// end of eleeye 3.21 hash.cpp
/*
// 從置換表中獲得主要變例路線，找到循環路線則返回"TRUE"
int Engine::GetPvStable(int *lpwmvPvLine, int mvFirst) 
{
  HashStruct *hsho; //, *lphsh;
  int i, nPvLineNum, bStable;
  MoveStruct tempmove;
  
  nPvLineNum = 0;  
  lpwmvPvLine[nPvLineNum] = mvFirst;     
  tempmove.move = mvFirst;
  bStable = 1;       
    while (true)
    {
    	
    	// 2. 吃子著法是穩定的﹔
  if (board.piece[tempmove.dest] != 0) 
  	{
    break;
  }
    	
    	
    	nPvLineNum ++;    
    	if (nPvLineNum >= 256)
    		break;	
    	makemovenoeval<0>(tempmove); //, 0);    	   
      if (checkloop(1)) 
      {        
        bStable = 0;
        break;
      }
      
  
  uint32_t offset = ((board.hkey ) & nHashMask);
    hsho = hshItems + offset;
  for (i = 0; i < HASH_LAYERS; i++, hsho++) 
  {
    if (hsho->hkey == (board.hkey>>38))  //upper 26bits
    {
    	tempmove.move = hsho->hmvBest;
      lpwmvPvLine[nPvLineNum] = tempmove.move;
    	break;
    	// return hsh;
    }
  }
  
  if (i==HASH_LAYERS || tempmove.move == 0)
  	break;

    }
    
    // 5. 在主要變例列表中加入結束標志，然後還原前面執行過的所有著法
    lpwmvPvLine[nPvLineNum] = 0;   
    for (int i=0; i<nPvLineNum; i++)
    {        
        unmakemovenoeval();
    }
    
    return bStable;
}
*/
// 從置換表中獲得主要變例路線，找到循環路線則返回"TRUE"
void Engine::GetPvLine(Board &board, int *lpwmvPvLine, int mvFirst) 
{
  HashStruct *hsho; //, *lphsh;
  int i, nPvLineNum; //,bStable;
  MoveStruct tempmove;
  
  nPvLineNum = 0;  
  lpwmvPvLine[nPvLineNum] = mvFirst;     
  tempmove.move = mvFirst;
         
    while (true)
    {
    	nPvLineNum ++;  
    	if (nPvLineNum >= 256)
    		break;	  	
    	board.makemovenoeval(tempmove, 0); //, 0);    	   
      if (board.checkloop(1)) //3
      {
        
        //bStable = 0;
        break;
      }
      
  
  uint32_t offset = ((board.hkey ) & nHashMask);
    hsho = hshItems + offset;
  for (i = 0; i < HASH_LAYERS; i++, hsho++) 
  {
    //if (hsho->hkey == (board.hkey>>38))  //upper 26bits 20230308
    if (hsho->hkey == (board.hkey>>37))  //upper 27bits 20230308
    {
    	tempmove.move = hsho->hmvBest;
      lpwmvPvLine[nPvLineNum] = tempmove.move;
    	break;
    	// return hsh;
    }
  }
  
  if (i==HASH_LAYERS || tempmove.move == 0 || board.LegalKiller(tempmove) == 0)
  	break;

    }
    
    // 5. 在主要變例列表中加入結束標志，然後還原前面執行過的所有著法
    lpwmvPvLine[nPvLineNum] = 0;   
    for (int i=0; i<nPvLineNum; i++)
    {        
        board.unmakemovenoeval();
    }
}


void Engine::PutPvLine(Board &board, int *lpwmvPvLine, int m_depth, int best) 
{
  //HashStruct *hsho; //, *lphsh;
  int nPvLineNum; //,bStable;
  MoveStruct tempmove;
  
  nPvLineNum = 0;  
  //lpwmvPvLine[nPvLineNum] = mvFirst;     
  tempmove.move = lpwmvPvLine[0]; //mvFirst;
         
    while (true)
    {
    	if (tempmove.move == 0 || board.LegalKiller(tempmove) == 0)
    		break;
    	nPvLineNum ++;  
    	if (nPvLineNum >= 256)
    		break;	  

if (nPvLineNum > 1)
{	     
	// lazy smp bug - solved in 2892r by add board as param
	int hashmove = 0;
  hashmove = ProbeMoveQ(board);
  if (hashmove == 0 || hashmove != board.m_bestmove) //tempmove.move)
  {
  	//
  	char c_hashmove[5], c_bestmove[5];  //, c_rootpv[5];
  	MoveStruct hash_tempmove;
  	hash_tempmove.move = hashmove;
  	com2char(c_hashmove, hash_tempmove.from, hash_tempmove.dest );
  	hash_tempmove.move = board.m_bestmove;
  	com2char(c_bestmove, hash_tempmove.from, hash_tempmove.dest );  	
  	printf("     **PutPv back at depth=%d hashmove=%s best move=%s\n", m_depth, c_hashmove, c_bestmove);
  	fflush(stdout);
  	//
  	RecordHash(HASH_PV, m_depth, best, board.m_bestmove, board); //tempmove.move);
  }		
// lazy smp  
  //int hashmove = 0;
  //hashmove = ProbeMoveQ(board);
  //if (hashmove == 0 || hashmove != tempmove.move)
  //{
  //	RecordHash(HASH_PV, 0, Evalscore(), tempmove.move, board);
  //}		
}  
      			
    	
    	board.makemovenoeval(tempmove, 0); 	   
      if (board.checkloop(1))  //(3)
      {        
        //bStable = 0;
        break;
      }

tempmove.move = lpwmvPvLine[nPvLineNum];
  
    } // end while true
    
       
    for (int i=0; i<nPvLineNum; i++)
    {        
        
        board.unmakemovenoeval();
    }
}


/*
// 從置換表中獲得主要變例路線，找到循環路線則返回"FALSE"
int Engine::GetPvLine(search_stack_t &ss) //, int mvFirst)
{
    //HashStruct hsh; //, *lphsh;
    int nPvLineNum = 0;
    int bStablePos = 1;
    MoveStruct tempmove;

    //for (; ; )
    while (true)
    {
        // 3. 如果沒有置換表項，或者是空著，那麼主要變例終止，否則把著法寫入主要變例列表
        //if (i == HASH_LAYERS || hsh.mvBest == 0)
        if (ss.pv[nPvLineNum]==0)
        {
            break;
        }
        //lpwmvPvLine[nPvLineNum] = hsh.mvBest;


        // 4. 執行這個著法，如果出現重復局面，那麼主要變例終止

        tempmove.move = ss.pv[nPvLineNum];
        // 2. 吃子著法是穩定的﹔
        if (board.piece[tempmove.dest])
        {
            //bStablePos = 1;
            //nPvLineNum --;
            break;
        }
        //makemoveNochk(tempmove);
        makemovenoeval<0>(tempmove); //, 0);

        nPvLineNum ++;
        if (checkloop(1))
        {
            bStablePos = 0;
            break;
        }
        //nPvLineNum ++;
    }

    // 5. 在主要變例列表中加入結束標志，然後還原前面執行過的所有著法
    //lpwmvPvLine[nPvLineNum] = 0;
    //for (nPvLineNum --; nPvLineNum >= 0; nPvLineNum --)
    for (int i=0; i<nPvLineNum; i++)
    {
        //unmakemoveNochk();
        unmakemovenoeval();
    }

    return bStablePos;
}
*/

//debug
#ifdef DEBUG
void Engine::PrintLog(char *szFileName)
{
    uint32_t n; //m
    //int k, nSrc, nDst, nCaptured;

    //FILE *out = fopen(szFileName, "a+"); //w+");   //use append

    //fprintf(out, "************************** Search Log ***************************\n");

    fprintf(out, "Search Depth: %3d  ",searchdepth);

    n = nTreeNodes + nLeafNodes + nQuiescNodes;
    //float TimeSpan = (clock() - board.m_startime) / 1000.0f;  // StartTimer/1000.0f;
    float TimeSpan = (GetTime() - board.m_startime) / 1000.0f;  // StartTimer/1000.0f;
    //fprintf(out, "Search Time    :   %8.3f Sec", TimeSpan);
    //fprintf(out, "  Speed (excl QS):   %8.0f nps", (nTreeNodes+nLeafNodes)/TimeSpan);
    //fprintf(out, "  Speed (overall):   %8.0f nps\n", n/TimeSpan);

    fprintf(out, "Tree:%10u Leaf:%10u Q:%10u %10u %8.3f Sec %8.0f NQNPS% 8.0f nps\n",nTreeNodes,nLeafNodes,nQuiescNodes,n,TimeSpan,(nTreeNodes+nLeafNodes)/TimeSpan,n/TimeSpan);

    //fprintf(out, "NullMoveCuts   = %u", nNullMoveCuts);
    //fprintf(out, "  NullMoveNodes  = %u", nNullMoveNodes);
    //fprintf(out, "  NullMove CutRate = %.2f%%\n\n", nNullMoveCuts/(float)nNullMoveNodes*100.0f);
//	fprintf(out, "  NullMove: Reduc=%3d  Ver=%3d  Cuts=%10u  Moves=%10u  Hits= %.2f%%\n", nullreduction, nullreduction+2, nNullMoveCuts, nNullMoveNodes, nNullMoveCuts/(float)nNullMoveNodes*100.0f);
//	fprintf(out, "  HistPrun: Shift=%3d  Ext=%3d  Vers=%10u  Nodes=%10u  ExtNodes=%10u  PerctVers= %.2f%%\n\n",
//	 HistPrunShift, ExtHistPrunShift,nHistPrunVers, nHistPrunNodes, nExtHistPrunNodes,  nHistPrunVers/(float)(nHistPrunNodes+nExtHistPrunNodes)*100.0f);
    fprintf(out, "  HistPrun =%10u  ExtHistPrun =%10u\n", nHistPrunNodes,nExtHistPrunNodes);
    /*
    	fprintf(out, "Hash表大小: %d Bytes  =  %d M\n", pHash->nHashSize*2*sizeof(CHashRecord), pHash->nHashSize*2*sizeof(CHashRecord)/1024/1024);
    	fprintf(out, "Hash覆蓋率: %d / %d = %.2f%%\n\n", pHash->nHashCovers, pHash->nHashSize*2, pHash->nHashCovers/float(pHash->nHashSize*2.0f)*100.0f);

    	uint32_t nHashHits = pHash->nHASH_ALPHA+pHash->nHashExact+pHash->nHASH_BETA;
    	fprintf(out, "Hash命中: %d = %d(alpha:%.2f%%) + %d(exact:%.2f%%) +%d(beta:%.2f%%)\n", nHashHits, pHash->nHASH_ALPHA, pHash->nHASH_ALPHA/(float)nHashHits*100.0f, pHash->nHashExact, pHash->nHashExact/(float)nHashHits*100.0f, pHash->nHASH_BETA, pHash->nHASH_BETA/(float)nHashHits*100.0f);
    	fprintf(out, "命中概率: %.2f%%\n", nHashHits/float(nTreeNodes+nLeafNodes)*100.0f);
    	fprintf(out, "樹枝命中: %d / %d = %.2f%%\n", nTreeHashHit, nTreeNodes, nTreeHashHit/(float)nTreeNodes*100.0f);
    	fprintf(out, "葉子命中: %d / %d = %.2f%%\n\n", nLeafHashHit, nLeafNodes, nLeafHashHit/(float)nLeafNodes*100.0f);


    	fprintf(out, "殺手移動 : \n");
    	k = n = 0;
    	for(m=0; m<MaxKiller; m++)
    	{
    		fprintf(out, "    Killer   %d : %8d /%8d = %.2f%%\n", m+1, nKillerCuts[m], nKillerNodes[m], nKillerCuts[m]/float(nKillerNodes[m]+0.001f)*100.0f);
    		n += nKillerCuts[m];
    		k += nKillerNodes[m];
    	}
    	fprintf(out, "    殺手剪枝率 : %8d /%8d = %.2f%%\n\n", n, k, n/float(k+0.001f)*100.0f);

    	fprintf(out, "Hash沖突   : %d\n", pHash->nCollision);
    	fprintf(out, "Null&Kill  : %d\n", pHash->nCollision-nHashMoves);
    	fprintf(out, "HashMoves  : %d\n", nHashMoves);
    	fprintf(out, "HashCuts   : %d\n", nHashCuts);
    	fprintf(out, "Hash剪枝率 : %.2f%%\n\n", nHashCuts/(float)nHashMoves*100.0f);
    */
#ifdef DEBUG
//	fprintf(out, " Killer BetaNodes: %d", nBetaNodes);
//	fprintf(out, " HashMoves  : %d", nHashMoves);
//	fprintf(out, " HashCuts   : %d\n", nHashCuts);
    fprintf(out, " FUTILITY_MARGIN : %d,  nFutility : %d ", FUT_MARGIN[1], nFutility);
    fprintf(out, " FUT_MARGIN[2] : %d,  nExtFutility : %d\n", FUT_MARGIN[2], nExtFutility);
    fflush(out);
#endif
//fprintf(out, " BetaNodes, BetaMoveSum: %d %d %.2f%%", nBetaNodes, nBetaMoveSum, nBetaNodes/(float)(nBetaMoveSum)*100.0f);
//	fprintf(out, " BetaCutAt1: %d %.2f%%\n", nBetaCutAt1, nBetaCutAt1/(float)(nBetaNodes)*100.0f);
    /*
    	n = nCheckCounts[1] + nCheckCounts[2] + nCheckCounts[3] + nCheckCounts[4];
    	fprintf(out, "將軍次數: %d\n", n);
    	fprintf(out, "探測次數: %d\n", nCheckCounts[0]);
    	fprintf(out, "成功概率: %.2f%%\n", n/(float)nCheckCounts[0]*100.0f);
    	fprintf(out, "    車帥: %d\n", nCheckCounts[1]);
    	fprintf(out, "    炮將: %d\n", nCheckCounts[2]);
    	fprintf(out, "    馬將: %d\n", nCheckCounts[3]);
    	fprintf(out, "    兵卒: %d\n\n", nCheckCounts[4]);

    	fprintf(out, "CheckEvasions = %d\n", nCheckEvasions);
    	fprintf(out, "解將 / 將軍   = %d / %d = %.2f%%\n\n", nCheckEvasions, n, nCheckEvasions/float(n)*100.0f);


    	// 顯示主分支
    	int BoardStep[90];
    	for(n=0; n<90; n++)
    		BoardStep[n] = Board[n];

    	static const char ChessName[14][4] = {"帥","車","炮","馬","象","士","卒", "將","車","炮","馬","相","仕","兵"};

    	fprintf(out, "\n主分支：PVLine***HashDepth**************************************\n");
    	for(m=0; m<nPvLineNum; m++)
    	{
    		nSrc = PvLine[m].src;
    		nDst = PvLine[m].dst;
    		nCaptured = BoardStep[nDst];

    		// 回合數與棋步名稱
    		fprintf(out, "    %2d. %s", m+1, GetStepName( PvLine[m], BoardStep ));

    		// 吃子著法
    		if(nCaptured>=0 && nCaptured<32)
    			fprintf(out, " k-%s", ChessName[nPieceType[nCaptured]]);
    		else
    			fprintf(out, "     ");

    		// 搜索深度
    		fprintf(out, "  depth = %2d", (PvLine[m].key)>>16);

    		// 將軍標志
    		nCaptured = short((PvLine[m].key)&0xFFFF);
    		if(nCaptured)
    			fprintf(out, "   Check Extended 1 board.ply ");
    		fprintf(out, "\n");

    		BoardStep[nDst] = BoardStep[nSrc];
    		BoardStep[nSrc] = -1;
    	}

    	fprintf(out, "\n\n***********************第%2d 回合********************************\n\n", (nCurrentStep+1)/2);
    	fprintf(out, "***********************電腦生成：%d 個分支**********************\n\n", nFirstLayerMoves);
    	for(m=0; m<(unsigned int)nFirstLayerMoves; m++)
    	{
    		nSrc = FirstLayerMoves[m].src;
    		nDst = FirstLayerMoves[m].dst;

    		// 尋找主分支
    		if(PvLine[0].src==nSrc && PvLine[0].dst==nDst)
    		{
    			fprintf(out, "*PVLINE=%d***********Nodes******History**************************\n", m+1);
    			fprintf(out, "*%2d.  ", m+1);
    		}
    		else
    			fprintf(out, "%3d.  ", m+1);

    		//n = m==0 ? FirstLayerMoves[m].key : FirstLayerMoves[m].key-FirstLayerMoves[m-1].key;	// 統計分支數目
    		n = FirstLayerMoves[m].key;																// 統計估值
    		fprintf(out, "%s = %6d    hs = %6d\n", GetStepName(FirstLayerMoves[m], Board), n, HistoryRecord[nSrc][nDst]);
    	}


    	fprintf(out, "\n\n***********************歷史記錄********************************\n\n", (nCurrentStep+1)/2);

    	char ArgPtr[5];
    	for(m=0; m<=(int)nCurrentStep; m++)
    	{
    		nSrc = StepRecords[m].src;
    		nDst = StepRecords[m].dst;

    		ArgPtr[0] = nSrc/10 + 'a';
    		ArgPtr[1] = nSrc%10 + '0';
    		ArgPtr[2] = nDst/10 + 'a';
    		ArgPtr[3] = nDst%10 + '0';
    		ArgPtr[4] = '\0';

    		fprintf(out, "%3d. %s  %2d  %2d  %12u\n", m, ArgPtr, StepRecords[m].capture, StepRecords[m].incheck, StepRecords[m].zobrist);
    	}
    */

    // 關閉文件
//debug
//	fclose(out);
}
//debug
#endif

#ifdef PERFT
/* for perft comparison
// depth:5  nodes:1333312995 time:39804 ms (fairy-stockfish), 56264 ms (javascipt wukong), 75850 ms (chameleon c++)
ucci
position fen rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1
go time 57000 increment 0 opptime 57000 oppincrement 0 limittime 60000 steptime 0 byotime 0
Info MaxTime:5700
depth=    1  Perft(1)=              44  Std Perft=              44  time=2
depth=    2  Perft(2)=            1920  Std Perft=            1920  time=1
depth=    3  Perft(3)=           79666  Std Perft=           79666  time=4
depth=    4  Perft(4)=         3290240  Std Perft=         3290240  time=119
depth=    5  Perft(5)=       133312995  Std Perft=       133312995  time=4820
depth=    5  Perft(5)=       133312995                              time=4915 TimeSpan(ms)=  4915 (2023c)
depth=    5  Perft(5)=       133312995  Std Perft=       133312995  time(ms)=5178 TimeSpan(ms)=5177
depth=    5  Perft(5)=       133312995  Std Perft=       133312995  time(ms)=5111 (2022b64) //if (board.m_hisindex > 0)
depth=    5  Perft(5)=       133312995  Std Perft=       133312995  time(ms)=4691 (2023d) 
bestmove a9a9
*/
uint64_t Engine::StdPerft(int depth)
{
static const uint64_t STD_PERFT[7] = {
1ULL,
44ULL,
1920ULL,
79666ULL,
3290240ULL,
133312995ULL,
5392831844ULL
};
return STD_PERFT[depth];
}

uint64_t Engine::Perft(int depth)
{

   //assert(depth >= 1);
   // assert(depth >= 0);
    if(depth==0)
    {
       return 1;
    }
    //MOVE move_list[256];
    int i, size; //n_moves, i;
    uint64_t nodes = 0;

    //n_moves = GenerateLegalMoves(move_list);//合法著法
    int incheck=0;
    //if (m_hisindex > 0)
    //    incheck=m_hisrecord[m_hisindex-1].htab.Chk;
    if (board.m_hisindex > 0)
        incheck=board.m_hisrecord[board.m_hisindex-1].htab.Chk;
    else
    {
        //incheck=board.IsInCheck(board.m_side, 0);	//not singlechk
        incheck=board.IsInCheck<0>(board.m_side);	//not singlechk
        //m_hisrecord[0].htab.Chk = incheck;
    }
    MoveTabStruct movetab[111], ncapmovetab[64];
    MoveStruct tempmove;

    long ncapsize; //=0;

    //int nBanMoves=0;
    if (incheck)
    {
        size=board.GenChkEvasCap(&movetab[0], incheck);
        ncapsize=board.GenChkEvasNCap(&movetab[size], incheck);
        size += ncapsize;
    }
    else
    {
        size=board.GenCap(&movetab[0], &ncapmovetab[0], ncapsize);
        memcpy(&movetab[size], &ncapmovetab[0], ncapsize * 4);
        size += ncapsize;
        ncapsize=board.GenNonCap(&movetab[size], depth);
        size += ncapsize;
    }



//----------------------------------
    // if(depth <= 1)
    //{
    //   return size; //n_moves;
    //}

    for (int i = 0; i < size; i++)
    {
        //MakeMove(move_list);
    		tempmove.move = movetab[i].table.move;
    		if ( board.makemovenoeval(tempmove, 1) < 0) //, 1) < 0 )	//illegal move
    			continue;
        nodes += Perft(depth - 1);
        //UndoMove(move_list);
        board.unmakemovenoeval();
    }
    return nodes;
}
/*
uint64_t Perft(int depth)
{
   assert(depth >= 1);

    MOVE move_list[256];
    int n_moves, i;
    uint64_t nodes = 0;

    n_moves = GenerateLegalMoves(move_list);//合法著法

     if(depth == 1)
    {
       return n_moves;
    }

    for (i = 0; i < n_moves; i++)
    {
        MakeMove(move_list);
        nodes += Perft(depth - 1);
        UndoMove(move_list);
    }
    return nodes;
}

下面是bugchess的結果, nodes指的是合理著法的個數。
---------------------------------------------------------------------------------------------------
rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w
     depth     nodes    checks            captures
         1        44         0                   2
         2      1920         6                  72
         3     79666       384                3159
         4   3290240     19380              115365
         5 133312995    953251             4917734


--------------------------------------------------------------------------------------------------
r1ba1a3/4kn3/2n1b4/pNp1p1p1p/4c4/6P2/P1P2R2P/1CcC5/9/2BAKAB2 w
     depth     nodes    checks            captures
         1        38         1                   1
         2      1128        12                  10
         3     43929      1190                2105
         4   1339047     21299               31409
         5  53112976   1496697             3262495

---------------------------------------------------------------------------------------------------
r2akab1r/3n5/4b3n/p1p1pRp1p/9/2P3P2/P3P3c/N2C3C1/4A4/1RBAK1B2 w
     depth     nodes    checks            captures
         1        58         1                   4
         2      1651        54                  70
         3     90744      1849                6642
         4   2605437     70934              128926
         5 140822416   3166538            10858766
*/
#endif //end PERFT
