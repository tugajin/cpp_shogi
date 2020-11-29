import sys
import numpy as np
from enum import Enum

PIECE_NUM = 16
SIDE_PIECE_NUM = 32
SQUARE_SIZE = 81
SFEN_SIDE_PIECE =  " . P L N S B R G K+P+L+N+S+B+R+G+. p l n s b r g k+p+l+n+s+b+r+g+k"

SFEN_FILE = "987654321"
SFEN_RANK = "abcdefghi"

class Direction(Enum):
     DIR_UP = 0
     DIR_UL = 1
     DIR_LF = 2
     DIR_DL = 3
     DIR_DW = 4
     DIR_DR = 5
     DIR_RG = 6
     DIR_UR = 7
     DIR_L_KNT = 8
     DIR_R_KNT = 9
     DIR_NONE = 10

def sq_to_file(sq):
    return int(sq / 9)   

def sq_to_rank(sq):
    return int(sq % 9)

def flip_sq(sq) :
    return SQUARE_SIZE - sq - 1

def get_direction(move_from, move_to):
    
    move_from_file = sq_to_file(move_from)
    move_from_rank = sq_to_rank(move_from)
    
    move_to_file = sq_to_file(move_to)
    move_to_rank = sq_to_rank(move_to)

    if False:
        pass
    elif move_from_file == move_to_file and move_from_rank == move_to_rank:
        return DIR_NONE
    elif move_from_file == move_to_file and move_from_rank > move_to_rank:
        return Direction.DIR_DW
    elif move_from_file == move_to_file and move_from_rank < move_to_rank: 
        return Direction.DIR_UP
    elif move_from_file > move_to_file and move_from_rank == move_to_rank:
        return Direction.DIR_RG
    elif move_from_file > move_to_file and move_from_rank > move_to_rank:
        return Direction.DIR_DR
    elif move_from_file > move_to_file and move_from_rank < move_to_rank:
        return Direction.DIR_UR if (move_to_rank - move_from_rank) == 1 else Direction.DIR_R_KNT
    elif move_from_file < move_to_file and move_from_rank == move_to_rank:
        return Direction.DIR_LF
    elif move_from_file < move_to_file and move_from_rank > move_to_rank:
        return Direction.DIR_DL
    elif move_from_file < move_to_file and move_from_rank < move_to_rank:
        return Direction.DIR_UL if (move_to_rank - move_from_rank) == 1 else Direction.DIR_L_KNT

class Side(Enum):
    BLACK = 0
    WHITE = 1
    SIDE_NUM = 2

class Piece(Enum):
    EMPTY = 0
    PAWN = 1
    LANCE = 2
    KNIGHT = 3
    SILVER = 4
    BISHOP = 5
    ROOK = 6
    GOLD = 7
    KING = 8
    PPAWN = 9
    PLANCE = 10
    PKNIGHT = 11
    PSILVER = 12
    PBISHOP = 13
    PROOK = 14
    
    @staticmethod
    def from_sfen(s):
        """
        文字列から駒(side_piece)に変換する
        """
        piece = SFEN_SIDE_PIECE.find(s)
        if piece == -1:
            return Piece.EMPTY
        else:
            sp = SidePiece(int(piece/2))
            return sp.to_piece()

    def to_sfen(self):
        """
        pieceから文字列に変換する
        """
        p = self.value
        piece = SFEN_SIDE_PIECE[(2*p):(2*p)+2]
        return  piece
    
    def prom(self):
        p = Piece(self.value + 8)
        return p

    def unprom(self):
        p = Piece(self.value - 8)
        return p

