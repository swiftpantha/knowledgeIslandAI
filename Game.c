/* 
Copyright
Possible game struct

*/


#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define NUM_POINTS 54
#define NUM_POINTS_X 6
#define NUM_POINTS_Y 11
#define NUM_REGIONS_X 5
#define NUM_REGIONS_Y 5
#define NUM_EDGES 72
#define NUM_DISCIPLINES 6
#define MAX_REGIONS_PER_POINT 3

#define START_DIFF 2
#define COL_INC 0
#define COL_DEC 1
#define ROW_INC 2
#define ROW_DEC 3

#define TRUE 1
#define FALSE 0

#define MAX_ACTION 7
#define TERRA_NULLIS -1
#define POSSIBLE_POINTS 4


typedef struct _board *Board;
typedef struct _uni *Uni;
typedef struct _hexagon *Hexagon;
typedef struct _point *Point;
typedef struct _edge *Edge;

// A university is defined by its ID. This also stores the uni's
// statistics
typedef struct _uni {
    int uniID;
    int KPIs;
    int campuses;
    int GO8Campuses;
    int ARCGrants;
    int patents;
    int papers;
    int students[NUM_DISCIPLINES];
} uni;


// Includes the hexagon's ID, dice value, and discipline.
typedef struct _hexagon {
    int regionID;
    int diceValue;
    int discipline;
    int x;
    int y;
} hexagon;


// A point is defined by its x and y coordinates
// and includes a contents integer,
// which region it is next to, and any retraining centers
typedef struct _point {
    int x;
    int y;
    int surroundingHexagons;
    Hexagon hexagons[MAX_REGIONS_PER_POINT];
    int contents;
    int retraining;
} point;

// An edge is defined by two points, where the first point is always
// before the second point in order of x, then y coordinates, so that
// an edge can not be defined two ways
typedef struct _edge {
    Uni ARC;
    double x;
    double y;
} edge;

// Stores the points in an array and the occupied edges
typedef struct _board {
    Point points[NUM_POINTS_X][NUM_POINTS_Y];
    Edge edgesUsed[NUM_EDGES];
    Hexagon regions[NUM_REGIONS_X][NUM_REGIONS_Y];
} board;

// The game struct stores the players in an array, the board, the
// current turn, and the value of the dice roll
typedef struct _game {
    Uni players[NUM_UNIS];
    Board gameBoard;
    int currentTurn;
    int roll;
    Uni mostARCs;
    Uni mostPubs;
} game;



static point pathToPoint(path givenPath);
static point charToPoint(char letter, int difference, point currentPoint);
static edge pathToEdgeF(path givenPath);
static int validPoint(int x, int y);
static int validRegion(int x, int y);
static edge findEdgeDetails(double x, double y, Game g);
static int getTotalEdges(Game g);
static int getTotalGO8s(Game g);
static int validNewEdge(Game g, edge line, int player);
static int validString(path destination);
static int validNewContents(Game g, path destination, int player);
static void createBoard(Game g, int discipline[], int dice[]);
static void createPlayer(Game g, int player);
static void createPoint(Game g, int x, int y);
static void createRegion(Game g, int x, int y, int index,
    int dice, int discipline);
static void addRegionToPoints(Game g, int x, int y);
static void createStartCampuses(Game g);
static void createRetrainingPoints(Game g);
static void disposeEdgesAndPlayers(Game g);
static void disposePoints(Game g);
static void disposeRegions(Game g);
// Static void startSpinoff(Game g, action a);
static void buildCampus(Game g, action a);
static void buildGO8(Game g, action a);
static void buildARC(Game g, action a);
static void addPub(Game g, action a);
static void addPatent(Game g, action a);
static void retrainStudents(Game g, action a);
static void giveResource(Game g, int x, int y, int resource);

// Create a game in the inital state
Game newGame(int discipline[], int dice[]) {
    // Create a game
    Game g;
    g = malloc (sizeof (struct _game));
    assert(g != NULL);
    // Initialise each uni with the appropriate details
    int i = 0;
    while (i < NUM_UNIS) {
        createPlayer(g, i);
        i++;
    }

    createBoard(g, discipline, dice);

    // Initialise the mostARCs and mostPubs as no one
    g->mostARCs = malloc (sizeof (struct _uni));
    g->mostARCs->ARCGrants = 0;
    g->mostARCs->uniID = NO_ONE;
    g->mostPubs = malloc (sizeof (struct _uni));
    g->mostPubs->papers = 0;
    g->mostPubs->uniID = NO_ONE;

    // Initialise the current turn and roll
    g->currentTurn = -1;
    g->roll = 0;
    // put in initial campuses and retraining centres

    createStartCampuses(g);
    createRetrainingPoints(g);

    return g;
}

// Create the board
static void createBoard(Game g, int discipline[], int dice[]) {
    g->gameBoard = malloc (sizeof (struct _board));
    assert(g->gameBoard != NULL);

    // Create the points that exist
    int x = 0;
    while (x < NUM_POINTS_X) {
        int y = 0;
        while (y < NUM_POINTS_Y) {
            if (validPoint(x, y)) {
                createPoint(g, x, y);
            }
            y++;
        }
        x++;
    }

    // Create the regions that exist
    x = 0;
    int i = 0;

    while (x < NUM_REGIONS_X) {
        int y = 0;
        while (y < NUM_REGIONS_Y) {
            if (validRegion(x, y)) {
                createRegion(g, x, y, i, dice[i], discipline[i]);
                // Add the region to the points around it
                addRegionToPoints(g, x, y);
                i++;
            }
            y++;
        }
        x++;
    }
}

