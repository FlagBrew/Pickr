#include <3ds.h>
#include <sf2d.h>
#include <sfil.h>
#include <sftd.h>
#include <stdio.h>
#include <stdlib.h>

sftd_font *font40, *font18, *font12;
sf2d_texture *_lives, *_lives16p;

int _time, diff, stages, maxStages, lives, matches;
bool pattern[16];
int color[3];

void exitServices() {
	sf2d_free_texture(_lives16p);
	sf2d_free_texture(_lives);
	sftd_free_font(font12);
	sftd_free_font(font18);
	sftd_free_font(font40);

	sdmcExit();
	romfsExit();
	sftd_fini();
	sf2d_fini();
}

void initVars() {
	lives = 5;
	_time = 320;
	diff = 10;
	stages = 0;	
}

void initServices() {
	srand(time(NULL));
	
	sf2d_init();
	sftd_init();
	sf2d_set_clear_color(RGBA8(255, 255, 255, 255));
	sf2d_set_vblank_wait(1);
	sdmcInit();
	romfsInit();
	
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	sf2d_end_frame();
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_end_frame();
	sf2d_swapbuffers();	
	
	_lives = sfil_load_PNG_file("romfs:/lives.png", SF2D_PLACE_RAM);
	_lives16p = sfil_load_PNG_file("romfs:/lives16p.png", SF2D_PLACE_RAM);
	font40 = sftd_load_font_file("romfs:/font.otf");
	font18 = sftd_load_font_file("romfs:/roboto.ttf");
	font12 = sftd_load_font_file("romfs:/roboto.ttf");
	sftd_draw_text(font40, 0, 0, RGBA8(0, 0, 0, 0), 40, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890:-.'!?()\"end");
	sftd_draw_text(font18, 0, 0, RGBA8(0, 0, 0, 0), 18, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890:-.'!?()\"end");
	sftd_draw_text(font12, 0, 0, RGBA8(0, 0, 0, 0), 12, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890:-.'!?()\"end");
	
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
	char tmp[24];
	snprintf(tmp, 24, "Levels completed: %d!", stages);
	
	while(aptMainLoop()) {
		hidScanInput();
		
		if (hidKeysDown() & KEY_A)
			return;
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			sftd_draw_text(font40, (400 - sftd_get_text_width(font40, 40, "End game")) / 2, 80, RGBA8(0,0,0,255), 40, "End game!");
			sftd_draw_text(font18, (400 - sftd_get_text_width(font18, 18, tmp)) / 2, 136, RGBA8(0,0,0,255), 18, tmp);
			sftd_draw_text(font12, (400 - sftd_get_text_width(font12, 12, "Press A to restart")) / 2, 165, RGBA8(200,0,0,200), 12, "Press A to restart");
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
			sftd_draw_textf(font12, 3, 2, RGBA8(0,0,0,200), 12, "Best score: %d", maxStages);
			sftd_draw_textf(font12, 3, 18, RGBA8(0,0,0,200), 12, "Games played: %d", matches);
		sf2d_end_frame();
		sf2d_swapbuffers();	
	}
}

void menu_start() {
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sftd_draw_text(font40, (400 - sftd_get_text_width(font40, 40, "PICKR")) / 2, 80, RGBA8(0,0,0,255), 40, "PICKR");
		sftd_draw_text(font18, (400 - sftd_get_text_width(font18, 18, "Press A to start a new game")) / 2, 136, RGBA8(0,0,0,200), 18, "Press A to start a new game");
		sftd_draw_text(font12, (400 - sftd_get_text_width(font12, 12, "Press B to exit")) / 2, 165, RGBA8(0,0,0,200), 12, "Press B to exit");
	sf2d_end_frame();
	
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		sftd_draw_text(font12, 2, 208, RGBA8(0,0,0,150), 12, "Based on Sean M. Tracey's idea");
		sftd_draw_text(font12, 2, 224, RGBA8(0,0,0,180), 12, "Made with       by Bernardo Giordano");
		sf2d_draw_texture(_lives16p, 60, 223);
	sf2d_end_frame();
	sf2d_swapbuffers();
}

void level() {
	int x, y;
	bool win = false;
	
	_time = 320;
	diff = (rand() % 2) ? diff : -diff;
	fillPattern();
	
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
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
			for (unsigned int i = 0; i < 5; i++) {
				if (i < lives)
					sf2d_draw_texture(_lives, 2 + i*27, 215);
				else
					sf2d_draw_texture_blend(_lives, 2 + i*27, 215, RGBA8(50,50,50,200));
			}
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
		
		_time--;
		if (_time == 0 && lives > 0) {
			_time = 320;
			lives--;
		}
		
		if (win) {
			diff = 10;
			stages++;
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