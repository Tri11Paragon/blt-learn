package statics;

import brock.*;

import java.awt.*;
import java.awt.geom.Point2D;

public class CodeReuse {

public static class Yertle {
    private final int width, height;
    private final Turtle turtle;


    // By separating out the logic into a sub-turtle class, you can have different turtles do different things.
    public Yertle(TurtleDisplayer displayer, int width, int height) {
        this.width = width;
        this.height = height;

        // it is common convention to use `this.` when initializing variables in the constructor.
        // even if the `this.` is not required due to variable shadowing.
        this.turtle = new Turtle();
        this.turtle.useDegree();
        displayer.placeTurtle(turtle);
    }

    private void side(double size){
        this.turtle.forward(size);
        this.turtle.right(90);
    }

    public void rectangle(double width, double height){
        this.turtle.penDown();
        side(width);
        side(height);
        side(width);
        side(height);
        this.turtle.penUp();
        this.turtle.endFill();
    }

    public void randomFill(double threshold){
        if (Math.random() >= threshold) {
            Color c = Color.getHSBColor((float)Math.random(), 1, (float)(Math.random() + 1) / 2);
            this.turtle.setFillColour(new Colour(c.getRed(), c.getGreen(), c.getBlue()));
            this.turtle.beginFill();
        }
    }

    public void randomRectangle(double maxWidth, double maxHeight) {
        double width = Math.random() * maxWidth + 10;
        double height = Math.random() * maxHeight + 10;
        this.rectangle(width, height);
    }

    public Point2D.Double moveToRandom(){
        double x = Math.random() * width - width / 2.0;
        double y = Math.random() * height - height / 2.0;
        this.turtle.moveTo(x, y);

        return new Point2D.Double(x, y);
    }

    public double randomRotation(){
        double angle = Math.random() * 360;

        this.turtle.right(angle);

        return angle;
    }

    // allow access to the underlying turtle without allowing the turtle variable's content to be changed
    public Turtle getTurtle(){
        return turtle;
    }
}

public static class Main {
    public static void main(String[] args){
        final int WIDTH = 600;
        final int HEIGHT = 600;

        final int MAX_WIDTH = 100;
        final int MAX_HEIGHT = 100;

        TurtleDisplayer displayer = new TurtleDisplayer(WIDTH, HEIGHT);

        Yertle yertle = new Yertle(displayer, WIDTH, HEIGHT);
        Yertle mertle = new Yertle(displayer, WIDTH, HEIGHT);

        for (int i = 0; i < 100; i++){
            var point = yertle.moveToRandom();
            var angle = yertle.randomRotation();
            yertle.randomFill(0.1);
            yertle.randomRectangle(MAX_WIDTH, MAX_HEIGHT);

            // move mertle to the opposite position and rotation
            mertle.getTurtle().moveTo(-point.getX(), -point.getY());
            mertle.getTurtle().right(-angle);

            mertle.randomFill(0.1);
            mertle.randomRectangle(MAX_WIDTH, MAX_HEIGHT);
        }
    }
}

}
