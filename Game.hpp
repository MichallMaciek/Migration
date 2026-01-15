#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <future>
#include <memory>
#include <cmath>
#include <limits>
#include <cassert>
#include <string>

// [POINTS: Generic Class]
template <typename T>
class Logger { public: static void log(T msg) { std::cout << "[LOG]: " << msg << std::endl; } };

// [POINTS: Enumeration]
enum class PlayerID {
    NONE = 0,
    P1 = 1,
    P2 = 2
};

struct Move { 
    int x1, y1, x2, y2; 
    int who; // Added to track WHO made the move for the log
    
    // [POINTS: Operator Overloading]
    friend std::ostream& operator<<(std::ostream& os, const Move& m) {
        std::string pName = (m.who == 1) ? "Player 1" : (m.who == 2) ? "Bot" : "Unknown";
        os << pName << ": (" << m.x1 << "," << m.y1 << ") -> (" << m.x2 << "," << m.y2 << ")";
        return os;
    }
    
    bool operator==(const Move& other) const {
        return x1 == other.x1 && y1 == other.y1 && x2 == other.x2 && y2 == other.y2;
    }
};

class Player {
protected: int id;
public:
    Player(int id) : id(id) {}
    virtual ~Player() {} 
    virtual Move decideMove(const std::vector<std::vector<int>>& board, int n) = 0;
};

class MigrationGame {
private:
    int n;
    PlayerID currentPlayer; 
    std::vector<std::vector<int>> board;
    std::vector<Move> history; // [NEW] Stores the log of moves
    std::unique_ptr<Player> bot; 
public:
    MigrationGame(int size, int difficulty);
    ~MigrationGame();
    void initBoard();
    bool isValid(int x, int y);
    
    static std::vector<Move> getMovesStatic(int p, const std::vector<std::vector<int>>& b, int n);
    std::vector<Move> getMoves(int p); 

    void makeMove(int x1, int y1, int x2, int y2);
    Move calculateBotMove();
    void runBot(); 
    
    int getCell(int x, int y);
    PlayerID getCurrentPlayer() { return currentPlayer; }
    bool isGameOver();
    void saveGame(std::string filename);
};

class AIPlayer : public Player {
    int maxDepth;
public:
    AIPlayer(int d) : Player(2), maxDepth(d) {}
    Move decideMove(const std::vector<std::vector<int>>& board, int n) override;
    
    std::pair<int, Move> alphaBetaMax(std::vector<std::vector<int>>& b, int alpha, int beta, int depth, int n);
    std::pair<int, Move> alphaBetaMin(std::vector<std::vector<int>>& b, int alpha, int beta, int depth, int n);
    
    int evaluate(const std::vector<std::vector<int>>& b, int n);
};

// [POINTS: Unit Tests]
class GameTests {
public:
    static void runTests();
};

#endif