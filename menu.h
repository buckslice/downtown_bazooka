#pragma once

// this class basically holds all of the UI stuff for now

#include "SFML/Graphics.hpp"
#include "player.h"
#include "resources.h"
#include <string>
#include <queue>

#define MAX_NUMBER_OF_ITEMS 3
#define DEFAULT_COLOR sf::Color(0,128,64)
#define TITLE_COLOR sf::Color(255, 128, 0)
#define SELECTED_COLOR sf::Color(0,255,128)

class TextOption {
    std::string Text;
    sf::Font &Font;
    sf::Text *Characters;
    sf::Vector2f Position;
    sf::Vector2f *DesiredOffsets;
    bool IsSelected;
public:
    TextOption() : TextOption("BLAH") {}
    TextOption(std::string text) : Text(text), Font(Resources::get().font) {
        int length = Text.length();
        Characters = new sf::Text[Text.length()];
        DesiredOffsets = new sf::Vector2f[length];
        for (int i = 0; i < length; i++) {
            Characters[i] = sf::Text(std::string(1, text[i]), Font, 80U);
            Characters[i].setPosition(sf::Vector2f(Mth::randRange(0.0f, 1200.0f), Mth::randRange(0.0f, 1200.0f)));
        }
        SetIsSelected(false);
    }
    ~TextOption() {
        delete[] Characters;
        delete[] DesiredOffsets;
    }

    void SetIsSelected(bool b) {
        IsSelected = b;
        int length = Text.length();
        for (int i = 0; i < length; i++) {
            Characters[i].setColor(IsSelected ? SELECTED_COLOR : DEFAULT_COLOR);
            DesiredOffsets[i] = IsSelected ? sf::Vector2f() : sf::Vector2f(Mth::randRange(-20.0f, 20.0f), Mth::randRange(-20.0f, 20.0f));
        }
    }

    void draw(sf::RenderWindow& window, sf::Vector2f basepos) {
        Position = basepos;
        int length = Text.length();
        for (int i = 0; i < length; i++) {
            sf::Vector2f desiredpos = Position + DesiredOffsets[i] + sf::Vector2f((i - length / 2)*45.0f, 0);
            Characters[i].setPosition(Characters[i].getPosition()*.90f + desiredpos*.10f);
            window.draw(Characters[i]);
        }
    }
};

class Menu {
public:
    Menu(Player* player);
    ~Menu();

    void draw(sf::RenderWindow& window, bool showFPS);
    bool update(GLfloat delta);
    void setVisible(bool visible);
    bool getVisible() const;

    static bool justClosed();
    static bool justOpened();

private:
    void move(bool up);

    TextOption **menu;

    sf::Text title;
    sf::Text instructions;
    sf::RectangleShape healthBar;
    sf::RectangleShape bossHealthBar;
    sf::RectangleShape overlay;

    sf::Texture titleTexture;
    sf::Sprite titleSprite;

    sf::Text fpsText;
    sf::Text deadText;
    std::queue<float> fpsValues;
    float totalFpsQueueValue = 0.0f;

    void updateFpsText(float delta);

    Player* player;
    int curSelection;
    bool visible = true;
    bool showingInstructions = false;

};