// Initialise a uni with the appropriate details
static void createPlayer(Game g, int player) {
    g->players[player] = malloc (sizeof (struct _uni));
    assert(g->players[player] != NULL);
    g->players[player]->uniID = player + 1;
    g->players[player]->KPIs = 20;
    g->players[player]->campuses = 2;
    g->players[player]->GO8Campuses = 0;
    g->players[player]->ARCGrants = 0;
    g->players[player]->patents = 0;
    g->players[player]->papers = 0;
    g->players[player]->students[STUDENT_BPS] = 3;
    g->players[player]->students[STUDENT_BQN] = 3;
    g->players[player]->students[STUDENT_MTV] = 1;
    g->players[player]->students[STUDENT_MJ] = 1;
    g->players[player]->students[STUDENT_MMONEY] = 1;
    g->players[player]->students[STUDENT_THD] = 0;
}

// Create an empty point
static void createPoint(Game g, int x, int y) {
    g->gameBoard->points[x][y] = malloc (sizeof (struct _point));
    assert(g->gameBoard->points[x][y] != NULL);
    g->gameBoard->points[x][y]->x = x;
    g->gameBoard->points[x][y]->y = y;
    g->gameBoard->points[x][y]->surroundingHexagons = 0;
    g->gameBoard->points[x][y]->contents = VACANT_VERTEX;
}

// Create a region
static void createRegion(Game g, int x, int y, int index,
    int dice, int discipline) {
    g->gameBoard->regions[x][y] = malloc (sizeof (struct _hexagon));
    assert(g->gameBoard->regions[x][y] != NULL);
    g->gameBoard->regions[x][y]->x = x;
    g->gameBoard->regions[x][y]->y = y;
    g->gameBoard->regions[x][y]->regionID = index;
    g->gameBoard->regions[x][y]->diceValue = dice;
    g->gameBoard->regions[x][y]->discipline = discipline;
}

// Put the region in the relevant points
static void addRegionToPoints(Game g, int x, int y) {
    int xPoint = x;
    while (xPoint < x + 2) {
        int yPoint = 2*y + x%2;
        while (yPoint < 2*y  + x%2 + 3) {
            g->gameBoard->points[xPoint][yPoint]->
                hexagons[g->gameBoard->
                points[xPoint][yPoint]->surroundingHexagons]
                = g->gameBoard->regions[x][y];
            g->gameBoard->points[xPoint][yPoint]->
                surroundingHexagons++;
            yPoint += 1;
        }
        xPoint++;
    }
}

// Add the starting campuses
static void createStartCampuses(Game g) {
    g->gameBoard->points[2][0]->contents = CAMPUS_A;
    g->gameBoard->points[3][10]->contents = CAMPUS_A;
    g->gameBoard->points[0][3]->contents = CAMPUS_B;
    g->gameBoard->points[5][7]->contents = CAMPUS_B;
    g->gameBoard->points[0][8]->contents = CAMPUS_C;
    g->gameBoard->points[5][2]->contents = CAMPUS_C;
}

// Add the retraining centres
static void createRetrainingPoints(Game g) {
    g->gameBoard->points[3][1]->retraining = STUDENT_MMONEY;
    g->gameBoard->points[4][1]->retraining = STUDENT_MMONEY;
    g->gameBoard->points[5][5]->retraining = STUDENT_BQN;
    g->gameBoard->points[5][6]->retraining = STUDENT_BQN;
    g->gameBoard->points[4][8]->retraining = STUDENT_MJ;
    g->gameBoard->points[4][9]->retraining = STUDENT_MJ;
    g->gameBoard->points[1][8]->retraining = STUDENT_BPS;
    g->gameBoard->points[1][9]->retraining = STUDENT_BPS;
    g->gameBoard->points[1][1]->retraining = STUDENT_MTV;
    g->gameBoard->points[2][1]->retraining = STUDENT_MTV;
}


// free all the memory malloced for the game
void disposeGame(Game g) {
    if (g->mostARCs->ARCGrants == 0) {
        free(g->mostARCs);
    }
    if (g->mostPubs->papers == 0) {
        free(g->mostPubs);
    }

    disposeEdgesAndPlayers(g);
    disposePoints(g);
    disposeRegions(g);

    free(g->gameBoard);
    free(g);
}

// free all the memory malloced for edges and players
static void disposeEdgesAndPlayers(Game g) {
    int numEdges = 0;
    int i = 0;
    while (i < NUM_UNIS) {
        numEdges += getARCs(g, i + 1);
        free(g->players[i]);
        i++;
    }
    i = 0;
    while (i < numEdges) {
        free(g->gameBoard->edgesUsed[i]);
        i++;
    }
}

// free all the memory malloced for points
static void disposePoints(Game g) {
    int x = 0;
    int y = 0;
    while (x < NUM_POINTS_X) {
        y = 0;
        while (y < NUM_POINTS_Y) {
            if (validPoint(x, y)) {
                free(g->gameBoard->points[x][y]);
            }
            y++;
        }
        x++;
    }
}

