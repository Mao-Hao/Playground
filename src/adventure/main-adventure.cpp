// Support Code written by Michael D. Shah
// Updated by Seth Cooper
// Please do not redistribute without asking permission.

#include "AdvActions.hpp"
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

class AdvEnemy : public GameObject {
public:
    AdvEnemy(float x, float y, std::weak_ptr<GameObject> player)
        : GameObject(x, y, SIZE, SIZE, TAG_ENEMY)
    {
        std::shared_ptr<SleepAction> sleepAction = std::make_shared<SleepAction>(*this, 1000);
        std::shared_ptr<ChaseAction> chaseAction = std::make_shared<ChaseAction>(*this, 6.0f, player);
        std::shared_ptr<Sequence> chaseSequence = std::make_shared<Sequence>();

        chaseSequence->addChild(sleepAction);
        chaseSequence->addChild(chaseAction);

        std::shared_ptr<BehaviorTree> bt = std::make_shared<BehaviorTree>(*this);
        bt->setRoot(sleepAction);
        addGenericCompenent(bt);

        // addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_PLAYER));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, true));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xdd, 0x22, 0x22));
    }
};

int main(int argc, char** argv)
{
    std::shared_ptr<Level> level = std::make_shared<Level>(30 * SIZE, 30 * SIZE);

    std::shared_ptr<AdvPlayer> player = std::make_shared<AdvPlayer>(2 * SIZE, 2 * SIZE);

    Blackboard::getInstance()->setPlayer(player);

    level->addObject(player);
    level->addObject(std::make_shared<AdvGoal>(18 * SIZE, 18 * SIZE));
    level->addObject(std::make_shared<AdvEnemy>(2 * SIZE, 17 * SIZE, player));

    SDLGraphicsProgram mySDLGraphicsProgram(level);

    mySDLGraphicsProgram.loop();

    return 0;
}
