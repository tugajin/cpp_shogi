import sqlite3
import shogi
import shogi.CSA
import glob

CSA_PATH = "/media/tugajin/HD-LDF-A/ok/*.csa"
DB_PATRH = "/media/tugajin/HD-LDF-A/db/record_test.sqlite3"

def main():
    conn = sqlite3.connect(DB_PATRH)
    c = conn.cursor()

    c.execute('drop table if exists csa_record')

    c.execute('create table csa_record (id integer primary key autoincrement, sfen_pos, sfen_move, winner)')
    
    kif_list = glob.glob(CSA_PATH)
    for i,path in enumerate(kif_list):
        print(i,' ',end='',flush=True)
        kif_info_list = shogi.CSA.Parser.parse_file(path)
        for kif in kif_info_list: 
            board = shogi.Board(sfen=kif['sfen'])
            moves = kif['moves']
            winner = kif['win']
        
            for move in moves:
                # 局面データを追加
                pos_sfen = board.sfen()
                move_sfen = move
                c.execute('insert into csa_record (sfen_pos, sfen_move, winner) values (?,?,?)',(pos_sfen, move_sfen,winner))
                # 次の局面へ移動
                board.push(shogi.Move.from_usi(move))
        if i % 10 == 0:
            conn.commit()

    conn.commit()
    conn.close()

main()