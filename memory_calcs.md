## Pawn Memory Size
100 of:
 flecs::id_t = 4
 Position = 4 * 2
 Velocity = 4 * 2
 PawnOccupying = flecs::id_t (4) + flecs::id_t (4) = 4 * 2
 PawnPathfindingGoal = flecs::id_t (4) + flecs::id_t (4) = 4 * 2
 PawnNextCell = flecs::id_t (4) + flecs::id_t (4) = 4 * 2
 --> 44 bytes
--> 4.4 kB

## Map Memory Size
### Grid Map
map_width * map_height of:
 flecs::id_t = 4
 GridCellStatic = 4 * 2 (2 ints) + 4 (1 int)
 --> map_width * map_height * 16 byte
--> (20x20) = 6.25 kB
--> (1024*1024) = 16 MB

### Template Grid
map_width * map_height of:
 T: (your defined)


