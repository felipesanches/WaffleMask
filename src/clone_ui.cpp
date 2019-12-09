#include <string>
#include "clone_ui.h"
#include "dmf.h"

extern int* CurBufR;
extern unsigned int SAMPLES_PER_BUFFER;

#define WAVE_AREA_WIDTH SAMPLES_PER_BUFFER
#define WAVE_AREA_HEIGHT 300
#define OPERATOR_WIDGET_WIDTH 500
#define OPERATOR_WIDGET_HEIGHT 160
#define MARGIN 10
#define PADDING 30

CloneUI::CloneUI(){
	SDL_CreateWindowAndRenderer(3*MARGIN + OPERATOR_WIDGET_WIDTH + WAVE_AREA_WIDTH,
	                            2*MARGIN + 4*OPERATOR_WIDGET_HEIGHT,
	                            SDL_WINDOW_RESIZABLE,
	                            &m_program_window, &m_program_window_renderer);                     
}

void CloneUI::program_loop(){
	bool keep_running = true;
	TTF_Init();
	m_font = TTF_OpenFont("Tomorrow-Regular.ttf", 10);
	while(keep_running){
		while(SDL_PollEvent(&m_program_window_event) > 0){
			switch(m_program_window_event.type){
				case SDL_QUIT:
					keep_running = false;
				}
		}

		update(1.0/60.0);
		draw();
	}
}

void CloneUI::update(double delta_time){
}

void CloneUI::draw_text(int x, int y, std::string text_str, SDL_Color textColor){
	SDL_Surface* textSurface = TTF_RenderText_Solid(m_font, text_str.c_str(), textColor);
	SDL_Texture* text = SDL_CreateTextureFromSurface(m_program_window_renderer, textSurface);
	int text_width = textSurface->w;
	int text_height = textSurface->h;
	SDL_FreeSurface(textSurface);
	SDL_Rect renderQuad = { x, y, text_width, text_height };
	SDL_RenderCopy(m_program_window_renderer, text, NULL, &renderQuad);
	SDL_DestroyTexture(text);
}

