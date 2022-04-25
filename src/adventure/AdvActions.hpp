#ifndef __ADVENTURE_ADVACTIONS_HPP__
#define __ADVENTURE_ADVACTIONS_HPP__

#include "base/BehaviorTree.hpp"
#include "base/GameObject.hpp"

static bool moveToward(GameObject& gameObject, float x, float y, float speed)
{
    const float epsilon = 0.01;

    float dX = x - gameObject.x();
    float dY = y - gameObject.y();
    const float len = sqrtf(dX * dX + dY * dY);
    if (len < epsilon) {
        return true;
    } else {
        const float scale = std::min(len, speed);
        dX = dX / len * scale;
        dY = dY / len * scale;

        gameObject.setX(gameObject.x() + dX);
        gameObject.setY(gameObject.y() + dY);

        return (speed >= len - epsilon);
    }
}

// return true if gameobject is less than distance from x,y
static bool isNear(const GameObject& gameObject, float x, float y, float distance)
{
    const float dX = x - gameObject.x();
    const float dY = y - gameObject.y();
    const float lensqr = dX * dX + dY * dY;
    return lensqr <= distance * distance;
}

//-----------------------------------------------------------------------------

class ChaseAction : public BehaviorNode {
public:
    ChaseAction(GameObject& gameObject, float speed, std::weak_ptr<GameObject> which)
        : self(gameObject)
        , mSpeed(speed)
        , mWhich(which)
    {
    }

    virtual Status update() override
    {
        std::shared_ptr<GameObject> whichShared = mWhich.lock();

        if (whichShared) {
            moveToward(self, whichShared->x(), whichShared->y(), mSpeed);
        }

        return Status::SUCCESS;
    }

private:
    GameObject& self;
    const float mSpeed;
    const std::weak_ptr<GameObject> mWhich;
};



#endif // __ADVENTURE_ADVACTIONS_HPP__