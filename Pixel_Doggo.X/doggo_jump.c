#include <xc.h> // for the core timer delay
#include "ssd1306.h"
#include "animations.h"
#include "doggo_jump.h"

void doggo_run(int frame) {
    // load next frame of running animation
    char color;
    char i;
    char j;
    for (i=0; i<17; i++) {
        for (j=0; j<11; j++) {
            color = doggo[frame][i]>>j & 1;
            ssd1306_drawPixel(X_POS+i, Y_POS-j, color);
        }
    }
}

void ground_move() {
    // load next frame of ground animation
    static char frame = 0;
    char color;
    int i;
    char j;
    for (i=0;i<128;i++) {
        for (j=0; j<4; j++) {
            color = ground[(i+frame)%10]>>j & 1;
            ssd1306_drawPixel(i, 31-j, color);
        }
    }
    frame = frame+4;
    if (frame==20) {
        frame = 0;
    }
}

void doggo_jump(int frame) {
    // load a frame of the jumping animation
    char color;
    char i;
    char j;
    for (i=0; i<17; i++) {
        for (j=0; j<11; j++) {
            color = doggo_jumping[i]>>j & 1;
            ssd1306_drawPixel(X_POS+i, Y_POS-j-jump_parabola[frame], color);
        }
    }
}

void doggo_wag(void) {
    // load next frame of wagging animation
    static char frame = 0;
    char color;
    char i;
    char j;
    for (i=0; i<17; i++) {
        for (j=0; j<11; j++) {
            color = doggo_intro[wag_pattern[frame]][i]>>j & 1;
            ssd1306_drawPixel(X_POS+i, Y_POS-j, color);
        }
    }
    frame++;
    if (frame==6) {
        frame = 0;
    }
}

void doggo_woof(void) {
    // display image of doggo woofing (head only)
    char color;
    char i;
    char j;
    for (i=14; i<17; i++) {
        for (j=0; j<11; j++) {
            color = doggo_intro[3][i]>>j & 1;
            ssd1306_drawPixel(X_POS+i, Y_POS-j, color);
        }
    }
    for (i=0; i<19; i++) {
        for (j=0; j<4; j++) {
            color = woof[i]>>j & 1;
            ssd1306_drawPixel(38+i, 18-j, color);
        }
    }
}

void print_pbtp(void) {
    // display "push button to play" for title screen
    int x_pos = 30;
    int y_pos = 23;
    char color;
    char i;
    char j;
    for (i=0; i<35; i++) {
        for (j=0; j<3; j++) {
            color = pbtp[i]>>j & 1;
            ssd1306_drawPixel(x_pos+(i*2), y_pos-j, color);
        }
        for (j=4; j<7; j++) {
            color = pbtp[i]>>j & 1;
            ssd1306_drawPixel(x_pos+(i*2)-1, y_pos+4-j, color);
        }
    }
}

void obstacles() {
    // manages obstacles and collisions
    char collision = 0;  // initialize collision detection off each call
    static char obstacle_flag[3] = {0,0,0};  // which obstacles are on the screen, initialize all off at beginning of game 
    static signed short obstacle_pos[3] = {-8,-8,-8}; // obstacle positions, initialize all just left of the screen
    static int rand_int = 0;  // random counter starts between 0 and 25, used to vary distance between obstacles
    char i;
    
    for (i=0; i<3; i++) {
        if (obstacle_flag[i]==0 & rand_int == 0) {  // obstacle is ready to spawn
            if (obstacle_pos[(i+1)%3]<90 & obstacle_pos[(i+2)%3]<90) {  // doggo has enough space to land and jump between obstacles
                obstacle_pos[i] = 136;  // place obstacle
                obstacle_flag[i] = 1;  // turn on flag
            }
        }
        if (obstacle_flag[i]==1) {  // obstacle is already on screen
            obstacle_pos[i]-= 4;  // set new obstacle position
            draw_obstacle(obstacle_pos[i]);  // draw obstacle in new position
            if (obstacle_pos[i]<43 & obstacle_pos[i]>10) {  // within collision range of doggo 
                collision = check_collision(obstacle_pos[i]);  // check for collision
            }
            if (obstacle_pos[i]<-7) {  // obstacle has moved off the screen
                obstacle_flag[i] = 0;  // reset flag
                obstacle_pos[i] = -8;  // set obstacle back to start position
            }
        }
    }
    if (rand_int<1) { // reset random counter
        rand_int = rand() % (25);
    }
    rand_int--;  // subtract one from random counter each call

    if (collision) {  // check for collision
        game_over = 1;  // set game_over flag
        ssd1306_drawString("GAME OVER",0,0);  // print "GAME OVER" in upper right hand corner
        for (i=0; i<3; i++) {
            obstacle_flag[i] = 0;   // reset flags
            obstacle_pos[i] = 0;    // reset positions
        }
        rand_int = 0;               // reset random counter
    }
}

void draw_obstacle(signed short pos) {  // display obstacle at a given x position
    char color;
    char i;
    char j;
    for (i=0; i<15; i++) {
        for (j=0; j<bush_mask[i]; j++) {
            color = bush[i]>>j & 1;
            ssd1306_drawPixel(pos+i-8, Y_POS+1-j, color);
        }
    }
}

char check_collision(signed short obstacle_pos) {  // check for a collision
    char collision = 0;
    if (bush_mask[8-obstacle_pos+X_POS+15]-2>doggo_height) {  // position of doggo snoot w/ respect to left side of obstacle
        collision = 1;
    }
    if (obstacle_pos < 30 & bush_mask[8-obstacle_pos+X_POS+3]>doggo_height) {  // position of doggo tail w/ respect to left side of obstacle
        collision = 1;
    }
    return collision;
}

void print_score(char reset) {
    static int score = 0;
    unsigned char score_string[20];
    
    if (reset) {
        score = 0;
    }
    else {
        if (score<1000) {
            sprintf(score_string,"Score: %d",score);
            ssd1306_drawString(score_string,0,15);
            score++;
        }
        else if (score<10000) {
            sprintf(score_string,"Score: %d",score);
            ssd1306_drawString(score_string,0,14);
            score++;
        }
        else {
            sprintf(score_string,"Score: %d",score);
            ssd1306_drawString(score_string,0,13);
            score++;
        }
    }
}