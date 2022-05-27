# MassAITesting
 A project primarily used to test UE5 Mass AI system
 
## Simple gather entities (Currently called RTSMovementTrait in CPP)
### AI process
- Entities go to smart objects (tree/rock) and 'collect' the resource
- Entities then go back to their 'house' which is their initial location
- Repeat until there are no more resources to gather
### Behind the scenes
- Entities use the SmartObjectSubsystem to communicate with smart objects
- This ensures that entities wont fight over a single resource
- Smart objects use simple gameplaytags to determine what is a rock and what is a tree

https://user-images.githubusercontent.com/1747157/170729609-3c2716ae-a6a0-40c5-86bf-a437a15e6705.mp4


## Discoveries
### Mass Visualization
- Visualization Trait
- Mass Viewer Info Fragment
- Mass Actor Fragment (For actor visualization)
- Mass Visualization LOD Processor (Index 41)

### Mass LOD
- Mass LOD Collector Trait
- Mass LOD Collector Processor (Index 17)

### Agent Radius
- Using Assorted Fragments and Agent Radius Fragment will allow you
to override the default radius of agents for the entity

### Movement Trait
- The height is simply interpolated based on the target location
and current height. It should not be relied on to give completely
accurate results when the distance is far. A possible solution
would be to update the target location z during movement.

