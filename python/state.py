import subprocess
import sys

BLACK = 0
WHITE = 1
SIDE_NUM = 2

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
PIECE_NUM = 32

SQUARE_SIZE = 81

SFEN_PIECE =  ". P L N S B R G K +P+L+N+S+B+R+G+.p l n s b r g k +p+l+n+s+b+r+g+k"

SFEN_FILE = "987654321"
SFEN_RANK = "abcdefghi"

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
    piece =  EMPTY
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
        self.prom = EMPTY
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
    
    def load_sfen(self,sfen_string):
    """
    文字列からaction形式に変換する
    """
        self.clear()
        sfen = sfen_string.split(":")
        s = sfen[0]
        self.score = float(sfen[-1])
        # drop move
        if s[1] == '*':
            to_file = self.sfen_to_file(s[2])
            to_rank = self.sfen_to_rank(s[3])
            self.move_to = self.make_square(to_file,to_rank)
            self.piece =  State.sfen_to_piece(s[0])

            self.prom = False
        else:
            from_file = self.sfen_to_file(s[0])
            from_rank = self.sfen_to_rank(s[1])
            to_file = self.sfen_to_file(s[2])
            to_rank = self.sfen_to_rank(s[3])

            self.move_from =   self.make_square(from_file,from_rank)
            self.move_to =  self.make_square(to_file,to_rank)
            self.piece = EMPTY
            self.prom = (len(s.strip()) == 5)

    def out(self):
    """
    actionの中身を表示する。デバッグ用
    """
        s = str(self.move_from) + ":" + str(self.move_to) + ":" + str(self.piece) + ":" + str(self.prom) + ":" + str(self.score)
        return s

class State:
    pos =  [EMPTY]*SQUARE_SIZE
    turn =  BLACK
    hand = [0]*PIECE_NUM

    def __init__(self):
    """
    コンストラクタ
    """
        self.clear()

    def clear(self):
    """
    初期化
    """
        self.pos =  [EMPTY]*SQUARE_SIZE
        self.turn = BLACK
        self.hand = [0]*PIECE_NUM

    @staticmethod
    def sfen_to_piece(s):
    """
    文字列から駒(piece)に変換する
    """
        piece = SFEN_PIECE.find(s)
        if piece == -1:
            return EMPTY
        else:
            return int(piece/2)
    
    @staticmethod
    def piece_to_sfen(p):
    """
    pieceから文字列に変換する
    """
        piece = SFEN_PIECE[(2*p):(2*p)+2]
        return  piece

    @staticmethod
    def piece_prom(p):
    """
    pieceを成った駒にします
    """
        return p + 8

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
                piece = self.sfen_to_piece(s)
                if prom_flag:
                    piece = self.piece_prom(piece)
                self.pos[pos_sp] =  piece
                pos_sp += 1
        #load turn
        if sfen_list[list_sp] == 'b':
            self.turn = BLACK
        elif sfen_list[list_sp] == 'w':
            self.turn = WHITE
        list_sp += 1
        #load hand
        num = 0
        while True:
            s = sfen_list[list_sp]
            list_sp += 1
            if list_sp >= sfen_len:
                break
            elif s == ' ':
                break
            elif s == '-':
                break
            elif s.isdigit():
                num = (num * 10) + int(s)
            else:
                piece = self.sfen_to_piece(s)
                if num == 0:
                    num = 1
                self.hand[piece] = num

    def out(self):
    """
    stateを文字列に変換します（デバッグ用）。
    """
        s = ""
        if self.turn == BLACK:
            s += "BLACK"
        elif self.turn == WHITE:
            s += "WHITE"
        s += "\n"
        for p in range(WPAWN,WGOLD):
            s +=  self.piece_to_sfen(p).strip() + ":" +str(self.hand[p]) + " "
        s += "\n"
        for index,p in enumerate(self.pos):
            s +=  self.piece_to_sfen(p)
            if (index + 1) % 9 == 0:
                s += "\n"
        for p in range(BPAWN,BGOLD):
            s +=  self.piece_to_sfen(p).strip() + ":" +str(self.hand[p]) + " "
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
            if p == EMPTY:
                num += 1
                pos_num += 1
            else:
                sfen = self.piece_to_sfen(p)
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
        
        if self.turn == BLACK:
            s += "b "
        elif self.turn == WHITE:
            s += "w "
        
        for p in range(PIECE_NUM):
            num = self.hand[p]
            if num != 0:
                piece = self.piece_to_sfen(p).strip()
                s += str(num) + piece

        return s



if  __name__ == '__main__':
    
    # 外部のプログラムを実行する
    proc = subprocess.run(["./shogi","init"],stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    result = proc.stdout.decode("utf8")
    
    #結果をpythonで読み込む
    state = State()
    state.load_sfen(result)
    print(state.out())
    print(state.out_sfen())

    # 外部のプログラムを実行する
    proc = subprocess.run(["./shogi","think"],stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    result = proc.stdout.decode("utf8")
    a_list = ActionList()
    a_list.init(result)
    for action in a_list.action_list:
        print(action.out())
        

