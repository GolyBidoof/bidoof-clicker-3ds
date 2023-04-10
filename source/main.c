#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include <citro2d.h>
#include <time.h>
#include <assert.h>

C2D_TextBuf g_staticBuf, g_dynamicBuf;
C2D_Text g_staticText[4];

// Simple sprite struct
#define MAX_SPRITES   768
#define FRAMERATE 	60

typedef struct
{
	C2D_Sprite spr;
} Sprite;

static C2D_SpriteSheet spriteSheet;
static C2D_SpriteSheet bidoofSprites;

static Sprite sprites[MAX_SPRITES];

float milisecondsSinceInit = 0;

typedef struct {
	float eggCount;
	float currentAutoEggBoost;
	float currentClickEggBoost;
	int amntOfKeysToBoostHeld;
} GameState;

typedef struct {
	int touchX;
	int touchY;
} TouchScreenPositions;

typedef struct {
	int animationFrame;
	int framerate;
	float currentLoopFrame;
} BidoofSpriteParams;

static float backgroundPosition = 0;

static GameState state = {0, 0.01, 1, 0};
static TouchScreenPositions touchScreenPositions = {0, 0};
static BidoofSpriteParams bidoofSpriteParams = {0, FRAMERATE/2, 0};

static void sceneInit(void)
{
	g_staticBuf  = C2D_TextBufNew(4096);
	g_dynamicBuf = C2D_TextBufNew(4096);

	// Parse the static text strings
	C2D_TextParse(&g_staticText[0], g_staticBuf, "Wait, what're you doing on a Nintendo 3DS???");

	// Optimize the static text strings
	C2D_TextOptimize(&g_staticText[0]);
}

void printTime(void) {
	time_t unixTime = time(NULL);
	struct tm* timeStruct = gmtime((const time_t *)&unixTime);

	int hours = timeStruct->tm_hour;
	int minutes = timeStruct->tm_min;
	int seconds = timeStruct->tm_sec;

	// Draw time
	char buf[20];
	C2D_Text timeCounter;
	snprintf(buf, sizeof(buf), " %02i:%02i:%02i", hours, minutes, seconds);
	C2D_TextParse(&timeCounter, g_dynamicBuf, buf);
	C2D_TextOptimize(&timeCounter);
	C2D_DrawText(&timeCounter, C2D_AlignLeft, 8.0f, 8.0f, 0.0f, 0.8f, 0.8f);
}

static void printEggTimer() {
	char buf[30];
	C2D_Text eggCounter;
	int eggCountInt = (int) state.eggCount;
	snprintf(buf, sizeof(buf), "%d", eggCountInt);
	C2D_TextParse(&eggCounter, g_dynamicBuf, buf);
	C2D_TextOptimize(&eggCounter);

	//Get amount of digits on an egg counter
	int eggCountDigits = 0;
	if (eggCountInt == 0) eggCountDigits = 1;
	while(eggCountInt!=0)  
   	{  
       eggCountInt/=10;  
       eggCountDigits++;  
   	}  

	C2D_DrawText(&eggCounter, C2D_AlignLeft, 366.0f-eggCountDigits*16.0f*0.8f, 8.0f, 0, 0.8f, 0.8f);
}

static void printNotifications() {
	C2D_DrawRectSolid(0, 220, 0, 400, 20, C2D_Color32(0x00, 0x00, 0x00, 0x20));
	C2D_DrawText(&g_staticText[0], 0, 8.0f, 223, 0.5f, 0.6f, 0.6f);
}

static void animateBidoofSprite() {
	Sprite* bidoofSprite = &sprites[7+bidoofSpriteParams.animationFrame];
	C2D_DrawSprite(&bidoofSprite->spr);
}

typedef struct {
	int maxSwing;
	int eggRotationDirection;
	float eggRotationFrame;
	float eggRotationSpeed;
} TouchScreenEggParams;

static TouchScreenEggParams touchScreenEggParams = {15, 1, 0, 0.05f};

static void sceneRenderBottom() {
	Sprite* bigEgg = &sprites[6];
	C2D_DrawSprite(&bigEgg->spr);
	C2D_SpriteScale(&bigEgg->spr, 1, 1);

	/*if (touchScreenPositions.touchX || touchScreenPositions.touchY) {
		C2D_SpriteScale(&bigEgg->spr, 3.6f, 3.6f);
	} else {
		C2D_SpriteScale(&bigEgg->spr, 4, 4);
	}*/

	C2D_SpriteRotateDegrees(&bigEgg->spr, touchScreenEggParams.eggRotationSpeed*touchScreenEggParams.eggRotationDirection);
}

