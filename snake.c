#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#define SCR_WIDTH 51
#define SCR_HEIGHT 21

#define UP 'w'
#define DOWN 's'
#define LEFT 'a'
#define RIGHT 'd'

typedef struct {
    int x, y;
} pos;

typedef struct {
    int score, moves, tail_len;
    char dir;
    pos head, *tail;
} snake;

void draw(void) {
    int scr_space_x = SCR_WIDTH+2, scr_space_y = SCR_HEIGHT+2;
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    
    for(int i = 0; i < scr_space_y; i++) {
        for(int j = 0; j < scr_space_x; j++) {
            attron(COLOR_PAIR(1));
            if(i == 0 || i == scr_space_y-1) mvprintw(i, j, "#");
            else if(j == 0 || j == scr_space_x-1) mvprintw(i, j, "#");
        }
    }
}

void print_stats(snake *player) {
    mvprintw(SCR_HEIGHT+3, 0, "Score: %d\nMove counter: %d\n"
    "Player pos: x=%d, y=%d", player->score, player->moves, player->head.x - (SCR_WIDTH/2+1), SCR_HEIGHT/2+1 - player->head.y);
}

void game_over_msg(snake *player) {
    char ans;
    nodelay(stdscr, FALSE);

    do {
        clear();
        printw("GAME OVER!! Do you wish to see your game stats? (y/n)\n");
        ans = getch();
        clear();
        if(ans == 'y') {
            float rating = ((float)player->score/player->moves) * (100);
            printw("Score: %d\nNumber of moves: %d\n"
            "Rating: %.2f\n\n", player->score, player->moves, rating);
        }
    } while(ans != 'y' && ans != 'n');
    
    printw("Goodbye and better luck next time!!\n(press any key to exit)");
    getch();
    return;
}

int difficulty_choice(void) {
    int choice = 0, confirm = 0, speed = 0;

    do {
        clear();
        printw("Choose difficulty:\n");
        printw("E - Easy\nM - Medium\nH - Hard"
        "\nI - Insane\nS - Increase as you score\n");

        choice = getch();
        switch(choice) {
            case 'e': 
                speed = 200;
                break;
            case 'm': 
                speed = 150;
                break;
            case 'h': 
                speed = 80;
                break;
            case 'i': 
                speed = 38;
                break;
            case 's':
                speed = -1;
                break;
            default: continue;
        }

        clear();
        printw("Confirm choice %c? (y/n)", choice-32);
        confirm = getch();
    } while(confirm != 'y');

    return speed;
}

snake *new_snake(void) {
    snake *ret = (snake*)malloc(sizeof(snake));
    ret->tail = (pos*)malloc(sizeof(pos));

    ret->moves = 0;
    ret->score = 0;
    ret->tail_len = 0;
    ret->head.x = SCR_WIDTH/2+1;
    ret->head.y = SCR_HEIGHT/2+1;

    return ret;
}

int update_snake(snake *player) {
    if(player->tail_len == 0) {
        player->tail[0] = player->head;
        player->tail_len = 1;
        return 1;
    }

    player->tail = (pos*)realloc(player->tail, (player->tail_len+1) * sizeof(pos));
    if(player->tail == NULL) return 0;

    player->tail[player->tail_len] = player->tail[player->tail_len-1];
    player->tail_len++;

    return 1;
}

void print_snake(snake *player) {
    clear();
    draw();

    init_pair(2, COLOR_GREEN, COLOR_BLACK);
	attron(COLOR_PAIR(2));
    mvaddch(player->head.y, player->head.x, 'O');

    for(int i = 0; i < player->tail_len; i++) {
        mvaddch(player->tail[i].y, player->tail[i].x, '+');
    }

    attroff(COLOR_PAIR(2));
    refresh();
}

int tail_collision(snake *player) {
    for(int i = 0; i < player->tail_len; i++) {
        if(player->head.x == player->tail[i].x && player->head.y == player->tail[i].y) {
            return 1;
        }
    }

    return 0;
}

int head_collision(snake *player) {
    if(player->head.x == SCR_WIDTH+1) return 1;
    if(player->head.x == 0) return 1;
    if(player->head.y == 0) return 1;
    if(player->head.y == SCR_HEIGHT+1) return 1;
    
    return 0;
}

