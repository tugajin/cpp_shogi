from state import *

import shogi
import shogi.CSA

import numpy as np
import time
import glob

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms
from torch.optim.lr_scheduler import StepLR

class CSADataset(torch.utils.data.Dataset):
    def __init__(self, csa_path):
        
        kif_list = glob.glob(csa_path)
        
        pos_data = []
        move_data = []

        print("start load")

        for path in kif_list:
            try:
                kif_list = shogi.CSA.Parser.parse_file(path)
                for kif in kif_list: 
                    board = shogi.Board(sfen=kif['sfen'])
                    moves = kif['moves']
                    winner = kif['win']
                
                    for move in moves:
                        # 局面データ、手のデータを追加
                        pos_data.append(board.sfen())
                        if board.turn == 0:
                            turn = 'b'
                        else:
                            turn = 'w'

                        move_data.append((move, winner, turn))
                    
                        # 次の局面へ移動
                        board.push(shogi.Move.from_usi(move))

            except:
                print("error:"+path)
        
        self.datanum = len(pos_data)
        self.data = pos_data
        self.label = move_data
        print("datanum:",self.datanum)
        print("end load")

    def __len__(self):
        return self.datanum

    def __getitem__(self, idx):
        out_data = self.data[idx]
        out_label = self.label[idx]

        return out_data, out_label

