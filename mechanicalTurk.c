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
#include <string.h>
#include <math.h>

#include "Game.h"
#include "mechanicalTurk.h"


#define START_1A "RL"
#define START_3A "LRLRRLLLLLL"
#define START_2A "LRLRRLRLRLLLLL"
#define START_1B "LRLRRLRLRRLRLLLLL"
#define START_3B "LRLRRLRLRRLRRLRLLLLL"
#define START_2B "LRLRRLRLRRLRRLRRLRLLLLL"

#define POINT_A ""
#define POINT_B "L"
#define POINT_C "LR"
#define POINT_D "LRR"
#define POINT_E "LRRR"
#define POINT_F "LRRRR"

//Player 1 Vertice 1A
#define PLAYER_1_VERT_1A1 3
#define PLAYER_1_VERT_1A2 7
#define PLAYER_1_VERT_1A3 8
//Player 1 Vertice 2A
#define PLAYER_1_VERT_2A1 8
#define PLAYER_1_VERT_2A2 8
#define PLAYER_1_VERT_2A3 12

//Player 2 Vertice 1A
#define PLAYER_2_VERT_1A1 14
#define PLAYER_2_VERT_1A2 17
#define PLAYER_2_VERT_1A3 18
//Player 2 Vertice 2A
#define PLAYER_2_VERT_2A1 14
#define PLAYER_2_VERT_2A2 15
#define PLAYER_2_VERT_2A3 18

//Player 3 Vertice 1A
#define PLAYER_3_VERT_1A1 12
#define PLAYER_3_VERT_1A2 13
#define PLAYER_3_VERT_1A3 16
//Player 3 Vertice 2A
#define PLAYER_3_VERT_2A1 13
#define PLAYER_3_VERT_2A2 16
#define PLAYER_3_VERT_2A3 17

//Player 1 Vertice 1B
#define PLAYER_1_VERT_1B1 11
#define PLAYER_1_VERT_1B2 10
#define PLAYER_1_VERT_1B3 15
//Player 1 Vertice 2B
#define PLAYER_1_VERT_2B1 11
#define PLAYER_1_VERT_2B2 10
#define PLAYER_1_VERT_2B3 6

//Player 2 Vertice 1B
#define PLAYER_2_VERT_1B1 0
#define PLAYER_2_VERT_1B2 4
#define PLAYER_2_VERT_1B3 1
//Player 2 Vertice 2B
#define PLAYER_2_VERT_2B1 0
#define PLAYER_2_VERT_2B2 4
#define PLAYER_2_VERT_2B3 3

//Player 3 Vertice 1B
#define PLAYER_3_VERT_1B1 2
#define PLAYER_3_VERT_1B2 5
#define PLAYER_3_VERT_1B3 6
//Player 3 Vertice 2B
#define PLAYER_3_VERT_2B1 2
#define PLAYER_3_VERT_2B2 5
#define PLAYER_3_VERT_2B3 1

//Define Regions Ranks
#define RANK_STUDENT_BPS 3
#define RANK_STUDENT_BQN 3
#define RANK_STUDENT_MJ 3
#define RANK_STUDENT_MMONEY 2
#define RANK_STUDENT_MTV 2
#define RANK_STUDENT_THD 0

#define FIRST_POINT 1
#define FIRST_SIDE 1

#define MAX_ATTEMPTS 5
#define NUM_CAMPUS 3

 
#define DEFAULT_DISCIPLINES {STUDENT_BQN, STUDENT_MMONEY, STUDENT_MJ, \
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

typedef struct _build {
    int point;
    int side;
} build;



action smartTrading (Res myRes, int actionCode);
action dumbTrading (int resource);
char* stringParser(int player, int vertice, int point);
//int resourceCheck(int a, int b, int c, int d, int e, int f);
action dumbBuilding (path destination, Game g, int vertice);
Res getMyRes (Game g);
action enoughToTradeThree(action a, Res myRes, int kind);
action enoughToTradeTwo(action a, Res myRes, int kind, int altRes, int altKind);
action enoughToTradeOne(action a, Res myRes, int kind, int altRes, int altKind);
void testStringParser(void);
void testDumbBuilding(Game g);
void testSmartTrading (Game g);
char* kindToName(int kind);
build buildDecider(Game g, int player);
void testRegionRanker(void);
int regionRanker(int Discipline);
void testBuildDecider(Game g);
int max(int num1, int num2);


