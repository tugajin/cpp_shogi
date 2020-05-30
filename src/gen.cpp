#include "common.hpp"
#include "gen.hpp"
#include "bit.hpp"
#include "pos.hpp"
#include "list.hpp"
#include "attack.hpp"
#include "sfen.hpp"


template<Piece pc, Side sd>void add_noprom_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	auto piece = (pc == Golds) ? pos.golds(sd) : pos.pieces(pc, sd);
	while (piece) {
		const auto from = piece.lsb();
		for (auto att = piece_attacks<pc>(sd, from, pos.pieces()) & target; att;) {
			const auto to = att.lsb();
			ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to)));
		}
	}
}
template<Side sd>void add_king_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	const auto from = pos.king(sd);
	for (auto att = get_king_attack(from) & target; att;) {
		const auto to = att.lsb();
		ml.add(move::make_move(from, to, King, pos.piece(to)));
	}
}
template<Side sd>void add_pawn_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	for (auto att = get_pawn_attack<sd>(pos.pieces(Pawn, sd)) & target; att;) {
		const auto to = att.lsb();
		const auto from = (sd == BLACK) ? to + SQ_RANK_INC : to - SQ_RANK_INC;
		ml.add(move::make_move(from, to, Pawn, pos.piece(to), square_is_prom<sd>(to)));
	}
}
template<Piece pc, Side sd>void add_bishop_rook_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	static_assert(pc == Bishop || pc == Rook, "pc error");
	for (auto piece = pos.pieces(pc, sd); piece;) {
		const auto from = piece.lsb();
		for (auto att = piece_attacks<pc>(sd, from, pos.pieces()) & target; att;) {
			const auto to = att.lsb();
			const auto prom = square_is_prom<sd>(from) || square_is_prom<sd>(to);
			ml.add(move::make_move(from, to, pc, pos.piece(to), prom));
		}
	}
}
template<Side sd>void add_silver_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	for (bit::Bitboard piece = pos.pieces(Silver, sd); piece;) {
		const auto from = piece.lsb();
		for (auto att = piece_attacks<Silver>(sd, from, pos.pieces()) & target; att;) {
			const auto to = att.lsb();
			const auto prom = square_is_prom<sd>(from) || square_is_prom<sd>(to);
			if (prom) {
				ml.add(move::make_move(from, to, Silver, pos.piece(to), true));
			}
			ml.add(move::make_move(from, to, Silver, pos.piece(to), false));
		}
	}
}
template<Piece pc, Side sd>void add_lance_knight_move(List& ml, const Pos& pos, const bit::Bitboard& target) {
	static_assert(pc == Lance || pc == Knight, "pc error");
	for (auto piece = pos.pieces(pc, sd); piece;) {
		const auto from = piece.lsb();
		for (auto att = piece_attacks<pc>(sd, from, pos.pieces())& target; att;) {
			const auto to = att.lsb();
			const auto to_rank = side_rank<sd>(square_rank(to));
			if (to_rank == Rank_1 || to_rank == Rank_2) {
				ml.add(move::make_move(from, to, pc, pos.piece(to), true));
			}
			else if (to_rank == Rank_3) {
				ml.add(move::make_move(from, to, pc, pos.piece(to), true));
				ml.add(move::make_move(from, to, pc, pos.piece(to), false));
			}
			else {
				ml.add(move::make_move(from, to, pc, pos.piece(to), false));
			}
		}
	}
}
template<bool has_knight, bool has_lance, bool has_pawn, Side sd, int num>
void add_drop_move(List& ml, const Pos& pos, const bit::Bitboard& target, const Piece p[]) {
	if (has_pawn) {
		//pawn
		const auto mask = bit::g_rank_mask[Rank_1]
			| bit::g_rank_mask[Rank_2]
			| bit::g_rank_mask[Rank_3]
			| bit::g_rank_mask[Rank_4]
			| bit::g_rank_mask[Rank_5]
			| bit::g_rank_mask[Rank_6]
			| bit::g_rank_mask[Rank_7]
			| bit::g_rank_mask[Rank_8];
		auto pawn = pos.pieces(Pawn, sd) + mask;
		const auto index = (uint32)(ml::pext(pawn.p<0>(), bit::g_rank_mask[Rank_9].p<0>())
			+ (ml::pext(pawn.p<1>(), bit::g_rank_mask[Rank_9].p<1>()) << 7));
		//std::cout<<"index:"<<index<<std::endl;
		//std::cout<<bit::g_double_pawn_mask[sd][index]<<std::endl;
		auto pawn_target = target & bit::g_double_pawn_mask[sd][index];
		const auto mate_with_sq = (sd == BLACK) ?
			pos.king(flip_turn(sd)) + SQ_RANK_INC :
			pos.king(flip_turn(sd)) - SQ_RANK_INC;
		if (is_valid_sq(mate_with_sq)
			&& pawn_target.is_set(mate_with_sq)
			&& is_mate_with_pawn_drop(mate_with_sq, pos)) {
			pawn_target.clear(mate_with_sq);
		}
		while (pawn_target) {
			ml.add(move::make_move(pawn_target.lsb(), Pawn));
		}

	}

	//rank1
	bit::Bitboard rank_1 = target & bit::g_rank_mask[side_rank<sd>(Rank_1)];
	while (bool(rank_1)) {
		const auto to = rank_1.lsb();
		for (auto i = 0; i < num; i++) {
			ml.add(move::make_move(to, p[i]));
		}
	}

	//rank2
	bit::Bitboard rank_2 = target & bit::g_rank_mask[side_rank<sd>(Rank_2)];
	while (bool(rank_2)) {
		const auto to = rank_2.lsb();
		for (auto i = 0; i < num; i++) {
			ml.add(move::make_move(to, p[i]));
		}
		if (has_lance) {
			ml.add(move::make_move(to, Lance));
		}
	}

	//rank3~rank9
	bit::Bitboard rank_39 = target
		& ~(bit::g_rank_mask[side_rank<sd>(Rank_1)]
			| bit::g_rank_mask[side_rank<sd>(Rank_2)]);
	while (bool(rank_39)) {
		const auto to = rank_39.lsb();
		for (auto i = 0; i < num; i++) {
			ml.add(move::make_move(to, p[i]));
		}
		if (has_lance) {
			ml.add(move::make_move(to, Lance));
		}
		if (has_knight) {
			ml.add(move::make_move(to, Knight));
		}
	}
}
template<Side sd>
void add_drop_move(List& ml, const Pos& pos, const bit::Bitboard& target) {

	const auto hand = pos.hand(sd);
	auto status = 0;
	Piece piece_list[4] = { };
	Piece* p = piece_list;

	(hand_has(hand, Pawn)) ? status += 1 : status;
	(hand_has(hand, Lance)) ? status += 2 : status;
	(hand_has(hand, Knight)) ? status += 4 : status;
	(hand_has(hand, Silver)) ?
		status += 8, (*p++) = Silver : status;
	(hand_has(hand, Gold)) ? status += 8, (*p++) = Gold : status;
	(hand_has(hand, Bishop)) ?
		status += 8, (*p++) = Bishop : status;
	(hand_has(hand, Rook)) ? status += 8, (*p++) = Rook : status;

#define ADD_DROP_MOVE(n) {\
  constexpr auto knight_flag = (n & (1 << 2)) != 0;\
  constexpr auto lance_flag  = (n & (1 << 1)) != 0;\
  constexpr auto pawn_flag   = (n & (1 << 0)) != 0;\
  constexpr auto num = n >> 3;\
  add_drop_move<knight_flag,lance_flag,pawn_flag,sd,num>(ml,pos,target,piece_list); \
  break;\
}
	switch (status) {
	case 0:
		break;
	case 1:
		ADD_DROP_MOVE(1)
	case 2:
		ADD_DROP_MOVE(2)
	case 3:
		ADD_DROP_MOVE(3)
	case 4:
		ADD_DROP_MOVE(4)
	case 5:
		ADD_DROP_MOVE(5)
	case 6:
		ADD_DROP_MOVE(6)
	case 7:
		ADD_DROP_MOVE(7)
	case 8:
		ADD_DROP_MOVE(8)
	case 9:
		ADD_DROP_MOVE(9)
	case 10:
		ADD_DROP_MOVE(10)
	case 11:
		ADD_DROP_MOVE(11)
	case 12:
		ADD_DROP_MOVE(12)
	case 13:
		ADD_DROP_MOVE(13)
	case 14:
		ADD_DROP_MOVE(14)
	case 15:
		ADD_DROP_MOVE(15)
	case 16:
		ADD_DROP_MOVE(16)
	case 17:
		ADD_DROP_MOVE(17)
	case 18:
		ADD_DROP_MOVE(18)
	case 19:
		ADD_DROP_MOVE(19)
	case 20:
		ADD_DROP_MOVE(20)
	case 21:
		ADD_DROP_MOVE(21)
	case 22:
		ADD_DROP_MOVE(22)
	case 23:
		ADD_DROP_MOVE(23)
	case 24:
		ADD_DROP_MOVE(24)
	case 25:
		ADD_DROP_MOVE(25)
	case 26:
		ADD_DROP_MOVE(26)
	case 27:
		ADD_DROP_MOVE(27)
	case 28:
		ADD_DROP_MOVE(28)
	case 29:
		ADD_DROP_MOVE(29)
	case 30:
		ADD_DROP_MOVE(30)
	case 31:
		ADD_DROP_MOVE(31)
	case 32:
		ADD_DROP_MOVE(32)
	case 33:
		ADD_DROP_MOVE(33)
	case 34:
		ADD_DROP_MOVE(34)
	case 35:
		ADD_DROP_MOVE(35)
	case 36:
		ADD_DROP_MOVE(36)
	case 37:
		ADD_DROP_MOVE(37)
	case 38:
		ADD_DROP_MOVE(38)
	case 39:
		ADD_DROP_MOVE(39)
	case 40:
		ADD_DROP_MOVE(40)
	default:
		assert(false);
	}
