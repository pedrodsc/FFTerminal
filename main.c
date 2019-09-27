#include <stdlib.h>
#include <ncurses.h>

/*
        COLOR_BLACK   0
        COLOR_RED     1
        COLOR_GREEN   2
        COLOR_YELLOW  3
        COLOR_BLUE    4
        COLOR_MAGENTA 5
        COLOR_CYAN    6
        COLOR_WHITE   7
*/


int main(int argc, char **argv){
    // Init curses
    initscr();
    if(has_colors() == FALSE){
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    
    printf("the size of the terminal is %d lines \t %d cols\n",LINES,COLS);
    
    for(int i = 0; i < LINES/2; i++)
        for(int j = 0; j < COLS/2; j++){
            if (j & 2)
                attron(COLOR_PAIR(1));
            else
                attroff(COLOR_PAIR(1));
            mvaddstr(i,j,"╔");
        }
    refresh();
    getch();
    endwin();
}
