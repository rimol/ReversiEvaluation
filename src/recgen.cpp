#include <direct.h>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include "engine.h"
#include "bitboard.h"
#include "recgen.h"
#include "recode.h"
#include "solver.h"

// 棋譜が正しく生成されているかチェックする
/*
    チェックポイント:
    * 石の数
    * 
*/
// static bool isValidRecodes(const Recode* recodes, int turns) {
//     for (int i = 0; i < turns; ++i) {
//         int stoneCount = popcount(recodes[i].p() | recodes[i].o());
//         if (stoneCount != i + 4) return false;
//     }
// }

void generateRecode(int n, int depth, std::string folderPath) {
    // 指定されたフォルダに保存用フォルダを作成
    // フォルダ名は現在時刻
    if (folderPath.back() != '\\') folderPath += '\\';
    folderPath += std::to_string(time(NULL)) + '\\';

    // フォルダ作成
    mkdir(folderPath.c_str());

    std::ofstream ofss[60];
    for (int i = 0; i < 60; ++i) {
        ofss[i] = std::ofstream(folderPath + std::to_string(i + 1) + ".bin", std::ios::binary);
    }

    std::random_device rnd; // 非決定的乱数生成器、これでメルセンヌ・ツイスタのシードを設定
    std::mt19937 mt(rnd());

    Recode recodes[60 + 1]; // 初期盤面 + 60手分

    // n回ループ
    for (int i = 0; i < n; ++i) {
        // 終局までの手数(最初から4つマスが埋まっているので、最大60)
        int turns = 60;
        bool passed = false;
        // 初期
        recodes[0] = Recode(0x0000000810000000ULL, 0x0000001008000000ULL, Black, 0);
        
        for (int j = 1; j <= 60 - depth; ++j) {
            const Recode current = recodes[j - 1];
            Bitboard moves = getMoves(current.p(), current.o());
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
                    recodes[--j].c = ~current.c;
                    continue;
                }
            }
            else {
                Recode next(current.board[Black], current.board[White], ~current.c, j);
                int sq = chooseRandomMove(current.p(), current.o(), mt);
                Bitboard flip = getFlip(current.p(), current.o(), 1ULL << sq);
                next.board[current.c] ^= flip | (1ULL << sq);
                next.board[~current.c] ^= flip;
                recodes[j] = next;
                passed = false;
            }
        }

        // 残りdepthマスでsolverを使う.
        Solution solution = solve(recodes[60 - depth].p(), recodes[60 - depth].o());
        turns = 60 - depth + solution.bestMoves.size();
        for (int j = 60 - depth + 1; j <= turns; ++j) {
            Recode current = recodes[j - 1];
            Bitboard sqbit = 1ULL << solution.bestMoves[j - (60 - depth + 1)];
            Bitboard flip = getFlip(current.p(), current.o(), sqbit);
            // パスでない
            if (flip != 0ULL) {
                Recode next(current.board[Black], current.board[White], ~current.c, j);
                next.board[current.c] ^= flip | sqbit;
                next.board[~current.c] ^= flip;
                recodes[j] = next;
            }
            else {
                recodes[--j].c = ~current.c;
                continue;
            }
        }

        // さっき作ったフォルダ内に保存していく
        int result = popcount(recodes[turns].board[Black]) - popcount(recodes[turns].board[White]);
        // 初期盤面(0手目終了時)は入れない。
        for (int j = 1; j <= turns; ++j) {
            recodes[j].result = ((recodes[j].c ^ 1 << 1) - 1) * result;
            ofss[j - 1].write(reinterpret_cast<char*>(&recodes[j]), sizeof(Recode));
        }
    }
}