# Final Project 4300 "LightFall":


## Design Documentation:



#### Game specifcation:

- The game will be comprsied of a modfied version of the Assignment 4 ECS engine.
- The tile size is 64*64 pixels
- The window size is *1344 X 768*  (20 * 12 tiles)
- Like assignment 3, the postions have been modfied such that 0,0 is the bottom left corner.


#### Level files:

- There are **no rooms**, only tiles.
- A background image may be specifed.
- TilesX/Y, gravity, speed can be *floats*.
- **Example:**
- Tile ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison
- Player TileX TileY ColBox_X ColBox_Y Speed Gravity JumpSpeed
- NPC ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison Gravity AI_Type


#### Additional components:

- NPC Behaviours: patrol, follow, hang (for spiders), boss
- Hang is the behaviour of "Spider" NPC's where they will drop down from the celling if the player is under them and then they will follow / attack the player.
- cLight => *To-do*  Should handle all light.


#### NPC's 

- Some Boss // TO-DO
- The t wo NPCs I think we could add would be the spider and zombies.
- Each NPC should have a interesting behaviour if the are caught in light. 