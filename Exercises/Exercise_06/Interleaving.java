/**
 * The Interleaving class creates three Thread objects, and runs the
 * runnables "MyRunnable1", "MyRunnable2", and "MyRunnable3" to print
 * 'a' and 'b', 'c' and 'd', and 'e' and 'f' respectively.
 * 
 * @author Siobhan McCarthy
 */
public class Interleaving {
    
    /** Thread to print ab. */

    static class MyRunnable1 implements Runnable {
	public void run() {
	    System.out.print('a');
	    System.out.print('b');
	}
    }

    /** Thread to print cd. */

    static class MyRunnable2 implements Runnable {
	public void run() {
	    System.out.print('c');
	    System.out.print('d');
	}
    }

    /** Thread to print ef. */

    static class MyRunnable3 implements Runnable {
	public void run() {
	    System.out.print('e');
	    System.out.print('f');
	}
    }

    /**
     * Starts the program.
     * 
     * @param args non-relevant command-line arguments
     * @author Siobhan McCarthy
     */
    public static void main( String[] args ) {
	// The three threads we make.
	Thread tab, tcd, tef;
  
	// A bunch of times.
	for ( int i = 0; i < 50000; i++ ) {
    	// Make one of each type of threads.
	    
	    tab = new Thread( new MyRunnable1() );
	    tcd = new Thread( new MyRunnable2() );
	    tef = new Thread( new MyRunnable3() );
	    
            // Start them all.

	    tab.start();
	    tcd.start();
	    tef.start();

	    // Join with our three threads.

	    try {
		tab.join();
	    } catch ( InterruptedException e ) {
		System.out.println( "threadab interrupted durring join!" );
            }
	    
	    try {
		tcd.join();
	    } catch ( InterruptedException e ) {
		System.out.println( "threadcd interrupted durring join!" );
	    }

	    try {
		tef.join();
	    } catch ( InterruptedException e ) {
		System.out.println( "threadef interrupted durring join!" );
	    }

	    // Print a newline to end the line they just printed.
	    System.out.println();
	}
    }
}
