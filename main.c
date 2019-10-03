#include "fft.h"
#include <stdlib.h>
#include <ncurses.h>
#include <math.h>
#include <complex.h>
#include <alsa/asoundlib.h>

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

#define LOOKUP_TABLE_SIZE   2048
#define N_SAMPLES           128
#define NORM_FACTOR         65536
#define LEVEL_ZERO          NORM_FACTOR/2
#define PERCENT_FRAME_Y     0.7
#define PERCENT_FRAME_X     0.97

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int draw_spectrum(WINDOW *local_win, int *fft_vector, int fft_size);

int main(int argc, char *argv[])
{
    int ch;
    int x = 0;
    // fft_buffer
    int fft_output[N_SAMPLES];
    
    for (int i=0;i<N_SAMPLES;i++)
        fft_output[i] = LEVEL_ZERO + LEVEL_ZERO*sin((float)i/10);
    
    WINDOW *fft_frame; // Frame for the fft
    int startx, starty, width, height;
    
    // Alsa variables
    int err;
    char *buffer;
    int buffer_frames = 128;
    unsigned int rate = 44100;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    
       
    // Alsa inicialization
    
    if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n", 
                 argv[1],
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "audio interface opened\n");
    
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params allocated\n");
    
    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params initialized\n");
    
    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params access setted\n");
    
    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params format setted\n");
    
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params rate setted\n");
    
    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params channels setted\n");
    
    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params setted\n");
    
    snd_pcm_hw_params_free (hw_params);
    
    fprintf(stdout, "hw_params freed\n");
    
    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "audio interface prepared\n");
    buffer = malloc(128 * snd_pcm_format_width(format) / 8 * 2);
    fprintf(stdout, "frame size %u\n",snd_pcm_format_width(format));
    fprintf(stdout, "buffer allocated\n");
    
    // Start curses
    initscr();
    cbreak();
    noecho();
    
    keypad(stdscr, TRUE);
    
    height = LINES*PERCENT_FRAME_Y;
    width = COLS*PERCENT_FRAME_X;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    
    printw("Press F1 to exit");
    
    refresh();
    
    fft_frame = create_newwin(height, width, starty, startx);
    
    
    while((ch = getch()) != KEY_F(1))
    {
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            fprintf (stderr, "read from audio interface failed (%s)\n",
                     err, snd_strerror (err));
            exit (1);
        }
        for (int i = 0; i < buffer_frames; i++)
            fft_output[i+N_SAMPLES/2] = buffer[i] + LEVEL_ZERO;
        
        draw_spectrum(fft_frame, fft_output, N_SAMPLES);
        mvprintw(3,0,"%d",x++);
    }
    
    endwin();
    
    free(buffer);
    
    fprintf(stdout, "buffer freed\n");
    
    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");
    
    exit (0);
}

int draw_spectrum(WINDOW *local_win, int *fft_vector, int fft_size)
{
    int height, width, spaces, group_size;

    float norm_factor, temp;
    
    getmaxyx(local_win, height, width);
    spaces = width - 2;
    
    // Unable to print (at least for now)
    if (spaces > fft_size)
    {
        mvwprintw(local_win,1,1,"Input vector too small.\n");
        wrefresh(local_win);
        return -1;
    }

    group_size = fft_size / (spaces + 1);
    
    norm_factor = (float) height / NORM_FACTOR;
    wclear(local_win);
    //wattrset(local_win,A_REVERSE);
    for (int k = 0; k < spaces; k++)
    {
        temp = 0;
        for(int g = 0; g < group_size; g++)
            temp = temp + fft_vector[k*group_size + g];
        
        temp = temp / group_size;
        
        for(int y = height-2; y > height - 2 - temp*norm_factor; y--)
                mvwaddch(local_win,y,k+1,'U');
        
    }
    mvwprintw(local_win,height-1,0,"H:%d W:%d",height,width);
    wrefresh(local_win);
    
    
    return 0;    
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{
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
