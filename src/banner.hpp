#ifndef CERTAMEN_BANNER_HPP
#define CERTAMEN_BANNER_HPP

#include <ftxui/dom/elements.hpp>

inline ftxui::Element render_banner()
{
    using namespace ftxui;
    return vbox({
        text(R"(  .,-::::: .,:::::: :::::::.. :::::::::::::::.     .        :  .,:::::::::.    :::.)") | dim,
        text(R"(,;;;'````' ;;;;'''' ;;;;``;;;;;;;;;;;;'''';;`;;    ;;,.    ;;; ;;;;''''`;;;;,  `;;;)") | dim,
        text(R"([[[         [[cccc   [[[,/[[['     [[    ,[[ '[[,  [[[[, ,[[[[, [[cccc   [[[[[. '[[)") | dim,
        text(R"($$$         $$""""   $$$$$$c       $$   c$$$cc$$$c $$$$$$$$"$$$ $$""""   $$$ "Y$c$$)") | dim,
        text(R"(`88bo,__,o, 888oo,__ 888b "88bo,   88,   888   888,888 Y88" 888o888oo,__ 888    Y88)") | dim,
        text(R"(  "YUMMMMMP"""""YUMMMMMMM   "W"    MMM   YMM   ""` MMM  M'  "MMM""""YUMMMMMM     YM)") | dim,
    }) | center;
}

#endif
