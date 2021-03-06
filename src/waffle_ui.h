#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "dmf.h"

#define VERTICAL_SLIDER 0
#define HORIZONTAL_SLIDER 1
struct UI_Item {
	int type; // 0 = vertical / 1 = horizontal
	SDL_Rect area;
	int value_min;
	int value_max;
	uint8_t* value;
};

class WaffleUI
{
public:
    WaffleUI();
    ~WaffleUI() = default;

    void program_loop();
    void update(double delta_time);
    void draw();

private:
    void vertical_slider(int x, int y, const char* name, uint8_t* value, int max_value);
    void horizontal_slider(int x, int y, const char* name, uint8_t* value, int min_value, int max_value);
    void draw_instrument_dialog(int x, int y);
    void draw_text(int x, int y, std::string text_str, SDL_Color textColor);
    void draw_fm_operator_graph(int x, int y, DMF::Instrument::FM_Operator op);
    void draw_fm_algorithm_icon(int x, int y, int alg);
    void draw_single_operator_block_icon(int num, int x, int y, int extra_x);
    void draw_waveform_viewer(int x, int y);
    void register_slider(int type, SDL_Rect rect, int value_min, int value_max, uint8_t* value);
    void update_slider(int x, int y);
    int get_note_from_keyboard(SDL_Keycode keysym);

    int instrument_dialog_yscroll;
    bool ui_needs_update;
    int grabbed_item;
    std::vector<UI_Item> ui_items;
    TTF_Font     *m_font;
    SDL_Window   *m_program_window;
    SDL_Event     m_program_window_event;
    SDL_Renderer *m_program_window_renderer;
};
