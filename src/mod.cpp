#include <algorithm>
#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
openrac::rac1::Save_RequestAutosave::fn request_autosave=nullptr;
std::uint16_t previous=0;
int selected=0;
int action=0;

int slot_count(){
    for(int i=0;i<16;++i){
        openrac_save_info info{};
        if(g->save_slot(i,info)<0) return i;
    }
    return 16;
}

void show_slot(){
    const int count=slot_count();
    if(count<=0){
        if(g->has_notice()) g->play_notice("No save slots available");
        return;
    }
    selected=qol::wrap_index(selected,count,0);
    openrac_save_info info{};
    const int state=g->save_slot(selected,info);
    std::string text="Save slot "+std::to_string(selected+1)+": ";
    if(state==1){
        text+=g->level_name(info.level)+" | "+std::to_string(info.bolts)+" bolts | "
              +std::to_string(info.gold_bolts)+" gold";
    }else{
        text+="empty";
    }
    if(g->has_api_11() && g->save_slot_current()==selected) text+=" | ACTIVE";
    if(g->has_notice()) g->play_notice(text);
}
}

void load(openrac::Mod& mod){
    if(mod.game()!="rac1" || !mod.has_play()) throw openrac::Error("Save Manager requires RAC1 play API");
    g=&mod;
    selected=std::clamp(static_cast<int>(mod.setting<std::int64_t>("default_inspection_slot"))-1,0,4);

    if(mod.has_api_11()){
        request_autosave=mod.required_function<openrac::rac1::Save_RequestAutosave::fn>(
            openrac::rac1::Save_RequestAutosave::name
        );
    }else{
        request_autosave=nullptr;
        mod.info("API 1.10: slot inspection enabled; manual save needs API 1.11+");
    }

    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto previous_slot=static_cast<std::uint16_t>(
            OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_UP
        );
        const auto next_slot=static_cast<std::uint16_t>(
            OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_DOWN
        );
        const auto save=static_cast<std::uint16_t>(
            OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_SQUARE
        );
        const auto info=static_cast<std::uint16_t>(
            OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_SELECT
        );
        if(qol::combo_pressed(now,previous,previous_slot)) action=-1;
        if(qol::combo_pressed(now,previous,next_slot)) action=1;
        if(qol::combo_pressed(now,previous,save)) action=2;
        if(qol::combo_pressed(now,previous,info)) action=3;
        previous=now;
    });

    mod.on_frame([]{
        if(action==-1 || action==1){
            const int count=slot_count();
            if(count>0) selected=qol::wrap_index(selected,count,action);
            action=0;
            show_slot();
            return;
        }
        if(action==3){
            action=0;
            show_slot();
            return;
        }
        if(action!=2) return;
        action=0;

        const auto status=g->play_status();
        if(status.phase!=OPENRAC_PLAY_PLAYING){
            if(g->has_notice()) g->play_notice("Manual save is available while playing");
            return;
        }
        if(!g->has_api_11()){
            if(g->has_notice()) g->play_notice("Manual save requires OpenRAC interface 1.11+");
            return;
        }
        const int active=g->save_slot_current();
        if(active<0){
            if(g->has_notice()) g->play_notice("No active save slot - use the normal Save Game flow first");
            return;
        }
        if(request_autosave==nullptr){
            if(g->has_notice()) g->play_notice("Save function unavailable");
            return;
        }
        request_autosave(0,-1);
        if(g->has_notice()) g->play_notice("Save requested for active slot "+std::to_string(active+1));
    });

    mod.info("L2+R2+Up/Down inspect slots; Square saves ACTIVE slot; Select shows info");
}
OPENRAC_MOD_CPP("save-manager","0.1.1",load)
