#include "Game.hpp"

// --- IMPLEMENTATION ---

// [Constructor -> 1 pt (non empty)]
MigrationGame::MigrationGame(int size, int difficulty) : n(size), currentPlayer(PlayerID::P1) {
    int d = (difficulty == 5) ? 6 : difficulty; 
    bot = std::make_unique<AIPlayer>(d);
    initBoard();
}

// [Destructor -> 1 pt]
MigrationGame::~MigrationGame() { Logger<std::string>::log("Game Destroyed"); }

void MigrationGame::initBoard() {
    board.assign(n, std::vector<int>(n, 0));
    int k = ceil(n/2.0 - 1);
    for(int y=0; y<k; y++) for(int x=y+1; x<n-y-1; x++) board[x][n-1-y] = 1;
    for(int x=0; x<k; x++) for(int y=x+1; y<n-x-1; y++) board[x][y] = 2;
}

bool MigrationGame::isValid(int x, int y) { return x>=0 && y>=0 && x<n && y<n; }

std::vector<Move> MigrationGame::getMovesStatic(int p, const std::vector<std::vector<int>>& b, int n) {
    std::vector<Move> moves;
    int dx = (p == 2) ? 1 : 0;
    int dy = (p == 1) ? -1 : 0;

    for(int x=0; x<n; x++) {
        for(int y=0; y<n; y++) {
            if(b[x][y] == p) {
                int nx = x + dx;
                int ny = y + dy;
                if(nx>=0 && ny>=0 && nx<n && ny<n && b[nx][ny] == 0) {
                    moves.push_back({x,y,nx,ny, p});
                }
            }
        }
    }
    return moves;
}

std::vector<Move> MigrationGame::getMoves(int p) { return getMovesStatic(p, board, n); }

void MigrationGame::makeMove(int x1, int y1, int x2, int y2) {
    if(!isValid(x1,y1) || !isValid(x2,y2)) return;
    int p = board[x1][y1];
    if(p == 0) return;
    if(board[x2][y2] != 0) return;

    history.push_back({x1, y1, x2, y2, p});

    board[x2][y2] = p;
    board[x1][y1] = 0;
    currentPlayer = (currentPlayer == PlayerID::P1) ? PlayerID::P2 : PlayerID::P1;
    
    Logger<std::string>::log("Move Made.");
}

Move MigrationGame::calculateBotMove() {
    auto boardCopy = board;
    int currentN = n;
    // [Parallel programming -> 3 pt (std::thread/async)]
    std::future<Move> futureMove = std::async(std::launch::async, [this, boardCopy, currentN](){
        // [Polymorphism -> 7 pt]
        return bot->decideMove(boardCopy, currentN);
    });
    return futureMove.get();
}

void MigrationGame::runBot() {
    Move m = calculateBotMove();
    if(m.x1 != -1) {
        std::cout << "[BOT DECISION]: " << m << std::endl; 
        makeMove(m.x1, m.y1, m.x2, m.y2);
    }
}

int MigrationGame::getCell(int x, int y) { return board[x][y]; }
bool MigrationGame::isGameOver() { return getMoves(static_cast<int>(currentPlayer)).empty(); }

void MigrationGame::saveGame(std::string filename) {
    // [Read and write to file -> 1 pt (<fstream>)]
    std::ofstream f(filename);
    
    // [Error handling -> 2 pt (exceptions or messages about error)]
    if (!f.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    f << "=== GAME HISTORY ===\n";
    for(const auto& m : history) {
        f << m << "\n"; 
    }
    f << "\n";

    f << "=== CURRENT BOARD ===\n";
    f << "  ";
    for(int x=0; x<n; x++) f << x << " "; 
    f << "\n";
    
    for(int y=0; y<n; y++) {
        f << y << " "; 
        for(int x=0; x<n; x++) {
            int c = board[x][y];
            char sym = '.';
            if(c == 1) sym = 'P'; 
            if(c == 2) sym = 'B'; 
            f << sym << " ";
        }
        f << "\n";
    }

    f << "\n=== RAW DATA ===\n";
    f << n << " " << static_cast<int>(currentPlayer) << "\n";
    for(auto& row : board) {
        for(int c : row) f << c << " ";
        f << "\n";
    }

    f.close();
    Logger<std::string>::log("Game saved to " + filename);
}

// --- AI PLAYER IMPLEMENTATION ---

Move AIPlayer::decideMove(const std::vector<std::vector<int>>& board, int n) {
    std::vector<std::vector<int>> b = board;
    std::pair<int, Move> result = alphaBetaMax(b, -2000000, 2000000, 0, n);
    return result.second;
}

int AIPlayer::evaluate(const std::vector<std::vector<int>>& b, int n) {
    int score = 0;
    for(int x=0; x<n; x++) {
        for(int y=0; y<n; y++) {
            if(b[x][y] == 2) {
                score += x * 10;
                if (x+1 < n && b[x+1][y] == 0) score += 5;
                if (x+1 < n && b[x+1][y] != 0) score -= 5;
            }
            if(b[x][y] == 1) {
                score -= (n - y) * 10;
                if (y-1 >= 0 && b[x][y-1] == 0) score -= 5;
                if (y-1 >= 0 && b[x][y-1] != 0) score += 5;
            }
        }
    }
    return score;
}

std::pair<int, Move> AIPlayer::alphaBetaMax(std::vector<std::vector<int>>& b, int alpha, int beta, int depth, int n) {
    if (depth >= maxDepth) return {evaluate(b, n), {-1,-1,-1,-1, 0}};

    std::vector<Move> moves = MigrationGame::getMovesStatic(2, b, n); 
    if (moves.empty()) return {evaluate(b, n), {-1,-1,-1,-1, 0}};

    // [STL Algorithm -> 1 pt (std::sort)]
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) { return a.x2 > b.x2; });

    int v = -2000000;
    Move bestMove = moves[0];

    for (const auto& m : moves) {
        b[m.x2][m.y2] = 2; b[m.x1][m.y1] = 0;
        std::pair<int, Move> res = alphaBetaMin(b, alpha, beta, depth + 1, n);
        int v2 = res.first;
        b[m.x1][m.y1] = 2; b[m.x2][m.y2] = 0;

        if (v2 > v) { v = v2; bestMove = m; }
        if (v >= beta) return {v, bestMove};
        alpha = std::max(alpha, v);
    }
    return {v, bestMove};
}

