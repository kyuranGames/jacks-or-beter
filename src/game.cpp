#include "game.h"
#include "resources.h"

// helper functions

static void draw();

// game state functions

static void menuTitle();

void newRound();
static void gameSetup();
static void gameMain();
typedef struct {
    u8 suit;
    u8 rank;
} card;

card hand[5];
u8 visibility[5];

u8 deck[52];
u8 posdeck;
int result;

int offset;

u8 pos;

int score;
int bet;
int lastwin;

u8 dispDialog;

int step = 0;

typedef void (*tick_func_t)();

//static tick_func_t gameTick {&menuTitle};
static tick_func_t gameTick = &menuTitle;


void tick()
{
    gameTick();
}

unsigned int rng()
{
    static unsigned int y = 0;

    y += micros(); // seeded with changing number
    y ^= y << 2; y ^= y >> 7; y ^= y << 7;
    return (y);
}


byte sequenceFrame;
const unsigned char PROGMEM frameSequences[] = {
    // bouncing Ball Frame Sequence
    0, 1, 2, 0, 0, 0, 0, 0,
    // bouncing Ball Height Sequence
    0, 0, 0, 1, 0, 0, 0, 0,
    // game title Height Sequence
    1, 2, 3, 4, 5, 4, 3, 2,
};

void menuCredits()
{
/*    u8 diamondsR[88 * 3 + 2];
 *
 *  arduboy.clear();
 *
 *
 *  if (arduboy.everyXFrames(2)) sequenceFrame++;
 *  if (sequenceFrame > 7) sequenceFrame = 0;
 *
 *  sprites.drawOverwrite(20, pgm_read_byte(&frameSequences[sequenceFrame + 16]), diamondsborder, 0);
 *
 *
 *  memcpy_P(diamondsR, diamonds, 88 * 3 + 2);
 *  for (int i = 2; i < 88 * 3 + 2; i++) {
 *      diamondsR[i] = diamondsR[i] & rng();
 *  }
 *
 * //        sprites.drawOverwrite(20, 0, diamondsR, 0);    // Doesn't work (image have to be in PROGMEM with this function)
 *
 *  drawOverwriteMaskDynamic(20, 0, diamondsR, 0);
 *
 */
    if ((arduboy.justPressed(A_BUTTON)) |
        (arduboy.justPressed(B_BUTTON)) |
        (arduboy.justPressed(UP_BUTTON)) |
        (arduboy.justPressed(DOWN_BUTTON))) {
        gameTick = &menuTitle;
    }

/*
 *  arduboy.setCursor(4, 32);
 *  arduboy.print("Idea    Oliver Dreer");
 *  arduboy.setCursor(4, 40);
 *  arduboy.print("GFX/HP48 Doug Cannon");
 *  arduboy.setCursor(4, 50);
 *  arduboy.print("Code   RedBug/Kyuran");
 *
 *
 *  if (!sound.playing()) {
 *      sound.playScore(song);
 *  }
 */
} // menuCredits

void nextRound()
{

    draw();

    char line[64];

    // sprintf(line, "Press A to continue");
    // arduboy.setCursor((128 - strlen(line) * 6) / 2, 20);
    // arduboy.print(line);

    if (visibility[0] != 0) {
    if (lastwin > 0) {
        sprintf(line, "You win %d", lastwin);
    } else if (lastwin < 0) {
        sprintf(line, "You loose %d", -lastwin);
    } else {
        strcpy(line, "Draw game");
    }
    arduboy.setCursor((128 - strlen(line) * 6) / 2, 16);
    arduboy.print(line);
  }


    if (arduboy.everyXFrames(30)) {
        dispDialog = !dispDialog;     // Propose to change the bet
    }

    // if (dispDialog) {
//    sprites.drawOverwrite(2 + (128 - 72) / 2, 2 + (60 - 15) / 2, diamgetready, 0);
// }

    if (offset<40) {
        sprites.drawOverwrite(2 + (128 - 100) / 2, offset-15, diamgetready, 0);
      } else  {
        sprites.drawOverwrite(2 + (128 - 100) / 2, 25, diamgetready, 0);
      }

    // }


    if (arduboy.justPressed(A_BUTTON)) {
        newRound();
        gameTick = &gameMain;
    }

    if (arduboy.justPressed(UP_BUTTON)) {
        if (bet >= 100) {
            bet += 100;
        } else if (bet >= 10) {
            bet += 10;
        } else {
            bet++;
        }
        if (bet > score) {
            bet = score;
        }
    }

    if (arduboy.justPressed(DOWN_BUTTON)) {
        if (bet >= 200) {
            bet -= 100;
        } else if (bet >= 20) {
            bet -= 10;
        } else if (bet > 1) {
            bet--;
        }
    }


    offset++;
} // nextRound


