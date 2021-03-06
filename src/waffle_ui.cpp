#include <string>
#include "waffle.h"
#include "waffle_ui.h"
#include "dmf.h"
#include "chips/mamedef.h"

extern DMF::Song song;
extern int active_instr;
extern int* CurBufR;
extern unsigned int SAMPLES_PER_BUFFER;
extern void play(UINT8 note, UINT8 velocity);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define WAVE_AREA_WIDTH SAMPLES_PER_BUFFER
#define WAVE_AREA_HEIGHT 300
#define OPERATOR_WIDGET_WIDTH 500
#define OPERATOR_WIDGET_HEIGHT 160
#define FM_OPERATOR_COMMON_PARAMS_HEIGHT 200
#define INSTRUMENT_DIALOG_HEIGHT (FM_OPERATOR_COMMON_PARAMS_HEIGHT + 4*OPERATOR_WIDGET_HEIGHT)
#define MARGIN 10
#define PADDING 30

WaffleUI::WaffleUI(){
	SDL_CreateWindowAndRenderer(WINDOW_WIDTH,
	                            WINDOW_HEIGHT,
	                            SDL_WINDOW_RESIZABLE,
	                            &m_program_window, &m_program_window_renderer);
	SDL_MaximizeWindow(m_program_window);
	SDL_SetRenderDrawBlendMode(m_program_window_renderer, SDL_BLENDMODE_BLEND);
	ui_needs_update = true;
	grabbed_item = -1;
	instrument_dialog_yscroll = 0;
}

void WaffleUI::program_loop(){
	int x, y, note, current_note;
	bool keep_running = true;
	TTF_Init();
	m_font = TTF_OpenFont("Tomorrow-Regular.ttf", 10);
	while(keep_running){
		while(SDL_PollEvent(&m_program_window_event) > 0){
			switch(m_program_window_event.type){
				case SDL_QUIT:
					keep_running = false;
					break;
				case SDL_WINDOWEVENT:
					if (m_program_window_event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
						ui_items.clear();
						ui_needs_update = true;
					}
					break;
				case SDL_MOUSEWHEEL:
					int w, h;
					SDL_GetRendererOutputSize(m_program_window_renderer, &w, &h);
					instrument_dialog_yscroll += m_program_window_event.wheel.y*10;
					instrument_dialog_yscroll = std::min(instrument_dialog_yscroll, 0);
					instrument_dialog_yscroll = std::max(instrument_dialog_yscroll, h-INSTRUMENT_DIALOG_HEIGHT);
					ui_items.clear();
					ui_needs_update = true;
					break;
				case SDL_MOUSEBUTTONDOWN:
					x = m_program_window_event.motion.x;
					y = m_program_window_event.motion.y;
					grabbed_item = -1;
					for (int i=0; i < ui_items.size(); i++){
						UI_Item item = ui_items[i];
						if (x >= item.area.x && x <= item.area.x + item.area.w &&
						    y >= item.area.y && y <= item.area.y + item.area.h){
							grabbed_item = i;
							update_slider(x, y);
						}
					}
					break;
				case SDL_MOUSEBUTTONUP:
					grabbed_item = -1;
					break;
				case SDL_MOUSEMOTION:
					if (grabbed_item != -1){
						x = m_program_window_event.motion.x;
						y = m_program_window_event.motion.y;
						update_slider(x, y);
					}
					break;
				case SDL_KEYDOWN:
					note = get_note_from_keyboard(m_program_window_event.key.keysym.sym);
					if (note != current_note){
						play(note, 63);
						current_note = note;
					}
					break;
				case SDL_KEYUP:
					play(current_note, 0);
					current_note = 0;
					break;
			}
		}

		update(1.0/60.0);
		draw();
	}
}

int WaffleUI::get_note_from_keyboard(SDL_Keycode keysym){
	int C3 = 48; //FIXME! Is this correct?
	switch(keysym){
		case SDLK_z: return C3;
		case SDLK_s: return C3 + 1;
		case SDLK_x: return C3 + 2;
		case SDLK_d: return C3 + 3;
		case SDLK_c: return C3 + 4;
		case SDLK_v: return C3 + 5;
		case SDLK_g: return C3 + 6;
		case SDLK_b: return C3 + 7;
		case SDLK_h: return C3 + 8;
		case SDLK_n: return C3 + 9;
		case SDLK_j: return C3 + 10;
		case SDLK_m: return C3 + 11;
		case SDLK_COMMA:
		case SDLK_q: return C3 + 12;
		case SDLK_2: return C3 + 13;
		case SDLK_w: return C3 + 14;
		case SDLK_3: return C3 + 15;
		case SDLK_e: return C3 + 16;
		case SDLK_r: return C3 + 17;
		case SDLK_5: return C3 + 18;
		case SDLK_t: return C3 + 19;
		case SDLK_6: return C3 + 20;
		case SDLK_y: return C3 + 21;
		case SDLK_7: return C3 + 22;
		case SDLK_u: return C3 + 23;
		case SDLK_i: return C3 + 24;
	}
	return 0;
}

