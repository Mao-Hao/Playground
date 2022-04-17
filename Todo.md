

[√] shared_ptr<BehaviorTree> bt = make_shared<BehaviorTree>(*this);

[√] addGenericCompenent(bt);

[ ] bt->update(level)

Action要访问GameObject乃至更多东西以便更改世界
想法1: 用基类的tick和update传递GameObject
想法2: Action行为创建的时候，把GameObject和它需要的数据塞进去

