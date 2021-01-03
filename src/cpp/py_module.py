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
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("device is", device)
    model = model.to(device)
    return model, device

def forward(model_tuple, feat):
    print(model_tuple[0])
    print(model_tuple[1])
    print(type(feat))
    print(feat)
    return [1.0,-2.0,3.0]