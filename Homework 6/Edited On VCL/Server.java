import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Scanner;
import java.util.Random;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.PublicKey;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.security.GeneralSecurityException;
import java.util.Base64;


/** A server that keeps up with a public key for every user, scrabble
 * word scores for all users. */
public class Server {
  /** Port number used by the server */
  public static final int PORT_NUMBER = 26108;

  /** Maximum length of a username. */
  public static int NAME_MAX = 10;
  
  /** Maximum length of a word. */
  public static int WORD_MAX = 24;
    
  /** State for recording user submissions */
  private class Submission {
    private String user;
    private String word;
    private int value;

    /** Constructor for a Submission object */
    public Submission(String user, String word, int value) {
      this.user = user;
      setWord(word);
      setValue(value);
    }

    /** Getters and Setters for fields of Submission */
    public String getUser() {
      return user;
    }

    public String getWord() {
      return word;
    }

    public int getValue() {
      return value;
    }

    private void setWord(String word) {
      this.word = word;
    }

    private void setValue(int value) {
      this.value = value;
    }

    /** Overridden equals() and hashCode() methods (not necessary, but included out of habit) */
    @Override
    public boolean equals(Object o) {
      if(o instanceof Submission) {
	Submission test = (Submission) o;
	return (test.getUser().equals(getUser()) && test.getWord().equals(getWord()) && test.getValue() == getValue());
      } else {
	return false;
      }
    }

    @Override
    public int hashCode() {
      return getValue();
    }
  }

  /** A thread definition. A thread, when run, simply runs the handleClient() function on the socket passed to it. */
  class MyThread extends Thread {

    /** The socket this thread runs on. */
    private Socket sock;

    /** Constructor for the thread. */
    public MyThread( Socket sock ) {
      this.sock = sock;
    } 

    /** Run the handleClient() function. */
    public void run() {
      handleClient( sock );
    }
  }

  /** A comparator which sorts Submission objects by their value in ascending order */
  class SortByValue implements Comparator<Submission> {
      
    public int compare(Submission a, Submission b) {
      return a.getValue() - b.getValue();
    }
      
  }

  /** A lock for synchronization */
  private static Object lock = new Object();

  /** A list of submissions to keep track of user submissions */
  private static volatile ArrayList<Submission> submissions = new ArrayList<Submission>();
  
  /** Record for an individual user. */
  private static class UserRec {
    // Name of this user.
    String name;

    // This user's public key.
    PublicKey publicKey;
  }

  /** List of all the user records. */
  private ArrayList< UserRec > userList = new ArrayList< UserRec >();

  /** Read the list of all users and their public keys. */
  private void readUsers() throws Exception {
    Scanner input = new Scanner( new File( "passwd.txt" ) );
    while ( input.hasNext() ) {
      // Create a record for the next user.
      UserRec rec = new UserRec();
      rec.name = input.next();

      // Get the key as a string of hex digits and turn it into a byte array.
      String base64Key = input.nextLine().trim();
      byte[] rawKey = Base64.getDecoder().decode( base64Key );
    
      // Make a key specification based on this key.
      X509EncodedKeySpec pubKeySpec = new X509EncodedKeySpec( rawKey );

      // Make an RSA key based on this specification
      KeyFactory keyFactory = KeyFactory.getInstance( "RSA" );
      rec.publicKey = keyFactory.generatePublic( pubKeySpec );

      // Add this user to the list of all users.
      userList.add( rec );
    }
  }

  /** Utility function to read a length then a byte array from the
      given stream.  TCP doesn't respect message boundaraies, but this
      is essientially a technique for marking the start and end of
      each message in the byte stream.  As a public, static method,
      this can also be used by the client. */
  public static byte[] getMessage( DataInputStream input ) throws IOException {
    int len = input.readInt();
    byte[] msg = new byte [ len ];
    input.readFully( msg );
    return msg;
  }

  /** Function analogous to the previous one, for sending messages. */
  public static void putMessage( DataOutputStream output, byte[] msg ) throws IOException {
    // Write the length of the given message, followed by its contents.
    output.writeInt( msg.length );
    output.write( msg, 0, msg.length );
    output.flush();
  }

