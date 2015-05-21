/*
 *  mechanicalTurk
 *
 *  Trades smart, builds dumb and etc.
 *
 *  Proudly Created by Moss and Anton
 *  Share Freely Creative Commons SA-BY-NC 3.0. 
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
 
#include "Game.h"
#include "mechanicalTurk.h"
 
#define DEFAULT_DISCIPLINES { STUDENT_BQN, STUDENT_MMONEY, STUDENT_MJ, \
                STUDENT_MMONEY, STUDENT_MJ, STUDENT_BPS, STUDENT_MTV, \
                STUDENT_MTV, STUDENT_BPS,STUDENT_MTV, STUDENT_BQN, \
                STUDENT_MJ, STUDENT_BQN, STUDENT_THD, STUDENT_MJ, \
                STUDENT_MMONEY, STUDENT_MTV, STUDENT_BQN, STUDENT_BPS }
#define DEFAULT_DICE {9,10,8,12,6,5,3,11,3,11,4,6,4,7,9,2,8,10,5}

typedef struct _res { // resources struct
    int BPS;
    int BQN;
    int THD;
    int MJ;
    int MTV;
    int MMONEY;
} res;

typedef struct _res * Res; // pointer to a _res struct

action smartTrading (Res myRes, int actionCode);
action dumbTrading (int resource);
char* stringParser(int player, int vertice);
//int resourceCheck(int a, int b, int c, int d, int e, int f);
action dumbBuilding (path destination, Game g);
Res getMyRes (Game g);
void testSmartTrading (Game g);

action decideAction (Game g) {
    //int mj;
    //int mm;
    //int mtv;
    action nextAction;
    // mj = getStudents (g, UNI_A, STUDENT_MJ);
    // mm = getStudents (g, UNI_A, STUDENT_MMONEY);
    // mtv = getStudents (g, UNI_A, STUDENT_MTV);
    // if(mj >= 1 && mm >= 1 && mtv >= 1) {
    
    nextAction.actionCode = START_SPINOFF;        
    
    Res myRes;
    myRes = getMyRes(g);
    // if we can't start a spinoff right now then use your head!
    // LATER: smartbuilding etc. may be a priority over the start spinoff
    // so we need to put them above this check
    if (isLegalAction(g, nextAction) != TRUE) {
        // thought 1: how about we smart trade?
        nextAction = smartTrading(myRes, START_SPINOFF);
    }
    return nextAction;
}

int main (int argc, char *argv[]) {
    
    int disciplines[] = DEFAULT_DISCIPLINES;
    int diceScores[] = DEFAULT_DICE;
    //Create a new game to test on
    Game testGame = newGame (disciplines, diceScores);
    
    printf("==== Tests start ====\n");
    
    testSmartTrading(testGame);
    
    printf ("All AI tests passed. You are awesome!\n");

    return EXIT_SUCCESS;
}

// Trades Only whenever enough to complete action succesfully
action smartTrading (Res myRes, int actionCode) {

    action nextAction;
    
    // Start spinoff smart trading:
    // Need 1 MJ, 1 MTV, 1M$
    // Know that MTV and M$ dissapear on dice score being 7
    // Logic:
    // Aim to be able to trade for the goal in one go with no passing, i.e. start any trading to MTV/MMONEY only when 
    // have sufficient amounts to be able to finish the trading and spinoff in this turn
    // If that's not the case then check these:
    // If you have 3 MTV or 3 M$ and no MJ - trade for MJ since it's safe
    if (actionCode == START_SPINOFF) {
        // have all resources already - just start spinoff
        if(myRes->MJ >= 1 && myRes->MTV >= 1 && myRes->MMONEY >= 1) {
            nextAction.actionCode = START_SPINOFF;
        }
        // otherwise may try to retrain
        else {
            nextAction.actionCode = RETRAIN_STUDENTS;        
        }
        // check if you can do trading to achieve 1 MJ, 1 MTV, 1 M$ in one go from now
        // BEHOLD! THE SPAGHETTI CODE IFS!!!
        // if no MTVs
        if(myRes->MTV == 0) {
            // if no MMONEYs
            if(myRes->MMONEY == 0) { 
                // if no MJs - try to trade for MJs
                if(myRes->MJ == 0) { 
                    // none of the kinds exist, check if you have enough of other
                    // convertable kinds
                    // to trade for MJ+MTV+MMONEY (one of each)
                    if(
                        (myRes->BPS >= 9) ||
                        (myRes->BPS >= 6 && myRes->BQN >= 3) ||
                        (myRes->BPS >= 3 && myRes->BQN >= 6) ||
                        (myRes->BQN >= 9)
                    ) {
                        // enough BPS? - trade them
                        if(myRes->BPS >= 3) {
                            nextAction.disciplineFrom = STUDENT_BPS;
                        }
                        // else, enough BQN? - trade them
                        else if(myRes->BQN >= 3) {
                            nextAction.disciplineFrom = STUDENT_BQN;
                        }
                        nextAction.disciplineTo = STUDENT_MJ;
                    }
                    // nope, not enough of other kinds to support the
                    // total trade - just PASS
                    else {
                        nextAction.actionCode = PASS;
                    }
                }
                // there are MJs - try to trade for MMONEYs
                else {
                    if(
                        // either has 6
                        (myRes->BPS >= 6) ||
                        (myRes->BQN >= 6) ||
                        (myRes->MJ >= 6) ||
                        // either pair has 3
                        (myRes->BPS >= 3 && myRes->BQN >= 3) ||
                        (myRes->BPS >= 3 && myRes->MJ >= 3) ||
                        (myRes->BQN >= 3 && myRes->MJ >= 3)
                    ) {
                        // enough BPS? - trade them
                        if(myRes->BPS >= 3) {
                            nextAction.disciplineFrom = STUDENT_BPS;
                        }
                        // else, enough BQN? - trade them
                        else if(myRes->BQN >= 3) {
                            nextAction.disciplineFrom = STUDENT_BQN;
                        }
                        // else, enough MJ? - trade them
                        else if(myRes->MJ >= 3) {
                            nextAction.disciplineFrom = STUDENT_MJ;
                        }
                        nextAction.disciplineTo = STUDENT_MMONEY;
                    }
                    // nope, not enough to retrain for MTV and MMONEY
                    // at once - so just PASS!
                    else {
                        nextAction.actionCode = PASS;
                    }
                }
            }
            // there are MMONEYs
            else {
                // means the only thing left is to trade for MTVs
                // enough BPS? - trade them
                if(myRes->BPS >= 3) {
                    nextAction.disciplineFrom = STUDENT_BPS;
                }
                // else, enough BQN? - trade them
                else if(myRes->BQN >= 3) {
                    nextAction.disciplineFrom = STUDENT_BQN;
                }
                // else, enough MJ? - trade them
                else if(myRes->MJ >= 3) {
                    nextAction.disciplineFrom = STUDENT_MJ;
                }
                // else, enough MMONEYs? - trade them
                else if(myRes->MMONEY >= 3) {
                    nextAction.disciplineFrom = STUDENT_MMONEY;
                }
                else {
                    nextAction.actionCode = PASS;
                }
                nextAction.disciplineTo = STUDENT_MTV;
            }
        } // endif No MTVs
        // if all else failed - we can't trade all of the stuff in one go yet
        if(nextAction.actionCode == PASS) {
            // if you have enough MTVs - trade them for MJ
            if(myRes->MTV >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MTV;
                nextAction.disciplineTo = STUDENT_MJ;
            }
            else if(myRes->MMONEY >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MMONEY;
                nextAction.disciplineTo = STUDENT_MJ;
            }
        } // endif PASS
    } // endif START_SPINOFF

    return nextAction;
}

// Trades for resource whenever there is enough resources to
// do so succesfully
action dumbTrading(int resource) {
    action nextAction;
    return nextAction;
}

// Tries to progressively build a campus at the destination
action dumbBuilding(path destination, Game g) {
    action nextAction;
    return nextAction;
}

// Gives you the path to the vertice for a given player.
/*
char* stringParser(int player, int vertice) {
    char destination[] = {'R', 'L'};
    
    TODO Moss: fails to compile
    
    if ((player  == 1) && (vertice == 1)) {
        char destination[] = {'R', 'L'};
    }
    if ((player  == 3) && (vertice == 1)) {
        char destination[] = {'L', 'R', 'L', 'R', 'R'};
    }
    // Supplies Position
    if ((player  == 3) && (vertice == 1)) {
        char destination[] = {'L', 'R', 'L', 'R', 'L', 'R', 'R', 'L', 'R', 'R'};
    }
    
    return destination;
}
*/

