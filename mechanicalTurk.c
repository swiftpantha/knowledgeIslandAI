/*
 *  mechanicalTurk
 *
 *  Trades smart, builds dumb and etc.
 *
 *  Proudly Created by Moss and Anton
 *  Share Freely Creative Commons SA-BY-NC 3.0. 
 *
 */

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

#define FIRST_POINT 1
#define FIRST_SIDE 1

#define MAX_ATTEMPTS 5
#define NON_VOLATILE 8

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
 
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
    int pack[6]; // array to pack resources in for easy access
} res;

typedef struct _res * Res; // pointer to a _res struct

action smartTrading (Res myRes, int actionCode);
action dumbTrading (int resource);
char* stringParser(int player, int vertice, int point);
//int resourceCheck(int a, int b, int c, int d, int e, int f);
action dumbBuilding (path destination, Game g);
Res getMyRes (Game g);
Res packRes (Res r);
action enoughToTradeThree(action a, Res myRes, int kind);
action enoughToTradeTwo(action a, Res myRes, int kind, int altRes, int altKind);
action enoughToTradeOne(action a, Res myRes, int kind, int altRes, int altKind);
void testStringParser(void);
void testDumbBuilding(Game g);
void testSmartTrading (Game g);
char* kindToName(int kind);
int getTradeKind(Res r, int range);
int getHowManySet(Res r, int amount);
int getHowManySetExcluding(Res r, int amount, int exclude);
int getWhatSet(Res r, int amount);

