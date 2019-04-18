#include <direct.h>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include "bitboard.h"
#include "recgen.h"
#include "recode.h"
#include "solver.h"

void generateRecode(int n, int depth, std::string folderPath) {
    // 指定されたフォルダに保存用フォルダを作成
    // フォルダ名は現在時刻
    std::stringstream ss;
    ss << folderPath;
    if (folderPath.back() != '\\') ss << '\\';
    ss << time(NULL);// << ".bin";

    // フォルダ作成
    mkdir(ss.str().c_str());

    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        std::stringstream _ss;
        _ss << ss.str() << "\\" << (i + 1) << ".bin";
        ofss[i] = std::ofstream(_ss.str(), std::ios::binary);
    }

    std::random_device rnd; // 非決定的乱数生成器、これでメルセンヌ・ツイスタのシードを設定
    std::mt19937 mt(rnd());

    Recode recodes[60 + 1]; // 初期盤面 + 60手分

    // n回ループ
    for (int i = 0; i < n; ++i) {
        bool passed = false;
        int result = 0;
        // 終局までの手数(最初から4つマスが埋まっているので、最大60)
        int turns = 60;
        // 初期
        recodes[0] = Recode(0x0000000810000000ULL, 0x0000001008000000ULL, 0);
        
        for (int j = 1; j <= 60; ++j) {
            const Recode current = recodes[j - 1];

            Bitboard moves = getMoves(current.p, current.o);
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
                    // 手番を変える
                    recodes[--j] = Recode(current.o, current.p, current.turn);
                    continue;
                }
            }
            else {
                Recode next(current.o, current.p, j);
                if (j <= 60 - depth) {
                    // 打つ手をランダムに決める
                    int chosen = mt() % (popcount(moves));
                    // 下からビットを剥がしていく
                    while (chosen--) moves &= moves - 1ULL;
                    // 一番下以外のビットを消す
                    Bitboard sqbit = -moves & moves;
                    Bitboard flip = getFlip(current.p, current.o, sqbit);

                    next.o ^= flip | sqbit;
                    next.p ^= flip;
                }
                // 残りdepthマスからsolverを使います
                else {
                    int minscore = 64;
                    Bitboard bestMove = 0ULL; // 適当
                    Bitboard bestFlip = 0ULL; // 適当
                    // 1手すすめて、相手から見たスコアを計算、それが最小になるように取る
                    while (moves) {
                        Bitboard sqbit = moves & -moves;
                        Bitboard flip = getFlip(current.p, current.o, sqbit);

                        int score = solve(current.o ^ flip, current.p ^ flip ^ sqbit);
                        if (score < minscore) {
                            minscore = score;
                            bestMove = sqbit;
                            bestFlip = flip;
                        }

                        moves ^= sqbit;
                    }

                    next.o ^= bestFlip | bestMove;
                    next.p ^= bestFlip;
                }

                recodes[j] = next;
                passed = false;
            }
        }

        result = popcount(recodes[turns].p) - popcount(recodes[turns].o);

        // さっき作ったフォルダ内に保存していく
        // 初期盤面(0手目終了時)は入れない。
        for (int j = 1; j <= turns; ++j) {
            recodes[j].result = result;
            ofss[j - 1].write(reinterpret_cast<char*>(&recodes[j]), sizeof(Recode));
        }
    }
}