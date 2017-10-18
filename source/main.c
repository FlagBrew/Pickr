#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include "pp2d/pp2d.h"

#define TEXTURE_LIVES_BIG 1
#define TEXTURE_LIVES_SMALL 2

static int _time, diff, stages, maxStages, lives, matches;
static bool pattern[16];
static u32 color[3];

void exitServices() {
	romfsExit();
	pp2d_exit();
}

void initVars() {
	lives = 5;
	_time = 320;
	diff = 10;
	stages = 0;	
}

void updatediff() {
	diff = 10;
}

void initServices() {
	srand(time(NULL));
	
	romfsInit();
	pp2d_init();
	pp2d_set_screen_color(GFX_TOP, ABGR8(255, 245, 245, 245));
	pp2d_set_screen_color(GFX_BOTTOM, ABGR8(255, 245, 245, 245));
	
	pp2d_load_texture_png(TEXTURE_LIVES_BIG, "romfs:/lives.png");
	pp2d_load_texture_png(TEXTURE_LIVES_SMALL, "romfs:/lives16p.png");

	initVars();
}

void fillPattern() {
	for (unsigned int i = 0; i < 16; i++)
		pattern[i] = false;
	
	pattern[rand() % 16] = true;
	
	color[0] = rand() % (255 - 2*diff) + diff;
	color[1] = rand() % (255 - 2*diff) + diff;
	color[2] = rand() % (255 - 2*diff) + diff;
}

void endgame() {
	char tmp[26], tmp2[20], tmp3[20];
	snprintf(tmp, 24, "Levels completed: %d!", stages);
	snprintf(tmp2, 20, "Best score: %d", maxStages);
	snprintf(tmp3, 20, "Games played: %d", matches);
	
	while(aptMainLoop() && !(hidKeysDown() & KEY_A)) {
		hidScanInput();
		
		pp2d_begin_draw(GFX_TOP, GFX_LEFT);
			pp2d_draw_text_center(GFX_TOP, 80, 1.0f, 1.0f, RGBA8(0,0,0,255), "End game!");
			pp2d_draw_text_center(GFX_TOP, 136, 0.7f, 0.7f, RGBA8(0,0,0,255), tmp);
			pp2d_draw_text_center(GFX_TOP, 170, 0.6f, 0.6f, RGBA8(200,0,0,200), "Press A to restart");
			
			pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
			
			pp2d_draw_text_center(GFX_BOTTOM, 82, 0.7f, 0.7f, RGBA8(0,0,0,200), tmp2);
			pp2d_draw_text_center(GFX_BOTTOM, 106, 0.7f, 0.7f, RGBA8(0,0,0,200), tmp3);
		pp2d_end_draw();
	}
}

void menu_start() {
	char ver[10];
	sprintf(ver, "v%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
	
	pp2d_begin_draw(GFX_TOP, GFX_LEFT);
		pp2d_draw_text_center(GFX_TOP, 70, 1.0f, 1.0f, RGBA8(0,0,0,255), "PICKR");
		pp2d_draw_text_center(GFX_TOP, 107, 0.65f, 0.65f, RGBA8(200,0,0,200), "Try to pick the different color square!");
		pp2d_draw_text_center(GFX_TOP, 150, 0.6f, 0.6f, RGBA8(0,0,0,200), "Press A to start a new game");
		pp2d_draw_text_center(GFX_TOP, 170, 0.5f, 0.5f, RGBA8(0,0,0,200), "Press B to exit");
		pp2d_draw_text(398 - pp2d_get_text_width(ver, 0.45f, 0.45f), 225, 0.45f, 0.45f, RGBA8(0,0,0,200), ver);
		
		pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
		pp2d_draw_text(3, 207, 0.5f, 0.5f, RGBA8(0,0,0,150), "Based on Sean M. Tracey's idea");
		pp2d_draw_text(3, 222, 0.5f, 0.5f, RGBA8(0,0,0,180), "Made with      by Bernardo Giordano");
		pp2d_draw_texture(TEXTURE_LIVES_SMALL, 70, 222);
	pp2d_end_draw();
}

void level() {
	int x, y;
	bool win = false;
	char stagestr[10];
	snprintf(stagestr, 10, "Level %d", stages);
	
	_time = 320;
	fillPattern();
	diff = (rand() % 2) ? diff : -diff;
	
	while(aptMainLoop() && _time > 0 && lives > 0) {
		hidScanInput();
		touchPosition touch;
		hidTouchRead(&touch);
		
		if (hidKeysDown() & KEY_TOUCH) {
			int x_start, y_start = 21;
			for (unsigned int i = 0; i < 4; i++) {
				x_start = 65;
				for (unsigned int j = 0; j < 4; j++) {
					if ((touch.px > x_start) && (touch.px < (x_start + 45)) && (touch.py > y_start) && (touch.py < (y_start + 45))) {
						if (pattern[i*4+j])
							win = true;
						else
							lives--;
					}
					x_start += 48;
				}
				y_start += 48;
			}
		}
		
		pp2d_begin_draw(GFX_TOP, GFX_LEFT);
			for (unsigned int i = 0, max = (lives < 5) ? 5 : lives; i < max; i++) {
				if (i < (unsigned int)lives)
					pp2d_draw_texture(TEXTURE_LIVES_BIG, 3 + i*27, 215);
				else
					pp2d_draw_texture_blend(TEXTURE_LIVES_BIG, 3 + i*27, 215, RGBA8(50,50,50,200));
			}
			pp2d_draw_text(396 - pp2d_get_text_width(stagestr, 0.7f, 0.7f), 218, 0.7f, 0.7f, RGBA8(200,0,0,200), stagestr);
		
			pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
			y = 21;
			for (unsigned int i = 0; i < 4; i++) {
				x = 65;
				for (unsigned int j = 0; j < 4; j++) {
					pp2d_draw_rectangle(x, y, 45, 45, (pattern[i*4+j]) ? RGBA8(color[0] - diff, color[1] - diff, color[2] - diff, 255) : RGBA8(color[0], color[1], color[2], 255));
					x += 48;
				}
				y += 48;
			}
			pp2d_draw_rectangle(0, 230, _time, 10, (_time > 100) ? RGBA8(0,255,0,255) : RGBA8(255,0,0,255));
		pp2d_end_draw();

		if (--_time == 0 && lives > 0) {
			_time = 320;
			lives--;
		}
		
		if (win) {
			if (++stages % 10 == 0 && lives < 10)
				lives++;
			updatediff();
			maxStages = (stages >= maxStages) ? stages : maxStages;
			level();
		}
	}
}

int main() {
	maxStages = 0;
	matches = 0;
	initServices();
	
	while(aptMainLoop()) {
		hidScanInput();
		
		if (hidKeysDown() & KEY_B)
			break;
		
		if (hidKeysDown() & KEY_A) {
			level();
			matches++;
			endgame();
			initVars();
		}
		
		menu_start();
	}
	
	exitServices();
	return 0;
}