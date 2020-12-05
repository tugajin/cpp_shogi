#include <string>
#include "move.hpp"
#include "pos.hpp"
#include "attack.hpp"

namespace move {

	bool pseudo_is_legal(const Move mv, const Pos& pos) {
	
#ifdef DEBUG
		if (!move_is_ok(mv, pos)) {
			Tee << pos << std::endl;
			Tee << move::move_to_string(mv) << std::endl;
			Tee << move::move_to_string(pos.last_move()) << std::endl;
			//Tee << uint64(pos.parent_->pos_key()) << std::endl;
			//Tee << uint64(pos.parent_->hand_key()) << std::endl;

			assert(false);
		}
#endif
		assert(move_is_ok(mv, pos));

		if (move::move_is_drop(mv)) {
			return true;
		}
		
		const auto from = move::move_from(mv);
		const auto to = move::move_to(mv);
		const auto pc = move::move_piece(mv);
		const auto sd = pos.turn();
		const auto xd = flip_turn(sd);
		const auto king = pos.king(sd);
		auto pieces = pos.pieces();

		pieces.clear(from);
		pieces.set(to);

		//king move
		if (pc == King) {
			return !has_attack(pos, xd, to, pieces);
		}
	

		//pinned piece
		auto beyond = bit::beyond(king, from);
		auto not_to = g_all_zero_bb;
		not_to.set(to);
		not_to = ~not_to;
		auto b = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(xd) & beyond & not_to;
		while (b) {
			const auto ds = b.lsb();
			if (piece_attack<Bishop>(xd, ds, king, pieces)) {
				return false;
			}
		}

		

		b = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(xd) & beyond & not_to;
		while (b) {
			const auto ds = b.lsb();
			if (piece_attack<Rook>(xd, ds, king, pieces)) {
				return false;
			}
		}

		b = pos.pieces(Lance, xd) & g_lance_mask[sd][king] & not_to;
		
		while (b) {
			const auto ds = b.lsb();
			if (piece_attack<Lance>(xd, ds, king, pieces)) {
				return false;
			}
		}

		return true;

	}

	bool pseudo_is_legal_debug(const Move mv, const Pos& pos) {
		auto new_pos = pos.succ(mv);
		return is_legal(new_pos);
	}

	bool is_check(const Move mv, const Pos& pos) {

		assert(move_is_ok(mv, pos));

		const auto sd = pos.turn();
		const auto king = pos.king(flip_turn(sd));
		const auto to = move_to(mv);
		auto pieces = pos.pieces();
		pieces.set(to);

		auto pc = move_piece(mv);
		if (move_is_drop(mv)) {
			//direct check
			return piece_attack(pc, sd, to, king, pieces);
		}
		else {
			const auto from = move_from(mv);
			pieces.clear(from);
			if (move_is_prom(mv)) {
				pc = piece_prom(pc);
			}
			//direct check
			if (piece_attack(pc, sd, to, king, pieces)) {
				return true;
			}

			//discover check
			auto beyond = bit::beyond(king, from);
			auto b = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(sd) & beyond;
			while (b) {
				const auto ds = b.lsb();
				if (piece_attack<Bishop>(sd, ds, king, pieces)) {
					return true;
				}
			}
			b = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(sd) & beyond;
			while (b) {
				const auto ds = b.lsb();
				if (piece_attack<Rook>(sd, ds, king, pieces)) {
					return true;
				}
			}
			b = pos.pieces(Lance, sd) & beyond;

			while (b) {
				const auto ds = b.lsb();
				if (piece_attack<Lance>(sd, ds, king, pieces)) {
					return true;
				}
			}
		}
		return false;

	}

	Move from_usi(const std::string& s, const Pos& pos) {
		if (std::isdigit(s[0])) { //move
			const auto from = sq_from_string(s.substr(0, 2));
			const auto to = sq_from_string(s.substr(2, 2));
			const auto pc = pos.piece(from);
			const auto cp = pos.piece(to);
			const auto prom = s.find("+") != std::string::npos;
			return move::make_move(from, to, pc, cp, prom);
		}
		else { //drop
			const auto pc = sfen_to_piece(s.substr(0, 1));
			const auto to = sq_from_string(s.substr(2, 2));
			return move::make_move(to, pc);
		}
	}