HIDDEN_LAYER_NUM = 128

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv0 = nn.Conv2d(104, HIDDEN_LAYER_NUM, kernel_size= 3, padding= 1)
        self.conv1  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv2  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv3  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv4  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv5  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv6  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv7  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv8  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv9  = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv10 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv11 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv12 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv13 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv14 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv15 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv16 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv17 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv18 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv19 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv20 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv21 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv22 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv23 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv24 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv25 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv26 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv27 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv28 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv29 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv30 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv31 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv32 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv33 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv34 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv35 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv36 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv37 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        self.conv38 = nn.Conv2d(HIDDEN_LAYER_NUM, HIDDEN_LAYER_NUM, kernel_size=3, padding= 1)
        
        self.bn0  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn1  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn2  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn3  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn4  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn5  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn6  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn7  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn8  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn9  = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn10 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn11 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn12 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn13 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn14 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn15 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn16 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn17 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn18 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn19 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn20 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn21 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn22 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn23 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn24 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn25 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn26 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn27 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn28 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn29 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn30 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn31 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn32 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn33 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn34 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn35 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn36 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn37 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)
        self.bn38 = nn.BatchNorm2d(HIDDEN_LAYER_NUM)

        self.conv_p1 = nn.Conv2d(HIDDEN_LAYER_NUM, 2, kernel_size=1)
        self.bn_p1 = nn.BatchNorm2d(2)
        self.fc_p2 = nn.Linear(2 * 9 * 9, 2187)

        self.conv_v1 = nn.Conv2d(HIDDEN_LAYER_NUM, 1, kernel_size=1)
        self.bn_v1 = nn.BatchNorm2d(1)
        self.fc_v2 = nn.Linear(9 * 9, HIDDEN_LAYER_NUM)
        self.fc_v3 = nn.Linear(HIDDEN_LAYER_NUM, 1)


    def forward(self, x):
        h0 = F.hardswish(self.bn0(self.conv0(x)))

        h1 = F.hardswish(self.bn1(self.conv1(h0)))
        h2 = F.hardswish(self.bn2(self.conv2(h1)) + h0)
        
        h3 = F.hardswish(self.bn3(self.conv3(h2)))
        h4 = F.hardswish(self.bn4(self.conv4(h3)) + h2)

        h5 = F.hardswish(self.bn5(self.conv5(h4)))
        h6 = F.hardswish(self.bn6(self.conv6(h5)) + h4)
        
        h7 = F.hardswish(self.bn7(self.conv7(h6)))
        h8 = F.hardswish(self.bn8(self.conv8(h7)) + h6)
        
        h9 = F.hardswish(self.bn9(self.conv9(h8)))
        h10 = F.hardswish(self.bn10(self.conv10(h9)) + h8)

        h11 = F.hardswish(self.bn11(self.conv11(h10)))
        h12 = F.hardswish(self.bn12(self.conv12(h11)) + h10)
        
        h13 = F.hardswish(self.bn13(self.conv13(h12)))
        h14 = F.hardswish(self.bn14(self.conv14(h13)) + h12)

        h15 = F.hardswish(self.bn15(self.conv15(h14)))
        h16 = F.hardswish(self.bn16(self.conv16(h15)) + h14)
        
        h17 = F.hardswish(self.bn17(self.conv17(h16)))
        h18 = F.hardswish(self.bn18(self.conv18(h17)) + h16)
        
        h19 = F.hardswish(self.bn19(self.conv19(h18)))
        h20 = F.hardswish(self.bn20(self.conv20(h19)) + h18)

        h21 = F.hardswish(self.bn21(self.conv21(h20)))
        h22 = F.hardswish(self.bn22(self.conv22(h21)) + h20)
        
        h23 = F.hardswish(self.bn23(self.conv23(h22)))
        h24 = F.hardswish(self.bn24(self.conv24(h23)) + h22)

        h25 = F.hardswish(self.bn25(self.conv25(h24)))
        h26 = F.hardswish(self.bn26(self.conv26(h25)) + h24)
        
        h27 = F.hardswish(self.bn27(self.conv27(h26)))
        h28 = F.hardswish(self.bn28(self.conv28(h27)) + h26)
        
        h29 = F.hardswish(self.bn29(self.conv29(h28)))
        h30 = F.hardswish(self.bn30(self.conv30(h29)) + h28)

        h31 = F.hardswish(self.bn31(self.conv31(h30)))
        h32 = F.hardswish(self.bn32(self.conv32(h31)) + h30)
        
        h33 = F.hardswish(self.bn33(self.conv33(h32)))
        h34 = F.hardswish(self.bn34(self.conv34(h33)) + h32)

        h35 = F.hardswish(self.bn35(self.conv35(h34)))
        h36 = F.hardswish(self.bn36(self.conv36(h35)) + h34)
        
        h37 = F.hardswish(self.bn37(self.conv37(h36)))
        h38 = F.hardswish(self.bn38(self.conv38(h37)) + h36)

        #policy network
        h_p1 = F.hardswish(self.bn_p1(self.conv_p1(h38)))
        h_p1 = torch.flatten(h_p1,1)
        out_p = self.fc_p2(h_p1)

        # value network
        h_v1 = F.hardswish(self.bn_v1(self.conv_v1(h38)))
        h_v1 = torch.flatten(h_v1,1)
        h_v2 = F.hardswish(self.fc_v2(h_v1))

        out_v = torch.tanh(self.fc_v3(h_v2))

        return (out_p, out_v)

def pos_sfen_to_tensor(sfen_data):

    data = []

    for sfen in sfen_data:
        state = State()
        state.load_sfen(sfen)
        state_array = state.to_array()
        data.append(state_array)
    
    return torch.tensor(np.array(data), dtype=torch.float32)


def move_sfen_to_tensor(sfen_data):

    sfen_move = sfen_data[0]
    sfen_result = sfen_data[1]
    sfen_turn = sfen_data[2]

    # 手を変換
    move_list = []
    for move, turn in zip(sfen_move, sfen_turn):
        action = Action()
        action.load_sfen(move)
        side = Side.BLACK if turn == 'b'  else Side.WHITE
        index = action.to_index(side)
        np_policy = np.full(2187, 0.0)

        np_policy[index] = 1.0
        move_list.append(np_policy)
    tensor_policy = torch.tensor(move_list, dtype=torch.float32)

    # 勝敗を変換
    result_list = []
    for result in sfen_result:
        if result == 'b':
            result_list.append(1)
        elif result == 'w':
            result_list.append(-1)
        else:
            result_list.append(0)
    tensor_result = torch.tensor(result_list, dtype=torch.float32)
    return (tensor_policy, tensor_result)

