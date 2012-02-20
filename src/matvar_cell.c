/*
 * Copyright (C) 2012   Christopher C. Hulbert
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRISTOPHER C. HULBERT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CHRISTOPHER C. HULBERT OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <string.h>
#include "matio_private.h"

/** @brief Returns a pointer to the Cell array at a specific index
 *
 * Returns a pointer to the Cell Array Field at the given 1-relative index.
 * MAT file must be a version 5 matlab file.
 * @ingroup MAT
 * @param matvar Pointer to the Cell Array MAT variable
 * @param index linear index of cell to return
 * @return Pointer to the Cell Array Field on success, NULL on error
 */
matvar_t *
Mat_VarGetCell(matvar_t *matvar,int index)
{
    int       nmemb = 1, i;
    matvar_t *cell = NULL;

    if ( matvar == NULL )
        return NULL;

    for ( i = 0; i < matvar->rank; i++ )
        nmemb *= matvar->dims[i];

    if ( index < nmemb )
        cell = *((matvar_t **)matvar->data + index);

    return cell;
}

/** @brief Indexes a cell array
 *
 * Finds cells of a cell array given a start, stride, and edge for each.
 * dimension.  The cells are placed in a pointer array.  The cells should not
 * be freed, but the array of pointers should be.  If copies are needed,
 * use Mat_VarDuplicate on each cell.
 * MAT File version must be 5.
 * @ingroup MAT
 * @param matvar Cell Array matlab variable
 * @param start vector of length rank with 0-relative starting coordinates for
 *              each diemnsion.
 * @param stride vector of length rank with strides for each diemnsion.
 * @param edge vector of length rank with the number of elements to read in
 *              each diemnsion.
 * @returns an array of pointers to the cells
 */
matvar_t **
Mat_VarGetCells(matvar_t *matvar,int *start,
                int *stride,int *edge)
{
    int i, j, N, I = 0;
    int inc[10] = {0,}, cnt[10] = {0,}, dimp[10] = {0,};
    matvar_t **cells;

    if ( (matvar == NULL) || (start == NULL) || (stride == NULL) ||
        (edge == NULL) ) {
        return NULL;
    } else if ( matvar->rank > 10 ) {
        return NULL;
    }

    inc[0] = stride[0]-1;
    dimp[0] = matvar->dims[0];
    N = edge[0];
    I = start[0];
    for ( i = 1; i < matvar->rank; i++ ) {
        inc[i]  = stride[i]-1;
        dimp[i] = matvar->dims[i-1];
        for ( j = i ; j--; ) {
            inc[i]  *= matvar->dims[j];
            dimp[i] *= matvar->dims[j+1];
        }
        N *= edge[i];
        if ( start[i] > 0 )
            I += start[i]*dimp[i-1];
    }
    cells = malloc(N*sizeof(matvar_t *));
    for ( i = 0; i < N; i+=edge[0] ) {
        for ( j = 0; j < edge[0]; j++ ) {
            cells[i+j] = *((matvar_t **)matvar->data + I);
            I += stride[0];
        }
        for ( j = 1; j < matvar->rank-1; j++ ) {
            cnt[j]++;
            if ( (cnt[j] % edge[j]) == 0 ) {
                cnt[j] = 0;
                if ( (I % dimp[j]) != 0 ) {
                    I += dimp[j]-(I % dimp[j]);
                }
            } else {
                I += matvar->dims[0]-edge[0]*stride[0]-start[0];
                I += inc[j];
                break;
            }
        }
    }
    return cells;
}

/** @brief Indexes a cell array
 *
 * Finds cells of a cell array given a linear indexed start, stride, and edge.
 * The cells are placed in a pointer array.  The cells themself should not
 * be freed as they are part of the original cell array, but the pointer array
 * should be.  If copies are needed, use Mat_VarDuplicate on each of the cells.
 * MAT file version must be 5.
 * @ingroup MAT
 * @param matvar Cell Array matlab variable
 * @param start starting index
 * @param stride stride
 * @param edge Number of cells to get
 * @returns an array of pointers to the cells
 */
matvar_t **
Mat_VarGetCellsLinear(matvar_t *matvar,int start,int stride,int edge)
{
    int i, I = 0;
    matvar_t **cells;

    if ( matvar == NULL || matvar->rank > 10 ) {
        cells = NULL;
    } else {
        cells = malloc(edge*sizeof(matvar_t *));
        for ( i = 0; i < edge; i++ ) {
            cells[i] = *((matvar_t **)matvar->data + I);
            I += stride;
        }
    }
    return cells;
}