#undef ADD_DROP_MOVE
}

template<Side sd>void add_direct_check(List& list, const Pos& pos) {
	const auto xd = flip_turn(sd);
	const auto king = pos.king(xd);
	const auto occ = pos.pieces();
	const auto global_target = ~pos.pieces(sd);
	{
		auto target = ((get_gold_attack(xd,king) & g_prom[sd] ) | (get_pawn_attack(xd,king) & g_middle[sd])) & global_target;
		add_pawn_move<sd>(list, pos, target);
	} {
		const auto target1 = get_gold_attack(xd,king) & g_prom[sd]  & global_target;
		const auto rank3_bb = (sd == BLACK) ? g_rank_mask[Rank_3] : g_rank_mask[Rank_7];
		const auto target2 = get_lance_attack(xd,king,occ) & (g_middle[sd] | rank3_bb) & global_target;
		for (auto piece = pos.pieces(Lance, sd); piece;) {
			const auto from = piece.lsb();
			const auto attacks = get_lance_attack(sd, from, pos.pieces());
			for (auto att = attacks & target1; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Lance, pos.piece(to), true));
			}
			for (auto att = attacks & target2; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Lance, pos.piece(to), false));
			}
		}
	} {
		auto target1 = get_gold_attack(xd,king) & g_prom[sd]  & global_target;
		const auto rank3_bb = (sd == BLACK) ? g_rank_mask[Rank_3] : g_rank_mask[Rank_7];
		auto target2 = get_knight_attack(xd,king) & (g_middle[sd] | rank3_bb) & global_target;
		for (auto piece = pos.pieces(Knight, sd); piece;) {
			const auto from = piece.lsb();
			const auto attacks = get_knight_attack(sd, from);
			for (auto att = attacks & target1; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Knight, pos.piece(to), true));
			}
			for (auto att = attacks & target2; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Knight, pos.piece(to), false));
			}
		}
	}{
		auto target1 = get_gold_attack(xd,king) & global_target;
		auto target2 = get_silver_attack(xd,king) & global_target;
		const auto rank4_bb = (sd == BLACK) ? g_rank_mask[Rank_4] : g_rank_mask[Rank_6];

		//prom
		auto piece = pos.pieces(Silver, sd) & (g_prom[sd] | rank4_bb);
		while (piece) {
			const auto from = piece.lsb();
			const auto attacks = get_silver_attack(sd, from);
			for (auto att = attacks & target1; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Silver, pos.piece(to), true));
			}
		}
		//noprom
		piece = pos.pieces(Silver, sd);
		while (piece) {
			const auto from = piece.lsb();
			const auto attacks = get_silver_attack(sd, from);
			for (auto att = attacks & target2; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Silver, pos.piece(to), false));
			}
		}
	} {
		auto target = get_gold_attack(xd,king) & global_target;
		add_noprom_move<Golds, sd>(list, pos, target);
	} {
		//add_king_move<sd>(list, pos, target);
	} {
		auto bishop_bb = get_bishop_attack(king,occ);
		auto target_bishop = bishop_bb & g_middle[sd] & global_target;
		auto target_pbishop = (bishop_bb | get_king_attack(king)) & global_target;
		//prom
		auto piece = pos.pieces(Bishop,sd) & g_prom[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_bishop_attack(from, pos.pieces()) & target_pbishop; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Bishop, pos.piece(to),true));
			}
		}
		//prom
		target_pbishop &= g_prom[sd]; 
		piece = pos.pieces(Bishop,sd) & g_middle[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_bishop_attack(from, pos.pieces()) & target_pbishop; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Bishop, pos.piece(to),true));
			}
		}

		//noprom
		piece = pos.pieces(Bishop,sd) & g_middle[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_bishop_attack(from, pos.pieces()) & target_bishop; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Bishop, pos.piece(to)));
			}
		}
	} {
		auto rook_bb = get_rook_attack(king,occ);
		auto target_rook = rook_bb & g_middle[sd] & global_target;
		auto target_prook = (rook_bb | get_king_attack(king)) & global_target;
		//prom
		auto piece = pos.pieces(Rook,sd) & g_prom[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_rook_attack(from, pos.pieces()) & target_prook; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Rook, pos.piece(to),true));
			}
		}
		//prom
		target_prook &= g_prom[sd]; 
		piece = pos.pieces(Rook,sd) & g_middle[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_rook_attack(from, pos.pieces()) & target_prook; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Rook, pos.piece(to),true));
			}
		}
		//noprom
		piece = pos.pieces(Rook,sd) & g_middle[sd];
		while (piece) {
			const auto from = piece.lsb();
			for (auto att = get_rook_attack(from, pos.pieces()) & target_rook; att;) {
				const auto to = att.lsb();
				list.add(move::make_move(from, to, Rook, pos.piece(to)));
			}
		}
	} {
		auto target = get_pbishop_attack(king,occ) & global_target;
		add_noprom_move<PBishop, sd>(list, pos, target);
	} {
		auto target = get_prook_attack(king,occ) & global_target;
		add_noprom_move<PRook, sd>(list, pos, target);
	}
}

