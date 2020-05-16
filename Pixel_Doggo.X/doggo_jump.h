#ifndef DOGGO_JUMP_H
#define	DOGGO_JUMP_H

void doggo_run(int frame);
void doggo_jump(int frame);
void doggo_wag(void);
void doggo_woof(void);
void ground_move(void);
void print_pbtp(void);
void obstacles(void);
void draw_obstacle(signed short pos);
char check_collision(signed short obstacle_pos);

extern char doggo_height;
extern char game_over;

#endif	/* DOGGO_JUMP_H */

