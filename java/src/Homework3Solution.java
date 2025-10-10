import brock.Picture;
import brock.PictureDisplayer;

import java.util.function.BiPredicate;

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

        spiral(50);
    }

    void mixRandom(){
        mix_images((x, y) -> Math.random() < 0.5);
    }

    void mixRandom2(){
        BrettRandom rand = new BrettRandom();
        mix_images((x, y) -> rand.brettPlex(x, y, 6, 64) < 0.0);
    }

    void mixHorizontal(int stripeSize){
        mix_images((x, y) -> (x / stripeSize) % 2 == 0);
    }

    void mixVertical(int stripeSize){
        mix_images((x, y) -> (y / stripeSize) % 2 == 0);
    }

    void checkerBoard(int stripeSize){
        mix_images((x, y) -> (x / stripeSize) % 2 == (y / stripeSize) % 2);
    }

    void checkerBoard2(int stripeSize){
        mix_images((x, y) -> Math.sin((double) x / stripeSize) < Math.cos((double) y / stripeSize));
    }

    void rings(int ringSize){
        final int halfWidth = out.getWidth() / 2;
        final int halfHeight = out.getHeight() / 2;
        mix_images((x, y) -> {
            var x1 = x - halfWidth;
            var y1 = y - halfHeight;
            return ((int)Math.sqrt(x1 * x1 + y1 * y1) % ringSize) < ringSize / 2;
        });
    }

    void spiral(double tightness) {
        mix_images((x, y) -> {
            // Calculate distance from center
            double dx = x - out.getWidth() / 2f;
            double dy = y - out.getHeight() / 2f;
            double distance = Math.sqrt(dx * dx + dy * dy);

            // Calculate angle (in radians)
            double angle = Math.atan2(dy, dx);

            // Combine angle and distance for spiral effect
            // The tightness parameter controls how quickly the spiral winds
            return ((int)((angle + distance / tightness) )) % 2 == 0;
        });
    }


    void sincos(int stripeSize){
        mix_images((x, y) -> Math.sin(x) / stripeSize < Math.cos(y) / stripeSize);
    }

    void tans(int stripeSize){
        mix_images((x, y) -> {
            return Math.tan(Math.PI/(x-out.getWidth()/2f)*out.getWidth())+Math.tan(Math.PI/(y-out.getHeight()/2f)*out.getHeight())>0;
        });
    }

    void pattern(int ringSize){
        final int halfWidth = out.getWidth() / 2;
        final int halfHeight = out.getHeight() / 2;
        mix_images((x, y) -> {
            var x1 = x - halfWidth;
            var y1 = y - halfHeight;
            return (x1 * x1 + y1 * y1) % (ringSize) == 0;
        });
    }

    void pattern2(int ringSize){
        final int halfWidth = out.getWidth() / 2;
        final int halfHeight = out.getHeight() / 2;
        mix_images((x, y) -> {
            var x1 = x - halfWidth;
            var y1 = y - halfHeight;
            return (x1 * x1 + y1 * y1) % (ringSize) < ringSize / 2;
        });
    }

    // You are very likely confused as to what this parameter is.
    // You do not need to know, nor was it required as a solution
    // I did this only to prevent code duplication.
    // Your code likely has a bunch of individual functions or a single function.
    void mix_images(BiPredicate<Integer, Integer> selectPred){
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

            if (selectPred.test(x, y)){
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

    public static class BrettRandom {
        private long seed;

        public BrettRandom() {
            this.seed = System.currentTimeMillis();
        }

        public BrettRandom(long seed) {
            this.seed = seed;
        }

        public void setSeed(long seed) {
            this.seed = seed;
        }

        public void setSeed(int x, int y) {
            this.seed = ((long)x) | ((long) y << 32);
        }

        public void setSeed(float x, float y) {
            setSeed(Float.floatToIntBits(x), Float.floatToIntBits(y));
        }

        public long nextLong() {
            seed = murmur64(seed);
            return seed;
        }

        public double nextDouble() {
            return nextLong() / (double) Long.MAX_VALUE;
        }

        public float nextFloat() {
            return (float) nextDouble();
        }

        public float brettPlex(int x, int y, int octaves, int scale) {
            // this is a bad algorithm
            double ac = 0;
            for (int i = -octaves; i < octaves + 1; i++) {
                for (int j = -octaves; j < octaves + 1; j++) {
                    setSeed((x + i) / scale, (y + j) / scale);
                    ac += (nextDouble() * 2 - 1);
                }
            }
            return (float)ac;
        }

        // random hash
        // https://lemire.me/blog/2018/08/15/fast-strongly-universal-64-bit-hashing-everywhere/
        public static long murmur64(long h) {
            h ^= h >> 33;
            h *= 0xff51afd7ed558ccdL;
            h ^= h >> 33;
            h *= 0xc4ceb9fe1a85ec53L;
            h ^= h >> 33;
            return h;
        }
    }

    public static void main(String[] args) {
        new Homework3Solution();
    }

}
