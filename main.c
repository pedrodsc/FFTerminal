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


#define NORM_FACTOR     1024
#define PERCENT_FRAME_Y 0.5
#define PERCENT_FRAME_X 0.85

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int draw_spectrum(WINDOW *local_win, int *fft_vector, int fft_size);

int main(int argc, char *argv[])
{
    WINDOW *fft_frame;
    
    int startx, starty, width, height;
    
    int fft_output[] = {100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234,
                        100,200,580,400,1000,150,670,234
    };
    
    // Start curses
    initscr();
    cbreak();
    noecho();
    
    keypad(stdscr, TRUE);
    
    height = LINES*PERCENT_FRAME_Y;
    width = COLS*PERCENT_FRAME_X;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    
    printw("Press any key to exit");
    
    refresh();
    
    fft_frame = create_newwin(height, width, starty, startx);
    
    draw_spectrum(fft_frame, fft_output, sizeof(fft_output)/sizeof(fft_output[0]));
    
    getch();    
    endwin();
    return 0;
}

int draw_spectrum(WINDOW *local_win, int *fft_vector, int fft_size){
    int height, width, spaces, group_size;
    int temp;
    float norm_factor;
    
    getmaxyx(local_win, height, width);
    spaces = width - 2;
    
    // Unable to print (at least for now)
    if (spaces > fft_size){
        mvwprintw(local_win,1,1,"Input vector too small.\n");
        wrefresh(local_win);
        return -1;
    }
    group_size = fft_size / (spaces + 1);
    
    norm_factor = (float) height / NORM_FACTOR;
    
    //wattrset(local_win,A_REVERSE);
    for (int k = 0; k < spaces; k++){
        temp = 0;
        for(int g = 0; g < group_size; g++)
            temp = temp + fft_vector[k*group_size + g];
        
        temp = temp / group_size;
        
        for(int y = height-2; y > temp*norm_factor; y--)
                mvwaddch(local_win,y,k+1,'U');
    }

    mvwprintw(local_win,height-1,0,"H:%d W:%d",height,width);
    wrefresh(local_win);
    
    return 0;    
}

WINDOW *create_newwin(int height, int width, int starty, int startx){
    WINDOW *local_win;
    
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0);
    
    wrefresh(local_win);
    
    return local_win;
}

void destroy_win(WINDOW *local_win){
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    /* The parameters taken are 
     * 1. win: the window on which to operate
     * 2. ls: character to be used for the left side of the window 
     * 3. rs: character to be used for the right side of the window 
     * 4. ts: character to be used for the top side of the window 
     * 5. bs: character to be used for the bottom side of the window 
     * 6. tl: character to be used for the top left corner of the window 
     * 7. tr: character to be used for the top right corner of the window 
     * 8. bl: character to be used for the bottom left corner of the window 
     * 9. br: character to be used for the bottom right corner of the window
     */
    wrefresh(local_win);
    delwin(local_win);
}
