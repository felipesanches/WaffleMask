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

#define VSLIDER_WIDTH 20
#define VSLIDER_HEIGHT 150
void CloneUI::vertical_slider(int x, int y, const char* name, int value, int max_value){
	int width = VSLIDER_WIDTH - 6;
	int height = VSLIDER_HEIGHT - 2 * 25;
	SDL_Rect rect;

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
		rect.w = 265;
		rect.h = 64;
		SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(m_program_window_renderer, &rect);

		// ADSR sliders:
		vertical_slider(x + 0 * VSLIDER_WIDTH, y, "A",  31, 31);
		vertical_slider(x + 1 * VSLIDER_WIDTH, y, "D",  14, 31);
		vertical_slider(x + 2 * VSLIDER_WIDTH, y, "S",  1,  31);
		vertical_slider(x + 3 * VSLIDER_WIDTH, y, "D2", 7,  31);
		vertical_slider(x + 4 * VSLIDER_WIDTH, y, "R",  15, 15);

		// Y coordinate for the next operator widget:
		y += OPERATOR_WIDGET_HEIGHT;
	}

	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_program_window_renderer);
}