action decideAction (Game g) {

    //int mj;
    //int mm;
    //int mtv;
    // mj = getStudents (g, UNI_A, STUDENT_MJ);
    // mm = getStudents (g, UNI_A, STUDENT_MMONEY);
    // mtv = getStudents (g, UNI_A, STUDENT_MTV);
    // if(mj >= 1 && mm >= 1 && mtv >= 1) {
    
    int player = getWhoseTurn(g);
    int attempts = 1;
    action nextAction;
    build decision;
    Res myRes;
    myRes = getMyRes(g);
    int excessTrade = 0;
    
    //Check if we have less campuses then we want, keep getting more.
    if (getCampuses(g, player) < NUM_CAMPUS) {
        decision = buildDecider(g, player);
        char *pathCampus = stringParser(player,
                                        decision.point, decision.side);
        if ((getCampus(g,pathCampus) != player) &&
            (attempts <= MAX_ATTEMPTS)) {
            //Thought 1 We start buiilding
            printf("Attempt: %d",attempts);
            nextAction = dumbBuilding(pathCampus, g, decision.point);
            //Check if we have enough resoures for this action.
            
            if (nextAction.actionCode == OBTAIN_ARC &&
                myRes->BPS > 0 &&
                myRes->BQN > 0) {
                printf("Enough Resources to build ARC\n");
                //Not enough resources. Try and trade BPS first
            } else if (myRes->BPS == 0){
                if (myRes->MTV > 3) {
                    excessTrade = STUDENT_MTV;
                } else if (myRes->BQN > 5) {
                    excessTrade = STUDENT_BQN;
                } else if (myRes->MJ > 5) {
                    excessTrade = STUDENT_MJ;
                } else if (myRes->MMONEY > 5) {
                    excessTrade = STUDENT_MMONEY;
                } else if (myRes->BPS > 5) {
                    excessTrade = STUDENT_BPS;
                }
                nextAction = enoughToTradeOne(nextAction,
                    myRes, STUDENT_BPS,excessTrade, excessTrade);
                
            } else if (myRes->BQN == 0){
                if (myRes->MTV > 3) {
                    excessTrade = STUDENT_MTV;
                } else if (myRes->BQN > 5) {
                    excessTrade = STUDENT_BQN;
                } else if (myRes->MJ > 5) {
                    excessTrade = STUDENT_MJ;
                } else if (myRes->MMONEY > 5) {
                    excessTrade = STUDENT_MMONEY;
                } else if (myRes->BPS > 5) {
                    excessTrade = STUDENT_BPS;
                }
                nextAction = enoughToTradeOne(nextAction,
                        myRes, STUDENT_BQN,excessTrade, excessTrade);
                
            } else if (nextAction.actionCode == BUILD_CAMPUS &&
                myRes->BPS > 0 && myRes->BQN > 0 && myRes->MJ > 0 &&
                myRes->MTV > 0) {
                printf("Enough Resources to build Campus\n");
                
            } else {
                //Trade for things to build
                printf("Trying to trade\n");
            }
            
            //Check everything is legal
            if (isLegalAction(g, nextAction) != TRUE){
                printf("Action isn't Legal\n");
                nextAction.actionCode = PASS;
            }
        }
    } else {
    //Thought 2 We start spinning
    nextAction.actionCode = START_SPINOFF;
        if (isLegalAction(g, nextAction) != TRUE) {
            // thought 1: how about we smart trade?
            nextAction = smartTrading(myRes, START_SPINOFF);
        }
    }
    return nextAction;
}

