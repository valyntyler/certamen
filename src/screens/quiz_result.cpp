#include "screens/quiz_result.hpp"
#include "app.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

ftxui::Component make_quiz_result_screen(AppState& state)
{
    auto back_btn = Button(" Back to Menu ", [&] {
        state.return_to_menu();
    }, ButtonOption::Simple());

    auto component = CatchEvent(back_btn, [&](Event event) {
        if (event == Event::Escape || event == Event::Character('q'))
        {
            state.return_to_menu();
            return true;
        }
        return false;
    });

    return Renderer(component, [&, back_btn] {
        int total = static_cast<int>(state.quiz_session.size());
        double pct = total > 0
            ? 100.0 * static_cast<double>(state.quiz_score) / static_cast<double>(total)
            : 0.0;

        std::ostringstream pct_str;
        pct_str << std::fixed << std::setprecision(1) << pct << "%";

        Color score_color = pct >= 70.0 ? Color::Green
                          : pct >= 40.0 ? Color::Yellow
                          : Color::RedLight;

        float gauge_val = total > 0
            ? static_cast<float>(state.quiz_score) / static_cast<float>(total)
            : 0.0f;

        return vbox({
            text(""),
            text(""),
            text(" Quiz Complete ") | bold | center,
            text(""),
            separator() | color(Color::GrayDark),
            text(""),
            hbox({
                filler(),
                text(std::to_string(state.quiz_score)) | bold | color(score_color),
                text(" / " + std::to_string(total)) | dim,
                text("  " + pct_str.str()) | color(score_color),
                filler(),
            }),
            text(""),
            gauge(gauge_val) | color(score_color),
            text(""),
            separator() | color(Color::GrayDark),
            text(""),
            back_btn->Render() | center,
            text(""),
        }) | borderRounded;
    });
}
