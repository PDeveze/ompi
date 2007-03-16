/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (C) 2006      Cisco Systems, Inc.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */
#include "ompi_config.h"
#include <stdio.h>

#include "ompi/mpi/c/bindings.h"
#include "ompi/request/request.h"
#include "ompi/request/grequest.h"

#if OMPI_HAVE_WEAK_SYMBOLS && OMPI_PROFILING_DEFINES
#pragma weak MPI_Request_get_status = PMPI_Request_get_status
#endif

#if OMPI_PROFILING_DEFINES
#include "ompi/mpi/c/profile/defines.h"
#endif

static const char FUNC_NAME[] = "MPI_Request_get_status";

/* Non blocking test for the request status. Upon completion, the request will
 * not be freed (unlike the test function). A subsequent call to test, wait
 * or free should be executed on the request.
 */
int MPI_Request_get_status(MPI_Request request, int *flag,
                           MPI_Status *status) 
{
    OPAL_CR_TEST_CHECKPOINT_READY();

    if( MPI_PARAM_CHECK ) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if( (NULL == flag) || (NULL == status) ) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_ARG, FUNC_NAME);
        }
    }

    opal_atomic_mb();
    if( (request == MPI_REQUEST_NULL) || (request->req_state == OMPI_REQUEST_INACTIVE) ) {
        *flag = true;
        if( MPI_STATUS_IGNORE != status ) {
            *status = ompi_status_empty;
        }
        return MPI_SUCCESS;
    }
    if( request->req_complete ) { 
        *flag = true; 
        /* If this is a generalized request, we *always* have to call
           the query function to get the status (MPI-2:8.2), even if
           the user passed STATUS_IGNORE. */
        if (OMPI_REQUEST_GEN == request->req_type) {
            ompi_grequest_invoke_query(request, &request->req_status);
        }
        if (MPI_STATUS_IGNORE != status) {
            *status = request->req_status;
        }
        return MPI_SUCCESS;
    }
    *flag = false;
#if OMPI_ENABLE_PROGRESS_THREADS == 0
    opal_progress();
#endif
    return MPI_SUCCESS;
}