void CloneUI::draw_wave(int x, int y){
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

void CloneUI::draw_graph(int x, int y, DMF::Instrument::FM_Operator op){
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

#define VSLIDER_WIDTH 24
#define VSLIDER_HEIGHT 160
void CloneUI::vertical_slider(int x, int y, const char* name, int value, int max_value){
	int width = VSLIDER_WIDTH - 6;
	int height = VSLIDER_HEIGHT - 2 * 25;
	SDL_Rect rect;

	SDL_Color WHITE = { 255, 255, 255, 0 };
	draw_text(x - 2 + (strlen(name) == 1 ? 5 : 0), y + 5, std::string(name), WHITE);
	draw_text(x + (value < 10 ? 5 : 0), y + 28 + height, std::to_string(value), WHITE);

	// slider body
	rect.x = x;
	rect.y = y + 25;
	rect.w = width;
	rect.h = height;
	SDL_SetRenderDrawColor(m_program_window_renderer, 128, 128, 128, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	// slider level
	rect.x = x - 2;
	rect.y = y + 25 + (height-5) * (value/float(max_value));
	rect.w = width + 4;
	rect.h = 5;
	SDL_SetRenderDrawColor(m_program_window_renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);
}

#define HSLIDER_WIDTH ((OPERATOR_GRAPH_AREA_WIDTH - MARGIN)/2)
#define HSLIDER_HEIGHT 10
void CloneUI::horizontal_slider(int x, int y, const char* name, int value, int min_value, int max_value){
	int width = HSLIDER_WIDTH;
	int height = HSLIDER_HEIGHT;
	SDL_Rect rect;

	SDL_Color WHITE = { 255, 255, 255, 0 };
	draw_text(x, y - 2, std::string(name) + std::string("  ") + std::to_string(value), WHITE);
	
	// slider body
	rect.x = x;
	rect.y = y + height;
	rect.w = width;
	rect.h = height;
	SDL_SetRenderDrawColor(m_program_window_renderer, 128, 128, 128, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	// slider level
	rect.x = x + (width-5) * ((value - min_value)/float(max_value - min_value));
	rect.y = y + height;
	rect.w = 5;
	rect.h = height;
	SDL_SetRenderDrawColor(m_program_window_renderer, 220, 220, 220, 255);
	SDL_RenderFillRect(m_program_window_renderer, &rect);
}

extern DMF::Song song;
extern int active_instr;

void CloneUI::draw(){

#if 0
//DEBUG:
song.instrument[active_instr].fm.op[0].TL = 0;
song.instrument[active_instr].fm.op[0].AR = 15;
song.instrument[active_instr].fm.op[0].D1R = 20;
song.instrument[active_instr].fm.op[0].SL = 7;
song.instrument[active_instr].fm.op[0].D2R = 0;
song.instrument[active_instr].fm.op[0].RR = 15;
#endif

	SDL_RenderClear(m_program_window_renderer);

	// WAVE FORMS:
	draw_wave(OPERATOR_WIDGET_WIDTH + 2*MARGIN, MARGIN);

	SDL_Rect rect;
	rect.x = 10;
	rect.y = 10;
	rect.w = OPERATOR_WIDGET_WIDTH;
	rect.h = 4*OPERATOR_WIDGET_HEIGHT;

	// Draw background:
	SDL_SetRenderDrawColor(m_program_window_renderer, 32, 32, 32, 32);
	SDL_RenderFillRect(m_program_window_renderer, &rect);

	int x = 0, y = MARGIN;
	// Render operators:
	for (int i=0; i<4; i++){
		x = MARGIN;

		if (i>0){
			// Separator line between the operator widgets:
			SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);
			SDL_RenderDrawLine(m_program_window_renderer,
			                                           x + PADDING,   y,
			                   x + OPERATOR_WIDGET_WIDTH - PADDING,   y);
		}

		x += PADDING;

		// Graph:
		draw_graph(x + 135, y + 25, song.instrument[active_instr].fm.op[i]);

		// ADSR sliders:
		vertical_slider(x + 0 * VSLIDER_WIDTH, y, "A",  song.instrument[active_instr].fm.op[i].AR,  31);
		vertical_slider(x + 1 * VSLIDER_WIDTH, y, "D",  song.instrument[active_instr].fm.op[i].D1R, 31);
		vertical_slider(x + 2 * VSLIDER_WIDTH, y, "S",  song.instrument[active_instr].fm.op[i].SL,  15);
		vertical_slider(x + 3 * VSLIDER_WIDTH, y, "D2", song.instrument[active_instr].fm.op[i].D2R, 31);
		vertical_slider(x + 4 * VSLIDER_WIDTH, y, "R",  song.instrument[active_instr].fm.op[i].RR,  15);

		// TL slider:
		vertical_slider(x + 420, y, "TL",  song.instrument[active_instr].fm.op[i].TL, 127);

		// Operator number
		SDL_Color YELLOW = { 255, 255, 0, 0 };
		draw_text(x + 135, y + 5, std::string("OPERATOR  ") + std::to_string(i+1), YELLOW);

		// MULT slider:
		horizontal_slider(x + 135, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT,
		                  "MULT",  song.instrument[active_instr].fm.op[i].MULT, 0, 15);

		// DT slider:
		horizontal_slider(x + 135, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT,
		                  "DT",  song.instrument[active_instr].fm.op[i].DT - 3, -3, 3);

		// RS slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT,
		                  "RS",  song.instrument[active_instr].fm.op[i].RS & 0x3, 0, 3);

		// SSG-EG slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT,
		                  "SSG-EG",  song.instrument[active_instr].fm.op[i].SSGMODE & 0x7, 0, 7);

		// Y coordinate for the next operator widget:
		y += OPERATOR_WIDGET_HEIGHT;
	}

	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_program_window_renderer);
}
