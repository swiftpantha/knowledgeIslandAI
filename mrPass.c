/*
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
    // if we can't start a spinoff - just pass
    if(isLegalAction(g, nextAction) != TRUE){
        nextAction.actionCode = PASS;        
    }
    return nextAction;
}