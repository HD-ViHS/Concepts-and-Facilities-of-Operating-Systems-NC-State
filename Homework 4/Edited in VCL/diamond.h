#ifndef __DIAMOND_H__
#define __DIAMOND_H__

/** Interface for the Diverging Diamond monitor.  If I was programming in
    Java or C++, this would all be wrapped up in a class.  Since we're using
    C, it's just a collection of functions, with an initialization
    function to init the state of the whole monitor. */

/** Initialize the monitor. */
void initMonitor();

/** Destroy the monitor, freeing any resources it uses. */
void destroyMonitor();

/** Ehter the west side of the intersection, heading north-east. */
void enterNE( char const *name );

/** Leave the west side of the intersection, heading north-east. */
void leaveNE( char const *name );

/** Ehter the west side of the intersection, heading north-west. */
void enterNW( char const *name );

/** Leave the west side of the intersection, heading north-west. */
void leaveNW( char const *name );

/** Ehter the east side of the intersection, heading south-east. */
void enterSE( char const *name );

/** Leave the east side of the intersection, heading south-east. */
void leaveSE( char const *name );

/** Ehter the east side of the intersection, heading south-west. */
void enterSW( char const *name );

/** Leave the east side of the intersection, heading south-west. */
void leaveSW( char const *name );

#endif