// free all the memory malloced for regions
static void disposeRegions(Game g) {
    int x = 0;
    int y = 0;

    while (x < NUM_REGIONS_X) {
        y = 0;
        while (y < NUM_REGIONS_Y) {
            if (validRegion(x, y)) {
                free(g->gameBoard->regions[x][y]);
            }
            y++;
        }
        x++;
    }
}


// make the specified action for the current player and update the
// game state accordingly->
// The function may assume that the action requested is legal->
// START_SPINOFF is not a legal action = here
void makeAction(Game g, action a) {
    int actionCode = a.actionCode;
    if (actionCode == BUILD_CAMPUS) {
        printf("Building a campus...\n");
        buildCampus(g, a);
        printf("Completed a campus...\n");
    } else if (actionCode == BUILD_GO8) {
        printf("Building a GO8...\n");
        buildGO8(g, a);
        printf("Completed a GO8...\n");
    } else if (actionCode == OBTAIN_ARC) {
        printf("Building an ARC...\n");
        buildARC(g, a);
        printf("Completed a ARC...\n");
    } else if (actionCode == OBTAIN_PUBLICATION) {
        printf("Obtained a publication...\n");
        addPub(g, a);
        printf("Completed a publication...\n");
    } else if (actionCode == OBTAIN_IP_PATENT) {
        printf("Obtaining a patent...\n");
        addPatent(g, a);
        printf("Completed a patent...\n");
    } else if (actionCode == RETRAIN_STUDENTS) {
        printf("Retraining students...\n");
        retrainStudents(g, a);
        printf("Completed a students...\n");
    }
}

// builds a campus for the current player in the location specified
static void buildCampus(Game g, action a) {
    point location = pathToPoint(a.destination);
    assert(location.x >= 0);
    int player = getWhoseTurn(g);
    g->gameBoard->points[location.x][location.y]->contents = player;
    g->players[player - 1]->campuses++;
    g->players[player - 1]->students[STUDENT_BPS]--;
    g->players[player - 1]->students[STUDENT_BQN]--;
    g->players[player - 1]->students[STUDENT_MJ]--;
    g->players[player - 1]->students[STUDENT_MTV]--;
    // 10 KPI per campus
    g->players[player - 1]->KPIs = g->players[player - 1]->KPIs + 10;
}

// builds a GO8 campus for the current player in the location specified
static void buildGO8(Game g, action a) {
    point location = pathToPoint(a.destination);
    assert(location.x >= 0);
    int player = getWhoseTurn(g);
    g->gameBoard->points[location.x][location.y]->contents = player + 3;
    g->players[player - 1]->campuses--;
    g->players[player - 1]->GO8Campuses++;
    g->players[player - 1]->students[STUDENT_MJ] += -2;
    g->players[player - 1]->students[STUDENT_MMONEY] += -3;
    // Give 'em an extra 10 per KPI
    g->players[player - 1]->KPIs = g->players[player - 1]->KPIs + 10;
}

// builds an ARC for the current player in the location specified
static void buildARC(Game g, action a) {
    int numEdges = getTotalEdges(g);
    edge location = pathToEdgeF(a.destination);
    int player = getWhoseTurn(g);
    g->gameBoard->edgesUsed[numEdges] = malloc (sizeof (struct _edge));
    assert(g->gameBoard->edgesUsed[numEdges] != NULL);
    g->gameBoard->edgesUsed[numEdges]->ARC = g->players[player - 1];
    g->gameBoard->edgesUsed[numEdges]->x = location.x;
    g->gameBoard->edgesUsed[numEdges]->y = location.y;

    g->players[player - 1]->ARCGrants++;
    g->players[player - 1]->students[STUDENT_BPS]--;
    g->players[player - 1]->students[STUDENT_BQN]--;
    // Give 'em a couple of points for a road
    g->players[player - 1]->KPIs = g->players[player - 1]->KPIs + 2;

    if (g->players[player - 1]->ARCGrants  > g->mostARCs->ARCGrants) {
        if (g->mostARCs->ARCGrants == 0) {
            free(g->mostARCs);
            g->mostARCs = g->players[player - 1];
            g->mostARCs->KPIs += 10;
        } else {
            g->mostARCs->KPIs += -10;
            g->mostARCs = g->players[player - 1];
            g->mostARCs->KPIs += 10;
        }
    }
}

// gives current player a publication
static void addPub(Game g, action a) {
    int player = getWhoseTurn(g);
    g->players[player - 1]->papers++;
    g->players[player - 1]->students[STUDENT_MJ]--;
    g->players[player - 1]->students[STUDENT_MTV]--;
    g->players[player - 1]->students[STUDENT_MMONEY]--;

    if (g->players[player - 1]->papers  > g->mostPubs->papers) {
        if (g->mostPubs->papers == 0) {
            free(g->mostPubs);
            g->mostPubs = g->players[player - 1];
            g->mostPubs->KPIs += 10;
        } else {
            g->mostPubs->KPIs += -10;
            g->mostPubs = g->players[player - 1];
            g->mostPubs->KPIs += 10;
        }
    }
}

