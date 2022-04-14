#include "base/PhysicsComponent.hpp"
#include "base/GameObject.hpp"
#include "base/Level.hpp"
#include <cmath>

PhysicsComponent::PhysicsComponent(GameObject & gameObject, bool solid):
  Component(gameObject),
  mSolid(solid),
  mVx(0.0f),
  mVy(0.0f)
{
}

void
PhysicsComponent::step(Level & level)
{
  GameObject & gameObject = getGameObject();

  float oldX = gameObject.x();
  float oldY = gameObject.y();

  gameObject.setX(gameObject.x() + mVx);
  gameObject.setY(gameObject.y() + mVy);

  if (!mSolid) {
    std::vector<std::shared_ptr<GameObject>> objects;
    if (level.getCollisions(gameObject, objects)) {
      for (auto obj: objects) {
	if (obj->physicsComponent()) {
	  if (obj->physicsComponent()->isSolid()) {
	    if (!gameObject.isColliding(*obj)) {
	      continue;
	    }

	    float resolveX = 0.0f;
	    if (oldX < obj->x() && mVx > 0.0f) {
	      resolveX = -(gameObject.x() - (obj->x() - gameObject.w()));
	    } else if (oldX > obj->x() && mVx < 0.0f) {
	      resolveX = (obj->x() - (gameObject.x() - obj->w()));
	    }

	    float resolveY = 0.0f;
	    if (oldY < obj->y() && mVy > 0.0f) {
	      resolveY = -(gameObject.y() - (obj->y() - gameObject.h()));
	    } else if (oldY > obj->y() && mVy < 0.0f) {
	      resolveY = (obj->y() - (gameObject.y() - obj->h()));
	    }

	    if (resolveX != 0.0f && resolveY != 0.0f) {
	      if (fabsf(resolveX) < fabsf(resolveY)) {
		gameObject.setX(gameObject.x() + resolveX);
		mVx = 0;
	      } else {
		gameObject.setY(gameObject.y() + resolveY);
		mVy = 0;
	      }
	    } else if (resolveX != 0.0f) {
	      gameObject.setX(gameObject.x() + resolveX);
	      mVx = 0;
	    } else if (resolveY != 0.0f) {
	      gameObject.setY(gameObject.y() + resolveY);
	      mVy = 0;
	    }
	  } else {
	    gameObject.collision(level, obj);
	  }
	}
      }
    }
  }
}
