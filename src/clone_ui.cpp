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

	for (int i=1; i<4; i++){
		// Separator line between the operator widgets:
		SDL_SetRenderDrawColor(m_program_window_renderer, 48, 198, 98, 255);
		SDL_RenderDrawLine(m_program_window_renderer,
		                                           MARGIN + PADDING,     MARGIN + i*OPERATOR_WIDGET_HEIGHT,
		                   MARGIN + OPERATOR_WIDGET_WIDTH - PADDING,     MARGIN + i*OPERATOR_WIDGET_HEIGHT);		
	}

	// Render operators:
	for (int i=0; i<4; i++){
		// Graph area:
		rect.x = MARGIN + 175;
		rect.y = MARGIN + i*OPERATOR_WIDGET_HEIGHT + 25;
		rect.w = 265;
		rect.h = 64;
		SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(m_program_window_renderer, &rect);
	}

	SDL_SetRenderDrawColor(m_program_window_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_program_window_renderer);
}
