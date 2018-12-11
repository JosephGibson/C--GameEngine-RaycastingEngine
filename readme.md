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
- **Example:** (out of date update later)
- Tile ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison
- Player TileX TileY ColBox_X ColBox_Y Speed Gravity JumpSpeed
- NPC ANIMATION_NAME TileX TileY Blocks_Movement Blocks_Vison Gravity AI_Type


####  Whats Done:
- Moving Tiles implemented. (Only works vertically!)
- Sound, and sound systems implemented.
- Steer implemented (See "eye".)
- Ray-Casting (Light duh.)
- Gravity and acceleration. 
- A "Goal" objective (its a purple portal)
- Weapons
- Power ups.
- Combat system

####  TO-DO:
- Code clean up
- Level 2 50% done
- Level 3 
- Write up.