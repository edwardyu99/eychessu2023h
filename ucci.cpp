/* ucci.h/ucci.cpp - Source Code for ElephantEye, Part I

ElephantEye - a Chinese Chess Program (UCCI Engine)
Designed by Morning Yellow, Version: 3.0, Last Modified: Nov. 2007
Copyright (C) 2004-2007 www.elephantbase.net

This part (ucci.h/ucci.cpp only) of codes is NOT published under LGPL, and
can be used without restriction.

20230123 - cast (char*) to avoid c++ error
*/

#include <stdio.h>
#include "base.h"
#include "base2.h"
#include "parse.h"
#include "pipe.h"
#include "ucci.h"  

/* UCCI指令分析模塊由三各UCCI指令解釋器組成。
 *
 * 其中第一個解釋器"BootLine()"最簡單，隻用來接收引擎啟動後的第一行指令
 * 輸入"ucci"時就返回"UCCI_COMM_UCCI"，否則一律返回"UCCI_COMM_UNKNOWN"
 * 前兩個解釋器都等待是否有輸入，如果沒有輸入則執行待機指令"Idle()"
 * 而第三個解釋器("BusyLine()"，隻用在引擎思考時)則在沒有輸入時直接返回"UCCI_COMM_UNKNOWN"
 */
static PipeStruct pipeStd;

const int MAX_MOVE_NUM = 512; //384; //512; //256;

static char szFen[LINE_INPUT_MAX_CHAR];
static uint32 dwCoordList[MAX_MOVE_NUM];

static Bool ParsePos(UcciCommStruct &UcciComm, char *lp) {
  int i;
  


  // 首先判斷是否指定了FEN串
  if (StrEqvSkip(lp, "fen ")) {
    strcpy(szFen, lp);
    UcciComm.szFenStr = szFen;
  // 然後判斷是否是startpos
  } else if (StrEqv(lp, "startpos")) {
    UcciComm.szFenStr = (char*)"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1";
    
  // 如果兩者都不是，就立即返回
  } else {
    return FALSE;
  }
  // 然後尋找是否指定了後續著法，即是否有"moves"關鍵字
  UcciComm.nMoveNum = 0;
  if (StrScanSkip(lp, (char*)" moves ")) {
    *(lp - strlen(" moves ")) = '\0';
    UcciComm.nMoveNum = MIN((int) (strlen(lp) + 1) / 5, MAX_MOVE_NUM); // 提示："moves"後面的每個著法都是1個空格和4個字符
    for (i = 0; i < UcciComm.nMoveNum; i ++) {
      dwCoordList[i] = *(uint32 *) lp; // 4個字符可轉換為一個"uint32"，存儲和處理起來方便
      lp += sizeof(uint32) + 1;
    }
    UcciComm.lpdwMovesCoord = dwCoordList;
  }
  return TRUE;
}

UcciCommEnum BootLine(void) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  pipeStd.Open();
  while (!pipeStd.LineInput(szLineStr)) {
    Idle();
  }
  if (StrEqv(szLineStr, "ucci")) {
    return UCCI_COMM_UCCI;
  } else {
    return UCCI_COMM_UNKNOWN;
  }
}

