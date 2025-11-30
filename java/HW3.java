import brock.*;
/**
 * Write a description of class HW3 here.
 *
 * @author      Samuel Awolola
 * @version     COSC1P02
 * @assigment   #3
 * @id          8629040
 * @version     1
 * @since       November 2
 */
public class HW3
{
    // NO instance variables
    /**
     * Constructor for objects of class HW3
     */
    public HW3()
    {
        // initialise local variables
        Picture pic = new Picture();
        PictureDisplayer display = new PictureDisplayer(pic);
        applyEffect(pic);
        display.waitForUser();
        
    }
    
    private int getRandomInteger(int min, int max){
        return(int)((max - min + 1)* Math.random() + min);
    }
    /**
     * This method loops through every pixel in the picture and sends each pixel to the 
     * applyRandomEffect method.
     * 
     * @param pic is the 'object' picture selected by the user.
     */
    private void applyEffect(Picture pic){
        while(pic.hasNext()){
            Pixel p = pic.next();
            applyRandomEffect(p);
        }
        
    }
    /**
     * This method receives a pixel and applys an effect based on a random number generator.
     */
    private void applyRandomEffect(Pixel p){
        int randNum = getRandomInteger(14, 17);
            if (randNum == 14){ applyGreyScale(p);
            }else if(randNum == 15){ applyRedPixel(p);
            }else if (randNum == 16){ applyGreenPixel(p);
            }else if (randNum == 17){ applyBluePixel(p);
            }
    }

    /**
     * The following methods apply an effect to each individual pixel passed to each method determined
     * by the random number generator.
     * 
     * @param p is the pixel sent by the applyRandomEffect method to the following methods.
     */
    private void applyGreyScale(Pixel p){
        
            int avg = (p.getRed() + p.getGreen() + p.getBlue()) / 3;
            p.setRed(avg);
            p.setGreen(avg);
            p.setBlue(avg);
    }
    
    private void applyRedPixel(Pixel p){
        
            int avg = (p.getRed() + p.getGreen() + p.getBlue()) / 3;
            p.setRed(avg);
            p.setGreen(0);
            p.setBlue(0);
    }
    
    private void applyGreenPixel(Pixel p){
        
            int avg = (p.getRed() + p.getGreen() + p.getBlue()) / 3;
            p.setRed(0);
            p.setGreen(avg);
            p.setBlue(0);
        }
        
    private void applyBluePixel(Pixel p){
        
            int avg = (p.getRed() + p.getGreen() + p.getBlue()) / 3;
            p.setRed(0);
            p.setGreen(0);
            p.setBlue(avg);
    }
}
    
    