void WaffleUI::update(double delta_time){
}

void WaffleUI::draw_text(int x, int y, std::string text_str, SDL_Color textColor){
	SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, text_str.c_str(), textColor);
	SDL_Texture* text = SDL_CreateTextureFromSurface(m_program_window_renderer, textSurface);
	int text_width = textSurface->w;
	int text_height = textSurface->h;
	SDL_FreeSurface(textSurface);
	SDL_Rect renderQuad = { x, y, text_width, text_height };
	SDL_RenderCopy(m_program_window_renderer, text, NULL, &renderQuad);
	SDL_DestroyTexture(text);
}

void WaffleUI::draw_waveform_viewer(int x, int y){
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = WAVE_AREA_WIDTH;
	rect.h = WAVE_AREA_HEIGHT;
	SDL_SetRenderDrawColor(m_program_window_renderer, 20, 20, 20, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);

	for (int i=10; i<SAMPLES_PER_BUFFER-10; i++){
		SDL_RenderDrawLine(m_program_window_renderer,
		                   /* X1: */  x + i,
		                   /* Y1: */  y + WAVE_AREA_HEIGHT/2 + CurBufR[i]/32,
		                   /* X2: */  x + (i+1),
		                   /* Y2: */  y + WAVE_AREA_HEIGHT/2 + CurBufR[(i+1)]/32
		);
	}
}

#define OPERATOR_GRAPH_AREA_WIDTH 265
#define OPERATOR_GRAPH_AREA_HEIGHT 64

void WaffleUI::draw_fm_operator_graph(int x, int y, DMF::Instrument::FM_Operator op){
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = OPERATOR_GRAPH_AREA_WIDTH;
	rect.h = OPERATOR_GRAPH_AREA_HEIGHT;
	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	float a = (31 - op.AR) / 31.0;
	float d = (31 - op.D1R) / 31.0;
	float s = (15 - op.SL) / 15.0;
	float d2 = (31 - op.D2R) / 31.0;
	float r = (15 - op.RR) / 15.0;
	float TotalLevel = (127 - op.TL) / 127.0;


	// Graph lines:
	SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);
	SDL_RenderDrawLine(m_program_window_renderer,
	                   /* X1: */  x,
	                   /* Y1: */  y + OPERATOR_GRAPH_AREA_HEIGHT,
	                   /* X2: */  x + OPERATOR_GRAPH_AREA_WIDTH * 0.9 * a * d,
	                   /* Y2: */  y + OPERATOR_GRAPH_AREA_HEIGHT * (1 - TotalLevel)
	);

	SDL_RenderDrawLine(m_program_window_renderer,
	                   /* X1: */  x + OPERATOR_GRAPH_AREA_WIDTH * 0.9 * a * d,
	                   /* Y1: */  y + OPERATOR_GRAPH_AREA_HEIGHT * (1 - TotalLevel),
	                   /* X2: */  x + OPERATOR_GRAPH_AREA_WIDTH * 0.9 * d,
	                   /* Y2: */  y + OPERATOR_GRAPH_AREA_HEIGHT * (1 - s * TotalLevel)
	);


	SDL_SetRenderDrawColor(m_program_window_renderer, 48, 98, 198, 255);
	SDL_RenderDrawLine(m_program_window_renderer,
	                   /* X1: */  x + OPERATOR_GRAPH_AREA_WIDTH * 0.9 * d,
	                   /* Y1: */  y + OPERATOR_GRAPH_AREA_HEIGHT * (1 - s * TotalLevel),
	                   /* X2: */  x + OPERATOR_GRAPH_AREA_WIDTH * (0.9 * d + (1 - 0.9 * d) * r),
	                   /* Y2: */  y + OPERATOR_GRAPH_AREA_HEIGHT - (op.RR != 0 ? 0 : OPERATOR_GRAPH_AREA_HEIGHT * s * TotalLevel)
	);


	SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);
	SDL_RenderDrawLine(m_program_window_renderer,
	                   /* X1: */  x + OPERATOR_GRAPH_AREA_WIDTH * 0.9 * d,
	                   /* Y1: */  y + OPERATOR_GRAPH_AREA_HEIGHT * (1 - s * TotalLevel),
	                   /* X2: */  x + OPERATOR_GRAPH_AREA_WIDTH * (0.9 * d + (1 - 0.9 * d) * d2),
	                   /* Y2: */  y + OPERATOR_GRAPH_AREA_HEIGHT - (op.D2R != 0 ? 0 : OPERATOR_GRAPH_AREA_HEIGHT * s * TotalLevel)
	);
}

