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

