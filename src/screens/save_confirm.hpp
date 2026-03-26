#ifndef QUIZZER_SCREEN_SAVE_CONFIRM_HPP
#define QUIZZER_SCREEN_SAVE_CONFIRM_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
struct AppState;

ftxui::Component make_save_confirm_screen(AppState& state, ftxui::ScreenInteractive& screen);

#endif
