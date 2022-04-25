#ifndef BASE_GAME_OBJECT
#define BASE_GAME_OBJECT

#include "base/GenericComponent.hpp"
#include "base/PhysicsComponent.hpp"
#include "base/RenderComponent.hpp"
#include <SDL.h>
#include <memory>
#include <vector>

const float SIZE = 30.0f;

class Level;

//! \brief Represents an object in the game.  Has some essential
//! properties (position and size), a tag (identifying the general
//! category of object), and a collection of components, including any
//! number of generic components and optionally a physics component
//! and a render component.
class GameObject {
public:
    GameObject(float x, float y, float w, float h, int tag);
    virtual ~GameObject();

    inline int tag() const { return mTag; }

    inline void setX(float x) { mX = x; }
    inline void setY(float y) { mY = y; }

    inline float x() const { return mX; }
    inline float y() const { return mY; }
    inline float w() const { return mW; }
    inline float h() const { return mH; }

    inline void addGenericCompenent(std::shared_ptr<GenericComponent> comp) { mGenericComponents.push_back(comp); }
    inline void setPhysicsCompenent(std::shared_ptr<PhysicsComponent> comp) { mPhysicsComponent = comp; }
    inline void setRenderCompenent(std::shared_ptr<RenderComponent> comp) { mRenderComponent = comp; }

    inline std::vector<std::shared_ptr<GenericComponent>> genericComponents() { return mGenericComponents; }
    inline std::shared_ptr<PhysicsComponent> physicsComponent() { return mPhysicsComponent; }
    inline std::shared_ptr<RenderComponent> renderComponent() { return mRenderComponent; }

    void update(Level& level); //!< Update the object.
    void collision(Level& level, std::shared_ptr<GameObject> obj); //!< Handle collisions with another object.
    void step(Level& level); //!< Do the physics step for the object.
    void render(SDL_Renderer* renderer); //!< Render the object.

    bool isColliding(const GameObject& obj) const; //!< Determine if this object is colliding with another.
    bool isColliding(float px, float py) const; //!< Determine if this object is colliding with a point.

    void* mData;

private:
    GameObject(const GameObject&) = delete;
    void operator=(GameObject const&) = delete;

    float mX, mY, mW, mH;
    int mTag;

    std::vector<std::shared_ptr<GenericComponent>> mGenericComponents;
    std::shared_ptr<PhysicsComponent> mPhysicsComponent;
    std::shared_ptr<RenderComponent> mRenderComponent;
};

#endif
