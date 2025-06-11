#pragma once
#include <SFML/Graphics.hpp>

namespace BayouBonanza {

class Animation {
public:
    Animation();
    bool load(const std::string& sheetPath, int frameWidth, int frameHeight, int frameCount, float frameTime);
    void update(float dt);
    void reset();
    const sf::Sprite& getSprite() const { return sprite; }
private:
    sf::Texture texture;
    sf::Sprite sprite;
    int frameWidth{0};
    int frameHeight{0};
    int frameCount{0};
    float frameTime{0.f};
    float currentTime{0.f};
    int currentFrame{0};
};

}
