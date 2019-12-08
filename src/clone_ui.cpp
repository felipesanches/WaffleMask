#include "clone_ui.h"

CloneUI::CloneUI(){
	SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_RESIZABLE,
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

	SDL_RenderPresent(m_program_window_renderer);
}
