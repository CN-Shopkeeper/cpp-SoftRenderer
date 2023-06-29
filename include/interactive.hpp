#pragma once

#include <chrono>
#include <string>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

class App {
   public:
    App(const char* title, int w, int h) : title_(title) {
        SDL_Init(SDL_INIT_EVERYTHING);
        if (TTF_Init() != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                         "SDL_ttf init failed: ", TTF_GetError());
        }
        if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
            SDL_LogError(SDL_LOG_CATEGORY_SYSTEM,
                         "SDL_image init failed: ", IMG_GetError());
        }
        font_ = TTF_OpenFont("resources/font/simsun.ttc", 20);
        if (!font_) {
            SDL_Log("font load failed");
        }
        window_ =
            SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
        if (!window_) {
            SDL_Log("can't create window");
        }
        renderer_ = SDL_CreateRenderer(window_, -1, 0);
        if (!renderer_) {
            SDL_Log("can't create SDL renderer");
        }
        SDL_Log("SDL init OK");
    }

    virtual ~App() {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        IMG_Quit();
        TTF_CloseFont(font_);
        TTF_Quit();
        SDL_Quit();
    }

    void Run() {
        OnInit();
        auto t = std::chrono::high_resolution_clock::now();
        SDL_Log("start app");
        while (!ShouldExit()) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    Exit();
                }
                if (event.type == SDL_KEYDOWN) {
                    OnKeyDown(event.key);
                }
                if (event.type == SDL_KEYUP) {
                    OnKeyUp(event.key);
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    OnMouseDown(event.button);
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    OnMouseUp(event.button);
                }
                if (event.type == SDL_MOUSEMOTION) {
                    OnMotion(event.motion);
                }
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        OnWindowResize(event.window.data1, event.window.data2);
                    }
                }
            }
            auto elapse = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - t);
            t = std::chrono::high_resolution_clock::now();
            SDL_SetWindowTitle(window_,
                               (title_ + "fps: " +
                                std::to_string(int(1000.0 / elapse.count())))
                                   .c_str());
            OnRender();
        }
        OnQuit();
    }

    void Exit() { isQuit_ = true; }
    bool ShouldExit() const { return isQuit_; }

    SDL_Window* GetWindow() const { return window_; }

    void SwapBuffer(SDL_Surface* surface, std::string text = "") {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        // 画面
        if (!texture) {
            SDL_Log("swap buffer failed");
        } else {
            SDL_RenderCopy(renderer_, texture, nullptr, nullptr);
            SDL_DestroyTexture(texture);
        }
        // 文字
        if (!text.empty()) {
            auto textSurface =
                TTF_RenderUTF8_Blended_Wrapped(font_, text.c_str(), {0, 0, 0, 255},600);
            auto textTexture =
                SDL_CreateTextureFromSurface(renderer_, textSurface);
            SDL_Rect rect = {0, 0, textSurface->w, textSurface->h};
            SDL_FreeSurface(textSurface);
            SDL_RenderCopy(renderer_, textTexture, nullptr, &rect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_RenderPresent(renderer_);
    }

    void RenderText(std::string text, SDL_Rect* dstRect) {
        auto surface =
            TTF_RenderUTF8_Blended(font_, text.c_str(), {0, 0, 0, 255});
        auto texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
    }

    virtual void OnInit() {}
    virtual void OnQuit() {}
    virtual void OnRender() {}

    virtual void OnKeyDown(const SDL_KeyboardEvent&) {}
    virtual void OnKeyUp(const SDL_KeyboardEvent&) {}
    virtual void OnMouseDown(const SDL_MouseButtonEvent&) {}
    virtual void OnMouseUp(const SDL_MouseButtonEvent&) {}
    virtual void OnMotion(const SDL_MouseMotionEvent&) {}
    virtual void OnWindowResize(int, int) {}

   private:
    bool isQuit_ = false;
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    std::string title_;
};