action decideAction (Game g) {
    int player = getWhoseTurn(g);
    int attempts = 1;
    //int mj;
    //int mm;
    //int mtv;
    action nextAction;
    // mj = getStudents (g, UNI_A, STUDENT_MJ);
    // mm = getStudents (g, UNI_A, STUDENT_MMONEY);
    // mtv = getStudents (g, UNI_A, STUDENT_MTV);
    // if(mj >= 1 && mm >= 1 && mtv >= 1) {
    
    char *pathCampus = stringParser(player, FIRST_POINT, FIRST_SIDE);
    
    Res myRes;
    myRes = getMyRes(g);
    
    nextAction.actionCode = PASS;
    
    //Here we build until we get x campuses then we smart trade for spin
    if ((getCampus(g,pathCampus) != player) &&
        //Though 1 We start building
        (attempts <= MAX_ATTEMPTS)) {
        // do we have enough resources to BUILD_CAMPUS?
        // if yes - it will allow smart building
        // if no - we will smartTrade until we have those resources
        nextAction = smartTrading(myRes, BUILD_CAMPUS);
        if(nextAction.actionCode == BUILD_CAMPUS) {
            printf("Attempt to do the dumbBuilding: %d",attempts);
            nextAction = dumbBuilding(pathCampus, g);
            attempts++; // TODO Moss what are the attempts for? You forgot to increase them here also
        }
    // either we can't build campuses in a smart way - blocked
    // or we have tried 5 times and it failed
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


int main (int argc, char *argv[]) {
    
    int disciplines[] = DEFAULT_DISCIPLINES;
    int diceScores[] = DEFAULT_DICE;
    
    
    //Create a new game to test on
    Game testGame = newGame (disciplines, diceScores);
    
    printf("==== Tests start ====\n");
    
    testSmartTrading(testGame);
    testStringParser();
    testDumbBuilding(testGame);
    
    printf ("All AI tests passed. You are awesome!\n");

    return EXIT_SUCCESS;
}


// Trades Only whenever enough to complete action succesfully
action smartTrading (Res myRes, int actionCode) {

    action nextAction;
    nextAction.actionCode = PASS; // default is PASS
    nextAction.disciplineFrom = 0; // default is 0
    nextAction.disciplineTo = 0; // default is 0
        
    // Start spinoff smart trading:
    // Need 1 MJ, 1 MTV, 1M$
    // Know that MTV and M$ dissapear on dice score being 7
    // Logic:
    // Aim to be able to trade for the goal in one go with no passing, i.e. start any trading to MTV/MMONEY only when 
    // have sufficient amounts to be able to finish the trading and spinoff in this turn
    if (actionCode == START_SPINOFF) {
        printf("> smartTrading for START_SPINOFF\n");
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
        else if(myRes->MTV < 1 && myRes->MMONEY < 1 && myRes->MJ < 1) {
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
        else {
            printf("> Can't trade in one go yet...\n");
            nextAction.actionCode = PASS;
            // always trade the volatile resources for stable ones
            // LATER MAY BE: trade for other non-volatile resources if we already have MJ supply
            if(myRes->MTV >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MTV;
                nextAction.disciplineTo = STUDENT_MJ; 
            }
            if(myRes->MMONEY >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MMONEY;
                nextAction.disciplineTo = STUDENT_MJ;
            }
        } // end else
    } // endif START_SPINOFF
    
    // BUILD_CAMPUS smart trading:
    // Need 1 BPS, 1 BQN, 1 MJ, 1 MTV
    // Know that MTV dissapear on dice score being 7
    // Logic:
    // Aim to be able to trade for the goal in one go with no passing, i.e. start trading to MTV only when 
    // have sufficient amounts to be able to finish the trading and build campus in this turn
    if (actionCode == BUILD_CAMPUS) {
        printf("> smartTrading for BUILD_CAMPUS\n");
        
        myRes = packRes(myRes); // get an array of res in myRes->pack for easy access
        
        // we have all kinds - build CAMPUS
        if(getHowManySet(myRes, 1) >= 4) {
            nextAction.actionCode = BUILD_CAMPUS;
        }
        // we have 12 MMONEYs and nothing else - trade for BPS (default)
        else if(myRes->MMONEY >= 12 && getHowManySet(myRes, 1) < 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = STUDENT_BPS;
        }
        // we have 9 MMONEYs and at least one kind is set - trade
        else if(myRes->MMONEY >= 9 && getHowManySet(myRes, 1) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 6 MMONEYs and at least 2 of kinds set - trade
        else if(myRes->MMONEY >= 6 && getHowManySet(myRes, 1) >= 2) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 3 MMONEYs and at least 3 of kinds set - trade
        else if(myRes->MMONEY >= 3 && getHowManySet(myRes, 1) >= 3) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 6 MMONEYs and 1 kind having 4 - trade
        else if(myRes->MMONEY >= 6 && getHowManySet(myRes, 4) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 3 MMONEYs and 1 kind having 7 - trade
        else if(myRes->MMONEY >= 3 && getHowManySet(myRes, 7) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 3 MMONEYs and 1 kind having 4
        // and 1 more set that is not the one having 4 - trade
        else if(myRes->MMONEY >= 3 && getHowManySet(myRes, 4) >= 1 && getHowManySetExcluding(myRes, 1, getWhatSet(myRes, 4)) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = STUDENT_MMONEY;
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 0 MMONEYs and 1 kind having 10 - trade
        else if(myRes->MMONEY < 1 && getHowManySet(myRes, 10) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = getWhatSet(myRes, 10); // what is the resource that has 10?
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 0 MMONEYs and 1 kind having 7
        // and 1 more (that is not the one having 7) set - trade
        else if(myRes->MMONEY < 1 && getHowManySet(myRes, 7) >= 1 && getHowManySetExcluding(myRes, 1, getWhatSet(myRes, 7)) >= 1) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = getWhatSet(myRes, 7);
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 0 MMONEYs and 2 kinds having 4 - trade
        else if(myRes->MMONEY < 1 && getHowManySet(myRes, 4) >= 2) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = getWhatSet(myRes, 4);
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // we have 0 MMONEYs and 1 kind having 4
        // and 2 more (that are not the one having 4) set - trade
        else if(myRes->MMONEY < 1 && getHowManySet(myRes, 4) >= 1 && getHowManySetExcluding(myRes, 1, getWhatSet(myRes, 4)) >= 2) {
            nextAction.actionCode = RETRAIN_STUDENTS;
            nextAction.disciplineFrom = getWhatSet(myRes, 4);
            nextAction.disciplineTo = getTradeKind(myRes, actionCode);
        }
        // if all else failed - we can't trade all of the stuff in one go yet
        else {
            printf("> Can't trade in one go yet...\n");
            nextAction.actionCode = PASS;
            // always trade the volatile resources for stable ones
            if(myRes->MTV >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MTV;
                nextAction.disciplineTo = getTradeKind(myRes, NON_VOLATILE);
            }
            if(myRes->MMONEY >= 3) {
                nextAction.actionCode = RETRAIN_STUDENTS;
                nextAction.disciplineFrom = STUDENT_MMONEY;
                nextAction.disciplineTo = getTradeKind(myRes, NON_VOLATILE);
            }
        } // end else
    } // end if BUILD_CAMPUS
    return nextAction;
}

// gets which kind to trade for
// out of range given
int getTradeKind(Res r, int range) {
    int tradeKind = 0;
    int kind = 0;
    int kind_end = 0;
    // depending on the action we scan different ranges
    if(range == BUILD_CAMPUS) {
        kind = STUDENT_BPS; // from BPS
        kind_end = STUDENT_MTV; // to MTV
    }
    else if(range == NON_VOLATILE) {
        kind = STUDENT_BPS; // from BPS
        kind_end = STUDENT_MJ; // to MJ
    }
    else { // START_SPINOFF
        kind = STUDENT_MJ; // from MJ
        kind_end = STUDENT_MMONEY; // to MMONEY
    }
    // trade for the first required kind
    while(kind <= kind_end && tradeKind == 0) {
        if(r->pack[kind] < 1) {
            tradeKind = kind;
        }
        kind++;
    }
    return tradeKind;
}

// gets how many of the kinds have the requested amount of units
// for BUILD_CAMPUS smartTrading only!
int getHowManySet(Res r, int amount) {
    int count = 0;
    int kind = 1;
    while(kind < STUDENT_MMONEY && count < 2) {
        if(r->pack[kind] >= amount) {
            count++;
        }
        kind++;
    }
    return count;
}

// gets how many of the kinds have the requested amount of units
// excluding the one set in exclude
// for BUILD_CAMPUS smartTrading only!
int getHowManySetExcluding(Res r, int amount, int exclude) {
    int count = 0;
    int kind = 1;
    while(kind < STUDENT_MMONEY && count < 2) {
        if(r->pack[kind] >= amount && kind != exclude) {
            count++;
        }
        kind++;
    }
    return count;
}

// gets which kind has the requested amount of units
// for BUILD_CAMPUS smartTrading only!
int getWhatSet(Res r, int amount) {
    int setKind = 0;
    int kind = 1;
    while(kind < STUDENT_MMONEY && setKind == 0) {
        if(r->pack[kind] >= amount) {
            setKind = kind;
        }
        kind++;
    }
    return setKind;
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
action dumbBuilding(path destination, Game g) {
    action nextAction;
    int length = 0;
    int player = getWhoseTurn(g);
    int vertice = FIRST_POINT;
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

Res packRes (Res r) {
    assert(r != NULL);
    r->pack[STUDENT_THD] = r->THD;
    r->pack[STUDENT_BPS] = r->BPS;
    r->pack[STUDENT_BQN] = r->BQN;
    r->pack[STUDENT_MJ] = r->MJ;
    r->pack[STUDENT_MTV] = r->MTV;
    r->pack[STUDENT_MMONEY] = r->MMONEY;
    return r;
}

void testDumbBuilding(Game g){
    throwDice(g, 1);
    throwDice(g, 1);
    throwDice(g, 1);
    
    action newAction;
    int player = getWhoseTurn(g);
    int attempts = 1;
    printf("\n\n======= TESTING DUMB BUILD =========\n");
    printf("Players %d's turn\n", player);
    
    char *pathCampus = stringParser(player, FIRST_POINT, FIRST_SIDE);
    
    //Initiate the current owner of the vertice
    int owner = getCampus(g,pathCampus);
    printf("Trying to get Campus at location: %s\n", pathCampus);
    printf("Before owner: %d\n", getCampus(g,pathCampus));

    //Repeatedly call this until that campus is ours!
    while ((owner != player) && (attempts <= MAX_ATTEMPTS)) {
        printf("Attempt: %d\n", attempts);
        newAction = dumbBuilding(pathCampus, g);
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
    printf("\n\n ======= testSmartTrading start =======\n");
    throwDice(g,7);
    Res myRes = getMyRes(g);
    action newAction;    
    
    printf("[Testing] smartTrading for START_SPINOFF\n");
    // START SPINOFF driven smart trading tests
    // Tests all situations by faking the amount of students
    printf("[Test 1.1] - have enough, should start spinoff\n");
    myRes->MJ = 1;
    myRes->MTV = 1;
    myRes->MMONEY = 1;
    newAction = smartTrading(myRes, START_SPINOFF);
    assert(newAction.actionCode == START_SPINOFF);

    // could trade BPS - but there is no point since you can't get 1-1-1 on this turn anyway
    printf("[Test 1.2] - not enough and no way to trade in one go, should PASS\n");
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
    printf("[Test 1.3] - Can trade all in this turn, start in order MJ->MTV->MMONEY\n");
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
    printf("[Test 1.4] - has 4 MTV should trade for MMONEY \n");
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
    printf("[Test 1.5] - Have 7 MJ, 0 MTV should trade for MTV\n");
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
    printf("[Test 1.6] - Have 3 MJ, 0 MTV, 0 MMONEY should PASS\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 3;
    myRes->MTV = 0;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, START_SPINOFF);
    assert (newAction.actionCode == PASS);


    printf("[Testing] smartTrading for BUILD_CAMPUS\n");

    printf("[Test 2.0] - Have none, but 12 MMONEY - should retrain for BPS\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 12;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_BPS);

    printf("[Test 2.1] - Have 1 BPS, 9 M$ - should retrain M$ for BQN\n");
    myRes->BPS = 1;
    myRes->BQN = 0;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 9;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_BQN);

    printf("[Test 2.2] - Have 4 MTV, 6 M$ and nothing else - should retrain M$ for BPS\n");
    myRes->BPS = 0;
    myRes->BQN = 0;
    myRes->MJ = 0;
    myRes->MTV = 4;
    myRes->MMONEY = 6;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_BPS);

    printf("[Test 2.3] - Have 4 MTV, 3 M$, 1 BPS and nothing else - should retrain M$ for BQN\n");
    myRes->BPS = 1;
    myRes->BQN = 0;
    myRes->MJ = 0;
    myRes->MTV = 4;
    myRes->MMONEY = 3;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_BQN);

    printf("[Test 2.4] - Have 4 MTV, 1 BPS, 1 BQN and nothing else - should retrain MTV for MJ\n");
    myRes->BPS = 1;
    myRes->BQN = 1;
    myRes->MJ = 0;
    myRes->MTV = 4;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MTV);
    assert (newAction.disciplineTo == STUDENT_MJ);

    printf("[Test 2.5] - Have 4 BPS, 0 BQN, 1 MJ, 1 MTV and nothing else - should retrain BPS for BQN\n");
    myRes->BPS = 4;
    myRes->BQN = 0;
    myRes->MJ = 1;
    myRes->MTV = 1;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_BPS);
    assert (newAction.disciplineTo == STUDENT_BQN);

    // can't get in one go - just PASS
    printf("[Test 2.6] - Have 1 BPS, 1 BQN, 3 MJ should PASS\n");
    myRes->BPS = 1;
    myRes->BQN = 1;
    myRes->MJ = 3;
    myRes->MTV = 0;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == PASS);

    // can't get in one go - just PASS
    printf("[Test 2.7] - Have 0 BPS, 3 BQN, 1 MJ, 1 MTV should PASS\n");
    myRes->BPS = 0;
    myRes->BQN = 3;
    myRes->MJ = 1;
    myRes->MTV = 1;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == PASS);

    printf("[Test 2.8] - Have 1 BPS, 4 BQN, 0 MJ, 1 MTV should retrain to MJ\n");
    myRes->BPS = 1;
    myRes->BQN = 4;
    myRes->MJ = 0;
    myRes->MTV = 1;
    myRes->MMONEY = 0;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_BQN);
    assert (newAction.disciplineTo == STUDENT_MJ);

    printf("[Test 2.9] - Have 1 BPS, 4 BQN, 0 MJ, 0 MTV, 3 MMONEY -> should retrain MMONEY to MJ\n");
    myRes->BPS = 1;
    myRes->BQN = 4;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 3;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_MJ);

    printf("[Test 2.10] - Have 0 BPS, 4 BQN, 0 MJ, 0 MTV, 6 MMONEY -> should retrain MMONEY to BPS\n");
    myRes->BPS = 0;
    myRes->BQN = 4;
    myRes->MJ = 0;
    myRes->MTV = 0;
    myRes->MMONEY = 6;
    newAction = smartTrading (myRes, BUILD_CAMPUS);
    assert (newAction.actionCode == RETRAIN_STUDENTS);
    assert (newAction.disciplineFrom == STUDENT_MMONEY);
    assert (newAction.disciplineTo == STUDENT_BPS);
    
    printf("======= testSmartTrading end =======\n\n");
}