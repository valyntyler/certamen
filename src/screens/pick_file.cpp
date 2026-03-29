#include "screens/pick_file.hpp"
#include "app.hpp"
#include "nav.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

ftxui::Component make_pick_file_screen(AppState& state)
{
    auto focusable = Renderer([](bool) { return text(""); });

    auto component = CatchEvent(focusable, [&](Event event) {
        int count = static_cast<int>(state.loaded_files.size());
        if (count == 0) return false;

        if (nav_up_down(event, state.pick_file_cursor, count)) return true;
        if (nav_numeric(event, state.pick_file_cursor, count)) return true;

        if (event == Event::Return)
        {
            state.target_file = state.pick_file_cursor;
            state.build_target_indices();

            if (state.pick_file_then == AppScreen::SET_METADATA)
            {
                auto& lf = state.loaded_files[state.target_file];
                state.meta_name_text = lf.name;
                state.meta_author_text = lf.author;
            }
            if (state.pick_file_then == AppScreen::SAVE_CONFIRM)
                state.compute_diff(state.target_file);

            state.current_screen = state.pick_file_then;
            return true;
        }

        if (event == Event::Escape)
        {
            state.return_to_menu();
            return true;
        }

        return false;
    });

    return Renderer(component, [&] {
        Elements body;
        body.push_back(text(""));
        body.push_back(text(" Which file? ") | bold | center);
        body.push_back(text(""));
        body.push_back(separator() | color(Color::GrayDark));
        body.push_back(text(""));

        for (int i = 0; i < static_cast<int>(state.loaded_files.size()); ++i)
        {
            bool sel = (i == state.pick_file_cursor);
            const auto& lf = state.loaded_files[i];

            int qcount = 0;
            for (const auto& q : state.questions)
                if (q.source_file == i) qcount++;

            auto entry = hbox({
                text(sel ? " > " : "   "),
                text(std::to_string(i + 1) + ". ") | dim,
                text(lf.filename) | (sel ? bold : nothing),
                text("  " + std::to_string(qcount) + "q") | dim,
            });
            if (sel) entry = entry | color(Color::Cyan);
            body.push_back(entry);
        }

        body.push_back(text(""));
        body.push_back(filler());
        body.push_back(separator() | color(Color::GrayDark));
        body.push_back(text(" j/k navigate  Enter select  Esc back ") | dim | center);

        return vbox(std::move(body)) | borderRounded;
    });
}
