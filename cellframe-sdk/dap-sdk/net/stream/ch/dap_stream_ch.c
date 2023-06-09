/*
 Copyright (c) 2017-2018 (c) Project "DeM Labs Inc" https://github.com/demlabsinc
  All rights reserved.

 This file is part of DAP (Deus Applications Prototypes) the open source project

    DAP (Deus Applicaions Prototypes) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DAP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with any DAP based project.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <io.h>
#endif

#include <pthread.h>

#include "dap_common.h"
#include "dap_events.h"
#include "dap_events_socket.h"
#include "dap_http_client.h"
#include "dap_uuid.h"
#include "dap_stream.h"
#include "dap_stream_ch.h"
#include "dap_stream_ch_proc.h"
#include "dap_stream_ch_pkt.h"
#include "dap_stream_worker.h"

#define LOG_TAG "dap_stream_ch"

#ifdef  DAP_SYS_DEBUG
enum    {MEMSTAT$K_STM_CH, MEMSTAT$K_NR};
static  dap_memstat_rec_t   s_memstat [MEMSTAT$K_NR] = {
    {.fac_len = sizeof(LOG_TAG) - 1, .fac_name = {LOG_TAG}, .alloc_sz = sizeof(dap_stream_ch_t)},
};
#endif


/**
 * @brief dap_stream_ch_init Init stream channel module
 * @return Zero if ok others if no
 */
int dap_stream_ch_init()
{
    if(stream_ch_proc_init() != 0 ){
        log_it(L_CRITICAL,"Can't init stream channel proc submodule");
        return -1;
    }
    if(dap_stream_ch_pkt_init() != 0 ){
        log_it(L_CRITICAL,"Can't init stream channel packet submodule");
        return -1;
    }


#ifdef  DAP_SYS_DEBUG
    for (int i = 0; i < MEMSTAT$K_NR; i++)
        dap_memstat_reg(&s_memstat[i]);
#endif

    log_it(L_NOTICE,"Module stream channel initialized");
    return 0;
}

/**
 * @brief dap_stream_ch_deinit Destroy stream channel submodule
 */
void dap_stream_ch_deinit()
{
}



typedef struct __dap_stm_ch_rec__ {
    dap_stream_ch_t     *stm_ch;
    UT_hash_handle          hh;
} dap_stm_ch_rec_t;

static dap_stm_ch_rec_t     *s_stm_chs = NULL;                          /* @RRL:  A has table to track using of events sockets context */
static pthread_rwlock_t     s_stm_ch_lock = PTHREAD_RWLOCK_INITIALIZER;


/*
 *   DESCRIPTION: Allocate a new <dap_stream_ch_t> context, add record into the hash table to track usage
 *      of the contexts.
 *
 *   INPUTS:
 *      NONE
 *
 *   IMPLICITE INPUTS:
 *      s_stm_chs;      A hash table
 *
 *   OUTPUTS:
 *      NONE
 *
 *   IMPLICITE OUTPUTS:
 *      s_stm_chs
 *
 *   RETURNS:
 *      non-NULL        A has been allocated <dap_events_socket> context
 *      NULL:           See <errno>
 */
static inline dap_stream_ch_t *dap_stream_ch_alloc (void)
{
int     l_rc;
dap_stream_ch_t *l_stm_ch;
dap_stm_ch_rec_t    *l_rec;

    if ( !(l_stm_ch = DAP_NEW_Z( dap_stream_ch_t )) )                       /* Allocate memory for new dap_events_socket context and the record */
        return  log_it(L_CRITICAL, "Cannot allocate memory for <dap_stream_ch_t> context, errno=%d", errno), NULL;

    if ( !(l_rec = DAP_NEW_Z( dap_stm_ch_rec_t )) )                         /* Allocate memory for new record */
        return  log_it(L_CRITICAL, "Cannot allocate memory for record, errno=%d", errno),
                DAP_DELETE(l_stm_ch), NULL;

    l_rec->stm_ch = l_stm_ch;                                               /* Fill new track record */

                                                                            /* Add new record into the hash table */
    l_rc = pthread_rwlock_wrlock(&s_stm_ch_lock);
    assert(!l_rc);
    HASH_ADD(hh, s_stm_chs, stm_ch, sizeof(dap_events_socket_t *), l_rec );

#ifdef  DAP_SYS_DEBUG
    s_memstat[MEMSTAT$K_STM_CH].alloc_nr += 1;
#endif
    l_rc = pthread_rwlock_unlock(&s_stm_ch_lock);
    assert(!l_rc);

    debug_if(g_debug_reactor, L_NOTICE, "dap_stream_ch_t:%p - is allocated", l_stm_ch);

    return  l_stm_ch;
}

