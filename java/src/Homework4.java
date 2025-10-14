import brock.*;

import javax.imageio.ImageIO;
import java.io.File;

public class Homework4 {

    private PictureDisplayer display;

    Picture in1;
    Picture out;

    public Homework4() {

    }

    public void encode(){
        setupEncode("./pictures/IMG_7599.JPG");
        encodeString("Hello World!");
        writeImage("./output.png", out);
    }

    public void decode(){
        setupDecode("./output.png");
        System.out.println(decodeString());
    }

    public void setupEncode(String pictureFile){
        display = new PictureDisplayer();

        // either download these pictures or use your own.
        in1 = new Picture(pictureFile);

        // Make an output picture of the same size as the input picture.
        out = new Picture(in1.getWidth(), in1.getHeight());

        display.placePicture(out);
        display.waitForUser();
    }

    public void encodeString(String str) {
        var bytes = str.getBytes();
        encodeInt(bytes.length);
        for (byte b : bytes)
            encodeByte(b);
    }

    public void encodeByte(byte b) {

    }

    // you will need to encode the length of the string as the first 4 bytes encoded.
    // why? because an int is 4 bytes, and we need to know how many bytes are in the string when decoding.
    public void encodeInt(int i) {

    }

    public void setupDecode(String pictureFile){
        in1 = new Picture(pictureFile);
    }

    public String decodeString() {
        int length = decodeInt();
        byte[] bytes = new byte[length];
        for (int i = 0; i < length; i++)
            bytes[i] = decodeByte();
        return new String(bytes);
    }

    public byte decodeByte() {
        return 0;
    }

    public int decodeInt() {
        return 0;
    }

    public static void writeImage(String file, Picture picture) {
        try {
            ImageIO.write(picture.getImage(), "png", new File(file));
        } catch (Exception e) {
            System.out.println("Error writing image file " + file);
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        new Homework4();
    }

}
