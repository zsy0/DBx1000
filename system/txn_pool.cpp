#include "txn_pool.h"
#include "global.h"
#include "helper.h"
#include "txn.h"
#include "mem_alloc.h"
#include "wl.h"
#include "ycsb_query.h"
#include "query.h"
#include "msg_queue.h"

void TxnPool::init(workload * wl, uint64_t size) {
  _wl = wl;
  //txn_man * items = (txn_man*)mem_allocator.alloc(sizeof(txn_man)*size,0);
  txn_man * txn;
  for(uint64_t i = 0; i < size; i++) {
    //put(items[i]);
    _wl->get_txn_man(txn);
    put(txn);
  }
}

void TxnPool::get(txn_man *& item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    _wl->get_txn_man(item);
  }
  item->reset();
}

void TxnPool::put(txn_man * item) {
  pool.enqueue(item);
}

void TxnPool::free_all() {
  txn_man * item;
  while(pool.try_dequeue(item)) {
    mem_allocator.free(item,sizeof(item));
  }
}

void AccessPool::init(workload * wl, uint64_t size) {
  _wl = wl;
  Access * items = (Access*)mem_allocator.alloc(sizeof(Access)*size,0);
  for(uint64_t i = 0; i < size; i++) {
    put(&items[i]);
  }
}

void AccessPool::get(Access *& item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    DEBUG_M("access_pool\n");
    item = (Access*)mem_allocator.alloc(sizeof(Access),0);
  }
}

void AccessPool::put(Access * item) {
  pool.enqueue(item);
}

void AccessPool::free_all() {
  Access * item;
  while(pool.try_dequeue(item)) {
    mem_allocator.free(item,sizeof(item));
  }
}

void TxnTablePool::init(workload * wl, uint64_t size) {
  _wl = wl;
  //txn_man * items = (txn_man*)mem_allocator.alloc(sizeof(txn_man)*size,0);
    txn_node * t_node = (txn_node *) mem_allocator.alloc(sizeof(struct txn_node) * size, g_thread_cnt);
  for(uint64_t i = 0; i < size; i++) {
    put(&t_node[i]);
  }
}

void TxnTablePool::get(txn_node *& item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    DEBUG_M("txn_table_pool\n");
    item = (txn_node *) mem_allocator.alloc(sizeof(struct txn_node), g_thread_cnt);
  }
}

void TxnTablePool::put(txn_node * item) {
  pool.enqueue(item);
}

void TxnTablePool::free_all() {
  txn_node * item;
  while(pool.try_dequeue(item)) {
    mem_allocator.free(item,sizeof(item));
  }
}

void QryPool::init(workload * wl, uint64_t size) {
  _wl = wl;
  base_query * qry=NULL;
  for(uint64_t i = 0; i < size; i++) {
    //put(items[i]);
#if WORKLOAD==YCSB
    ycsb_query * m_qry = (ycsb_query *) mem_allocator.alloc(sizeof(ycsb_query),0);
    m_qry = new ycsb_query();
    m_qry->requests = (ycsb_request*)mem_allocator.alloc(sizeof(ycsb_request)*REQ_PER_QUERY,0);
    m_qry->part_to_access = (uint64_t*)mem_allocator.alloc(sizeof(uint64_t)*PART_PER_TXN,0);
    qry = m_qry;
#endif
    put(qry);
  }
}

void QryPool::get(base_query *& item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    DEBUG_M("query_pool\n");
#if WORKLOAD==YCSB
    ycsb_query * qry = NULL;
    qry = (ycsb_query *) mem_allocator.alloc(sizeof(ycsb_query),0);
    qry = new ycsb_query();
    qry->requests = (ycsb_request*)mem_allocator.alloc(sizeof(ycsb_request)*REQ_PER_QUERY,0);
    qry->part_to_access = (uint64_t*)mem_allocator.alloc(sizeof(uint64_t)*PART_PER_TXN,0);
#endif
    item = (base_query*)qry;
  }
  item->clear();
  item->base_reset();
  item->reset();
}

void QryPool::put(base_query * item) {
  assert(item);
  /*
  ycsb_query * qry = (ycsb_query *) item;
  mem_allocator.free(qry->requests,sizeof(ycsb_query)*REQ_PER_QUERY);
  mem_allocator.free(qry->part_to_access,sizeof(uint64_t)*PART_PER_TXN);
  printf("freeing %lx\n",(uint64_t)qry);
  fflush(stdout);
  assert(qry);
  mem_allocator.free(qry,sizeof(ycsb_query));
  */

  pool.enqueue(item);
}

void QryPool::free_all() {
  base_query * item;
  while(pool.try_dequeue(item)) {
    mem_allocator.free(item,sizeof(item));
  }
}

void MsgPool::init(workload * wl, uint64_t size) {
  _wl = wl;
  msg_entry* entry;
  for(uint64_t i = 0; i < size; i++) {
    entry = (msg_entry*) mem_allocator.alloc(sizeof(struct msg_entry), 0);
    put(entry);
  }
}

void MsgPool::get(msg_entry* & item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    DEBUG_M("msg_pool\n");
    item = (msg_entry*) mem_allocator.alloc(sizeof(struct msg_entry), 0);
  }
}

void MsgPool::put(msg_entry* item) {
  pool.enqueue(item);
}

void MsgPool::free_all() {
  msg_entry * item;
  while(pool.try_dequeue(item)) {
    mem_allocator.free(item,sizeof(item));
  }
}

/*
template <class T>
void TxnPool::init(uint64_t size) {
  T * items = (T*)mem_allocator.alloc(sizeof(T)*size,0);
  for(uint64_t i = 0; i < size; i++) {
    put(items[i]);
  }
}

template <class T>
void TxnPool::get(T *& item) {
  bool r = pool.try_dequeue(item);
  if(!r) {
    item = (T)mem_allocator.alloc(sizeof(T),0);
  }
}

template <class T>
void TxnPool::put(T * item) {
  pool.enqueue(item);
}
*/
