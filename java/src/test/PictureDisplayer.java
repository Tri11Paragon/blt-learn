package test;


import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.awt.image.ImageObserver;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;
import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.filechooser.FileNameExtensionFilter;

import brock.Picture;
import material.MaterialButton;
import utilities.IconGenerator;
import utilities.Screenshot;
import utilities.StyledFileChooser;

public class PictureDisplayer {
    private JFrame frame;
    private JPanel canvasPanel;
    private PictureDisplayer.MagnifierPanel magnifierCanvas;
    private PictureDisplayer.ColorSwatchPanel colorSwatch;
    private JLabel rgbLabel;
    private JLabel coordLabel;
    private JLabel sizeLabel;
    private JScrollPane scrollPane;
    private JPanel rightPanel;
    private Picture picture;
    private final AtomicReference<CountDownLatch> userLatchRef;
    private int lastValidX;
    private int lastValidY;
    private Color lastValidCenterColor;
    private static final int MAGNIFIER_SIZE = 7;
    private static final int MAGNIFIER_SCALE = 20;
    private static final int SWATCH_SIZE = 24;
    private MaterialButton continueButton;
    private MaterialButton saveButton;
    private JPanel continueButtonPlaceholder;
    private final CountDownLatch uiReadyLatch;
    private static final int MIN_CANVAS_VIEWPORT_WIDTH_DEFAULT = 100;
    private static final int MIN_CANVAS_VIEWPORT_HEIGHT_DEFAULT = 100;
    private boolean allowCanvasStretch;

    public PictureDisplayer() {
        this(300, 300);
    }

    public PictureDisplayer(int width, int height) {
        this(new Picture(width, height));
    }

