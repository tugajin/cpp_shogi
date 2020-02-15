#include <string>
#include "sfen.hpp"
#include "pos.hpp"
#include "util.hpp"

Square sfen_square(int sq) {
    const auto f = square_file(Square(sq));
    const auto r = RANK_SIZE - square_rank(Square(sq)) - 1;
    return square_make(File(r), Rank(f));
}

Pos pos_from_sfen(const std::string &s) {
    
    auto i = 0;
    if(s[i] == ' ') { i++; }
    bit::Bitboard piece_side[PIECE_SIDE_SIZE];
    for(auto &bb : piece_side) {
        bb.init();
    }
    auto sq = 0;
    auto run = 0;
    auto prom_flag = false;
    while(true) {
        auto c = s[i++];
        if(c == '\0' || c == ' ') { break; }
        if(c == '/') {
            sq += run;
            run = 0;
            if(sq >= SQUARE_SIZE) {
                throw BadInput();
            }
        } else if(std::isdigit(c)) {
            run = run * 10 + (c-'0');
        } else if(c == '+'){
            prom_flag = true;
            continue;
        }else {
            sq += run;
            run = 0;
            if (sq >= SQUARE_SIZE) {
                throw BadInput();
            }
            auto ps = PieceSide(find(c,PieceSfenChar)/2);//HAC
            if (prom_flag) { ps = piece_side_prom(ps); }
            piece_side[ps].set(sfen_square(sq));
            sq++;
            prom_flag = false;
        }
    }
    if(s[i] == ' ') { i++; }
    auto turn = BLACK;
    if(s[i] != '\0') {
        turn = Side(find(s[i++],SideChar));
    }
    if(s[i] == ' ') { i++; }
    int hand[PIECE_SIDE_SIZE] = {};
    auto num = 0;
    while (true) {
        auto c = s[i++];
        if (c == ' ') {
            break;
        } else if (c == '-') {
            break;
        } else if (std::isdigit(c)) {
            num = (num) ? 10 * num + int(c - '0') : int(c - '0');
        } else {
            std::string pc_str = { c };
            auto ps = PieceSide(find(c,PieceSfenChar)/2);//HAC
            num = (num) ? num : 1;
            hand[ps] = num;
            num = 0;
        }
    }
    return Pos(turn,piece_side,hand);
}

std::string out_sfen(const  Pos &pos) {
    std::string s = "";
    auto num = 0;
    RANK_FOREACH(r) {
        FILE_FOREACH_REV(f) {
            const auto sq = square_make(f,r);
            const auto pc = pos.piece(sq);
            if(pc == PieceNone) {
                num++;
            } else {
                if(num) {
                    s += ml::to_string(num);
                }
                const auto sd = pos.side(sq);
                const auto pd = piece_side_make(pc,sd);
                s +=   ml::rtrim(piece_side_to_sfen(pd));
                num = 0;
            }
        }
        if(r + 1 != RANK_SIZE) {
            if(num) {
                s += ml::to_string(num);
            }
            s += "/";
            num = 0;
        }
    }
    s += " ";
    SIDE_FOREACH(sd) {
        HAND_FOREACH(pc) {
            const auto pd = piece_side_make(pc,sd);
            const auto num = hand_num(pos.hand(sd),pc);
            if(!num) {
            } else if(num > 1) {
                s += ml::to_string(num);
            }  else {
                s += ml::rtrim(piece_side_to_sfen(pd));
            }
        }
    }
    return s;
}