/*
 *   DESCRIPTION: Release has been allocated <dap_stream_ch_t>. Check firstly against hash table.
 *
 *   INPUTS:
 *      a_stm_ch:   A context to be released
 *
 *   IMPLICITE INPUTS:
 *      s_stm_chs;      A hash table
 *
 *   OUTPUT:
 *      NONE
 *
 *   IMPLICITE OUTPUTS:
 *      s_stm_chs
 *
 *   RETURNS:
 *      0:          a_es contains valid pointer
 *      <errno>
 */
static inline int dap_stm_ch_free (
                    dap_stream_ch_t *a_stm_ch
                        )
{
int     l_rc;
dap_stm_ch_rec_t    *l_rec = NULL;

    l_rc = pthread_rwlock_wrlock(&s_stm_ch_lock);
    assert(!l_rc);

    HASH_FIND(hh, s_stm_chs, &a_stm_ch, sizeof(dap_stream_ch_t *), l_rec );
    if ( l_rec && (l_rec->stm_ch == a_stm_ch) )
        HASH_DELETE(hh, s_stm_chs, l_rec);                           /* Remove record from the table */

#ifdef  DAP_SYS_DEBUG
    atomic_fetch_add(&s_memstat[MEMSTAT$K_STM_CH].free_nr, 1);
#endif

    l_rc = pthread_rwlock_unlock(&s_stm_ch_lock);
    assert(!l_rc);

    if ( !l_rec )
        log_it(L_ERROR, "dap_stream_ch_t:%p - no record found!", a_stm_ch);
    else {
        DAP_DELETE(l_rec->stm_ch);
        DAP_DELETE(l_rec);



        debug_if(g_debug_reactor, L_NOTICE, "dap_stream_ch_t:%p - is released", a_stm_ch);

    }

    return  0;  /* SS$_SUCCESS */
}





/**
 * @brief dap_stream_ch_new Creates new stream channel instance
 * @return
 */
dap_stream_ch_t* dap_stream_ch_new(dap_stream_t* a_stream, uint8_t a_id)
{
    stream_ch_proc_t * proc=dap_stream_ch_proc_find(a_id);
    if(proc){
        //dap_stream_ch_t* l_ch_new = DAP_NEW_Z(dap_stream_ch_t);
        dap_stream_ch_t* l_ch_new = dap_stream_ch_alloc();

        l_ch_new->me = l_ch_new;
        l_ch_new->stream = a_stream;
        l_ch_new->proc = proc;
        l_ch_new->ready_to_read = true;
        l_ch_new->uuid = dap_uuid_generate_uint64();

        pthread_mutex_init(&(l_ch_new->mutex),NULL);

        // Init on stream worker
        dap_stream_worker_t * l_stream_worker = a_stream->stream_worker;
        l_ch_new->stream_worker = l_stream_worker;

        pthread_rwlock_wrlock(&l_stream_worker->channels_rwlock);
        HASH_ADD(hh_worker,l_stream_worker->channels, uuid,sizeof (l_ch_new->uuid ),l_ch_new);
        pthread_rwlock_unlock(&l_stream_worker->channels_rwlock);


        // Proc new callback
        if(l_ch_new->proc->new_callback)
            l_ch_new->proc->new_callback(l_ch_new,NULL);

        a_stream->channel[l_ch_new->stream->channel_count] = l_ch_new;
        a_stream->channel_count++;

        return l_ch_new;
    }else{
        log_it(L_WARNING, "Unknown stream processor with id %uc",a_id);
        return NULL;
    }
}

/**
 * @brief stream_ch_delete Delete channel instance
 * @param ch Channel delete
 */
