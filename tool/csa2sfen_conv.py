import glob
import shogi
import shogi.CSA

kif_list = glob.glob("./2019/*.csa")
#kif_list.extend(glob.glob("./2018/*.csa"))
#kif_list.extend(glob.glob("./2017/*.csa"))
#kif_list.extend(glob.glob("./2016/*.csa"))

with open("./out.sfen", mode="a") as f:
    for path in kif_list:
        try:
            kif = shogi.CSA.Parser.parse_file(path)[0]
            string = "startpos moves"
            if len(kif['moves']) == 0:
                print("null:"+path)
                continue
            for mv in kif['moves']:
                string = string + " "
                string += mv
            string += "\n"
            f.write(string)
        except:
            print("error:"+path)
