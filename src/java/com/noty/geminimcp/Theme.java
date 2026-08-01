package com.noty.geminimcp;

import java.awt.Color;

public class Theme {
    // Windows 11 Dark Theme Colors
    public static final Color BG_PRIMARY = new Color(26, 26, 46);
    public static final Color BG_SECONDARY = new Color(22, 33, 62);
    public static final Color BG_CARD = new Color(15, 52, 96);
    public static final Color BG_CARD_HOVER = new Color(26, 58, 106);
    public static final Color TEXT_PRIMARY = new Color(224, 224, 224);
    public static final Color TEXT_SECONDARY = new Color(160, 160, 160);
    public static final Color TEXT_MUTED = new Color(107, 107, 107);
    public static final Color BORDER_COLOR = new Color(42, 42, 74);
    public static final Color ACCENT_PRIMARY = new Color(102, 126, 234);
    public static final Color ACCENT_SECONDARY = new Color(118, 75, 162);
    public static final Color SUCCESS = new Color(72, 187, 120);
    public static final Color DANGER = new Color(245, 101, 101);
    public static final Color WARNING = new Color(246, 173, 85);
    public static final Color INFO = new Color(66, 153, 225);
    
    public static Color getAccentGradient(float position) {
        if (position < 0.5f) {
            float t = position / 0.5f;
            return interpolate(ACCENT_PRIMARY, ACCENT_SECONDARY, t);
        } else {
            float t = (position - 0.5f) / 0.5f;
            return interpolate(ACCENT_SECONDARY, ACCENT_PRIMARY, t);
        }
    }
    
    private static Color interpolate(Color c1, Color c2, float t) {
        int r = (int)(c1.getRed() + (c2.getRed() - c1.getRed()) * t);
        int g = (int)(c1.getGreen() + (c2.getGreen() - c1.getGreen()) * t);
        int b = (int)(c1.getBlue() + (c2.getBlue() - c1.getBlue()) * t);
        return new Color(r, g, b);
    }
}