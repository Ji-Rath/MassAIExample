# MassAITesting
 A project primarily used to test UE5 Mass AI system
 
## Simple gather entities (Currently called RTSAgentTrait in CPP)
### AI process
- Entities go to smart objects (tree/rock) and 'collect' the resource
- Entities then go back to their 'house' which is their initial location
- Repeat until there are no more resources to gather
### Behind the scenes
- Entities use the SmartObjectSubsystem to communicate with smart objects
- This ensures that entities wont fight over a single resource
- Smart objects use simple gameplaytags to determine what is a rock and what is a tree

https://user-images.githubusercontent.com/1747157/174648031-43e36fd4-816c-42d7-b087-d4fbd3643475.mp4
 
https://user-images.githubusercontent.com/1747157/172963214-0ef75cdd-75bd-494e-8a21-666cb7a5ba35.mp4
 
https://user-images.githubusercontent.com/1747157/170729609-3c2716ae-a6a0-40c5-86bf-a437a15e6705.mp4

## Discoveries
### Mass Visualization
- Visualization Trait
- Mass Viewer Info Fragment
- Mass Actor Fragment (For actor visualization)
- Mass Visualization LOD Processor (Index 41)
- LODCollectorProcessor is disabled by default for some reason, which is needed when
entity counts are high

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
- StateTree and Fragments are sortof like BehaviorTree and Blackboard
- ObserverProcessors can observe more than one fragment/tag by overriding the
Register() function.

### State Tree Information
- Evaluators basically gather data to be used in the state tree
- Enter Conditions are used on leaf nodes to see whether a leaf should be executed
- Tasks from ST are like BT, execute logic
- Transitions allow the state tree to go to other branches based on a condition
- Reference: https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/
- Category for UPROPERTY in InstanceData determines what kind of value it is (Input, Output, Parameter)
- State Tree will throw an ensure when there are fragments missing for a task to execute.
The only way I know to debug which fragments are missing is to goto the task and look
for TStateTreeExternalDataHandle in the header file.
<details>
<summary> <b> State Tree Experimental Findings </b> </summary>
<ul>
<li>
Tick on StateTree Tasks are only ran once and with subscribed signals
(see UMassStateTreeProcessor)
</li>
<li>
I found no feasable way to subscribe signals in MassStateTreeProcessor.
As a hacky solution just reuse one of the hardcoded signals
</li>
<li>
A <b>SmartObjectDefinition</b> needs <b>USmartObjectMassBehaviorDefinition</b>
and ALL default tag filters to show on <b>Mass SmartObject Eval</b> evaluator.
</li>
<li>
<b>UseSmartObjectTask</b> will only execute
<b>USmartObjectMassBehaviorDefinition</b>, meaning only C++ logic for the time
</li>
<li>
Destroying a smart object safely in <b>USmartObjectMassBehaviorDefinition</b>
should be done using PushCommand(). Lets the <b>SmartObjectUseTask</b> release the
smart object before destruction. (may be source of ensures being fired, need
to investigate further)
</li>
<li>
Empty states with transitions seem to produce unexpected behavior. The state tree
also always needs to be in an active state, even if idle.
</li>
<li>
SmartObjectUseTask modifies MassMoveTarget around line 163-164, caused a headache
since entities would not move after using a smart object.
</li>
</ul>

</details>

### Smart Objects
- FMassSmartObjectHandler should be used rather than directly getting the smart
object subsystem in mass....i think (seems to be used in~~~~ state tree tasks).
### Instanced Static Meshes and Visualization
- Its possible to apply unique textures to each instance through an atlas and getting
a random num to choose the frame - dont forget to use
[Vertex Interpolator and use a small float to fix precision issues](https://unrealcommunity.wiki/using-per-instance-custom-data-on-instanced-static-mesh-bpiygo0s).
- Vertex animation can be achieved by a similar tactic of using instance custom data
to determine which animation to play
- To setup vertex animation, I used [Vertex_Anim_Toolset](https://github.com/BenVlodgi/Vertex_Anim_Toolset) and a UE5 fork
- In the processor, order is important when giving instance custom floats.
**FMassRepresentationFragment**, **FMassRepresentationLODFragment**, and **RepresentationSubsystem**
should be all you need to get started with instance custom data.
(See URTSAnimationProcessor)

### City Sample and AnimToTexture Plugin
#### This is in regards to how ISM processors get anim data and update custom data
- Processors get their data from MassRepresentationSubsystem. At some point,
actor templates are added via **FindOrAddTemplateActor()**. (Update: This is done for us most likely
somewhere in the visualization/representation processors if you have actor visualization)
- The CDO is then retrieved via **GetDefaultObject()** and data can be retrieved. 
In this case, it is a **CrowdCharacterDataAsset**.
- A **FCrowdCharacterDefinition** is generated based on the data asset which contains
the key info for animation among other things. (its a little more complex than described
since the data is really retrieved from the **FCrowdCharacterDefinition**, just selected
based on the human's properties)
- Finally, the animation data (**UAnimToTextureDataAsset**) is saved to the entities **FCrowdAnimationFragment** for
future use in processors
- Note: Data assets appear to be added to the character BP (high actor visualization), this is why the data can be accessed.
- The actor can be retrieved using **FMassRepresentationFragment.HighResTemplateActorIndex** and **RepresentationSubsystem->GetTemplateActorClass**
- To be honest, a simple SharedFragment is probably sufficient for simple use-cases.
I definitely might change my mind when I attempt to sync actor/ISM animation
- Actual anim state index is updated in **UMassProcessor_Animation** and custom data is updated at
**UMassCrowdUpdateISMVertexAnimationProcessor::UpdateISMVertexAnimation** in various processors

### HashGrids
- TPointHashGrid3 performance is considerably worse compared to THierarchicalHashGrid2D
- This could be caused by the extra dimension, testing scenario, or more efficient logic for mass (as THierarchicalHashGrid2D is used in MassAvoidance)
- Worst case scenario for similar search query: 235.9μs -> 8μs (x30 performance boost!)
### TODO
- Find a way to use Mass SmartObject Eval effectively in the State Tree (DONE)
- Convert logic in RTSMovementProcessor to State Tree (KINDOF DONE)
- Agent gets stuck after using one smart object, find out why (DONE)
- The UseSmartObject task also messes with the MoveTargetFragment. It seems to
be doing some stuff in ActivateActionAnimate() at MassZoneGraphNavigationUtils too.
  (DONE)
