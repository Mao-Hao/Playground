#ifndef BASE_REMOVE_ON_COLLIDE_COMPONENT
#define BASE_REMOVE_ON_COLLIDE_COMPONENT

#include "base/GenericComponent.hpp"

//! \brief A component that removes a game object (of a given tag) on collision.
class RemoveOnCollideComponent : public GenericComponent {
public:
    RemoveOnCollideComponent(GameObject& gameObject, int tag);

    virtual void collision(Level& level, std::shared_ptr<GameObject> obj) override;

private:
    int mTag;
};

#endif
