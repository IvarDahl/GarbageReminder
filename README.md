# GarbageReminder
An Arduino thingy that displays the next garbage date and reminds you to take your garbage containers to the driveway


/*
 * GarbageReminder v1.0
 * Written by Ivar Dahl (ivar.dahl@gmail.com)
 * 
 * Feel free to copy, tweak, share, sell or whatever you want to do. This is just a hobby, and if someone else likes it, that's great :)
 * This code is somewhat spaghetti. In the next version I will add a SD card reader/writer, to store the calendar (or maybe the garbage company has an API?). 
 * That makes it easier to maintain the calendar.
 * 
 * In this version: 
 * Garbage dates in this area are on a two week schedule. Every other garbage day, they pick up unsorted garbage. 
 * The other garabage date, they pick up unsorted AND sorted (plastic and paper).
 * 
 * The code calculates the next garbarge date (14 days ahead), when the user presses the button on the current garbage date. 
 * As garbage day in this area is Monday, the system first alerts on the preceding Thursday, just in case we plan to leave for the weekend.
 * When the user acknowledge the alert, the system waits until Sunday evening, and alerts again. When acknowledged on Sunday, the final alert activates
 * on Monday morning. When aknowledged on Monday, the system updates and displays the next garbage date. This date is stored in EEPROM.
 * The LED flashes once (repetetive) if it is time for unsorted only, and flashes three times (repetetive) if it's time for unsorted AND sorted garbage.
 * 
 * As some garbage dates don't follow the standard 14 day schedule (due to holidays etc), there are som hard coded exceptions in the code.
 * 
 * To set date and time from serial monitor command line:
 * ddmmyyyyhhmmsst [ENTER] (day month year hour minute seconds) t = instruct prog to set time
 * Eks: 01052019120000t = 01.05.19 12:00:00 (t = Set Time)
 * 
 * To set next alert date from serial monitor command line:
 * ddmmyyyyhhmm[1|0]n [ENTER](day month year hour minute [restavfall, mat|alle spann] n = instruct prog to set and save next alert time
 * Eks: 2005201918001 = Set next alert to 20.05.19 18:00 (Last digit 1 = Rastavfall + mat - If set to 0 -> Rastavfall, mat, papp, plast)
 * 