  /** Private helper function for calculating a word's value. */
  private int getPointValue( String word ) {
    // A *Pointers string defines all characters worth * points.
    String onePointers = "aeioulnstrAEIOULNSTR";
    String twoPointers = "dgDG";
    String threePointers = "bcmpBCMP";
    String fourPointers = "fhvwyFHVWY";
    String fivePointers = "kK";
    String eightPointers = "jxJX";
    String tenPointers = "qzQZ";
    
    int total = 0;
    for(int i = 0; i < word.length(); i++ ) {
      char current = word.charAt(i);
      if( onePointers.indexOf( current ) >= 0 ) {
	      total++;
      } else if( twoPointers.indexOf( current ) >= 0 ) {
	      total += 2;
      } else if( threePointers.indexOf( current ) >= 0 ) {
	      total += 3;
      } else if( fourPointers.indexOf( current ) >= 0 ) {
	      total += 4;
      } else if( fivePointers.indexOf( current ) >= 0 ) {
	      total += 5;
      } else if( eightPointers.indexOf( current ) >= 0 ) {
	      total += 8;
      } else if( tenPointers.indexOf( current ) >= 0 ) {
	      total += 10;
      } else {
	      return -1;
      }
    }
    return total;
  }
      
  /** Handle interaction with our client. */
  public void handleClient( Socket sock ) {
    try {
      // Get formatted input/output streams for this thread.  These can read and write
      // strings, arrays of bytes, ints, lots of things.
      DataOutputStream output = new DataOutputStream( sock.getOutputStream() );
      DataInputStream input = new DataInputStream( sock.getInputStream() );
      
      // Get the username.
      String username = input.readUTF();

      // Make a random sequence of bytes to use as a challenge string.
      Random rand = new Random();
      byte[] challenge = new byte [ 16 ];
      rand.nextBytes( challenge );

      // Make a session key for communiating over AES.  We use it later, if the
      // client successfully authenticates.
      byte[] sessionKey = new byte [ 16 ];
      rand.nextBytes( sessionKey );

      // Find this user.  We don't need to synchronize here, since the set of users never
      // changes.
      UserRec rec = null;
      for ( int i = 0; rec == null && i < userList.size(); i++ )
        if ( userList.get( i ).name.equals( username ) )
          rec = userList.get( i );

      // Did we find a record for this user?
      if ( rec != null ) {
        // Make sure the client encrypted the challenge properly.
        Cipher RSADecrypter = Cipher.getInstance( "RSA" );
        RSADecrypter.init( Cipher.DECRYPT_MODE, rec.publicKey );
          
        Cipher RSAEncrypter = Cipher.getInstance( "RSA" );
        RSAEncrypter.init( Cipher.ENCRYPT_MODE, rec.publicKey );
          
        // Send the client the challenge.
        putMessage( output, challenge );
          
        // Get back the client's encrypted challenge.
        byte[] encryptedChallenge = getMessage(input);

        // Make sure the client properly encrypted the challenge.
        byte[] decryptedChallenge = RSADecrypter.doFinal(encryptedChallenge);
	if(! Arrays.equals(challenge, decryptedChallenge)) {
	    throw new GeneralSecurityException("Challenge string could not be decryted by server");
	}

        // Send the client our session key (encrypted)
        byte[] encryptedSessionKey = RSAEncrypter.doFinal(sessionKey);
	putMessage(output, encryptedSessionKey);

        // Make AES cipher objects to encrypt and decrypt with
        // the session key.
        SecretKey key = new SecretKeySpec( sessionKey, "AES" );
	Cipher sessionKeyEncrypter = Cipher.getInstance( "AES/ECB/PKCS5Padding" );
	sessionKeyEncrypter.init( Cipher.ENCRYPT_MODE, key );

	Cipher sessionKeyDecrypter = Cipher.getInstance( "AES/ECB/PKCS5Padding" );
	sessionKeyDecrypter.init( Cipher.DECRYPT_MODE, key );

        // Get the first client command
        String request = new String( sessionKeyDecrypter.doFinal( getMessage( input ) ) );

        // All requests start with a verb.
        while ( ! request.equals( "quit" ) ) {
          StringBuilder reply = new StringBuilder();
	  
	  // User wants to query a word
	  if ( request.startsWith( "query " ) ) {
	    Scanner requestScanner = new Scanner(request);
	    requestScanner.next(); // Move past 'query'
	    String word = requestScanner.next(); // Get the word
	    // If the word is too long, the command is longer than two words, or the word is more than just alphabetical chars
	    if( word.length() > WORD_MAX || requestScanner.hasNext() || ! word.matches( "[a-zA-Z]+" ) ) {
	      reply.append("Invalid command\n");
	    } else {
	      int value = getPointValue( word );
	      if( value < 0 ) { // getPointValue returns -1 on an invalid word.
		reply.append( "Invalid command\n" ); // Technically, this line should be impossible to reach, but I'm coding java like I code C now, always slightly terrified.
	      } else {
		reply.append( value );
		reply.append( "\n" );
	      }
	    }
	  // User wants to submit a word
	  } else if ( request.startsWith( "submit " ) ) {
	    Scanner requestScanner = new Scanner(request);
	    requestScanner.next(); // Move past 'submit'
	    String word = requestScanner.next(); // Get the next word
	    // If the word is too long, the command is longer than two words, or the word is more than just alphabetical chars
	    if( word.length() > WORD_MAX || requestScanner.hasNext() || ! word.matches( "[a-zA-Z]+" ) ) { 
	      reply.append("Invalid command\n");
	    } else {
	      Submission test = new Submission( rec.name, word, getPointValue(word) ); // Create a new test Submission object
	      synchronized ( lock ) { // Block access to the shared submissions state
		boolean edited = false;
		for(Submission s : submissions) {
		  if(s.getUser().equals( test.getUser() ) ) {
		    s.setValue(test.getValue());
		    s.setWord(test.getWord());
		    edited = true;
		    break;
		  }
		}
		if(! edited ) {
		  submissions.add( test ); // User has not yet submitted a word.
		}
	      } // Out of locked access
	    }
	  // User wants a report of submitted words
	  } else if ( request.startsWith( "report" ) ) {
	    Scanner requestScanner = new Scanner( request );
	    // If the next word isn't 'report' or there's more than just the word 'report' in the command
	    if(! requestScanner.next().equals("report") || requestScanner.hasNext() ) { 
	      reply.append( "Invalid command\n" );
	    } else {
	      Comparator<Submission> c = new SortByValue(); // Comparator to sort submissions by value
	      synchronized( lock ) { // Block access to the shared submissions state
		submissions.sort( c ); // Sort the list of submissions by their value
		for( Submission s : submissions ) { // Print every submission in the sorted collection
		  reply.append(String.format("%10s %24s %3d\n", s.getUser(), s.getWord(), s.getValue()));
		}
	      } // Out of locked access
	    }  
	  } else {
	    reply.append("Invalid command\n");
	  }
              
          // Send the reply back to our client
          putMessage( output, sessionKeyEncrypter.doFinal( reply.toString().getBytes() ) );
              
          // Get the next command.
          request = new String( sessionKeyDecrypter.doFinal( getMessage( input ) ) );
        }
      }
    } catch ( IOException e ) {
      System.out.println( "IO Error: " + e );
    } catch( GeneralSecurityException e ){
      System.err.println( "Encryption error: " + e );
    } finally {
      try {
        // Close the socket on the way out.
        sock.close();
      } catch ( Exception e ) {
      }
    }
  }

  /** Esentially, the main method for our server, as an instance method
      so we can access non-static fields. */
  private void run( String[] args ) {
    ServerSocket serverSocket = null;
    
    // One-time setup.
    try {
      // Read the map and the public keys for all the users.
      readUsers();

      // Open a socket for listening.
      serverSocket = new ServerSocket( PORT_NUMBER );
    } catch( Exception e ){
      System.err.println( "Can't initialize server: " + e );
      e.printStackTrace();
      System.exit( 1 );
    }
     
    // Keep trying to accept new connections and serve them.
    while( true ){
      try {

	// Try to get a new client connection.
        Socket sock = serverSocket.accept();

	// Create a new thread base on the socket connection and start it
	MyThread client = new MyThread( sock );
	client.start();
        
      } catch( IOException e ){
        System.err.println( "Failure accepting client " + e );
      }
    }
  }

  public static void main( String[] args ) {
    // Make a server object, so we can use non-static fields and methods.
    Server server = new Server();
    server.run( args );
  }
}