/*
int main (int argc, char *argv[]) {
    
    int disciplines[] = DEFAULT_DISCIPLINES;
    int diceScores[] = DEFAULT_DICE;
    
    
    //Create a new game to test on
    Game testGame = newGame (disciplines, diceScores);
    
    printf("==== Tests start ====\n");
    
    testRegionRanker();
    testSmartTrading(testGame);
    testStringParser();
    testDumbBuilding(testGame);
    testBuildDecider(testGame);
    
    printf ("All AI tests passed. You are awesome!\n");

    return EXIT_SUCCESS;
}
*/

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
        nextAction.actionCode = PASS;
        // have all resources already - just start spinoff
        if(myRes->MJ >= 1 && myRes->MTV >= 1 && myRes->MMONEY >= 1) {
            printf("> Enough to spinoff\n");
            nextAction.actionCode = START_SPINOFF;
        }
        // check if you can do trading to achieve 1 MJ, 1 MTV, 1 M$ in one go from now
        // BEHOLD! THE SPAGHETTI CODE IFS!!!
        
        /* * *
        * Enough to trade for three kinds
        */
        // if none present - need to trade for three
        if(myRes->MTV < 1 && myRes->MMONEY < 1 && myRes->MJ < 1) {
            // trade for MJs first
            nextAction = enoughToTradeThree(nextAction, myRes, STUDENT_MJ); 
            // Note: after this action we will only have enough to trade 2
            // but there will be MJs now
        } 
        
        /* * *
        * Enough to trade for two kinds
        */
        // we have MJs but no MTVs and no MMONEYs
        else if(myRes->MJ >= 1 && myRes->MTV < 1 && myRes->MMONEY < 1) {
            // trade for MTVs first (no particular reason)
            nextAction = enoughToTradeTwo(nextAction, myRes, STUDENT_MTV, myRes->MJ, STUDENT_MJ); 
        }
        // we have MTVs no MJs and no MMONEYs
        else if(myRes->MTV >=1 && myRes->MJ < 1 && myRes->MMONEY < 1) {
            // trade for MJs, may trade MTVs
            nextAction = enoughToTradeTwo(nextAction, myRes, STUDENT_MJ, myRes->MTV, STUDENT_MTV);
        }
        // we have MMONEYs no MJs and no MTVs
        else if(myRes->MMONEY >= 1 && myRes->MJ < 1 && myRes->MTV < 1) {
            // trade for MJs, may trade MMONEYs
            nextAction = enoughToTradeTwo(nextAction, myRes, STUDENT_MJ, myRes->MMONEY, STUDENT_MMONEY); 
        }

        /* * *
        * Trading for one kind
        */
        // we have MJs and MTVs and no MMONEYs
        else if(myRes->MJ >= 1 && myRes->MTV >= 1 && myRes->MMONEY < 1) {
            // trade for MMONEYs
            nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MMONEY, myRes->MJ, STUDENT_MJ); 
            // not possible? try to use MTVs as alternative
            if(nextAction.actionCode == PASS) {
                nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MMONEY, myRes->MTV, STUDENT_MTV); 
            }
        }
        // we have MJs and MMONEYs but no MTVs
        else if(myRes->MJ >= 1 && myRes->MMONEY >= 1 && myRes->MTV < 1) {
            // trade for MTVs
            nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MTV, myRes->MJ, STUDENT_MJ); 
            // not possible? try to use MMONEYs as alternative
            if(nextAction.actionCode == PASS) {
                nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MTV, myRes->MMONEY, STUDENT_MMONEY); 
            }
        }
        // we have MTVs and MMONEYs but no MJs (suddenly)
        else if(myRes->MTV >= 1 && myRes->MMONEY >= 1 && myRes->MJ < 1) {
            // trade for MJs
            nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MJ, myRes->MTV, STUDENT_MTV); 
            // not possible? try to use MMONEYs as alternative
            if(nextAction.actionCode == PASS) {
                nextAction = enoughToTradeOne(nextAction, myRes, STUDENT_MJ, myRes->MMONEY, STUDENT_MMONEY); 
            }
        }
        // if all else failed - we can't trade all of the stuff in one go yet
        if(nextAction.actionCode == PASS) {
            printf("> Can't trade in one go yet...\n");
        } // endif PASS
    } // endif START_SPINOFF

    return nextAction;
}

// checks if there is enough resources to trade all three kinds
action enoughToTradeThree(action a, Res myRes, int kind) {
    printf("> Enough to trade three?\n");
    printf("> Attempt to trade BPS, BQN for %s\n", kindToName(kind));
    if(
        (myRes->BPS >= 9) ||
        (myRes->BPS >= 6 && myRes->BQN >= 3) ||
        (myRes->BPS >= 3 && myRes->BQN >= 6) ||
        (myRes->BQN >= 9)
    ) {
        printf("- YES\n");
        a.actionCode = RETRAIN_STUDENTS;
        // enough BPS? - trade them
        if(myRes->BPS >= 3) {
            a.disciplineFrom = STUDENT_BPS;
        }
        // else, enough BQN? - trade them
        else if(myRes->BQN >= 3) {
            a.disciplineFrom = STUDENT_BQN;
        }
        a.disciplineTo = kind;
    }
    // nope, not enough of other kinds to support the
    // total trade - just PASS
    else {
        printf("- NO\n");
        a.actionCode = PASS;
    }
    return a;
}

