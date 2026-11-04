#include <cstdint>
#include <string>
#include "openrac/mod.hpp"
#include "openrac/games/rac1.hpp"
#include "logic.hpp"

namespace {
openrac::Mod* g=nullptr;
openrac::UiPage page;
openrac::rac1::Save_RequestAutosave::fn request_autosave=nullptr;
std::uint16_t previous=0;
bool open_requested=false;
bool pending_save=false;

int slot_count(){
    for(int i=0;i<16;++i){
        openrac_save_info info{};
        if(g->save_slot(i,info)<0) return i;
    }
    return 16;
}
}

void load(openrac::Mod& mod){
    if(mod.game()!="rac1" || !mod.has_play()) throw openrac::Error("Save Manager requires RAC1 play API");
    g=&mod;
    if(mod.has_api_11()){
        request_autosave=mod.required_function<openrac::rac1::Save_RequestAutosave::fn>(
            openrac::rac1::Save_RequestAutosave::name
        );
    }

    page=mod.ui_page([&mod](const openrac::UiPage& p,const openrac::UiEvent& event){
        openrac::UiContent c;
        c.title="Save Manager";
        const int count=slot_count();
        for(int i=0;i<count;++i){
            openrac_save_info info{};
            const int state=mod.save_slot(i,info);
            std::string text="Slot "+std::to_string(i+1)+": ";
            if(state==1){
                text+=mod.level_name(info.level)+" | "+std::to_string(info.bolts)+" bolts | "
                     +std::to_string(info.gold_bolts)+" gold";
            }else{
                text+="empty";
            }
            if(mod.has_api_11() && mod.save_slot_current()==i) text+=" | ACTIVE";
            c.items.push_back(openrac::UiItem::text(text));
        }

        const bool can_save=mod.has_api_11() && request_autosave!=nullptr && mod.save_slot_current()>=0;
        const int save_item=static_cast<int>(c.items.size());
        c.items.push_back(openrac::UiItem::button("Save active slot",can_save));
        c.footer={
            mod.has_api_11() ? "Uses the game's active save slot" : "Manual save requires OpenRAC API 1.11+",
            "L2 + R2 + Select opens this page"
        };
        p.set(c);

        if(event.is(OPENRAC_UI_CHOOSE) && event.item()==save_item && can_save){
            pending_save=true;
            p.back();
        }
    });

    mod.on_pad([](int port,std::uint8_t* report,int size){
        if(port!=0 || report==nullptr || size<2) return;
        const auto now=qol::pad_buttons(report);
        const auto mask=static_cast<std::uint16_t>(OPENRAC_PAD_L2|OPENRAC_PAD_R2|OPENRAC_PAD_SELECT);
        if(qol::combo_pressed(now,previous,mask)) open_requested=true;
        if((now&mask)==mask) qol::consume_buttons(report,mask);
        previous=now;
    });

    mod.on_frame([]{
        if(open_requested){
            open_requested=false;
            page.open();
        }
        if(pending_save && g->play_status().phase==OPENRAC_PLAY_PLAYING){
            pending_save=false;
            request_autosave(0,-1);
        }
    });
}
OPENRAC_MOD_CPP("save-manager","0.2.0",load)
