## Tests

### Testing that all the is0 nodes compile

There is a debug option in the "Debug" menu called "Test is0 Nodes" that will do just that.

### Simulation modules

Modules that need physics simulation step by step can store a `lastTimepoint` and when the need to rerender with a new timepoint they now how long has elapsed, and can run as many steps as needed, based of a step size. This both allwos to query those modules at any timepoint, and to have a step size decorelated from the update rate of the application. PB: when a parameter of those system changes,we need to run them all the way to the current timepoint, which can be slow. (can we do a preview were it runs as fast as possible to reach the current time ? But this might be confusing if time is paused but simulation still seems to run)