// gives current player a patent
static void addPatent(Game g, action a) {
    int player = getWhoseTurn(g);
    g->players[player - 1]->patents++;
    g->players[player - 1]->students[STUDENT_MJ]--;
    g->players[player - 1]->students[STUDENT_MTV]--;
    g->players[player - 1]->students[STUDENT_MMONEY]--;
    // Give 'em some points for some special paper
    g->players[player - 1]->KPIs = g->players[player - 1]->KPIs + 10;
}

// retrains the current player's students
static void retrainStudents(Game g, action a) {
    int player = getWhoseTurn(g);
    int ratio = getExchangeRate(g, player,
        a.disciplineFrom, a.disciplineTo);
    g->players[player - 1]->students[a.disciplineFrom] += -1 * ratio;
    g->players[player - 1]->students[a.disciplineTo]++;
}

// advance the game to the next turn,
// assuming that the dice has just been rolled and produced diceScore
// the game starts in turn -1 (we call this state "Terra Nullis") and
// moves to turn 0 as soon as the first dice is thrown->
void throwDice(Game g, int diceScore) {
    g->currentTurn++;

    // give out resources based on diceScore
        int x = 0;
    while (x < NUM_REGIONS_X) {
        int y = 0;
        while (y < NUM_REGIONS_Y) {
            if (validRegion(x, y)) {
                if (g->gameBoard->regions[x][y]->diceValue == diceScore) {
                    int resource = g->gameBoard->regions[x][y]->discipline;
                    giveResource(g, x, y, resource);
                }
            }
            y++;
        }
        x++;
    }
    if (diceScore == 7) {
        int i = 0;
        while (i < NUM_UNIS) {
            g->players[i]->students[STUDENT_THD] +=
                g->players[i]->students[STUDENT_MMONEY] +
                g->players[i]->students[STUDENT_MTV];
            g->players[i]->students[STUDENT_MMONEY] = 0;
            g->players[i]->students[STUDENT_MTV] = 0;
            i++;
        }
    }
}

// distribute resources from a particular region to surrounding campuses
static void giveResource(Game g, int x, int y, int resource) {
    int xPoint = x;
    while (xPoint < x + 2) {
        int yPoint = 2*y + x%2;
        while (yPoint < 2*y  + x%2 + 3) {
            if (g->gameBoard->points[xPoint][yPoint]->contents !=
                VACANT_VERTEX) {
                if (g->gameBoard->points[xPoint][yPoint]->contents <=
                    NUM_UNIS) {
                    g->players[g->gameBoard->
                        points[xPoint][yPoint]->contents - 1]->
                            students[resource]++;
                } else {
                    // Give double for people with GO8 campus
                    g->players[g->gameBoard->points[xPoint][yPoint]->
                        contents - NUM_UNIS - 1]->students[resource] += 2;
                }
            }
            yPoint += 1;
        }
        xPoint++;
    }
}


/* **** Functions which GET data about the game aka GETTERS **** */

// what type of students are produced by the specified region?
// regionID is the index of the region in the newGame arrays (above)
// see discipline codes above
int getDiscipline(Game g, int regionID) {
    int x = 0;
    int discipline = 0;
    while (x < NUM_REGIONS_X) {
        int y = 0;
        while (y < NUM_REGIONS_Y) {
            if (validRegion(x, y)) {
                if (g->gameBoard->regions[x][y]->regionID == regionID) {
                    discipline = g->gameBoard->regions[x][y]->discipline;
                }
            }
            y++;
        }
        x++;
    }
    return discipline;
}

// what dice value produces students in the specified region?
// 2->->12
int getDiceValue(Game g, int regionID) {
    int x = 0;
    int dice = 0;
    while (x < NUM_REGIONS_X) {
        int y = 0;
        while (y < NUM_REGIONS_Y) {
            if (validRegion(x, y)) {
                if (g->gameBoard->regions[x][y]->regionID == regionID) {
                    dice = g->gameBoard->regions[x][y]->diceValue;
                }
            }
            y++;
        }
        x++;
    }
    return dice;
}

// which university currently has the prestige award for the most ARCs?
// this is NO_ONE until the first arc is purchased after the game
// has started->
int getMostARCs(Game g) {
    return g->mostARCs->uniID;
}

// which university currently has the prestige award for the most pubs?
// this is NO_ONE until the first publication is made->
int getMostPublications(Game g) {
    return g->mostPubs->uniID;
}

// return the current turn number of the game -1,0,1, ->->
int getTurnNumber(Game g) {
    return g->currentTurn;
}

// return the player id of the player whose turn it is
// the result of this function is NO_ONE during Terra Nullis
int getWhoseTurn(Game g) {
    int uniID = (g->currentTurn)%NUM_UNIS + 1;
    if (g->currentTurn == -1) {
        uniID = NO_ONE;
    }
    return uniID;
}

// return the contents of the given vertex (ie campus code or
// VACANT_VERTEX)
int getCampus(Game g, path pathToVertex) {
    point thePoint = pathToPoint(pathToVertex);
    assert(thePoint.x >= 0);
    return g->gameBoard->points[thePoint.x][thePoint.y]->contents;
}

// the contents of the given edge (ie ARC code or vacent ARC)
int getARC(Game g, path pathToEdge) {
    edge theEdge = pathToEdgeF(pathToEdge);
    theEdge = findEdgeDetails(theEdge.x, theEdge.y, g);
    int uniID = theEdge.ARC->uniID;
    if (uniID == VACANT_ARC) {
        free(theEdge.ARC);
    }
    return uniID;
}

