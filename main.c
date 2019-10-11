#include "fft.h"
#include <stdlib.h>
#include <inttypes.h>
#include <ncurses.h>
#include <math.h>
#include <complex.h>
#include <alsa/asoundlib.h>
#include <time.h>

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

#define N_SAMPLES           1024
#define NORM_FACTOR         16384
#define LEVEL_ZERO          NORM_FACTOR/2
#define PERCENT_FRAME_Y     0.9
#define PERCENT_FRAME_X     0.94
#define NO_KEY              -1

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int draw_spectrum(WINDOW *local_win, unsigned int *fft_vector, int fft_size);
int draw_wave(WINDOW *local_win, int *samples, int size);

int main(int argc, char *argv[])
{
    int ch;
    int x = 0;
    int avrg = 10;
    
    clock_t t;
    double elapsed_time;
    
    // fft_buffer
    int samples[N_SAMPLES];
    unsigned int fft_spectrum[N_SAMPLES];
    double complex fft_input[N_SAMPLES];
    double complex fft_output[N_SAMPLES];
    double complex *lookup;
    
    // FFT inicialization
    lookup = fft_create_lookup(N_SAMPLES);
    
    WINDOW *fft_frame; // Frame for the fft
    WINDOW *audio_frame; // Frame for the fft
    int startx, starty, width, height;
    
    // Alsa variables
    int err;
    char *capture_device = "default";
    short *buffer;
    short buffer_frames = N_SAMPLES*2;
    unsigned int rate = 44100;
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    
       
    // Alsa inicialization
    if (argc > 1)
        capture_device = argv[1];
    
    if ((err = snd_pcm_open (&capture_handle, capture_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
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
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    
    height = LINES*PERCENT_FRAME_Y/2;
    width = COLS*PERCENT_FRAME_X;
    //width = 128;
    starty = (LINES - height) / 4;
    startx = (COLS - width) / 2;
    
    printw("Press F1 to exit");
    
    refresh();
    
    fft_frame = create_newwin(height, width, starty-5, startx);
    audio_frame = create_newwin(height, width, starty*4, startx);
    

    while((ch = getch()) != KEY_F(1))
    {
        if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
            printf("read from audio interface failed (%d) - %s\n",
                     err, snd_strerror (err));
            exit (1);
        }
        // Get samples
        for (int n = 0; n < N_SAMPLES; n++){
            samples[n] = buffer[n*2];
        }
        
        for (int n = 0; n < N_SAMPLES; n++)
            fft_input[n] = (double complex) samples[n];
        
        t = clock();

        fft_compute(lookup, fft_input, fft_output, N_SAMPLES);
        fft_abs(fft_output, fft_spectrum, N_SAMPLES);
        
        t = clock() - t;
        elapsed_time = ((double) t) / CLOCKS_PER_SEC * 1000000.;
        
        // Normalize
        for (int n = 0; n < N_SAMPLES; n++)
            fft_spectrum[n] = 2*fft_spectrum[n]/(NORM_FACTOR/N_SAMPLES);
        
        draw_spectrum(fft_frame, fft_spectrum, N_SAMPLES/4);
        
        for (int i=0;i<N_SAMPLES;i++)
            samples[i] = LEVEL_ZERO + samples[i];
        draw_wave(audio_frame,samples,N_SAMPLES);
        
        mvprintw(0,20,"FFT Size: %d \t Sampling Frequency: %dHz \t Calc time: %.2lfms  ",N_SAMPLES,rate,elapsed_time);
        mvprintw(1,0,"%d",x);
        refresh();
        x++;
    }
    
    endwin();
    
    free(buffer);
    
    fprintf(stdout, "buffer freed\n");
    
    snd_pcm_close (capture_handle);
    fprintf(stdout, "audio interface closed\n");
    
    exit (0);
}

int draw_spectrum(WINDOW *local_win, unsigned int *fft_vector, int fft_size)
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
    
    norm_factor = (float) (height-1) / NORM_FACTOR;
    wclear(local_win);
    box(local_win, 0 , 0);
    
    for (int k = 0; k < spaces; k++)
    {
        temp = 0;
        for(int g = 0; g < group_size; g++)
            temp = temp + fft_vector[k*group_size + g];
        
        temp = temp / group_size;
        wattron(local_win,A_REVERSE);
        for(y = height - 2; y > height - 1 - temp*norm_factor; y--)
            mvwaddch(local_win,y,k+1,' ');
        wattroff(local_win,A_REVERSE);

    }
    
    mvwprintw(local_win,height-1,0,"H:%d W:%d",height,width);
    mvwaddch(local_win,height-1,width/4*1,'|');
    mvwaddch(local_win,height-1,width/4*2,'|');
    mvwaddch(local_win,height-1,width/4*3,'|');
    
    wrefresh(local_win);
    
    return 0;    
}

int draw_wave(WINDOW *local_win, int *samples, int size)
{
    int height, width, spaces, start_pos = 0, y;
    const int hysteresis = 10000, trig_w = 3;
    int low_side, high_side;
    float norm_factor, temp;
    
    getmaxyx(local_win, height, width);
    spaces = width - 2;
    
    // Unable to print (at least for now)
    if (spaces > size)
    {
        mvwprintw(local_win,1,1,"Input vector too small.\n");
        wrefresh(local_win);
        return -1;
    }
    
    // Trigger
//     for (int l = 50; l < size-50; l++){
//         low_side = (samples[l - trig_w] + samples[l-1- trig_w] + samples[l-2- trig_w] + samples[l-3- trig_w])/4;
//         high_side = (samples[l+trig_w] + samples[l+1+trig_w] + samples[l+2+trig_w] + samples[l+3+trig_w])/4;
//         if (low_side < LEVEL_ZERO && (high_side > LEVEL_ZERO + hysteresis))
//             start_pos = l;
//     }
        
    norm_factor = (float) height / NORM_FACTOR;
    wclear(local_win);
    box(local_win, 0 , 0);
    
    for (int k = 0; k < spaces; k++)
    {
        temp = samples[k + start_pos];
        y = height - temp*norm_factor;
        mvwaddch(local_win,y,k+1,'*');
    }
    
    mvwprintw(local_win,height-1,0,"H:%d W:%d",height,width);
    
    wrefresh(local_win);
    
    return 0;    
}


/*
 * int draw_wave(WINDOW *local_win, int *samples, int size)
 * {
 *    int height, width, spaces, group_size, y;
 *    
 *    float norm_factor, temp;
 *    
 *    getmaxyx(local_win, height, width);
 *    spaces = width - 2;
 *    
 *    // Unable to print (at least for now)
 *    if (spaces > size)
 *    {
 *        mvwprintw(local_win,1,1,"Input vector too small.\n");
 *        wrefresh(local_win);
 *        return -1;
 *    }
 *    
 *    group_size = size / (spaces + 1);
 *    
 *    norm_factor = (float) height / NORM_FACTOR;
 *    wclear(local_win);
 *    box(local_win, 0 , 0);
 *    
 *    for (int k = 0; k < spaces; k++)
 *    {
 *        temp = 0;
 *        for(int g = 0; g < group_size; g++)
 *            temp = temp + samples[k*group_size + g];
 *        
 *        temp = temp / group_size;
 *        
 *        y = height - temp*norm_factor;
 *        mvwaddch(local_win,y,k+1,'*');
 *    }
 *    
 *    mvwprintw(local_win,height-1,0,"H:%d W:%d",height,width);
 *    
 *    wrefresh(local_win);
 *    
 *    return 0;    
 * }
 */



WINDOW *create_newwin(int height, int width, int starty, int startx)
{
    WINDOW *local_win;
    
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0);
    
    wrefresh(local_win);
    
    return local_win;
}
