#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define FLAG '?'
#define CLOSED_CELL '~'
#define OPENED_CELL ' '

typedef struct {
    int has_mine;
    int opened;
    int flagged;
    int count; 
} Cell;

typedef struct {
    int width, height, mines;
    Cell **field;
    int cursor_x, cursor_y;
    int remaining;
    int first_open; 
} Game;

Game game;

////////////////////// UTILS

void end_game_msg(const char *msg){
    clear();
    mvprintw(LINES/2, (COLS-strlen(msg))/2, "%s", msg);
    mvprintw(LINES/2+2, (COLS-25)/2, "Press any key to exit...");
    refresh();
    getch();
    endwin();
    exit(0);
}

////////////////////// GENERATE FIELD

void generate_field(int width, int height, int mines){
    game.width = width;
    game.height = height;
    game.mines = mines;
    game.cursor_x = 0;
    game.cursor_y = 0;
    game.first_open = 1;

    game.field = malloc(sizeof(Cell*) * height);
    for(int i=0;i<height;i++){
        game.field[i] = malloc(sizeof(Cell) * width);
        for(int j=0;j<width;j++){
            game.field[i][j].has_mine = 0;
            game.field[i][j].opened = 0;
            game.field[i][j].flagged = 0;
            game.field[i][j].count = 0;
        }
    }

    game.remaining = width*height - mines;
}

void place_mines(int safe_x, int safe_y){
    int placed = 0;
    while(placed < game.mines){
        int x = rand() % game.width;
        int y = rand() % game.height;

        if(abs(x-safe_x)<=1 && abs(y-safe_y)<=1) continue;

        if(!game.field[y][x].has_mine){
            game.field[y][x].has_mine = 1;
            placed++;
        }
    }

    for(int y=0;y<game.height;y++){
        for(int x=0;x<game.width;x++){
            if(game.field[y][x].has_mine) continue;
            int cnt = 0;
            for(int dy=-1;dy<=1;dy++){
                for(int dx=-1;dx<=1;dx++){
                    int ny=y+dy, nx=x+dx;
                    if(nx>=0 && nx<game.width && ny>=0 && ny<game.height){
                        if(game.field[ny][nx].has_mine) cnt++;
                    }
                }
            }
            game.field[y][x].count = cnt;
        }
    }
}

////////////////////// DRAW FIELD

void draw_field(){
    clear();
    int start_y = (LINES - game.height) / 2;
    int start_x = (COLS - game.width*2) / 2;

    for(int y=0;y<game.height;y++){
        for(int x=0;x<game.width;x++){
            move(start_y+y, start_x + x*2);
            if(game.cursor_x==x && game.cursor_y==y) attron(A_REVERSE);

            if(game.field[y][x].opened){
                if(game.field[y][x].has_mine){
                    addch('*');
                } else if(game.field[y][x].count>0){
                    addch('0'+game.field[y][x].count);
                } else {
                    addch(OPENED_CELL);
                }
            } else if(game.field[y][x].flagged){
                addch(FLAG);
            } else {
                addch(CLOSED_CELL);
            }
            addch(' ');

            if(game.cursor_x==x && game.cursor_y==y) attroff(A_REVERSE);
        }
    }
    mvprintw(start_y+game.height+1, (COLS-60)/2, "WASD/HJKL - move | Space - open | F - flag | Q - quit");
    refresh();
}

////////////////////// GAME LOGIC

void open_cell(int y, int x){
    if(y<0 || y>=game.height || x<0 || x>=game.width) return;
    Cell *c = &game.field[y][x];
    if(c->opened || c->flagged) return;

    if(game.first_open){
        place_mines(x,y);
        game.first_open = 0;
    }

    c->opened = 1;
    game.remaining--;

    if(c->has_mine){
        draw_field();
        end_game_msg("You hit a mine! Game Over.");
    }

    if(c->count == 0){
        for(int dy=-1;dy<=1;dy++){
            for(int dx=-1;dx<=1;dx++){
                if(dy!=0 || dx!=0)
                    open_cell(y+dy, x+dx);
            }
        }
    }

    if(game.remaining == 0){
        draw_field();
        end_game_msg("Congratulations! You won!");
    }
}

void toggle_flag(int y,int x){
    Cell *c = &game.field[y][x];
    if(c->opened) return;
    c->flagged = !c->flagged;
}

////////////////////// INPUT

void handle_input(int ch){
    switch(ch){
        case 'w': case 'k': if(game.cursor_y>0) game.cursor_y--; break;
        case 's': case 'j': if(game.cursor_y<game.height-1) game.cursor_y++; break;
        case 'a': case 'h': if(game.cursor_x>0) game.cursor_x--; break;
        case 'd': case 'l': if(game.cursor_x<game.width-1) game.cursor_x++; break;
        case ' ': open_cell(game.cursor_y, game.cursor_x); break;
        case 'f': case 'F': toggle_flag(game.cursor_y, game.cursor_x); break;
        case 'q': case 'Q': end_game_msg("Quit"); break;
    }
}

////////////////////// MAIN MENU

int main(){
    initscr();
    keypad(stdscr, TRUE);
    curs_set(0);
    noecho();
    srand(time(NULL));

    int mode_selected = 0;
    while(1){
        clear();
        const char *title = "Miner - select difficulty:";
        mvprintw(LINES/2-4, (COLS-strlen(title))/2, "%s", title);

        mvprintw(LINES/2-2, (COLS-10)/2, "%s Easy", mode_selected==0 ? ">" : " ");
        mvprintw(LINES/2-1, (COLS-10)/2, "%s Medium", mode_selected==1 ? ">" : " ");
        mvprintw(LINES/2,   (COLS-10)/2, "%s Hard", mode_selected==2 ? ">" : " ");

        mvprintw(LINES/2+2, (COLS-30)/2, "Use W/S to move, Enter to select");
        refresh();

        int ch = getch();
        if(ch=='w' && mode_selected>0) mode_selected--;
        if(ch=='s' && mode_selected<2) mode_selected++;
        if(ch=='\n'){
            if(mode_selected==0) generate_field(5+3,5+3,5+3); // easy
            if(mode_selected==1) generate_field(7+3,7+3,10+3); // medium
            if(mode_selected==2) generate_field(10+3,10+3,15+3); // hard
            break;
        }
    }

    while(1){
        draw_field();
        int ch = getch();
        handle_input(ch);
    }

    endwin();
    return 0;
}
