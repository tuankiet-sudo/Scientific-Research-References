import csv
import numpy as np
import matplotlib.pyplot as plt


class Point:
    def __init__(self, t, v) -> None:
        self.t = t
        self.v = v


class Line:
    def __init__(self, p1, p2) -> None:
        self.slope = (p2.v-p1.v)/(p2.t-p1.t)
        self.intercept = p1.v - p1.t*self.slope
                
    def set(self, slp, itc):
        self.slope = slp
        self.intercept = itc

    def subs(self, p):
        return self.slope*p.t + self.intercept
        
        
class ConvexHull:
    def __init__(self) -> None:
        self.upper = []
        self.lower = []
        
    def insert_u(self, p):
        self.upper.append(p)
        
        while len(self.upper) > 2:
            p1 = self.upper[-1]
            p2 = self.upper[-2]
            p3 = self.upper[-3]
            
            if Line(p1, p3).subs(p2) >= p2.v:
                del self.upper[-2]
            else:
                return
            
    def get_u(self):
        time = [p.t for p in self.upper]
        value = [p.v for p in self.upper]
        
        return time, value
    
    def get_u_line(self):
        max_t = -1
        index = -1
        for i in range(len(self.upper)-1):
            if max_t < self.upper[i+1].t - self.upper[i].t:
                index = i
                max_t = self.upper[i+1].t - self.upper[i].t
            elif max_t == self.upper[i+1].t - self.upper[i].t:
                if Line(self.upper[i+1], self.upper[i]).slope > Line(self.upper[index], self.upper[index+1]).slope:
                    index = i

        return Line(self.upper[index], self.upper[index+1])

    def insert_l(self, p):
        self.lower.append(p)
        
        while len(self.lower) > 2:
            p1 = self.lower[-1]
            p2 = self.lower[-2]
            p3 = self.lower[-3]
            
            if p2.v >= Line(p1, p3).subs(p2):
                del self.lower[-2]
            else:
                return

    def get_l(self):
        time = [p.t for p in self.lower]
        value = [p.v for p in self.lower]
        
        return time, value
    
    def get_l_line(self):
        max_t = -1
        index = -1
        for i in range(len(self.lower)-1):
            if max_t < self.lower[i+1].t - self.lower[i].t:
                index = i
                max_t = self.lower[i+1].t - self.lower[i].t
            elif max_t == self.lower[i+1].t - self.lower[i].t:
                if Line(self.lower[i+1], self.lower[i]).slope > Line(self.lower[index], self.lower[index+1]).slope:
                    index = i

        return Line(self.lower[index], self.lower[index+1])


class Segment:
    def __init__(self, length, upper, lower, cvx) -> None:
        self.length = length
        self.upper = upper
        self.lower = lower
        self.cvx = cvx
        
    def dump(self):
        print("{} -> u = {}x + {} -> l = {}x + {}".format(
            self.length, self.upper.slope, self.upper.intercept,
            self.lower.slope, self.lower.intercept
        ))


class Poly:
    def __init__(self, length, a, b, c) -> None:
        self.length = length
        self.a = a
        self.b = b
        self.c = c
        
    def dump(self):
        print("{} -> y = {}x^2 + {}x + {}".format(
            self.length, self.a, self.b, self.c
        ))


class Algo:
    def __init__(self) -> None:
        self.segs = []
        
    def fit(self, data, error) -> None:
        p1, p2 = None, None
        prev_p, pivot = None, None
        u, l = None, None

        u_b, l_b = None, None
        b, c = None, None

        cvx = ConvexHull()
        count = 0        
        flag, change = False, False
        
        for p in data:
            base_p = p
            if not flag:
                if p1 == None:
                    pivot = p
                    p1 = Point(0, p.v)
                    p = Point(p.t - pivot.t, p.v)
                elif p2 == None:
                    p2 = Point(p.t - pivot.t, p.v)
                    p = Point(p.t - pivot.t, p.v)
                    u = Line(p1, Point(p2.t, p2.v+error))
                    l = Line(p1, Point(p2.t, p2.v-error))
                else:
                    p = Point(p.t - pivot.t, p.v)
                    if p.v - u.subs(p) > error or l.subs(p) - p.v > error:
                        flag = True
                        b = u.slope
                        c = u.intercept
                        u_b = (prev_p.v+error-prev_p.t*b-c) / (prev_p.t*prev_p.t)
                        l_b = (prev_p.v-error-prev_p.t*b-c) / (prev_p.t*prev_p.t)
                        # self.segs.append(Poly(count, 0, u.slope, u.intercept))
                        # count = 0
                        # flag = False
                        # p2 = None
                        # pivot = Point(p.t+pivot.t, p.v)
                        # p = Point(0, p.v)
                        # p1 = Point(0, p.v)
                    else:                    
                        if u.subs(p) - p.v > error:
                            u = Line(p1, Point(p.t, p.v+error))
                        if p.v - l.subs(p) > error:
                            l = Line(p1, Point(p.t, p.v-error))
                            
                prev_p = p

            if flag:
                if p.t != base_p.t - pivot.t:
                    p = Point(p.t - pivot.t, p.v)
                n_u_b = (p.v+error-p.t*b-c) / (p.t*p.t)
                n_l_b = (p.v-error-p.t*b-c) / (p.t*p.t)
            
                if l_b > u_b or n_l_b > n_u_b or l_b > n_u_b or n_l_b > u_b:
                    self.segs.append(Poly(count, (u_b+l_b)/2, b, c))
                    count = 0
                    flag = False
                    p2 = None
                    pivot = base_p
                    p = Point(0, p.v)
                    p1 = Point(0, p.v)
                else:
                    u_b = min(n_u_b, u_b)
                    l_b = max(n_l_b, l_b)
                    
            
            count += 1
            
        
