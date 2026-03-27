#include "screens/menu.hpp"
#include "app.hpp"
#include "banner.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component make_menu_screen(AppState& state)
{
    static const std::vector<std::string> entries = {
        "  Take Quiz",
        "  Add Question",
        "  Remove Question",
        "  Change Answer",
        "  Edit Choice",
        "  List Questions",
        "  Set Author and Name",
        "  Save and Exit",
        "  Quit without Saving",
    };

    auto menu = Menu(&entries, &state.menu_selected);
    auto component = Container::Vertical({menu});

    component |= CatchEvent([&](Event event) {
        if (event.is_character())
        {
            char ch = event.character()[0];
            if (ch >= '1' && ch <= '9')
                state.menu_selected = ch - '1';
            else
                return false;
        }

        if (event == Event::Return ||
            (event.is_character() && event.character()[0] >= '1' && event.character()[0] <= '9'))
        {
            switch (state.menu_selected)
            {
                case 0:
                    if (state.questions.empty())
                        state.status_message = "No questions loaded.";
                    else
                        state.start_quiz();
                    return true;
                case 1:
                    state.reset_add_form();
                    state.current_screen = AppScreen::ADD_QUESTION;
                    return true;
                case 2:
                    if (state.questions.empty())
                        state.status_message = "No questions to remove.";
                    else
                    {
                        state.select_question_idx = 0;
                        state.current_screen = AppScreen::REMOVE_QUESTION;
                    }
                    return true;
                case 3:
                    if (state.questions.empty())
                        state.status_message = "No questions available.";
                    else
                    {
                        state.select_question_idx = 0;
                        state.select_new_answer = 0;
                        state.change_answer_phase = 0;
                        state.current_screen = AppScreen::CHANGE_ANSWER;
                    }
                    return true;
                case 4:
                    if (state.questions.empty())
                        state.status_message = "No questions available.";
                    else
                    {
                        state.edit_choice_question_idx = 0;
                        state.edit_choice_choice_idx = 0;
                        state.edit_choice_phase = 0;
                        state.current_screen = AppScreen::EDIT_CHOICE;
                    }
                    return true;
                case 5:
                    if (state.questions.empty())
                        state.status_message = "No questions available.";
                    else
                    {
                        state.list_selected = 0;
                        state.current_screen = AppScreen::LIST_QUESTIONS;
                    }
                    return true;
                case 6:
                    state.meta_name_text = state.quiz_name;
                    state.meta_author_text = state.quiz_author;
                    state.current_screen = AppScreen::SET_METADATA;
                    return true;
                case 7:
                    state.compute_diff();
                    state.current_screen = AppScreen::SAVE_CONFIRM;
                    return true;
                case 8:
                    state.compute_diff();
                    state.current_screen = AppScreen::QUIT_CONFIRM;
                    return true;
            }
        }
        return false;
    });

    return Renderer(component, [&, menu] {
        bool has_changes = (state.questions != state.saved_questions)
                        || (state.quiz_name != state.saved_quiz_name)
                        || (state.quiz_author != state.saved_quiz_author);

        auto info_bar = hbox({
            text(" Randomise: ") | dim,
            state.randomise
                ? text(" ON ") | color(Color::Green) | bold
                : text(" OFF ") | dim,
            text("  ") | dim, separator(), text("  ") | dim,
            text(std::to_string(state.questions.size())) | bold,
            text(" questions") | dim,
            text("  ") | dim, separator(), text("  ") | dim,
            text(state.filename) | dim,
            filler(),
            text(has_changes ? " modified " : "") | color(Color::Yellow) | dim,
        });

        Elements content;
        content.push_back(text(""));
        content.push_back(render_banner());
        content.push_back(text(""));
        content.push_back(separator() | color(Color::GrayDark));
        content.push_back(text(""));
        content.push_back(menu->Render() | vscroll_indicator | frame | flex);
        content.push_back(text(""));
        content.push_back(separator() | color(Color::GrayDark));
        content.push_back(info_bar);

        if (!state.status_message.empty())
        {
            content.push_back(separator() | color(Color::GrayDark));
            content.push_back(text(" " + state.status_message + " ") | color(Color::Yellow));
        }

        content.push_back(separator() | color(Color::GrayDark));
        content.push_back(text(" 1-9 select  R randomise  Enter confirm ") | dim | center);

        return vbox(std::move(content)) | borderRounded;
    });
}