void menuTitle()
{
    sprites.drawOverwrite(0, 0, intro, 0);

    if (arduboy.justPressed(A_BUTTON)) {
        gameTick = &gameSetup;
        sound.toneMutesScore(true);
    }

    if (arduboy.justPressed(B_BUTTON)) {
        if (arduboy.audio.enabled()) {
            arduboy.audio.off();
        } else {
            arduboy.audio.on();
        }
        arduboy.audio.saveOnOff();
    }

    if (arduboy.justPressed(DOWN_BUTTON)) {
        gameTick = &menuCredits;
    }

    if (!arduboy.audio.enabled()) {
        arduboy.fillRect(128 - 16, 0, 16, 16, BLACK);
    }

    // if (arduboy.everyXFrames(30)) {
    //     dispDialog = !dispDialog;
    // }
    //
    // if (dispDialog) {
    //     sprites.drawOverwrite(2 + (128 - 72) / 2, 2 + (60 - 15) / 2, diamplay, 0);
    // }


    if (!sound.playing()) {
        sound.playScore(song);
    }

} // menuTitle


// Poker

void shuffleDeck()
{
    for (int i = 0; i < 52; i++) {
        deck[i] = i;
    }

    // Fisher-Yates shuffle
    for (int i = 51; i > 0; i--) {
        int j = rand() % i;
        u8 tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
    posdeck = 0;
}

/*
 * function sortPokerHand
 * This function sorts the poker hand by rank
 */
void sortPokerHandRank()
{
    int tempSuit = 0;
    int tempRank = 0;

    for ( int i = 0; i < 4; i++ ) {
        for (int j = i + 1; j < 5; j++) {
            tempSuit = hand[i].suit;
            tempRank = hand[i].rank;
            if ( hand[i].rank > hand[j].rank ) {
                hand[i].suit = hand[j].suit;
                hand[i].rank = hand[j].rank;
                hand[j].suit = tempSuit;
                hand[j].rank = tempRank;
            }
        }
    }
}


int testHandOfAKind()
{
    /*
     * int ranking = 4;
     *
     * while(ranking>0) {
     *
     *    for(int i = 0; i <= 5-ranking; i++) {
     *      u8 count=0;
     *
     *      for(int j = 0; j < ranking; j++) {
     *        if (hand[j+i].rank==hand[j+i+1].rank) {
     *          count++;
     *        }
     *      }
     *
     *      if (count==ranking) {
     *        return ranking;
     *      }
     *    }
     *
     *    ranking--;
     *  }
     *
     *
     * return 0; */

    u8 rank[13] = {0};
    int ranking = 0;

    for ( int i = 0; i < 13; i++ ) {
        rank[i] = 0;
    }

    for (int i = 0; i < 5; i++) {
        rank[hand[i].rank]++;
    }


    for ( int i = 0; i < 13; i++ ) {
        if ( rank[i] > ranking ) {
            ranking = rank[i];
        }
    }

    return ranking;
} // testHandOfAKind

int checkMaxRank()
{
    u8 rank[13] = {0};
    int ranking = 0;

    for ( int i = 0; i < 13; i++ ) {
        rank[i] = 0;
    }

    for (int i = 0; i < 5; i++) {
        if (hand[i].rank >= 10) {
            rank[hand[i].rank]++;
        }
    }

    for ( int i = 0; i < 13; i++ ) {
        if ( rank[i] > ranking ) {
            ranking = rank[i];
        }
    }

    return ranking;
} // checkMaxRank

int testFullHouse()
{
    int fullHouse = 0;

    if ( ( hand[0].rank == hand[1].rank) &&
         ( hand[2].rank == hand[3].rank && hand[3].rank == hand[4].rank) ) {
        fullHouse = 1;
    } else if ( ( hand[0].rank == hand[1].rank && hand[1].rank == hand[2].rank) &&
                ( hand[3].rank == hand[4].rank) ) {
        fullHouse = 1;
    }

    return fullHouse;
}

int testHandFlush()
{
    int isFlush = 0;
    int suit[5] = {0};

    for (int i = 0; i < 5; i++) {
        suit[hand[i].suit]++;
    }

    for ( int i = 0; i < 4; i++ ) {
        if ( suit[i] == 5 ) {
            isFlush = 1;
        }
    }


    return isFlush;
}


int testHandTwoKind()
{
    int twoOfKind = 0;

    if ( hand[0].rank == hand[1].rank && hand[2].rank == hand[3].rank ) {
        twoOfKind = 1;
    } else if ( hand[0].rank == hand[1].rank && hand[3].rank == hand[4].rank ) {
        twoOfKind = 1;
    } else if ( hand[1].rank == hand[2].rank && hand[3].rank == hand[4].rank ) {
        twoOfKind = 1;
    }

    return twoOfKind;
}

int testHandStraight()
{
    int isStraight = 0;
    int rank[5] = {0};

    for ( int i = 0; i < 5; i++ ) {
        rank[i] = hand[i].rank;
    }

    if ( (rank[0] + 1 == rank[1]) && (rank[1] + 1 == rank[2]) && (rank[2] + 1 == rank[3]) && (rank[3] + 1 == rank[4]) ) {
        isStraight = 1;
    } else if ( (rank[0] == 1) && (rank[1] == 10) && (rank[2] == 11) && (rank[3] == 12) && (rank[4] == 13) ) {
        isStraight = 1;
    }

    return isStraight;
}


// Game initialization

void computeResult()
{
    sortPokerHandRank();

    result = -1;

    u8 isPokerHandOfKind = testHandOfAKind();
    u8 isTwoKind = testHandTwoKind();
    u8 isFlush = testHandFlush();
    u8 isStraight = testHandStraight();
    u8 isFullHouse = testFullHouse();
    u8 maxRank = checkMaxRank();

    if ((isFlush) && (isStraight) && (hand[0].rank == 0)) { // Royal Flush
        result = 0;
    } else if ((isFlush) && (isStraight)) { // Straight Flush
        result = 1;
    } else if ((isPokerHandOfKind == 4)) { // 4 of a kind
        result = 2;
    } else if ((isFullHouse)) { // Full house
        result = 3;
    } else if ((isFlush)) { // Flush
        result = 4;
    } else if ((isStraight)) { // Straight
        result = 5;
    } else if ((isPokerHandOfKind == 3)) { // 3 of a kind
        result = 6;
    } else if ((isTwoKind)) { // Two pair
        result = 7;
    } else if (maxRank >= 2) { // Jacks or better
        result = 8;
    }


} // computeResult

void newRound()
{

    shuffleDeck();

    for (int i = 0; i < 5; i++) {
        hand[i].suit = deck[posdeck] / 13;
        hand[i].rank = deck[posdeck] - (hand[i].suit * 13);
        posdeck++;
        visibility[i] = 1;
    }

    computeResult();

    step = 0;
}

void gameSetup()
{
    arduboy.initRandomSeed();

    pos = 0;

    score = 100;
    bet = 25;

    newRound();

    offset=0;

    for (int i = 0; i < 5; i++) {
        visibility[i] = 0;
    }

    gameTick = &nextRound;
}

void gameOver()
{

    draw();

    if (arduboy.everyXFrames(30)) {
        dispDialog = !dispDialog;
    }

    if (dispDialog) {
        sprites.drawOverwrite(2 + (128 - 72) / 2, 2 + (60 - 15) / 2, diamgameover, 0);
    }

    if ((arduboy.justPressed(A_BUTTON)) |
        (arduboy.justPressed(B_BUTTON)) |
        (arduboy.justPressed(UP_BUTTON)) |
        (arduboy.justPressed(DOWN_BUTTON))) {
        gameTick = &menuTitle;
    }

}



void playSound(int i)
{

    switch (i) {
        case 1:
            sound.tone(WALL_FREQ, WALL_DUR); // Good bounce
            break;
        case 2:
            sound.tone(WALL_FREQ, PADDLE_DUR); // Bounce
            break;
    }


}

void menuHelp()
{
  char pos=offset-64;
  static u8 first=0;

  if (pos>0) {
      pos=0;
    }

  sprites.drawOverwrite(0, pos, help, 0);

  if ((arduboy.justPressed(A_BUTTON)) |
      (arduboy.justPressed(B_BUTTON)) |
      (arduboy.justPressed(UP_BUTTON)) |
      (arduboy.justPressed(DOWN_BUTTON))) {
      gameTick = &gameMain;
  }

  u8 sc=first;
  for(char y=0;y<=64;y+=16) {
    char py=y-(offset%16);
    sprites.drawOverwrite(0, py, suit, sc);
    sprites.drawOverwrite(118, py, suit, sc);
    sc=(sc+1)%4;
  }

  if ((offset%16)==15) {
    first=(first+1)%4;
  }

  offset++;
}

// Running game

void gameMain()
{

    char line[64];

    draw();


    if (arduboy.justPressed(LEFT_BUTTON)) {
        if (pos > 0) {
            pos--;
        } else {
            pos = 4;
        }
    }

    if (arduboy.justPressed(RIGHT_BUTTON)) {
        if (pos < 4 ) {
            pos++;
        } else {
            pos = 0;
        }
    }

    if (arduboy.justPressed(A_BUTTON)) {
        if (visibility[pos] == 0) {
            visibility[pos] = 1;
        } else {
            visibility[pos] = 0;
        }
    }

    if (arduboy.justPressed(UP_BUTTON)) {
        //
    }

    if (arduboy.justPressed(DOWN_BUTTON)) {
        // Display help

        arduboy.clear();

        offset = 0;
        gameTick = &menuHelp;
    }

    if (arduboy.justPressed(B_BUTTON)) {

        step++;

        for (int i = 0; i < 5; i++) {
            if (!visibility[i]) {

                hand[i].suit = deck[posdeck] / 13;
                hand[i].rank = deck[posdeck] - (hand[i].suit * 13);

                visibility[i] = 1;
                posdeck++;
            }
        }

        computeResult();

        if (step == 2) {
            lastwin = 0;
            if (result >= 0) {
                u8 coef[] = {250, 50, 25, 9, 6, 4, 3, 2, 1};
                lastwin = bet * coef[result];
            }
            lastwin = lastwin - bet;

            score = score + lastwin;

            offset=0;
            gameTick = &nextRound;

            if (score <= 0) {
                gameTick = &gameOver;
            }

            if (bet > score) {
                bet = score;
            }


        }



//        gameTick = &menuTitle;
//        sound.toneMutesScore(false);
    }
    //
    // if (arduboy.justPressed(A_BUTTON)) {
    //     if (life == 0) {
    //         gameTick = &menuTitle;
    //         sound.toneMutesScore(false);
    //     }
    // }

} // gameMain2



void draw()
{
    sprites.drawOverwrite(0, 0, diammain, 0);

    char line[64];

    if (score > 32000) {
        score = 32000;
    }

    if (bet > 999) {
        bet = 999;
    }

    sprintf(line, "Score:%5d Bet:%3d %d", score, bet, 2 - step);
    arduboy.setCursor(0, 0);
    arduboy.print(line);

    if (result >= 0) {

        switch (result) {
            case 0:
                strcpy(line, "Royal Flush");
                break;
            case 1:
                strcpy(line, "Straight Flush");
                break;
            case 2:
                strcpy(line, "4 of a kind");
                break;
            case 3:
                strcpy(line, "Full house");
                break;
            case 4:
                strcpy(line, "Flush");
                break;
            case 5:
                strcpy(line, "Straight");
                break;
            case 6:
                strcpy(line, "3 of a kind");
                break;
            case 7:
                strcpy(line, "Two pair");
                break;
            case 8:
                strcpy(line, "Jacks or better");
                break;
        } // switch

        arduboy.setCursor((128 - strlen(line) * 6) / 2, 8);
        arduboy.print(line);
    }

    for (int i = 0; i < 5; i++) {
        sprites.drawOverwrite(2 + i * 25, 36, cart, visibility[i]); // 0: hidden, 1: visible
        if (visibility[i]) {
            sprites.drawOverwrite(4 + i * 25, 43, value, hand[i].rank);
            sprites.drawOverwrite(14 + i * 25, 43, suit, hand[i].suit);
        }

        if (pos != i) {
            arduboy.fillRect(2 + i * 25, 61, 24, 3, BLACK);
        }

    }



    // arduboy.fillRect(x3 + 0, y3 + 1, 1 + 3 + 1, 0 + 3 + 0, WHITE);
    // arduboy.fillRect(x3 + 1, y3 + 0, 0 + 3 + 0, 1 + 3 + 1, WHITE);

} // draw2
