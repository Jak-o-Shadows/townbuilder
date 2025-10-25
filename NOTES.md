# Townbuilder


# Desired Features
 * Paths give bonus walking speed
   * When combined with pathfinding, this will incentise the pawns to walk on the paths



# Pawn Traits
Core traits are
 * Hunger
 * Thirst
 * Cold
 * *Comfort*
   * 
 * Social
   * People connections
   * Social areas - pubs, firepits, etc.
 * Fun
   * Alcohol
 * Tiredness/Sleep
 * Age
 * Sickness??
   * Generic illness metric, or specific injuries?

Secondary traits
 * Strength
 * Speed


# Buildings
 * Forester
 * Stonecutter
 * Forager
 



&insp
 * Ixion
  * https://store.steampowered.com/app/1113120/IXION/
  * Has trains
 * Banished
 * Timberborn
 * Pharoah
  * https://store.steampowered.com/app/1351080/Pharaoh_A_New_Era/
  * Isometric
 * Golbulation 2
  * https://globulation2.org/wiki/Main_Page
 * Tiny Glade
   * ♥♥ Tilt Shift ♥♥
  


  More variety
   * More building types
   * More crop types
   * More ways to make the bars fill up
 * If things have colour variations, let the player choose them


OVERLAYS
 * Path cost


Trading
 * Hidden model in background, not just random


Use real world elevation map?

Ability to pick up pawns and just move them lol (/throw them)


Everything goes brown in summer

# Development Ethos

## Data Logging
To understand the behaviour of the game, a robust data logging system is used. This logs data to a database, using the ECS components to provide the data, which is a really neat way of not having to poke down into a class hierarchy.

A database is used because:
 * Inherently good at tabular data
 * Easy to access the logged data with common tools
 * Databases already exist - less code to write
 * Databases can be encrypted
 * Can do incremental writing, so in the case of a crash, we might have some of the data

The SOCI library is used to better support different database types. This allows logging to a sqlite3 database for on-local storage, but also lets us swap to logging to a central database when the game is released (but also letting the user opt back into local storage).

### Visualisation
As the database is a regular database, most programming languages can open them. We want to be able to visualise the data in a live dashboard, but also do auto-exports so we can setup automatic reports. We choose Python and Django for the live dashboard, with the Altair plotting library. Python supports data analysis and plotting well, and Django is a mature choice that enables quite a complex dashboard. The Altair plotting library is one of the few Python plotting libraries (the other being Matplotlib) that can export plots to png WITHOUT having to render them via a web-browser (even if that web-browser is something more like selenium - it's still a pain and not possible on the server we want to do the reports on).
