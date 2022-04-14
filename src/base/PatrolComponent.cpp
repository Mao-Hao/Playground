#include "base/PatrolComponent.hpp"
#include "base/GameObject.hpp"
#include <cmath>

PatrolComponent::PatrolComponent(GameObject & gameObject, float toX, float toY, float speed):
  GenericComponent(gameObject)
{
  mForward = true;
  mStep = 0;
  
  mDX = toX - gameObject.x();
  mDY = toY - gameObject.y();
  float len = sqrtf(mDX * mDX + mDY * mDY);
  mDX = mDX / len * speed;
  mDY = mDY / len * speed;
  mSteps = len / speed;
}

void
PatrolComponent::update(Level & level)
{
  GameObject & gameObject = getGameObject();

  gameObject.setX(gameObject.x() + mDX * (mForward ? 1.0f : -1.0f));
  gameObject.setY(gameObject.y() + mDY * (mForward ? 1.0f : -1.0f));

  mStep ++;
  if (mStep >= mSteps) {
    mStep = 0;
    mForward = !mForward;
  }
}
