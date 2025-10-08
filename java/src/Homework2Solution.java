import brock.Colour;
import brock.Turtle;
import brock.TurtleDisplayer;

public class Homework2Solution {

    TurtleDisplayer display;
    Turtle yertle;

    public Homework2Solution() {
        display = new TurtleDisplayer(600, 600);
        yertle = new Turtle();

        display.placeTurtle(yertle);
        display.waitForUser();

        drawRow2(0, 0, 5, 25, 50, 25, Colour.RED, Colour.BLUE, true);
        drawRow(0, 50, 10, 25, 25, Colour.GREEN, Colour.LIGHT_GRAY, false);

        drawRowOfRows(-200, -200, 10, 5, 25, 25, Colour.MAGENTA, Colour.CYAN, true);
    }

    void drawRowOfRows(int x, int y, int amountX, int amountY, int width, int height, Colour c1, Colour c2, boolean filled) {
        for (int i = 0; i < amountY; i++) {
            Colour c1v = (i & 1) == 0 ? c1 : c2;
            Colour c2v = (i & 1) == 0 ? c2 : c1;
            drawRow(x, y + i * height, amountX, width, height, c1v, c2v, filled);
        }
    }

    void drawRow(int x, int y, int amount, int width, int height, Colour c1, Colour c2, boolean filled) {
        for (int i = 0; i < amount; i++) {
            Colour c = c1;
            if ((i & 1) == 0)
                c = c2;
            yertle.setPenColour(c);
            yertle.setFillColour(c);
            drawSquare(x + i * width, y, width, height, filled);
        }
    }

    // this is a more complex version of the drawRow function.
    // you may be thinking that this is a lot of parameters and that it looks cluttered.
    void drawRow2(int x, int y, int amount, int width1, int width2, int height, Colour c1, Colour c2, boolean filled) {
        for (int i = 0; i < amount; i++) {
            int drawX = (int) (x + i * width1/2.0 + i * width2/2.0);
            if ((i & 1) == 1){
                // odd number
                yertle.setPenColour(c1);
                yertle.setFillColour(c1);
                drawSquare(drawX, y, width1, height, filled);
            } else {
                // even number
                yertle.setPenColour(c2);
                yertle.setFillColour(c2);
                drawSquare(drawX, y, width2, height, filled);
            }
        }
    }

    // function takes in an x,y coordinate and draws a square of width, height (width=height for square)
    void drawSquare(int x, int y, int width, int height, boolean filled) {
        yertle.penUp();
        yertle.setAngle(0);
        yertle.moveTo(x - width / 2.0, y - height / 2.0);

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

    public static void main(String[] args) {
        new Homework2Solution();
    }

}
