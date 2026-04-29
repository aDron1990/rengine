# Orbit Camera

`OrbitCamera` stores distance from its target and derives the camera transform from yaw, pitch, and that distance each update.

Mouse wheel zoom is scaled by the current distance. This makes far-away zoom changes larger while keeping close-range zoom precise. The distance is clamped to a small positive minimum so the camera never crosses the target point and the view direction does not invert.
