# Final Project 4300 "LightFall":


## Design Documentation:

#### Game specifcation:

- The game will be comprsied of a modfied version of the Assignment 4 ECS engine.
- The tile size is 64*64 
- The window size is 1344 X 768  (20 * 12 tiles)
- Like assignment 3, the postions have been modfied such that 0,0 is the bottom left corner.



#### Level files:

- Unlike the asignments, the player is specifed by tiles.
- There are no rooms, only tiles.
- A background image may be specifed.
- All NPC's have a gravity compontent.
- TilesX/Y, gravity, speed can be floats. 
- Example:
- Tile ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison
- Player TileX TileY ColBox_X ColBox_Y Speed Gravity JumpSpeed
- NPC ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison Gravity AI_Types
- 
-AI_Types
-
#### Additional components:

- NPC Behaviours: patrol, follow, hang.
- Hang is the behaviour of "Spider" NPC's where they will drop down from the celling if the player is under them and then they will follow / attack the player.
- cLight => To-do, Should handle all light.


#### NPC's 

- We need a good idea for a boss.
- The two NPCs I think we could add would be the spider and zombies.
- I think each NPC should have a interesting behaviour if the are caught in light.