import sys
import os
sys.path.append(os.path.join(os.path.dirname(os.path.abspath('__file__')),'../../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath('__file__')),'../../../python'))

import torch
from resnet import *

def load_model():
    model = Net()
    print("load model .... ",end="",flush=True)
    model.load_state_dict(torch.load(os.path.join(os.path.dirname(os.path.abspath('__file__')),'../../../python/model.pt')))
    print("end")
    #device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    device = torch.device("cpu")
    print("device is", device)
    model = model.to(device)
    return model, device

def my_close(model_tuple):
    model = model_tuple[0]
    device = model_tuple[1]
    
    new_device = torch.device("cpu")
    model = model.to(new_device)
    return model, new_device

def forward(model_tuple, feat):
    model = model_tuple[0]
    device = model_tuple[1]
    feat = torch.tensor(feat, dtype=torch.float32)
    feat = feat.to(device)
    output = model(feat)
    policy_score = output[0].cpu().detach().numpy()
    value_score = output[1].cpu().detach().numpy()
    print(policy_score)
    print(value_score)
    print((policy_score.shape))
    print((value_score.shape))
    
    return (policy_score, value_score)