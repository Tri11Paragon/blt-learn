import brock.Turtle;
import brock.TurtleDisplayer;

public class Homework2 {

    TurtleDisplayer display;
    Turtle yertle;

    public Homework2() {
        display = new TurtleDisplayer();
        yertle = new Turtle();

        display.placeTurtle(yertle);
        display.waitForUser();
    }

    // first, copy over your drawSquare function from the previous work. If you don't have it, then you can use mine!
    // second, make a drawRow function that draws at an (x,y) coordinate, an amount, a width and height for the squares
    // two colors which you must alternate, and a boolean controlling if it should fill the squares.
    // finally, make a function that draws a stack of rows, with alternating colors. What you should see is a checkerboard pattern.

    public static void main(String[] args) {
        new Homework2();
    }

}
