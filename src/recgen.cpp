// #include <direct.h>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include "bitboard.h"
#include "board.h"
#include "recgen.h"
#include "recode.h"
#include "solver.h"

void generateRecode(int n, std::string folderPath) {
    // 指定されたフォルダに保存用ファイルを作成
    // ファイル名は現在時刻
    std::stringstream ss;
    ss << folderPath;
    if (folderPath.back() != '\\') ss << '\\';
    ss << time(NULL) << ".bin";

    // ファイルを作成し、バイナリモードで開く
    std::ofstream ofs(ss.str(), std::ios::binary);

    std::random_device rnd; // 非決定的乱数生成器、これでメルセンヌ・ツイスタのシードを設定
    std::mt19937 mt(rnd());

    std::vector<Board> boards(60 + 1); // 初期盤面 + 60手分

    // n回ループ
    for (int i = 0; i < n; ++i) {
        bool passed = false;
        int result = 0;
        // 終局までの手数(最初から4つマスが埋まっているので、最大60)
        int turns = 60;
        // 初期
        boards[0] = Board(Black, 0x0000000810000000ULL, 0x0000001008000000ULL);
        
        for (int j = 1; j <= 60; ++j) {
            Board current = boards[j - 1];
            Color c = current.c;
            Bitboard p = current.bits[c];
            Bitboard o = current.bits[~c];

            // printBoard(current); // 0-59までの盤面を表示できたら正解

            Bitboard moves = getMoves(p, o);
            // 打つ手がない！
            if (moves == 0ULL) {
                // ２回連続パス=終局
                if (passed) {
                    // j手目が打てないので、j-1手で終局
                    turns = j - 1;
                    break;
                }
                else {
                    passed = true;
                    boards[--j] = c == Black ? Board(~c, p, o) : Board(~c, o, p);
                    continue;
                }
            }
            else {
                if (j <= 50) {
                    // 打つ手をランダムに決める
                    int chosen = mt() % (popcount(moves));
                    // 下からビットを剥がしていく
                    while (chosen--) moves &= moves - 1ULL;
                    // 一番下以外のビットを消す
                    Bitboard sqbit = -moves & moves;
                    Bitboard flip = getFlip(p, o, sqbit);

                    p ^= flip | sqbit;
                    o ^= flip;
                }
                // 残り10マスからソルバーを使います
                else {
                    int minscore = 64;
                    Bitboard bestMove = 0ULL; // 適当
                    Bitboard bestFlip = 0ULL; // 適当
                    // 1手すすめて、相手から見たスコアを計算、それが最小になるように取る
                    while (moves) {
                        Bitboard sqbit = moves & -moves;
                        Bitboard flip = getFlip(p, o, sqbit);

                        int score = solve(o ^ flip, p ^ flip ^ sqbit);
                        if (score < minscore) {
                            minscore = score;
                            bestMove = sqbit;
                            bestFlip = flip;
                        }

                        moves ^= sqbit;
                    }

                    p ^= bestFlip | bestMove;
                    o ^= bestFlip;
                }

                boards[j] = c == Black ? Board(~c, p, o) : Board(~c, o, p);
                passed = false;
            }
        }

        // std::cout << turns << std::endl; // これ意味ある？（草）
        result = popcount(boards[turns].bits[Black]) - popcount(boards[turns].bits[White]);

        // さっき作ったフォルダ内に保存していく
        // 初期盤面(0手目終了時)は入れない。
        for (int j = 1; j <= turns; ++j) {
            Recode recode(boards[j], j, result);
            ofs.write(reinterpret_cast<char*>(&recode), sizeof(Recode));
        }
    }
}