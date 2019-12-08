#pragma once

#include <SDL2/SDL.h>

class CloneUI
{
public:
    CloneUI();
    ~CloneUI() = default;

    void program_loop();
    void update(double delta_time);
    void draw();

private:
    void vertical_slider(int x, int y, const char* name, int value, int max_value);
    SDL_Window   *m_program_window;
    SDL_Event     m_program_window_event;
    SDL_Renderer *m_program_window_renderer;
};
