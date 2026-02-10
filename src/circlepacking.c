/*
 * circle_packing_scip.c
 *
 * Minimal SCIP C implementation adapted from the SCIP example `circlepacking.c`.
 * Places a set of given circles into a rectangle minimizing the rectangle area
 * (objective: minimize a = w * h), with non-overlap quadratic constraints
 * ( (x_i-x_j)^2 + (y_i-y_j)^2 >= (r_i + r_j - 0.1)^2 ).
 *
 */



//#define useINIT
//#define CUT_OFF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "scip/scip.h"
#include "scip/scipdefplugins.h"
#include "scip/cons_linear.h"
#include "scip/cons_varbound.h"
#include "scip/cons_nonlinear.h"

int main(void)
{
    SCIP* scip = NULL;
    SCIP_VAR **x = NULL, **y = NULL;
    SCIP_VAR *w = NULL, *h = NULL, *a = NULL; /* width, height, area */
    SCIP_CONS* cons = NULL;
    const int ncircles = 14;
    /* radii (matching your Gurobi example: 13,13,8,8,8,8,8,4,4,4,4,4,4,4) */
    SCIP_Real rvals[14] = {13.0, 13.0, 8.0, 8.0, 8.0, 8.0, 8.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0};

    /* LB */
    SCIP_Real pi = 3.1459;
    SCIP_Real LB = 0.0;
    int i, j;
    for( i = 0; i < ncircles; ++i )
    {
        LB += pi * rvals[i] * rvals[i];
    }

    /* create SCIP environment and include default plugins */
    SCIP_CALL( SCIPcreate(&scip) );
    SCIP_CALL( SCIPincludeDefaultPlugins(scip) );
    SCIP_CALL( SCIPsetIntParam(scip, "display/verblevel", 5) );
    SCIP_CALL( SCIPsetIntParam(scip, "display/freq", 100) );


    /* output detailed information on model options and objective */
    SCIPinfoMessage(scip, NULL, "Creating problem.\n");
    SCIPinfoMessage(scip, NULL, "Lower bound on area: %g\n", LB);

    /* create empty problem */
    SCIP_CALL( SCIPcreateProbBasic(scip, "optimal_circle_packing") );

    /* allocate arrays for x and y variables */
    SCIP_ALLOC( BMSallocMemoryArray(&x, ncircles) );
    SCIP_ALLOC( BMSallocMemoryArray(&y, ncircles) );

    /* create variables
     * x_i, y_i continuous variables.  We'll set loose bounds [0, 80] and
     * enforce tighter bounds with linear constraints relating them to w/h.
     */
    for( i = 0; i < ncircles; ++i )
    {
        char name[64];
        SCIPsnprintf(name, sizeof(name), "x_%d", i);
        SCIP_CALL( SCIPcreateVarBasic(scip, &x[i], name, 0.0, 80.0, 0.0, SCIP_VARTYPE_CONTINUOUS) );
        SCIP_CALL( SCIPaddVar(scip, x[i]) );

        SCIPsnprintf(name, sizeof(name), "y_%d", i);
        SCIP_CALL( SCIPcreateVarBasic(scip, &y[i], name, 0.0, 80.0, 0.0, SCIP_VARTYPE_CONTINUOUS) );
        SCIP_CALL( SCIPaddVar(scip, y[i]) );
    }

    /* create width (w), height (h) and area (a) variables */
    SCIP_CALL( SCIPcreateVarBasic(scip, &w, "x_max", 0.0, 80.0, 0.0, SCIP_VARTYPE_CONTINUOUS) );
    SCIP_CALL( SCIPaddVar(scip, w) );
    SCIP_CALL( SCIPcreateVarBasic(scip, &h, "y_max", 0.0, 80.0, 0.0, SCIP_VARTYPE_CONTINUOUS) );
    SCIP_CALL( SCIPaddVar(scip, h) );

    /* area variable: objective variable (a = w*h will be enforced by a quadratic constraint) */
    SCIP_CALL( SCIPcreateVarBasic(scip, &a, "area", LB, SCIPinfinity(scip), 1.0, SCIP_VARTYPE_CONTINUOUS) );
    SCIP_CALL( SCIPaddVar(scip, a) );

    /* linear boundary constraints to keep circles within [r_i, w-r_i] and [r_i, h-r_i] */
    for( i = 0; i < ncircles; ++i )
    {
        char name[64];
        /* x_i >= r_i  -->  lhs = r_i */
        SCIPsnprintf(name, sizeof(name), "boundaryleft_%d", i);
        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, name, 1, &x[i], & (SCIP_Real){1.0}, rvals[i], SCIPinfinity(scip)) );
        SCIP_CALL( SCIPaddCons(scip, cons) );
        SCIP_CALL( SCIPreleaseCons(scip, &cons) );

        /* x_i + r_i <= w  -->  x_i - w <= -r_i  (rhs = -r_i) */
        SCIP_VAR* vars_xw[2] = { x[i], w };
        SCIP_Real coefs_xw[2] = { 1.0, -1.0 };
        SCIPsnprintf(name, sizeof(name), "boundaryright_%d", i);
        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, name, 2, vars_xw, coefs_xw, -SCIPinfinity(scip), -rvals[i]) );
        SCIP_CALL( SCIPaddCons(scip, cons) );
        SCIP_CALL( SCIPreleaseCons(scip, &cons) );

        /* y_i >= r_i */
        SCIPsnprintf(name, sizeof(name), "boundarybottom_%d", i);
        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, name, 1, &y[i], & (SCIP_Real){1.0}, rvals[i], SCIPinfinity(scip)) );
        SCIP_CALL( SCIPaddCons(scip, cons) );
        SCIP_CALL( SCIPreleaseCons(scip, &cons) );

        /* y_i + r_i <= h  --> y_i - h <= -r_i */
        SCIP_VAR* varsyh[2] = { y[i], h };
        SCIP_Real coefsyh[2] = { 1.0, -1.0 };
        SCIPsnprintf(name, sizeof(name), "boundarytop_%d", i);
        SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, name, 2, varsyh, coefsyh, -SCIPinfinity(scip), -rvals[i]) );
        SCIP_CALL( SCIPaddCons(scip, cons) );
        SCIP_CALL( SCIPreleaseCons(scip, &cons) );
    }

    /* enforce w <= 80 and h <= 80 explicitly (vars already have ub = 80, but we keep these to match your model) */
    SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, "w_ub", 1, &w, & (SCIP_Real){1.0}, -SCIPinfinity(scip), 80.0) );
    SCIP_CALL( SCIPaddCons(scip, cons) );
    SCIP_CALL( SCIPreleaseCons(scip, &cons) );

    SCIP_CALL( SCIPcreateConsBasicLinear(scip, &cons, "h_ub", 1, &h, & (SCIP_Real){1.0}, -SCIPinfinity(scip), 80.0) );
    SCIP_CALL( SCIPaddCons(scip, cons) );
    SCIP_CALL( SCIPreleaseCons(scip, &cons) );

    /* quadratic constraint to link area variable: -a + w*h <= 0  (relaxed as <= 0)
     * we create it as linear var a with coef -1 and one quadratic term w*h with coef 1.
     */
    {
        SCIP_VAR* linvars[1] = { a };
        SCIP_Real lincoefs[1] = { -1.0 };
        SCIP_VAR* quadvars1[1] = { w };
        SCIP_VAR* quadvars2[1] = { h };
        SCIP_Real quadcoefs[1] = { 1.0 };
        SCIP_CALL( SCIPcreateConsQuadraticNonlinear(scip, &cons, "area_link",
                                                  1, linvars, lincoefs,
                                                  1, quadvars1, quadvars2, quadcoefs,
                                                  -SCIPinfinity(scip), 0.0,
                                                  TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );
        SCIP_CALL( SCIPaddCons(scip, cons) );
        SCIP_CALL( SCIPreleaseCons(scip, &cons) );
    }

    /* quadratic non-overlap constraints: (x_i - x_j)^2 + (y_i - y_j)^2 >= (r_i + r_j - 0.1)^2 */
    for( i = 0; i < ncircles; ++i )
    {
        for( j = i+1; j < ncircles; ++j )
        {
            SCIP_VAR* quadvars1[6] = { x[i], x[j], x[i], y[i], y[j], y[i] };
            SCIP_VAR* quadvars2[6] = { x[i], x[j], x[j], y[i], y[j], y[j] };
            SCIP_Real quadcoefs[6] = { 1.0, 1.0, -2.0, 1.0, 1.0, -2.0 };
            SCIP_Real margin = 0.1; /* your model used a tiny overlap-margin 0.1 */
            SCIP_Real rhs = (rvals[i] + rvals[j] - margin) * (rvals[i] + rvals[j] - margin);

            char name[64];
            SCIPsnprintf(name, sizeof(name), "nooverlap_%d_%d", i, j);

            /* create quadratic nonlinear constraint: sum_quad >= rhs  --> lhs = rhs, rhs = +inf */
            SCIP_CALL( SCIPcreateConsQuadraticNonlinear(scip, &cons, name,
                                                      0, NULL, NULL,
                                                      6, quadvars1, quadvars2, quadcoefs,
                                                      rhs, SCIPinfinity(scip),
                                                      TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );
            SCIP_CALL( SCIPaddCons(scip, cons) );
            SCIP_CALL( SCIPreleaseCons(scip, &cons) );
        }
    }

    /* objective already set by variable 'a' with coefficient 1.0; ensure minimization */
    SCIP_CALL( SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE) );

    /* Optional: provide an initial solution (commented out). To enable, fill the start_coords array below
     * with pairs (x,y) for each circle and uncomment the block. The example constructs a SCIP_SOL,
     * sets variable values with SCIPsetSolVal, and then tries to add it with SCIPtrySolFree.
     */
#ifdef useINIT
    {
        SCIP_SOL* sol = NULL;
        SCIP_Bool stored = FALSE;
        SCIP_Real start_coords[14][2] = {
            {13.0, 13.0}, {13.0, 39.0}, {49.4, 44.0}, {49.4, 8.0}, {33.4, 44.0}, {33.4, 8.0}, {29.5, 26.0},
            {54.4, 30.0}, {54.4, 22.0}, {46.8, 19.7}, {46.8, 32.2}, {41.8, 26.0}, {38.9, 18.6}, {38.9, 33.4}
        };
        SCIP_CALL( SCIPcreateSol(scip, &sol, NULL) );
        for( i = 0; i < ncircles; ++i )
        {
            SCIP_CALL( SCIPsetSolVal(scip, sol, x[i], start_coords[i][0]) );
            SCIP_CALL( SCIPsetSolVal(scip, sol, y[i], start_coords[i][1]) );
        }
        SCIP_CALL( SCIPsetSolVal(scip, sol, w, 58.4) );
        SCIP_CALL( SCIPsetSolVal(scip, sol, h, 52.0) );
        SCIP_CALL( SCIPsetSolVal(scip, sol, a, 58.4 * 52.0) );

        SCIP_CALL( SCIPtransformProb(scip));

        /* try to add solution (free after try) */
        SCIP_CALL( SCIPtrySolFree(scip, &sol, TRUE, FALSE, TRUE, TRUE, TRUE, &stored) );
        if( stored )
            SCIPinfoMessage(scip, NULL, "initial solution accepted\n");
        else
            SCIPinfoMessage(scip, NULL, "initial solution not stored\n");
    }
#endif

    SCIP_CALL( SCIPwriteOrigProblem(scip, "circle_packing.cip", "cip", FALSE) );

#ifdef CUT_OFF
    SCIP_CALL( SCIPchgVarUb(scip, a, 2000) );
#endif

    /* output detailed information on model options and objective */
    SCIPinfoMessage(scip, NULL, "Solving problem.\n");
    /* Transform problem before solve */
    SCIP_CALL( SCIPtransformProb(scip) );
    /* solve */
    SCIP_CALL( SCIPsolve(scip) );

    /* print best solution */
    {
        SCIP_SOL* best = SCIPgetBestSol(scip);
        if( best != NULL )
        {
            SCIPinfoMessage(scip, NULL, "Best objective (area) = %g\n", SCIPgetSolVal(scip, best, a));
            SCIPinfoMessage(scip, NULL, "w = %g, h = %g\n", SCIPgetSolVal(scip, best, w), SCIPgetSolVal(scip, best, h));
            for( i = 0; i < ncircles; ++i )
            {
                SCIPinfoMessage(scip, NULL, "circle %d: x=%g y=%g r=%g\n", i,
                                SCIPgetSolVal(scip, best, x[i]), SCIPgetSolVal(scip, best, y[i]), rvals[i]);
            }
        }
        else
        {
            SCIPinfoMessage(scip, NULL, "No feasible solution found\n");
        }
    }

    /* free variables and arrays */
    for( i = 0; i < ncircles; ++i )
    {
        SCIP_CALL( SCIPreleaseVar(scip, &x[i]) );
        SCIP_CALL( SCIPreleaseVar(scip, &y[i]) );
    }
    SCIP_CALL( SCIPreleaseVar(scip, &w) );
    SCIP_CALL( SCIPreleaseVar(scip, &h) );
    SCIP_CALL( SCIPreleaseVar(scip, &a) );

    BMSfreeMemoryArray(&x);
    BMSfreeMemoryArray(&y);

    /* free SCIP */
    SCIP_CALL( SCIPfree(&scip) );

    return 0;
}
