import shogi
import shogi.CSA

import numpy as np
import time
import glob
import sqlite3

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torchvision import datasets, transforms

from resnet import *

import cpp_lib


class CSADataset(torch.utils.data.Dataset):

    def __init__(self, db_path):
        self.conn = sqlite3.connect(db_path)
        c = self.conn.cursor()
        c.execute('SELECT count(1) FROM csa_record')
        row = c.fetchone()
        self.datanum = row[0]
        print("datanum:", self.datanum)
        print("end load")
    
    def __del__(self):
        self.conn.commit()
        self.conn.close()

    def __len__(self):
        return self.datanum

    def __getitem__(self, idx):
        c = self.conn.cursor()
        c.execute('SELECT sfen_pos, sfen_move, winner FROM csa_record where id = ?',((idx+1),))
        row = c.fetchone()
        out_data = row[0]
        out_label = (row[1],row[2])
        return out_data, out_label


def pos_sfen_to_tensor(sfen_data):

    data = np.zeros((len(sfen_data), 131, 9, 9))
    cpp_lib.sfen_to_tensor(sfen_data, data)
    
    return torch.tensor(data, dtype=torch.float32)


def move_sfen_to_tensor(sfen_pos_str, sfen_move_str):

    sfen_move = sfen_move_str[0]
    sfen_result = sfen_move_str[1]

    # 手を変換
    move_list = []
    for pos, move in zip(sfen_pos_str, sfen_move):
        if ' b ' in pos:
            turn = 'b'
        else:
            turn = 'w'
        index = cpp_lib.move_to_index(move, turn)
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
        # listに変換してやる必要がある
        data = pos_sfen_to_tensor(list(sfen_data))
        
        policy_target, value_target = move_sfen_to_tensor(sfen_data, sfen_target)
        
        data, policy_target, value_target = data.to(device), policy_target.to(device), value_target.to(device)

        optimizer.zero_grad()
        policy_output, value_output = model(data)

        m2 = nn.LogSoftmax(dim=1)

        policy_loss = torch.sum(-(policy_target) * m2(policy_output)) / len(sfen_data)
        value_loss = torch.sum((value_output - value_target) ** 2) / len(sfen_data)
        loss = policy_loss + value_loss

        loss.backward()
        optimizer.step()
        all_loss += loss.item()
        num += 1
        if batch_idx % 16 == 0:
        #if False:
            print('Train Epoch: {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                epoch, batch_idx * len(data), len(loader.dataset),
                100. * batch_idx / len(loader), loss.item()))
    if False:
    #if epoch % 1 == 0:
        print('Train Epoch:',epoch, " loss:",all_loss / len(loader))

def test(model, device, loader):
    model.eval()

    test_policy_loss = 0
    test_value_loss = 0
    correct = 0

    with torch.no_grad():

        for (sfen_data, sfen_target) in loader:
            # sfenを局面情報へ変換
            # listに変換してやる必要がある
            data = pos_sfen_to_tensor(list(sfen_data))
        
            policy_target, value_target = move_sfen_to_tensor(sfen_data, sfen_target)
        
            data, policy_target, value_target = data.to(device), policy_target.to(device), value_target.to(device)

            policy_output, value_output = model(data)

            m2 = nn.LogSoftmax(dim=1)

            policy_loss = torch.sum(-(policy_target) * m2(policy_output))
            value_loss = torch.sum((value_output - value_target) ** 2)
            test_policy_loss += policy_loss.item()
            test_value_loss += value_loss.item()
            
            policy_pred = policy_output.argmax(dim=1, keepdim=True)  # get the index of the max log-probability
            polciy_target_index = policy_target.argmax(dim=1, keepdim=True)
            
            correct += policy_pred.eq(polciy_target_index.view_as(policy_pred)).sum().item()

    test_policy_loss /= len(loader.dataset)
    test_value_loss /= len(loader.dataset)

    print('\nTest set: Average policy_loss: {:.4f} value_loss: {:.4f}, Accuracy: {}/{} ({:.0f}%)\n'.format(
            test_policy_loss, test_value_loss, correct, len(loader.dataset),
            100. * correct / len(loader.dataset)))

def disp_test(model, device, dataset):
    model.eval()
    tensor_list = []
    with torch.no_grad():
        #for i in range(len(dataset)):
        for i in [1, 5]:
            sfen_data, sfen_target = dataset[i]
            # sfenを局面情報へ変換
            data = pos_sfen_to_tensor([sfen_data])
            tensor_list.append(data)
            # sfenを手と勝ち負け情報へ変換
            data = data.to(device)
            board = shogi.Board(sfen=sfen_data)
            print(board.kif_str())
            if ' b ' in sfen_data:
                turn = 'b'
            else:
                turn = 'w'
            index = cpp_lib.move_to_index(sfen_target[0], turn)
            policy,value = model(data)
            print("policy BEST:",policy[0][index])
            print("policy_index",index)
            print("policy_max:",torch.max(policy,1))
            print("policy_min:",torch.min(policy,1))
            print("policy_mean:",torch.mean(policy))
            print("value:",value[0])

def save_model(model,epoch):
    model_path = 'model.pt'
    model_history_path = str(epoch) + model_path
    torch.save(model.state_dict(), model_path)
    torch.save(model.state_dict(), model_history_path)

def main():
    
    torch.manual_seed(0)

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(device)

    test_dataset = CSADataset("/media/tugajin/HD-LDF-A/db/record.sqlite3")
    train_dataset = CSADataset("/media/tugajin/HD-LDF-A/db/record_test.sqlite3")

    train_loader = torch.utils.data.DataLoader(train_dataset, batch_size=512, shuffle=True)
    test_loader = torch.utils.data.DataLoader(test_dataset, batch_size=512, shuffle=True)

    model = Net().to(device)
    optimizer = optim.Adam(model.parameters(), lr=1e-4, weight_decay=1e-8)

    for epoch in range(1, 50):
        train(model, device, train_loader, optimizer, epoch)
        test(model, device, test_loader)
        save_model(model, epoch)

if __name__ == '__main__':
    main()