// checks if there is enough resources to trade two kinds
// takes altRes and altKind to use alternative kind if BPS/BQN are not sufficient
action enoughToTradeTwo(action a, Res myRes, int kind, int altRes, int altKind) {
    printf("> Enough to trade two?\n");
    printf("> Attempt to trade BPS, BQN or %s for %s\n", kindToName(altKind), kindToName(kind));
    if(
        // either has 6
        (myRes->BPS >= 6) ||
        (myRes->BQN >= 6) ||
        (altRes >= 6) ||
        // either pair has 3
        (myRes->BPS >= 3 && myRes->BQN >= 3) ||
        (myRes->BPS >= 3 && altRes >= 3) ||
        (myRes->BQN >= 3 && altRes >= 3)
    ) {
        printf("- YES\n");
        a.actionCode = RETRAIN_STUDENTS;
        // enough BPS? - trade them
        if(myRes->BPS >= 3) {
            a.disciplineFrom = STUDENT_BPS;
        }
        // else, enough BQN? - trade them
        else if(myRes->BQN >= 3) {
            a.disciplineFrom = STUDENT_BQN;
        }
        // else, enough MJ? - trade them
        else if(altRes >= 4) {
            printf("> %s >= 4\n", kindToName(altKind));
            a.disciplineFrom = altKind;
        }
        a.disciplineTo = kind;
    }
    // nope, not enough to retrain for MTV and MMONEY
    // at once - so just PASS!
    else {
        printf("> Not enough to trade for two\n");
        a.actionCode = PASS;
    }
    return a;    
}

// checks if there is enough to trade for one kind of students
// takes altRes and altKind to use alternative kind if BPS/BQN are not sufficient
action enoughToTradeOne(action a, Res myRes, int kind, int altRes, int altKind) {
    printf("> Enough to trade for one?\n");
    printf("> Attempt to trade BPS,BQN or %s for %s\n", kindToName(altKind), kindToName(kind));
    a.actionCode = RETRAIN_STUDENTS;
    // enough BPS? - trade them
    if(myRes->BPS >= 3) {
        printf("> BPS >= 3\n");
        a.disciplineFrom = STUDENT_BPS;
    }
    // else, enough BQN? - trade them
    else if(myRes->BQN >= 3) {
        printf("> BPS >= 3\n");
        a.disciplineFrom = STUDENT_BQN;
    }
    // else, enough altRes? - trade them
    else if(altRes >= 4) {
        printf("> %s >= 4\n", kindToName(altKind));
        a.disciplineFrom = altKind;
    }
    else {
        printf("> Not enough of any kind to trade\n");
        a.actionCode = PASS;
    }
    a.disciplineTo = kind;
    return a;
}

// Trades for resource whenever there is enough resources to
// do so succesfully
action dumbTrading(int resource) {
    action nextAction;
    return nextAction;
}

// Tries to progressively build a campus at the destination
action dumbBuilding(path destination, Game g, int vertice) {
    action nextAction;
    int length = 0;
    int player = getWhoseTurn(g);
    int counter = 0;
    int end = 0;
    int flag = TRUE;
    
    //Later decide if A or B is better
    
    //retrieve the path for the first arc that you need
    length = (int) strlen(destination);
    
    //From the start of the path start going through and seeing if we
    //own it.
    while (flag == TRUE) {
        end  = length - vertice + counter;
        printf("\n#########\n");
        printf("Building the road %d from destination\n", counter - vertice);
        strcpy(nextAction.destination, destination);
        nextAction.destination[end] = '\0';
        nextAction.actionCode = OBTAIN_ARC;
        //If we don't own it stop - we want to buy an arc.
        if (getARC(g, nextAction.destination) != player) {
            flag = FALSE;
            printf("We don't own the arc: %s\n", nextAction.destination);
        } else {
            printf("Already Owned\n");
        }
        
        if ((strlen(destination) == strlen(nextAction.destination))
            && (getARC(g, nextAction.destination) == player)) {
            nextAction.actionCode = BUILD_CAMPUS;
            printf("Trying to Build Campus\n");
            flag = FALSE;
        }
        counter++;
    }


    printf("Building ARC: %s\n",
           nextAction.destination);
    
    return nextAction;
}

