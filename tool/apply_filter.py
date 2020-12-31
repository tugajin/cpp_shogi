import glob
import shutil
import os

kif_list = glob.glob("./2019/*.csa")
kif_list.extend(glob.glob("./2018/*.csa"))
kif_list.extend(glob.glob("./2017/*.csa"))

for path in kif_list:
    is_toryo = False
    rate_is_good = False
    black_rating_flag = False
    white_rating_flag = False
    with open(path) as f:
        csa_list = f.readlines()
        for csa_string in csa_list:
            if '_rate' in csa_string:
                csa_string_list = csa_string.split(':')
                try:
                    rate = float(csa_string_list[-1])
                    if 'black_' in csa_string:
                        black_rating_flag = (rate >= 3600)
                    if 'white_' in csa_string:
                        white_rating_flag = (rate >= 3600)
                except:
                    pass
            if  '%TORYO' in csa_string:
                is_toryo = True
    if is_toryo and black_rating_flag and white_rating_flag:
        basename = os.path.basename(path)
        print(basename)
        dist_path = './ok/' + basename
        shutil.copyfile(path, dist_path)