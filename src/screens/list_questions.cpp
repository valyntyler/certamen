#include "screens/list_questions.hpp"
#include "app.hpp"
#include "syntax.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component make_list_questions_screen(AppState& state)
{
    auto answer_toggle  = Checkbox(" Answers", &state.list_show_answers);
    auto code_toggle    = Checkbox(" Code", &state.list_show_code);
    auto explain_toggle = Checkbox(" Explain", &state.list_show_explain);

    auto back_btn = Button(" Back ", [&] {
        state.current_screen = AppScreen::MENU;
        state.status_message.clear();
    }, ButtonOption::Simple());

    auto controls = Container::Horizontal({answer_toggle, code_toggle, explain_toggle, back_btn});

    auto component = Container::Vertical({controls});

    component |= CatchEvent([&](Event event) {
        if (event == Event::Character('j') || event == Event::ArrowDown)
        {
            if (state.list_selected < static_cast<int>(state.questions.size()) - 1)
                state.list_selected++;
            return true;
        }
        if (event == Event::Character('k') || event == Event::ArrowUp)
        {
            if (state.list_selected > 0) state.list_selected--;
            return true;
        }
        if (event == Event::Character('b') || event == Event::Escape)
        {
            state.current_screen = AppScreen::MENU;
            state.status_message.clear();
            return true;
        }
        return false;
    });

    return Renderer(component, [&, answer_toggle, code_toggle, explain_toggle, back_btn] {
        // Left panel: question list
        Elements list_entries;
        for (std::size_t i = 0; i < state.questions.size(); ++i)
        {
            bool selected = (static_cast<int>(i) == state.list_selected);
            auto entry = hbox({
                text(selected ? " > " : "   "),
                text(std::to_string(i + 1) + ". ") | dim,
                text(state.questions[i].question) | (selected ? bold : nothing),
                state.questions[i].code
                    ? text(" [code]") | color(Color::Cyan) | dim
                    : text(""),
                state.questions[i].explain
                    ? text(" [exp]") | color(Color::Yellow) | dim
                    : text(""),
            });
            if (selected) entry = entry | color(Color::Cyan);
            list_entries.push_back(entry);
        }

        auto list_panel = vbox(std::move(list_entries))
            | vscroll_indicator | frame | flex;

        // Right panel: detail
        Elements detail;
        if (state.list_selected >= 0 &&
            state.list_selected < static_cast<int>(state.questions.size()))
        {
            const auto& q = state.questions[state.list_selected];

            detail.push_back(paragraph(q.question) | bold);

            if (state.list_show_code && q.code && !q.code->empty())
            {
                detail.push_back(text(""));
                detail.push_back(render_code_block(*q.code, q.language));
            }

            detail.push_back(text(""));
            for (std::size_t j = 0; j < q.choices.size(); ++j)
            {
                bool correct = (static_cast<int>(j) == q.answer);
                auto line = hbox({
                    text("  " + std::to_string(j + 1) + ". "),
                    text(q.choices[j])
                        | (state.list_show_answers && correct ? color(Color::Green) : nothing),
                    text(state.list_show_answers && correct ? "  correct" : "") | dim,
                });
                detail.push_back(line);
            }

            if (state.list_show_explain && q.explain && !q.explain->empty())
            {
                detail.push_back(text(""));
                detail.push_back(paragraph(" " + *q.explain) | dim);
            }
        }
        else
        {
            detail.push_back(text(" Select a question ") | dim | center);
        }

        auto detail_panel = vbox(std::move(detail))
            | vscroll_indicator | frame | flex
            | size(HEIGHT, GREATER_THAN, 15);

        auto toggles_bar = hbox({
            answer_toggle->Render(),
            text("  "),
            code_toggle->Render(),
            text("  "),
            explain_toggle->Render(),
            filler(),
            back_btn->Render() | color(Color::RedLight),
        });

        return vbox({
            text(""),
            text(" Questions ") | bold | center,
            text(""),
            separator() | color(Color::GrayDark),
            toggles_bar,
            separator() | color(Color::GrayDark),
            hbox({
                list_panel | size(WIDTH, EQUAL, 42),
                separator() | color(Color::GrayDark),
                detail_panel | flex,
            }) | flex,
            separator() | color(Color::GrayDark),
            text(" j/k navigate  Tab toggles  b back ") | dim | center,
        }) | borderRounded;
    });
}
