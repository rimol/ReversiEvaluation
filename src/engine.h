#include "bitboard.h"
#include "eval.h"
#include "search.h"
#include <random>
#include <unordered_map>

struct MoveWithScore {
    int move;
    double score;
};

class ReversiEngine {
public:
    // 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
    virtual int chooseMove(Bitboard p, Bitboard o, int depth) = 0;
    virtual ~ReversiEngine() {}
};

class RandomEngine : public ReversiEngine {
    std::mt19937 mt;

public:
    // 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
    int chooseMove(Bitboard p, Bitboard o, int depth);

    RandomEngine();
};

// AlphaBetaを実装しただけのやつ。negaScout版とかのテスト用に使う予定
// これをNegaScoutの親にするとよさそう.
class AlphaBetaEngine : public ReversiEngine {
protected:
    const Evaluator &evaluator;
    double negaAlpha(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth);

public:
    virtual std::vector<MoveWithScore> evalAllMoves(Bitboard p, Bitboard o, int depth);
    // 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
    virtual int chooseMove(Bitboard p, Bitboard o, int depth);

    AlphaBetaEngine(const Evaluator &evaluator);
    virtual ~AlphaBetaEngine() {}
};

// 本命
class NegaScoutEngine : public AlphaBetaEngine {
    std::unordered_map<PositionKey, SearchedPosition<double>, PositionKey::PositionHash> transpositionTable;
    double negaScout(Bitboard p, Bitboard o, double alpha, double beta, bool passed, int depth);

public:
    std::vector<MoveWithScore> evalAllMoves(Bitboard p, Bitboard o, int depth);
    // 返り値は石を置くマス番号。(bitboard.h参照、0<=sq<=63)
    int chooseMove(Bitboard p, Bitboard o, int depth);

    NegaScoutEngine(const Evaluator &evaluator);
    ~NegaScoutEngine() {}
};