class Swing:
    def __init__(self) -> None:
        self.segs = []
        
    def fit(self, data, error) -> None:
        p1, p2 = None, None
        u, l = None, None
        cvx = ConvexHull()
        count = 0
        
        for p in data:
            if p1 == None:
                p1 = p
            elif p2 == None:
                p2 = p
                u = Line(p1, Point(p2.t, p2.v+error))
                l = Line(p1, Point(p2.t, p2.v-error))
            else:
                if p.v - u.subs(p) > error or l.subs(p) - p.v > error:
                    self.segs.append(Segment(count, u, l, cvx))
                    count = 0
                    p2 = None
                    p1 = p
                    cvx = ConvexHull()
                else:                    
                    if u.subs(p) - p.v > error:
                        u = Line(p1, Point(p.t, p.v+error))
                    if p.v - l.subs(p) > error:
                        l = Line(p1, Point(p.t, p.v-error))

            cvx.insert_u(Point(p.t, p.v-error))
            cvx.insert_l(Point(p.t, p.v+error))
            count += 1


# ax2 + bx + c <= u
def ineq_u(slp, itc, u):
    ineq = []
    for p in u:
        if p.t != 0:
            ineq.append((p.v - slp*p.t - itc) / (p.t*p.t))

    return min(ineq)

# ax2 + bx + c >= l
def ineq_l(slp, itc, l):
    ineq = []
    for p in l:
        if p.t != 0:
            ineq.append((p.v - slp*p.t - itc) / (p.t*p.t))

    return max(ineq)
           
            
def load_data(filename):
    data = []
    with open(filename, 'r') as file:
        csvFile = csv.reader(file)
        for line in csvFile:
            data.append(Point(int(line[0]), float(line[1])))
            
    return np.array(data)


# TESTING ENVIRONMENT
STEP = 19
ERROR = 20

# MAIN ENTRY
# data = load_data("data/UCR-time-series/Fungi.csv")
data = load_data("data/Synthesis/quadratic.csv")
p_data = load_data("data/Synthesis/p_quadratic.csv")
swing = Swing()
swing.fit(data, ERROR)
algo = Algo()
algo.fit(data, ERROR)

print(len(swing.segs), len(algo.segs))

# PLOT
plt.figure(figsize=(16, 8))
# plt.plot([p.t for p in data], [p.v for p in data], color="red")
plt.plot([p.t for p in data], [p.v-ERROR for p in data], color="red")
plt.plot([p.t for p in data], [p.v+ERROR for p in data], color="red")
# plt.plot([p.t for p in p_data], [p.v for p in p_data], color="black")


index = 0

for seg in algo.segs:
    t = np.array([index + i for i in range(seg.length)])
    plt.plot(t, [seg.a*i*i + seg.b*i + seg.c for i in range(seg.length)], color="black")
    index += seg.length
    
# for seg in swing.segs:
#     t = np.array([index + i for i in range(seg.length)])
#     slp = (seg.upper.slope + seg.lower.slope)/2 
#     itc = (seg.upper.intercept + seg.lower.intercept)/2
#     # plt.plot(t, [slp*i + itc for i in t], color="black")
#     plt.plot(t, [seg.upper.subs(Point(i, -1)) for i in t], color="green")
#     plt.plot(t, [seg.lower.subs(Point(i, -1)) for i in t], color='blue')
    
#     # cvx_u = seg.cvx.get_u()
#     # plt.plot(cvx_u[0], cvx_u[1], color="green")
#     # cvx_l = seg.cvx.get_l()
#     # plt.plot(cvx_l[0], cvx_l[1], color="blue")
    
#     # u = seg.cvx.get_u_line()
#     # l = seg.cvx.get_l_line()
    
#     # plt.plot(t, [u.subs(Point(i, -1)) for i in t], color="green")
#     # plt.plot(t, [l.subs(Point(i, -1)) for i in t], color='blue')
    
#     index += seg.length

plt.show()


