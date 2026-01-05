#include "Game.hpp"

// --- IMPLEMENTATION ---

MigrationGame::MigrationGame(int size, int difficulty) : n(size), currentPlayer(1) {
    bot = std::make_unique<AIPlayer>(difficulty);
    initBoard();
}

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
                int nx = x+dx, ny = y+dy;
                // COLLISION CHECK: b[nx][ny] == 0
                if(nx>=0 && ny>=0 && nx<n && ny<n && b[nx][ny] == 0) {
                    moves.push_back({x,y,nx,ny});
                }
            }
        }
    }
    return moves;
}

std::vector<Move> MigrationGame::getMoves(int p) { return getMovesStatic(p, board, n); }

void MigrationGame::makeMove(int x1, int y1, int x2, int y2) {
    // 1. Check Bounds
    if(!isValid(x1,y1) || !isValid(x2,y2)) return;
    
    // 2. Check Source
    int p = board[x1][y1];
    if(p == 0) return;
    
    // 3. COLLISION CHECK (Crucial Fix)
    if(board[x2][y2] != 0) return; // Cannot move if blocked!

    board[x2][y2] = p;
    board[x1][y1] = 0;
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
}

Move MigrationGame::calculateBotMove() {
    auto boardCopy = board;
    int currentN = n;
    std::future<Move> futureMove = std::async(std::launch::async, [this, boardCopy, currentN](){
        return bot->decideMove(boardCopy, currentN);
    });
    return futureMove.get();
}

void MigrationGame::runBot() {
    Move m = calculateBotMove();
    if(m.x1 != -1) makeMove(m.x1, m.y1, m.x2, m.y2);
}

int MigrationGame::getCell(int x, int y) { return board[x][y]; }
bool MigrationGame::isGameOver() { return getMoves(currentPlayer).empty(); }

void MigrationGame::saveGame(std::string filename) {
    std::ofstream f(filename);
    f << n << " " << currentPlayer << "\n";
    for(auto& row : board) {
        for(int c : row) f << c << " ";
        f << "\n";
    }
    f.close();
}

// --- AI PLAYER ---
Move AIPlayer::decideMove(const std::vector<std::vector<int>>& board, int n) {
    std::vector<std::vector<int>> b = board;
    std::vector<Move> moves = MigrationGame::getMovesStatic(2, b, n);
    if(moves.empty()) return {-1,-1,-1,-1};
    
    Move bestMove = moves[0];
    int maxEval = -100000;

    for(const auto& m : moves) {
        b[m.x2][m.y2] = 2; b[m.x1][m.y1] = 0;
        int eval = minimax(b, maxDepth - 1, false, -100000, 100000, n);
        b[m.x1][m.y1] = 2; b[m.x2][m.y2] = 0;
        if(eval > maxEval) { maxEval = eval; bestMove = m; }
    }
    return bestMove;
}

int AIPlayer::evaluate(const std::vector<std::vector<int>>& b, int n) {
    int score = 0;
    for(int x=0; x<n; x++) {
        for(int y=0; y<n; y++) {
            if(b[x][y] == 2) score += x * 10;      // AI wants High X
            if(b[x][y] == 1) score -= (n-y) * 10;  // Player wants Low Y
        }
    }
    return score;
}

int AIPlayer::minimax(std::vector<std::vector<int>>& b, int depth, bool isMax, int alpha, int beta, int n) {
    if(depth == 0) return evaluate(b, n);
    int player = isMax ? 2 : 1;
    auto moves = MigrationGame::getMovesStatic(player, b, n);
    if(moves.empty()) return isMax ? -10000 : 10000;

    if(isMax) {
        int maxEval = -100000;
        for(const auto& m : moves) {
            b[m.x2][m.y2] = 2; b[m.x1][m.y1] = 0;
            int eval = minimax(b, depth-1, false, alpha, beta, n);
            b[m.x1][m.y1] = 2; b[m.x2][m.y2] = 0;
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if(beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = 100000;
        for(const auto& m : moves) {
            b[m.x2][m.y2] = 1; b[m.x1][m.y1] = 0;
            int eval = minimax(b, depth-1, true, alpha, beta, n);
            b[m.x1][m.y1] = 1; b[m.x2][m.y2] = 0;
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha) break;
        }
        return minEval;
    }
}

// --- EXPORTS ---
#ifdef CONSOLE_APP
int main() { /* Same as before */ return 0; }
#else
extern "C" {
#include <jni.h>
static MigrationGame* g = nullptr;
JNIEXPORT void JNICALL Java_Migration_initGame(JNIEnv*, jobject, jint n, jint d){ if(g) delete g; g = new MigrationGame(n, d); }
JNIEXPORT jint JNICALL Java_Migration_getCell(JNIEnv*, jobject, jint x, jint y){ return g ? g->getCell(x,y) : 0; }
JNIEXPORT jint JNICALL Java_Migration_getPlayer(JNIEnv*, jobject){ return g ? g->getCurrentPlayer() : 0; }
JNIEXPORT void JNICALL Java_Migration_applyMove(JNIEnv*, jobject, jint x1, jint y1, jint x2, jint y2){ if(g) g->makeMove(x1,y1,x2,y2); }
JNIEXPORT jboolean JNICALL Java_Migration_isOver(JNIEnv*, jobject){ return g ? g->isGameOver() : true; }
JNIEXPORT void JNICALL Java_Migration_saveNative(JNIEnv* env, jobject, jstring f){ if(!g)return; const char *s = env->GetStringUTFChars(f, 0); g->saveGame(std::string(s)); env->ReleaseStringUTFChars(f, s); }
JNIEXPORT jintArray JNICALL Java_Migration_getBotMove(JNIEnv* env, jobject){
    if(!g) return nullptr; Move m = g->calculateBotMove();
    jintArray res = env->NewIntArray(4); jint f[4] = {m.x1, m.y1, m.x2, m.y2}; env->SetIntArrayRegion(res, 0, 4, f); return res;
}
}
#endif