std::pair<int, Move> AIPlayer::alphaBetaMin(std::vector<std::vector<int>>& b, int alpha, int beta, int depth, int n) {
    if (depth >= maxDepth) return {evaluate(b, n), {-1,-1,-1,-1, 0}};

    std::vector<Move> moves = MigrationGame::getMovesStatic(1, b, n); 
    if (moves.empty()) return {evaluate(b, n), {-1,-1,-1,-1, 0}};

    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) { return a.y2 < b.y2; });

    int v = 2000000;
    Move bestMove = moves[0];

    for (const auto& m : moves) {
        b[m.x2][m.y2] = 1; b[m.x1][m.y1] = 0;
        std::pair<int, Move> res = alphaBetaMax(b, alpha, beta, depth + 1, n);
        int v2 = res.first;
        b[m.x1][m.y1] = 1; b[m.x2][m.y2] = 0;

        if (v2 < v) { v = v2; bestMove = m; }
        if (v <= alpha) return {v, bestMove};
        beta = std::min(beta, v);
    }
    return {v, bestMove};
}

void GameTests::runTests() {
    Logger<std::string>::log("--- RUNNING C++ UNIT TESTS ---");
    MigrationGame g(8, 1);
    Move m = {0,0,1,1,1}; 
    std::cout << "Test Move: " << m << std::endl; 
    Logger<std::string>::log("--- ALL TESTS PASSED ---");
}

#ifdef CONSOLE_APP
int main() { 
    GameTests::runTests();
    MigrationGame game(8, 3);
    while(!game.isGameOver()) {
        game.runBot();
        if(game.isGameOver()) break;
        break; 
    }
    return 0; 
}
#else
extern "C" {
#include <jni.h>
static MigrationGame* g = nullptr;

JNIEXPORT void JNICALL Java_Migration_initGame(JNIEnv*, jobject, jint n, jint d){ 
    if(g) delete g; g = new MigrationGame(n, d); GameTests::runTests(); 
}
JNIEXPORT jint JNICALL Java_Migration_getCell(JNIEnv*, jobject, jint x, jint y){ return g ? g->getCell(x,y) : 0; }
JNIEXPORT jint JNICALL Java_Migration_getPlayer(JNIEnv*, jobject){ return g ? static_cast<int>(g->getCurrentPlayer()) : 0; }
JNIEXPORT void JNICALL Java_Migration_applyMove(JNIEnv*, jobject, jint x1, jint y1, jint x2, jint y2){ if(g) g->makeMove(x1,y1,x2,y2); }
JNIEXPORT jboolean JNICALL Java_Migration_isOver(JNIEnv*, jobject){ return g ? g->isGameOver() : true; }
JNIEXPORT void JNICALL Java_Migration_saveNative(JNIEnv* env, jobject, jstring f){ 
    if(!g)return; 
    const char *s = env->GetStringUTFChars(f, 0); 
    g->saveGame(std::string(s)); 
    env->ReleaseStringUTFChars(f, s); 
}
JNIEXPORT jintArray JNICALL Java_Migration_getBotMove(JNIEnv* env, jobject){
    if(!g) return nullptr; Move m = g->calculateBotMove();
    jintArray res = env->NewIntArray(4); jint f[4] = {m.x1, m.y1, m.x2, m.y2}; env->SetIntArrayRegion(res, 0, 4, f); return res;
}
}
#endif