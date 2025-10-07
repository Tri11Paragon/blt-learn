import brock.Turtle;
import brock.TurtleDisplayer;

public class HomeworkBase {

    TurtleDisplayer display;
    Turtle yertle;

    public HomeworkBase() {
        display = new TurtleDisplayer();
        yertle = new Turtle();

        display.placeTurtle(yertle);
        display.waitForUser();
    }

    public static void main(String[] args) {
        new HomeworkBase();
    }

}
