#include "Animation.h"

namespace BayouBonanza {

Animation::Animation() {}

bool Animation::load(const std::string& sheetPath, int fw, int fh, int count, float ft) {
    if (!texture.loadFromFile(sheetPath)) {
        return false;
    }
    frameWidth = fw;
    frameHeight = fh;
    frameCount = count;
    frameTime = ft;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0,0,frameWidth,frameHeight));
    return true;
}

void Animation::update(float dt) {
    if (frameCount <= 1) return;
    currentTime += dt;
    while (currentTime >= frameTime) {
        currentTime -= frameTime;
        currentFrame = (currentFrame + 1) % frameCount;
        sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
    }
}

void Animation::reset() {
    currentTime = 0.f;
    currentFrame = 0;
    sprite.setTextureRect(sf::IntRect(0,0,frameWidth,frameHeight));
}

}
