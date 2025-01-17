import numpy as np

def solve(p1, p2, p3):
    A = np.array([
        [p1[0]*p1[0], p1[0], 1], 
        [p2[0]*p2[0], p2[0], 1], 
        [p3[0]*p3[0], p3[0], 1]
    ])
    b = np.array([p1[1], p2[1], p3[1]])

    print(np.linalg.solve(A, b))
    

solve((0, 5.8), (1, 58.03), (2, 109.32))
solve((0, 5.8), (1, 51.03), (2, 107.32))
