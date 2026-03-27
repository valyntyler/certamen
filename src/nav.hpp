#ifndef CERTAMEN_NAV_HPP
#define CERTAMEN_NAV_HPP

#include <ftxui/component/event.hpp>

inline bool nav_up_down(const ftxui::Event& event, int& selected, int count)
{
    if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('k'))
    {
        if (selected > 0) selected--;
        return true;
    }
    if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('j'))
    {
        if (selected < count - 1) selected++;
        return true;
    }
    return false;
}

inline bool nav_numeric(const ftxui::Event& event, int& selected, int count)
{
    if (!event.is_character()) return false;
    char ch = event.character()[0];
    int num = ch - '1';
    if (num >= 0 && num < count)
    {
        selected = num;
        return true;
    }
    return false;
}

#endif
