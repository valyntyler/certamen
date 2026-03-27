#include "screens/change_answer.hpp"
#include "app.hpp"
#include "syntax.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component make_change_answer_screen(AppState& state)
{
    auto focusable = Renderer([](bool) { return text(""); });

    auto component = CatchEvent(focusable, [&](Event event) {
        if (state.questions.empty()) return false;

        if (state.change_answer_phase == 0)
        {
            int count = static_cast<int>(state.questions.size());
            if (event == Event::ArrowUp || event == Event::Character('k'))
            {
                if (state.select_question_idx > 0) state.select_question_idx--;
                return true;
            }
            if (event == Event::ArrowDown || event == Event::Character('j'))
            {
                if (state.select_question_idx < count - 1) state.select_question_idx++;
                return true;
            }
            if (event.is_character())
            {
                char ch = event.character()[0];
                int num = ch - '1';
                if (num >= 0 && num < count)
                {
                    state.select_question_idx = num;
                    return true;
                }
            }
            if (event == Event::Return)
            {
                state.change_answer_phase = 1;
                state.select_new_answer = state.questions[state.select_question_idx].answer;
                return true;
            }
        }
        else
        {
            const auto& q = state.questions[state.select_question_idx];
            int num_choices = static_cast<int>(q.choices.size());

            if (event == Event::ArrowUp || event == Event::Character('k'))
            {
                if (state.select_new_answer > 0) state.select_new_answer--;
                return true;
            }
            if (event == Event::ArrowDown || event == Event::Character('j'))
            {
                if (state.select_new_answer < num_choices - 1) state.select_new_answer++;
                return true;
            }
            if (event.is_character())
            {
                char ch = event.character()[0];
                int num = ch - '1';
                if (num >= 0 && num < num_choices)
                {
                    state.select_new_answer = num;
                    return true;
                }
            }
            if (event == Event::Return)
            {
                state.questions[state.select_question_idx].answer = state.select_new_answer;
                state.status_message = "Answer updated.";
                state.change_answer_phase = 0;
                return true;
            }
        }

        if (event == Event::Escape || event == Event::Character('b'))
        {
            if (state.change_answer_phase == 1)
                state.change_answer_phase = 0;
            else
            {
                state.current_screen = AppScreen::MENU;
                state.status_message.clear();
            }
            return true;
        }

        return false;
    });

    return Renderer(component, [&] {
        if (state.questions.empty())
            return text(" No questions. ") | center | borderRounded;

        Elements body;

        if (state.change_answer_phase == 0)
        {
            body.push_back(text(""));
            body.push_back(text(" Change Answer ") | bold | center);
            body.push_back(text(""));
            body.push_back(separator() | color(Color::GrayDark));
            body.push_back(text(""));

            for (int i = 0; i < static_cast<int>(state.questions.size()); ++i)
            {
                bool sel = (i == state.select_question_idx);
                const auto& q = state.questions[i];
                std::string answer_hint = "  [" +
                    std::to_string(q.answer + 1) + ": " +
                    q.choices[q.answer] + "]";
                auto entry = hbox({
                    text(sel ? " > " : "   "),
                    text(std::to_string(i + 1) + ". ") | dim,
                    text(q.question) | (sel ? bold : nothing),
                    text(answer_hint) | dim | color(Color::Green),
                });
                if (sel) entry = entry | color(Color::Cyan);
                body.push_back(entry);
            }

            body.push_back(text(""));
            body.push_back(separator() | color(Color::GrayDark));
            body.push_back(text(" j/k navigate  Enter select  Esc back ") | dim | center);
        }
        else
        {
            const auto& q = state.questions[state.select_question_idx];

            body.push_back(text(""));
            body.push_back(text(" Change Answer ") | bold | center);
            body.push_back(text(""));
            body.push_back(separator() | color(Color::GrayDark));
            body.push_back(text(""));
            body.push_back(paragraph(" " + q.question) | bold);

            if (q.code && !q.code->empty())
            {
                body.push_back(text(""));
                body.push_back(render_code_block(*q.code, q.language));
            }

            body.push_back(text(""));

            for (int i = 0; i < static_cast<int>(q.choices.size()); ++i)
            {
                bool is_current = (i == q.answer);
                bool is_new = (i == state.select_new_answer);
                std::string marker = is_new ? " > " : "   ";

                auto choice_el = hbox({
                    text(marker),
                    text(std::to_string(i + 1) + ". "),
                    text(q.choices[i]) | (is_new ? bold : nothing),
                    text(is_current ? "  (current)" : "") | dim,
                });
                if (is_new) choice_el = choice_el | color(Color::Cyan);
                else choice_el = choice_el | dim;
                body.push_back(choice_el);
            }

            body.push_back(text(""));
            body.push_back(separator() | color(Color::GrayDark));
            body.push_back(text(" j/k navigate  Enter save  Esc back ") | dim | center);
        }

        return vbox(std::move(body)) | vscroll_indicator | frame | flex | borderRounded;
    });
}