template<Side sd>void add_discover_check(List& list, const Pos& pos) {
	const auto xd = flip_turn(sd);
	auto pins_bb = pins(pos,xd) & pos.pieces(sd);
	const auto king = pos.king(xd);
	const auto occ = pos.pieces();
	const auto target = ~pos.pieces(sd);

	while(pins_bb) {
		const auto from = pins_bb.lsb();
		const auto pc = pos.piece(from);
		const auto ray = ~(between(king, from) | beyond(king, from));
		switch (pc) {
			case Pawn: {
				const auto to = (sd == BLACK) ? from + Inc_N : from + Inc_S;
				if(is_valid_sq(to) && ray.is_set(to)) {
					if (square_is_prom<sd>(to)) {
						const auto direct_bb = get_gold_attack(xd,king) & target;
						if(!direct_bb.is_set(to)) {
							list.add(move::make_move(from, to, Pawn, pos.piece(to), true));
						}
					} else {
						const auto direct_sq = (sd == BLACK) ? from + Inc_S : from + Inc_N;
						if(direct_sq != to && is_valid_sq(direct_sq) && target.is_set(to)) {
							list.add(move::make_move(from, to, Pawn, pos.piece(to), false));
						}
					}
				}
				break;
			}
			case Lance: {
				auto direct_bb = ~(get_lance_attack(xd,king,occ));
				const auto rank3_bb = (sd == BLACK) ? g_rank_mask[Rank_3] : g_rank_mask[Rank_7];
				const auto lance_attack = get_lance_attack(sd,from,occ);
				auto attack_bb = lance_attack & (g_middle[sd] | rank3_bb) & target & ray & direct_bb;
				//noprom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Lance, pos.piece(to)));
				}
				direct_bb = ~(get_gold_attack(xd,king) & g_prom[sd]);
				attack_bb = lance_attack & g_prom[sd] & target & ray & direct_bb;
				//prom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Lance, pos.piece(to),true));
				}
				break;
			}
			case Knight: {
				auto direct_bb = ~(get_knight_attack(xd,king));
				const auto rank3_bb = (sd == BLACK) ? g_rank_mask[Rank_3] : g_rank_mask[Rank_7];
				const auto knight_attack = get_knight_attack(sd,from);
				auto attack_bb = knight_attack & (g_middle[sd] | rank3_bb) & target & ray & direct_bb;
				//noprom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Knight, pos.piece(to)));
				}
				direct_bb = ~(get_gold_attack(xd,king) & g_prom[sd]);
				attack_bb = knight_attack & g_prom[sd] & target & ray & direct_bb;
				//prom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Knight, pos.piece(to),true));
				}
				break;
			}
			case Silver: {
				auto direct_bb = ~(get_silver_attack(xd,king));
				auto attack_bb = get_silver_attack(sd,from) & target & ray & direct_bb;
				//noprom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Silver, pos.piece(to)));
				}
				const auto rank4_bb = (sd == BLACK) ? g_rank_mask[Rank_4] : g_rank_mask[Rank_6];
				direct_bb = ~(get_gold_attack(xd,king) & (g_prom[sd] | rank4_bb));
				attack_bb = get_silver_attack(sd,from) & (g_prom[sd] | rank4_bb) & target & ray & direct_bb;
				//prom
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, Silver, pos.piece(to),true));
				}
				break;
			}
			case Gold: 
			case PPawn:
			case PLance:
			case PKnight:
			case PSilver: {
				const auto direct_bb = ~get_gold_attack(xd,king);
				auto attack_bb = get_gold_attack(sd,from) & target & ray & direct_bb;
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, pos.piece(from), pos.piece(to)));
				}
				break;
			}
			case Bishop: {
				if (square_is_prom<sd>(from)) {
					auto direct_bb = ~(get_pbishop_attack(king,occ));
					auto attack_bb = get_bishop_attack(from,occ) & target & ray & direct_bb;
					//prom
					while(attack_bb) {
						const auto to = attack_bb.lsb();
						list.add(move::make_move(from, to, Bishop, pos.piece(to),true));
					}
				} else {
					auto direct_bb = ~(get_bishop_attack(king,occ) | (get_king_attack(king) & g_prom[sd]));
					auto attack_bb = get_bishop_attack(from,occ) & target & ray & direct_bb;
					while(attack_bb) {
						const auto to = attack_bb.lsb();
						list.add(move::make_move(from, to, Bishop, pos.piece(to),square_is_prom<sd>(to)));
					}
				}
				break;
			}
			case Rook: {
				if (square_is_prom<sd>(from)) {
					auto direct_bb = ~(get_prook_attack(king,occ));
					auto attack_bb = get_rook_attack(from,occ) & target & ray & direct_bb;
					//prom
					while(attack_bb) {
						const auto to = attack_bb.lsb();
						list.add(move::make_move(from, to, Rook, pos.piece(to),true));
					}
				} else {
					auto direct_bb = ~(get_rook_attack(king,occ) | (get_king_attack(king) & g_prom[sd]));
					auto attack_bb = get_rook_attack(from,occ) & target & ray & direct_bb;
					while(attack_bb) {
						const auto to = attack_bb.lsb();
						list.add(move::make_move(from, to, Rook, pos.piece(to),square_is_prom<sd>(to)));
					}
				}
				break;
			}
			case King: {
				const auto direct_bb = ~get_king_attack(king);
				auto attack_bb = get_king_attack(from) & target & ray & direct_bb;
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, King, pos.piece(to)));
				}
				break;
			}
			case PBishop: {
				const auto direct_bb = ~get_pbishop_attack(king,occ);
				auto attack_bb = get_pbishop_attack(from,occ) & target & ray & direct_bb;
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, PBishop, pos.piece(to)));
				}
				break;
			}
			case PRook: {
				const auto direct_bb = ~get_prook_attack(king,occ);
				auto attack_bb = get_prook_attack(from,occ) & target & ray & direct_bb;
				while(attack_bb) {
					const auto to = attack_bb.lsb();
					list.add(move::make_move(from, to, PRook, pos.piece(to)));
				}
				break;
			}
			default:
				assert(false);
				break;
		}
	}
}

