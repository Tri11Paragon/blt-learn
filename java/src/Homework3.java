import brock.*;

public class Homework3 {

    private static final int SIZE = 600;
    private PictureDisplayer display ;

    Picture in1;
    Picture in2;
    Picture out;

    public Homework3() {
        display = new PictureDisplayer();

        // picture should be the same size!
        // either download these pictures or use your own.
        in1 = new Picture("./pictures/IMG_7599.JPG");
        in2 = new Picture("./pictures/IMG_8934.JPG");

        // Make an output picture of the same size as the input pictures,
        // if one image is smaller, we should use its size.
        out = new Picture(Math.min(in1.getWidth(), in2.getWidth()), Math.min(in1.getHeight(), in2.getHeight()));
        display.placePicture(out);
        display.waitForUser();
    }


    public static void main(String[] args) {
        new Homework3();
    }

}