void dap_stream_ch_delete(dap_stream_ch_t *a_ch)
{
    dap_stream_worker_t * l_stream_worker = a_ch->stream_worker;
    if(l_stream_worker){
        pthread_rwlock_wrlock(&l_stream_worker->channels_rwlock);
        HASH_DELETE(hh_worker,l_stream_worker->channels, a_ch);
        pthread_rwlock_unlock(&l_stream_worker->channels_rwlock);
    }

    pthread_mutex_lock(&a_ch->mutex);

    if (a_ch->proc)
        if (a_ch->proc->delete_callback)
            a_ch->proc->delete_callback(a_ch, NULL);
    a_ch->stream->channel[a_ch->stream->channel_count--] = NULL;

    pthread_mutex_unlock(&a_ch->mutex);

    //DAP_DELETE(a_ch);
    dap_stm_ch_free (a_ch);
}

/**
 * @brief dap_stream_ch_find_by_uuid_unsafe
 * @param a_worker
 * @param a_ch_uuid
 * @return
 */
dap_stream_ch_t * dap_stream_ch_find_by_uuid_unsafe(dap_stream_worker_t * a_worker, dap_stream_ch_uuid_t a_ch_uuid)
{
    dap_stream_ch_t *l_ch = NULL;

    if( a_worker == NULL ){
        log_it(L_WARNING,"Attempt to search for uuid 0x%016"DAP_UINT64_FORMAT_U" in NULL worker", a_ch_uuid);
        return NULL;
    }

    pthread_rwlock_rdlock(&a_worker->channels_rwlock);
    if ( a_worker->channels)
        HASH_FIND(hh_worker,a_worker->channels ,&a_ch_uuid, sizeof(a_ch_uuid), l_ch );
    pthread_rwlock_unlock(&a_worker->channels_rwlock);
    return l_ch;

}

/**
 * @brief dap_stream_ch_set_ready_to_read
 * @param a_ch
 * @param a_is_ready
 */
void dap_stream_ch_set_ready_to_read_unsafe(dap_stream_ch_t * a_ch,bool a_is_ready)
{
    if( a_ch->ready_to_read != a_is_ready){
        //log_it(L_DEBUG,"Change channel '%c' to %s", (char) ch->proc->id, is_ready?"true":"false");
        a_ch->ready_to_read=a_is_ready;
        dap_events_socket_set_readable_unsafe(a_ch->stream->esocket, a_is_ready);
    }
}

/**
 * @brief dap_stream_ch_set_ready_to_write
 * @param ch
 * @param is_ready
 */
void dap_stream_ch_set_ready_to_write_unsafe(dap_stream_ch_t * ch,bool is_ready)
{
    if(ch->ready_to_write!=is_ready){
        //log_it(L_DEBUG,"Change channel '%c' to %s", (char) ch->proc->id, is_ready?"true":"false");
        ch->ready_to_write=is_ready;
        if(is_ready && ch->stream->conn_http)
            ch->stream->conn_http->state_write=DAP_HTTP_CLIENT_STATE_DATA;
        dap_events_socket_set_writable_unsafe(ch->stream->esocket, is_ready);
    }
}

static void s_print_workers_channels()
{
    uint32_t l_worker_count = dap_events_worker_get_count();
    dap_stream_ch_t* l_msg_ch = NULL;
    dap_stream_ch_t* l_msg_ch_tmp = NULL;
    //print all worker connections
    dap_events_worker_print_all();
    for (uint32_t i = 0; i < l_worker_count; i++){
        uint32_t l_channel_count = 0;
        dap_worker_t* l_worker = dap_events_worker_get(i);
        if (!l_worker) {
            log_it(L_CRITICAL, "Can't get stream worker - worker thread don't exist");
            continue;
        }
        dap_stream_worker_t* l_stream_worker = DAP_STREAM_WORKER(l_worker);
        if (l_stream_worker->channels)
            HASH_ITER(hh_worker, l_stream_worker->channels, l_msg_ch, l_msg_ch_tmp) {
                //log_it(L_DEBUG, "Worker id = %d, channel uuid = 0x%llx", l_worker->id, l_msg_ch->uuid);
                l_channel_count += 1;
        }
        log_it(L_DEBUG, "Active workers l_channel_count = %d on worker %d", l_channel_count, l_stream_worker->worker->id);
    }
    return;
}