void WaffleUI::register_slider(int type, SDL_Rect area, int value_min, int value_max, uint8_t* value){
	if (ui_needs_update){
		UI_Item item;
		item.type = type;
		item.area = area;
		item.value_min = value_min;
		item.value_max = value_max;
		item.value = value;
		ui_items.push_back(item);
	}
}

void WaffleUI::update_slider(int x, int y){
	int coord, coord_min, coord_max;
	UI_Item* item = &ui_items[grabbed_item];
	if (item->type == VERTICAL_SLIDER){
		coord = m_program_window_event.motion.y;
		coord_min = item->area.y;
		coord_max = item->area.y + item->area.h;
	} else { /* HORIZONTAL_SLIDER */
		coord = m_program_window_event.motion.x;
		coord_min = item->area.x;
		coord_max = item->area.x + item->area.w;
	}
	coord = std::max(coord_min, std::min(coord, coord_max));
	int value = ((coord - coord_min) / float(coord_max - coord_min)) * \
	             (item->value_max - item->value_min);
	if (*(item->value) != value){
		*(item->value) = value;
		setup_instrument(song.instrument[active_instr]);
	}
}



#define VSLIDER_WIDTH 24
#define VSLIDER_HEIGHT 160
void WaffleUI::vertical_slider(int x, int y, const char* name, uint8_t* value, int max_value){
	int width = VSLIDER_WIDTH - 6;
	int height = VSLIDER_HEIGHT - 2 * 25;
	SDL_Rect rect;

	SDL_Color WHITE = { 255, 255, 255, 0 };
	draw_text(x - 2 + (strlen(name) == 1 ? 5 : 0), y + 5, std::string(name), WHITE);
	draw_text(x + (*value < 10 ? 5 : 0), y + 28 + height, std::to_string(*value), WHITE);

	// slider body
	rect.x = x;
	rect.y = y + 25;
	rect.w = width;
	rect.h = height;
        register_slider(VERTICAL_SLIDER, rect, 0, max_value, value);
	SDL_SetRenderDrawColor(m_program_window_renderer, 128, 128, 128, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	// slider level
	rect.x = x - 2;
	rect.y = y + 25 + (height-5) * (*value/float(max_value));
	rect.w = width + 4;
	rect.h = 5;
	SDL_SetRenderDrawColor(m_program_window_renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);
}

#define HSLIDER_WIDTH ((OPERATOR_GRAPH_AREA_WIDTH - MARGIN)/2)
#define HSLIDER_HEIGHT 10
void WaffleUI::horizontal_slider(int x, int y, const char* name, uint8_t* value, int min_value, int max_value){
	int width = HSLIDER_WIDTH;
	int height = HSLIDER_HEIGHT;
	SDL_Rect rect;

	SDL_Color WHITE = { 255, 255, 255, 0 };
	draw_text(x, y - 2, std::string(name) + std::string("  ") + std::to_string(min_value + *value), WHITE);
	
	// slider body
	rect.x = x;
	rect.y = y + height;
	rect.w = width;
	rect.h = height;
	register_slider(HORIZONTAL_SLIDER, rect, min_value, max_value, value);
	SDL_SetRenderDrawColor(m_program_window_renderer, 128, 128, 128, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	// slider level
	rect.x = x + (width-5) * (*value)/float(max_value - min_value);
	rect.y = y + height;
	rect.w = 5;
	rect.h = height;
	SDL_SetRenderDrawColor(m_program_window_renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);
}

void WaffleUI::draw(){
	int w, h;
	SDL_GetRendererOutputSize(m_program_window_renderer, &w, &h);

	SDL_RenderClear(m_program_window_renderer);
	draw_waveform_viewer(MARGIN, MARGIN);
	draw_instrument_dialog(w - OPERATOR_WIDGET_WIDTH, instrument_dialog_yscroll);
}

void WaffleUI::draw_single_operator_block_icon(int num, int x, int y, int extra_x){
	int K = 24;
	int L = 12;
	float l = (K-L)/2.0;
	SDL_Rect r;
	r.w = L;
	r.h = L;
	r.x = x + l;
	r.y = y - 0.5*L;
	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 255, 0, 255);
	SDL_RenderFillRect(m_program_window_renderer, &r);
	SDL_RenderDrawLine(m_program_window_renderer,
	   /* X1: */  x,               /* Y1: */  y,
	   /* X2: */  x + K + extra_x, /* Y2: */  y
	);

	SDL_Color BLACK = { 0, 0, 0, 255 };
	draw_text(x + K/2.0 - 2, y-L/2.0, std::to_string(num), BLACK);

	if (num==1){
		SDL_SetRenderDrawColor(m_program_window_renderer, 0, 255, 0, 255);
		SDL_RenderDrawLine(m_program_window_renderer,
		   /* X1: */  x + l/2.0, /* Y1: */  y,
		   /* X2: */  x + l/2.0, /* Y2: */  y - L/2 - l/2.0
		);
		SDL_RenderDrawLine(m_program_window_renderer,
		   /* X1: */  x + l/2.0,     /* Y1: */  y - L/2 - l/2.0,
		   /* X2: */  x + K - l/2.0, /* Y2: */  y - L/2 - l/2.0
		);
		SDL_RenderDrawLine(m_program_window_renderer,
		   /* X1: */  x + K - l/2.0, /* Y1: */  y,
		   /* X2: */  x + K - l/2.0, /* Y2: */  y - L/2 - l/2.0
		);
	}
}

void WaffleUI::draw_fm_algorithm_icon(int x, int y, int alg){
	int K = 24;
	int L = 12;

	switch (alg){
		case 0:
			draw_single_operator_block_icon(1,       x, y, 0);
			draw_single_operator_block_icon(2,   x + K, y, 0);
			draw_single_operator_block_icon(3, x + 2*K, y, 0);
			draw_single_operator_block_icon(4, x + 3*K, y, 0);
			break;
		case 1:
			draw_single_operator_block_icon(1,       x, y - L, 0);
			draw_single_operator_block_icon(2,       x, y + L, 0);
			draw_single_operator_block_icon(3,   x + K,     y, 0);
			draw_single_operator_block_icon(4, x + 2*K,     y, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + K, /* Y1: */  y - L,
			   /* X2: */  x + K, /* Y2: */  y + L
			);
			break;
		case 2:
			draw_single_operator_block_icon(1,       x, y - L, K);
			draw_single_operator_block_icon(2,       x, y + L, 0);
			draw_single_operator_block_icon(3,   x + K, y + L, 0);
			draw_single_operator_block_icon(4, x + 2*K,     y, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K, /* Y1: */  y - L,
			   /* X2: */  x + 2*K, /* Y2: */  y + L
			);
			break;
		case 3:
			draw_single_operator_block_icon(1,       x, y - L, 0);
			draw_single_operator_block_icon(2,   x + K, y - L, 0);
			draw_single_operator_block_icon(3,   x + K, y + L, 0);
			draw_single_operator_block_icon(4, x + 2*K,     y, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K, /* Y1: */  y - L,
			   /* X2: */  x + 2*K, /* Y2: */  y + L
			);
			break;
		case 4:
			draw_single_operator_block_icon(1,       x, y - L, 0);
			draw_single_operator_block_icon(2,   x + K, y - L, 0);
			draw_single_operator_block_icon(3,       x, y + L, 0);
			draw_single_operator_block_icon(4,   x + K, y + L, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K, /* Y1: */  y - L,
			   /* X2: */  x + 2*K, /* Y2: */  y + L
			);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K,         /* Y1: */  y,
			   /* X2: */  x + 2*K + 0.5*L, /* Y2: */  y
			);
			break;
		case 5:
			draw_single_operator_block_icon(1,       x,         y, 0);
			draw_single_operator_block_icon(2,   x + K, y - 1.5*L, 0);
			draw_single_operator_block_icon(3,   x + K,         y, 0);
			draw_single_operator_block_icon(4,   x + K, y + 1.5*L, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 1*K, /* Y1: */  y - 1.5*L,
			   /* X2: */  x + 1*K, /* Y2: */  y + 1.5*L
			);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K, /* Y1: */  y - 1.5*L,
			   /* X2: */  x + 2*K, /* Y2: */  y + 1.5*L
			);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K,         /* Y1: */  y,
			   /* X2: */  x + 2*K + 0.5*L, /* Y2: */  y
			);
			break;
		case 6:
			draw_single_operator_block_icon(1,       x, y - 1.5*L, 0);
			draw_single_operator_block_icon(2,   x + K, y - 1.5*L, 0);
			draw_single_operator_block_icon(3,   x + K,         y, 0);
			draw_single_operator_block_icon(4,   x + K, y + 1.5*L, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K, /* Y1: */  y - 1.5*L,
			   /* X2: */  x + 2*K, /* Y2: */  y + 1.5*L
			);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + 2*K,         /* Y1: */  y,
			   /* X2: */  x + 2*K + 0.5*L, /* Y2: */  y
			);
			break;
		case 7:
			draw_single_operator_block_icon(1,   x, y - 2.25*L, 0);
			draw_single_operator_block_icon(2,   x, y - 0.75*L, 0);
			draw_single_operator_block_icon(3,   x, y + 0.75*L, 0);
			draw_single_operator_block_icon(4,   x, y + 2.25*L, 0);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + K, /* Y1: */  y - 2.25*L,
			   /* X2: */  x + K, /* Y2: */  y + 2.25*L
			);
			SDL_RenderDrawLine(m_program_window_renderer,
			   /* X1: */  x + K,         /* Y1: */  y,
			   /* X2: */  x + K + 0.5*L, /* Y2: */  y
			);
			break;
	}
}


