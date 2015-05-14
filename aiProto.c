/*  Copyright 
 *  Mr Pass.  Brain the size of a planet!
 *
 *  Proundly Created by Moss and Anton
 *  Share Freely Creative Commons SA-BY-NC 3.0. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Game.h"
#include "mechanicalTurk.h"

#define TESTING 1

typedef struct _res {
    int BPS;
    int BQN;
    int THD;
    int MJ;
    int MTV;
    int MMONEY;
} res;

void smartTrading(int actionCode);
void dumbTrading(int resource);
char stringParser(int player, int vertice);
int resourceCheck(int a, int b, int c, int d, int e, int f);
void dumbBuilding(path destination, Game g);
void testAI(void);
res getMyRes(Game g);

action decideAction(Game g) {
    // int mj;
    // int mm;
    // int mtv;
    action nextAction;
    // mj = getStudents (g, UNI_A, STUDENT_MJ);
    // mm = getStudents (g, UNI_A, STUDENT_MMONEY);
    // mtv = getStudents (g, UNI_A, STUDENT_MTV);
    // if(mj >= 1 && mm >= 1 && mtv >= 1) {
    nextAction.actionCode = START_SPINOFF;
    // if we can't start a spinoff - just pass
    if (isLegalAction(g, nextAction) != TRUE)
        nextAction.actionCode = PASS;
    }
    return nextAction;
}

// Trades Only whenever enough to complete action succesfully
void smartTrading(Game g, int actionCode) {
}

// Trades for resource whenever there is enough resources to
// do so succesfully
void dumbTrading(Game g, int resource) {
}

// Tries to progressively build a campus at the destination
void dumbBuilding(path destination, Game g) {
}

// Gives you the path to the vertice for a given player.
char stringParser(int player, int vertice) {
    if ((player  == 1) && (vertice == 1)) {
        char destination = {'R', 'L'}
    }
    if ((player  == 3) && (vertice == 1)) {
        char destination = {'L', 'R', 'L', 'R', 'R'}
    }
    // Supplies Position
    if ((player  == 3) && (vertice == 1)) {
        char destination = {'L', 'R', 'L', 'R', 'L', 'R', 'R', 'L', 'R', 'R'}
    }
    return char* destination
}

assert(((player == 1) && (vertice == 1)), stringParser(1, 1) == "LR");
assert(((player == 2) && (vertice == 1)), stringParser(2, 1) == "LRLRR");
assert(((player == 3) && (vertice == 1)), stringParser(3, 1) == "LRLRLRRLRR");


// Checks if we have resources
int resourceCheck(Game g, int ThD, int BPS, int BQN,
    int MJ, int MTV, int MM) {
    if (TESTING) {
    } else if (TESTING) {
    }
    return
}



void testAI(void) {
}

// we have this function so we can fake our resources and test the AI decision
res getMyRes(Game g) {
    int player = getWhoseTurn(g);
    res myRes;
    myRes.BPS = g->getStudents(g, player, STUDENT_BPS);
    myRes.BQN = g->getStudents(g, player, STUDENT_BQN);
    myRes.THD = g->getStudents(g, player, STUDENT_THD);
    myRes.MJ = g->getStudents(g, player, STUDENT_MJ);
    myRes.MTV = g->getStudents(g, player, STUDENT_MTV);
    myRes.MMONEY = g->getStudents(g, player, STUDENT_MMONEY);
}
