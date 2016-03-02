/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /log/lock_free.hpp
 *
 *  Created on: 2016年2月29日
 *      Author: imdb
 *		   Email: 
 * 
 * Description:
 *
 */

#ifndef LOCK_FREE_HPP_
#define LOCK_FREE_HPP_
#include <memory>
using std::atomic;
using std::shared_ptr;
using std::make_shared;
using RetCode = int;
/*
 * 无锁链表
 * 支持多线程头部插入,读取
 * 单线程删除
 */
template<typename T>
class Node{
 public:
  T PayLoad;
  shared_ptr<Node<T>> Next;
  Node(){}
  shared_ptr<Node<T>> getNext() { return Next;}
  string ToString() {
     return String(PayLoad);
  }
};

template<typename T>
class LockFreeList {
 public:
  atomic<shared_ptr<Node<T>>> Head;
  LockFreeList() { Head=nullptr;}
        shared_ptr<Node<T>> Insert() {
    shared_ptr<Node<T>> new_node = make_shared<Node<T>>();
    do {
        new_node->Next = Head;
    } while (!Head.compare_exchange_strong(new_node->Next, new_node));

    return new_node;
   }
  RetCode RemoveNext(shared_ptr<Node<T>>  & pre) {
    if (pre->Next != nullptr) {
      pre->Next = pre->Next->Next;
     }
    return 0;
  }
  string ToString() {
    string buffer = "lockfreelist\n";
    shared_ptr<Node<T>> ptr = Head;
    while (ptr != nullptr) {
      cout << "iterator1" << endl;
      buffer += ptr->PayLoad.ToString() + string("\n");
      cout << "iterator2" << endl;
      ptr = ptr->Next;
      cout << "iterator3" << endl;
    }
    return buffer;
  }
};
template<typename T>
class Node2{
 public:
  T PayLoad;
  Node2<T> * Next;
  Node2(){}
  Node2<T> getNext() { return Next;}
  string ToString() {
     return String(PayLoad);
  }
};

template<typename T>
class LockFreeList2 {
 public:
  atomic<Node2<T>*> Head;
  LockFreeList2() { Head = nullptr;}
  T* Insert() {
   Node2<T> *  new_node = new Node2<T>();

    do {
        new_node->Next = Head.load();
    } while (!Head.compare_exchange_weak(new_node->Next, new_node));

    return &new_node->PayLoad;

   }
  RetCode RemoveNext(shared_ptr<Node<T>>  & pre) {
    if (pre->Next != nullptr) {
      pre->Next = pre->Next->Next;
     }
    return 0;
  }
  string ToString() {
    string buffer = "lockfreelist\n";
    auto ptr = Head.load();
    while (ptr != nullptr) {
      cout << "iterator1" << endl;
      buffer += ptr->PayLoad.ToString() + string("\n");
      cout << "iterator2" << endl;
      ptr = ptr->Next;
      cout << "iterator3" << endl;
    }
    return buffer;
  }
};
#endif //  LOCK_FREE_HPP_ 