// MOSS PAULY

// returns TRUE if it is legal for the current
// player to make the specified action, FALSE otherwise->
//
// "legal" means everything is legal:
//   * that the action code is a valid action code which is legal to
//     be made at this time
//   * that any path is well formed and legal ie consisting only of
//     the legal direction characters and of a legal length,
//     and which does not leave the island into the sea at any stage->
//   * that disciplines mentioned in any retraining actions are valid
//     discipline numbers, and that the university has sufficient
//     students of the correct type to perform the retraining
//
// eg when placing a campus consider such things as:
//   * is the path a well formed legal path
//   * does it lead to a vacent vertex?
//   * under the rules of the game are they allowed to place a
//     campus at that vertex?  (eg is it adjacent to one of their ARCs?)
//   * does the player have the 4 specific students required to pay for
//     that campus?
// It is not legal to make any action during Terra Nullis ie
// before the game has started->
// It is not legal for a player to make the moves OBTAIN_PUBLICATION
// or OBTAIN_IP_PATENT (they can make the move START_SPINOFF)
// you can assume that any pths passed in are NULL terminated strings->
int isLegalAction(Game g, action a) {
    int isLegal = TRUE;
    printf("Checking if an action is legal\n");
    int player = getWhoseTurn(g);
    int flag = 1;
    // Protect from stupid shit
    if ((a.actionCode < PASS) || (a.actionCode > RETRAIN_STUDENTS)) {
        printf("Invalid Action Code\n");
        flag = 0;
    } else if (a.actionCode == RETRAIN_STUDENTS) {
        printf("You've Called a retrain, checking if discipline");
        printf("within the bounds\n");
        // If it's a retrain action make sure the to
        // and from are in a nice range.
        if ((a.disciplineTo < STUDENT_THD)
            || (a.disciplineTo > STUDENT_MMONEY)) {
            printf("Invalid Discpline Code\n");
            flag = 0;
        }
        if ((a.disciplineFrom < STUDENT_THD)
            || (a.disciplineFrom > STUDENT_MMONEY)) {
            printf("Invalid Discpline Code\n");
            flag = 0;
        }
    }

    printf("The Flag when testing is %d\n", flag);

    if (flag == 1) {
        // Check all the basic stuff
        if (getTurnNumber(g) == TERRA_NULLIS) {
            isLegal = FALSE;
        } else if ((a.actionCode < 0) || (a.actionCode > MAX_ACTION)) {
            isLegal = FALSE;
        } else if ((getWhoseTurn(g) < 0) || (getWhoseTurn(g) > NUM_UNIS)) {
            isLegal = FALSE;
        } else if (a.actionCode == OBTAIN_IP_PATENT ||
            a.actionCode == OBTAIN_PUBLICATION) {
            isLegal = FALSE;
        }
        if (a.actionCode <= 3 && a.actionCode > 0 && isLegal) {
            if (validString(a.destination) == FALSE) {
                isLegal = FALSE;
            } else if (validPoint(pathToPoint(a.destination).x,
                pathToPoint(a.destination).y) == FALSE) {
                isLegal = FALSE;
            }
        }

        // edge actionEdge = pathToEdgeF (a.destination);

        // Check that someone can get an arc
        if (a.actionCode == OBTAIN_ARC && isLegal) {
            printf("Checking if the arc is legal. Arc:%s\n", a.destination);
            if (validNewEdge(g, pathToEdgeF(a.destination), player) == FALSE) {
                printf("Geographically Valid Edge\n");
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_BQN) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_BPS) < 1) {
                isLegal = FALSE;
            }
        }

        // Check that someone can get a Campus
        if (a.actionCode == BUILD_CAMPUS && isLegal) {
            if (validNewContents(g, a.destination, player) == FALSE) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_BQN) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_BPS) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MJ) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MTV) < 1) {
                isLegal = FALSE;
            }
        }

        if (a.actionCode == BUILD_GO8 && isLegal) {
            point actionPoint =  pathToPoint(a.destination);
            if (g->gameBoard->points[actionPoint.x][actionPoint.y]->contents !=
                player) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MJ) < 2) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MMONEY) < 3) {
                isLegal = FALSE;
            } else if (getTotalGO8s(g) == 8) {
                isLegal = FALSE;
            }
        }

        if (a.actionCode == START_SPINOFF && isLegal) {
            if (getStudents(g, player, STUDENT_MJ) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MTV) < 1) {
                isLegal = FALSE;
            } else if (getStudents(g, player, STUDENT_MMONEY) < 1) {
                isLegal = FALSE;
            }
        }

        if (a.actionCode == RETRAIN_STUDENTS && isLegal) {
            int exchange = getExchangeRate(g, player, a.disciplineFrom,
                a.disciplineTo);
            if (getStudents(g, player, a.disciplineFrom) < exchange) {
                isLegal = FALSE;
            } else if (a.disciplineFrom == STUDENT_THD) {
                isLegal = FALSE;
            }
        }
    } else if (flag == 0) {
        isLegal = FALSE;
    }
    return isLegal;
}
// --- get data about a specified player ---

// return the number of KPI points the specified player currently has
int getKPIpoints(Game g, int player) {
    return g->players[player - 1]->KPIs;
}

