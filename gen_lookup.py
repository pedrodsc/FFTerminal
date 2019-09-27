# TODO
#
# 1. Gerar uma tabela de lookup com N termos de exp(-j2pik/N) 0 < k < N -1
# 2. Escrever em lookup.txt

import numpy as np

N = 1024

lookup = np.exp( np.complex(0,-2*np.pi/N) * np.arange(0,N))

print(lookup)