static void sceneRenderTop()
{
	// Clear the dynamic text buffer
	C2D_TextBufClear(g_dynamicBuf);

	// Draw background
	C2D_DrawRectSolid(0, 1, 0, 400, 32, C2D_Color32(0xD2, 0xB4, 0x8C, 0xFF));

	Sprite* backgroundImageTile1 = &sprites[3];
	Sprite* backgroundImageTile2 = &sprites[4];
	Sprite* backgroundImageTile3 = &sprites[5];
	C2D_SpriteSetPos(&backgroundImageTile1->spr, (int) -backgroundPosition, 40);
	C2D_SpriteSetPos(&backgroundImageTile2->spr, (int) -backgroundPosition+256, 40);
	C2D_SpriteSetPos(&backgroundImageTile3->spr, (int) -backgroundPosition+512, 40);
	C2D_DrawSprite(&backgroundImageTile1->spr);
	C2D_DrawSprite(&backgroundImageTile2->spr);
	C2D_DrawSprite(&backgroundImageTile3->spr);

	// Draw egg
	Sprite* egg = &sprites[0];
	C2D_SpriteRotateDegrees(&egg->spr, 1.0f);
	C2D_DrawSprite(&egg->spr);

	// Draw background bars
	Sprite* backgroundBarTop = &sprites[1];
	Sprite* backgroundBarBottom = &sprites[2];
	C2D_DrawSprite(&backgroundBarTop->spr);
	C2D_DrawSprite(&backgroundBarBottom->spr);
	
	animateBidoofSprite();

	printEggTimer();
	printTime();
	printNotifications();
}

static void sceneExit(void)
{
	// Delete the text buffers
	C2D_TextBufDelete(g_dynamicBuf);
	C2D_TextBufDelete(g_staticBuf);
}

static void initBidoofAnimationSprites() {
	int i=0;

	for (i=0; i<8; i++) {
		Sprite* bidoof_cell = &sprites[7+i];
		C2D_SpriteFromSheet(&bidoof_cell->spr, bidoofSprites, i);
		C2D_SpriteSetCenter(&bidoof_cell->spr, 0.5f, 1);
		C2D_SpriteSetPos(&bidoof_cell->spr, 200, 230);
		C2D_SpriteScale(&bidoof_cell->spr, 3, 3);
	}
}

static void initSprites() {
	// Egg
	Sprite* egg = &sprites[0];

	C2D_SpriteFromSheet(&egg->spr, spriteSheet, 0);
	C2D_SpriteSetCenter(&egg->spr, 0.5f, 0.5f);
	C2D_SpriteSetPos(&egg->spr, 384, 19);
	C2D_SpriteScale(&egg->spr, 0.8f, 0.8f);

	//Background bar
	Sprite* backgroundBarTop = &sprites[1];
	C2D_SpriteFromSheet(&backgroundBarTop->spr, spriteSheet, 1);
	C2D_SpriteSetCenter(&backgroundBarTop->spr, 0, 0);
	C2D_SpriteSetPos(&backgroundBarTop->spr, 0, 0);

	Sprite* backgroundBarBottom = &sprites[2];
	C2D_SpriteFromSheet(&backgroundBarBottom->spr, spriteSheet, 1);
	C2D_SpriteSetCenter(&backgroundBarBottom->spr, 0, 0);
	C2D_SpriteSetPos(&backgroundBarBottom->spr, 0, 36);

	Sprite* backgroundImageTile1 = &sprites[3];
	C2D_SpriteFromSheet(&backgroundImageTile1->spr, spriteSheet, 2);
	C2D_SpriteSetCenter(&backgroundImageTile1->spr, 0, 0);
	C2D_SpriteSetPos(&backgroundImageTile1->spr, 0, 40);

	Sprite* backgroundImageTile2 = &sprites[4];
	C2D_SpriteFromSheet(&backgroundImageTile2->spr, spriteSheet, 2);
	C2D_SpriteSetCenter(&backgroundImageTile2->spr, 0, 0);
	C2D_SpriteSetPos(&backgroundImageTile2->spr, 256, 40);

	Sprite* backgroundImageTile3 = &sprites[5];
	C2D_SpriteFromSheet(&backgroundImageTile3->spr, spriteSheet, 2);
	C2D_SpriteSetCenter(&backgroundImageTile3->spr, 0, 0);
	C2D_SpriteSetPos(&backgroundImageTile3->spr, 512, 40);

	Sprite* bigEgg = &sprites[6];
	C2D_SpriteFromSheet(&bigEgg->spr, spriteSheet, 0);
	C2D_SpriteSetCenter(&bigEgg->spr, 0.5, 0.5);
	C2D_SpriteSetPos(&bigEgg->spr, 320/2, 240/2);
	C2D_SpriteScale(&bigEgg->spr, 4, 4);

	initBidoofAnimationSprites();
}