// return the number of ARC grants the specified player currently has
int getARCs(Game g, int player) {
    if (player <= 0) {
        player = 1;
    }
    return g->players[player - 1]->ARCGrants;
}

// return the number of GO8 campuses the specified player currently has
int getGO8s(Game g, int player) {
    return g->players[player - 1]->GO8Campuses;
}

// return the number of normal Campuses the specified player currently has
int getCampuses(Game g, int player) {
    return g->players[player - 1]->campuses;
}

// return the number of IP Patents the specified player currently has
int getIPs(Game g, int player) {
    return g->players[player - 1]->patents;
}

// return the number of Publications the specified player currently has
int getPublications(Game g, int player) {
    return g->players[player - 1]->papers;
}

// return the number of students of the specified discipline type
// the specified player currently has
int getStudents(Game g, int player, int discipline) {
    return g->players[player - 1]->students[discipline];
}

// return how many students of discipline type disciplineFrom
// the specified player would need to retrain in order to get one
// student of discipline type disciplineTo->  This will depend
// on what retraining centers, if any, they have a campus at->
int getExchangeRate(Game g, int player,
                     int disciplineFrom, int disciplineTo) {
    printf("in getExchange rate\n");
    int ratio = 3;
    if (disciplineFrom == STUDENT_MMONEY) {
        if (g->gameBoard->points[3][1]->contents == player
            || g->gameBoard->points[3][1]->contents == player + NUM_UNIS) {
            ratio = 2;
        } else if (g->gameBoard->points[4][1]->contents == player
            || g->gameBoard->points[4][1]->contents == player + NUM_UNIS) {
            ratio = 2;
        }
    } else if (disciplineFrom == STUDENT_BQN) {
        if (g->gameBoard->points[5][5]->contents == player
            || g->gameBoard->points[5][5]->contents == player + NUM_UNIS) {
            ratio = 2;
        } else if (g->gameBoard->points[5][6]->contents == player
            || g->gameBoard->points[5][6]->contents == player + NUM_UNIS) {
            ratio = 2;
        }
    } else if (disciplineFrom == STUDENT_MJ) {
        if (g->gameBoard->points[4][8]->contents == player
            || g->gameBoard->points[4][8]->contents == player + NUM_UNIS) {
            ratio = 2;
        } else if (g->gameBoard->points[4][9]->contents == player
            || g->gameBoard->points[4][9]->contents == player + NUM_UNIS) {
            ratio = 2;
        }
    } else if (disciplineFrom == STUDENT_BPS) {
        if (g->gameBoard->points[1][8]->contents == player
            || g->gameBoard->points[1][8]->contents == player + NUM_UNIS) {
            ratio = 2;
        } else if (g->gameBoard->points[1][9]->contents == player
            || g->gameBoard->points[1][9]->contents == player + NUM_UNIS) {
            ratio = 2;
        }
    } else if (disciplineFrom == STUDENT_MTV) {
        if (g->gameBoard->points[1][1]->contents == player
            || g->gameBoard->points[1][1]->contents == player + NUM_UNIS) {
            ratio = 2;
        } else if (g->gameBoard->points[2][1]->contents == player
            || g->gameBoard->points[2][1]->contents == player + NUM_UNIS) {
            ratio = 2;
        }
    }
    printf("end of getExchange rate\n");
    return ratio;
}

// converts a given path to the point's coordinates
static point pathToPoint(path givenPath) {
    point newPoint;
    newPoint.x = 2;
    newPoint.y = 0;
    int diff = START_DIFF;

    int i = 0;
    while (givenPath[i] != '\0') {
        printf("Path to point while loop entered\n");
        if (!(givenPath[i] == 'L' || givenPath[i] == 'R'
            || givenPath[i] == 'B')) {
            newPoint.x = -1;
        } else {
            point oldPoint = newPoint;
            newPoint = charToPoint(givenPath[i], diff, newPoint);
            if (newPoint.x != -1) {
                if (oldPoint.x + 1 == newPoint.x) {
                    diff = COL_INC;
                } else if (oldPoint.x - 1 == newPoint.x) {
                    diff = COL_DEC;
                } else if (oldPoint.y - 1 == newPoint.y) {
                    diff = ROW_DEC;
                } else if (oldPoint.y + 1 == newPoint.y) {
                    diff = ROW_INC;
                }
            }
        printf("Path to point while loop finished\n");
        }
        i++;
    }
    return newPoint;
}

// converts a given path to the edge's coordinates
static edge pathToEdgeF(path givenPath) {
    printf("Entered pathToEdgeF function\n");
    point newPoint;
    newPoint.x = 2;
    newPoint.y = 0;
    int diff = START_DIFF;

    int i = 0;
    while (givenPath[i] != '\0') {
        printf("Converting path to edge coords\n");
        assert(givenPath[i] == 'L'
            || givenPath[i] == 'R'
            || givenPath[i] == 'B');
        point oldPoint = newPoint;
        newPoint = charToPoint(givenPath[i], diff, newPoint);
        if (oldPoint.x + 1 == newPoint.x) {
            diff = COL_INC;
        } else if (oldPoint.x - 1 == newPoint.x) {
            diff = COL_DEC;
        } else if (oldPoint.y - 1 == newPoint.y) {
            diff = ROW_DEC;
        } else if (oldPoint.y + 1 == newPoint.y) {
            diff = ROW_INC;
        }
        i++;
    }
    edge newEdge;
    if (diff == COL_INC) {
        newEdge.x = newPoint.x - 0.5;
        newEdge.y = newPoint.y;
    } else if (diff == COL_DEC) {
        newEdge.x = newPoint.x + 0.5;
        newEdge.y = newPoint.y;
    } else if (diff == ROW_INC) {
        newEdge.x = newPoint.x;
        newEdge.y = newPoint.y - 0.5;
    } else if (diff == ROW_DEC) {
        newEdge.x = newPoint.x;
        newEdge.y = newPoint.y + 0.5;
    }
    else { // got just "\0" in the string
    // this will never be reached because START_DIFF = ROW_INC
    // is that ok?
        printf("none works?\n");
        newEdge.x = 0;
        newEdge.y = 0;
    }
    return newEdge;
}



