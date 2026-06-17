#include "App.hpp"
#include "Core/Context.hpp"

int main(int, char**) {
    auto context = Core::Context::GetInstance();
    Util::Logger::Init();
    Util::Logger::SetLevel(Util::Logger::Level::INFO);
    App app;

    while (!context->GetExit()) {
        switch (app.GetCurrentState()) {
            case App::State::START:
                app.Start();
                break;

            // ==========================================
            // Stack all active game states right here!
            // ==========================================
            case App::State::UPDATE:
            case App::State::DIALOGUE:
            case App::State::START_MENU:
            case App::State::POKEMON_MENU:
            case App::State::INVENTORY_MENU:
            case App::State::BATTLE:
            case App::State::SHOP:
            case App::State::TITLE:
                app.Update();
                break;

            case App::State::END:
                app.End();
                context->SetExit(true);
                break;
        }
        context->Update();
    }
    return 0;
}