# MassAITesting
 A project primarily used to test UE5 Mass AI system

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

### General Mass Information
- Fragments hold data (FMassFragment)
- Filter entities using tags (FMassTag)
- Traits contain fragments/tags (UMassEntityTraitBase)
- Processors use fragment data to perform tasks on entities (UMassProcessor)
- StateTree and Fragments is sortof like BehaviorTree and Blackboard

### State Tree Information
- Evaluators basically gather data to be used in the state tree
- Enter Conditions are used on leaf nodes to see whether a leaf should be executed
- Tasks from ST are like BT, execute logic
- Transitions allow the state tree to go to other branches based on a condition
- Reference: https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/
- Category for UPROPERTY in InstanceData determines what kind of value it is (Input, Output, Parameter)

### Smart Objects
- FMassSmartObjectHandler should be used rather than directly getting the smart
object subsystem in mass....i think (seems to be used in state tree tasks).
### TODO
- Find a way to use Mass SmartObject Eval effectively in the State Tree
- Convert logic in RTSMovementProcessor to State Tree