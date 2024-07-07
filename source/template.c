#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>

#include "oggplayer.h"
#include <bgm_ogg.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#define PI_DIGITS 10000000
int calculated = 0;
int dpS = 0;
int AdpS = 0;

lwp_t infoThread;
lwp_t iETAThread;
lwp_t buttonThread;
static void* infoThreadFunc(void* arg) {
    while(true) {
        usleep(30 * (1000*1000));
        float percent_done = (float)calculated / PI_DIGITS * 100.0;
        printf("\nCalculated %d digits. AdpS: %d. %.2f%% done.\n\n", calculated, AdpS, percent_done);
    }
}
static void* iETAThreadFunc(void* arg) {
    while(true) {
        usleep(1 * (1000*1000));
        //printf("\nCalculated %d digits.\n", calculated);
        AdpS = dpS;
        dpS = 0;
    }
}
static void* buttonThreadFunc(void* arg) {
    while(true) {
        WPAD_ScanPads();
        s32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);
        if(pressed & WPAD_BUTTON_HOME) exit(0);
        usleep(1000);
    }
}

// Function to calculate Pi digits using the BBP algorithm
void calculate_pi(int *r, int pi_digits)
{
	int i, k;
	int b, d;
	int c = 0;
	int size = 2000 + pi_digits + 1;

	for (i = 0; i < size; i++)
	{
		r[i] = 2000;
	}

	for (k = 2000 + pi_digits; k > 0; k -= 14)
	{
		d = 0;
		i = k;
		for (;;)
		{
			d += r[i] * 10000;
			b = 2 * i - 1;
			r[i] = d % b;
			d /= b;
			i--;
			if (i == 0)
				break;
			d *= i;
		}
		printf("%.4d", c + d / 10000);
		c = d % 10000;
        calculated+=4;
        dpS+=4;
	}
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//---------------------------------------------------------------------------------

    // Init ASND.
    ASND_Init();

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

    // Play BGM.
    PlayOgg(bgm_ogg, bgm_ogg_size, 0, OGG_INFINITE_TIME);

	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf("\x1b[%d;%dH", row, column);
	printf("\x1b[2;0H");

	// Info.
	printf("Calculating PI to %d digits.\n", PI_DIGITS);
	printf("MAKE SURE YOUR WII IS IN A WELL-VENTILATED AREA.\n");

	int *r;
	int size = 2000 + PI_DIGITS + 1;

	// Allocate memory for r array dynamically
	r = (int *)malloc(size * sizeof(int));

	if (r == NULL)
	{
		printf("Memory allocation failed. Exiting...\n");
		return 1;
	}

    if (r == NULL) {
        printf("Memory allocation failed. Exiting...\n");
        return 1;
    }

    if (LWP_CreateThread(&infoThread, infoThreadFunc, NULL, NULL, 0, 80) != 0) {
        printf("Failed to create information thread.\n");
        return 1;
    }

    if (LWP_CreateThread(&iETAThread, iETAThreadFunc, NULL, NULL, 0, 80) != 0) {
        printf("Failed to create information ETA thread.\n");
        return 1;
    }

    if (LWP_CreateThread(&buttonThread, buttonThreadFunc, NULL, NULL, 0, 80) != 0) {
        printf("Failed to create button handler thread.\n");
        return 1;
    }

	calculate_pi(r, PI_DIGITS);

	// Free dynamically allocated memory
	free(r);

	return 0;
}