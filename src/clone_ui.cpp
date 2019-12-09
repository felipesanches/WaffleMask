#include <string>
#include "clone_ui.h"

#define OPERATOR_WIDGET_WIDTH 500
#define OPERATOR_WIDGET_HEIGHT 160
#define MARGIN 10
#define PADDING 30

CloneUI::CloneUI(){
	SDL_CreateWindowAndRenderer(20 + OPERATOR_WIDGET_WIDTH,
	                            20 + 4*OPERATOR_WIDGET_HEIGHT,
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

#define OPERATOR_GRAPH_AREA_WIDTH 265
#define OPERATOR_GRAPH_AREA_HEIGHT 64

#define HSLIDER_WIDTH ((OPERATOR_GRAPH_AREA_WIDTH - MARGIN)/2)
#define HSLIDER_HEIGHT 10
void CloneUI::horizontal_slider(int x, int y, const char* name, int value, int min_value, int max_value){
	int width = HSLIDER_WIDTH;
	int height = HSLIDER_HEIGHT;
	SDL_Rect rect;

	SDL_Color WHITE = { 255, 255, 255, 0 };
	draw_text(x, y - 2, std::string(name) + std::string(" ") + std::to_string(value), WHITE);
	
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

void CloneUI::draw(){
	SDL_RenderClear(m_program_window_renderer);

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

		// Graph area:
		rect.x = x + 135;
		rect.y = y + 25;
		rect.w = OPERATOR_GRAPH_AREA_WIDTH;
		rect.h = OPERATOR_GRAPH_AREA_HEIGHT;
		SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(m_program_window_renderer, &rect);

		// ADSR sliders:
		vertical_slider(x + 0 * VSLIDER_WIDTH, y, "A",  31, 31);
		vertical_slider(x + 1 * VSLIDER_WIDTH, y, "D",  14, 31);
		vertical_slider(x + 2 * VSLIDER_WIDTH, y, "S",  1,  31);
		vertical_slider(x + 3 * VSLIDER_WIDTH, y, "D2", 7,  31);
		vertical_slider(x + 4 * VSLIDER_WIDTH, y, "R",  15, 15);

		// TL slider:
		vertical_slider(x + 420, y, "TL",  18, 127);

		// Operator number
		SDL_Color YELLOW = { 255, 255, 0, 0 };
		draw_text(x + 135, y + 5, std::string("OPERATOR  ") + std::to_string(i+1), YELLOW);

		// MULT slider:
		horizontal_slider(x + 135, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT, "MULT",  1, 0, 15);

		// DT slider:
		horizontal_slider(x + 135, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT, "DT",  0, -2, 2);

		// RS slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 27 + OPERATOR_GRAPH_AREA_HEIGHT, "RS",  0, 0, 15);

		// SSG-EG slider:
		horizontal_slider(x + 135 + MARGIN + HSLIDER_WIDTH, y + 32 + OPERATOR_GRAPH_AREA_HEIGHT + 2*HSLIDER_HEIGHT, "SSG-EG",  0, 0, 15);

		// Y coordinate for the next operator widget:
		y += OPERATOR_WIDGET_HEIGHT;
	}

	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_program_window_renderer);
}