    public PictureDisplayer(final Picture picture) {
        this.userLatchRef = new AtomicReference<>(new CountDownLatch(0));
        this.lastValidX = -1;
        this.lastValidY = -1;
        this.lastValidCenterColor = Color.WHITE;
        this.uiReadyLatch = new CountDownLatch(1);
        this.allowCanvasStretch = true;
        if (picture == null) {
            this.picture = new Picture();
        } else {
            this.picture = picture;
        }

        SwingUtilities.invokeLater(() -> {
            this.initializeUIComponents();
            this.frame.pack();
            this.frame.setLocationRelativeTo((Component)null);
            this.frame.setVisible(true);
            this.uiReadyLatch.countDown();
        });
        (new Thread(() -> {
            try {
                this.picture.getImageLoadLatch().await();
                SwingUtilities.invokeLater(this::updateDisplay);
            } catch (InterruptedException var2) {
                Thread.currentThread().interrupt();
            }

        })).start();
        Timer timer = new Timer(10, new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                assert picture != null;
                if (picture.hasImageChanged()) {
                    PictureDisplayer.this.updateDisplay();
                    picture.resetImageChanged();
                }

            }
        });
        timer.start();
    }

    public void placePicture(Picture newPicture) {
        if (newPicture == null) {
            this.picture = new Picture();
        } else {
            this.picture = newPicture;
        }

        (new Thread(() -> {
            try {
                this.picture.getImageLoadLatch().await();
                SwingUtilities.invokeLater(this::updateDisplay);
            } catch (InterruptedException var2) {
                Thread.currentThread().interrupt();
            }

        })).start();
        Timer timer = new Timer(10, new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (PictureDisplayer.this.picture.hasImageChanged()) {
                    PictureDisplayer.this.updateDisplay();
                    PictureDisplayer.this.picture.resetImageChanged();
                }

            }
        });
        timer.start();
    }

    public CountDownLatch getUiReadyLatch() {
        return this.uiReadyLatch;
    }

    private void initializeUIComponents() {
        String title = "Picture Displayer";
        if (this.picture != null && this.picture.getFileName() != null) {
            title = title + " - " + this.picture.getFileName();
        }

        title = title + " — 1.0.10";
        this.frame = new JFrame(title) {
            @Override
            public void dispose() {
                System.exit(0);
            }
        };
        this.frame.setDefaultCloseOperation(2);
        this.canvasPanel = new JPanel() {
            protected void paintComponent(Graphics g) {
                super.paintComponent(g);
                Graphics2D g2d = (Graphics2D)g;
                g2d.setComposite(AlphaComposite.SrcOver);
                BufferedImage image = PictureDisplayer.this.picture != null ? PictureDisplayer.this.picture.getImage() : null;
                if (image != null) {
                    int imageWidth = image.getWidth();
                    int imageHeight = image.getHeight();
                    int panelWidth = this.getWidth();
                    int panelHeight = this.getHeight();
                    double scale = (double)1.0F;
                    if (!PictureDisplayer.this.allowCanvasStretch || imageWidth <= panelWidth && imageHeight <= panelHeight) {
                        if (!PictureDisplayer.this.allowCanvasStretch && (imageWidth < panelWidth || imageHeight < panelHeight)) {
                            scale = (double)1.0F;
                        }
                    } else {
                        double scaleX = (double)panelWidth / (double)imageWidth;
                        double scaleY = (double)panelHeight / (double)imageHeight;
                        scale = Math.min(scaleX, scaleY);
                    }

                    int drawWidth = (int)((double)imageWidth * scale);
                    int drawHeight = (int)((double)imageHeight * scale);
                    int x = 0;
                    int y = 0;
                    g2d.drawImage(image, x, y, drawWidth, drawHeight, this);
                    g2d.setColor(Color.BLACK);
                    g2d.setStroke(new BasicStroke(1.0F));
                    g2d.drawRect(x, y, drawWidth, drawHeight);
                } else {
                    g2d.setColor(Color.LIGHT_GRAY);
                    g2d.fillRect(0, 0, this.getWidth(), this.getHeight());
                    g2d.setColor(Color.BLACK);
                    String message = "No Image Available";
                    FontMetrics fm = g2d.getFontMetrics();
                    int x = (this.getWidth() - fm.stringWidth(message)) / 2;
                    int y = (this.getHeight() - fm.getHeight()) / 2 + fm.getAscent();
                    g2d.drawString(message, x, y);
                }

            }

            public Dimension getPreferredSize() {
                return PictureDisplayer.this.picture != null && PictureDisplayer.this.picture.getImage() != null ? new Dimension(Math.max(1, PictureDisplayer.this.picture.getWidth()), Math.max(1, PictureDisplayer.this.picture.getHeight())) : new Dimension(300, 300);
            }

            public Dimension getMaximumSize() {
                return PictureDisplayer.this.allowCanvasStretch ? new Dimension(Integer.MAX_VALUE, Integer.MAX_VALUE) : this.getPreferredSize();
            }
        };
        this.canvasPanel.setOpaque(false);
        this.canvasPanel.setBackground(Color.LIGHT_GRAY);
        this.magnifierCanvas = new PictureDisplayer.MagnifierPanel();
        if (this.picture != null && this.picture.getWidth() > 0 && this.picture.getHeight() > 0 && this.picture.getImage() != null) {
            this.lastValidX = 0;
            this.lastValidY = 0;
            int initialArgb = this.picture.getImage().getRGB(this.lastValidX, this.lastValidY);
            this.lastValidCenterColor = new Color(initialArgb, true);
        } else {
            this.lastValidX = 0;
            this.lastValidY = 0;
            this.lastValidCenterColor = Color.WHITE;
        }

        this.magnifierCanvas.drawMagnifierContent(this.lastValidX, this.lastValidY);
        this.colorSwatch = new PictureDisplayer.ColorSwatchPanel();
        this.colorSwatch.setBorder(BorderFactory.createLineBorder(Color.BLACK));
        this.colorSwatch.setColor(this.lastValidCenterColor);
        this.colorSwatch.setMaximumSize(this.colorSwatch.getPreferredSize());
        String fontFamily = "Consolas";
        int fontSize = 14;
        Font textFont = new Font(fontFamily, 0, fontSize);
        this.rgbLabel = new JLabel(String.format("<html><font color='red'>R</font> = <font color='red'>%3d</font>, <font color='green'>G</font> = <font color='green'>%3d</font>, <font color='blue'>B</font> = <font color='blue'>%3d</font></html>", this.lastValidCenterColor.getRed(), this.lastValidCenterColor.getGreen(), this.lastValidCenterColor.getBlue()));
        this.rgbLabel.setFont(textFont);
        this.rgbLabel.setHorizontalAlignment(2);
        this.coordLabel = new JLabel(String.format("(X = %d, Y = %d)", this.lastValidX, this.lastValidY));
        this.coordLabel.setFont(textFont);
        this.coordLabel.setHorizontalAlignment(2);
        this.sizeLabel = new JLabel(String.format("↔ Width: %d Height: %d ↕", this.picture.getWidth(), this.picture.getHeight()));
        this.sizeLabel.setFont(textFont);
        this.sizeLabel.setHorizontalAlignment(2);
        this.canvasPanel.addMouseMotionListener(new MouseAdapter() {
            public void mouseMoved(MouseEvent e) {
                PictureDisplayer.this.handleMouseUpdate(e.getX(), e.getY());
            }
        });
        this.canvasPanel.addMouseListener(new MouseAdapter() {
            public void mouseExited(MouseEvent e) {
                PictureDisplayer.this.handleMouseUpdate(PictureDisplayer.this.lastValidX, PictureDisplayer.this.lastValidY);
            }
        });
        this.continueButton = new MaterialButton("Continue");
        this.continueButton.addActionListener((e) -> {
            CountDownLatch currentLatch = (CountDownLatch)this.userLatchRef.get();
            if (currentLatch != null) {
                currentLatch.countDown();
            }

        });
        this.continueButton.setVisible(false);
        MaterialButton screenshotButton = new MaterialButton("Take Screenshot", IconGenerator.getScreenshotIcon(16));
        screenshotButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                Screenshot s = new Screenshot(PictureDisplayer.this.frame);
                s.takeScreenshot();
            }
        });
        this.saveButton = new MaterialButton("Save");
        this.saveButton.addActionListener((e) -> this.savePicture());
        MaterialButton closeButton = new MaterialButton("Close");
        closeButton.addActionListener((e) -> this.frame.dispose());
        int maxButtonWidth = 0;
        int buttonHeight = this.continueButton.getPreferredSize().height;
        maxButtonWidth = Math.max(maxButtonWidth, this.continueButton.getPreferredSize().width);
        maxButtonWidth = Math.max(maxButtonWidth, screenshotButton.getPreferredSize().width);
        maxButtonWidth = Math.max(maxButtonWidth, this.saveButton.getPreferredSize().width);
        maxButtonWidth = Math.max(maxButtonWidth, closeButton.getPreferredSize().width);
        Dimension fixedButtonSize = new Dimension(maxButtonWidth, buttonHeight);
        this.continueButton.setPreferredSize(fixedButtonSize);
        this.continueButton.setMaximumSize(fixedButtonSize);
        screenshotButton.setPreferredSize(fixedButtonSize);
        screenshotButton.setMaximumSize(fixedButtonSize);
        this.saveButton.setPreferredSize(fixedButtonSize);
        this.saveButton.setMaximumSize(fixedButtonSize);
        closeButton.setPreferredSize(fixedButtonSize);
        closeButton.setMaximumSize(fixedButtonSize);
        this.continueButtonPlaceholder = new JPanel();
        this.continueButtonPlaceholder.setLayout(new BorderLayout());
        this.continueButtonPlaceholder.add(this.continueButton, "Center");
        this.continueButtonPlaceholder.setPreferredSize(fixedButtonSize);
        this.continueButtonPlaceholder.setMaximumSize(fixedButtonSize);
        this.continueButtonPlaceholder.setOpaque(false);
        this.rightPanel = new JPanel();
        this.rightPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
        this.rightPanel.setLayout(new BoxLayout(this.rightPanel, 1));
        this.rightPanel.setBackground(UIManager.getColor("Panel.background"));
        JPanel componentsPanel = new JPanel();
        componentsPanel.setLayout(new BoxLayout(componentsPanel, 1));
        componentsPanel.setAlignmentX(0.0F);
        componentsPanel.setOpaque(false);
        JPanel magnifierSectionPanel = new JPanel();
        magnifierSectionPanel.setLayout(new BoxLayout(magnifierSectionPanel, 1));
        magnifierSectionPanel.setAlignmentX(0.0F);
        magnifierSectionPanel.setOpaque(false);
        JLabel magnifierLabel = new JLabel("Magnifier");
        magnifierLabel.setFont(textFont);
        magnifierLabel.setAlignmentX(0.0F);
        magnifierSectionPanel.add(magnifierLabel);
        magnifierSectionPanel.add(Box.createVerticalStrut(5));
        this.magnifierCanvas.setAlignmentX(0.0F);
        magnifierSectionPanel.add(this.magnifierCanvas);
        JPanel infoSectionPanel = new JPanel();
        infoSectionPanel.setLayout(new BoxLayout(infoSectionPanel, 1));
        infoSectionPanel.setAlignmentX(0.0F);
        infoSectionPanel.setOpaque(false);
        JLabel colorSwatchLabel = new JLabel("Current Pixel Color");
        colorSwatchLabel.setFont(textFont);
        colorSwatchLabel.setAlignmentX(0.0F);
        infoSectionPanel.add(colorSwatchLabel);
        infoSectionPanel.add(Box.createVerticalStrut(5));
        JPanel colorInfoPanel = new JPanel();
        colorInfoPanel.setLayout(new BoxLayout(colorInfoPanel, 0));
        colorInfoPanel.setAlignmentX(0.0F);
        colorInfoPanel.setOpaque(false);
        this.colorSwatch.setAlignmentX(0.0F);
        colorInfoPanel.add(this.colorSwatch);
        colorInfoPanel.add(Box.createHorizontalStrut(10));
        this.rgbLabel.setAlignmentX(0.0F);
        colorInfoPanel.add(this.rgbLabel);
        colorInfoPanel.add(Box.createHorizontalGlue());
        JLabel tempRgbMeasurer = new JLabel(String.format("R = %3d, G = %3d, B = %3d", 255, 255, 255));
        tempRgbMeasurer.setFont(textFont);
        int rgbLabelEstimatedWidth = tempRgbMeasurer.getPreferredSize().width;
        int panelWidth = 34 + rgbLabelEstimatedWidth;
        int panelHeight = Math.max(24, tempRgbMeasurer.getPreferredSize().height);
        colorInfoPanel.setPreferredSize(new Dimension(panelWidth, panelHeight));
        colorInfoPanel.setMaximumSize(new Dimension(panelWidth, panelHeight));
        infoSectionPanel.add(colorInfoPanel);
        infoSectionPanel.add(Box.createVerticalStrut(5));
        JLabel coordTextLabel = new JLabel("Coordinates");
        coordTextLabel.setFont(textFont);
        coordTextLabel.setAlignmentX(0.0F);
        infoSectionPanel.add(coordTextLabel);
        infoSectionPanel.add(Box.createVerticalStrut(5));
        this.coordLabel.setAlignmentX(0.0F);
        infoSectionPanel.add(this.coordLabel);
        infoSectionPanel.add(Box.createVerticalStrut(15));
        JLabel sizeTextLabel = new JLabel("Image Dimensions");
        sizeTextLabel.setFont(textFont);
        sizeTextLabel.setAlignmentX(0.0F);
        infoSectionPanel.add(sizeTextLabel);
        infoSectionPanel.add(Box.createVerticalStrut(5));
        this.sizeLabel.setAlignmentX(0.0F);
        infoSectionPanel.add(this.sizeLabel);
        componentsPanel.add(magnifierSectionPanel);
        componentsPanel.add(Box.createVerticalStrut(15));
        componentsPanel.add(infoSectionPanel);
        JPanel buttonsPanel = new JPanel();
        buttonsPanel.setLayout(new BoxLayout(buttonsPanel, 1));
        buttonsPanel.setAlignmentX(0.0F);
        buttonsPanel.setOpaque(false);
        this.continueButtonPlaceholder.setAlignmentX(0.0F);
        buttonsPanel.add(this.continueButtonPlaceholder);
        buttonsPanel.add(Box.createVerticalStrut(5));
        screenshotButton.setAlignmentX(0.0F);
        buttonsPanel.add(screenshotButton);
        buttonsPanel.add(Box.createVerticalStrut(5));
        this.saveButton.setAlignmentX(0.0F);
        buttonsPanel.add(this.saveButton);
        buttonsPanel.add(Box.createVerticalStrut(5));
        closeButton.setAlignmentX(0.0F);
        buttonsPanel.add(closeButton);
        this.rightPanel.add(componentsPanel);
        this.rightPanel.add(Box.createVerticalStrut(25));
        this.rightPanel.add(buttonsPanel);
        this.rightPanel.add(Box.createVerticalGlue());
        this.rightPanel.validate();
        int minRightPanelWidth = this.rightPanel.getPreferredSize().width + 20;
        this.rightPanel.setMinimumSize(new Dimension(minRightPanelWidth, this.rightPanel.getPreferredSize().height));
        this.rightPanel.setPreferredSize(new Dimension(minRightPanelWidth, this.rightPanel.getPreferredSize().height));
        this.rightPanel.setMaximumSize(new Dimension(minRightPanelWidth, Integer.MAX_VALUE));
        JPanel mainContentPane = new JPanel(new BorderLayout());
        mainContentPane.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
        mainContentPane.setBackground(Color.WHITE);
        this.scrollPane = new JScrollPane(this.canvasPanel);
        this.scrollPane.setVerticalScrollBarPolicy(20);
        this.scrollPane.setHorizontalScrollBarPolicy(30);
        this.scrollPane.getViewport().setOpaque(false);
        this.scrollPane.setViewportBorder((Border)null);
        this.scrollPane.setBorder((Border)null);
        mainContentPane.add(this.scrollPane, "Center");
        mainContentPane.add(this.rightPanel, "East");
        this.frame.setContentPane(mainContentPane);
    }

    public void updateDisplay() {
        if (this.scrollPane != null) {
            int currentImageWidth = this.picture != null && this.picture.getImage() != null ? this.picture.getWidth() : 300;
            int currentImageHeight = this.picture != null && this.picture.getImage() != null ? this.picture.getHeight() : 300;
            Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            int maxScreenFrameWidth = (int)((double)screenSize.width * 0.66);
            int maxScreenFrameHeight = (int)((double)screenSize.height * 0.9);
            JScrollBar tempVScrollBar = new JScrollBar(1);
            JScrollBar tempHScrollBar = new JScrollBar(0);
            int estimatedVScrollBarWidth = tempVScrollBar.getPreferredSize().width;
            int estimatedHScrollBarHeight = tempHScrollBar.getPreferredSize().height;
            int horizontalFramePadding = this.frame.getInsets().left + this.frame.getInsets().right + this.frame.getContentPane().getInsets().left + this.frame.getContentPane().getInsets().right + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).left + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).right + this.rightPanel.getPreferredSize().width;
            int verticalFramePadding = this.frame.getInsets().top + this.frame.getInsets().bottom + this.frame.getContentPane().getInsets().top + this.frame.getContentPane().getInsets().bottom + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).top + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).bottom;
            boolean horizontalScrollbarWillBeNeeded = false;
            boolean verticalScrollbarWillBeNeeded = false;

            boolean changedPrediction;
            do {
                changedPrediction = false;
                boolean currentHSB = currentImageWidth > maxScreenFrameWidth - horizontalFramePadding - (verticalScrollbarWillBeNeeded ? estimatedVScrollBarWidth : 0);
                boolean currentVSB = currentImageHeight > maxScreenFrameHeight - verticalFramePadding - (horizontalScrollbarWillBeNeeded ? estimatedHScrollBarHeight : 0);
                if (currentHSB != horizontalScrollbarWillBeNeeded || currentVSB != verticalScrollbarWillBeNeeded) {
                    horizontalScrollbarWillBeNeeded = currentHSB;
                    verticalScrollbarWillBeNeeded = currentVSB;
                    changedPrediction = true;
                }
            } while(changedPrediction);

            if (!horizontalScrollbarWillBeNeeded && !verticalScrollbarWillBeNeeded) {
                this.allowCanvasStretch = false;
            } else {
                this.allowCanvasStretch = true;
            }

            int requiredFrameWidth = currentImageWidth + horizontalFramePadding;
            if (verticalScrollbarWillBeNeeded) {
                requiredFrameWidth += estimatedVScrollBarWidth;
            }

            int requiredFrameHeight = currentImageHeight + verticalFramePadding;
            if (horizontalScrollbarWillBeNeeded) {
                requiredFrameHeight += estimatedHScrollBarHeight;
            }

            int minCanvasWidthConsidered = Math.min(currentImageWidth, 100);
            if (currentImageWidth == 0) {
                minCanvasWidthConsidered = 100;
            }

            int absoluteMinFrameWidth = this.frame.getInsets().left + this.frame.getInsets().right + this.frame.getContentPane().getInsets().left + this.frame.getContentPane().getInsets().right + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).left + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).right + minCanvasWidthConsidered + this.rightPanel.getPreferredSize().width;
            int minCanvasHeightConsidered = Math.min(currentImageHeight, 100);
            if (currentImageHeight == 0) {
                minCanvasHeightConsidered = 100;
            }

            int absoluteMinFrameHeight = Math.max(minCanvasHeightConsidered + verticalFramePadding, this.rightPanel.getPreferredSize().height + this.frame.getInsets().top + this.frame.getInsets().bottom + this.frame.getContentPane().getInsets().top + this.frame.getContentPane().getInsets().bottom + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).top + (this.scrollPane != null ? this.scrollPane.getInsets() : new Insets(0, 0, 0, 0)).bottom);
            requiredFrameWidth = Math.max(requiredFrameWidth, absoluteMinFrameWidth);
            requiredFrameHeight = Math.max(requiredFrameHeight, absoluteMinFrameHeight);
            int finalFrameWidth = Math.min(requiredFrameWidth, maxScreenFrameWidth);
            int finalFrameHeight = Math.min(requiredFrameHeight, maxScreenFrameHeight);
            this.canvasPanel.setPreferredSize(new Dimension(currentImageWidth, currentImageHeight));
            this.sizeLabel.setText(String.format("↔ Width: %d Height: %d ↕", currentImageWidth, currentImageHeight));
            this.canvasPanel.revalidate();
            this.canvasPanel.repaint();
            this.scrollPane.revalidate();
            this.scrollPane.repaint();
            this.scrollPane.getViewport().doLayout();
            this.frame.setSize(finalFrameWidth, finalFrameHeight);
            this.frame.setMinimumSize(new Dimension(finalFrameWidth, finalFrameHeight));
            this.frame.setLocationRelativeTo((Component)null);
            this.handleMouseUpdate(this.lastValidX, this.lastValidY);
        }
    }

    public void waitForUser() {
        try {
            this.uiReadyLatch.await();
        } catch (InterruptedException var8) {
            Thread.currentThread().interrupt();
            return;
        }

        CountDownLatch currentLatch = new CountDownLatch(1);
        this.userLatchRef.set(currentLatch);
        SwingUtilities.invokeLater(() -> {
            this.continueButton.setVisible(true);
            this.continueButtonPlaceholder.revalidate();
            this.continueButtonPlaceholder.repaint();
        });
        boolean var7 = false;

        label59: {
            try {
                var7 = true;
                currentLatch.await();
                var7 = false;
                break label59;
            } catch (InterruptedException var9) {
                Thread.currentThread().interrupt();
                var7 = false;
            } finally {
                if (var7) {
                    SwingUtilities.invokeLater(() -> {
                        this.continueButton.setVisible(false);
                        this.continueButtonPlaceholder.revalidate();
                        this.continueButtonPlaceholder.repaint();
                    });
                    this.userLatchRef.compareAndSet(currentLatch, new CountDownLatch(0));
                }
            }

            SwingUtilities.invokeLater(() -> {
                this.continueButton.setVisible(false);
                this.continueButtonPlaceholder.revalidate();
                this.continueButtonPlaceholder.repaint();
            });
            this.userLatchRef.compareAndSet(currentLatch, new CountDownLatch(0));
            return;
        }

        SwingUtilities.invokeLater(() -> {
            this.continueButton.setVisible(false);
            this.continueButtonPlaceholder.revalidate();
            this.continueButtonPlaceholder.repaint();
        });
        this.userLatchRef.compareAndSet(currentLatch, new CountDownLatch(0));
    }

    private void handleMouseUpdate(int x, int y) {
        if (this.picture != null && this.picture.getImage() != null) {
            boolean isMouseOverImage = x >= 0 && y >= 0 && x < this.picture.getWidth() && y < this.picture.getHeight();
            if (isMouseOverImage) {
                this.lastValidX = x;
                this.lastValidY = y;
                int argb = this.picture.getImage().getRGB(this.lastValidX, this.lastValidY);
                Color c = new Color(argb, true);
                this.lastValidCenterColor = c;
                this.magnifierCanvas.drawMagnifierContent(this.lastValidX, this.lastValidY);
                this.colorSwatch.setColor(this.lastValidCenterColor);
                this.rgbLabel.setText(String.format("<html><font color='red'>R</font> = <font color='red'>%3d</font>, <font color='green'>G</font> = <font color='green'>%3d</font>, <font color='blue'>B</font> = <font color='blue'>%3d</font></html>", this.lastValidCenterColor.getRed(), this.lastValidCenterColor.getGreen(), this.lastValidCenterColor.getBlue()));
                this.coordLabel.setText(String.format("(X = %d, Y = %d)", this.lastValidX, this.lastValidY));
                this.canvasPanel.repaint();
            } else {
                if (this.lastValidX != -1 && this.lastValidY != -1) {
                    this.magnifierCanvas.drawMagnifierContent(this.lastValidX, this.lastValidY);
                    this.colorSwatch.setColor(this.lastValidCenterColor);
                    this.rgbLabel.setText(String.format("<html><font color='red'>R</font> = <font color='red'>%3d</font>, <font color='green'>G</font> = <font color='green'>%3d</font>, <font color='blue'>B</font> = <font color='blue'>%3d</font></html>", this.lastValidCenterColor.getRed(), this.lastValidCenterColor.getGreen(), this.lastValidCenterColor.getBlue()));
                    this.coordLabel.setText(String.format("(X = %d, Y = %d)", this.lastValidX, this.lastValidY));
                } else {
                    this.magnifierCanvas.drawEmptyMagnifierGridOnly(this.magnifierCanvas.magImage.createGraphics());
                    this.colorSwatch.setColor(Color.WHITE);
                    this.rgbLabel.setText("<html><font color='red'>R</font> = - , <font color='green'>G</font> = - , <font color='blue'>B</font> = - </html>");
                    this.coordLabel.setText("(X = -, Y = -)");
                }

                this.canvasPanel.repaint();
            }

        } else {
            this.magnifierCanvas.drawEmptyMagnifierGridOnly(this.magnifierCanvas.magImage.createGraphics());
            this.colorSwatch.setColor(Color.WHITE);
            this.rgbLabel.setText("<html><font color='red'>R</font> = - , <font color='green'>G</font> = - , <font color='blue'>B</font> = - </html>");
            this.coordLabel.setText("(X = -, Y = -)");
            this.canvasPanel.repaint();
        }
    }

    private void savePicture() {
        if (this.picture != null && this.picture.getImage() != null) {
            try {
                String pictureName = this.picture.getFileName();
                if (pictureName == null || pictureName.isEmpty()) {
                    pictureName = "output.png";
                }

                File pictureFile = new File(pictureName);
                String baseName = pictureFile.getName();
                String extension = "png";
                int dotIndex = baseName.lastIndexOf(46);
                if (dotIndex > 0) {
                    extension = baseName.substring(dotIndex + 1).toLowerCase();
                    baseName = baseName.substring(0, dotIndex);
                }

                File suggestedFile = this.getNextVersionedFile(baseName, extension);
                StyledFileChooser fileChooser = new StyledFileChooser(this.frame, 1);
                fileChooser.setDialogTitle("Save Picture");
                fileChooser.setCurrentDirectory(new File(System.getProperty("user.dir")));
                fileChooser.setSelectedFile(suggestedFile);
                FileNameExtensionFilter imageFilter = new FileNameExtensionFilter("Image Files (*.png, *.jpg, *.jpeg)", new String[]{"png", "jpg", "jpeg"});
                fileChooser.setFileFilter(imageFilter);
                fileChooser.setAcceptAllFileFilterUsed(false);
                int result = fileChooser.showSaveDialog(this.frame);
                if (result == 0) {
                    File outputFile = fileChooser.getSelectedFile();
                    String fileName = outputFile.getName();
                    String chosenExt = "";
                    int dot = fileName.lastIndexOf(46);
                    if (dot > 0 && dot < fileName.length() - 1) {
                        chosenExt = fileName.substring(dot + 1).toLowerCase();
                    } else {
                        chosenExt = extension;
                    }

                    if (outputFile.exists()) {
                        int overwrite = JOptionPane.showConfirmDialog(this.frame, "File already exists. Overwrite?", "Confirm Overwrite", 0);
                        if (overwrite != 0) {
                            return;
                        }
                    }

                    String formatName = chosenExt.equals("jpg") ? "jpeg" : chosenExt;
                    if (formatName.isEmpty()) {
                        formatName = "png";
                    }

                    ImageIO.write(this.picture.getImage(), formatName, outputFile);
                }
            } catch (IOException e) {
                JOptionPane.showMessageDialog(this.frame, "Error saving picture: " + e.getMessage(), "Save Error", 0);
                e.printStackTrace();
            }

        } else {
            JOptionPane.showMessageDialog(this.frame, "No picture loaded to save.", "Save Error", 2);
        }
    }

    private File getNextVersionedFile(String baseName, String extension) {
        int version = 1;

        File file;
        do {
            file = new File(baseName + "-v" + version + "." + extension);
            ++version;
        } while(file.exists());

        return file;
    }

    private class MagnifierPanel extends JPanel {
        private BufferedImage magImage = new BufferedImage(140, 140, 2);

        public MagnifierPanel() {
            this.setOpaque(false);
        }

        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            Graphics2D g2d = (Graphics2D)g;
            g2d.setComposite(AlphaComposite.SrcOver);
            g2d.drawImage(this.magImage, 0, 0, (ImageObserver)null);
        }

        public Dimension getPreferredSize() {
            return new Dimension(140, 140);
        }

        public void drawMagnifierContent(int centerX, int centerY) {
            Graphics2D magGC = this.magImage.createGraphics();
            magGC.setComposite(AlphaComposite.SrcOver);
            magGC.clearRect(0, 0, this.magImage.getWidth(), this.magImage.getHeight());
            int half = 3;
            boolean hasOutOfBoundsCellsInMagnifierView = false;

            for(int dy = -half; dy <= half; ++dy) {
                for(int dx = -half; dx <= half; ++dx) {
                    int px = centerX + dx;
                    int py = centerY + dy;
                    double rectX = (double)((dx + half) * 20);
                    double rectY = (double)((dy + half) * 20);
                    boolean isOutOfBounds = PictureDisplayer.this.picture == null || PictureDisplayer.this.picture.getImage() == null || px < 0 || py < 0 || px >= PictureDisplayer.this.picture.getWidth() || py >= PictureDisplayer.this.picture.getHeight();
                    if (isOutOfBounds) {
                        magGC.setColor(Color.BLACK);
                        magGC.fillRect((int)rectX, (int)rectY, 20, 20);
                        hasOutOfBoundsCellsInMagnifierView = true;
                    } else {
                        int argb = PictureDisplayer.this.picture.getImage().getRGB(px, py);
                        Color c = new Color(argb, true);
                        if (c.getAlpha() == 0) {
                            magGC.setColor(Color.WHITE);
                            magGC.fillRect((int)rectX, (int)rectY, 20, 20);
                        } else {
                            magGC.setColor(c);
                            magGC.fillRect((int)rectX, (int)rectY, 20, 20);
                        }
                    }
                }
            }

            magGC.setColor(Color.GRAY);
            magGC.setStroke(new BasicStroke(1.0F));

            for(int i = 0; i <= 7; ++i) {
                magGC.drawLine(i * 20, 0, i * 20, 140);
                magGC.drawLine(0, i * 20, 140, i * 20);
            }

            if (hasOutOfBoundsCellsInMagnifierView) {
                magGC.setColor(Color.RED);
                magGC.setStroke(new BasicStroke(2.0F));

                for(int dy = -half; dy <= half; ++dy) {
                    for(int dx = -half; dx <= half; ++dx) {
                        int px = centerX + dx;
                        int py = centerY + dy;
                        boolean isOutOfBounds = PictureDisplayer.this.picture == null || PictureDisplayer.this.picture.getImage() == null || px < 0 || py < 0 || px >= PictureDisplayer.this.picture.getWidth() || py >= PictureDisplayer.this.picture.getHeight();
                        if (isOutOfBounds) {
                            double rectX = (double)((dx + half) * 20);
                            double rectY = (double)((dy + half) * 20);
                            magGC.drawLine((int)rectX + 3, (int)rectY + 3, (int)rectX + 20 - 3, (int)rectY + 20 - 3);
                            magGC.drawLine((int)rectX + 3, (int)rectY + 20 - 3, (int)rectX + 20 - 3, (int)rectY + 3);
                        }
                    }
                }
            }

            double centerMagX = (double)(half * 20);
            double centerMagY = (double)(half * 20);
            magGC.setColor(Color.RED);
            magGC.setStroke(new BasicStroke(2.0F));
            magGC.drawRect((int)centerMagX, (int)centerMagY, 20, 20);
            magGC.setColor(hasOutOfBoundsCellsInMagnifierView ? Color.RED : Color.GRAY);
            magGC.setStroke(new BasicStroke(3.0F));
            magGC.drawRect(0, 0, 140, 140);
            magGC.dispose();
            this.repaint();
        }

        private void drawEmptyMagnifierGridOnly(Graphics2D magGC) {
            magGC.setComposite(AlphaComposite.SrcOver);
            magGC.clearRect(0, 0, this.magImage.getWidth(), this.magImage.getHeight());
            magGC.setColor(Color.WHITE);
            magGC.fillRect(0, 0, this.magImage.getWidth(), this.magImage.getHeight());
            magGC.setColor(Color.GRAY);
            magGC.setStroke(new BasicStroke(1.0F));

            for(int i = 0; i <= 7; ++i) {
                magGC.drawLine(i * 20, 0, i * 20, 140);
                magGC.drawLine(0, i * 20, 140, i * 20);
            }

            magGC.setStroke(new BasicStroke(3.0F));
            magGC.drawRect(0, 0, 140, 140);
            magGC.dispose();
            this.repaint();
        }
    }

    private class ColorSwatchPanel extends JPanel {
        private Color currentColor;

        private ColorSwatchPanel() {
            this.currentColor = Color.WHITE;
        }

        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            g.setColor(this.currentColor);
            g.fillRect(0, 0, this.getWidth(), this.getHeight());
        }

        public Dimension getPreferredSize() {
            return new Dimension(24, 24);
        }

        public void setColor(Color c) {
            this.currentColor = c;
            this.repaint();
        }
    }
}
