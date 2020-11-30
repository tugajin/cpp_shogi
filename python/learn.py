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
                kif = shogi.CSA.Parser.parse_file(path)[0]
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

        print("end load")

    def __len__(self):
        return self.datanum

    def __getitem__(self, idx):
        out_data = self.data[idx]
        out_label = self.label[idx]

        return out_data, out_label

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(104, 128, kernel_size= 3, padding= 1)
        self.conv2 = nn.Conv2d(128, 128, kernel_size=3, padding= 1)
        self.conv3 = nn.Conv2d(128, 128, kernel_size=3, padding= 1)
        self.conv4 = nn.Conv2d(128, 128, kernel_size=3, padding= 1)

        self.fc1 = nn.Linear(10368, 128)
        self.fc2 = nn.Linear(128, 2187)
        self.fc3 = nn.Linear(128, 1)

    def forward(self, x):
        x = self.conv1(x)
        x = F.relu(x)

        #x = self.conv2(x)
        #x = F.relu(x)

        #x = self.conv3(x)
        #x = F.relu(x)

        x = self.conv4(x)
        x = F.relu(x)

        x = torch.flatten(x, 1)
        x = self.fc1(x)
        x = F.relu(x)

        # policy network
        px = self.fc2(x)
        #m = nn.Softmax(dim=1)
        #px = m(px)

        # value network
        vx = self.fc3(x)
        vx = F.tanh(vx)
        return (px, vx)

def pos_sfen_to_tensor(sfen_data):

    data = []

    for sfen in sfen_data:
        state = State()
        state.load_sfen(sfen)
        state_array = state.to_array()
        data.append(state_array)
    
    #return torch.tensor(np.array(data), dtype=torch.float64)
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
        #np_policy = np.full(2187, 0.000045746)
        np_policy = np.full(2187, 0.0)

        np_policy[index] = 1.0
        move_list.append(np_policy)
    #tensor_policy = torch.tensor(move_list, dtype=torch.float64)
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
    #tensor_result = torch.tensor(result_list, dtype=torch.float64)
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

        #policy_loss = torch.sum(-(policy_target) * torch.log((policy_output))) / len(sfen_data)
        #policy_loss = torch.sum(-(policy_target) * torch.log(m(policy_output))) / len(sfen_data)
        policy_loss = torch.sum(-(policy_target) * m2(policy_output)) / len(sfen_data)
        value_loss = torch.sum((value_output - value_target) ** 2) / len(sfen_data)
        loss = policy_loss + value_loss
        #loss =  value_loss
        #loss = policy_loss

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
            move = shogi.Move.from_usi(sfen_target[0])
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
        if epoch % 1000 == 0:
            disp_test(model, device, dataset)
        #test(model, device, test_loader())

if __name__ == '__main__':
    main()