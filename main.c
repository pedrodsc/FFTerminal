#include "fft.h"
#include <stdlib.h>
#include <inttypes.h>
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
#define PERCENT_FRAME_X     0.9

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
    short *buffer;
    short buffer_frames = N_SAMPLES*2;
    unsigned int rate = 44100;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    
       
    // Alsa inicialization
    
    if ((err = snd_pcm_open (&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        printf("cannot open audio device %s (%s)\n", 
                 argv[1],
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "audio interface opened\n");
    
    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        printf("cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params allocated\n");
    
    if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        printf("cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params initialized\n");
    
    if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("cannot set access type (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params access setted\n");
    
    if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
        printf("cannot set sample format (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params format setted\n");
    
    if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
        printf("cannot set sample rate (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params rate setted\n");
    
    if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) {
        printf("cannot set channel count (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params channels setted\n");
    
    if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        printf("cannot set parameters (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    fprintf(stdout, "hw_params setted\n");
    
    snd_pcm_hw_params_free (hw_params);
    
    fprintf(stdout, "hw_params freed\n");
    
    if ((err = snd_pcm_prepare (capture_handle)) < 0) {
        printf("cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        exit (1);
    }
    
    printf("audio interface prepared\n");
    buffer = malloc(buffer_frames * snd_pcm_format_width(format) / 8 * 2);
    printf("frame size %u\n",snd_pcm_format_width(format));
    printf("buffer allocated\n");
    
    // Start curses
    initscr();
    cbreak();
    noecho();
    
    keypad(stdscr, TRUE);
    
    height = LINES*PERCENT_FRAME_Y;
    width = COLS*PERCENT_FRAME_X;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    
    printw("The program exits when the count reaches 0");
    
    refresh();
    
    fft_frame = create_newwin(height, width, starty, startx);
    
    int count_down = 1000;
    if (argc == 3)
        count_down = strtoimax(argv[2],NULL,10);
    while(count_down--)
    {
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            printf("read from audio interface failed (%s)\n",
                     err, snd_strerror (err));
            exit (1);
        }
        for (int n = 0; n < N_SAMPLES; n++){
            fft_output[n] = buffer[n*2] + LEVEL_ZERO;
        }
        
        draw_spectrum(fft_frame, fft_output, N_SAMPLES);
        mvprintw(3,0,"Count down: %d",count_down);
        refresh();
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
    int height, width, spaces, group_size, y;

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
        y = height - 2 - temp*norm_factor;
        mvwaddch(local_win,y,k+1,'*');
//         for(y = height-2; y > height - 2 - temp*norm_factor; y--)
//                 mvwaddch(local_win,y,k+1,'U');
        
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