class SidePiece(Enum):
    EMPTY = 0
    BPAWN = 1
    BLANCE = 2
    BKNIGHT = 3
    BSILVER = 4
    BBISHOP = 5
    BROOK = 6
    BGOLD = 7
    BKING = 8
    BPPAWN = 9
    BPLANCE = 10
    BPKNIGHT = 11
    BPSILVER = 12
    BPBISHOP = 13
    BPROOK = 14

    WPAWN = 17
    WLANCE = 18
    WKNIGHT = 19
    WSILVER = 20
    WBISHOP = 21
    WROOK = 22
    WGOLD = 23
    WKING = 24
    WPPAWN = 25
    WPLANCE = 26
    WPKNIGHT = 27
    WPSILVER = 28
    WPBISHOP = 29
    WPROOK = 30

    def to_piece(self):
        index = self.value & 15
        return Piece(index)
    
    @staticmethod
    def from_sfen(s):
        """
        文字列から駒(side_piece)に変換する
        """
        piece = SFEN_SIDE_PIECE.find(s)
        if piece == -1:
            return SidePiece.EMPTY
        else:
            return SidePiece(int(piece/2))

    def to_sfen(self):
        """
        pieceから文字列に変換する
        """
        p = self.value
        piece = SFEN_SIDE_PIECE[(2*p):(2*p)+2]
        return  piece

    def prom(self):
        p = SidePiece(self.value + 8)
        return p

    def unprom(self):
        p = SidePiece(self.value - 8)
        return p
    def side(self):
        return Side.BLACK if self.value <= SidePiece.BPROOK.value else Side.WHITE 

class ActionList:
    action_list = []
    def init(self,s):
        """
        初期化する。action_listにactionが格納される
        """
        a_list = s.split()
        for sfen_str in a_list:
            action = Action()
            action.load_sfen(sfen_str)
            self.action_list.append(action)