UcciCommEnum IdleLine(UcciCommStruct &UcciComm, Bool bDebug) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  char *lp;
  int i;
  Bool bGoTime;

  while (!pipeStd.LineInput(szLineStr)) {
    Idle();
  }
  lp = szLineStr;
  if (bDebug) {
    printf("info idleline [%s]\n", lp);
    fflush(stdout);
  }
  if (FALSE) {
  // "IdleLine()"是最復雜的UCCI指令解釋器，大多數的UCCI指令都由它來解釋，包括：

  // 1. "isready"指令
  } else if (StrEqv(lp, "isready")) {
    return UCCI_COMM_ISREADY;

  // 2. "setoption <option> [<arguments>]"指令
  } else if (StrEqvSkip(lp, "setoption ")) {
    if (FALSE) {

    // (1) "batch"選項
    } else if (StrEqvSkip(lp, "batch ")) {
      UcciComm.Option = UCCI_OPTION_BATCH;
      if (StrEqv(lp, "on")) {
        UcciComm.bCheck = TRUE;
      } else if (StrEqv(lp, "true")) {
        UcciComm.bCheck = TRUE;
      } else {
        UcciComm.bCheck = FALSE;
      } // 由於"batch"選項默認是關閉的，所以隻有設定"on"或"true"時才打開，下同

    // (2) "debug"選項
    } else if (StrEqvSkip(lp, "debug ")) {
      UcciComm.Option = UCCI_OPTION_DEBUG;
      if (StrEqv(lp, "on")) {
        UcciComm.bCheck = TRUE;
      } else if (StrEqv(lp, "true")) {
        UcciComm.bCheck = TRUE;
      } else {
        UcciComm.bCheck = FALSE;
      }

    // (3) "ponder"選項
    } else if (StrEqvSkip(lp, "ponder ")) {
      UcciComm.Option = UCCI_OPTION_PONDER;
      if (StrEqv(lp, "on")) {
        UcciComm.bCheck = TRUE;
      } else if (StrEqv(lp, "true")) {
        UcciComm.bCheck = TRUE;
      } else {
        UcciComm.bCheck = FALSE;
      }

    // (4) "usebook"選項
    } else if (StrEqvSkip(lp, "usebook ")) {
      UcciComm.Option = UCCI_OPTION_USEBOOK;
      if (StrEqv(lp, "off")) {
        UcciComm.bCheck = FALSE;
      } else if (StrEqv(lp, "false")) {
        UcciComm.bCheck = FALSE;
      } else {
        UcciComm.bCheck = TRUE;
      }

    // (5) "useegtb"選項
    } else if (StrEqvSkip(lp, "useegtb ")) {
      UcciComm.Option = UCCI_OPTION_USEEGTB;
      if (StrEqv(lp, "off")) {
        UcciComm.bCheck = FALSE;
      } else if (StrEqv(lp, "false")) {
        UcciComm.bCheck = FALSE;
      } else {
        UcciComm.bCheck = TRUE;
      }

    // (6) "bookfiles"選項
    } else if (StrEqvSkip(lp, "bookfiles ")) {
      UcciComm.Option = UCCI_OPTION_BOOKFILES;
      UcciComm.szOption = lp;

    // (7) "egtbpaths"選項
    } else if (StrEqvSkip(lp, "egtbpaths ")) {
      UcciComm.Option = UCCI_OPTION_EGTBPATHS;
      UcciComm.szOption = lp;

    // (8) "evalapi"選項
    } else if (StrEqvSkip(lp, "evalapi ")) {
      UcciComm.Option = UCCI_OPTION_EVALAPI;
      UcciComm.szOption = lp;

    // (9) "hashsize"選項
    } else if (StrEqvSkip(lp, "hashsize ")) {
      UcciComm.Option = UCCI_OPTION_HASHSIZE;
      UcciComm.nSpin = Str2Digit(lp, 0, 1024);

    // (10) "threads"選項
    } else if (StrEqvSkip(lp, "threads ")) {
      UcciComm.Option = UCCI_OPTION_THREADS;
      UcciComm.nSpin = Str2Digit(lp, 0, 32);

    // (11) "promotion"選項
    } else if (StrEqvSkip(lp, "promotion ")) {
      UcciComm.Option = UCCI_OPTION_PROMOTION;
      if (StrEqv(lp, "on")) {
        UcciComm.bCheck = TRUE;
      } else if (StrEqv(lp, "true")) {
        UcciComm.bCheck = TRUE;
      } else {
        UcciComm.bCheck = FALSE;
      }

    // (12) "idle"選項
    } else if (StrEqvSkip(lp, "idle ")) {
      UcciComm.Option = UCCI_OPTION_IDLE;
      if (FALSE) {
      } else if (StrEqv(lp, "none")) {
        UcciComm.Grade = UCCI_GRADE_NONE;
      } else if (StrEqv(lp, "small")) {
        UcciComm.Grade = UCCI_GRADE_SMALL;
      } else if (StrEqv(lp, "medium")) {
        UcciComm.Grade = UCCI_GRADE_MEDIUM;
      } else if (StrEqv(lp, "large")) {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      } else {
        UcciComm.Grade = UCCI_GRADE_NONE;
      }

    // (13) "pruning"選項
    } else if (StrEqvSkip(lp, "pruning ")) {
      UcciComm.Option = UCCI_OPTION_PRUNING;
      if (FALSE) {
      } else if (StrEqv(lp, "none")) {
        UcciComm.Grade = UCCI_GRADE_NONE;
      } else if (StrEqv(lp, "small")) {
        UcciComm.Grade = UCCI_GRADE_SMALL;
      } else if (StrEqv(lp, "medium")) {
        UcciComm.Grade = UCCI_GRADE_MEDIUM;
      } else if (StrEqv(lp, "large")) {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      } else {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      }

    // (14) "knowledge"選項
    } else if (StrEqvSkip(lp, "knowledge ")) {
      UcciComm.Option = UCCI_OPTION_KNOWLEDGE;
      if (FALSE) {
      } else if (StrEqv(lp, "none")) {
        UcciComm.Grade = UCCI_GRADE_NONE;
      } else if (StrEqv(lp, "small")) {
        UcciComm.Grade = UCCI_GRADE_SMALL;
      } else if (StrEqv(lp, "medium")) {
        UcciComm.Grade = UCCI_GRADE_MEDIUM;
      } else if (StrEqv(lp, "large")) {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      } else {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      }

    // (15) "randomness"選項
    } else if (StrEqvSkip(lp, "randomness ")) {
      UcciComm.Option = UCCI_OPTION_RANDOMNESS;
      if (FALSE) {
      } else if (StrEqv(lp, "none")) {
        UcciComm.Grade = UCCI_GRADE_NONE;
      } else if (StrEqv(lp, "small")) {
        UcciComm.Grade = UCCI_GRADE_SMALL;
      } else if (StrEqv(lp, "medium")) {
        UcciComm.Grade = UCCI_GRADE_MEDIUM;
      } else if (StrEqv(lp, "large")) {
        UcciComm.Grade = UCCI_GRADE_LARGE;
      } else {
        UcciComm.Grade = UCCI_GRADE_NONE;
      }

    // (16) "style"選項
    } else if (StrEqvSkip(lp, "style ")) {
      UcciComm.Option = UCCI_OPTION_STYLE;
      if (FALSE) {
      } else if (StrEqv(lp, "solid")) {
        UcciComm.Style = UCCI_STYLE_SOLID;
      } else if (StrEqv(lp, "normal")) {
        UcciComm.Style = UCCI_STYLE_NORMAL;
      } else if (StrEqv(lp, "risky")) {
        UcciComm.Style = UCCI_STYLE_RISKY;
      } else {
        UcciComm.Style = UCCI_STYLE_NORMAL;
      }

    // (17) "newgame"選項
    } else if (StrEqv(lp, "newgame")) {
      UcciComm.Option = UCCI_OPTION_NEWGAME;

    // (18) 無法識別的選項，有擴充的余地
    } else {
      UcciComm.Option = UCCI_OPTION_UNKNOWN;
    }
    return UCCI_COMM_SETOPTION;

  // 3. "position {<special_position> | fen <fen_string>} [moves <move_list>]"指令
  } else if (StrEqvSkip(lp, "position ")) {
    return ParsePos(UcciComm, lp) ? UCCI_COMM_POSITION : UCCI_COMM_UNKNOWN;

  // 4. "banmoves <move_list>"指令，處理起來和"position ... moves ..."是一樣的
  } else if (StrEqvSkip(lp, "banmoves ")) {
    UcciComm.nBanMoveNum = MIN((int) (strlen(lp) + 1) / 5, MAX_MOVE_NUM);
    for (i = 0; i < UcciComm.nBanMoveNum; i ++) {
      dwCoordList[i] = *(uint32 *) lp;
      lp += sizeof(uint32) + 1;
    }
    UcciComm.lpdwBanMovesCoord = dwCoordList;
    return UCCI_COMM_BANMOVES;    
      
  // 5. "go [ponder | draw] <mode>"指令
  } else if (StrEqvSkip(lp, "go ")) {
    UcciComm.bPonder = UcciComm.bDraw = FALSE;
    // 首先判斷到底是"go"、"go ponder"還是"go draw"
    if (StrEqvSkip(lp, "ponder ")) {
      UcciComm.bPonder = TRUE;
    } else if (StrEqvSkip(lp, "draw ")) {
      UcciComm.bDraw = TRUE;
    }
    // 然後判斷思考模式
    bGoTime = FALSE;
    if (FALSE) {
    } else if (StrEqvSkip(lp, "depth ")) {
      UcciComm.Go = UCCI_GO_DEPTH;
      UcciComm.nDepth = Str2Digit(lp, 0, UCCI_MAX_DEPTH);
    } else if (StrEqvSkip(lp, "nodes ")) {
      UcciComm.Go = UCCI_GO_NODES;
      UcciComm.nDepth = Str2Digit(lp, 0, 2000000000);
    } else if (StrEqvSkip(lp, "time ")) {
      UcciComm.nTime = Str2Digit(lp, 0, 2000000000);
      bGoTime = TRUE;

    // 如果沒有說明是固定深度還是設定時限，就固定深度為"UCCI_MAX_DEPTH"
    } else {
      UcciComm.Go = UCCI_GO_DEPTH;
      UcciComm.nDepth = UCCI_MAX_DEPTH;
      //1892m - go infinity          
    }
    // 如果是設定時限，就要判斷是時段制還是加時制
    if (bGoTime) {
      if (FALSE) {
      } else if (StrScanSkip(lp, (char*)" movestogo ")) {
        UcciComm.Go = UCCI_GO_TIME_MOVESTOGO;
        UcciComm.nMovesToGo = Str2Digit(lp, 1, 999);
      } else if (StrScanSkip(lp, (char*)" increment ")) {
        UcciComm.Go = UCCI_GO_TIME_INCREMENT;
        UcciComm.nIncrement = Str2Digit(lp, 0, 999999);
//20190922 - get opptime      
        if (StrScanSkip(lp, (char*)" opptime ")) {
        UcciComm.nOpptime = Str2Digit(lp, 0, 2000000000);
        }        
        
      // 如果沒有說明是時段制還是加時制，就設定為步數是1的時段制
      } else {
        UcciComm.Go = UCCI_GO_TIME_MOVESTOGO;
        //1892m - go infinity 
        if (UcciComm.nDepth == UCCI_MAX_DEPTH)
        	UcciComm.nMovesToGo = 999;
        else	
        UcciComm.nMovesToGo = 1;
      }
    }
    return UCCI_COMM_GO;

  // 6. "stop"指令
  } else if (StrEqv(lp, "stop")) {
    return UCCI_COMM_STOP;

  // 7. "probe {<special_position> | fen <fen_string>} [moves <move_list>]"指令
  } else if (StrEqvSkip(lp, "probe ")) {
    return ParsePos(UcciComm, lp) ? UCCI_COMM_PROBE : UCCI_COMM_UNKNOWN;

  // 8. "quit"指令
  } else if (StrEqv(lp, "quit")) {
    return UCCI_COMM_QUIT;

  // 9. 無法識別的指令
  } else {
    return UCCI_COMM_UNKNOWN;
  }
}