template<Side sd>void add_drop_check(List& list, const Pos& pos) {
	const auto hand = pos.hand(sd);
	const auto xd = flip_turn(sd);
	const auto king = pos.king(xd);
	const auto occ = pos.pieces();
	const auto target = ~occ;

#define ADD_CHECK_MOVE(Piece) do { \
	if(hand_has(hand,Piece)) {\
		auto tmp_target = piece_attacks<Piece>(xd,king,occ) & target;\
		while(tmp_target) {\
			const auto to = tmp_target.lsb();\
			list.add(move::make_move(to, Piece));\
		}\
	} }while(0)

	ADD_CHECK_MOVE(Gold);
	ADD_CHECK_MOVE(Silver);
	ADD_CHECK_MOVE(Rook);
	ADD_CHECK_MOVE(Bishop);
	ADD_CHECK_MOVE(Lance);
	ADD_CHECK_MOVE(Knight);
	//pawn
	if(hand_has(hand,Pawn)) {
		const auto to = (sd == BLACK) ? king + Inc_S : king + Inc_N;
		if(is_valid_sq(to) && pos.piece(to) == PieceNone) {
			//check double pawn and mwpd
			if(!(get_pseudo_file_attack(king) & pos.pieces(Pawn,sd)) 
			&& !is_mate_with_pawn_drop(to,pos)) {
				list.add(move::make_move(to, Pawn));
			}
		}
	}
