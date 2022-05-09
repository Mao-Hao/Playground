# Final Project

Name: Hao Mao


## brief

With the professor's approval, I completed my final project based on some code from the CS5850 course.

I implemented a behavior tree and used it to define the behavior of the AI, and then built a tiny game

If you can't compile or execute, please check the Makefile, maybe because we have different lib locations

## what I have done

-   Implemented a behavior tree(see src/base/BehaviorTree.hpp).
    It has several types of nodes, Sequence, Selector, Repeater, Parallel, and etc..
    The nodes can communicate with each other through a blackboard

-   Implemented a tiny game called avoid(see src/avoid/*)
    We can type make run-avoid to compile and run the game.

-   Implemented two types of enemy AI based on the behavior tree I implemented.

    -   RushEnemy:
        The color of RushEnemy will change as the state changes
        When you are far away from them, nothing happens
        Once you get close, you will be remembered where you first entered the detection range
        Then after a short wait, the enemy will rush towards you
        Run away in order to bring them back to their initial state

                                        rushSequence
                                        /           \
                                rushSelector    rushAction
                                /         \
                               /           \
                    isRushingCondition  rushSequence1
                                          /       \
                                    waitAction  detectNearbyAction

    -   SleepEnemy:
        These white enemies are distributed in four corners
        they will sleep for 30 seconds or be woken up by you (you are too close)
        after which they will keep hunting you

## References:
-   https://www.gamedeveloper.com/programming/behavior-trees-for-ai-how-they-work
-   https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter06_The_Behavior_Tree_Starter_Kit.pdf
-   http://www.pandabehaviour.com/demos
