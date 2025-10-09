import brock.Picture;
import brock.PictureDisplayer;

public class Homework3Solution {

    private PictureDisplayer display ;

    Picture in1;
    Picture in2;
    Picture out;

    public Homework3Solution() {
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

        mix_images();
    }

    void mix_images(){
        while (in1.hasNext() && in2.hasNext() && out.hasNext()){
            var p1 = in1.next();
            var p2 = in2.next();
            var o = out.next();

            var x = o.getX();
            var y = o.getY();

            var r1 = p1.getRed();
            var g1 = p1.getGreen();
            var b1 = p1.getBlue();

            var r2 = p2.getRed();
            var g2 = p2.getGreen();
            var b2 = p2.getBlue();

            if ((int)(x / 50.0f) % 2 == 0){
                o.setRed(r1);
                o.setGreen(g1);
                o.setBlue(b1);
            } else {
                o.setRed(r2);
                o.setGreen(g2);
                o.setBlue(b2);
            }
        }
    }

    public static void main(String[] args) {
        new Homework3Solution();
    }

}
