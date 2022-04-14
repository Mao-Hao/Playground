// Support Code written by Michael D. Shah
// Updated by Seth Cooper
// Please do not redistribute without asking permission.

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
#include <memory>

static const int TAG_PLAYER = 1;
static const int TAG_GOAL = 2;
static const int TAG_BLOCK = 3;
static const int TAG_ENEMY = 4;

class JmpInputComponent : public GenericComponent {
public:
    JmpInputComponent(GameObject& gameObject, float speed, float jump, float gravity)
        : GenericComponent(gameObject)
        , mSpeed(speed)
        , mJump(jump)
        , mGravity(gravity)
    {
    }

    virtual void update(Level& level) override
    {
        bool left = InputManager::getInstance().isKeyDown(SDLK_LEFT);
        bool right = InputManager::getInstance().isKeyDown(SDLK_RIGHT);
        bool jump = InputManager::getInstance().isKeyPressed(SDLK_UP);

        GameObject& gameObject = getGameObject();
        std::shared_ptr<PhysicsComponent> pc = gameObject.physicsComponent();

        if (left && !right) {
            pc->setVx(-mSpeed);
        } else if (!left && right) {
            pc->setVx(mSpeed);
        } else {
            pc->setVx(0.0f);
        }

        if (jump) {
            bool onGround = false;
            std::vector<std::shared_ptr<GameObject>> objects;
            if (level.getCollisions(gameObject.x() + 0.02f * gameObject.w(), gameObject.y() + gameObject.h() + 2.0f, objects)) {
                for (auto obj : objects) {
                    if (obj->tag() == TAG_BLOCK) {
                        onGround = true;
                    }
                }
            }
            if (level.getCollisions(gameObject.x() + 0.98f * gameObject.w(), gameObject.y() + gameObject.h() + 2.0f, objects)) {
                for (auto obj : objects) {
                    if (obj->tag() == TAG_BLOCK) {
                        onGround = true;
                    }
                }
            }
            if (onGround) {
                pc->setVy(-mJump);
                return;
            }
        }

        pc->setVy(std::min(pc->vy() + mGravity, mJump));
    }

private:
    float mSpeed;
    float mJump;
    float mGravity;
};

const float SIZE = 40.0f;

class JmpPlayer : public GameObject {
public:
    JmpPlayer(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_PLAYER)
    {
        addGenericCompenent(std::make_shared<JmpInputComponent>(*this, 10.0f, 20.0f, 1.0f));
        addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_GOAL));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0x00, 0xff, 0x00));
    }
};

class JmpGoal : public GameObject {
public:
    JmpGoal(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_GOAL)
    {
        addGenericCompenent(std::make_shared<PatrolComponent>(*this, x - SIZE * 2.0f, y, 2.0f));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xff, 0x99, 0x00));
    }
};

class JmpBlock : public GameObject {
public:
    JmpBlock(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_BLOCK)
    {
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, true));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0x99, 0x99, 0x99));
    }
};

class JmpEnemy : public GameObject {
public:
    JmpEnemy(float x, float y, std::weak_ptr<GameObject> player)
        : GameObject(x, y, SIZE, SIZE, TAG_ENEMY)
    {
        // TODO PART 4: set up state machine
        const float x0 = x;
        const float y0 = y;
        const float x1 = x + SIZE * 4;
        const float y1 = y;
        const float x2 = x + SIZE;
        const float y2 = y + SIZE * 4.0f;

        std::shared_ptr<StateComponent::State> moveTo0State = std::make_shared<MoveState>(2.0f, x0, y0);
        std::shared_ptr<StateComponent::State> moveTo1State = std::make_shared<MoveState>(2.0f, x1, y1);
        std::shared_ptr<StateComponent::State> moveTo2State = std::make_shared<MoveState>(2.0f, x2, y2);

        std::shared_ptr<StateComponent::State> chaseState = std::make_shared<ChaseState>(6.0f, player);
        std::shared_ptr<StateComponent::State> returnState = std::make_shared<MoveState>(4.0f, x0, y0);

        std::shared_ptr<StateComponent> sc = std::make_shared<StateComponent>(*this);
        sc->setStartState(moveTo0State);
        sc->addTransition(moveTo0State, moveTo1State, std::make_shared<PointProximityTransition>(x0, y0, SIZE / 4.0f));
        sc->addTransition(moveTo1State, moveTo2State, std::make_shared<PointProximityTransition>(x1, y1, SIZE / 4.0f));
        sc->addTransition(moveTo2State, moveTo0State, std::make_shared<PointProximityTransition>(x2, y2, SIZE / 4.0f));

        sc->addTransition(moveTo0State, chaseState, std::make_shared<ObjectProximityTransition>(player, SIZE * 5.0f));
        sc->addTransition(moveTo1State, chaseState, std::make_shared<ObjectProximityTransition>(player, SIZE * 5.0f));
        sc->addTransition(moveTo2State, chaseState, std::make_shared<ObjectProximityTransition>(player, SIZE * 5.0f));

        sc->addTransition(chaseState, returnState, std::make_shared<TimedTransition>(60));
        sc->addTransition(returnState, chaseState, std::make_shared<ObjectProximityTransition>(player, SIZE * 3.0f));

        sc->addTransition(returnState, moveTo0State, std::make_shared<PointProximityTransition>(x0, y0, SIZE / 4.0f));

        addGenericCompenent(sc);

        addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_PLAYER));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xdd, 0x22, 0x22));
    }
};

class JmpWanderEnemy : public GameObject {
public:
    JmpWanderEnemy(float x, float y)
        : GameObject(x, y, SIZE, SIZE, TAG_ENEMY)
    {
        std::shared_ptr<StateComponent::State> wanderState = std::make_shared<WanderState>(3.0f);

        std::shared_ptr<StateComponent> sc = std::make_shared<StateComponent>(*this);

        sc->setStartState(wanderState);

        addGenericCompenent(sc);
        addGenericCompenent(std::make_shared<RemoveOnCollideComponent>(*this, TAG_PLAYER));
        setPhysicsCompenent(std::make_shared<PhysicsComponent>(*this, false));
        setRenderCompenent(std::make_shared<RectRenderComponent>(*this, 0xdd, 0x22, 0x22));
    }
};

int main(int argc, char** argv)
{
    std::shared_ptr<Level> level = std::make_shared<Level>(20 * SIZE, 20 * SIZE);

    std::shared_ptr<JmpPlayer> player = std::make_shared<JmpPlayer>(2 * SIZE, 16 * SIZE);
    level->addObject(player);
    level->addObject(std::make_shared<JmpGoal>(18 * SIZE, 2 * SIZE));
    level->addObject(std::make_shared<JmpEnemy>(5 * SIZE, 5 * SIZE, player));
    level->addObject(std::make_shared<JmpWanderEnemy>(10 * SIZE, 5 * SIZE));

    for (int ii = 0; ii < 5; ++ii) {
        level->addObject(std::make_shared<JmpBlock>((2 * ii + 5) * SIZE, (15 - 2 * ii) * SIZE));
    }
    for (int ii = 0; ii < 20; ++ii) {
        level->addObject(std::make_shared<JmpBlock>(ii * SIZE, 19 * SIZE));
    }

    SDLGraphicsProgram mySDLGraphicsProgram(level);

    mySDLGraphicsProgram.loop();

    return 0;
}