	std::string move_to_usi(const Move mv) {
		if (mv == MOVE_NONE || mv == MOVE_NULL) {
			return "0000";
		}
		std::string s = "";
		if (move_is_drop(mv)) {
			s += ml::trim(piece_to_sfen(move_piece(mv)));
			s += "*";
			s += sq_to_string(move_to(mv));
		}
		else {
			s += sq_to_string(move_from(mv));
			s += sq_to_string(move_to(mv));
			if (move_is_prom(mv)) {
				s += "+";
			}
		}
		return s;
	}

	std::string move_to_string(const Move mv) {
		if (mv == MOVE_NONE) {
			return "0000";
		}
		if (mv == MOVE_NULL) {
			return "NULL";
		}
		std::string s = "";
		if (move_is_drop(mv)) {
			s += "drop:";
			s += sq_to_string(move_to(mv));
			s += " piece:";
			s += ml::trim(piece_to_sfen(move_piece(mv)));

		}
		else {
			s += "from:";
			s += sq_to_string(move_from(mv));
			s += " to:";
			s += sq_to_string(move_to(mv));
			s += " piece:";
			s += ml::trim(piece_to_sfen(move_piece(mv)));
			s += " cap:";
			s += ml::trim(piece_to_sfen(move_cap(mv)));
			if (move_is_prom(mv)) {
				s += "+";
			}
		}
		return s;
	}

	bool move_is_ok(const Move mv) {
		if (mv == MOVE_NONE) {
			return false;
		}
		if (mv == MOVE_NULL) {
			return false;
		}
		const auto from = move_from(mv);
		const auto to = move_to(mv);
		const auto piece = move_piece(mv);
		const auto cap = move_cap(mv);
		const auto prom = move_is_prom(mv);
		const auto is_drop = move_is_drop(mv);
		if (from == SQUARE_SIZE) {
			if (!is_drop) {
				return false;
			}
		}
		else {
			if (!is_valid_sq(from)) {
				return false;
			}
		}
		if (!is_valid_sq(to)) {
			return false;
		}
		if (!piece_is_ok(piece)) {
			return false;
		}
		if (cap != PieceNone) {
			if (!piece_is_ok(cap)) {
				return false;
			}
		}
		if (prom) {
			if (piece != Pawn &&
				piece != Knight &&
				piece != Lance &&
				piece != Silver &&
				piece != Bishop &&
				piece != Rook) {
				return false;
			}
		}
		return true;
	}
	bool move_is_ok(const Move mv, const Pos& pos) {
		if (!move_is_ok(mv)) {
			return false;
		}

		const auto from = move_from(mv);
		const auto to = move_to(mv);
		const auto piece = move_piece(mv);
		const auto cap = move_cap(mv);
		const auto prom = move_is_prom(mv);
		const auto sd = pos.turn();

		if (move_is_drop(mv)) {
			if (!hand_has(pos.hand(sd), piece)) {
				return false;
			}
			if (pos.piece(to) != PieceNone) {
				return false;
			}
		}
		else {
			if (pos.piece(from) != piece) {
				return false;
			}
			const auto  side_bb = pos.pieces(sd);
			if (!side_bb.is_set(from)) {
				return false;
			}
			if (side_bb.is_set(to)) {
				return false;
			}
		}
		if (cap == King) {
			return false;
		}
		if (pos.turn() == BLACK) {
			if (prom) {
				if (square_is_prom<BLACK>(from) || square_is_prom<BLACK>(to)) {
				}
				else {
					return false;
				}
			}
		}
		else {
			if (prom) {
				if (square_is_prom<WHITE>(from) || square_is_prom<WHITE>(to)) {
				}
				else {
					return false;
				}
			}
		}

		return true;
	}

}