// Gives you the path to the vertice for a given player.
char* stringParser(int player, int vertice, int point) {
    char destination[PATH_LIMIT];
    int baselength = 0;
    int taillength = 0;
    
    //Effectively zeros the thing for each player
    if (point == 1){
        if (player == 1){
            strcpy(destination, START_1A);
            baselength = strlen(START_1A);
        } else if (player == 2){
            strcpy(destination, START_2A);
            baselength = strlen(START_2A);
        } else if (player == 3){
            strcpy(destination, START_3A);
            baselength = strlen(START_3A);
        }
    } else if (point == 2){
        if (player == 1){
            strcpy(destination, START_1B);
            baselength = strlen(START_1B);
        } else if (player == 2){
            strcpy(destination, START_2B);
            baselength = strlen(START_2B);
        } else if (player == 3){
            strcpy(destination, START_3B);
            baselength = strlen(START_3B);
        }
    }
    
    //Appends the vertice that we want to purchase
    if (vertice == 1) {
        strcpy(destination+baselength, POINT_A);
        taillength = 0;
    } else if (vertice == 2) {
        strcpy(destination+baselength, POINT_B);
        taillength = 1;
    } else if (vertice == 3) {
        strcpy(destination+baselength, POINT_C);
        taillength = 2;
    } else if (vertice == 4) {
        strcpy(destination+baselength, POINT_D);
        taillength = 3;
    } else if (vertice == 5) {
        strcpy(destination+baselength, POINT_E);
        taillength = 4;
    } else if (vertice == 6) {
        strcpy(destination+baselength, POINT_F);
        taillength = 5;

    }
    
    printf("The entire path is: %s\n", destination);

    
    char *result = malloc(baselength + taillength + 1);
    
    strcpy(result, destination);
    
    return result;
}

void testStringParser(void){
    printf("Starting the testing of String Parser\n");
    assert(strcmp(stringParser(1, 1, 1), START_1A)==0);
    assert(strcmp(stringParser(2, 1, 1), START_2A)==0);
    assert(strcmp(stringParser(3, 1, 1), START_3A)==0);
    assert(strcmp(stringParser(1, 1, 2), START_1B)==0);
    assert(strcmp(stringParser(2, 1, 2), START_2B)==0);
    assert(strcmp(stringParser(3, 1, 2), START_3B)==0);
    printf("String Parser testing complete\n");

}
char* kindToName(int kind) {
    char *result = malloc(7); // MMONEY is the longest
    if(kind == STUDENT_BPS) {
        strcpy(result, "BPS");
    }
    else if(kind == STUDENT_BQN) {
        strcpy(result, "BQN");
    }
    else if(kind == STUDENT_THD) {
        strcpy(result, "THD");
    }
    else if(kind == STUDENT_MJ) {
        strcpy(result, "MJ");
    }
    else if(kind == STUDENT_MTV) {
        strcpy(result, "MTV");
    }
    else if(kind == STUDENT_MMONEY) {
        strcpy(result, "MMONEY");
    }
    return result;
}

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

void testDumbBuilding(Game g){
    throwDice(g, 1);
    throwDice(g, 1);

    action newAction;
    int player = getWhoseTurn(g);
    int attempts = 1;
    int vertice = 1;
    printf("\n\n======= TESTING DUMB BUILD =========\n");
    printf("Players %d's turn\n", player);
    
    char *pathCampus = stringParser(player, vertice, 2);
    
    //Initiate the current owner of the vertice
    int owner = getCampus(g,pathCampus);
    printf("Trying to get Campus at location: %s\n", pathCampus);
    printf("Before owner: %d\n", getCampus(g,pathCampus));

    //Repeatedly call this until that campus is ours!
    while ((owner != player) && (attempts <= MAX_ATTEMPTS)) {
        printf("Attempt: %d\n", attempts);
        newAction = dumbBuilding(pathCampus, g,vertice);
        printf("\nCurrent Owner of ARC: %d\n",
               getARC(g,newAction.destination));
        
        if (isLegalAction(g, newAction)){
            printf("Starting MakeAction %d\n", newAction.actionCode);
            makeAction(g, newAction);
            printf("Finished MakeAction\n");
            if (newAction.actionCode == OBTAIN_ARC) {
            printf("\nNew Owner of ARC: %d\n",
                   getARC(g, newAction.destination));
            } else if (newAction.actionCode == BUILD_CAMPUS) {
                printf("\nNew Owner of Campus: %d\n",
                       getARC(g, newAction.destination));
            }
        }
        attempts++;
        owner = getCampus(g,pathCampus);
    }
    
    printf("\nAfter owner: %d\n", getCampus(g,pathCampus));
}

