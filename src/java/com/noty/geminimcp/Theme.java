package com.noty.geminimcp;

import java.awt.Color;
import java.awt.GradientPaint;

public class Theme {
    // Windows 11 Dark Theme with vibrant colors
    public static final Color BG_PRIMARY = new Color(18, 18, 30);
    public static final Color BG_SECONDARY = new Color(28, 28, 48);
    public static final Color BG_CARD = new Color(38, 38, 68);
    public static final Color BG_CARD_HOVER = new Color(48, 48, 88);
    public static final Color TEXT_PRIMARY = new Color(235, 235, 255);
    public static final Color TEXT_SECONDARY = new Color(180, 180, 210);
    public static final Color TEXT_MUTED = new Color(120, 120, 160);
    public static final Color BORDER_COLOR = new Color(60, 60, 90);

    // Vibrant accent colors
    public static final Color ACCENT_PRIMARY = new Color(108, 92, 231);
    public static final Color ACCENT_SECONDARY = new Color(162, 88, 237);
    public static final Color ACCENT_GRADIENT_START = new Color(108, 92, 231);
    public static final Color ACCENT_GRADIENT_END = new Color(162, 88, 237);

    // Status colors with glow effect
    public static final Color SUCCESS = new Color(72, 220, 120);
    public static final Color SUCCESS_GLOW = new Color(72, 220, 120, 60);
    public static final Color DANGER = new Color(255, 107, 107);
    public static final Color DANGER_GLOW = new Color(255, 107, 107, 60);
    public static final Color WARNING = new Color(255, 215, 90);
    public static final Color WARNING_GLOW = new Color(255, 215, 90, 60);
    public static final Color INFO = new Color(90, 180, 255);
    public static final Color INFO_GLOW = new Color(90, 180, 255, 60);

    // Additional vibrant colors
    public static final Color PURPLE = new Color(155, 89, 182);
    public static final Color PINK = new Color(231, 76, 160);
    public static final Color CYAN = new Color(52, 152, 219);
    public static final Color ORANGE = new Color(243, 156, 18);
    public static final Color MINT = new Color(26, 188, 156);

    public static Color getAccentGradient(float position) {
        float r1 = ACCENT_GRADIENT_START.getRed() / 255f;
        float g1 = ACCENT_GRADIENT_START.getGreen() / 255f;
        float b1 = ACCENT_GRADIENT_START.getBlue() / 255f;
        float r2 = ACCENT_GRADIENT_END.getRed() / 255f;
        float g2 = ACCENT_GRADIENT_END.getGreen() / 255f;
        float b2 = ACCENT_GRADIENT_END.getBlue() / 255f;

        float t = position;
        int r = (int)((r1 + (r2 - r1) * t) * 255);
        int g = (int)((g1 + (g2 - g1) * t) * 255);
        int b = (int)((b1 + (b2 - b1) * t) * 255);
        return new Color(
            Math.min(255, Math.max(0, r)),
            Math.min(255, Math.max(0, g)),
            Math.min(255, Math.max(0, b))
        );
    }

    public static GradientPaint createGradient(int x1, int y1, int x2, int y2) {
        return new GradientPaint(x1, y1, ACCENT_GRADIENT_START, x2, y2, ACCENT_GRADIENT_END);
    }
}