import torch
import torch.nn as nn
import torch.nn.functional as F

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
        #loss = policy_loss
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
