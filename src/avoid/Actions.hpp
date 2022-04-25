#ifndef __ACTIONS_HPP__
#define __ACTIONS_HPP__

#include "base/BehaviorTree.hpp"
#include "base/GameObject.hpp"
#include "base/RectRenderComponent.hpp"

#include <iostream>

static bool moveToward(GameObject& gameObject, float x, float y, float speed)
{
    std::cout << "moveToward" << std::endl;
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

// class IdleAction : public BehaviorNode {
// public:
//     IdleAction() = default;

// };

class RushWaitAction : public BehaviorNode {
public:
    RushWaitAction(GameObject& gameObject, Uint32 duration)
        : self(gameObject)
        , mDuration(duration)
    {
    }

    virtual void onEnter() override
    {
        mBlackboard->setRushXY(mBlackboard->getPlayerPtr()->x(), mBlackboard->getPlayerPtr()->y());
        mStartTime = SDL_GetTicks();
        self.setRenderCompenent(std::make_shared<RectRenderComponent>(self, 0x22, 0x22, 0xdd));
    }

    virtual Status update() override
    {
        const Uint32 now = SDL_GetTicks();
        if (now - mStartTime >= mDuration) {
            bool* isRushing = reinterpret_cast<bool*>(self.mData);
            *isRushing = true;
            std::cout << "RushWaitAction: SUCCESS" << std::endl;

            return Status::SUCCESS;
        } else {
            return Status::RUNNING;
        }
    }

private:
    GameObject& self;
    Uint32 mDuration;
    Uint32 mStartTime;
};

class RushAction : public BehaviorNode {
public:
    RushAction(GameObject& gameObject, float speed)
        : self(gameObject)
        , mSpeed(speed)
    {
    }

    virtual void onEnter() override
    {
        self.setRenderCompenent(std::make_shared<RectRenderComponent>(self, 0xdd, 0x22, 0xdd));
    }

    virtual Status update() override
    {
        bool* isReady = reinterpret_cast<bool*>(self.mData);
        if (!isReady)
            return Status::FAILURE;

        if (moveToward(self, mBlackboard->getRushX(), mBlackboard->getRushY(), mSpeed)) {
            *isReady = false;
            std::cout << "RushAction: Finished" << std::endl;
        }
        return Status::SUCCESS;
    }

private:
    GameObject& self;
    const float mSpeed;
};

class RushDetectNearbyAction : public BehaviorNode {
public:
    RushDetectNearbyAction(GameObject& gameObject, float distance)
        : self(gameObject)
        , mDistance(distance)
    {
    }

    virtual Status update() override
    {
        if (isNear(self, mBlackboard->getPlayerPtr()->x(), mBlackboard->getPlayerPtr()->y(), mDistance)) {
            return Status::SUCCESS;
        } else {
            self.setRenderCompenent(std::make_shared<RectRenderComponent>(self, 0x22, 0xdd, 0x22));
            return Status::FAILURE;
        }
    }

private:
    GameObject& self;
    float mDistance;
};

class IsRushingCondition : public BehaviorNode {
public:
    IsRushingCondition(GameObject& gameObject)
        : self(gameObject)
    {
    }

    virtual Status update() override
    {
        bool* isRushing = reinterpret_cast<bool*>(self.mData);
        if (*isRushing) {
            std::cout << "IsRushingCondition: SUCCESS" << std::endl;
            return Status::SUCCESS;
        } else {
            std::cout << "IsRushingCondition: FAILURE" << std::endl;
            return Status::FAILURE;
        }
    }

private:
    GameObject& self;
};

class SleepAction : public BehaviorNode {
public:
    SleepAction(GameObject& gameObject, Uint32 duration)
        : self(gameObject)
        , mDuration(duration)
    {
    }

    virtual void onEnter() override
    {
        mStartTime = SDL_GetTicks();
    }

    virtual Status update() override
    {
        bool* isSleeping = reinterpret_cast<bool*>(self.mData);
        if (*isSleeping == false) {
            return Status::FAILURE;
        }

        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - mStartTime;
        if (elapsedTime >= mDuration || isNear(self, mBlackboard->getPlayerPtr()->x(), mBlackboard->getPlayerPtr()->y(), SIZE * 2.5f)) {
            *isSleeping = false;
            return Status::SUCCESS;
        } else {
            return Status::RUNNING;
        }
    }

private:
    GameObject& self;
    Uint32 mDuration;
    Uint32 mStartTime;
};

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

#endif // __ACTIONS_HPP__