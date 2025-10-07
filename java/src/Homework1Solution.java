import brock.Turtle;
import brock.TurtleDisplayer;

public class Homework1Solution {

    TurtleDisplayer display;
    Turtle yertle;

    public Homework1Solution() {
        display = new TurtleDisplayer();
        yertle = new Turtle();

        display.placeTurtle(yertle);
        display.waitForUser();

//        drawTriangle(50, 50, 100, false);
//        drawTriangle(-50, -50, 75, true);

//        drawSquare(50, 50, 100, 100, false);
//        drawSquare(-50, -50, 100, 100, true);

//        drawSquare(50, 50, 100, 50, false);
//        drawSquare(-50, -50, 100, 50, true);

//        drawCircle(0, 0, 100, false);

        drawLine(-100, -100, 100, 100);
    }

    // Make functions to draw various polygons {Equilateral Triangle, Square, Circle} at a position (x, y) with a size.
    // Provide an option to fill the shape.


    // function takes in an x,y coordinate and draws a triangle of size size
    void drawTriangle(int x, int y, int size, boolean filled) {
        // we have no idea what angle the turtle is facing, so we need to set it to a known angle.
        // 0 is the default angle, meaning facing right.
        yertle.setAngle(0);
        // continuing with the theme of making the turtle in a consistent state, we should turn off the pen.
        yertle.penUp();

        // move to the position the user wants us to draw the triangle at.
        // because how turtle draws, we must offset by 1/3 the size, to center the triangle on (x,y)
        yertle.moveTo(x + size / 3.0, y + size / 3.0);

        // if how I got to that is confusing to you, comment out that line and replace it with this:
//        yertle.moveTo(x, y);
        // what do you notice?

        // start drawing the triangle.
        yertle.penDown();
        if (filled) {
            yertle.beginFill();
        }
        for (int i = 0; i < 3; i++) {
            // each time we draw a side of the triangle, we need to turn the turtle 1/3 of a circle.
            yertle.right(2 * Math.PI / 3);
            yertle.forward(size);
        }
        if (filled) {
            yertle.endFill();
        }
    }

    // function takes in an x,y coordinate and draws a square of width, height (width=height for square)
    void drawSquare(int x, int y, int width, int height, boolean filled) {
        yertle.setAngle(0);
        yertle.penUp();

        yertle.moveTo(x - width / 2.0, y - height / 2.0);
//        yertle.moveTo(x, y);

        yertle.penDown();
        if (filled) {
            yertle.beginFill();
        }
        for (int i = 0; i < 2; i++) {
            yertle.forward(width);
            yertle.left(Math.PI / 2);
            yertle.forward(height);
            yertle.left(Math.PI / 2);
        }
        if (filled) {
            yertle.endFill();
        }
    }

    // function takes in an x,y coordinate and draws a circle of size size
    void drawCircle(int x, int y, int size, boolean filled) {
        yertle.setAngle(0);
        yertle.penUp();
        yertle.moveTo(x, y);
        yertle.penDown();
        yertle.drawCircle(size, filled);
    }

    // Make a function to draw a line from (x1, y1) to (x2, y2).
    void drawLine(int x1, int y1, int x2, int y2) {
        // there's no reason to waste time trying to figure out the angle math.
        yertle.penUp();
        yertle.moveTo(x1, y1);
        yertle.penDown();
        yertle.moveTo(x2, y2);
    }

    public static void main(String[] args) {
        new Homework1Solution();
    }

}
