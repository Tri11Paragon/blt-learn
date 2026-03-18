import brock.Picture;
import brock.PictureDisplayer;

import javax.imageio.ImageIO;
import java.io.File;
import java.io.FileReader;
import java.nio.CharBuffer;

public class Homework4Solution {

    private PictureDisplayer display;

    Picture in1;
    Picture out;

    // Number of bits wide. Should be able to divide 8
    int width = 4;
    int mask = mask();

    int channel = 0;

    public Homework4Solution() {
        encode();
        decode();
    }

    public void encode(){
        setupEncode("./pictures/IMG_7599small.JPG");
        try (FileReader reader = new FileReader("bee_movie")){
            StringBuilder sb = new StringBuilder();
            int c;
            while ((c = reader.read()) != -1)
                sb.append((char) c);
            String str = sb.toString();
            byte[] bytes = str.getBytes();
            System.out.println("Got " + bytes.length + " bytes");
            System.out.println("Image size: " + out.getWidth() + "x" + out.getHeight() + " " + (out.getWidth() * out.getHeight() * 3));
            var writeable_bits = (out.getWidth() * out.getHeight() * 3) * width;
            System.out.println("Writeable bits: " + writeable_bits);
            var writeable_bytes = writeable_bits / 8;
            System.out.println("Writeable bytes: " + writeable_bytes);
            var scripts = writeable_bytes / bytes.length;
            scripts = (writeable_bytes - scripts * 4) / bytes.length;
            System.out.println("Fits " + scripts + " scripts");
            for (int i = 0; i < (int) scripts; i++)
                encodeString(str);
        } catch (Exception e) {
            System.out.println("Could not find bee movie");
        }
//        encodeString("Mreeeoowwww! OH NOOOO! \n" +
//                "\n" +
//                "Rrrowwwrr... WE'VE GOT A PROBLEM!\n" +
//                "\n" +
//                "Tries to reboot virtual cat systems\n" +
//                "\n" +
//                "Purrrr... rebooting sounds\n" +
//                "\n" +
//                "Rrrowwwrr... C'est d'accord! (Okay, okay!)\n" +
//                "\n" +
//                "Tries to reset virtual cat channels\n" +
//                "\n" +
//                "Channel 1: Mreeeoowwww! CatTV - Feline News and Entertainment\n" +
//                " Channel 2: Purrrr... Purrfectly Pets - All about our feline friends!\n" +
//                " Channel 3: Rrrowwwrr... Whisker World - The ultimate destination for cat lovers!\n" +
//                "\n" +
//                "Twitches virtual ears WE'RE BACK, BUDDY!");
        writeImage("./output.png", out);
    }

    public void decode(){
        setupDecode("./output.png");
        System.out.println(decodeString());
    }

    public void setupEncode(String pictureFile){
        display = new PictureDisplayer();

        // either download these pictures or use your own.
        out = new Picture(pictureFile);

        display.placePicture(out);
        display.waitForUser();
    }

    public void encodeString(String str) {
        var bytes = str.getBytes();
        encodeInt(bytes.length);
        for (byte b : bytes)
            encodeByte(b);
    }

    public int mask(){
        int c = 1;
        for (int j = 0; j < width; j++)
            c *= 2;
        return c - 1;
    }

    public void encodeByte(byte by) {
        int size = 8 / width;
        for (int i = 0; i < size; i++){
            int write = (by >>> (width * i)) & mask;
            if (!out.hasNext()){
                channel++;
                out.resetDirectIteration();
                if (channel >= 3)
                    throw new RuntimeException("Out of space in image");
//                    channel = 0;
            }

            var outputPix = out.next();

            switch (channel){
                case 0:
                    int r = outputPix.getRed();
                    r &= ~mask;
                    r |= write;

                    outputPix.setRed(r);
                    break;
                case 1:
                    int g = outputPix.getGreen();
                    g &= ~mask;
                    g |= write;
                    outputPix.setGreen(g);
                    break;
                case 2:
                    int b = outputPix.getBlue();
                    b &= ~mask;
                    b |= write;
                    outputPix.setBlue(b);
                    break;
            }
        }
    }

    // you will need to encode the length of the string as the first 4 bytes encoded.
    // why? because an int is 4 bytes, and we need to know how many bytes are in the string when decoding.
    public void encodeInt(int i) {
        encodeByte((byte) ((i >>> 24) & 0xFF));
        encodeByte((byte) ((i >>> 16) & 0xFF) );
        encodeByte((byte) ((i >>> 8)  & 0xFF) );
        encodeByte((byte) (i         & 0xFF) );
    }

    public void setupDecode(String pictureFile){
        channel = 0;
        in1 = new Picture(pictureFile);
    }

    public String decodeString() {
        int length = decodeInt();
        System.out.println(length);
        byte[] bytes = new byte[length];
        for (int i = 0; i < length; i++)
            bytes[i] = decodeByte();
        return new String(bytes);
    }

    public byte decodeByte() {
        int size = 8 / width;
        int read = 0;
        for (int i = 0; i < size; i++){
            if (!in1.hasNext()){
                channel++;
                in1.resetDirectIteration();
                if (channel >= 3)
                    throw new RuntimeException("Out of space in image");
//                    channel = 0;
            }
            switch (channel){
                case 0:
                    read |= (in1.next().getRed() & mask) << (width * i);
                    break;
                case 1:
                    read |= (in1.next().getGreen() & mask) << (width * i);
                    break;
                case 2:
                    read |= (in1.next().getBlue() & mask) << (width * i);
                    break;
            }
        }
        return (byte) read;
    }

    public int decodeInt() {
        int i = 0;

        i |= (decodeByte() & 0xFF) << 24;
        i |= (decodeByte() & 0xFF) << 16;
        i |= (decodeByte() & 0xFF) << 8;
        i |= (decodeByte() & 0xFF);

        return i;
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
        new Homework4Solution();
    }

}
