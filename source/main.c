#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sftd_font *font40, *font18;
int _time;
int diff;
int stages;
bool pattern[16];
u32 color[3];

void exitServices() {
	sftd_free_font(font18);
	sftd_free_font(font40);
	
	hidExit();
	sdmcExit();
	romfsExit();
	sftd_fini();
	sf2d_fini();
}

void initServices() {
	srand(time(NULL));
	
	sf2d_init();
	sftd_init();
	sf2d_set_clear_color(RGBA8(255, 255, 255, 255));
	sf2d_set_vblank_wait(1);
	sdmcInit();
	romfsInit();
	
	font40 = sftd_load_font_file("romfs:/font.otf");
	font18 = sftd_load_font_file("romfs:/font.otf");
	sftd_draw_text(font40, 0, 0, RGBA8(0, 0, 0, 0), 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890:-.'!?()\"end");
	sftd_draw_text(font18, 0, 0, RGBA8(0, 0, 0, 0), 18, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890:-.'!?()\"end");

	_time = 320;
	diff = 10;
	stages = 0;
}

void fillPattern() {
	for (unsigned int i = 0; i < 16; i++)
		pattern[i] = false;
	
	pattern[rand() % 16] = true;
	
	color[0] = rand() % 255;
	color[1] = rand() % 255;
	color[2] = rand() % 255;
}

void endgame() {
	while(aptMainLoop()) {
		hidScanInput();
		
		if (hidKeysDown() & KEY_A)
			return;
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sftd_draw_text(font40, (400 - sftd_get_text_width(font40, 40, "You lose!")) / 2, 80, RGBA8(0,0,0,255), 40, "You lose!");
			sftd_draw_textf(font18, (400 - sftd_get_text_width(font18, 18, "Levels completed: 00!")) / 2, 130, RGBA8(0,0,0,255), 18, "Levels completed: %d!", stages);
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sf2d_end_frame();
		sf2d_swapbuffers();	
	}
}

void menu_start() {
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sftd_draw_text(font40, (400 - sftd_get_text_width(font40, 40, "PICKR")) / 2, 80, RGBA8(0,0,0,255), 40, "PICKR");
		sftd_draw_text(font18, (400 - sftd_get_text_width(font18, 18, "Press A to start a new game")) / 2, 130, RGBA8(0,0,0,255), 18, "Press A to start a new game");
	sf2d_end_frame();
	
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void level() {
	int x, y;
	bool win = false;
	
	_time = 320;
	fillPattern();
	
	while(aptMainLoop() && _time > 0) {
		hidScanInput();
		touchPosition touch;
		hidTouchRead(&touch);
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			y = 21;
			for (unsigned int i = 0; i < 4; i++) {
				x = 65;
				for (unsigned int j = 0; j < 4; j++) {
					sf2d_draw_rectangle(x, y, 45, 45, (pattern[i*4+j]) ? RGBA8(color[0] - diff, color[1] - diff, color[2] - diff, 255) : RGBA8(color[0], color[1], color[2], 255));
					x += 48;
				}
				y += 48;
			}
			sf2d_draw_rectangle(0, 230, _time, 10, (_time > 100) ? RGBA8(0,255,0,255) : RGBA8(255,0,0,255));
		sf2d_end_frame();
		sf2d_swapbuffers();
		
		if (hidKeysHeld() & KEY_TOUCH) {
			int x_start, y_start = 21;
			for (unsigned int i = 0; i < 4; i++) {
				x_start = 65;
				for (unsigned int j = 0; j < 4; j++) {
					if ((touch.px > x_start) && (touch.px < (x_start + 45)) && (touch.py > y_start) && (touch.py < (y_start + 45)) && pattern[i*4+j])
						win = true;
					x_start += 48;
				}
				y_start += 48;
			}
		}
		
		_time--;
		
		if (win) {
			stages++;
			level();
		}
	}
}

int main() {
	initServices();
	
	while(aptMainLoop()) {
		hidScanInput();
		
		if (hidKeysDown() & KEY_B)
			break;
		
		if (hidKeysDown() & KEY_A) {
			level();
			endgame();
		}
		
		menu_start();
	}
	
	exitServices();
	return 0;
}