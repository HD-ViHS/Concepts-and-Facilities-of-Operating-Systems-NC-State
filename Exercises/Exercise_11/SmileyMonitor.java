import java.util.Random;

public class SmileyMonitor {
  /** Number of times for each thread to print its character. */
  static final public int LIMIT = 1000;

  /**
     A thread with a function to let it wait periodically.  In
     production, you wouldn't really want or need something like this
     as part of a multi-threaded program.  Here it varies thread
     execution timing so we see more of the possible execution orders
     for our threads.  This can help give you confidence that your
     solution is working correctly. */
  static class PokyThread extends Thread {
    /** Source of randomness for sometimes waiting. */
    private Random rand = new Random();

    /** Make the caller wait between 0 and t milliseconds. */
    protected void hesitate( int t ) {
      try {
        Thread.sleep( rand.nextInt( t ) );
      } catch ( InterruptedException e ) {
      }
    }
  }

  /** Type for our monitor, to control the synchronization of our
      threads.  You'll need to extend this monitor to enforce
      the required synchronization behavior. */
  private static class MyMonitor {

    boolean eyesPrinted = false;
    boolean nosePrinted = false;
    boolean mouthPrinted = true;
      
    private synchronized void printEyes( char sym ) {

      while(!(!eyesPrinted && !nosePrinted && mouthPrinted))
	try {
	  wait();
	} catch ( InterruptedException e) {
	}

      System.out.print( sym );
      eyesPrinted = true;
      mouthPrinted = false;
      notifyAll();

    }

    private synchronized void printNose( char sym ) {

      while(!(eyesPrinted && !nosePrinted && !mouthPrinted))
        try {
	  wait();
	} catch ( InterruptedException e) {
	}
      
      System.out.print( sym );
      nosePrinted = true;
      eyesPrinted = false;
      notifyAll();

    }

    private synchronized void printMouth( char sym ) {

      while(!(!eyesPrinted && nosePrinted && !mouthPrinted))
        try {
	  wait();
	} catch ( InterruptedException e) {
	}
      
      System.out.println( sym );
      mouthPrinted = true;
      nosePrinted = false;
      notifyAll();

    }
  }

  /** One instance of our monitor, for all our threads to use. */
  private static MyMonitor monitor = new MyMonitor();
  
  static class EyePrinter extends PokyThread {
    /** Symbol this thread is supposed to print for the eyes. */
    private char sym;

    /** Make an instance of this thread that prints the given mouth symbol. */
    public EyePrinter( char sym ) {
      this.sym = sym;
    }
    
    public void run() {
      for ( int i = 0; i < LIMIT; i++ ) {
        // Wait, then print my symbol for the eyes.
        hesitate( 10 );
        monitor.printEyes( sym );
      }
    }
  }
  
  static class NosePrinter extends PokyThread {
    /** Symbol this thread is supposed to print for the nose. */
    private char sym;

    /** Make an instance of this thread that prints the given mouth symbol. */
    public NosePrinter( char sym ) {
      this.sym = sym;
    }
    
    public void run() {
      for ( int i = 0; i < LIMIT; i++ ) {
        // Wait, then print my symbol for the nose.
        hesitate( 10 );
        monitor.printNose( sym );
      }
    }
  }
  
  static class MouthPrinter extends PokyThread {
    /** Symbol this thread is supposed to print for the mouth. */
    private char sym;

    /** Make an instance of this thread that prints the given mouth symbol. */
    public MouthPrinter( char sym ) {
      this.sym = sym;
    }
    
    public void run() {
      for ( int i = 0; i < LIMIT; i++ ) {
        // Wait, then print my symbol for the mouth.
        hesitate( 10 );
        monitor.printMouth( sym );
      }
    }
  }

  public static void main( String[] args ) {
    // Make our threads, in an array so it's easy to iterate over them.
    Thread[] threads = {
      new EyePrinter( ':' ),
      new EyePrinter( '8' ),
      new EyePrinter( ';' ),
      new NosePrinter( '-' ),
      new NosePrinter( '*' ),
      new NosePrinter( '+' ),
      new MouthPrinter( ')' ),
      new MouthPrinter( '|' ),
      new MouthPrinter( '(' ),
    };

    // Start them all.
    for ( int i = 0; i < threads.length; i++ )
      threads[ i ].start();

    // Wait for them to finish.
    try {
    for ( int i = 0; i < threads.length; i++ )
      threads[ i ].join();
    } catch ( InterruptedException e ) {
    }
  }
}
