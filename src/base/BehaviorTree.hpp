#ifndef __BEHAVIOR_TREE_HPP__
#define __BEHAVIOR_TREE_HPP__

#include "base/GenericComponent.hpp"

#include <memory>
#include <stdlib.h>
#include <vector>

class Blackboard {
private:
    Blackboard() = default;
    ~Blackboard() = default;
    Blackboard(const Blackboard&) = delete;
    void operator=(const Blackboard&) = delete;
    Blackboard(Blackboard&&) = delete;
    void operator=(Blackboard&&) = delete;

public:
    static Blackboard* getInstance()
    {
        static Blackboard* instance = new Blackboard();
        return instance;
    }

    void setPlayer(std::shared_ptr<GameObject> player)
    {
        mPlayerPtr = player;
    }

    std::shared_ptr<GameObject> getPlayerPtr() const { return mPlayerPtr; }

    void setRushXY(int x, int y)
    {
        mRushX = x;
        mRushY = y;
    }

    float getRushX() const { return mRushX; }
    float getRushY() const { return mRushY; }

private:
    std::shared_ptr<GameObject> mPlayerPtr = nullptr;
    float mRushX, mRushY;
};

enum class Status {
    INVALID,
    SUCCESS,
    FAILURE,
    RUNNING,
    ABORTED,
};

class BehaviorNode {
private:
    Status mStatus;

public:
    virtual void onEnter() { }
    virtual void onExit() { }

    BehaviorNode()
        : mStatus(Status::INVALID)
    {
        mBlackboard = Blackboard::getInstance();
    }
    virtual ~BehaviorNode() { }

    virtual Status update() = 0;
    // virtual Status update(GameObject& gameObject, Level& level) = 0;

    virtual Status tick()
    {
        if (mStatus != Status::RUNNING)
            onEnter();

        mStatus = update();

        if (mStatus != Status::RUNNING)
            onExit();

        return mStatus;
    }

    void reset() { mStatus = Status::INVALID; }
    bool isRunning() const { return mStatus == Status::RUNNING; }
    bool isExit() const { return mStatus == Status::SUCCESS || mStatus == Status::FAILURE; }
    Status getStatus() const { return mStatus; }
    void abort()
    {
        onExit();
        mStatus = Status::ABORTED;
    }

    Blackboard* mBlackboard;
};

// ActionNode: accessing information and making changes to the world

// ConditionNode: tree's primary way of checking for information in the world
// e.g. is the player in a certain area?

class Decorator : public BehaviorNode {
protected:
    std::shared_ptr<BehaviorNode> mChild;

public:
    Decorator(std::shared_ptr<BehaviorNode> child)
        : mChild(child)
    {
    }
};

class Inverter : public Decorator {
public:
    Inverter(std::shared_ptr<BehaviorNode> child)
        : Decorator(child)
    {
    }

    Status update() override
    {
        mChild->tick();
        if (mChild->getStatus() == Status::SUCCESS)
            return Status::FAILURE;
        else if (mChild->getStatus() == Status::FAILURE)
            return Status::SUCCESS;
        else
            return mChild->getStatus();
    }
};

class Repeater : public Decorator {
protected:
    int mCount;
    int mLimit;

public:
    Repeater(std::shared_ptr<BehaviorNode> child)
        : Decorator(child)
    {
    }

    virtual void onEnter() override
    {
        mCount = 0;
    }

    virtual Status update() override
    {
        while (true) {
            mChild->tick();
            if (mChild->getStatus() == Status::RUNNING)
                break;
            if (mChild->getStatus() == Status::FAILURE)
                return Status::FAILURE;
            if (++mCount >= mLimit)
                return Status::SUCCESS;
            mChild->reset();
        }
        return Status::INVALID;
    }

    void setLimit(int limit) { mLimit = limit; }
};

// FIXME:
class RepeatUntilFailure : public Decorator {
public:
    RepeatUntilFailure(std::shared_ptr<BehaviorNode> child)
        : Decorator(child)
    {
    }

    Status update() override
    {
        while (true) {
            mChild->tick();
            if (mChild->getStatus() == Status::FAILURE)
                return Status::FAILURE;
            mChild->reset();
        }
    }
};

