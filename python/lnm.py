
import gurobipy as gp


model = gp.Model("optimal_circle_packing")

x_max = model.addVar(name="x_max")
y_max = model.addVar(name="y_max")

# circle centers and radii
circles = [
    (model.addVar(name=f"x_{i}"), model.addVar(name=f"y_{i}"), r)
    # for i, r in enumerate([130, 130, 80, 80, 80, 80, 80, 40, 40, 40, 40, 40, 40, 40])
    for i, r in enumerate([13, 13, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4])
]

# constraints to keep circles within bounds
for x_i, y_i, r in circles:
    model.addConstr(x_i - r >= 0)
    model.addConstr(x_i + r <= x_max)
    model.addConstr(y_i - r >= 0)
    model.addConstr(y_i + r <= y_max)

model.addConstr(x_max <= 80)
model.addConstr(y_max <= 80)

# constraints to prevent circle overlap
for i in range(len(circles)):
    for j in range(i + 1, len(circles)):
        x_i, y_i, r_i = circles[i]
        x_j, y_j, r_j = circles[j]
        model.addConstr(
            (x_i - x_j) * (x_i - x_j) + (y_i - y_j) * (y_i - y_j) >= (r_i + r_j - 0.1) * (r_i + r_j - 0.1)
        )

# objective: minimize area of bounding rectangle
model.setObjective(x_max * y_max, gp.GRB.MINIMIZE)

model.setParam('TimeLimit', 40000)
model.setParam("OutputFlag", 1)
model.setParam('NonConvex', 2)

# provide starting solution:
initial_positions = [
    (13, 13), (13, 39),
    (49.4, 44), (49.4, 8), (33.4, 44), (33.4, 8), (29.5, 26),
    (54.4, 30), (54.4, 22), (46.8, 19.7), (46.8, 32.2), (41.8, 26), (38.9, 18.6), (38.9, 33.4)
]

for (x_i, y_i, r), (x_start, y_start) in zip(circles, initial_positions):
    x_i.Start = x_start
    y_i.Start = y_start

x_max.Start = 58.4
y_max.Start = 52


model.optimize()

# draw results
import matplotlib.pyplot as plt
from matplotlib.patches import Circle

fig, ax = plt.subplots()
for x_i, y_i, r in circles:
    circle = Circle((x_i.X, y_i.X), r, fill=False)
    ax.add_artist(circle)
ax.set_xlim(0, x_max.X)
ax.set_ylim(0, y_max.X)
ax.set_aspect('equal', adjustable='box')
plt.show()
