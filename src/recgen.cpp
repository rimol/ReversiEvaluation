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

    std::vector<Board> boards(60);
    for (int i = 0; i < n; ++i) {
        bool passed = false;
        int result = 0;
        // 終局までの手数
        int turns = 60;
        // 初期
        boards[0] = Board(Black, 0x0000000810000000ULL, 0x0000001008000000ULL);
        
        for (int j = 1; j < 60; ++j) {
            Board current = boards[j - 1];
            int c = (int)current.c;
            Bitboard p = current.bits[c];
            Bitboard o = current.bits[c ^ 1];

            printBitboard(p);
            printBitboard(o);

            Bitboard moves = getMoves(p, o);
            if (moves == 0ULL) {
                if (passed) {
                    result = popcount(current.bits[Black]) - popcount(current.bits[White]);
                    turns = j;
                    break;
                }
                else {
                    passed = true;
                    boards[--j] = Board((Color)(c ^ 1), o, p);
                    continue;
                }
            }
            else {
                // 打つ手をランダムに決める
                int chosen = mt() % (popcount(moves));
                // 下からビットを剥がしていく
                while (chosen--) moves &= moves - 1;
                // 一番下以外のビットを消す
                Bitboard sqbit = -moves & moves;
                Bitboard flip = getFlip(p, o, sqbit);

                p ^= flip | sqbit;
                o ^= flip;

                boards[j] = Board((Color)(c ^ 1), o, p);
            }
        }

        if (turns == 60) {
            result = popcount(boards[59].bits[Black]) - popcount(boards[59].bits[White]);
        }

        // さっき作ったフォルダ内に保存していく
        for (int j = 0; j < turns; ++j) {
            Recode recode(boards[j], j, result);
            ofs.write(reinterpret_cast<char*>(&recode), sizeof(Recode));
        }
    }
}