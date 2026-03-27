#include "screens/remove_question.hpp"
#include "app.hpp"
#include "nav.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component make_remove_question_screen(AppState& state)
{
    auto component = CatchEvent(Renderer([](bool) { return text(""); }), [&](Event event) {
        if (state.questions.empty()) return false;
        int count = static_cast<int>(state.questions.size());
        if (nav_up_down(event, state.remove_question_idx, count)) return true;
        if (nav_numeric(event, state.remove_question_idx, count)) return true;
        if (event == Event::Return)
        {
            state.questions.erase(state.questions.begin() + state.remove_question_idx);
            state.status_message = "Question removed.";
            if (state.remove_question_idx >= static_cast<int>(state.questions.size()) &&
                !state.questions.empty())
            {
                state.remove_question_idx = static_cast<int>(state.questions.size()) - 1;
            }
            if (state.questions.empty())
                state.current_screen = AppScreen::MENU;
            return true;
        }
        if (event == Event::Escape || event == Event::Character('b'))
        {
            state.return_to_menu();
            return true;
        }
        return false;
    });

    return Renderer(component, [&] {
        if (state.questions.empty())
            return text(" No questions. ") | center | borderRounded;

        Elements body;
        body.push_back(text(""));
        body.push_back(text(" Remove Question ") | bold | center);
        body.push_back(text(""));
        body.push_back(separator() | color(Color::GrayDark));
        body.push_back(text(""));

        for (int i = 0; i < static_cast<int>(state.questions.size()); ++i)
        {
            bool sel = (i == state.remove_question_idx);
            auto entry = hbox({
                text(sel ? " > " : "   "),
                text(std::to_string(i + 1) + ". ") | dim,
                text(state.questions[i].question) | (sel ? bold : nothing),
                state.questions[i].code
                    ? text(" [code]") | color(Color::Cyan) | dim
                    : text(""),
                state.questions[i].explain
                    ? text(" [exp]") | color(Color::Yellow) | dim
                    : text(""),
            });
            if (sel) entry = entry | color(Color::Cyan);
            body.push_back(entry);
        }

        body.push_back(text(""));
        body.push_back(separator() | color(Color::GrayDark));
        body.push_back(text(" j/k navigate  Enter remove  Esc back ") | dim | center);

        return vbox(std::move(body)) | vscroll_indicator | frame | flex | borderRounded;
    });
}
