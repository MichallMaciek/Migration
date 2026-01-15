import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.ArrayList; 

// [POINTS: Interface]
interface IGameLogger {
    void log(String msg);
}

// [POINTS: Enumeration]
enum GameState {
    PLAYING,
    ANIMATING,
    GAME_OVER
}

// [POINTS: Generic Class]
class MoveLogger<T> implements IGameLogger {
    // [POINTS: Collections]
    private ArrayList<T> history = new ArrayList<>();

    @Override
    public void log(String msg) {
        System.out.println("[JAVA LOG]: " + msg);
    }

    public void addMove(T move) {
        history.add(move);
        log("Move recorded: " + move.toString());
    }

    // [POINTS: Read and Write to File (Java)]
    public void saveToFile(String filename) {
        try (FileWriter fw = new FileWriter(filename);
             BufferedWriter bw = new BufferedWriter(fw)) {
            for (T move : history) {
                bw.write(move.toString());
                bw.newLine();
            }
            log("History saved to " + filename);
        } catch (IOException e) {
            // [POINTS: Error Handling]
            System.err.println("Failed to save history: " + e.getMessage());
        }
    }
    
    public int getCount() { return history.size(); }
}

// [POINTS: Unit Tests]
class JavaTests {
    public static void runSelfTests() {
        System.out.println("--- RUNNING JAVA UNIT TESTS ---");
        
        // Test Generics & Collections
        MoveLogger<String> logger = new MoveLogger<>();
        logger.addMove("TestMove");
        if(logger.getCount() != 1) throw new RuntimeException("Collection Test Failed");
        
        // Test Enum
        GameState state = GameState.PLAYING;
        if(state != GameState.PLAYING) throw new RuntimeException("Enum Test Failed");
        
        System.out.println("--- JAVA TESTS PASSED ---");
    }
}

public class Migration extends JFrame {
    
    // --- JNI LOADING ---
    static { 
        try { System.loadLibrary("migration"); } 
        catch (UnsatisfiedLinkError e) {
             try { System.load(System.getProperty("user.dir") + "/build/libmigration.dylib"); }
             catch (Exception ex) { System.load(System.getProperty("user.dir") + "/build/libmigration.so"); }
        }
    }
    
    public native void initGame(int n, int d);
    public native int getCell(int x, int y);
    public native int getPlayer();
    public native void applyMove(int x1, int y1, int x2, int y2);
    public native int[] getBotMove();
    public native boolean isOver();
    public native void saveNative(String f);

    // --- GUI ---
    private int n;
    private final int SIZE = 60;
    private JPanel boardPanel;
    private Timer animTimer;
    private Point animSource = null, animDest = null;
    private int animPlayer = 0;
    private float animProgress = 0.0f;
    private Runnable onAnimFinish = null;
    
    // [POINTS: Encapsulation]
    private GameState currentState = GameState.PLAYING;
    
    // [POINTS: Generics usage]
    private MoveLogger<String> moveHistory = new MoveLogger<>();