static void addEggs(float frameCount) {
	state.eggCount += state.currentAutoEggBoost*frameCount;
	state.eggCount += state.currentClickEggBoost*state.amntOfKeysToBoostHeld;
	state.amntOfKeysToBoostHeld = 0;
}

static void updateBidoofAnimation(float frameCount) {
	bidoofSpriteParams.currentLoopFrame += frameCount;
	if (bidoofSpriteParams.currentLoopFrame>FRAMERATE/bidoofSpriteParams.framerate) {
		bidoofSpriteParams.currentLoopFrame = 0;
		bidoofSpriteParams.animationFrame = (bidoofSpriteParams.animationFrame + 1) % 8;
	}

	touchScreenEggParams.eggRotationFrame += frameCount;
	if (touchScreenEggParams.eggRotationFrame > touchScreenEggParams.maxSwing) {
		touchScreenEggParams.eggRotationFrame -= touchScreenEggParams.maxSwing;
		touchScreenEggParams.eggRotationDirection = -touchScreenEggParams.eggRotationDirection;
	}
}

static void updateDeltaPhysics() {
	float currentTime = svcGetSystemTick() / CPU_TICKS_PER_MSEC;
	float deltaDiff = currentTime - milisecondsSinceInit;
	float frameCount = deltaDiff/FRAMERATE;
	backgroundPosition = backgroundPosition + frameCount;
	if (backgroundPosition > 256) {
		backgroundPosition -= 256;
	}
	addEggs(frameCount);
	updateBidoofAnimation(frameCount);
	milisecondsSinceInit = currentTime;
}

int main()
{
	// Initialize the libs
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	// Create screen
	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Load graphics
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spriteSheet) svcBreak(USERBREAK_PANIC);
	bidoofSprites = C2D_SpriteSheetLoad("romfs:/gfx/bidoof_spritesheet.t3x");
	if (!bidoofSprites) svcBreak(USERBREAK_PANIC);

	// Initialize sprites
	initSprites();

	// Initialize the scene
	sceneInit();

	milisecondsSinceInit = svcGetSystemTick() / CPU_TICKS_PER_MSEC;

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();

		// Respond to user input
		u32 kDown = hidKeysDown();
		//u32 kHeld = hidKeysHeld();

		if (kDown & KEY_A) {
			state.amntOfKeysToBoostHeld++;
		}
		if (kDown & KEY_B) {
			state.amntOfKeysToBoostHeld++;
		}
		if (kDown & KEY_L) {
			state.amntOfKeysToBoostHeld++;
		}
		if (kDown & KEY_R) {
			state.amntOfKeysToBoostHeld++;
		}
		if (kDown & KEY_X) {
			state.amntOfKeysToBoostHeld++;
		}
		if (kDown & KEY_Y) {
			state.amntOfKeysToBoostHeld++;
		}

		touchPosition touch;
		hidTouchRead(&touch);

		Sprite* bigEgg = &sprites[6];

		if (!touchScreenPositions.touchX && !touchScreenPositions.touchY && (touch.px || touch.py)) {
			C2D_SpriteScale(&bigEgg->spr, 0.8f, 0.8f);
			state.amntOfKeysToBoostHeld++;
			touchScreenPositions.touchX = touch.px;
			touchScreenPositions.touchY = touch.py;
		} else if ((touchScreenPositions.touchX || touchScreenPositions.touchY) && !touch.px && !touch.py) {
			C2D_SpriteScale(&bigEgg->spr, 1.25f, 1.25f);
			touchScreenPositions.touchX = 0;
			touchScreenPositions.touchY = 0;
		}


		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		updateDeltaPhysics();

		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(bottom, C2D_Color32(0xB9, 0x99, 0x76, 0xFF));
		C2D_SceneBegin(bottom);
		sceneRenderBottom();
		C2D_TargetClear(top, C2D_Color32(0xB9, 0x99, 0x76, 0xFF));
		C2D_SceneBegin(top);
		sceneRenderTop();
		C3D_FrameEnd(0);
	}

	// Deinitialize the scene
	sceneExit();

	// Delete graphics
	C2D_SpriteSheetFree(spriteSheet);

	// Deinitialize the libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}