class Action:
    move_from = -1
    move_to = -1
    piece =  Piece.EMPTY
    prom = False
    score = -9999999

    def __init__(self):
        """
        コンストラクタ
        """
        self.clear()
    
    def clear(self):
        """
        初期化
        """
        self.move_from = -1
        self.move_to = -1
        self.piece = Piece.EMPTY
        self.prom = False
        self.score = -9999999

    @staticmethod
    def make_square(file,rank):
        """
        縦(file)、横(rank)からStateクラスのposにアクセスするためのindexに変換する。
        """
        return file + (rank*9)
    
    @staticmethod
    def sfen_to_file(s):
        """
        文字列からfileに変換する。
        """
        return SFEN_FILE.find(s)

    @staticmethod
    def sfen_to_rank(s):
        """
        文字列からrankに変換する。
        """
        return SFEN_RANK.find(s)
    
    def load_sfen(self,s):
        """
        文字列からaction形式に変換する
        """
        self.clear()
        # drop move
        if s[1] == '*':
            to_file = self.sfen_to_file(s[2])
            to_rank = self.sfen_to_rank(s[3])
            self.move_to = self.make_square(to_file,to_rank)
            self.piece =  State.sfen_to_side_piece(s[0])
            self.piece = self.piece.to_piece()

            self.prom = False
        else:
            from_file = self.sfen_to_file(s[0])
            from_rank = self.sfen_to_rank(s[1])
            to_file = self.sfen_to_file(s[2])
            to_rank = self.sfen_to_rank(s[3])

            self.move_from =   self.make_square(from_file,from_rank)
            self.move_to =  self.make_square(to_file,to_rank)
            self.piece = Piece.EMPTY
            self.prom = (len(s.strip()) == 5)

    def out(self):
        """
        actionの中身を表示する。デバッグ用
        """
        s = str(self.move_from) + ":" + str(self.move_to) + ":" + str(self.piece) + ":" + str(self.prom) + ":" + str(self.score)
        return s

    def is_drop(self):
        return self.move_from == -1

    def to_index(self, turn):

        CLS_UP = 0
        CLS_UL = CLS_UP + SQUARE_SIZE
        CLS_LF = CLS_UL + SQUARE_SIZE
        CLS_DL = CLS_LF + SQUARE_SIZE
        CLS_DW = CLS_DL + SQUARE_SIZE
        CLS_DR = CLS_DW + SQUARE_SIZE
        CLS_RG = CLS_DR + SQUARE_SIZE
        CLS_UR = CLS_RG + SQUARE_SIZE
        CLS_L_KNT = CLS_UR + SQUARE_SIZE
        CLS_R_KNT = CLS_L_KNT + SQUARE_SIZE
        CLS_UP_PROM = CLS_R_KNT + SQUARE_SIZE
        CLS_UL_PROM = CLS_UP_PROM + SQUARE_SIZE
        CLS_LF_PROM = CLS_UL_PROM + SQUARE_SIZE
        CLS_DL_PROM = CLS_LF_PROM + SQUARE_SIZE
        CLS_DW_PROM = CLS_DL_PROM + SQUARE_SIZE
        CLS_DR_PROM = CLS_DW_PROM + SQUARE_SIZE
        CLS_RG_PROM = CLS_DR_PROM + SQUARE_SIZE
        CLS_UR_PROM = CLS_RG_PROM + SQUARE_SIZE
        CLS_L_KNT_PROM = CLS_UR_PROM + SQUARE_SIZE
        CLS_R_KNT_PROM = CLS_L_KNT_PROM + SQUARE_SIZE
        CLS_HAND_PAWN = CLS_R_KNT_PROM + SQUARE_SIZE
        CLS_HAND_LANCE = CLS_HAND_PAWN + SQUARE_SIZE
        CLS_HAND_KNIGHT = CLS_HAND_LANCE + SQUARE_SIZE
        CLS_HAND_SILVER = CLS_HAND_KNIGHT + SQUARE_SIZE
        CLS_HAND_GOLD = CLS_HAND_SILVER + SQUARE_SIZE
        CLS_HAND_BISHOP = CLS_HAND_GOLD + SQUARE_SIZE
        CLS_HAND_ROOK = CLS_HAND_BISHOP + SQUARE_SIZE
        CLS_MOVE_END = CLS_HAND_ROOK + SQUARE_SIZE

        PC_DROP_INDEX = { Piece.PAWN: CLS_HAND_PAWN, Piece.LANCE: CLS_HAND_LANCE, Piece.KNIGHT:CLS_HAND_KNIGHT, Piece.SILVER:CLS_HAND_SILVER, 
                          Piece.GOLD:CLS_HAND_GOLD, Piece.BISHOP:CLS_HAND_BISHOP, Piece.ROOK:CLS_HAND_ROOK}
        PC_DIRECTION_PROM_INDEX = {Direction.DIR_UP : CLS_UP_PROM, Direction.DIR_UL : CLS_UL_PROM, Direction.DIR_LF : CLS_LF_PROM, Direction.DIR_DL : CLS_DL_PROM, 
                                    Direction.DIR_DW : CLS_DW_PROM, Direction.DIR_DR : CLS_DR_PROM, Direction.DIR_RG : CLS_RG_PROM, Direction.DIR_UR : CLS_UR_PROM, 
                                    Direction.DIR_L_KNT : CLS_L_KNT_PROM, Direction.DIR_R_KNT : CLS_R_KNT_PROM}
        PC_DIRECTION_INDEX      = {Direction.DIR_UP : CLS_UP, Direction.DIR_UL : CLS_UL, Direction.DIR_LF : CLS_LF, Direction.DIR_DL : CLS_DL, Direction.DIR_DW : CLS_DW, 
                                    Direction.DIR_DR : CLS_DR, Direction.DIR_RG : CLS_RG, Direction.DIR_UR : CLS_UR, Direction.DIR_L_KNT : CLS_L_KNT, Direction.DIR_R_KNT : CLS_R_KNT}
        my_from = self.move_from
        my_to = self.move_to
        my_piece = self.piece
        my_prom = self.prom

        if turn == Side.WHITE:
            my_from = flip_sq(my_from)
            my_to = flip_sq(my_to)

        index = 0

        if self.is_drop():
            if my_piece in PC_DROP_INDEX:
                index = PC_DROP_INDEX[my_piece] + my_to
            else:
                print("index error!")
        else:
            direction = get_direction(my_from, my_to)
            if my_prom:
                if direction in PC_DIRECTION_PROM_INDEX:
                    index = PC_DIRECTION_PROM_INDEX[direction] + my_to
                else:
                    print("index error2!")
            else:
                if direction in PC_DIRECTION_INDEX:
                    index = PC_DIRECTION_INDEX[direction] + my_to
                else:
                    print("index error3!")
        return index

