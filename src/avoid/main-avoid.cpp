// Support Code written by Michael D. Shah
// Updated by Seth Cooper
// Please do not redistribute without asking permission.

#include "Actions.hpp"
#include "base/BehaviorTree.hpp"
#include "base/GameObject.hpp"
#include "base/GenericComponent.hpp"
#include "base/InputManager.hpp"
#include "base/Level.hpp"
#include "base/PatrolComponent.hpp"
#include "base/RectRenderComponent.hpp"
#include "base/RemoveOnCollideComponent.hpp"
#include "base/SDLGraphicsProgram.hpp"
#include "base/StateComponent.hpp"
#include "base/StatesAndTransitions.hpp"

#include <iostream>
#include <memory>

static const int TAG_PLAYER = 1;
static const int TAG_GOAL = 2;
static const int TAG_BLOCK = 3;
static const int TAG_ENEMY = 4;

class AdvInputComponent : public GenericComponent {
public:
    AdvInputComponent(GameObject& gameObject, float speed)
        : GenericComponent(gameObject)
        , mSpeed(speed)
    {
    }

    virtual void update(Level& level) override
    {
        bool left = InputManager::getInstance().isKeyDown(SDLK_LEFT);
        bool right = InputManager::getInstance().isKeyDown(SDLK_RIGHT);
        bool up = InputManager::getInstance().isKeyDown(SDLK_UP);
        bool down = InputManager::getInstance().isKeyDown(SDLK_DOWN);

        GameObject& gameObject = getGameObject();
        std::shared_ptr<PhysicsComponent> pc = gameObject.physicsComponent();

        if (left && !right) {
            pc->setVx(-mSpeed);
        } else if (!left && right) {
            pc->setVx(mSpeed);
        } else {
            pc->setVx(0.0f);
        }

        if (up && !down) {
            pc->setVy(-mSpeed);
        } else if (!up && down) {
            pc->setVy(mSpeed);
        } else {
            pc->setVy(0.0f);
        }
    }

private:
    float mSpeed;
};

class AdvPlayer : public GameObject {
public:
    AdvPlayer(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_PLAYER)
    {
        addGenericCompenent(std::make_shared<AdvInputComponent>(*this, 10.0f));
        addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_GOAL));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0x00, 0xff, 0xaa));
    }
};

class AdvGoal : public GameObject {
public:
    AdvGoal(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_GOAL)
    {
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xff, 0xff, 0x00));
    }
};

class AdvBlock : public GameObject {
public:
    AdvBlock(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_BLOCK)
    {
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, true));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0x99, 0x99, 0x99));
    }
};

class SleepEnemy : public GameObject {
private:
    bool isSleeping;

public:
    SleepEnemy(float x, float y, std::weak_ptr<GameObject> player)
        : GameObject(x, y, SIZE, SIZE, TAG_ENEMY)
    {
        isSleeping = true;
        mData = &isSleeping;

        std::shared_ptr<SleepAction> sleepAction = std::make_shared<SleepAction>(*this, 1000 * 30);
        std::shared_ptr<Inverter> inverter = std::make_shared<Inverter>(sleepAction);

        std::shared_ptr<ChaseAction> chaseAction = std::make_shared<ChaseAction>(*this, 6.0f, player);
        std::shared_ptr<Sequence> chaseSequence = std::make_shared<Sequence>();

        chaseSequence->addChild(inverter);
        chaseSequence->addChild(chaseAction);

        std::shared_ptr<BehaviorTree> bt = std::make_shared<BehaviorTree>(*this);
        bt->setRoot(chaseSequence);
        addGenericCompenent(bt);

        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, true));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xdd, 0xdd, 0xdd));
    }
};

class RushEnemy : public GameObject {
private:
    bool isRunning;

public:
    RushEnemy(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_ENEMY)
    {
        isRunning = false;
        mData = &isRunning;
        // behavior tree
        std::shared_ptr<BehaviorTree> bt = std::make_shared<BehaviorTree>(*this);
        addGenericCompenent(bt);

        // first level of the tree (root)
        std::shared_ptr<Sequence> rushSequence = std::make_shared<Sequence>();
        bt->setRoot(rushSequence);

        // second level of the tree
        std::shared_ptr<Selector> rushSelector = std::make_shared<Selector>();
        std::shared_ptr<RushAction> rushAction = std::make_shared<RushAction>(*this, 3.5 * 6.0f);
        rushSequence->addChild(rushSelector);
        rushSequence->addChild(rushAction);

        // third level of the tree
        std::shared_ptr<IsRushingCondition> isRushingCondition = std::make_shared<IsRushingCondition>(*this);
        std::shared_ptr<Sequence> rushSequence1 = std::make_shared<Sequence>();
        rushSelector->addChild(isRushingCondition);
        rushSelector->addChild(rushSequence1);

        // fourth level of the tree
        std::shared_ptr<RushDetectNearbyAction> detectNearbyAction = std::make_shared<RushDetectNearbyAction>(*this, SIZE * 7.0f);
        std::shared_ptr<RushWaitAction> waitAction = std::make_shared<RushWaitAction>(*this, 1500);
        rushSequence1->addChild(detectNearbyAction);
        rushSequence1->addChild(waitAction);

        addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_PLAYER));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0x22, 0xdd, 0x22));
    }
};

int main(int argc, char** argv)
{
    std::shared_ptr<Level> level = std::make_shared<Level>(30 * SIZE, 30 * SIZE);

    std::shared_ptr<AdvPlayer> player = std::make_shared<AdvPlayer>(14 * SIZE, 14 * SIZE);

    Blackboard::getInstance()->setPlayer(player);

    level->addObject(player);
    level->addObject(std::make_shared<AdvGoal>(9 * SIZE, 9 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(9 * SIZE, 19 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(19 * SIZE, 19 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(19 * SIZE, 9 * SIZE));

    level->addObject(std::make_shared<AdvGoal>(4 * SIZE, 14 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(14 * SIZE, 4 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(14 * SIZE, 24 * SIZE));
    level->addObject(std::make_shared<AdvGoal>(24 * SIZE, 14 * SIZE));

    level->addObject(std::make_shared<RushEnemy>(4 * SIZE, 4 * SIZE));
    level->addObject(std::make_shared<RushEnemy>(4 * SIZE, 24 * SIZE));
    level->addObject(std::make_shared<RushEnemy>(24 * SIZE, 4 * SIZE));
    level->addObject(std::make_shared<RushEnemy>(24 * SIZE, 24 * SIZE));

    level->addObject(std::make_shared<SleepEnemy>(0 * SIZE, 0 * SIZE, player));
    level->addObject(std::make_shared<SleepEnemy>(29 * SIZE, 0 * SIZE, player));
    level->addObject(std::make_shared<SleepEnemy>(0 * SIZE, 29 * SIZE, player));
    level->addObject(std::make_shared<SleepEnemy>(29 * SIZE, 29 * SIZE, player));

    SDLGraphicsProgram mySDLGraphicsProgram(level);

    mySDLGraphicsProgram.loop();

    return 0;
}
