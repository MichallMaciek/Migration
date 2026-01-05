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

template <typename T>
class Logger { public: static void log(T msg) { std::cout << "[LOG]: " << msg << std::endl; } };

struct Move { int x1, y1, x2, y2; };

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
    int currentPlayer;
    std::vector<std::vector<int>> board;
    std::unique_ptr<Player> bot;
public:
    MigrationGame(int size, int difficulty);
    ~MigrationGame();
    void initBoard();
    bool isValid(int x, int y);
    
    // Core logic made static/public helpers for Minimax access
    static std::vector<Move> getMovesStatic(int p, const std::vector<std::vector<int>>& b, int n);
    std::vector<Move> getMoves(int p); 

    void makeMove(int x1, int y1, int x2, int y2);
    Move calculateBotMove();
    void runBot(); 
    
    int getCell(int x, int y);
    int getCurrentPlayer() { return currentPlayer; }
    bool isGameOver();
    void saveGame(std::string filename);
};

class AIPlayer : public Player {
    int maxDepth;
public:
    AIPlayer(int d) : Player(2), maxDepth(d) {}
    Move decideMove(const std::vector<std::vector<int>>& board, int n) override;
    
    // Minimax Algorithm
    int minimax(std::vector<std::vector<int>>& b, int depth, bool isMax, int alpha, int beta, int n);
    int evaluate(const std::vector<std::vector<int>>& b, int n);
};

#endif