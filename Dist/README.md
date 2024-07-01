<pre>

                     ___              __           _     __
                    /   |  ____  ____/ /________  (_)___/ /____
                   / /| | / __ \/ __  / ___/ __ \/ / __  / ___/
                  / ___ |/ / / / /_/ / /  / /_/ / / /_/ (__  )
                 /_/  |_/_/ /_/\__,_/_/   \____/_/\__,_/____/
                                                                                                                                  
                                 version 1.5
                     Copyright (c) 2023 Elwynor Technologies

</pre>

## ANDROIDS!

 The year is 2089.  Humans have merged with machines to become the ultimate 
 power of the known universe:  ANDROIDS!
 
 The Androids stay in fighting trim by playing a game in which "prizes" of
 different values appear at random at various locations within an enclosed
 playing area. The object of the game is to accumulate as many points as
 possible, both by beating your opponents to the prizes, and by "ramming" your
 opponents to take their points away.  You may become invisible at any time to
 avoid pursuit or to ambush your opponents.  Special high-voltage obstacles
 litter the playfield, and the border is made of duodenum alloy... points are
 deducted for colliding with any of these. "Top-Three" listings include Total
 Points and Points Per Hour.

## WHAT IS ANDROIDS?

 Androids! was the first real-time multi-player ANSI-graphics game developed
 for use with The Major BBS.

 Each player is represented by a letter (or sometimes a question mark) that
 moves around under that player's control, picking up "prizes" (represented by
 digits, question marks, or diamonds), and trying to "ram" the letters
 representing the other players.

 Running over a prize adds that prize's value to your score, and makes the
 prize disappear.  Similarly, when you ram into another player, points are
 subtracted from that player's score and added to yours (assuming the other
 player has any points to give you), and the other player is sometimes
 teleported to a random new location.  Each time you collide with a wall or any
 of the obstacles in the playfield, you lose points.

## GAMEPLAY

 If your system-type is "IBM PC or compatible", then "IBM extended character
 set graphics" are used to display the playfield.  Otherwise, simple @-signs
 are used, although they may appear in different colors if your system has
 color-display capability.

 Upon entering the game, you're given this prompt:

 Enter 'P' to play, 'T' for Top-Three, 'D' for details, or 'X' to exit:

 If 'P' is selected, then the screen clears and the playfield is drawn. If your
 cursor-movement keys generate ANSI output, you can use them to move around.
 This works under standard PC comm programs such as ProComm and Telix.
 Otherwise, use the diamond-shaped set of keys I, J, K, and M:  I means "move
 up", J is "move left", K is "move right", and M is "move down".  Press the
 space bar to become invisible.  Press  ENTER at any time to interrupt play.

 Top-Three listings are maintained in two categories:  total points, and
 points-per-hour.  This is because total points achieved can be arbitrarily
 high, if a user is willing to stay on long enough.  So in a sense, a top
 rating in points-per-hour is a better indicator of game skill, whereas total
 points measures perseverance.

## INSTALLATION AND CONFIGURATION
 
 Simply unzip the archive to your Major BBS server directory, add 
 it to your menu, configure the MSG file to your liking, and start 
 The Major BBS!  It's that easy! 
 
 Androids has one message file. Within it, in LEVEL4 you can configure the 
 points impact of various activities as well as maximum concurrent players. 
 
 There are many text blocks to customize in LEVEL6
 
## MODULE HISTORY
 
 The Major BBS when running under DOS included a hook into the 18hz realtime
 interrupt. This allowed the BBS and GSBL to stack functions that would be 
 called every time this timer cycled. Whereas the "realtime kick" routine
 could be as often as once per second, this realtime hook ran about 55 times
 per second - much faster, and enabled realtime activities. Of course, games
 came quickly. The GALACTICOMM Flash Protocol was one implementation. Access
 was through the teleconference and required users to have custom game programs
 on their machine to play. But the BBS itself could also offer realtime 
 multiuser services. Androids was written in 1989 by Tim Stryker to demonstrate
 this capability. Other ISVs wrote games that used it afterward, although it 
 was never super popular.
 
 The UNIX and NT versions of The Major BBS and Worldgroup never implemented a
 realtime timer. It was re-added in Beta R30 of The Major BBS V10 by Rick 
 Hadsall, so Androids was ported and resurrected.

 Androids was acquired by Elwynor Technologies in 2005 along with the rest of 
 the Galacticomm intellectual property. It was ported to The Major BBS v10 
 in May 2023.

  
## LICENSE

 This project is licensed under the AGPL v3. Additional terms apply to 
 contributions and derivative projects. Please see the LICENSE file for 
 more details.

## CONTRIBUTING

 We welcome contributions from the community. By contributing, you agree to the
 terms outlined in the CONTRIBUTING file.

## CREATING A FORK

 If you create an entirely new project based on this work, it must be licensed 
 under the AGPL v3, assign all right, title, and interest, including all 
 copyrights, in and to your fork to Rick Hadsall and Elwynor Technologies, and 
 you must include the additional terms from the LICENSE file in your project's 
 LICENSE file.

## COMPILATION

 This is a Major BBS v10 module. To compile for Major BBS v10, import this
 project folder in the isv/ subtree of Visual Studio 2022, right click the 
 project name and choose build! When ready to build for "release", ensure you
 are building for release. Note that this could build but will not play on
 Worldgroup 3.2 due to the lack of support for the realtime handler.

## PACKING UP

 The DIST folder includes all of the items that should be packaged up in a 
 ELWAND.ZIP. When unzipped in a Major BBS V10 installation folder, it 
 "installs" the module.
