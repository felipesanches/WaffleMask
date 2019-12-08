#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
    void draw_text(int x, int y, std::string text_str, SDL_Color textColor);
    TTF_Font     *m_font;
    SDL_Window   *m_program_window;
    SDL_Event     m_program_window_event;
    SDL_Renderer *m_program_window_renderer;
};