void testSmartTrading(Game g) {
    printf("testSmartTrading start\n");
    throwDice(g,7);
    Res myRes = getMyRes(g);
    action newAction;    
    
    // START SPINOFF driven smart trading tests
    // Tests all situations by faking the amount of students
    printf("[Test 1] - have enough, should start spinoff\n");
    myRes->MJ = 1;
    myRes->MTV = 1;
    myRes->MMONEY = 1;
    newAction = smartTrading(myRes, START_SPINOFF);
    assert(newAction.actionCode == START_SPINOFF);

    // could trade BPS - but there is no point since you can't get 1-1-1 on this turn anyway
    printf("[Test 2] - not enough and no way to trade in one go, should PASS\n");
    myRes->BPS = 4;
    myRes->BQN = 2;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 2;
    newAction = smartTrading (myRes, START_SPINOFF);
    printf("action? %d\n", newAction.actionCode);
    assert (newAction.actionCode == PASS);

    // without passing
    // 1 turn - trade BPS for MJ
    // 2 turn - trade BQN for MMONEY
    // 3 turn - spinoff
    printf("[Test 3] - Can trade all in this turn, start in order MJ->MTV->MMONEY\n");
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
    printf("[Test 4] - has 4 MTV should trade for MMONEY \n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 2;
    myRes->MTV = 4;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MTV);
    assert (newAction.disciplineTo == STUDENT_MMONEY);

    // if there are 7 MJs - we definitely have resources to trade for spinoff in one go
    printf("[Test 5] - Have 7 MJ, 0 MTV should trade for MTV\n");
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
    printf("[Test 6] - Have 3 MJ, 0 MTV, 0 MMONEY should PASS\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 3;
    myRes->MTV = 0;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == PASS);
    
    printf("testSmartTrading end\n");
}