// takes a char (L, R, B), point, and initial direction and gets
// the next point
static point charToPoint(char letter, int difference, point currentPoint) {
    if (difference == COL_DEC) {
        if (letter == 'L') {
            currentPoint.y++;
        } else if (letter == 'R') {
            currentPoint.y--;
        } else if (letter == 'B') {
            currentPoint.x++;
        }
    } else if (difference == COL_INC) {
        if (letter == 'L') {
            currentPoint.y--;
        } else if (letter == 'R') {
            currentPoint.y++;
        } else if (letter == 'B') {
            currentPoint.x--;
        }
    } else if (difference == ROW_DEC) {
        if (currentPoint.x%2 == currentPoint.y%2) {
            if (letter == 'L') {
                currentPoint.y--;
            } else if (letter == 'R') {
                currentPoint.x++;
            } else if (letter == 'B') {
                currentPoint.y++;
            }
        } else {
            if (letter == 'L') {
                currentPoint.x--;
            } else if (letter == 'R') {
                currentPoint.y--;
            } else if (letter == 'B') {
                currentPoint.y++;
            }
        }
    } else if (difference == ROW_INC) {
        if (currentPoint.x%2 == currentPoint.y%2) {
            if (letter == 'L') {
                currentPoint.x++;
            } else if (letter == 'R') {
                currentPoint.y++;
            } else if (letter == 'B') {
                currentPoint.y--;
            }
        } else {
            if (letter == 'L') {
                currentPoint.y++;
            } else if (letter == 'R') {
                currentPoint.x--;
            } else if (letter == 'B') {
                currentPoint.y--;
            }
        }
    }

    if (!validPoint(currentPoint.x, currentPoint.y)) {
        currentPoint.x = -1;
    }
    return currentPoint;
}

// checks a point is on the board
static int validPoint(int x, int y) {
    int isValidPoint = TRUE;
    if (x < 0 || y < 0
        || x > 5 || y > 10) {
        isValidPoint = FALSE;
    } else if (x - 0 + y - 0 <= 1) {
        isValidPoint = FALSE;
    } else if (5 - x + y - 0 <= 1) {
        isValidPoint = FALSE;
    } else if (x - 0 + 10 - y <= 1) {
        isValidPoint = FALSE;
    } else if (5 - x + 10 - y <= 1) {
        isValidPoint = FALSE;
    }
    return isValidPoint;
}

static int validRegion(int x, int y) {
    int isValidRegion = TRUE;
    if (x < 0 || y < 0
        || x > 4 || y > 4) {
        isValidRegion = FALSE;
    } else if ((x == 0 || x == 4) && y == 0) {
        isValidRegion = FALSE;
    } else if ((x != 2) && y == 4) {
        isValidRegion = FALSE;
    }
    return isValidRegion;
}

/*
static point findPointDetails (int x, int y, Game g) {
    point thePoint;
    thePoint.x = x;
    thePoint.y = y;
    thePoint.contents = VACANT_VERTEX;
    int i = 0;
    while (i < NUM_POINTS) {
        if (g->gameBoard->points[i]->x == x && g->gameBoard->points[i]->y == y) {
            thePoint = g->gameBoard->points[i];
        }
        i++;
    }
    return thePoint;
}*/



static int validString(path destination) {
    int isValidString = TRUE;
    int i = 0;

    while (destination[i] != '\0') {
        char chr = destination[i];
        if ((chr != 'L') && (chr != 'R') && (chr != 'B')) {
            isValidString = FALSE;
        }
        i++;
    }

    if (i > PATH_LIMIT) {
        isValidString = FALSE;
    }
    return isValidString;
}
/*
//Some quick little tests for valid string
assert(validString ("LLLR") == TRUE);
assert(validString ("RLRLR") == TRUE);
assert(validString ("LRRRRRRLLR") == TRUE);
assert(validString ("LaslkfhaifhaiosR") == FALSE);*/


static edge findEdgeDetails(double x, double y, Game g) {
    edge theEdge;
    theEdge.x = x;
    theEdge.y = y;
    theEdge.ARC = malloc (sizeof (struct _uni));
    theEdge.ARC->uniID = VACANT_ARC;
    int numEdges = 0;
    int i = 0;
    while (i < NUM_UNIS) {
        numEdges += getARCs(g, i + 1);
        i++;
    }
    i = 0;
    while (i < numEdges) {
        if (g->gameBoard->edgesUsed[i]->x == x &&
            g->gameBoard->edgesUsed[i]->y == y) {
            free(theEdge.ARC);
            theEdge.ARC = g->gameBoard->edgesUsed[i]->ARC;
        }
        i++;
    }
    return theEdge;
}