class State:

    def __init__(self):
        """
        コンストラクタ
        """
        self.clear()

    def clear(self):
        """
        初期化
        """
        self.pos =  [SidePiece.EMPTY] * SQUARE_SIZE
        self.turn = Side.BLACK
        self.hand = [0] * SIDE_PIECE_NUM

    def load_sfen(self, sfen_string):
        """
        文字列からstateに変換します。
        """
        self.clear()
        sfen_list = list(sfen_string)
        sfen_len = len(sfen_list)
        list_sp = 0
        pos_sp = 0
        prom_flag = False
        #load pos
        while True:
            s = sfen_list[list_sp]
            list_sp += 1
            if list_sp >= sfen_len:
                break
            elif s.isdigit():
                pos_sp += int(s)
            elif s == ' ':
                pos_sp += 1
                break
            elif s == '+':
                prom_flag = True
            elif s == '/':
                continue
            else:
                piece = SidePiece.from_sfen(s)
                if prom_flag:
                    piece = piece.prom()
                self.pos[pos_sp] =  piece
                pos_sp += 1
                prom_flag = False
        #load turn
        if sfen_list[list_sp] == 'b':
            self.turn = Side.BLACK
        elif sfen_list[list_sp] == 'w':
            self.turn = Side.WHITE
        list_sp += 1
        #load hand
        num = 0
        while True:
            s = sfen_list[list_sp]
            list_sp += 1
            if list_sp >= sfen_len:
                break
            elif s == ' ':
                continue
            elif s == '-':
                continue
            elif s.isdigit():
                num = (num * 10) + int(s)
            else:
                sp = SidePiece.from_sfen(s)
                if num == 0:
                    num = 1
                self.hand[sp.value] = num
                num = 0

    def out(self):
        """
        stateを文字列に変換します（デバッグ用）。
        """
        s = ""
        if self.turn == Side.BLACK:
            s += "BLACK"
        elif self.turn == Side.WHITE:
            s += "WHITE"
        s += "\n"
        for p in range(SidePiece.WPAWN.value, SidePiece.WGOLD.value+1):
            sp = SidePiece(p)
            s +=  sp.to_sfen().strip() + ":" +str(self.hand[sp.value]) + " "
        s += "\n"
        for index,p in enumerate(self.pos):
            s +=  p.to_sfen()
            if (index + 1) % 9 == 0:
                s += "\n"
        for p in range(SidePiece.BPAWN.value,SidePiece.BGOLD.value+1):
            sp = SidePiece(p)
            s +=  sp.to_sfen().strip() + ":" +str(self.hand[sp.value]) + " "
        s += "\n"
        return s

    def out_sfen(self):
        """
        stateを文字列(sfen形式)に変換します。
        """
        s = ""
        num = 0
        pos_num = 0
        for index,p in enumerate(self.pos):
 
            if p == SidePiece.EMPTY:
                num += 1
                pos_num += 1
            else:
                sfen = p.to_sfen()
                if num != 0:
                    s += str(num)
                s += sfen.strip()
                pos_num += 1
                num = 0
            if (pos_num) % 9 == 0:
                if num != 0:
                    s += str(num)
                s += '/'
                num = 0
        if s[-1] == '/':
            s = s[:-1]
        s += ' '
        
        if self.turn == Side.BLACK:
            s += "b "
        elif self.turn == Side.WHITE:
            s += "w "
        
        for p in SidePiece:
            num = self.hand[p.value]
            if num != 0:
                piece = p.to_sfen().strip()
                s += str(num) + piece

        return s

    def to_tensor(self):
        
        feat = np.zeros((112,9,9))

        F_HAND_PAWN_POS = 0
        E_HAND_PAWN_POS = F_HAND_PAWN_POS + 18
        F_HAND_LANCE_POS = E_HAND_PAWN_POS + 18
        E_HAND_LANCE_POS = F_HAND_LANCE_POS + 4
        F_HAND_KNIGHT_POS = E_HAND_LANCE_POS + 4
        E_HAND_KNIGHT_POS = F_HAND_KNIGHT_POS + 4
        F_HAND_SILVER_POS = E_HAND_KNIGHT_POS + 4
        E_HAND_SILVER_POS = F_HAND_SILVER_POS + 4
        F_HAND_GOLD_POS = E_HAND_SILVER_POS + 4
        E_HAND_GOLD_POS = F_HAND_GOLD_POS + 4
        F_HAND_BISHOP_POS = E_HAND_GOLD_POS + 4
        E_HAND_BISHOP_POS = F_HAND_BISHOP_POS + 2
        F_HAND_ROOK_POS = E_HAND_BISHOP_POS + 2
        E_HAND_ROOK_POS = F_HAND_ROOK_POS + 2
        F_POS_PAWN_POS = E_HAND_ROOK_POS + 2
        E_POS_PAWN_POS = F_POS_PAWN_POS + 1
        F_POS_LANCE_POS = E_POS_PAWN_POS + 1
        E_POS_LANCE_POS = F_POS_LANCE_POS + 1
        F_POS_KNIGHT_POS = E_POS_LANCE_POS + 1
        E_POS_KNIGHT_POS = F_POS_KNIGHT_POS + 1
        F_POS_SILVER_POS = E_POS_KNIGHT_POS + 1
        E_POS_SILVER_POS = F_POS_SILVER_POS + 1
        F_POS_GOLD_POS = E_POS_SILVER_POS + 1
        E_POS_GOLD_POS = F_POS_GOLD_POS + 1
        F_POS_BISHOP_POS = E_POS_GOLD_POS + 1
        E_POS_BISHOP_POS = F_POS_BISHOP_POS + 1
        F_POS_ROOK_POS = E_POS_BISHOP_POS + 1
        E_POS_ROOK_POS = F_POS_ROOK_POS + 1
        F_POS_KING_POS = E_POS_ROOK_POS + 1
        E_POS_KING_POS = F_POS_KING_POS + 1
        F_POS_PPAWN_POS = E_POS_KING_POS + 1
        E_POS_PPAWN_POS = F_POS_PPAWN_POS + 1
        F_POS_PLANCE_POS = E_POS_PPAWN_POS + 1
        E_POS_PLANCE_POS = F_POS_PLANCE_POS + 1
        F_POS_PKNIGHT_POS = E_POS_PLANCE_POS + 1
        E_POS_PKNIGHT_POS = F_POS_PKNIGHT_POS + 1
        F_POS_PSILVER_POS = E_POS_PKNIGHT_POS + 1
        E_POS_PSILVER_POS = F_POS_PSILVER_POS + 1
        F_POS_PBISHOP_POS = E_POS_PSILVER_POS + 1
        E_POS_PBISHOP_POS = F_POS_PBISHOP_POS + 1
        F_POS_PROOK_POS = E_POS_PBISHOP_POS + 1
        E_POS_PROOK_POS = F_POS_PROOK_POS + 1
        POS_END_SIZE = E_POS_PROOK_POS + 1

        F_HAND_INDEX = { Piece.PAWN : F_HAND_PAWN_POS, Piece.LANCE : F_HAND_LANCE_POS, Piece.KNIGHT : F_HAND_KNIGHT_POS,
                         Piece.SILVER : F_HAND_SILVER_POS, Piece.GOLD : F_HAND_GOLD_POS, Piece.BISHOP : F_HAND_BISHOP_POS,
                         Piece.ROOK : F_HAND_ROOK_POS}
        E_HAND_INDEX = { Piece.PAWN : E_HAND_PAWN_POS, Piece.LANCE : E_HAND_LANCE_POS, Piece.KNIGHT : E_HAND_KNIGHT_POS,
                         Piece.SILVER : E_HAND_SILVER_POS, Piece.GOLD : E_HAND_GOLD_POS, Piece.BISHOP : E_HAND_BISHOP_POS,
                         Piece.ROOK : E_HAND_ROOK_POS}
        F_POS_INDEX = { Piece.PAWN : F_POS_PAWN_POS, Piece.LANCE : F_POS_LANCE_POS, Piece.KNIGHT : F_POS_KNIGHT_POS,
                         Piece.SILVER : F_POS_SILVER_POS, Piece.GOLD : F_POS_GOLD_POS, Piece.BISHOP : F_POS_BISHOP_POS,
                         Piece.ROOK : F_POS_ROOK_POS, Piece.PPAWN : F_POS_PPAWN_POS, Piece.PLANCE : F_POS_PLANCE_POS, Piece.PKNIGHT : F_POS_PKNIGHT_POS,
                         Piece.PSILVER : F_POS_PSILVER_POS, Piece.KING : F_POS_KING_POS, Piece.PBISHOP : F_POS_PBISHOP_POS,
                         Piece.PROOK : F_POS_PROOK_POS}
        E_POS_INDEX = { Piece.PAWN : E_POS_PAWN_POS, Piece.LANCE : E_POS_LANCE_POS, Piece.KNIGHT : E_POS_KNIGHT_POS,
                         Piece.SILVER : E_POS_SILVER_POS, Piece.GOLD : E_POS_GOLD_POS, Piece.BISHOP : E_POS_BISHOP_POS,
                         Piece.ROOK : E_POS_ROOK_POS, Piece.PPAWN : E_POS_PPAWN_POS, Piece.PLANCE : E_POS_PLANCE_POS, Piece.PKNIGHT : E_POS_PKNIGHT_POS,
                         Piece.PSILVER : E_POS_PSILVER_POS, Piece.KING : E_POS_KING_POS, Piece.PBISHOP : E_POS_PBISHOP_POS,
                         Piece.PROOK : E_POS_PROOK_POS}
 
        if self.turn == Side.BLACK:
            me_turn = Side.BLACK
            opp_turn = Side.WHITE
        else:
            me_turn = Side.WHITE
            opp_turn = Side.BLACK
        
        for sq, spc in enumerate(self.pos):
            sq_file = sq_to_file(sq)
            sq_rank = sq_to_rank(sq)
            if spc == SidePiece.EMPTY:
                continue
            pc = spc.to_piece()
            if spc.side() == me_turn:
                index = F_POS_INDEX[pc]
            else:
                index = E_POS_INDEX[pc]
            feat[index][sq_file][sq_rank] = 1

        for hp, hand_num in enumerate(self.hand):
            if hand_num > 0:
                hp_side = SidePiece(hp)
                hp = hp_side.to_piece()
                # 手駒は重ね合わせで表現してみる
                if hp_side.side() == me_turn:
                    for num in range(hand_num + 1):
                        index = F_HAND_INDEX[hp] + num
                        feat[index] = np.ones((9,9))
                else:
                    for num in range(hand_num + 1):
                        index = E_HAND_INDEX[hp] + num
                        feat[index] = np.ones((9,9))

        return feat

if  __name__ == '__main__':
    
    result = "l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1"
    
    #結果をpythonで読み込む
    state = State()
    state.load_sfen(result)
    print(state.out())
    print(state.out_sfen())

    action = Action()
    action.load_sfen("9a9b")
    print(action.to_index(Side.BLACK))
    print(action.to_index(Side.WHITE))
    print(state.to_tensor())
