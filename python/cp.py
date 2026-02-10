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

# variables (give nonnegative lower bounds to help solver)
x_max = model.addVar("x_max", lb=0.0)
y_max = model.addVar("y_max", lb=0.0)

# circle centers and radii
radii = [13, 13, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 4]
circles = []
for i, r in enumerate(radii):
    xi = model.addVar(f"x_{i}", lb=0.0)
    yi = model.addVar(f"y_{i}", lb=0.0)
    circles.append((xi, yi, float(r)))

# constraints to keep circles within bounds
for xi, yi, r in circles:
    model.addCons(xi - r >= 0.0)
    model.addCons(xi + r <= x_max)
    model.addCons(yi - r >= 0.0)
    model.addCons(yi + r <= y_max)

model.addCons(x_max <= 80.0)
model.addCons(y_max <= 80.0)

# constraints to prevent circle overlap (nonlinear)
for i in range(len(circles)):
    for j in range(i + 1, len(circles)):
        xi, yi, ri = circles[i]
        xj, yj, rj = circles[j]
        minsep = (ri + rj - 0.1)
        # nonlinear quadratic distance constraint
        model.addCons((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj) >= minsep * minsep)

# objective: minimize area of bounding rectangle (nonlinear product)
model.setObjective(x_max * y_max, "minimize")

# parameters
# time limit: SCIP parameter name is 'limits/time' (seconds)
model.setParam('limits/time', 40000.0)
# verbosity: increase to show progress (value might be adjusted by user)
model.setParam('display/verblevel', 4)

# provide starting solution (feasible initial solution)
initial_positions = [
    (13, 13), (13, 39),
    (49.4, 44), (49.4, 8), (33.4, 44), (33.4, 8), (29.5, 26),
    (54.4, 30), (54.4, 22), (46.8, 19.7), (46.8, 32.2), (41.8, 26), (38.9, 18.6), (38.9, 33.4)
]

sol = model.createSol()
for (xi, yi, _), (xs, ys) in zip(circles, initial_positions):
    model.setSolVal(sol, xi, float(xs))
    model.setSolVal(sol, yi, float(ys))
model.setSolVal(sol, x_max, 58.4)
model.setSolVal(sol, y_max, 52.0)

# add the solution to SCIP's solution pool as a feasible start
# the second argument (check) defaults to True when omitted; if you want to skip checking use False.
model.addSol(sol)

# optimize
model.optimize()

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