#undef ADD_CHECK_MOVE

}

template<Side sd>void gen_legals(List& list, const Pos& pos) {
	List tmp;
	gen_moves<sd>(tmp, pos);
	list.clear();
	for (auto i = 0; i < tmp.size(); i++) {
		const auto mv = tmp[i];
		if (move::pseudo_is_legal(mv, pos)) {
			list.add(mv);
		}
	}
}
void gen_legals(List& list, const Pos& pos) {
	(pos.turn() == BLACK) ? gen_legals<BLACK>(list, pos)
		: gen_legals<WHITE>(list, pos);
}
template<MoveType mt, Side sd>
void gen_moves(List& ml, const Pos& pos, const bit::Bitboard* checks) {

	constexpr auto xd = flip_turn(sd);
	bit::Bitboard target, target2;
	switch (mt) {
	case TACTICAL:
		target = pos.pieces(xd);
		target2 = target | (~pos.pieces(sd) & bit::g_prom[sd]);
		break;
	case QUIET:
		target = ~pos.pieces();
		target2 = target & bit::g_middle[sd];
		break;
	case DROP:
		target = ~pos.pieces();
		break;
	case CHECK:
		target = ~pos.pieces(sd);
		target2 = ~pos.pieces();
		break;
	case EVASION: {
		//取れるなら取ってしまったほうがいい気がする
		target = ~pos.pieces(sd);
		add_king_move<sd>(ml, pos, target);
		assert(checks != nullptr);
		if (checks->pop_cnt() == 2) {
			return;
		}
		assert(checks->pop_cnt() == 1);
		auto check_bb = *checks;
		const auto checker_sq = check_bb.lsb<false>();
		target = target2 = between(checker_sq, pos.king(sd));
		target.set(checker_sq);//capture checker
		add_pawn_move<sd>(ml, pos, target);
		add_lance_knight_move<Lance, sd>(ml, pos, target);
		add_lance_knight_move<Knight, sd>(ml, pos, target);
		add_silver_move<sd>(ml, pos, target);
		add_noprom_move<Golds, sd>(ml, pos, target);
		add_bishop_rook_move<Bishop, sd>(ml, pos, target);
		add_bishop_rook_move<Rook, sd>(ml, pos, target);
		add_noprom_move<PBishop, sd>(ml, pos, target);
		add_noprom_move<PRook, sd>(ml, pos, target);

		add_drop_move<sd>(ml, pos, target2);
		return;
	}
	default:
		assert(false);
		break;
	}
	switch (mt) {
	case DROP:
		add_drop_move<sd>(ml, pos, target);
		break;
	case CHECK:
		add_direct_check<sd>(ml, pos);
		add_discover_check<sd>(ml, pos);
		add_drop_check<sd>(ml, pos);
		break;
	default:
		add_pawn_move<sd>(ml, pos, target2);
		add_lance_knight_move<Lance, sd>(ml, pos, target);
		add_lance_knight_move<Knight, sd>(ml, pos, target);
		add_silver_move<sd>(ml, pos, target);
		add_noprom_move<Golds, sd>(ml, pos, target);
		add_king_move<sd>(ml, pos, target);
		add_bishop_rook_move<Bishop, sd>(ml, pos, target);
		add_bishop_rook_move<Rook, sd>(ml, pos, target);
		add_noprom_move<PBishop, sd>(ml, pos, target);
		add_noprom_move<PRook, sd>(ml, pos, target);
		break;
	}
}
template<Side sd>void gen_moves(List& list, const Pos& pos) {
	const auto chk = checks(pos);
	gen_moves<sd>(list, pos, &chk);
}
template<Side sd>void gen_moves(List& list, const Pos& pos, const bit::Bitboard* checks) {
	if (!(*checks)) {
		gen_moves<TACTICAL, sd>(list, pos, checks);
		gen_moves<QUIET, sd>(list, pos, checks);
		gen_moves<DROP, sd>(list, pos, checks);
	}
	else {
		gen_moves<EVASION, sd>(list, pos, checks);
	}
}
void gen_moves(List& list, const Pos& pos) {
	(pos.turn() == BLACK) ? gen_moves<BLACK>(list, pos) : gen_moves<WHITE>(list, pos);
}