def train(model, device, loader, optimizer, epoch):
    model.train()
    all_loss = 0
    num = 0
    for batch_idx, (sfen_data, sfen_target) in enumerate(loader):
        # sfenを局面情報へ変換
        data = pos_sfen_to_tensor(sfen_data)
        # sfenを手と勝ち負け情報へ変換
        policy_target, value_target = move_sfen_to_tensor(sfen_target)

        data, policy_target, value_target = data.to(device), policy_target.to(device), value_target.to(device)

        optimizer.zero_grad()
        policy_output, value_output = model(data)

        m = nn.Softmax(dim=1)
        m2 = nn.LogSoftmax(dim=1)

        policy_loss = torch.sum(-(policy_target) * m2(policy_output)) / len(sfen_data)
        value_loss = torch.sum((value_output - value_target) ** 2) / len(sfen_data)
        loss = policy_loss + value_loss

        loss.backward()
        optimizer.step()
        all_loss += loss.item()
        num += 1
        #if batch_idx % 16 == 0:
        if False:
            print('Train Epoch: {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                epoch, batch_idx * len(data), len(loader.dataset),
                100. * batch_idx / len(loader), loss.item()))
    if epoch % 10 == 0:
        print('Train Epoch: ',epoch, " loss:",all_loss)
def test(model, device, loader):
    model.eval()

    test_loss = 0
    correct = 0

    with torch.no_grad():
        for data, target in loader:
            data, target = data.to(device), target.to(device)
            output = model(data)

            test_loss += F.null_loss(output, target, reduction = 'sum').item()
            pred = output.argmax(dim=1, keepdim=True)  # get the index of the max log-probability
            correct += pred.eq(target.view_as(pred)).sum().item()

    test_loss /= len(loader.dataset)

    print('\nTest set: Average loss: {:.4f}, Accuracy: {}/{} ({:.0f}%)\n'.format(
            test_loss, correct, len(test_loader.dataset),
            100. * correct / len(test_loader.dataset)))

def disp_test(model, device, dataset):
    model.eval()
    tensor_list = []
    with torch.no_grad():
        #for i in range(len(dataset)):
        for i in [0, 150, 300]:
            sfen_data, sfen_target = dataset[i]
            # sfenを局面情報へ変換
            data = pos_sfen_to_tensor([sfen_data])
            tensor_list.append(data)
            # sfenを手と勝ち負け情報へ変換
            data = data.to(device)
            board = shogi.Board(sfen=sfen_data)
            print(board.kif_str())
            action = Action()
            action.load_sfen(sfen_target[0])
            if board.turn == 0:
                turn = Side.BLACK
            else:
                turn = Side.WHITE
            index = action.to_index(turn)
            policy,value = model(data)
            #print(policy[0])
            print("policy BEST:",policy[0][index])
            print("policy_max:",torch.max(policy,1))
            print("policy_min:",torch.min(policy,1).values)
            print("policy_mean:",torch.mean(policy))
            print("policy_index",index)
            print("value:",value[0])

def main():
    
    torch.manual_seed(0)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    dataset = CSADataset("../record/*.csa")

    train_loader = torch.utils.data.DataLoader(dataset, batch_size=32, shuffle=True)

    model = Net().to(device)
    #model = model.double()
    optimizer = optim.Adam(model.parameters(), lr=1e-5, weight_decay=1e-8)

    for epoch in range(1, 100000):
        train(model, device, train_loader, optimizer, epoch)
        if epoch % 100 == 0:
            disp_test(model, device, dataset)
        #test(model, device, test_loader())

if __name__ == '__main__':
    main()