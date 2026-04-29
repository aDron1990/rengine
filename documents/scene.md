# Scene

`Scene` is the owner of an `entt::registry` and of high-level `Object` instances that need per-frame updates.

The registry remains the storage for components and systems. Systems should continue to receive or access `entt::registry&` so render, physics, orbit, and origin-rebase code can iterate component views directly.

Objects are lightweight C++ behavior wrappers around entities. `Scene::createObject<T>()` constructs an object with the scene registry, stores it as a `std::unique_ptr<Object>`, and returns a reference for setup code. The object destructor destroys its entity if it is still valid.

`Scene::update()` iterates owned objects in creation order and calls `Object::update()`. There is no object hierarchy yet, so parent-child transform propagation, activation inheritance, and ordered tree traversal are intentionally out of scope.

Free entities can still be created with `Scene::createEntity()` when an entity only needs components and does not need an object wrapper.