int move_snake(snake *player) {
    pos temp;
    if(player->tail_len > 0) {
        temp = player->tail[0];
        player->tail[0] = player->head;
    }

    switch(player->dir) {
        case UP:
            player->head.y--;
            break;
        case DOWN:
            player->head.y++;
            break;
        case RIGHT:
            player->head.x++;
            break;
        case LEFT:
            player->head.x--;
            break;
    }

    if(player->tail_len > 1) {
        for(int i = player->tail_len-1; i > 1; i--) {
            player->tail[i] = player->tail[i-1];
        }
        player->tail[1] = temp;
    }

    print_snake(player);
    return !(tail_collision(player) || head_collision(player));
}

void free_snake(snake *player) {
    if(player->tail != NULL) free(player->tail);
    free(player);
}

int coincides_w_player(snake *player, pos food) {
    if(player->head.x == food.x && player->head.y == food.y) {
        return 1;
    }
    
    for(int i = 0; i < player->tail_len; i++) {
        if(player->tail[i].x == food.x && player->tail[i].y == food.y) {
            return 1;
        }
    }

    return 0;
}

void print_food(pos food) {
    init_pair(3, COLOR_RED, COLOR_RED);
	attron(COLOR_PAIR(3));
    mvaddch(food.y, food.x, '$');
    attroff(COLOR_PAIR(3));
}

pos spawn_food(snake *player) {
    pos food;
    srand(time(NULL));

    do {
        food.x = rand()%SCR_WIDTH+1;
        food.y = rand()%SCR_HEIGHT+1;
    } while(coincides_w_player(player, food));

    print_food(food);
    return food;
}

int food_eaten(snake *player, pos food) {
    if(food.x == player->head.x && food.y == player->head.y) {
        return 1;
    }

    return 0;
}

int get_direction(void) {
    return getch();
}

void game(void) {
    int can_move = 0, eaten = 0;
    int game_mode = difficulty_choice();
    float delay, speed;
    char prev_dir = 0;
    snake *player = new_snake();
    pos food_pos;

    //make getch a non-blocking call to get player directions
    //and initialize player and food position
    nodelay(stdscr, TRUE);
    food_pos = spawn_food(player);

    if(game_mode != -1) {
        delay = game_mode;
        speed = delay;
    } else {
        speed = 250;
        delay = speed;
    }

    while(1) {
        can_move = move_snake(player);
        print_food(food_pos);
        print_stats(player);

        if(!can_move) {
            refresh();
            napms(500);
            game_over_msg(player);
            break;
        }

        eaten = food_eaten(player, food_pos);
        if(eaten) {
            player->score++;

            if(game_mode == -1 && speed > 10) {
                if(speed > 200) speed -= 12;
                else if(speed > 150) speed -= 10;
                else if(speed > 100) speed -= 8;
                else if(speed > 75) speed -= 6;
                else if(speed > 60) speed -= 5;
                else if(speed > 40) speed -= 3;
                else speed -= 2;
            }

            update_snake(player);
            print_snake(player);
            print_stats(player);
            food_pos = spawn_food(player);
        }
        
        //shitty way to get movement direction
        //can't find a better way to do it :(
        switch(get_direction()) {
            case UP:
                if(player->tail_len == 0 || player->dir != DOWN) player->dir = UP;
                if(speed != -1) delay = speed * 1.35;
                break;
            case DOWN:
                if(player->tail_len == 0 || player->dir != UP) player->dir = DOWN;
                delay = speed * 1.35;
                break;
            case LEFT:
                if(player->tail_len == 0 || player->dir != RIGHT) player->dir = LEFT;
                delay = speed;
                break;
            case RIGHT:
                if(player->tail_len == 0 || player->dir != LEFT) player->dir = RIGHT;
                delay = speed;
                break;
            default: 
                prev_dir = player->dir;
        }
        if(prev_dir != player->dir) player->moves++;

        refresh();
        napms(delay);
    }

    free_snake(player);
}

int main() {
    //initialize ncurses window
    initscr();

    //initialize color 
    start_color();

    //initialize game
    game();

    //end ncurses window
    endwin();
}