UcciCommEnum BusyLine(UcciCommStruct &UcciComm, Bool bDebug) {
  char szLineStr[LINE_INPUT_MAX_CHAR];
  char *lp;
  if (pipeStd.LineInput(szLineStr)) {
    if (bDebug) {
      printf("info busyline [%s]\n", szLineStr);
      fflush(stdout);
    }
    // "BusyLine"隻能接收"isready"、"ponderhit"和"stop"這三條指令
    if (FALSE) {
    } else if (StrEqv(szLineStr, "isready")) {
      return UCCI_COMM_ISREADY;
    } else if (StrEqv(szLineStr, "ponderhit draw")) {
      return UCCI_COMM_PONDERHIT_DRAW;
    // 注意：必須首先判斷"ponderhit draw"，再判斷"ponderhit"
    } else if (StrEqv(szLineStr, "ponderhit")) {
      return UCCI_COMM_PONDERHIT;
    } else if (StrEqv(szLineStr, "stop")) {
      return UCCI_COMM_STOP;
    } else if (StrEqv(szLineStr, "quit")) {
      return UCCI_COMM_QUIT;
    } else {
      lp = szLineStr;
      if (StrEqvSkip(lp, "probe ")) {
        return ParsePos(UcciComm, lp) ? UCCI_COMM_PROBE : UCCI_COMM_UNKNOWN;
      } else {
        return UCCI_COMM_UNKNOWN;
      }
    }
  } else {
    return UCCI_COMM_UNKNOWN;
  }
}
