#include "base/Level.hpp"
#include <algorithm>

Level::Level(int w, int h)
    : mW(w)
    , mH(h)
{
}

void Level::addObject(std::shared_ptr<GameObject> object)
{
    mObjectsToAdd.push_back(object);
}

void Level::removeObject(std::shared_ptr<GameObject> object)
{
    mObjectsToRemove.push_back(object);
}

bool Level::hasObject(std::shared_ptr<GameObject> object) const
{
    return std::find(mObjects.begin(), mObjects.end(), object) != mObjects.end();
}

bool Level::getCollisions(const GameObject& obj, std::vector<std::shared_ptr<GameObject>>& objects) const
{
    objects.clear();
    for (auto gameObject : mObjects) {
        if (gameObject.get() != &obj && gameObject->isColliding(obj)) {
            objects.push_back(gameObject);
        }
    }
    return !objects.empty();
}

bool Level::getCollisions(float px, float py, std::vector<std::shared_ptr<GameObject>>& objects) const
{
    objects.clear();
    for (auto gameObject : mObjects) {
        if (gameObject->isColliding(px, py)) {
            objects.push_back(gameObject);
        }
    }
    return !objects.empty();
}

void Level::update()
{
    for (auto obj : mObjectsToAdd) {
        mObjects.push_back(obj);
    }
    mObjectsToAdd.clear();

    for (auto gameObject : mObjects) {
        gameObject->update(*this);
    }
    for (auto gameObject : mObjects) {
        gameObject->step(*this);
    }

    for (auto obj : mObjectsToRemove) {
        auto elem = std::find(mObjects.begin(), mObjects.end(), obj);
        if (elem != mObjects.end()) {
            mObjects.erase(elem);
        }
    }
    mObjectsToRemove.clear();
}

void Level::render(SDL_Renderer* renderer)
{
    for (auto gameObject : mObjects) {
        gameObject->render(renderer);
    }
}
