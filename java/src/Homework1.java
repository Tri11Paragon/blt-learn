import brock.Turtle;
import brock.TurtleDisplayer;

public class Homework1 {

    TurtleDisplayer display;
    Turtle yertle;

    public Homework1() {
        display = new TurtleDisplayer();
        yertle = new Turtle();

        display.placeTurtle(yertle);
        display.waitForUser();

        drawString("Your Name Here");
    }

    public void drawString(String str){
        for (char c : str.toLowerCase().toCharArray())
            drawCharacter(c);
    }

    public void drawCharacter(char c){

    }

    public static void main(String[] args) {
        new Homework1();
    }

}