class Composite : public BehaviorNode {
protected:
    std::vector<std::shared_ptr<BehaviorNode>> mChildren;

public:
    void addChild(std::shared_ptr<BehaviorNode> child)
    {
        mChildren.push_back(child);
    }
    void removeChild(std::shared_ptr<BehaviorNode> child);
};

// And
class Sequence : public Composite {
public:
    std::vector<std::shared_ptr<BehaviorNode>>::iterator mCurrentChild;

    virtual void onEnter() override
    {
        mCurrentChild = mChildren.begin();
    }

    virtual Status update() override
    {
        while (true) {
            if (mCurrentChild == mChildren.end())
                return Status::SUCCESS;

            Status status = (*mCurrentChild)->tick();

            if (status == Status::RUNNING || status == Status::FAILURE)
                return status;

            ++mCurrentChild;
        }
    }

    virtual ~Sequence() { }
};

class Filter : public Sequence {
public:
    void addCondition(std::shared_ptr<BehaviorNode> condition)
    {
        mChildren.insert(mChildren.begin(), condition);
    }

    void addBehavior(std::shared_ptr<BehaviorNode> behavior)
    {
        mChildren.push_back(behavior);
    }
};

class RandomSequence : public Sequence {
    // TODO:
};

// Or
class Selector : public Composite {
public:
    std::vector<std::shared_ptr<BehaviorNode>>::iterator mCurrentChild;

    virtual void onEnter() override
    {
        mCurrentChild = mChildren.begin();
    }

    virtual Status update() override
    {
        while (true) {
            if (mCurrentChild == mChildren.end())
                return Status::FAILURE;

            Status status = (*mCurrentChild)->tick();

            if (status == Status::RUNNING || status == Status::SUCCESS)
                return status;

            ++mCurrentChild;
        }
    }

    virtual ~Selector() { }
};

class RandomSelector : public Selector {
public:
    Status update() override
    {
        std::vector<std::shared_ptr<BehaviorNode>>::iterator currentChild = mChildren.begin();
        std::advance(currentChild, rand() % mChildren.size());
        return (*currentChild)->tick();
    }
};

class ActiveSelector : public Selector {
    // TODO:
};

class Parallel : public Composite {
public:
    enum class Policy {
        RequireOne,
        RequireAll
    };

    Parallel(Policy success, Policy failure)
        : mSuccessPolicy(success)
        , mFailurePolicy(failure)
    {
    }

    virtual ~Parallel() { }

protected:
    Policy mSuccessPolicy;
    Policy mFailurePolicy;

    virtual Status update() override
    {
        size_t successCount = 0;
        size_t failureCount = 0;

        for (auto child : mChildren) {
            if (!child->isExit())
                child->tick();

            if (child->getStatus() == Status::SUCCESS) {
                ++successCount;
                if (mSuccessPolicy == Policy::RequireOne)
                    return Status::SUCCESS;
            }

            if (child->getStatus() == Status::FAILURE) {
                ++failureCount;
                if (mFailurePolicy == Policy::RequireOne)
                    return Status::FAILURE;
            }
        }

        if (mFailurePolicy == Policy::RequireAll && failureCount == mChildren.size())
            return Status::FAILURE;

        if (mSuccessPolicy == Policy::RequireAll && successCount == mChildren.size())
            return Status::SUCCESS;

        return Status::RUNNING;
    }

    virtual void onExit() override
    {
        for (auto child : mChildren)
            if (child->isRunning())
                child->abort();
    }
};

class Monitor : public Parallel {
    // TODO:
};

class BehaviorTree : public GenericComponent {
public:
    BehaviorTree(GameObject& gameObject)
        : GenericComponent(gameObject)
    {
    }
    // TODO: cooldown, status check, reset, pass the GameObject to the nodes?
    virtual void update(Level& level) override
    {
        if (mRoot == nullptr)
            return;

        mRoot->tick();
    }

    void setRoot(std::shared_ptr<BehaviorNode> root) { mRoot = root; }

private:
    std::shared_ptr<BehaviorNode> mRoot;
};

#endif