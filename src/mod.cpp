#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "logic.hpp"

namespace {
struct SaveRequestAutosave {
    static constexpr const char* id="rac1.save.request_autosave";
    static constexpr const char* name="Save_RequestAutosave";
    static constexpr const char* signature="i32(i32,i32)";
    using fn=std::int32_t(*)(std::int32_t,std::int32_t);
};
openrac::Mod* g=nullptr;
std::uint16_t previous=0;
int selected=1;
int action=0;
void show_slot(){
    openrac_save_info info{};
    const int state=g->save_slot(selected-1,info);
    std::string text="Save slot "+std::to_string(selected)+": ";
    if(state==1) text+=g->level_name(info.level)+" | "+std::to_string(info.bolts)+" bolts";
    else text+=state==0 ? "empty" : "unavailable";
    if(g->has_notice()) g->play_notice(text);
}
}
void load(openrac::Mod& mod){
    if(mod.game()!="rac1" || !mod.has_play()) throw openrac::Error("Save Manager requires RAC1 play API");
    g=&mod;
    selected=static_cast<int>(mod.setting<std::int64_t>("default_slot"));
    if(selected<1 || selected>8) selected=1;
    mod.hook<SaveRequestAutosave>([](auto original,std::int32_t slot,std::int32_t flags){ return original(slot,flags); });
    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto left=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_LEFT);
        const auto right=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_RIGHT);
        const auto save=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_SQUARE);
        const auto info=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_TRIANGLE);
        if(qol::combo_pressed(now,previous,left)) action=-1;
        if(qol::combo_pressed(now,previous,right)) action=1;
        if(qol::combo_pressed(now,previous,save)) action=2;
        if(qol::combo_pressed(now,previous,info)) action=3;
        previous=now;
    });
    mod.on_frame([]{
        if(action==-1 || action==1){ selected=qol::wrap_slot(selected,action); action=0; show_slot(); return; }
        if(action==3){ action=0; show_slot(); return; }
        if(action==2){
            action=0;
            const auto s=g->play_status();
            auto fn=g->original<SaveRequestAutosave>();
            if(fn!=nullptr && s.phase==OPENRAC_PLAY_PLAYING){
                const auto result=fn(selected-1,0);
                if(g->has_notice()) g->play_notice(result>=0 ? "Quick save requested for slot "+std::to_string(selected) : "Quick save failed");
            } else if(g->has_notice()) g->play_notice("Quick save is available while playing");
        }
    });
    mod.info("L2+R2+Left/Right slot; Square save; Triangle inspect");
}
OPENRAC_MOD_CPP("save-manager","0.1.0",load)