template void gen_moves<BLACK>(List& ml, const Pos& pos);
template void gen_moves<WHITE>(List& ml, const Pos& pos);

template void gen_moves<BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

template void gen_moves<TACTICAL, BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<TACTICAL, WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

template void gen_moves<QUIET, BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<QUIET, WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

template void gen_moves<DROP, BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<DROP, WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

template void gen_moves<EVASION, BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<EVASION, WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

template void gen_moves<CHECK, BLACK>(List& ml, const Pos& pos, const bit::Bitboard* checks);
template void gen_moves<CHECK, WHITE>(List& ml, const Pos& pos, const bit::Bitboard* checks);

namespace gen {

	void test() {
		{
			Pos pos = pos_from_sfen("l+N4Bn1/3+N1kS+P1/p2sgsp+Lp/1p3pP2/3pP1R1G/P1PPbP1s1/KP6P/L3rg3/7NL w 2PGp");
			Tee << pos << std::endl;
			List list;
			gen_legals<WHITE>(list, pos);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen(START_SFEN);
			Tee << pos << std::endl;
			List list;
			gen_moves<BLACK>(list, pos);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/1PPPPPP1P/1B5R1/LNSGKGSNL b 2P 1");
			Tee << pos << std::endl;
			List list;
			gen_moves<BLACK>(list, pos);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("k8/4l4/9/9/9/4K4/9/9/9 b 10PLKSGBR 1");
			Tee << pos << std::endl;
			List list;
			gen_moves<BLACK>(list, pos);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("lnsgkgsnl/1r7/pppppp1pp/6p2/9/2P6/PP+bPPPPPP/1B3K1R1/L1SG1GSNL w n");
			Tee << pos << std::endl;
			List list;
			gen_moves<WHITE>(list, pos);
			Tee << list << std::endl;

		}
		{
			Pos pos = pos_from_sfen("l6nl/5+P3/3p1S1gk/p1p5P/3n2Sp1/1PPb2P2/P5GS1/R8/LN4bKL w PRG7pnsg");
			Tee << pos << std::endl;
			List list;
			gen_moves<WHITE>(list, pos);
			Tee << list << std::endl;

		}
		{
			Pos pos = pos_from_sfen("4k4/4l4/9/9/9/9/4B4/9/r3K4 b ");
			Tee << pos << std::endl;
			List list;
			gen_legals<BLACK>(list,pos);
			Tee << list << std::endl;

		}
		{
			Pos pos = pos_from_sfen("4k4/9/2L6/9/B8/9/9/9/4K4 b ");
			Tee << pos << std::endl;
			List list;
			gen_legals<BLACK>(list,pos);
			for(auto i = 0; i < list.size(); i++) {
				Tee<<move::move_to_string(list.move(i))<<std::endl;
				Tee<<move::is_check(list.move(i),pos)<<std::endl;
			}

		}
		{
			Pos pos = pos_from_sfen("1nk6/2g1p3p/1rs1g3l/lppp1+B2P/P1P2PpPb/p1N1PrPp1/NP1P1G2L/L1S2K1+p1/1+s2G1SN w ");
			Tee << pos << std::endl;
			List list;
			gen_moves<CHECK,WHITE>(list,pos,nullptr);
			for(auto i = 0; i < list.size(); i++) {
				Tee<<move::move_to_string(list.move(i))<<std::endl;
			}

		}
	}

}

