import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.lang.InterruptedException;

/**
 * The Zerosum class contains the functionality to count all the sequences in a list of
 * numbers from standard input that add up to 0. It makes this process faster through
 * multithreading.
 */
public class Zerosum {

    /** ArrayList to hold all the integers */
    private static List<Integer> intList = new ArrayList<Integer>();

    /** Defines a Worker thread */
    private static class Worker extends Thread {
        
        /** The index the worker should start counting from */
        private int start;

        /** The number of starting indices the worker jumps every time it
         * finishes counting the whole sequence in intList (for the purpose
         * of making sure no series get double counted)
         */
        private int step;

        /**
         * Whether or not the worker should report its findings.
         */
        private boolean report;

        /**
         * The number of sequences the worker has found.
         */
        public int result = 0;

        /**
         * Constructor for a worker
         * 
         * @param start the starting index for the worker
         * @param step the number of indeces to jump after counting all 
         *             sequences from a starting index (will always be
         *             the total number of workers)
         * @param report whether the program is called in report mode
         */
        public Worker (int start, int step, boolean report) {
            this.start = start;
            this.step = step;
            this.report = report;
        }

        /**
         * The function the thread runs. It counts all the sequences which
         * are the thread's responsibilty to count which add to 0 and sums
         * them in result.
         */
        public void run() {
            for(int i = start; i < intList.size(); i += step) {
		        int currentTotal = 0;
                for(int j = i; j < intList.size(); j++) {
                    currentTotal += intList.get(j);
                    if(currentTotal == 0) {
                        result++;
                        if(report) {
                            System.out.printf("%d .. %d\n", i, j);
                        }
                    }
                }
            }
        }
    }

    /**
     * Starts the program
     * 
     * @param args command-line arguments used to specify how many workers to deploy and
     *             whether to operate in command mode.
     */
    public static void main(String[] args) {

        // Total count from all workers
        int finalTalley = 0;

        // Ensure fidelity of arguments
        if(args.length > 2) {
            System.out.printf("usage: java Zerosum <workers>%n");
            System.out.printf("       java Zerosum <workers> report%n");

            return;
        }

        // How many workers?
        int numWorkers = Integer.parseInt(args[0]);

        // Are we operating in report mode?
        boolean report = (args.length == 2 && "report".equals(args[1]));

        // Scanner for retrieving input from standard input
        Scanner input = new Scanner(System.in);

        // Build the intList
        while(input.hasNextInt()) {
            intList.add(input.nextInt());
        }

        // No leaky memory (close the scanner)
        input.close();

        // Build a worker array to organize the workers by index
        Worker[] workers = new Worker[numWorkers];

        // Create and start each worker
        for(int i = 0; i < numWorkers; i++) {
            workers[i] = new Worker(i, numWorkers, report);
            workers[i].start();
        }

        try {
            // Wait for each worker
            for(int i = 0; i < numWorkers; i++) {
		        workers[i].join();
                // Add each worker's private sum to the total sum
		        finalTalley += workers[i].result;
            }
        } catch ( InterruptedException e ) {
            System.out.println( "Interrupted during join!" );
        }

        // Print the total sum
	    System.out.printf("Total: %d%n", finalTalley);

    }
}