static int validNewEdge(Game g, edge line, int player) {
    printf("Entering validNewEdge\n");
    int isValid = FALSE;
    double x = line.x;
    double y = line.y;
    point points[4];
    int i = 0;

    printf("Getting points in validNewEdge. Line x:%lf y:%lf\n", line.x, line.y);
    // Get all possible points
    if ((x >= 0) && (y >= 0)) {
        points[0] = * (g->gameBoard->points[(int)floor(x)][(int)floor(y)]);
        printf("Obtained a point\n");
        points[1] = *(g->gameBoard->points[(int)ceil(x)][(int)floor(y)]);
        points[2] = *(g->gameBoard->points[(int)floor(x)][(int)ceil(y)]);
        points[3] = *(g->gameBoard->points[(int)ceil(x)][(int)ceil(y)]);
        printf("Obtained new points in validNewEdge\n");

        printf("About to enter while loop in validNewEdge\n");
        while (i < 4) {
            printf("In point while loop\n");
            if ((points[i].contents != 0) && (points[i].contents % NUM_UNIS ==
                player % NUM_UNIS)) {
                isValid = TRUE;
            }
            i++;
        }

        // Check it's not already occupied
        line = findEdgeDetails(x, y, g);
        // Check if any of the surrounding edges are owned by the player
        double x1 = 0;
        double x2 = 0.5;
        double y1 = 1;
        double y2 = 0.5;
        if (floor(y) == y) {
            x1 = 0.5;
            y1 = -0.5;
        } else if (((int)floor (y))%2 == ((int)(x))%2) {
            x2 = -0.5;
        } 
        edge adjacentEdge = findEdgeDetails(line.x + x1,
            line.y + y1, g);
        if (adjacentEdge.ARC->uniID == player) {
            isValid = TRUE;
        } else if (adjacentEdge.ARC->uniID == VACANT_ARC) {
            free(adjacentEdge.ARC);
        }
        adjacentEdge = findEdgeDetails(line.x - x1,
            line.y - y1, g);
        if (adjacentEdge.ARC->uniID == player) {
            isValid = TRUE;
        } else if (adjacentEdge.ARC->uniID == VACANT_ARC) {
            free(adjacentEdge.ARC);
        }
        adjacentEdge = findEdgeDetails(line.x + x2,
            line.y + y2, g);
        if (adjacentEdge.ARC->uniID == player) {
            isValid = TRUE;
        } else if (adjacentEdge.ARC->uniID == VACANT_ARC) {
            free(adjacentEdge.ARC);
        }
        adjacentEdge = findEdgeDetails(line.x - x2,
            line.y - y2, g);
        if (adjacentEdge.ARC->uniID == player) {
            isValid = TRUE;
        } else if (adjacentEdge.ARC->uniID == VACANT_ARC) {
            free(adjacentEdge.ARC);
        }
        printf("Exiting validNewEdge\n");
    } else {
        isValid = FALSE;
    }
    if (line.ARC->uniID != VACANT_ARC) {
        isValid = FALSE;
    } else {
        free(line.ARC);
    }
    return isValid;
}

// Check a campus can go there
static int validNewContents(Game g, path destination, int player) {
    int i = 0;
    // double x = 0;
    // double y = 0;
    int isValid = FALSE;
    point newPoint = pathToPoint(destination);
    assert(newPoint.x >= 0);
    // point points[POSSIBLE_POINTS];
    int adjacent = 0;

    long pathLength = strlen(destination);
    path adj[3];
    strcpy(adj[0], destination);
    strcpy(adj[1], destination);
    strcpy(adj[2], destination);
    adj[0][pathLength] = 'L';
    adj[0][pathLength + 1] = '\0';
    adj[1][pathLength] = 'R';
    adj[1][pathLength + 1] = '\0';
    adj[2][pathLength] = 'B';
    adj[2][pathLength + 1] = '\0';
    while (i < 3) {
        point aPoint = pathToPoint(adj[i]);
        if (aPoint.x >= 0) {
            if (g->gameBoard->points[aPoint.x][aPoint.y]->contents != 0) {
                adjacent++;
                isValid = FALSE;
            }
        }
        edge anEdge = pathToEdgeF(adj[i]);
        anEdge = findEdgeDetails(anEdge.x, anEdge.y, g);
        if (anEdge.ARC->uniID == player && adjacent == 0) {
            isValid = TRUE;
        }
        if (anEdge.ARC->uniID == VACANT_ARC) {
            free(anEdge.ARC);
        }
        i++;
    }
    if (getCampus(g, destination) != 0) {
        isValid = FALSE;
    }

    return  isValid;
}

static int getTotalEdges(Game g) {
    int numEdges = 0;
    int i = 0;
    while (i < NUM_UNIS) {
        numEdges += getARCs(g, i + 1);
        i++;
    }
    return numEdges;
}

static int getTotalGO8s(Game g) {
    int numGO8s = 0;
    int i = 0;
    while (i < NUM_UNIS) {
        numGO8s += getGO8s(g, i + 1);
        i++;
    }
    return numGO8s;
}
