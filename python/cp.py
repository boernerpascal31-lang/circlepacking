#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Feb 10 15:15:41 2026

@author: boerner
"""

# scip_circle_packing.py
from pyscipopt import Model
import matplotlib.pyplot as plt
from matplotlib.patches import Circle

# create model
model = Model("optimal_circle_packing")

# circle centers and radii
radii = [13, 13, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4]
#radii = [13, 13, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4,2,2,2,2,2,2,2,2,2,2,2,2,2,2]
#radii = [65, 65, 40, 40, 40, 40, 40, 20, 20, 20, 20, 20, 20, 20, 10, 10, 10, 10, 10, 10, 10]
circles = []
for i, r in enumerate(radii):
    xi = model.addVar(f"x_{i}", lb=0.0)
    yi = model.addVar(f"y_{i}", lb=0.0)
    circles.append((xi, yi, float(r)))

# LB
pi = 3.1459
LB = 0.0
for r in radii:
    LB += pi * r*r    

print("Lower bound on area:", LB)

# variables (give nonnegative lower bounds to help solver)
x_max = model.addVar("x_max", lb=0.0)
y_max = model.addVar("y_max", lb=0.0)
obj = model.addVar("obj", lb=LB)

# constraints to keep circles within bounds
for xi, yi, r in circles:
    model.addCons(xi - r >= 0.0)
    model.addCons(xi + r <= x_max)
    model.addCons(yi - r >= 0.0)
    model.addCons(yi + r <= y_max)

model.addCons(x_max <= 800.0)
model.addCons(y_max <= 800.0)

# constraints to prevent circle overlap (nonlinear)
for i in range(len(circles)):
    for j in range(i + 1, len(circles)):
        xi, yi, ri = circles[i]
        xj, yj, rj = circles[j]
        minsep = (ri + rj - 0.1)
        # nonlinear quadratic distance constraint
        model.addCons((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj) >= minsep * minsep)


# objective: minimize area of bounding rectangle (nonlinear product)
model.addCons(x_max * y_max <= obj)
model.setObjective(obj, "minimize")

# -------------------------
# Symmetry breaking: enforce lexicographic ordering of circle centers for circles with equal radii
# -------------------------
# Build indexable lists for convenience (do NOT change existing `circles`)
xvars = [c[0] for c in circles]
yvars = [c[1] for c in circles]

# group indices by equal radii
groups_of_equal_radii = {}
for i, r in enumerate(radii):
    groups_of_equal_radii.setdefault(r, []).append(i)

# Lexicographic ordering in x for equal radii groups
# and a secondary tie-breaker on y using big-M to avoid strict nonlinear lexicographic constraints
BIGM = 800.0  # safe given your x_max/y_max <= 800

for indices in groups_of_equal_radii.values():
    if len(indices) <= 1:
        continue
    # enforce x_i <= x_j for consecutive indices in the group
    for i, j in zip(indices[:-1], indices[1:]):
        model.addCons(xvars[i] <= xvars[j])


# parameters
# time limit: SCIP parameter name is 'limits/time' (seconds)
model.setParam('limits/time', 40000.0)
# verbosity: increase to show progress (value might be adjusted by user)
model.setParam('display/verblevel', 4)

# set time limit
#model.setParam('limits/time', 30)

# optimize
model.optimize()

#add printing sol
solution = model.getBestSol()
if solution is not None:
    model.printSol(solution)

# fetch results and draw
fig, ax = plt.subplots()
for xi, yi, r in circles:
    xv = model.getVal(xi)
    yv = model.getVal(yi)
    circle = Circle((xv, yv), r, fill=False)
    ax.add_artist(circle)

xmax_val = model.getVal(x_max)
ymax_val = model.getVal(y_max)

ax.set_xlim(0, xmax_val)
ax.set_ylim(0, ymax_val)
ax.set_aspect('equal', adjustable='box')
plt.title(f"Area = {xmax_val * ymax_val:.3f}")
plt.show()
