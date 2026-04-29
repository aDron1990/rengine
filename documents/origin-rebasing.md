# Origin Rebasing

The engine keeps two position spaces for large simulations:

- `WorldPosition::positionKm` stores large-space coordinates in `double`, where `1.0` is one kilometer.
- `Transform::position` remains the local engine coordinate in `float`, where `1.0f` is one meter. Rendering, physics, picking, cameras, and bounding boxes continue to use this local transform.

`OriginRebaseSystem` owns the current local origin in kilometers. It converts world positions to local transforms with:

```cpp
transform.position = glm::vec3((worldPosition.positionKm - originKm) * 1000.0);
```

The simulation area is six kilometers wide around the local origin. An entity with `OriginAnchor` drives rebasing. When that anchor moves at least three kilometers away from the current origin on any axis, the origin snaps to the nearest three-kilometer step toward the anchor and all `WorldPosition + Transform` entities are resynchronized.

`OrbitalEngine` reads and writes `WorldPosition` and uses `glm::dvec3`/`double` values in kilometers. `OrbitalBody::velocityKmPerSec` is measured in kilometers per second, and `Celestial::GM` is measured in cubic kilometers per second squared. Orbit preview lines are converted back to local meters before they are stored in `LineRenderer`.

Physics remains local-space for now. Bodies controlled by orbital simulation are pushed into Jolt by syncing `WorldPosition -> Transform` and then applying the local transform to the physics body.