// This function returns whether to build point at vertice 1|2 at A|B
build buildDecider(Game g, int player){
    build decision;
    int point1 = 0;
    int point2 = 0;
    int point3 = 0;
    int point4 = 0;
    int biggest = 0;
    
    if (player == 1) {
        //Get the value for the first point
        point1 = regionRanker(getDiscipline(g, PLAYER_1_VERT_1A1)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_1A2)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_1A3));
        
        //And the second
        point2 = regionRanker(getDiscipline(g, PLAYER_1_VERT_2A1)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_2A2)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_2A3));
        
        //And the third
        point3 = regionRanker(getDiscipline(g, PLAYER_1_VERT_1B1)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_1B2)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_1B3));
        
        //And the fourth
        point4 = regionRanker(getDiscipline(g, PLAYER_1_VERT_2B1)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_2B2)) +
        regionRanker(getDiscipline(g, PLAYER_1_VERT_2B3));
        
        //Make any point we already own non-viable
        if (getCampus(g, stringParser(player, 1, 1)) == player) {
            point1 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 1)) == player) {
            point2 = 0;
        }
        if (getCampus(g, stringParser(player, 1, 2)) == player) {
            point3 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 2)) == player) {
            point4 = 0;
        }
        
        biggest = max(point1,point2);
        biggest = max(biggest, point3);
        biggest = max(biggest, point4);
        printf("Biggest: %d\n", biggest);

        
        if (point1 == biggest){
            decision.point = 1;
            decision.side = 1;
        } else if (point2 == biggest) {
            decision.point = 2;
            decision.side = 1;
        } else if (point3 == biggest) {
            decision.point = 1;
            decision.side = 2;
        } else if (point4 == biggest) {
            decision.point = 2;
            decision.side = 2;
        }
    } else if (player == 2) {
        //Get the value for the first point
        point1 = regionRanker(getDiscipline(g, PLAYER_2_VERT_1A1)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_1A2)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_1A3));
        
        //And the second
        point2 = regionRanker(getDiscipline(g, PLAYER_2_VERT_2A1)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_2A2)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_2A3));
        
        //And the third
        point3 = regionRanker(getDiscipline(g, PLAYER_2_VERT_1B1)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_1B2)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_1B3));
        
        //And the fourth
        point4 = regionRanker(getDiscipline(g, PLAYER_2_VERT_2B1)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_2B2)) +
        regionRanker(getDiscipline(g, PLAYER_2_VERT_2B3));
        
        //Make any point we already own non-viable
        if (getCampus(g, stringParser(player, 1, 1)) == player) {
            point1 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 1)) == player) {
            point2 = 0;
        }
        if (getCampus(g, stringParser(player, 1, 2)) == player) {
            point3 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 2)) == player) {
            point4 = 0;
        }
        
        biggest = max(point1,point2);
        biggest = max(biggest, point3);
        biggest = max(biggest, point4);
        printf("Biggest: %d\n", biggest);

        if (point1 == biggest){
            decision.point = 1;
            decision.side = 1;
        } else if (point2 == biggest) {
            decision.point = 2;
            decision.side = 1;
        } else if (point3 == biggest) {
            decision.point = 1;
            decision.side = 2;
        } else if (point4 == biggest) {
            decision.point = 2;
            decision.side = 2;
        }
        
    } else if (player == 3) {
        
        //Get the value for the first point
        point1 = regionRanker(getDiscipline(g, PLAYER_3_VERT_1A1)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_1A2)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_1A3));
        
        //And the second
        point2 = regionRanker(getDiscipline(g, PLAYER_3_VERT_2A1)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_2A2)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_2A3));
        
        //And the third
        point3 = regionRanker(getDiscipline(g, PLAYER_3_VERT_1B1)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_1B2)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_1B3));
        
        //And the fourth
        point4 = regionRanker(getDiscipline(g, PLAYER_3_VERT_2B1)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_2B2)) +
        regionRanker(getDiscipline(g, PLAYER_3_VERT_2B3));
        
        //Make any point we already own non-viable
        if (getCampus(g, stringParser(player, 1, 1)) == player) {
            point1 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 1)) == player) {
            point2 = 0;
        }
        if (getCampus(g, stringParser(player, 1, 2)) == player) {
            point3 = 0;
        }
        if (getCampus(g, stringParser(player, 2, 2)) == player) {
            point4 = 0;
        }
        
        biggest = max(point1,point2);
        biggest = max(biggest, point3);
        biggest = max(biggest, point4);
        printf("Biggest: %d\n", biggest);

        if (point1 == biggest){
            decision.point = 1;
            decision.side = 1;
        } else if (point2 == biggest) {
            decision.point = 2;
            decision.side = 1;
        } else if (point3 == biggest) {
            decision.point = 1;
            decision.side = 2;
        } else if (point4 == biggest) {
            decision.point = 2;
            decision.side = 2;
        }
        
    } else {
        printf("Waring: Player Number out of bounds\n");
    }
    
    printf("Optimum Placment at vertice: %d\n Side: %d\n",
           decision.point, decision.side);
    return decision;
}

void testBuildDecider(Game g) {
    int player = 1;
    build decision;
    decision = buildDecider(g, player);
    assert(decision.point == 2);
    assert(decision.side == 1);
    
}

int regionRanker(int Discipline){
    int rank = 0;
    printf("Discipline code: %d\n", Discipline);
    //Return the rank for the given region
    if (Discipline == STUDENT_BPS){
        rank = RANK_STUDENT_BPS;
    } else if (Discipline == STUDENT_BQN) {
        rank = RANK_STUDENT_BQN;
    } else if (Discipline == STUDENT_MJ) {
        rank = RANK_STUDENT_MJ;
    } else if (Discipline == STUDENT_MMONEY) {
        rank = RANK_STUDENT_MMONEY;
    } else if (Discipline == STUDENT_MTV) {
        rank = RANK_STUDENT_MTV;
    } else if (Discipline == STUDENT_THD) {
        rank = RANK_STUDENT_THD;
    }
    return rank;
}

void testRegionRanker(void) {
    assert(regionRanker(STUDENT_BPS) == RANK_STUDENT_BPS);
    assert(regionRanker(STUDENT_BQN) == RANK_STUDENT_BQN);
    assert(regionRanker(STUDENT_MMONEY) == RANK_STUDENT_MMONEY);
    assert(regionRanker(STUDENT_MJ) == RANK_STUDENT_MJ);
    assert(regionRanker(STUDENT_MTV) == RANK_STUDENT_MTV);
    assert(regionRanker(STUDENT_THD) == RANK_STUDENT_THD);
}


int max(int num1, int num2) {
    int result;
    
    if (num1 > num2)
        result = num1;
    else
        result = num2;
    
    return result;
}