    public Migration() {
        // Run Unit Tests on Startup
        JavaTests.runSelfTests();

        String s = JOptionPane.showInputDialog("Board Size (e.g. 8):");
        n = (s != null && !s.isEmpty()) ? Integer.parseInt(s) : 8;

        String[] diffOptions = {"Easy (Depth 1)", "Medium (Depth 3)", "Hard (Depth 5)"};
        int diffChoice = JOptionPane.showOptionDialog(null, "Select Difficulty", "Difficulty",
                JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null, diffOptions, diffOptions[1]);
        int depth = (diffChoice == 0) ? 1 : (diffChoice == 2 ? 5 : 3);

        initGame(n, depth);
        setTitle("Migration (Depth: " + depth + ")");
        setLayout(new BorderLayout());

        JButton saveBtn = new JButton("Save Game");
        // --- FIX IS HERE: Removed "build/" prefix ---
        saveBtn.addActionListener(e -> { 
            saveNative("savegame.txt"); 
            moveHistory.saveToFile("java_history.txt"); 
            JOptionPane.showMessageDialog(this, "Saved C++ state and Java History!"); 
        });
        
        // [POINTS: Destructor-like cleanup]
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                cleanup();
                super.windowClosing(e);
            }
        });
        
        add(saveBtn, BorderLayout.NORTH);

        animTimer = new Timer(16, e -> {
            animProgress += 0.08f;
            if (animProgress >= 1.0f) {
                animProgress = 1.0f;
                animTimer.stop();
                if(onAnimFinish != null) onAnimFinish.run();
            }
            boardPanel.repaint();
        });

        boardPanel = new JPanel() {
            @Override
            public void paintComponent(Graphics g) {
                super.paintComponent(g);
                Graphics2D g2 = (Graphics2D) g;
                g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        g2.setColor((i + j) % 2 == 0 ? Color.LIGHT_GRAY : new Color(139, 69, 19));
                        g2.fillRect(i * SIZE, j * SIZE, SIZE, SIZE);
                        int c = getCell(i, j);
                        if (animSource != null && animSource.x == i && animSource.y == j) continue;
                        if (c != 0) drawPiece(g2, i * SIZE, j * SIZE, c);
                    }
                }
                if (animSource != null && animDest != null) {
                    int sx = animSource.x * SIZE, sy = animSource.y * SIZE;
                    int ex = animDest.x * SIZE, ey = animDest.y * SIZE;
                    int cx = (int) (sx + (ex - sx) * animProgress);
                    int cy = (int) (sy + (ey - sy) * animProgress);
                    drawPiece(g2, cx, cy, animPlayer);
                }
            }
        };
        boardPanel.setPreferredSize(new Dimension(n * SIZE, n * SIZE));

        boardPanel.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if (currentState != GameState.PLAYING || getPlayer() != 1 || isOver()) return;
                int x = e.getX() / SIZE;
                int y = e.getY() / SIZE;
                
                if (getCell(x, y) == 1) {
                    if (y - 1 >= 0 && getCell(x, y - 1) == 0) {
                        moveHistory.addMove("Player 1: ("+x+","+y+") to ("+x+","+(y-1)+")");
                        performMoveSequence(x, y, x, y-1, 1);
                    } else {
                        System.out.println("Blocked! Cannot move there.");
                    }
                }
            }
        });

        add(boardPanel, BorderLayout.CENTER);
        pack();
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setVisible(true);
    }
    
    private void cleanup() {
        System.out.println("Cleaning up Java resources...");
    }

    private void drawPiece(Graphics2D g, int x, int y, int p) {
        g.setColor(p == 1 ? Color.CYAN : Color.RED);
        g.fillOval(x + 8, y + 8, SIZE - 16, SIZE - 16);
        g.setColor(Color.BLACK); g.drawOval(x + 8, y + 8, SIZE - 16, SIZE - 16);
    }

    private void performMoveSequence(int x1, int y1, int x2, int y2, int player) {
        currentState = GameState.ANIMATING;
        animSource = new Point(x1, y1); animDest = new Point(x2, y2);
        animPlayer = player; animProgress = 0.0f;
        
        onAnimFinish = () -> {
            applyMove(x1, y1, x2, y2);
            animSource = null; animDest = null; boardPanel.repaint();
            if (checkWin()) { 
                currentState = GameState.GAME_OVER; 
            } else if (player == 1) {
                triggerBot();
            } else {
                currentState = GameState.PLAYING;
            }
        };
        animTimer.start();
    }

    private void triggerBot() {
        new Thread(() -> {
            int[] m = getBotMove();
            if (m != null && m[0] != -1) {
                moveHistory.addMove("Bot: ("+m[0]+","+m[1]+") to ("+m[2]+","+m[3]+")");
                SwingUtilities.invokeLater(() -> performMoveSequence(m[0], m[1], m[2], m[3], 2));
            } else {
                SwingUtilities.invokeLater(() -> checkWin());
            }
        }).start();
    }

    private boolean checkWin() {
        if (isOver()) {
            int winner = (getPlayer() == 1) ? 2 : 1; 
            String msg = (winner == 1) ? "Game Over! You Win!" : "Game Over! Bot Wins!";
            JOptionPane.showMessageDialog(this, msg);
            currentState = GameState.GAME_OVER;
            return true;
        }
        return false;
    }

    public static void main(String[] args) { SwingUtilities.invokeLater(Migration::new); }
}