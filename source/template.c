#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#define PI_DIGITS 10000000

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

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
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

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
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");

	// Info.
	printf("Calculating PI to %d digits.\n", PI_DIGITS);
	printf("MAKE SURE YOUR WII IS IN A WELL-VENTALATED AREA.\n");

    int *r;
    int i, k;
    int b, d;
    int c = 0;
    
    // Calculate the size of the array needed based on PI_DIGITS
    int size = 2000 + PI_DIGITS + 1;
    
    // Allocate memory for r array dynamically
    r = (int *)malloc(size * sizeof(int));
    
    if (r == NULL) {
        printf("Memory allocation failed. Exiting...\n");
        return 1;
    }

    for (i = 0; i < size; i++) {
        r[i] = 2000;
    }
    r[i] = 0;

    for (k = 2000 + PI_DIGITS; k > 0; k -= 14) {
        d = 0;
        WPAD_ScanPads();
        s32 pressed = WPAD_ButtonsDown(WPAD_CHAN_0);

        i = k;
        for (;;) {
            d += r[i] * 10000;
            b = 2 * i - 1;

            r[i] = d % b;
            d /= b;
            i--;
            if (i == 0) break;
            d *= i;
        }

        if(pressed & WPAD_BUTTON_HOME) break;
        
        // Print the calculated digits as they are determined
        printf("%.4d", c + d / 10000);
        c = d % 10000;
    }

    // Free dynamically allocated memory
    free(r);

    return 0;
}