void WaffleUI::draw_instrument_dialog(int x, int y){

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = OPERATOR_WIDGET_WIDTH;
	rect.h = INSTRUMENT_DIALOG_HEIGHT;

	// Draw background:
	SDL_SetRenderDrawColor(m_program_window_renderer, 32, 32, 32, 220);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	horizontal_slider(x + 100, y + 50,
	                  "FMS:  ",  &(song.instrument[active_instr].fm.LFO), 0, 7);
	horizontal_slider(x + 100, y + 80,
	                  "AMS:  ",  &(song.instrument[active_instr].fm.LFO2), 0, 3);
	horizontal_slider(x + 100, y + 110,
	                  "FEEDBACK:  ",  &(song.instrument[active_instr].fm.FB), 0, 7);
	horizontal_slider(x + 100, y + 140,
	                  "ALGORITHM:  ",  &(song.instrument[active_instr].fm.ALG), 0, 7);

	draw_fm_algorithm_icon(x + 250, y + 140, song.instrument[active_instr].fm.ALG);

	y += FM_OPERATOR_COMMON_PARAMS_HEIGHT;
	int x0 = x;
	// Render operators:
	for (int i=0; i<4; i++){
		x = x0;

		if (i>0){
			// Separator line between the operator widgets:
			SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);
			SDL_RenderDrawLine(m_program_window_renderer,
			                                           x + PADDING,   y,
			                   x + OPERATOR_WIDGET_WIDTH - PADDING,   y);
		}

		x += PADDING;

		// Graph:
		draw_fm_operator_graph(x + 135, y + 25, song.instrument[active_instr].fm.op[i]);

		// ADSR sliders:
		vertical_slider(x + 0 * VSLIDER_WIDTH, y, "A",  &(song.instrument[active_instr].fm.op[i].AR),  31);
		vertical_slider(x + 1 * VSLIDER_WIDTH, y, "D",  &(song.instrument[active_instr].fm.op[i].D1R), 31);
		vertical_slider(x + 2 * VSLIDER_WIDTH, y, "S",  &(song.instrument[active_instr].fm.op[i].SL),  15);
		vertical_slider(x + 3 * VSLIDER_WIDTH, y, "D2", &(song.instrument[active_instr].fm.op[i].D2R), 31);
		vertical_slider(x + 4 * VSLIDER_WIDTH, y, "R",  &(song.instrument[active_instr].fm.op[i].RR),  15);

		// TL slider:
		vertical_slider(x + 420, y, "LEVEL",  &(song.instrument[active_instr].fm.op[i].TL), 127);

		// Operator number
		SDL_Color YELLOW = { 255, 255, 0, 0 };
		draw_text(x + 135, y + 5, std::string("OPERATOR  ") + std::to_string(i+1), YELLOW);

		// MULT slider:
		horizontal_slider(x + 135, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT,
		                  "MULT:  ",  &(song.instrument[active_instr].fm.op[i].MULT), 0, 15);

		// DT slider:
		horizontal_slider(x + 135, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT,
		                  "DETUNE:  ",  &(song.instrument[active_instr].fm.op[i].DT), -3, 3);

		// RS slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT,
		                  "RS:  ",  &(song.instrument[active_instr].fm.op[i].RS), 0, 3);

		// SSG-EG slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT,
		                  "SSG-EG:  ",  &(song.instrument[active_instr].fm.op[i].SSGMODE), 0, 7);

		// Y coordinate for the next operator widget:
		y += OPERATOR_WIDGET_HEIGHT;
	}

	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_program_window_renderer);

	ui_needs_update = false;
}