// Checks if we have resources
/*int resourceCheck(Game g, int ThD, int BPS, int BQN,
    int MJ, int MTV, int MM) {
    if (TESTING) {
    } else if (TESTING) {
    }
    return
}*/

// we have this function so we can fake our resources and test the AI decision
Res getMyRes (Game g) {
    // Create a Res
    Res r;
    r = malloc (sizeof (struct _res));
    assert(r != NULL);
    
    int player = getWhoseTurn (g);
    if(player != 0) {
        r->BPS = getStudents (g, player, STUDENT_BPS);
        r->BQN = getStudents (g, player, STUDENT_BQN);
        r->THD = getStudents (g, player, STUDENT_THD);
        r->MJ = getStudents (g, player, STUDENT_MJ);
        r->MTV = getStudents (g, player, STUDENT_MTV);
        r->MMONEY = getStudents (g, player, STUDENT_MMONEY);
    }
    return r;
}

void testSmartTrading(Game g) {
    printf("testSmartTrading start\n");
    throwDice(g,7);
    Res myRes = getMyRes(g);
    action newAction;    
    
    // START SPINOFF driven smart trading tests
    // Tests all situations by faking the amount of students
    printf("Test 1 - have enough, should start spinoff\n");
    myRes->MJ = 1;
    myRes->MTV = 1;
    myRes->MMONEY = 1;
    newAction = smartTrading(myRes, START_SPINOFF);
    assert(newAction.actionCode == START_SPINOFF);

    // could trade BPS - but there is no point since you can't get 1-1-1 on this turn anyway
    printf("Test 2 - not enough and no way to trade in one go, should PASS\n");
    myRes->BPS = 4;
    myRes->BQN = 2;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 2;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == PASS);

    // without passing
    // 1 turn - trade BPS for MJ
    // 2 turn - trade BQN for MMONEY
    // 3 turn - spinoff
    printf("Test 3 - Can trade all in this turn, start in order MJ->MTV->MMONEY\n");
    myRes->BPS = 3;
    myRes->BQN = 3;
    myRes->MJ = 0;
    myRes->MTV = 2;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_BPS);
    assert (newAction.disciplineTo == STUDENT_MJ);

    // LATER: should trade for something out of BPS/BQN/MJ we don't have supply of!
    printf("Test 4 - can trade 3 MTV for anything, should trade for MJ \n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 2;
    myRes->MTV = 3;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MTV);
    assert (newAction.disciplineTo == STUDENT_MTV);

    // if there are 7 MJs - we definitely have resources to trade for spinoff in one go
    printf("Test 5 - Have 7 MJ, 0 MTV should trade for MTV\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 1;
    myRes->MTV = 4;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MTV);
    assert (newAction.disciplineTo == STUDENT_MMONEY);

    // can't get in one go - just PASS
    printf("Test 6 - Have 3 MJ, 0 MTV, 0 MMONEY should PASS\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 3;
    myRes->MTV = 0;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == PASS);
    
    printf("testSmartTrading end\n");
}