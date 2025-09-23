import brock.*;

public class Example1 {

    /**
     * <p>
     *      In Java, all programs start with a main function.
     *      The main function always takes an array of Strings which are the
     *      arguments passed to the program.
     * </p>
     * <p>
     *      If you have never used the command line and have no idea what that means,
     *      you can ignore it for now... Know that all Java programs *must* have a function
     *      named exactly like this, with this exact signature to be able to run.
     * </p>
     *
     * <p>
     *      BlueJ Abstracts away the main function, but
     *      I think this leads to a rather counter-intuitive
     *      view of how the program actually runs.
     * </p>
     */
    public static void main(String[] args) {
        /*
         * Now this is a line you've probably seen before but may not understand.
         * The professors at Brock have designed a library which attempts to make learning Java programming more engaging.
         * We do this through the use of the turtle graphics system.
         *
         * In simple terms, the TurtleDisplayer class is a canvas. There's a lot of internal state management that goes
         * into make windows appear on your desktop and capable of drawing.
         *
         * you may notice the `new` here, and you may know that this allocates memory for you.
         * In Java this goes a bit deeper. With a big asterisk this is the only way to construct objects in Java.
         * You get absolutely no say in where, when, or how it is allocated, only that it will return a reference
         * to an object somewhere in memory that directly corresponds to the class you asked to be allocated.
         *
         * Knowing beyond this isn't necessary to use Java, and to be honest, I didn't learn about it until about a year
         * after I stopped using Java regularly. If you can accept that this is how objects are created, then you'll do fine!
         */
        TurtleDisplayer display = new TurtleDisplayer();
        /*
         * The same applies for our turtle here. We are asking the JVM to allocate a new Turtle for us, which we are storing
         * as a reference called "yertle"
         *
         * It is important to note here that when you use the variable,
         * Java passes what is known as a **reference copy** to the underlying value.
         *
         * This means that the name "yertle" is never known to the program.
         * It is never used as an identifier and is only used for you (the programmer) to keep track of the object with a common name.
         */
        Turtle yertle = new Turtle();

        /*
         * If you are confused about what "using a variable" is, consider the following examples:
         */

        /*
         * This function call is using the variable yertle.
         * Again, this passes a copy of the reference to the Turtle object known as 'yertle'
         * the function "placeTurtle" is then free to do whatever it wants with the turtle.
         * Any function calls it makes on the turtle or otherwise changing the state of the turtle is reflected in our
         * 'yertle' object because as I keep saying, Java passes around reference copies. Meaning that this variable
         * "references" the Turtle object in memory allocated at the previous line.
         *
         * Now at this point you may be wondering why I keep using the explicit term "reference copy" and
         * the real knowledgeable of you knows this distinction is nonsense.
         *
         * The reason to make this distinction is that in other languages which allow references (mainly C++)
         * reassigning to the reference will change the underlying variable, while in Java it does not.
         *
         * If you still don't understand, that's fine; I will go into more detail later,
         * just for now know that *object* variables in Java
         * are references that share the same underlying memory.
         */
        display.placeTurtle(yertle);

        /*
         * Another common usage of variables is to print them!
         */
        System.out.println(yertle);
        /*
         * In this case, what do you think this will do?
         *
         * If you said it will print "yertle" to the console, you would be incorrect. Do you know why?
         *
         *
         *
         * Answer:
         *
         * This goes back to variables being references. Again, the 'yertle' identifier is just a name we use to identify
         * an object. It has no real meaning to the compiler beyond keeping track of what objects exist where.
         *
         * So what this does is it will print the object yertle. Because yertle doesn't define a toString() method,
         * it will output the *full* name of the class along with a memory location representing where the reference points in memory.
         *
         * e.g.:
         * `brock.Turtle@48524010`
         */

        display.waitForUser();



    }

}